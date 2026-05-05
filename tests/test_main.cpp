//
// Created by Patrick Charlson on 5/5/2026.
//


// =====================================================================
//  test Suite
// =====================================================================

#include "test_helpers.h"

#include <chrono>
#include <iomanip>
#include <iostream>

int main() {
  using clock = std::chrono::steady_clock;
  const auto t0 = clock::now();


  // ---- Suite banner ----
  std::cout << "\n"
            << TestColor::BrightMagenta << TestColor::Bold
            << "╔═══════════════════════════════════════════════════════════╗\n"
            << "║                                                           ║\n"
            << "║      PFMS v1.0  ·  AUTOMATED TEST SUITE                   ║\n"
            << "║                                                           ║\n"
            << "╚═══════════════════════════════════════════════════════════╝" << TestColor::Reset << "\n";

  std::cout << "  " << TestColor::Dim << "Personal Finance & Liquidity Management System" << TestColor::Reset << "\n";
  std::cout << "  " << TestColor::Dim << "Domain layer · 9 feature groups · ~194 checks" << TestColor::Reset << "\n";


  run_auth_tests();
  run_bucket_tests();
  run_deposit_tests();
  run_transfer_tests();
  run_withdraw_tests();
  run_liquidity_tests();
  run_journal_tests();
  run_crypto_tests();
  run_safety_tests();

  // Trailing summary line for the very last section
  if (section_failed == 0) {
    std::cout << "  " << TestColor::Dim << "└─ " << section_passed << " checks, all passing" << TestColor::Reset
              << "\n";
  } else {
    std::cout << "  " << TestColor::BrightRed << TestColor::Bold << "└─ " << section_failed << " of "
              << (section_passed + section_failed) << " checks failed" << TestColor::Reset << "\n";
  }

  // ---- Timing ----
  const auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - t0).count();

  // ---- Summary card ----
  const int total = passed + failed;
  const double pct = total > 0 ? (100.0 * passed / total) : 0.0;
  const bool clean = (failed == 0);

  std::cout << "\n"
            << TestColor::BrightCyan << "┌────────────────────────────────────────────────────────────┐\n"
            << "│                       RESULTS                              │\n"
            << "├────────────────────────────────────────────────────────────┤\n"
            << TestColor::Reset;

  // Passed line — green, padded
  std::cout << TestColor::BrightCyan << "│ " << TestColor::Reset << "  " << TestColor::BrightGreen
            << "✓ Passed:    " << TestColor::Bold << std::setw(4) << passed << TestColor::Reset << TestColor::Dim
            << "    (" << std::fixed << std::setprecision(1) << pct << "%)" << TestColor::Reset << std::string(28, ' ')
            << TestColor::BrightCyan << "│" << TestColor::Reset << "\n";

  // Failed line — red if any, dim if zero
  std::cout << TestColor::BrightCyan << "│ " << TestColor::Reset << "  ";
  if (clean) {
    std::cout << TestColor::Dim << "✓ Failed:    " << std::setw(4) << failed << TestColor::Reset;
    std::cout << std::string(40, ' ');
  } else {
    std::cout << TestColor::BrightRed << "✗ Failed:    " << TestColor::Bold << std::setw(4) << failed
              << TestColor::Reset;
    std::cout << std::string(40, ' ');
  }
  std::cout << TestColor::BrightCyan << "│" << TestColor::Reset << "\n";

  // Sections covered
  std::cout << TestColor::BrightCyan << "│ " << TestColor::Reset << "  " << TestColor::Cyan
            << "▸ Sections:  " << TestColor::Bold << std::setw(4) << section_count << TestColor::Reset
            << std::string(40, ' ') << TestColor::BrightCyan << "│" << TestColor::Reset << "\n";

  // Duration
  std::cout << TestColor::BrightCyan << "│ " << TestColor::Reset << "  " << TestColor::Magenta
            << "⏱ Duration:  " << TestColor::Bold << std::setw(4) << elapsed_ms << " ms" << TestColor::Reset
            << std::string(37, ' ') << TestColor::BrightCyan << "│" << TestColor::Reset << "\n";

  std::cout << TestColor::BrightCyan << "├────────────────────────────────────────────────────────────┤\n"
            << TestColor::Reset;

  // Verdict line
  if (clean) {
    std::cout << TestColor::BrightCyan << "│ " << TestColor::Reset << "  " << TestColor::BgGreen << TestColor::Bold
              << "  ALL TESTS PASSED  " << TestColor::Reset << "   " << TestColor::BrightGreen << "Suite is healthy."
              << TestColor::Reset << std::string(17, ' ') << TestColor::BrightCyan << "│" << TestColor::Reset << "\n";
  } else {
    std::cout << TestColor::BrightCyan << "│ " << TestColor::Reset << "  " << TestColor::BgRed << TestColor::Bold
              << "  " << failed << " TEST(S) FAILING  " << TestColor::Reset << "  " << TestColor::BrightRed
              << "Review failures above." << TestColor::Reset << std::string(15, ' ') << TestColor::BrightCyan << "│"
              << TestColor::Reset << "\n";
  }

  std::cout << TestColor::BrightCyan << "└────────────────────────────────────────────────────────────┘"
            << TestColor::Reset << "\n\n";

  return clean ? 0 : 1;
}
