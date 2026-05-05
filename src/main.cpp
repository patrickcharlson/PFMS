
#include "../include/Color.h"
#include "../include/PFMS.h"
#include <iostream>


int main() {
  Color::enable();
  std::cout << "\n";
  std::cout << Color::Cyan << "========================================" << Color::Reset << "\n";
  std::cout << Color::Bold << Color::BrightGreen << " Personal Finance & Liquidity Management" << Color::Reset << "\n";
  std::cout << " " << Color::Bold << "      System (PFMS)" << Color::Reset << Color::Dim << " — Version 1.0"
            << Color::Reset << "\n";
  std::cout << Color::Cyan << "========================================" << Color::Reset << "\n";
  PFMS ms;
  ms.run();
  std::cout << "\n Goodbye.\n";
  return 0;
}
