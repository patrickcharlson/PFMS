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

  static void showHeader(const std::string &title);
  static void showFooter(const std::string &prompt);
  static void showError(const std::string &message);
  static void showWarning(const std::string &message);
  static void showInfo(const std::string &message);

  static bool readLine(const std::string &prompt, std::string &out, bool maskHelp = true);

  static void showHelp(const std::string &screen);


  void runWelcome();
  void runRegister();
  void runLogin();
  void runMainMenu();
};

#endif // PFMS_PFMS_H
