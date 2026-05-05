//
// Created by Patrick Charlson on 5/5/2026.
//


// =====================================================================
//  Cross-cutting invariants
// =====================================================================

#include "Account.h"
#include "AuthService.h"
#include "test_helpers.h"


static void test_arithmetic_integrity_across_mixed_ops() {
  section("Arithmetic integrity across mixed operations");
  Account a;
  a.createBucket("Rent", 40, true);
  a.createBucket("Food", 30);
  a.createBucket("Savings", 20);

  a.deposit(1000);
  CHECK(approx(reconciledTotal(a), a.totalBalance()), "after deposit 1");

  a.deposit(500);
  CHECK(approx(reconciledTotal(a), a.totalBalance()), "after deposit 2");

  a.transferFromUnallocated(2, 50);
  CHECK(approx(reconciledTotal(a), a.totalBalance()), "after transfer to Savings");

  a.withdraw(100);
  CHECK(approx(reconciledTotal(a), a.totalBalance()), "after withdrawal of $100");

  a.toggleCommitted(2);
  CHECK(approx(reconciledTotal(a), a.totalBalance()), "after toggle (no balance change)");
}

static void test_session_clear_resets_buckets() {
  section("clearSession() empties bucket state");
  Account a;
  a.createBucket("X", 50);
  a.deposit(500);
  CHECK(a.buckets().size() == 1, "1 bucket pre-clear");

  a.clearSession();
  CHECK(a.buckets().empty(), "buckets cleared");
  // NOTE: documents current implementation behaviour. SEC-04 requires that
  // session data is cleared on logout — check the implementation also clears
  // totalBalance_, unallocated_, and any journal kept on User/PFMS.
}

static void test_e2e_typical_user_flow() {
  section("End-to-end  Typical user workflow");
  AuthService svc;
  svc.registerUser("patrick", "test1234");
  CHECK(svc.login("patrick", "test1234") == LoginOutcome::Success, "login");

  Account& acc = svc.currentUser()->account();
  acc.createBucket("Rent", 40, true); // committed
  acc.createBucket("Food", 15);
  acc.createBucket("Savings", 20);
  acc.deposit(1000);

  // Smart-Distribute layout:
  //   Rent     = $400 (40%, committed)
  //   Food     = $150 (15%)
  //   Savings  = $200 (20%)
  //   Unalloc  = $250 (25% remainder)
  //   Total    = $1000
  //   STS      = $600  (total - committed)
  CHECK(approx(acc.safeToSpend(), 600.0), "STS = $600 (Rent committed)");
  CHECK(approx(acc.buckets()[0].balance(), 400.0), "Rent = $400");
  CHECK(approx(acc.buckets()[1].balance(), 150.0), "Food = $150");
  CHECK(approx(acc.buckets()[2].balance(), 200.0), "Savings = $200");
  CHECK(approx(acc.unallocated(), 250.0), "Unallocated = $250");

  // Transfer unallocated funds into Savings
  acc.transferFromUnallocated(2, 100);
  CHECK(approx(acc.buckets()[2].balance(), 300.0), "Savings = $300 after transfer");
  CHECK(approx(acc.unallocated(), 150.0), "Unallocated = $150 after transfer");

  // Withdraw $50 — well within Safe to Spend ($600)
  // Account drains in priority order: unallocated first, so unalloc drops
  // by $50 and committed Rent stays untouched.
  CHECK(acc.checkWithdrawal(50) == WithdrawCheck::Ok, "$50 within STS — Ok");
  CHECK(acc.withdraw(50).ok, "withdraw $50 succeeds");
  CHECK(approx(acc.totalBalance(), 950.0), "Total = $950 after $50 withdrawal");
  CHECK(approx(acc.buckets()[0].balance(), 400.0), "Rent (committed) untouched after within-STS withdrawal");

  // Try a withdrawal that exceeds total balance — hard reject
  CHECK(acc.checkWithdrawal(2000) == WithdrawCheck::ExceedsBalance, "$2000 vs total $950 -> ExceedsBalance");
  CHECK(!acc.withdraw(2000).ok, "BR-05 rejection: withdraw $2000 refused");
  CHECK(approx(acc.totalBalance(), 950.0), "Total unchanged after rejected over-balance withdraw");

  // Try a withdrawal that exceeds STS but is within total — warning path.
  // STS is now $550 (total $950 - committed $400). $700 should trigger the
  // ExceedsSafeToSpend warning case the UI uses to prompt for confirmation.
  CHECK(acc.checkWithdrawal(700) == WithdrawCheck::ExceedsSafeToSpend,
        "$700 vs STS $550 / total $950 -> ExceedsSafeToSpend (warning path)");

  // Invariant must still hold across the whole workflow
  CHECK(approx(reconciledTotal(acc), acc.totalBalance()), "QA-01 invariant after full workflow");

  // Logout clears session state
  svc.logout();
  CHECK(svc.currentUser() == nullptr, "currentUser nulled after logout");
}

static void test_e2e_lockout_then_restart() {
  section("End-to-end  Lockout simulation");
  AuthService svc;
  svc.registerUser("alice", "secret123");

  for (int i = 0; i < 3; ++i)
    svc.login("alice", "wrong");
  CHECK(svc.isLocked(), "service locks after 3 failures");
  CHECK(svc.login("alice", "secret123") == LoginOutcome::Locked, "even correct password blocked once locked");

  // "Restart required to retry." A new AuthService instance
  // simulates a process restart.
  AuthService freshSvc;
  freshSvc.registerUser("alice", "secret123");
  CHECK(freshSvc.login("alice", "secret123") == LoginOutcome::Success, "fresh service allows login");
  CHECK(!freshSvc.isLocked(), "fresh service is not locked");
  CHECK(freshSvc.failedAttempts() == 0, "failure counter starts at 0");
}

static void attack_deposit_infinity() {
  section("Infinity / NaN deposit rejected or handled");
  Account a;
  // SRS doesn't say what to do here — but we must not corrupt state.
  constexpr double inf = std::numeric_limits<double>::infinity();
  constexpr double nan = std::numeric_limits<double>::quiet_NaN();
  const double balBefore = a.totalBalance();

  a.deposit(inf);
  a.deposit(nan);
  CHECK(!std::isinf(a.totalBalance()), "totalBalance is finite after inf deposit");
  CHECK(!std::isnan(a.totalBalance()), "totalBalance is not NaN after nan deposit");
  CHECK(approx(reconciledTotal(a), a.totalBalance()), "invariant holds even after weird inputs");
  // Note: silently absorbing inf/nan is acceptable per SRS as long as
  // state stays consistent. Loud rejection would be cleaner but not required.
  (void) balBefore;
}

static void test_whitespace_variants_rejected() {
  section("Whitespace variants of the same name treated as duplicates");
  // "Rent " (trailing space) shouldn't sneak past the check — that's a typo,
  // not a different bucket.
  Account a;
  a.createBucket("Rent", 30);
  CHECK(!a.createBucket("Rent ", 20).ok, "'Rent ' (trailing space) rejected");
  CHECK(!a.createBucket(" Rent", 20).ok, "' Rent' (leading space) rejected");
  CHECK(!a.createBucket("Rent\t", 20).ok, "'Rent\\t' (trailing tab) rejected");
}

void run_safety_tests() {
  test_arithmetic_integrity_across_mixed_ops();
  test_session_clear_resets_buckets();
  test_e2e_typical_user_flow();
  test_e2e_lockout_then_restart();
  attack_deposit_infinity();
  test_whitespace_variants_rejected();
}
