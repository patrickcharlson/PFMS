//
// Created by Patrick Charlson on 5/5/2026.
//
#include "test_helpers.h"
#include "../include/Account.h"

int passed = 0;
int failed = 0;


double reconciledTotal(const Account& a) {
  double sum = a.unallocated();
  for (const auto& b: a.buckets())
    sum += b.balance();
  return sum;
}