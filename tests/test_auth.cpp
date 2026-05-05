//
// Created by Patrick Charlson on 5/5/2026.
//

// =====================================================================
//  User Authentication
// =====================================================================

#include "AuthService.h"
#include "test_helpers.h"

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
  CHECK(!svc.registerUser("AlIcE", "more012").ok, "'AlIcE' rejected as duplicate");
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


void run_auth_tests() {
  test_authentication_register_and_duplicate();
  test_authentication_empty_inputs();
  test_authentication_login_success_failure();
  test_authentication_lockout_after_three_failures();
  test_authentication_failure_counter_resets_on_success();
  test_authentication_logout();
  test_registration_username_case_insensitive();
  test_login_username_case_insensitive();
}
