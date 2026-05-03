//
// Created by Patrick Charlson on 25/4/2026.
//

#include "../include/AuthService.h"


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


static bool approx(const double a, double b, double eps = 1e-6) { return std::fabs(a - b) < eps; }

static void section(const std::string& name) { std::cout << "\n[" << name << "]\n"; }

int main() {

  // =========================================================================
  // Account :: createBucket
  // =========================================================================
  section("Account :: createBucket happy path");
  {
    Account a;
    const auto [ok, message] = a.createBucket("Rent", 40.0, true);
    CHECK(ok, "valid bucket returns ok");
    CHECK(a.buckets().size() == 1, "buckets size == 1");
    CHECK(a.buckets()[0].name() == "Rent", "buckets stored");
    CHECK(approx(a.buckets()[0].percentage(), 40.0), "bucket percentage stored");
    CHECK(a.buckets()[0].committed() == true, "bucket committed flag stored");
  }

  section("Account :: createBucket rejects empty name");
  {
    Account a;
    auto [ok, message] = a.createBucket("", 10.0);
    CHECK(!ok, "empty name rejected");
    CHECK(a.buckets().empty(), "no bucket added on failure");
  }

  // ---- Lockout after 3 failed attempts ----
  section("Lockout after 3 failed attempts");
  {
    AuthService svc;
    svc.registerUser("bob", "rightpass");
    CHECK(svc.login("bob", "wrong1") == LoginOutcome::BadCredentials, "fail 1");
    CHECK(svc.login("bob", "wrong2") == LoginOutcome::BadCredentials, "fail 2");
    CHECK(svc.login("bob", "wrong3") == LoginOutcome::Locked, "fail 3 -> locked");
    CHECK(svc.isLocked(), "isLocked() == true");
    CHECK(svc.login("bob", "rightpass") == LoginOutcome::Locked, "even correct pw blocked");
  }
  
  // =========================================================================
  // Account :: withdrawFromBucket
  // =========================================================================

  section("Account :: withdrawFromBucket valid withdrawal");
  {
    Account a;

    auto [bucketOk, bucketMsg] = a.createBucket("Food", 100.0, false);
    CHECK(bucketOk, "Food bucket created");

    auto [depositOk, depositMsg] = a.deposit(100.00);
    CHECK(depositOk, "deposit 100.00 succeeds");

    auto [withdrawOk, withdrawMsg] = a.withdrawFromBucket(0, 50.00);
    CHECK(withdrawOk, "withdraw 50.00 succeeds");

    CHECK(approx(a.totalBalance(), 50.00), "total balance reduced to 50.00");
    CHECK(approx(a.buckets()[0].balance(), 50.00), "Food bucket balance reduced to 50.00");
  }

  section("Account :: withdrawFromBucket rejects amount greater than total balance");
  {
    Account a;

    a.createBucket("Food", 100.0, false);
    a.deposit(100.00);

    auto [ok, message] = a.withdrawFromBucket(0, 150.00);

    CHECK(!ok, "withdrawal greater than total balance rejected");
    CHECK(approx(a.totalBalance(), 100.00), "total balance remains 100.00");
    CHECK(approx(a.buckets()[0].balance(), 100.00), "bucket balance remains 100.00");
  }

  section("Account :: withdrawFromBucket rejects amount greater than selected bucket balance");
  {
    Account a;

    a.createBucket("Food", 50.0, false);
    a.createBucket("Rent", 50.0, true);
    a.deposit(100.00);

    auto [ok, message] = a.withdrawFromBucket(0, 80.00);

    CHECK(!ok, "withdrawal greater than selected bucket balance rejected");
    CHECK(approx(a.totalBalance(), 100.00), "total balance remains unchanged");
    CHECK(approx(a.buckets()[0].balance(), 50.00), "Food bucket balance remains unchanged");
  }

  section("Account :: withdrawFromBucket rejects negative amount");
  {
    Account a;

    a.createBucket("Food", 100.0, false);
    a.deposit(100.00);

    auto [ok, message] = a.withdrawFromBucket(0, -50.00);

    CHECK(!ok, "negative withdrawal rejected");
    CHECK(approx(a.totalBalance(), 100.00), "total balance remains unchanged after negative withdrawal");
    CHECK(approx(a.buckets()[0].balance(), 100.00), "bucket balance remains unchanged after negative withdrawal");
  }

  section("Account :: withdrawFromBucket rejects zero amount");
  {
    Account a;

    a.createBucket("Food", 100.0, false);
    a.deposit(100.00);

    auto [ok, message] = a.withdrawFromBucket(0, 0.00);

    CHECK(!ok, "zero withdrawal rejected");
    CHECK(approx(a.totalBalance(), 100.00), "total balance remains unchanged after zero withdrawal");
    CHECK(approx(a.buckets()[0].balance(), 100.00), "bucket balance remains unchanged after zero withdrawal");
  }

  section("Account :: withdrawFromBucket rejects invalid bucket index");
  {
    Account a;

    a.createBucket("Food", 100.0, false);
    a.deposit(100.00);

    auto [ok, message] = a.withdrawFromBucket(5, 50.00);

    CHECK(!ok, "invalid bucket index rejected");
    CHECK(approx(a.totalBalance(), 100.00), "total balance remains unchanged after invalid bucket selection");
    CHECK(approx(a.buckets()[0].balance(), 100.00), "bucket balance remains unchanged after invalid bucket selection");
  }

  section("Account :: withdrawFromBucket works after committed bucket confirmation");
  {
    Account a;

    a.createBucket("Rent", 100.0, true);
    a.deposit(200.00);

    CHECK(a.buckets()[0].committed() == true, "Rent bucket is committed");

    auto [ok, message] = a.withdrawFromBucket(0, 50.00);

    CHECK(ok, "withdrawal from committed bucket succeeds after confirmation logic");
    CHECK(approx(a.totalBalance(), 150.00), "total balance reduced after committed bucket withdrawal");
    CHECK(approx(a.buckets()[0].balance(), 150.00), "committed bucket balance reduced correctly");
  }

  std::cout << "\n========================================\n";
  std::cout << " RESULTS: " << passed << " passed, " << failed << " failed\n";
  std::cout << "========================================\n";
  return failed == 0 ? 0 : 1;
}
