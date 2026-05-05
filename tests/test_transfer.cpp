//
// Created by Patrick Charlson on 5/5/2026.
//

// =====================================================================
//  Manual Transfer
// =====================================================================

#include "Account.h"
#include "test_helpers.h"


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

void run_transfer_tests() {
  test_transfer_basic();
  test_transfer_invalid();
}
