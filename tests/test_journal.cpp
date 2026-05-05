//
// Created by Patrick Charlson on 5/5/2026.
//


// =====================================================================
//  Transaction Journal
// =====================================================================

#include "Account.h"
#include "test_helpers.h"


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

void run_journal_tests() {
  test_journal_starts_empty();
  test_journal_logs_deposit();
  test_journal_logs_withdrawal();
  test_journal_logs_transfer();
  test_journal_rejected_ops_not_logged();
  test_journal_records_chronological_order();
  test_journal_type_labels();
  test_journal_cleared_on_session_clear();
}
