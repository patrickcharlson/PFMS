//
// Created by sithm on 03/05/2026.
//

#ifndef PFMS_TRANSACTION_H
#define PFMS_TRANSACTION_H

#include <string>
#include <vector>

class Transaction {
public:
  Transaction(std::string type, std::string details, double amount);
  std::string toString() const;

private:
  std::string type_;
  std::string details_;
  double amount_;
};

class TransactionJournal {
public:
  void addTransaction(const std::string& type, const std::string& details, double amount);
  void displayTransactions() const;

private:
  std::vector<Transaction> transactions_;
};

#endif // PFMS_TRANSACTION_H
