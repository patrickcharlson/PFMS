//
// Created by Patrick Charlson on 25/4/2026.
//

#include "../include/PFMS.h"

#include <iostream>
#include <string>


static constexpr int LINE_WIDTH = 40;
static const std::string DIVIDER(LINE_WIDTH, '=');
static const std::string SUBDIV(LINE_WIDTH, '-');

PFMS::PFMS() = default;

void PFMS::run() {
  while (!quit_) {
    if (auth_.isLocked()) {
      showHeader("LOCKED");
      showError("Session locked after 3 failed login attempts.");
      std::cout << "Please restart the application to retry.\n\n";
      quit_ = true;
      break;
    }
    if (!auth_.currentUser())
      runWelcome();
    else
      runMainMenu();
  }
}

void PFMS::showHeader(const std::string &title) {
  std::cout << "\n" << DIVIDER << "\n";
  std::cout << " PFMS — " << title << "  [? = Help]\n";
  std::cout << DIVIDER << "\n";
}


// ---------- Screen scaffolding ----------

void PFMS::showFooter(const std::string &prompt) { std::cout << SUBDIV << "\n " << prompt << " "; }

void PFMS::showError(const std::string &message) { std::cout << "\n[ERROR] " << message << "\n"; }

void PFMS::showWarning(const std::string &message) { std::cout << "\n*** WARNING ***\n " << message << "\n"; }

void PFMS::showInfo(const std::string &message) { std::cout << "\n " << message << "\n"; }


// ---------- Input helpers ----------

bool PFMS::readLine(const std::string &prompt, std::string &out, bool /*maskHelp*/) {
  std::cout << " " << prompt << " ";
  if (!std::getline(std::cin, out)) {
    out.clear();
    return false;
  }

  while (!out.empty() && (out.back() == '\r' || out.back() == ' ' || out.back() == '\t'))
    out.pop_back();
  return true;
}


// ---------- Help ----------

void PFMS::showHelp(const std::string &screen) {
  std::cout << "\n" << SUBDIV << "\n HELP — " << screen << "\n" << SUBDIV << "\n";
  if (screen == "Main Menu") {
    std::cout << " Choose a menu option by number.\n"
              << " Account Summary shows your Safe to Spend balance.\n"
              << " Buckets let you partition funds by purpose.\n"
              << " Mark a bucket as Committed to exclude it from Safe to Spend.\n";
  } else if (screen == "Bucket Menu") {
    std::cout << " Create, edit, delete or toggle commitment status on buckets.\n"
              << " Total of all bucket percentages may not exceed 100%.\n";
  } else if (screen == "Deposit") {
    std::cout << " Enter a positive amount. Funds are auto-distributed across\n"
              << " your buckets according to their percentages. Any rounding\n"
              << " remainder goes to the unallocated pool.\n";
  } else if (screen == "Withdraw") {
    std::cout << " Enter a positive amount up to your total balance.\n"
              << " If you withdraw more than Safe to Spend, you'll be warned\n"
              << " before any committed funds are touched.\n";
  } else {
    std::cout << " Type a number to select, or '?' on most prompts for help.\n";
  }
  std::cout << SUBDIV << "\n";
}

// ---------- Welcome / Auth ----------

void PFMS::runWelcome() {
  showHeader("WELCOME");
  std::cout << " [1] Login\n [2] Register\n [3] Exit\n";
  showFooter("Enter choice (1-3) or ? for help:");
  std::string line;
  if (!readLine("", line)) {
    quit_ = true;
  }
  if (line == "?") {
    showHelp("Welcome");
  }
  if (line == "1")
    runLogin();
  else if (line == "2")
    runRegister();
  else if (line == "3")
    quit_ = true;
  else
    showError("Please enter 1, 2 or 3.");
}

void PFMS::runRegister() {
  showHeader("REGISTER");
  std::string username, password;
  if (!readLine("Choose a username (e.g., patrick):", username) || username.empty()) {
    showError("Username cannot be empty.");
  }
  if (!readLine("Choose a password (min 4 characters):", password) || password.empty()) {
    showError("Password cannot be empty.");
  }
  if (auto [ok, message] = auth_.registerUser(username, password); !ok)
    showError(message);

  else {
    showInfo(message);
    if (const auto outcome = auth_.login(username, password); outcome != LoginOutcome::Success)
      showError("Auto-login failed; please log in manually.");
  }
}

void PFMS::runLogin() {
  showHeader("LOGIN");
  std::string username, password;
  if (!readLine("Username:", username))
    return;
  if (!readLine("Password:", password))
    return;
  switch (const auto outcome = auth_.login(username, password); outcome) {
    case LoginOutcome::Success:
      showInfo("Welcome back, " + username + ".");
      break;
    case LoginOutcome::BadCredentials:
      showError("Invalid credentials. Attempts remaining: " + std::to_string(3 - auth_.failedAttempts()) + ".");
      break;
    case LoginOutcome::Locked:
      showError("Session locked after 3 failed login attempts, Restart required.");
      break;
  }
}


// ---------- Main Menu ----------

void PFMS::runMainMenu() {
  showHeader("MAIN MENU — " + auth_.currentUser()->username());
  std::cout << " [1] Account Summary\n"
            << " [2] Manage Buckets\n"
            << " [3] Deposit\n"
            << " [4] Withdraw\n"
            << " [5] Transfer from Unallocated\n"
            << " [6] Transaction Journal\n"
            << " [7] Logout\n";
  showFooter("Enter choice (1-7) or ? for help:");
  std::string line;
  if (!readLine("", line)) {
    quit_ = true;
    return;
  }
  if (line == "?") {
    showHelp("Main Menu");
  }

  else if (line == "7") {
    auth_.logout();
    showInfo("Logged out. Session cleared.");
  } else
    showError("Please enter a number from 1 to 7.");
}
