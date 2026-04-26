//
// Created by Patrick Charlson on 21/4/2026.
//

#include "../include/Account.h"

#include <cmath>

inline double round2(double v) { return std::round(v * 100.0) / 100.0; }

double Account::committedTotal() const {
  double sum = 0.0;
  for (const auto& b: buckets_)
    if (b.committed())
      sum += b.balance();
  return round2(sum);
}

double Account::safeToSpend() const { return round2(totalBalance_ - committedTotal()); }


// --------- Journal & session ---------

void Account::clearSession() { buckets_.clear(); }
