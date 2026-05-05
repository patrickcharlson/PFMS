//
// Created by Patrick Charlson on 5/5/2026.
//

#ifndef PFMS_TEST_HELPERS_H
#define PFMS_TEST_HELPERS_H

#include <cmath>
#include <iostream>

extern int passed;
extern int failed;

#define CHECK(cond, label)                                                                                             \
  do {                                                                                                                 \
    if (cond) {                                                                                                        \
      ++passed;                                                                                                        \
      std::cout << "  PASS  " << label << "\n";                                                                        \
    } else {                                                                                                           \
      ++failed;                                                                                                        \
      std::cout << "  FAIL  " << label << "\n";                                                                        \
    }                                                                                                                  \
  } while (0)

static bool approx(const double a, const double b, const double eps = 1e-6) { return std::fabs(a - b) < eps; }

static void section(const std::string& name) { std::cout << "\n[" << name << "]\n"; }

// Helper used across multiple test files: sum of bucket balances + unallocated
// must equal totalBalance.
class Account;
double reconciledTotal(const Account& a);

void run_auth_tests();
void run_bucket_tests();
void run_deposit_tests();
void run_transfer_tests();
void run_withdraw_tests();
void run_liquidity_tests();
void run_journal_tests();
void run_crypto_tests();
void run_safety_tests();

#endif // PFMS_TEST_HELPERS_H
