//
// Created by Patrick Charlson on 5/5/2026.
//


// =====================================================================
//  Deposit and Smart-Distribute
// =====================================================================

#include "Account.h"
#include "test_helpers.h"


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

void run_deposit_tests() {
  test_deposit_basic();
  test_deposit_invalid_amount();
  test_smart_distribute_proportional_allocation();
  test_smart_distribute_arithmetic_integrity();
  test_smart_distribute_with_no_buckets();
  test_smart_distribute_multiple_deposits();
}
