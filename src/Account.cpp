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

double Account::safeToSpend() const { return round2(totalBalance_ - committedTotal()); }

double Account::allocatedPercentageTotal() const {
  double sum = 0.0;
  for (const auto& b: buckets_)
    sum += b.percentage();
  return sum;
}


// --------- Journal & session ---------

void Account::clearSession() { buckets_.clear(); }
