//
// Created by Patrick Charlson on 5/5/2026.
//

#ifndef PFMS_TEST_HELPERS_H
#define PFMS_TEST_HELPERS_H

#include <cmath>
#include <iostream>

extern int passed;
extern int failed;
extern int section_count; // total sections seen
extern int section_passed; // checks passed in current section
extern int section_failed; // checks failed in current section

// ---- Colour palette ----
namespace TestColor {
  inline const std::string Reset = "\033[0m";
  inline const std::string Bold = "\033[1m";
  inline const std::string Dim = "\033[2m";
  inline const std::string Underline = "\033[4m";
  inline const std::string Reverse = "\033[7m";

  // Standard
  inline const std::string Green = "\033[32m";
  inline const std::string Red = "\033[31m";
  inline const std::string Yellow = "\033[33m";
  inline const std::string Blue = "\033[34m";
  inline const std::string Magenta = "\033[35m";
  inline const std::string Cyan = "\033[36m";

  // Bright
  inline const std::string BrightGreen = "\033[92m";
  inline const std::string BrightRed = "\033[91m";
  inline const std::string BrightYellow = "\033[93m";
  inline const std::string BrightBlue = "\033[94m";
  inline const std::string BrightMagenta = "\033[95m";
  inline const std::string BrightCyan = "\033[96m";

  // Background
  inline const std::string BgGreen = "\033[42m";
  inline const std::string BgRed = "\033[41m";
  inline const std::string BgYellow = "\033[43m";
} // namespace TestColor

// ---- CHECK macro: PASS in green badge, FAIL in red reverse-video ----
//
// PASS uses subtle green text + dim label so the eye glides past hundreds of
// passing checks. FAIL uses reverse video (white text on red background) +
// bold label so a single failure is impossible to miss when scrolling.
#define CHECK(cond, label)                                                                                             \
  do {                                                                                                                 \
    if (cond) {                                                                                                        \
      ++passed;                                                                                                        \
      ++section_passed;                                                                                                \
      std::cout << "  " << TestColor::BrightGreen << "✓ PASS" << TestColor::Reset << "  " << TestColor::Dim << (label) \
                << TestColor::Reset << "\n";                                                                           \
    } else {                                                                                                           \
      ++failed;                                                                                                        \
      ++section_failed;                                                                                                \
      std::cout << "  " << TestColor::BgRed << TestColor::Bold << " ✗ FAIL " << TestColor::Reset << "  "               \
                << TestColor::BrightRed << TestColor::Bold << (label) << TestColor::Reset << "\n";                     \
    }                                                                                                                  \
  } while (0)

static bool approx(const double a, const double b, const double eps = 1e-6) { return std::fabs(a - b) < eps; }

// ---- Section header with auto-numbering ----
//
// Prints a small mini-summary of the previous section before starting the new
// one (so you can see at a glance whether the section we just left was clean).
inline void section(const std::string& name) {
  // If we're not on the first section, show how the previous one ended
  if (section_count > 0) {
    if (section_failed == 0) {
      std::cout << "  " << TestColor::Dim
                << "└─ " << section_passed << " checks, all passing"
                << TestColor::Reset << "\n";
    } else {
      std::cout << "  " << TestColor::BrightRed << TestColor::Bold
                << "└─ " << section_failed << " of "
                << (section_passed + section_failed) << " checks failed"
                << TestColor::Reset << "\n";
    }
  }
  ++section_count;
  section_passed = 0;
  section_failed = 0;

  std::cout << "\n"
            << TestColor::BrightCyan << TestColor::Bold
            << "▸ §" << section_count << "  "
            << TestColor::Reset
            << TestColor::BrightCyan
            << name
            << TestColor::Reset
            << "\n";
}

// ---- Group banner — shown by each run_*_tests() at its start ----
//
// Big visual marker so you know which feature group has just begun.
// Use a coloured background bar so it really pops between sections.
inline void groupBanner(const std::string& title) {
  std::cout << "\n"
            << TestColor::BrightYellow << TestColor::Bold
            << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
            << TestColor::Reset << "\n"
            << TestColor::BrightYellow << TestColor::Bold
            << "  " << title
            << TestColor::Reset << "\n"
            << TestColor::BrightYellow << TestColor::Bold
            << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
            << TestColor::Reset << "\n";
}
// Helper used across multiple test files: sum of bucket balances + unallocated
// must equal totalBalance.
class Account;
double reconciledTotal(const Account& a);

void run_auth_tests();
void run_bucket_tests();
void run_deposit_tests();
void run_transfer_tests();
void run_withdraw_tests();
void run_liquidity_tests();
void run_journal_tests();
void run_crypto_tests();
void run_safety_tests();

#endif // PFMS_TEST_HELPERS_H
