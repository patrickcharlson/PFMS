//
// Created by Patrick Charlson on 21/4/2026.
//

#ifndef PFMS_ACCOUNT_H
#define PFMS_ACCOUNT_H

#include "Bucket.h"
#include <string>
#include <vector>

struct Status {
  bool ok;
  std::string message;
  static Status success(std::string m = "") { return {true, std::move(m)}; }
  static Status failure(std::string m) { return {false, std::move(m)}; }
};

class Account {
public:
  Status createBucket(const std::string& name, double percentage, bool committed = false);
  Status editBucket(size_t index, const std::string& newName, double newPercentage);
  Status deleteBucket(size_t index);
  Status toggleCommitted(size_t index);

  const std::vector<Bucket>& buckets() const { return buckets_; }


  // ---- Liquidity calculator  ----

  double totalBalance() const { return totalBalance_; }
  double committedTotal() const;
  double safeToSpend() const;


  // ---- Session lifecycle ----

  // Wipes all in-memory state. Called on logout.
  void clearSession();

private:
  std::vector<Bucket> buckets_;
  double totalBalance_{0.0};
};


#endif // PFMS_ACCOUNT_H
