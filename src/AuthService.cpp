//
// Created by Patrick Charlson on 25/4/2026.
//

#include <string>

#include "../include/AuthService.h"
#include "../include/Sha256.h"

User::User(std::string username, std::string passwordHash) :
    username_(std::move(username)), passwordHash_(std::move(passwordHash)) {}


Status AuthService::registerUser(const std::string &username, const std::string &password) {
  if (username.empty())
    return Status::failure("Username cannot be empty.");
  if (password.size() < 4)
    return Status::failure("Password must be at least 4 characters.");

  std::string hash = Sha256::hash(password);
  users_[username] = std::make_unique<User>(username, hash);
  return Status::success("Account created. You can now log in.");
}

LoginOutcome AuthService::login(const std::string &username, const std::string &password) {
  if (locked_)
    return LoginOutcome::Locked;

  const auto it = users_.find(username);

  if (const bool ok = it != users_.end() && it->second->passwordHash() == Sha256::hash(password); !ok) {
    ++failedAttempts_;
    if (failedAttempts_ >= 3)
      locked_ = true;
    return locked_ ? LoginOutcome::Locked : LoginOutcome::BadCredentials;
  }

  failedAttempts_ = 0;
  currentUser_ = it->second.get();
  return LoginOutcome::Success;
}
