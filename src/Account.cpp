//
// Created by Patrick Charlson on 21/4/2026.
//

#include "../include/Account.h"

#include <cmath>

inline double round2(const double v) { return std::round(v * 100.0) / 100.0; }

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

double Account::safeToSpend() const { return round2(totalBalance_ - committedTotal()); }

double Account::allocatedPercentageTotal() const {
  double sum = 0.0;
  for (const auto& b: buckets_)
    sum += b.percentage();
  return sum;
}


// --------- Journal & session ---------

void Account::clearSession() { buckets_.clear(); }
