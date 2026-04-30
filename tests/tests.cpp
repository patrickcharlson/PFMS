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

  std::cout << "\n========================================\n";
  std::cout << " RESULTS: " << passed << " passed, " << failed << " failed\n";
  std::cout << "========================================\n";
  return failed == 0 ? 0 : 1;
}
