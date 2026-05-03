//
// Created by Patrick Charlson on 29/4/2026.
//

#ifndef PFMS_COLOR_H
#define PFMS_COLOR_H

#include <string>

namespace Color {
  // Reset to terminal defaults — always pair this with any color above.
  inline const std::string Reset = "\033[0m";

  // Styles
  inline const std::string Bold = "\033[1m";
  inline const std::string Dim = "\033[2m";

  // Foreground colors
  inline const std::string Red = "\033[31m";
  inline const std::string Green = "\033[32m";
  inline const std::string Yellow = "\033[33m";
  inline const std::string Blue = "\033[34m";
  inline const std::string Magenta = "\033[35m";
  inline const std::string Cyan = "\033[36m";
  inline const std::string White = "\033[37m";

  // Bright variants — usually clearer on dark terminals
  inline const std::string BrightRed = "\033[91m";
  inline const std::string BrightGreen = "\033[92m";
  inline const std::string BrightYellow = "\033[93m";
  inline const std::string BrightCyan = "\033[96m";

  // Call once at program start. Enables ANSI escape processing on
  // Windows 10+; does nothing on macOS/Linux where it's already on.
  void enable();

} // namespace Color


#endif // PFMS_COLOR_H
