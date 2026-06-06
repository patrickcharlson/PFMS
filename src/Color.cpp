//
// Created by Patrick Charlson on 29/4/2026.
//

//
// Color.cpp
//

#include "Color.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace Color {

  void enable() {
#ifdef _WIN32
    // Windows 10+ supports ANSI escape sequences but the feature is
    // off by default for backwards compatibility. Flip it on for both
    // stdout and stderr so colored output renders properly.
    const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
      DWORD mode = 0;
      if (GetConsoleMode(hOut, &mode)) {
        SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
      }
    }
    const HANDLE hErr = GetStdHandle(STD_ERROR_HANDLE);
    if (hErr != INVALID_HANDLE_VALUE) {
      DWORD mode = 0;
      if (GetConsoleMode(hErr, &mode)) {
        SetConsoleMode(hErr, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
      }
    }
#endif
    // macOS and Linux: nothing to do — ANSI is already supported.
  }

}