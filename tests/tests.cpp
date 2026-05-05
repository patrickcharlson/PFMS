//
// Created by Patrick Charlson on 25/4/2026.
//

#include "../include/AuthService.h"
#include "Sha256.h"

#include <cmath>
#include <iostream>
#include <string>

static int passed{0};
static int failed{0};

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

static double reconciledTotal(const Account& a) {
  double sum = a.unallocated();
  for (const auto& b: a.buckets())
    sum += b.balance();
  return sum;
}


// =====================================================================
//  User Authentication
// =====================================================================

static void test_authentication_register_and_duplicate() {
  section("Register & duplicate username rejection");
  AuthService svc;
  CHECK(svc.registerUser("alice", "secret123").ok, "register new user 'alice' succeeds");
  CHECK(!svc.registerUser("alice", "differentpw").ok, "duplicate 'alice' rejected");
  CHECK(svc.registerUser("bob", "anotherpw").ok, "different username 'bob' accepted");
}

static void test_authentication_empty_inputs() {
  section("Empty input validation");
  AuthService svc;
  CHECK(!svc.registerUser("", "validpw").ok, "empty username rejected");
  CHECK(!svc.registerUser("user", "abc").ok, "password under 4 chars rejected");
  CHECK(svc.registerUser("user", "abcd").ok, "exactly 4-char password accepted");
}

static void test_authentication_login_success_failure() {
  section("Login success & invalid credentials");
  AuthService svc;
  svc.registerUser("alice", "secret123");

  CHECK(svc.login("alice", "secret123") == LoginOutcome::Success, "correct credentials succeed");
  svc.logout();
  CHECK(svc.login("alice", "wrongpw") == LoginOutcome::BadCredentials, "wrong password rejected");
  CHECK(svc.login("nobody", "secret123") == LoginOutcome::BadCredentials, "unknown username rejected");
}

static void test_authentication_lockout_after_three_failures() {
  section("Lockout after exactly 3 failed attempts");
  AuthService svc;
  svc.registerUser("alice", "secret123");

  CHECK(svc.login("alice", "wrong1") == LoginOutcome::BadCredentials, "fail 1 -> BadCredentials");
  CHECK(svc.login("alice", "wrong2") == LoginOutcome::BadCredentials, "fail 2 -> BadCredentials");
  CHECK(svc.login("alice", "wrong3") == LoginOutcome::Locked, "fail 3 -> Locked");
  CHECK(svc.isLocked(), "isLocked() returns true after 3 failures");
  CHECK(svc.failedAttempts() == 3, "failedAttempts counter = 3");
  CHECK(svc.login("alice", "secret123") == LoginOutcome::Locked, "even correct pw blocked once locked");
}

static void test_authentication_failure_counter_resets_on_success() {
  section("Failure counter resets on successful login");
  AuthService svc;
  svc.registerUser("alice", "secret123");

  svc.login("alice", "wrong");
  svc.login("alice", "wrong");
  CHECK(svc.failedAttempts() == 2, "2 failures recorded");
  svc.login("alice", "secret123");
  CHECK(svc.failedAttempts() == 0, "counter resets to 0 after successful login");
}

static void test_authentication_logout() {
  section("Logout");
  AuthService svc;
  svc.registerUser("alice", "secret123");
  svc.login("alice", "secret123");
  CHECK(svc.currentUser() != nullptr, "currentUser set after login");

  svc.logout();
  CHECK(svc.currentUser() == nullptr, "currentUser nulled after logout");
}

static void test_registration_username_case_insensitive() {
  section("§4.1 REQ-1  Username uniqueness is case-insensitive");
  AuthService svc;
  CHECK(svc.registerUser("alice", "secret123").ok, "register 'alice' succeeds");
  CHECK(!svc.registerUser("Alice", "different456").ok, "'Alice' rejected as duplicate");
  CHECK(!svc.registerUser("ALICE", "another789").ok, "'ALICE' rejected as duplicate");
  CHECK(!svc.registerUser("AlIcE", "more012").ok,    "'AlIcE' rejected as duplicate");
}

static void test_login_username_case_insensitive() {
  section("§4.1 REQ-3  Login accepts any case of registered username");
  AuthService svc;
  svc.registerUser("alice", "secret123");
  CHECK(svc.login("alice", "secret123") == LoginOutcome::Success, "login as 'alice' works");
  svc.logout();
  CHECK(svc.login("Alice", "secret123") == LoginOutcome::Success, "login as 'Alice' works");
  svc.logout();
  CHECK(svc.login("ALICE", "secret123") == LoginOutcome::Success, "login as 'ALICE' works");
}


// =====================================================================
//  Virtual Bucket Management
// =====================================================================

static void test_bucket_create_basic() {
  section("Create bucket — basic");
  Account a;
  CHECK(a.createBucket("Rent", 40).ok, "create Rent at 40%");
  CHECK(a.buckets().size() == 1, "bucket count = 1");
  CHECK(a.buckets()[0].name() == "Rent", "name preserved");
  CHECK(approx(a.buckets()[0].percentage(), 40.0), "percentage preserved");
  CHECK(!a.buckets()[0].committed(), "default committed = false");
}

static void test_bucket_create_with_committed_flag() {
  section("Create bucket — committed flag");
  Account a;
  a.createBucket("Rent", 40, true);
  CHECK(a.buckets()[0].committed(), "committed bucket created with committed=true");
}

static void test_bucket_create_invalid_inputs() {
  section("Create bucket — invalid inputs rejected");
  Account a;
  CHECK(!a.createBucket("", 50).ok, "empty name rejected");
  CHECK(!a.createBucket("X", -1).ok, "negative percentage rejected");
  CHECK(!a.createBucket("X", 101).ok, ">100% percentage rejected");
  CHECK(a.createBucket("X", 0).ok, "0% accepted (placeholder bucket)");
  CHECK(a.createBucket("Y", 100).ok, "exactly 100% accepted");
}

static void test_bucket_100_percent_cap() {
  section("100% allocation cap");
  Account a;
  CHECK(a.createBucket("A", 40).ok, "A at 40% accepted");
  CHECK(a.createBucket("B", 30).ok, "B at 30% (total 70%) accepted");
  CHECK(a.createBucket("C", 30).ok, "C at 30% (total 100%) accepted");
  CHECK(!a.createBucket("D", 1).ok, "D at 1% would exceed 100% — rejected");
  CHECK(approx(a.allocatedPercentageTotal(), 100.0), "total allocation locked at 100%");
}

static void test_bucket_edit() {
  section("Edit bucket");
  Account a;
  a.createBucket("Rent", 40);
  a.createBucket("Food", 30);

  CHECK(a.editBucket(0, "Mortgage", 50).ok, "edit Rent -> Mortgage at 50%");
  CHECK(a.buckets()[0].name() == "Mortgage", "name updated");
  CHECK(approx(a.buckets()[0].percentage(), 50.0), "percentage updated");
  CHECK(!a.editBucket(99, "X", 10).ok, "invalid index rejected");
  CHECK(!a.editBucket(0, "", 50).ok, "empty new name rejected");
  CHECK(!a.editBucket(0, "M", -10).ok, "negative new percentage rejected");
}

static void test_bucket_edit_revalidates_100_percent() {
  section("Edit re-validates 100% cap");
  Account a;
  a.createBucket("A", 40);
  a.createBucket("B", 60); // total = 100%

  CHECK(!a.editBucket(0, "A", 50).ok, "edit A 40->50 would push total to 110% — rejected");
  CHECK(approx(a.buckets()[0].percentage(), 40.0), "A percentage unchanged after rejected edit");
  CHECK(a.editBucket(0, "A", 30).ok, "edit A 40->30 (total drops to 90%) accepted");
}

static void test_bucket_delete_returns_balance() {
  section("Delete returns balance to unallocated pool");
  Account a;
  a.createBucket("Rent", 50);
  a.deposit(200);
  const double rentBefore = a.buckets()[0].balance();
  const double unallocBefore = a.unallocated();

  CHECK(a.deleteBucket(0).ok, "delete Rent");
  CHECK(a.buckets().empty(), "bucket count = 0 after delete");
  CHECK(approx(a.unallocated(), unallocBefore + rentBefore), "deleted balance returned to unallocated");
  CHECK(approx(a.totalBalance(), 200.0), "total balance unchanged");
}

static void test_bucket_delete_invalid_index() {
  section("Delete invalid index");
  Account a;
  CHECK(!a.deleteBucket(0).ok, "delete from empty account rejected");
  a.createBucket("X", 10);
  CHECK(!a.deleteBucket(5).ok, "delete out-of-range index rejected");
}

static void test_bucket_toggle_committed() {
  section("Toggle committed status");
  Account a;
  a.createBucket("Rent", 40);
  CHECK(!a.buckets()[0].committed(), "initially not committed");

  CHECK(a.toggleCommitted(0).ok, "toggle 1 succeeds");
  CHECK(a.buckets()[0].committed(), "now committed");

  CHECK(a.toggleCommitted(0).ok, "toggle 2 succeeds");
  CHECK(!a.buckets()[0].committed(), "back to not committed");

  CHECK(!a.toggleCommitted(99).ok, "invalid index rejected");
}

static void test_case_insensitive_duplicate_rejected() {
  section("Duplicate detection is case-insensitive");
  Account a;
  CHECK(a.createBucket("Rent", 30).ok, "create 'Rent'");

  CHECK(!a.createBucket("rent", 20).ok, "'rent' (lowercase) rejected as duplicate of 'Rent'");
  CHECK(!a.createBucket("RENT", 20).ok, "'RENT' (uppercase) rejected as duplicate");
  CHECK(!a.createBucket("ReNt", 20).ok, "'ReNt' (mixed case) rejected as duplicate");
  CHECK(a.buckets().size() == 1, "still only 1 bucket");
}


// =====================================================================
//  Deposit and Smart-Distribute
// =====================================================================

static void test_deposit_basic() {
  section("Deposit basic");
  Account a;
  CHECK(a.deposit(100).ok, "deposit $100");
  CHECK(approx(a.totalBalance(), 100.0), "total = $100");
  CHECK(approx(a.unallocated(), 100.0), "all goes to unallocated when no buckets");
}

static void test_deposit_invalid_amount() {
  section("Deposit invalid amount rejected");
  Account a;
  CHECK(!a.deposit(0).ok, "deposit $0 rejected");
  CHECK(!a.deposit(-50).ok, "deposit -$50 rejected");
  CHECK(approx(a.totalBalance(), 0.0), "balance unchanged after rejected deposits");
}

static void test_smart_distribute_proportional_allocation() {
  section("Smart-Distribute proportional allocation");
  Account a;
  a.createBucket("Rent", 40, true);
  a.createBucket("Food", 15);
  a.createBucket("Savings", 20);
  a.deposit(1000);

  CHECK(approx(a.buckets()[0].balance(), 400.0), "Rent (40%) gets $400");
  CHECK(approx(a.buckets()[1].balance(), 150.0), "Food (15%) gets $150");
  CHECK(approx(a.buckets()[2].balance(), 200.0), "Savings (20%) gets $200");
  CHECK(approx(a.unallocated(), 250.0), "Unallocated (25% remainder) gets $250");
}

static void test_smart_distribute_arithmetic_integrity() {
  section("Arithmetic integrity after deposit");
  Account a;
  a.createBucket("A", 33);
  a.createBucket("B", 33);
  a.createBucket("C", 33);
  a.deposit(100); // Awkward 33% × $100 = rounding-prone

  CHECK(approx(reconciledTotal(a), a.totalBalance()), "sum(buckets) + unalloc == totalBalance");
  CHECK(approx(a.totalBalance(), 100.0), "total balance is exactly $100");
}

static void test_smart_distribute_with_no_buckets() {
  section("Deposit with no buckets — all unallocated");
  Account a;
  a.deposit(500);
  CHECK(approx(a.unallocated(), 500.0), "all $500 goes to unallocated");
  CHECK(approx(a.totalBalance(), 500.0), "total balance correct");
}

static void test_smart_distribute_multiple_deposits() {
  section("Multiple deposits accumulate correctly");
  Account a;
  a.createBucket("X", 50);
  a.deposit(100);
  a.deposit(200);
  a.deposit(300);
  CHECK(approx(a.totalBalance(), 600.0), "totalBalance = 100+200+300 = $600");
  CHECK(approx(a.buckets()[0].balance(), 300.0), "X gets 50% of $600 = $300");
  CHECK(approx(a.unallocated(), 300.0), "remainder $300 in unallocated");
  CHECK(approx(reconciledTotal(a), a.totalBalance()), "QA-01 invariant holds across deposits");
}


// =====================================================================
//  Manual Transfer
// =====================================================================

static void test_transfer_basic() {
  section("Manual transfer from unallocated");
  Account a;
  a.createBucket("Goal", 0); // 0% so deposit goes to unallocated
  a.deposit(500);
  CHECK(approx(a.unallocated(), 500.0), "all in unallocated");

  CHECK(a.transferFromUnallocated(0, 200).ok, "transfer $200 to Goal");
  CHECK(approx(a.buckets()[0].balance(), 200.0), "Goal balance = $200");
  CHECK(approx(a.unallocated(), 300.0), "Unallocated = $300");
  CHECK(approx(a.totalBalance(), 500.0), "Total balance unchanged");
}

static void test_transfer_invalid() {
  section("Transfer rejection cases");
  Account a;
  a.createBucket("Goal", 0);
  a.deposit(100);

  CHECK(!a.transferFromUnallocated(0, 0).ok, "$0 transfer rejected");
  CHECK(!a.transferFromUnallocated(0, -50).ok, "negative transfer rejected");
  CHECK(!a.transferFromUnallocated(0, 200).ok, "transfer > unallocated rejected");
  CHECK(!a.transferFromUnallocated(99, 50).ok, "invalid bucket index rejected");
  CHECK(approx(a.unallocated(), 100.0), "unallocated unchanged after all rejections");
}

// =====================================================================
//  Liquidity Calculator (Safe to Spend)
// =====================================================================

static void test_safe_to_spend_basic() {
  section("Safe to Spend basic computation");
  Account a;
  a.createBucket("Rent", 40, true); // committed
  a.createBucket("Food", 60); // not committed
  a.deposit(1000);

  CHECK(approx(a.committedTotal(), 400.0), "Committed total = $400");
  CHECK(approx(a.safeToSpend(), 600.0), "Safe to Spend = $1000 - $400 = $600");
}

static void test_safe_to_spend_zero_when_all_committed() {
  section("Safe to Spend = 0 when all funds committed");
  Account a;
  a.createBucket("A", 50, true);
  a.createBucket("B", 50, true);
  a.deposit(1000);
  CHECK(approx(a.safeToSpend(), 0.0), "all committed -> Safe to Spend = $0");
}

static void test_safe_to_spend_recalculates_on_toggle() {
  section("Safe to Spend recalculates on toggle");
  Account a;
  a.createBucket("Rent", 40, true);
  a.createBucket("Food", 60);
  a.deposit(1000);
  CHECK(approx(a.safeToSpend(), 600.0), "STS = $600 (Rent committed)");

  a.toggleCommitted(0);
  CHECK(approx(a.safeToSpend(), 1000.0), "STS jumps to $1000 after un-committing");

  a.toggleCommitted(1);
  CHECK(approx(a.safeToSpend(), 400.0), "STS = $400 after committing Food");
}

static void test_safe_to_spend_no_buckets() {
  section("Safe to Spend with no buckets");
  Account a;
  a.deposit(500);
  CHECK(approx(a.safeToSpend(), 500.0), "STS = total when no committed funds");
  CHECK(approx(a.committedTotal(), 0.0), "committed total = 0");
}

// =====================================================================
//  Funds Withdrawal & Validation
// =====================================================================

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


// =====================================================================
//  Cross-cutting invariants
// =====================================================================

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


// =====================================================================
//  SHA-256
// =====================================================================

static void test_sha256_reference_vector() {
  section("SHA-256 against FIPS 180-4 reference");
  // Known SHA-256 of "abc" per FIPS 180-4
  const std::string expected = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";
  CHECK(Sha256::hash("abc") == expected, "SHA-256('abc') matches reference");

  // Empty string vector
  const std::string emptyExpected = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
  CHECK(Sha256::hash("") == emptyExpected, "SHA-256('') matches reference");
}

static void test_sha256_determinism_and_distinctness() {
  section("SHA-256 deterministic & distinct");
  CHECK(Sha256::hash("password") == Sha256::hash("password"), "same input -> same hash");
  CHECK(Sha256::hash("password") != Sha256::hash("Password"), "case-sensitive: 'P' differs from 'p'");
  CHECK(Sha256::hash("password ") != Sha256::hash("password"), "trailing space changes hash");
  CHECK(Sha256::hash("a").size() == 64, "hash is 64 hex characters");
}

static void test_password_never_stored_plaintext() {
  section("Plain-text passwords never stored");
  AuthService svc;
  const std::string plainPw = "MySecret2026";
  svc.registerUser("alice", plainPw);
  svc.login("alice", plainPw);

  // The User class only exposes passwordHash() — there is no getter for plain.
  // We verify the stored hash equals SHA-256(plain), not the plain itself.
  const std::string storedHash = svc.currentUser()->passwordHash();
  CHECK(storedHash != plainPw, "stored value is NOT the plain password");
  CHECK(storedHash == Sha256::hash(plainPw), "stored value IS the SHA-256 hash");
  CHECK(storedHash.size() == 64, "stored hash is 64 chars (hex)");
}


// =====================================================================
//  Transaction Journal
// =====================================================================

static void test_journal_starts_empty() {
  section("Journal starts empty");
  const Account a;
  CHECK(a.journal().empty(), "journal is empty on fresh Account");
}

static void test_journal_logs_deposit() {
  section("Deposit creates a journal entry");
  Account a;
  a.deposit(150.00);
  CHECK(a.journal().size() == 1, "journal has exactly 1 entry after deposit");

  const auto& tx = a.journal().front();
  CHECK(tx.type() == TxType::Deposit, "entry type is Deposit");
  CHECK(approx(tx.amount(), 150.00), "entry amount = $150.00");
  CHECK(!tx.description().empty(), "entry has a non-empty description");
  CHECK(!tx.formattedTimestamp().empty(), "entry has a timestamp");
}

static void test_journal_logs_withdrawal() {
  section("Withdrawal creates a journal entry");
  Account a;
  a.createBucket("X", 100);
  a.deposit(200);
  a.withdraw(50);

  CHECK(a.journal().size() == 2, "journal has 2 entries (deposit + withdraw)");
  const auto& tx = a.journal().back();
  CHECK(tx.type() == TxType::Withdrawal, "second entry type is Withdrawal");
  CHECK(approx(tx.amount(), 50.00), "second entry amount = $50.00");
}

static void test_journal_logs_transfer() {
  section("Transfer creates a journal entry");
  Account a;
  a.createBucket("Goal", 0);
  a.deposit(500); // entry 1
  a.transferFromUnallocated(0, 100); // entry 2

  CHECK(a.journal().size() == 2, "journal has 2 entries");
  const auto& tx = a.journal().back();
  CHECK(tx.type() == TxType::Transfer, "second entry type is Transfer");
  CHECK(approx(tx.amount(), 100.00), "second entry amount = $100.00");
  CHECK(tx.description().find("Goal") != std::string::npos, "transfer description references destination bucket");
}

static void test_journal_rejected_ops_not_logged() {
  section("Rejected operations don't appear in journal");
  Account a;
  a.createBucket("X", 100);
  a.deposit(100);
  CHECK(a.journal().size() == 1, "1 entry after successful deposit");

  a.deposit(0); // rejected — non-positive
  a.deposit(-50); // rejected — negative
  a.withdraw(500); // rejected — exceeds balance
  a.withdraw(-1); // rejected — non-positive
  CHECK(a.journal().size() == 1, "still 1 entry — rejected ops not logged");
}

static void test_journal_records_chronological_order() {
  section("Journal preserves chronological order");
  Account a;
  a.createBucket("X", 100);
  a.deposit(100); // [0] deposit
  a.deposit(50); // [1] deposit
  a.withdraw(30); // [2] withdrawal

  CHECK(a.journal().size() == 3, "3 entries logged");
  CHECK(a.journal()[0].type() == TxType::Deposit, "[0] = Deposit");
  CHECK(a.journal()[1].type() == TxType::Deposit, "[1] = Deposit");
  CHECK(a.journal()[2].type() == TxType::Withdrawal, "[2] = Withdrawal");
  CHECK(approx(a.journal()[0].amount(), 100.0), "[0] amount = $100");
  CHECK(approx(a.journal()[1].amount(), 50.0), "[1] amount = $50");
  CHECK(approx(a.journal()[2].amount(), 30.0), "[2] amount = $30");
}

static void test_journal_type_labels() {
  section("Transaction  typeLabel format");
  Account a;
  a.createBucket("X", 100);
  a.deposit(100);
  a.withdraw(20);

  CHECK(a.journal()[0].typeLabel() == "DEPOSIT", "deposit -> 'DEPOSIT'");
  CHECK(a.journal()[1].typeLabel() == "WITHDRAWAL", "withdrawal -> 'WITHDRAWAL'");
}

static void test_journal_cleared_on_session_clear() {
  section("Journal cleared on clearSession()");
  Account a;
  a.createBucket("X", 100);
  a.deposit(100);
  a.withdraw(20);
  CHECK(a.journal().size() == 2, "2 entries before clear");

  a.clearSession();
  CHECK(a.journal().empty(), "journal empty after clearSession");
}

// =====================================================================
//  End-to-end scenarios
// =====================================================================

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


// =====================================================================
//  ACCOUNT MATHEMATICS — TRY TO BREAK ARITHMETIC INTEGRITY
// =====================================================================

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


// =====================================================================
//  Misc
// =====================================================================

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

int main() {

  // User Authentication
  test_authentication_register_and_duplicate();
  test_authentication_empty_inputs();
  test_authentication_login_success_failure();
  test_authentication_lockout_after_three_failures();
  test_authentication_failure_counter_resets_on_success();
  test_authentication_logout();
  test_registration_username_case_insensitive();
  test_login_username_case_insensitive();

  // Virtual Bucket Management
  test_bucket_create_basic();
  test_bucket_create_with_committed_flag();
  test_bucket_create_invalid_inputs();
  test_bucket_100_percent_cap();
  test_bucket_edit();
  test_bucket_edit_revalidates_100_percent();
  test_bucket_delete_returns_balance();
  test_bucket_delete_invalid_index();
  test_bucket_toggle_committed();
  test_case_insensitive_duplicate_rejected();

  // Deposit and Smart-Distribute
  test_deposit_basic();
  test_deposit_invalid_amount();
  test_smart_distribute_proportional_allocation();
  test_smart_distribute_arithmetic_integrity();
  test_smart_distribute_with_no_buckets();
  test_smart_distribute_multiple_deposits();

  // Manual Transfer
  test_transfer_basic();
  test_transfer_invalid();

  // Liquidity Calculator (Safe to Spend)
  test_safe_to_spend_basic();
  test_safe_to_spend_zero_when_all_committed();
  test_safe_to_spend_recalculates_on_toggle();
  test_safe_to_spend_no_buckets();

  //  Funds Withdrawal & Validation
  test_withdraw_basic_within_safe_to_spend();
  test_withdraw_invalid_amount();
  test_withdraw_exceeds_total_balance_br05();
  test_withdraw_exceeds_safe_to_spend_warning_path();
  test_withdraw_committed_protection_drain_priority();
  test_withdraw_atomicity();
  test_withdraw_zero_balance_account();

  // Cross-cutting
  test_arithmetic_integrity_across_mixed_ops();
  test_session_clear_resets_buckets();

  // Crypto
  test_sha256_reference_vector();
  test_sha256_determinism_and_distinctness();
  test_password_never_stored_plaintext();

  // Transactions / Journal
  test_journal_starts_empty();
  test_journal_logs_deposit();
  test_journal_logs_withdrawal();
  test_journal_logs_transfer();
  test_journal_rejected_ops_not_logged();
  test_journal_records_chronological_order();
  test_journal_type_labels();
  test_journal_cleared_on_session_clear();

  // End-to-end
  test_e2e_typical_user_flow();
  test_e2e_lockout_then_restart();

  // Hostile tests
  attack_deposit_infinity();

  // Misc
  test_whitespace_variants_rejected();

  std::cout << "\n========================================\n";
  std::cout << " RESULTS: " << passed << " passed, " << failed << " failed\n";
  std::cout << "========================================\n";
  return failed == 0 ? 0 : 1;
}
