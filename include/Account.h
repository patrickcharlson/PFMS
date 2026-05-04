//
// Created by Patrick Charlson on 21/4/2026.
//

#ifndef PFMS_ACCOUNT_H
#define PFMS_ACCOUNT_H

#include "Bucket.h"
#include "Transaction.h"

#include <string>
#include <vector>

struct Status {
  bool ok;
  std::string message;
  static Status success(std::string m = "") { return {true, std::move(m)}; }
  static Status failure(std::string m) { return {false, std::move(m)}; }
};

// Withdrawal pre-check result — the UI uses this to decide whether
// to show a warning before calling withdraw().
enum class WithdrawCheck { Ok, ExceedsBalance, ExceedsSafeToSpend };

class Account {
public:
  // ---- Bucket management  ----

  Status createBucket(const std::string& name, double percentage, bool committed = false);
  Status editBucket(size_t index, const std::string& newName, double newPercentage);
  Status deleteBucket(size_t index);
  Status toggleCommitted(size_t index);

  // ---- Money operations  ----
  Status deposit(double amount);
  WithdrawCheck checkWithdrawal(double amount) const;
  Status withdraw(double amount);
  Status transferFromUnallocated(size_t bucketIndex, double amount);

  const std::vector<Bucket>& buckets() const { return buckets_; }


  // ---- Liquidity calculator  ----

  double totalBalance() const { return totalBalance_; }
  double committedTotal() const;
  double safeToSpend() const;
  double unallocated() const { return unallocated_; }
  double allocatedPercentageTotal() const;


  // ---- Journal ----
  const std::vector<Transaction>& journal() const { return journal_; }

  // ---- Session lifecycle ----

  // Wipes all in-memory state. Called on logout.
  void clearSession();

private:
  std::vector<Bucket> buckets_;
  std::vector<Transaction> journal_;
  double totalBalance_{0.0};
  double unallocated_{0.0};

  void distributeDeposit(double amount);
  void log(TxType type, double amount, const std::string& description);
};


#endif // PFMS_ACCOUNT_H
