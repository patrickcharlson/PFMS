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

Account::WithdrawCheck Account::checkWithdrawal(const double amount) const {
  if (amount > totalBalance_ + 1e-9)
    return WithdrawCheck::ExceedsBalance;
  if (amount > safeToSpend() + 1e-9)
    return WithdrawCheck::ExceedsSafeToSpend;
  return WithdrawCheck::Ok;
}

Status Account::withdraw(double amount) {
  if (!(amount > 0.0))
    return Status::failure("Withdraw amount must be positive.");
  amount = round2(amount);
  if (amount > totalBalance_ + 1e-9)
    return Status::failure("Amount exceeds available balance. Please enter a value up to " + fmtMoney(totalBalance_) +
                           ".");
  double remaining = amount;

  auto drainFrom = [&](const bool committed) {
    if (remaining <= 0.0)
      return;
    double poolTotal = 0.0;
    std::vector<size_t> indices;
    for (size_t i = 0; i < buckets_.size(); ++i) {
      if (buckets_[i].committed() == committed) {
        poolTotal += buckets_[i].balance();
        indices.push_back(i);
      }
    }
    if (poolTotal <= 0.0 || indices.empty())
      return;

    const double take = std::min(remaining, poolTotal);
    double drawn = 0.0;
    for (size_t k = 0; k < indices.size(); ++k) {
      const size_t i = indices[k];
      const bool last = k + 1 == indices.size();
      double share = last ? round2(take - drawn) : round2(take * (buckets_[i].balance() / poolTotal));
      if (share > buckets_[i].balance())
        share = buckets_[i].balance();
      buckets_[i].adjustBalance(-share);
      drawn = round2(drawn + share);
    }
    remaining = round2(remaining - take);
  };

  const double fromUnalloc = std::min(unallocated_, remaining);
  unallocated_ = round2(unallocated_ - fromUnalloc);
  remaining = round2(remaining - fromUnalloc);

  drainFrom(/*committed =*/false);
  drainFrom(/*committed =*/true);

  totalBalance_ = round2(totalBalance_ - amount);
  return Status::success("Withdrew " + fmtMoney(amount) + ".");
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
