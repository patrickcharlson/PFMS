
#include "Color.h"
#include "PFMS.h"
#include <iostream>


int main() {
  Color::enable();
  std::cout << "\n";
  std::cout << "========================================\n";
  std::cout << " Personal Finance & Liquidity Management\n";
  std::cout << "      System (PFMS) — Version 1.0\n";
  std::cout << "========================================\n";
  PFMS ms;
  ms.run();
  std::cout << "\n Goodbye.\n";
  return 0;
}
