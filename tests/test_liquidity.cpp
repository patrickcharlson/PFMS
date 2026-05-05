//
// Created by Patrick Charlson on 5/5/2026.
//

// =====================================================================
//  Liquidity Calculator (Safe to Spend)
// =====================================================================

#include "Account.h"
#include "test_helpers.h"


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

void run_liquidity_tests() {
  test_safe_to_spend_basic();
  test_safe_to_spend_zero_when_all_committed();
  test_safe_to_spend_recalculates_on_toggle();
  test_safe_to_spend_no_buckets();
}
