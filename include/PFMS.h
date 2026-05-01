//
// Created by Patrick Charlson on 25/4/2026.
//

#ifndef PFMS_PFMS_H
#define PFMS_PFMS_H

#include <string>

#include "AuthService.h"

class PFMS {
public:
  PFMS();
  void run();

private:
  AuthService auth_;
  bool quit_{false};


  // ---- Screen builders (layout: title, body, footer) ----

  static void showHeader(const std::string& title);
  static void showFooter(const std::string& prompt);
  static void showError(const std::string& message);
  static void showWarning(const std::string& message);
  static void showInfo(const std::string& message);


  // ---- Input helpers ----

  static bool readLine(const std::string& prompt, std::string& out, bool maskHelp = true);
  static bool readDouble(const std::string& prompt, double& out);
  static bool readSizeT(const std::string& prompt, size_t& out);
  static bool confirm(const std::string& prompt); // Y/N - returns true on Y

  // ---- Help (context-sensitive) ----

  static void showHelp(const std::string& screen);


  // ---- Screens ----

  void runWelcome();
  void runRegister();
  void runLogin();
  void runMainMenu();
  void runAccountSummary();
  void runBucketMenu();
  void runCreateBucket();
  void runEditBucket();
  void runDeleteBucket();
  void runToggleCommitted();
  void runDeposit();
  void runWithdraw();


  // ---- Formatting ----

  static std::string fmtMoney(double v);
};

#endif // PFMS_PFMS_H
