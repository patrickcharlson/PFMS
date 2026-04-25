#include <iostream>

#include "../include/PFMS.h"

int main() {
  std::cout << "\n";
  std::cout << "========================================\n";
  std::cout << " Personal Finance & Liquidity Management\n";
  std::cout << "      System (PFMS) — Version 1.0\n";
  std::cout << "========================================\n";
  PFMS ui;
  ui.run();
  std::cout << "\n Goodbye.\n";
  return 0;
}
