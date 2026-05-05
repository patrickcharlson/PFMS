//
// Created by Patrick Charlson on 5/5/2026.
//

// =====================================================================
//  Funds Withdrawal & Validation
// =====================================================================

#include "Account.h"
#include "test_helpers.h"


static void test_withdraw_basic_within_safe_to_spend() {
  section("Withdraw within Safe to Spend — no warning needed");
  Account a;
  a.createBucket("Rent", 40, true); // committed, $400 after deposit
  a.createBucket("Food", 60); // not committed, $600 after deposit
  a.deposit(1000);
  // Safe to Spend = $600. Withdrawing $300 stays within STS.

  CHECK(a.checkWithdrawal(300) == WithdrawCheck::Ok, "$300 vs STS $600 -> Ok");
  CHECK(a.withdraw(300).ok, "withdraw $300 succeeds");
  CHECK(approx(a.totalBalance(), 700.0), "total = $700 after withdrawal");
}

static void test_withdraw_invalid_amount() {
  section("Withdraw invalid amount rejected");
  Account a;
  a.createBucket("X", 100);
  a.deposit(100);

  CHECK(!a.withdraw(0).ok, "$0 withdraw rejected");
  CHECK(!a.withdraw(-10).ok, "negative withdraw rejected");
  CHECK(approx(a.totalBalance(), 100.0), "balance unchanged after rejected withdraws");
}

static void test_withdraw_exceeds_total_balance_br05() {
  section("Hard reject when amount exceeds total balance");
  Account a;
  a.createBucket("X", 100);
  a.deposit(100);

  CHECK(a.checkWithdrawal(200) == WithdrawCheck::ExceedsBalance, "checkWithdrawal($200 vs $100) -> ExceedsBalance");
  const auto [ok, message] = a.withdraw(200);
  CHECK(!ok, "withdraw($200) rejected");
  CHECK(approx(a.totalBalance(), 100.0), "balance unchanged after rejected over-balance withdrawal");
}

static void test_withdraw_exceeds_safe_to_spend_warning_path() {
  section("ExceedsSafeToSpend triggers warning path");
  Account a;
  a.createBucket("Rent", 40, true); // $400 committed
  a.createBucket("Food", 60); // $600 not committed
  a.deposit(1000);
  // Safe to Spend = $600. Withdrawing $800 exceeds STS but is within total.

  CHECK(a.checkWithdrawal(800) == WithdrawCheck::ExceedsSafeToSpend,
        "checkWithdrawal($800 vs STS $600 / total $1000) -> ExceedsSafeToSpend");
  // The UI layer is responsible for showing the *** WARNING *** prompt and
  // collecting Y/N confirmation. Once confirmed, withdraw() proceeds.
  CHECK(a.withdraw(800).ok, "withdraw($800) succeeds after (simulated) confirmation");
  CHECK(approx(a.totalBalance(), 200.0), "total = $200 after $800 withdrawal");
}

static void test_withdraw_committed_protection_drain_priority() {
  section("Committed buckets protected from withdrawals within STS");
  Account a;
  a.createBucket("Rent", 40, true); // committed
  a.createBucket("Food", 15);
  a.createBucket("Savings", 20);
  a.deposit(1000);
  // Layout: Rent=$400 (committed), Food=$150, Savings=$200, Unalloc=$250
  // Safe to Spend = $600. Withdraw $400 — should NOT touch Rent.

  a.withdraw(400);
  CHECK(approx(a.buckets()[0].balance(), 400.0), "Rent (committed) untouched after within-STS withdrawal");
  CHECK(approx(a.totalBalance(), 600.0), "total dropped by $400");
  CHECK(approx(reconciledTotal(a), a.totalBalance()), "QA-01 invariant holds");
}

static void test_withdraw_atomicity() {
  section("Atomic withdrawal — rejected ops leave no partial state");
  Account a;
  a.createBucket("X", 100);
  a.deposit(100);
  const double balBefore = a.totalBalance();
  const double bucketBefore = a.buckets()[0].balance();

  a.withdraw(200); // exceeds balance — rejected
  a.withdraw(-1); // negative — rejected
  a.withdraw(0); // zero — rejected

  CHECK(approx(a.totalBalance(), balBefore), "totalBalance unchanged after rejections");
  CHECK(approx(a.buckets()[0].balance(), bucketBefore), "bucket unchanged after rejections");
}

static void test_withdraw_zero_balance_account() {
  section("Withdraw from empty account rejected");
  Account a;
  CHECK(a.checkWithdrawal(50) == WithdrawCheck::ExceedsBalance, "any withdraw on $0 account -> ExceedsBalance");
  CHECK(!a.withdraw(50).ok, "withdraw($50) rejected on empty account");
}

void run_withdraw_tests() {
  test_withdraw_basic_within_safe_to_spend();
  test_withdraw_invalid_amount();
  test_withdraw_exceeds_total_balance_br05();
  test_withdraw_exceeds_safe_to_spend_warning_path();
  test_withdraw_committed_protection_drain_priority();
  test_withdraw_atomicity();
  test_withdraw_zero_balance_account();
}
