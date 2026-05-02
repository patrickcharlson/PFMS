//
// Created by Patrick Charlson on 25/4/2026.
//

#include "PFMS.h"

#include "Color.h"

#include <iomanip>
#include <iostream>
#include <string>
#include <sstream>


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

void PFMS::showHeader(const std::string& title) {
  std::cout << "\n" << DIVIDER << "\n";
  std::cout << " PFMS — " << title << "  [? = Help]\n";
  std::cout << DIVIDER << "\n";
}


// ---------- Screen scaffolding ----------

void PFMS::showFooter(const std::string& prompt) { std::cout << SUBDIV << "\n " << prompt << " "; }

void PFMS::showError(const std::string& message) {
  std::cout << "\n" << Color::BrightRed << "[ERROR] " << message << Color::Reset << "\n";
}

void PFMS::showWarning(const std::string& message) {
  std::cout << "\n"
            << Color::BrightYellow << "*** WARNING ***" << Color::Reset << "\n " << Color::Yellow << message
            << Color::Reset << "\n";
}
void PFMS::showInfo(const std::string& message) {
  std::cout << "\n " << Color::Green << message << Color::Reset << "\n";
}

// ---------- Input helpers ----------

bool PFMS::readLine(const std::string& prompt, std::string& out, bool /*maskHelp*/) {
  std::cout << " " << prompt << " ";
  if (!std::getline(std::cin, out)) {
    out.clear();
    return false;
  }

  while (!out.empty() && (out.back() == '\r' || out.back() == ' ' || out.back() == '\t'))
    out.pop_back();
  return true;
}

bool PFMS::readDouble(const std::string& prompt, double& out) {
  std::string line;
  if (!readLine(prompt, line))
    return false;
  if (line == "?")
    return false;
  try {
    size_t idx = 0;
    const double v = std::stod(line, &idx);
    if (idx != line.size())
      return false;
    out = v;
    return true;
  } catch (...) {
    return false;
  }
}

bool PFMS::readSizeT(const std::string& prompt, size_t& out) {
  std::string line;
  if (!readLine(prompt, line))
    return false;
  if (line == "?")
    return false;
  try {
    size_t idx = 0;
    const long long v = std::stoll(line, &idx);
    if (idx != line.size() || v < 0)
      return false;
    out = static_cast<size_t>(v);
    return true;
  } catch (...) {
    return false;
  }
}

bool PFMS::confirm(const std::string& prompt) {
  std::string line;
  while (true) {
    if (!readLine(prompt + " (Y/N):", line))
      return false;
    if (line == "Y" || line == "y")
      return true;
    if (line == "N" || line == "n")
      return false;
    showError("Please enter Y or N.");
  }
}


// ---------- Help ----------

void PFMS::showHelp(const std::string& screen) {
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
  if (line == "1")
    runAccountSummary();
  else if (line == "2")
    runBucketMenu();
  else if (line == "3")
    runDeposit();
  else if (line == "4")
  runWithdraw();
  else if (line == "7") {
    auth_.logout();
    showInfo("Logged out. Session cleared.");
  } else
    showError("Please enter a number from 1 to 7.");
}


// ---------- Account Summary  ----------

void PFMS::runAccountSummary() {
  const auto& acc = auth_.currentUser()->account();
  showHeader("ACCOUNT SUMMARY");
  std::cout << " " << Color::Bold << Color::BrightGreen << "SAFE TO SPEND: " << fmtMoney(acc.safeToSpend())
            << Color::Reset << "\n";
  std::cout << "" << SUBDIV << "\n";
  std::cout << " Total Balance:    " << Color::Cyan << fmtMoney(acc.totalBalance()) << Color::Reset << "\n";
  std::cout << " Committed Funds:  " << Color::Yellow << fmtMoney(acc.committedTotal()) << Color::Reset << "\n";
  std::cout << " " << Color::Bold << "BUCKETS:" << Color::Reset << "\n";
  if (acc.buckets().empty()) {
    std::cout << "   (no buckets configured)\n";
  } else {
    size_t i = 1;
    for (const auto& b: acc.buckets()) {
      std::cout << "   [" << i++ << "] " << std::left << std::setw(15) << b.name() << " " << std::setw(10)
                << fmtMoney(b.balance()) << " " << std::setw(4)
                << (std::to_string(static_cast<int>(b.percentage())) + "%") << " ";
      if (b.committed()) {
        std::cout << Color::Yellow << "COMMITTED" << Color::Reset;
      }
      std::cout << "\n";
    }
  }
  std::cout << " " << Color::Dim << "Unallocated:      " << fmtMoney(acc.unallocated()) << Color::Reset << "\n";
  showFooter("Press Enter to return to Main Menu.");
  std::string s;
  std::getline(std::cin, s);
}


// ---------- Bucket Menu  ----------

void PFMS::runBucketMenu() {
  while (true) {
    showHeader("BUCKET MANAGEMENT");
    std::cout << " [1] Create Bucket\n [2] Edit Bucket\n [3] Delete Bucket\n"
              << " [4] Toggle Committed Status\n [5] Back\n";
    showFooter("Enter choice (1-5) or ? for help:");
    std::string line;
    if (!readLine("", line))
      return;
    if (line == "?") {
      showHelp("Bucket Menu");
      continue;
    }
    if (line == "1")
      runCreateBucket();
    else if (line == "2")
      runEditBucket();
    else if (line == "3")
      runDeleteBucket();
    else if (line == "4")
      runToggleCommitted();
    else if (line == "5")
      return;
    else
      showError("Please enter a number from 1 to 5.");
  }
}

void PFMS::runCreateBucket() {
  auto& acc = auth_.currentUser()->account();
  showHeader("CREATE BUCKET");
  std::string name;
  if (!readLine("Bucket name (e.g., Rent):", name) || name.empty()) {
    showError("Bucket name cannot be empty.");
    return;
  }
  double pct;
  if (!readDouble("Allocation percentage (e.g,. 40):", pct)) {
    showError("Bucket name cannot be empty");
    return;
  }
  std::string commitLine;
  const bool committed = confirm("Mark this bucket as Committed?");
  if (auto [ok, message] = acc.createBucket(name, pct, committed); ok)
    showInfo(message);
  else
    showError(message);
}

void PFMS::runEditBucket() {
  auto& acc = auth_.currentUser()->account();
  showHeader("EDIT BUCKET");
  if (acc.buckets().empty()) {
    showError("No buckets to edit.");
    return;
  }
  size_t i = 1;
  for (const auto& b: acc.buckets())
    std::cout << " [" << i++ << "] " << b.name() << " (" << static_cast<int>(b.percentage()) << "%)\n";

  size_t sel;
  if (!readSizeT("Select bucket number:", sel) || sel < 1 || sel > acc.buckets().size()) {
    showError("Invalid bucket selection.");
    return;
  }
  std::string newName;
  if (!readLine("New name:", newName) || newName.empty()) {
    showError("Bucket name cannot be empty.");
    return;
  }
  double newPct;
  if (!readDouble("New percentage (0-100):", newPct)) {
    showError("Please enter a numeric percentage.");
    return;
  }
  if (auto [ok, message] = acc.editBucket(sel - 1, newName, newPct); ok)
    showInfo(message);
  else
    showError(message);
}

void PFMS::runDeleteBucket() {
  auto& acc = auth_.currentUser()->account();
  showHeader("DELETE BUCKET");
  if (acc.buckets().empty()) {
    showError("No buckets to delete.");
    return;
  }
  size_t i = 1;
  for (const auto& b: acc.buckets())
    std::cout << " [" << i++ << "] " << b.name() << " " << fmtMoney(b.balance()) << "\n";

  size_t sel;
  if (!readSizeT("Select bucket number to delete:", sel) || sel < 1 || sel > acc.buckets().size()) {
    showError("Invalid bucket selection.");
    return;
  }
  showWarning("Deleting '" + acc.buckets()[sel - 1].name() + "' will return its balance (" +
              fmtMoney(acc.buckets()[sel - 1].balance()) + ") to the unallocated pool.");
  if (!confirm("Proceed with deletion?")) {
    showInfo("Deletion cancelled.");
    return;
  }
  if (auto [ok, message] = acc.deleteBucket(sel - 1); ok)
    showInfo(message);
  else
    showError(message);
}

void PFMS::runToggleCommitted() {
  auto& acc = auth_.currentUser()->account();
  showHeader("TOGGLE COMMITTED");
  if (acc.buckets().empty()) {
    showHeader("No buckets configured.");
    return;
  }
  size_t i = 1;
  for (const auto& b: acc.buckets()) {
    std::cout << " [" << i++ << "] " << b.name() << " " << (b.committed() ? "[COMMITTED]" : "[not committed]") << "\n";
  }

  size_t sel;
  if (!readSizeT("Select bucket number:", sel) || sel < 1 || sel > acc.buckets().size()) {
    showError("Invalid bucket selection.");
    return;
  }
  if (auto [ok, message] = acc.toggleCommitted(sel - 1); ok) {
    showInfo(message);
    showInfo("Updated Safe to Spend: " + fmtMoney(acc.safeToSpend()));
  } else {
    showError(message);
  }
}


// ---------- Deposit / Withdraw / Transfer  ----------

void PFMS::runDeposit() {
  auto& acc = auth_.currentUser()->account();
  showHeader("DEPOSIT");
  double amount;
  if (!readDouble("Enter deposit amount (e.g., 150.00):", amount)) {
    showError("Please enter a numeric amount (e.g., 150.00).");
    return;
  }
  auto [ok, message] = acc.deposit(amount);
  if (!ok) {
    showError(message);
    return;
  }
  showInfo(message);
  showInfo("New Total Balance: " + fmtMoney(acc.totalBalance()));
  showInfo("Safe to Spend:     " + fmtMoney(acc.safeToSpend()));
}

void PFMS::runWithdraw() {
  auto& acc = auth_.currentUser()->account();

  showHeader("WITHDRAW");

  if (acc.totalBalance() <= 0.0) {
    showError("No funds are available to withdraw.");
    return;
  }

  if (acc.buckets().empty()) {
    showError("No buckets available. Create a bucket before withdrawing.");
    return;
  }

  std::cout << " Available buckets:\n";

  size_t i = 1;
  for (const auto& b : acc.buckets()) {
    std::cout << " [" << i++ << "] "
              << b.name()
              << " — Balance: " << fmtMoney(b.balance())
              << (b.committed() ? " — COMMITTED" : "")
              << "\n";
  }

  size_t selectedBucket;
  if (!readSizeT("Select bucket number:", selectedBucket) ||
      selectedBucket < 1 ||
      selectedBucket > acc.buckets().size()) {
    showError("Invalid bucket selection.");
    return;
  }

  double amount;
  if (!readDouble("Withdrawal amount (e.g., 50.00):", amount)) {
    showError("Please enter a numeric withdrawal amount.");
    return;
  }

  if (!(amount > 0.0)) {
    showError("Withdrawal amount must be positive.");
    return;
  }

  if (amount > acc.totalBalance() + 1e-9) {
    showError("Withdrawal amount cannot exceed total balance.");
    return;
  }

  const auto& bucket = acc.buckets()[selectedBucket - 1];

  if (amount > bucket.balance() + 1e-9) {
    showError("Withdrawal amount cannot exceed the selected bucket balance.");
    return;
  }

  if (bucket.committed()) {
    showWarning("You are withdrawing from a committed bucket: " + bucket.name() +
                ". This may affect money reserved for important expenses.");

    if (!confirm("Do you want to continue?")) {
      showInfo("Withdrawal cancelled.");
      return;
    }
  }

  if (auto [ok, message] = acc.withdrawFromBucket(selectedBucket - 1, amount); ok) {
    showInfo(message);
    showInfo("New Total Balance: " + fmtMoney(acc.totalBalance()));
    showInfo("Safe to Spend:     " + fmtMoney(acc.safeToSpend()));
  } else {
    showError(message);
  }
}


// ---------- Formatting ----------

std::string PFMS::fmtMoney(const double v) {
  const bool negative = v < 0;
  const double abs = negative ? -v : v;
  std::ostringstream raw;
  raw << std::fixed << std::setprecision(2) << abs;
  std::string s = raw.str();

  const auto dot = s.find('.');
  std::string intPart = s.substr(0, dot);
  const std::string decPart = s.substr(dot);

  if (intPart.size() > 3) {
    for (int i = static_cast<int>(intPart.size()) - 3; i > 0; i -= 3)
      intPart.insert(static_cast<size_t>(i), ",");
  }
  return std::string(negative ? "-$" : "$") + intPart + decPart;
}
