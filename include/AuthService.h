//
// Created by Patrick Charlson on 25/4/2026.
//

#ifndef PFMS_AUTH_SERVICE_H
#define PFMS_AUTH_SERVICE_H

#include <map>
#include <memory>
#include <string>

#include "Account.h"

class User {
public:
  User(std::string username, std::string passwordHash);

  const std::string& username() { return username_; }
  const std::string& passwordHash() { return passwordHash_; }
  Account& account() { return account_; }

private:
  std::string username_;
  std::string passwordHash_;
  Account account_;
};

enum class LoginOutcome { Success, BadCredentials, Locked };

class AuthService {
public:
  Status registerUser(const std::string& username, const std::string& password);

  LoginOutcome login(const std::string& username, const std::string& password);

  bool isLocked() const { return locked_; }
  int failedAttempts() const { return failedAttempts_; }

  User *currentUser() { return currentUser_; }
  void logout();

private:
  std::map<std::string, std::unique_ptr<User>> users_;
  User *currentUser_{nullptr};
  int failedAttempts_{0};
  bool locked_{false};
};

#endif // PFMS_AUTH_SERVICE_H
