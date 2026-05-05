//
// Created by Patrick Charlson on 5/5/2026.
//


// =====================================================================
//  test Suite
// =====================================================================

#include "test_helpers.h"
#include <iostream>

int main() {
  std::cout << "===========================================================\n";
  std::cout << " PFMS v1.0 — Test Suite\n";
  std::cout << "===========================================================\n";

  run_auth_tests();
  run_bucket_tests();
  run_deposit_tests();
  run_transfer_tests();
  run_withdraw_tests();
  run_liquidity_tests();
  run_journal_tests();
  run_crypto_tests();
  run_safety_tests();

  std::cout << "\n===========================================================\n";
  std::cout << " RESULTS: " << passed << " passed, " << failed << " failed\n";
  std::cout << "===========================================================\n";
  return failed == 0 ? 0 : 1;
}
