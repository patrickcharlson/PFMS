//
// Created by sithm on 29/04/2026.
//

#include "../include/Transaction.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>


Transaction::Transaction(const TxType type, const double amount, std::string description) :
    type_(type), amount_(amount), description_(std::move(description)), timestamp_(std::time(nullptr)) {}

std::string Transaction::typeLabel() const {
  switch (type_) {
    case TxType::Deposit:
      return "DEPOSIT";
    case TxType::Withdrawal:
      return "WITHDRAWAL";
    case TxType::Transfer:
      return "TRANSFER";
  }
  return "UNKNOWN";
}

std::string Transaction::formattedTimestamp() const {
  std::tm tmBuf{};
#if defined(_WIN32)
  localtime_s(&tmBuf, &timestamp_);
#else
  localtime_r(&timestamp_, &tmBuf);
#endif
  std::ostringstream oss;
  oss << std::put_time(&tmBuf, "%Y-%m-%d %H:%M:%S");
  return oss.str();
}
