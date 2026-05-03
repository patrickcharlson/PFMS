//
// Created by sithm on 29/04/2026.
//

#include "Transaction.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>

Transaction::Transaction(std::string type, std::string details, double amount)
    : type_(std::move(type)), details_(std::move(details)), amount_(amount) {}

std::string Transaction::toString() const {
  std::ostringstream out;
  out << type_ << " | " << details_ << " | $"
      << std::fixed << std::setprecision(2) << amount_;
  return out.str();
}

void TransactionJournal::addTransaction(const std::string& type,
                                        const std::string& details,
                                        double amount) {
  transactions_.emplace_back(type, details, amount);
}

void TransactionJournal::displayTransactions() const {
  if (transactions_.empty()) {
    std::cout << "No transactions recorded yet.\n";
    return;
  }

  std::cout << "\n===== Transaction Journal =====\n";

  for (size_t i = 0; i < transactions_.size(); ++i) {
    std::cout << i + 1 << ". " << transactions_[i].toString() << "\n";
  }
}