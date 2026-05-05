//
// Created by Patrick Charlson on 5/5/2026.
//

// =====================================================================
//  SHA-256
// =====================================================================

#include "AuthService.h"
#include "Sha256.h"
#include "test_helpers.h"


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

void run_crypto_tests() {
  test_sha256_reference_vector();
  test_sha256_determinism_and_distinctness();
  test_password_never_stored_plaintext();
}
