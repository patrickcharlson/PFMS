//
// Created by Patrick Charlson on 25/4/2026.
//

#include "../include/AuthService.h"


#include <iostream>
#include <string>

static int passed {0};
static int failed {0};

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


static void section(const std::string &name) { std::cout << "\n[" << name << "]\n"; }

int main() {
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
}
