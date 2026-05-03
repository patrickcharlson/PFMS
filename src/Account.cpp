//
// Created by Patrick Charlson on 21/4/2026.
//

#include "Account.h"

#include <cmath>
#include <iomanip>
#include <sstream>

inline double round2(const double v) { return std::round(v * 100.0) / 100.0; }

inline std::string fmtMoney(const double v) {
  std::ostringstream oss;
  oss << "$" << std::fixed << std::setprecision(2) << v;
  return oss.str();
}

double Account::committedTotal() const {
  double sum = 0.0;
  for (const auto& b: buckets_)
    if (b.committed())
      sum += b.balance();
  return round2(sum);
}


// --------- Bucket management ---------

Status Account::createBucket(const std::string& name, double percentage, bool committed) {
  if (name.empty())
    return Status::failure("Bucket name cannot be empty.");

  if (percentage < 0.0 || percentage > 100.0)
    return Status::failure("Percentage must be between 0 and 100.");

  if (const double newTotal = allocatedPercentageTotal() + percentage; newTotal > 100.0 + 1e-9)
    return Status::failure("Total bucket allocation would exceed 100%. Currently allocated: " +
                           std::to_string(static_cast<int>(allocatedPercentageTotal())) + "%.");

  buckets_.emplace_back(name, percentage, committed);
  return Status::success("Bucket '" + name + "' created.");
}

Status Account::editBucket(const size_t index, const std::string& newName, const double newPercentage) {
  if (index >= buckets_.size())
    return Status::failure("Invalid bucket selection");

  if (newName.empty())
    return Status::failure("Bucket name cannot be empty.");

  if (newPercentage < 0.0 || newPercentage > 100.0)
    return Status::failure("Percentage must be between 0 and 100.");

  if (const double currentExcludingThis = allocatedPercentageTotal() - buckets_[index].percentage();
      currentExcludingThis + newPercentage > 100.0 + 1e-9)
    return Status::failure("Edit would push total allocation above 100%.");

  buckets_[index].setName(newName);
  buckets_[index].setPercentage(newPercentage);
  return Status::success("Bucket updated.");
}

Status Account::deleteBucket(const size_t index) {
  if (index >= buckets_.size())
    return Status::failure("Invalid bucket selection.");
  unallocated_ = round2(unallocated_ + buckets_[index].balance());
  const std::string name = buckets_[index].name();
  buckets_.erase(buckets_.begin() + static_cast<long>(index));
  return Status::success("Bucket '" + name + "' deleted; balance returned to unallocated pool.");
}

Status Account::toggleCommitted(const size_t index) {
  if (index >= buckets_.size())
    return Status::failure("Invalid bucket selection.");
  const bool now = !buckets_[index].committed();
  buckets_[index].setCommitted(now);
  return Status::success(std::string("Bucket marked as ") + (now ? "COMMITTED." : "not committed."));
}


// --------- Money operations ---------

Status Account::deposit(double amount) {
  if (!(amount > 0.0))
    return Status::failure("Deposit amount must be positive.");
  amount = round2(amount);
  distributeDeposit(amount);
  totalBalance_ = round2(totalBalance_ + amount);
  return Status::success("Deposited " + fmtMoney(amount) + ".");
}

Status Account::withdrawFromBucket(const size_t index, double amount) {
  if (index >= buckets_.size())
    return Status::failure("Invalid bucket selection.");

  if (!(amount > 0.0))
    return Status::failure("Withdrawal amount must be positive.");

  amount = round2(amount);

  if (amount > totalBalance_ + 1e-9)
    return Status::failure("Withdrawal amount cannot exceed total balance.");

  if (amount > buckets_[index].balance() + 1e-9)
    return Status::failure("Withdrawal amount cannot exceed the selected bucket balance.");

  buckets_[index].adjustBalance(-amount);
  totalBalance_ = round2(totalBalance_ - amount);

  if (buckets_[index].balance() < 0.005)
    buckets_[index].adjustBalance(-buckets_[index].balance());

  if (totalBalance_ < 0.005)
    totalBalance_ = 0.0;

  return Status::success("Withdrawal completed successfully.");
}

void Account::distributeDeposit(const double amount) {
  double allocated = 0.0;
  for (auto& b: buckets_) {
    const double share = round2(amount * b.percentage() / 100.0);
    b.adjustBalance(share);
    allocated += share;
  }
  const double remainder = round2(amount - allocated);
  unallocated_ = round2(unallocated_ + remainder);
}


// --------- Liquidity calculator ---------

double Account::safeToSpend() const { return round2(totalBalance_ - committedTotal()); }

double Account::allocatedPercentageTotal() const {
  double sum = 0.0;
  for (const auto& b: buckets_)
    sum += b.percentage();
  return sum;
}


// --------- Journal & session ---------

void Account::clearSession() { buckets_.clear(); }
