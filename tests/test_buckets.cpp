//
// Created by Patrick Charlson on 5/5/2026.
//


// =====================================================================
//  Virtual Bucket Management
// =====================================================================


#include "Account.h"
#include "test_helpers.h"

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

void run_bucket_tests() {
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
}
