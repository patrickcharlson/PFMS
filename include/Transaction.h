//
// Created by sithm on 03/05/2026.
//

#ifndef PFMS_TRANSACTION_H
#define PFMS_TRANSACTION_H

#include <string>
#include <ctime>

enum class TxType { Deposit, Withdrawal, Transfer };

class Transaction {
public:
  Transaction(TxType type, double amount, std::string description);

  TxType type() const { return type_; }
  double amount() const { return amount_; }
  const std::string& description() const { return description_; }
  std::time_t timestamp() const { return timestamp_; }

  std::string typeLabel() const;
  std::string formattedTimestamp() const;

private:
  TxType type_;
  double amount_;
  std::string description_;
  std::time_t timestamp_;
};

#endif // PFMS_TRANSACTION_H
