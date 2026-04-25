//
// Created by Patrick Charlson on 25/4/2026.
//

#ifndef PFMS_BUCKET_H
#define PFMS_BUCKET_H

#include <string>


class Bucket {
  public:
  Bucket(std::string name, double percentage, bool committed = false);

  const std::string &name() { return name_; }
  double percentage() const { return percentage_; }
  double balance() const { return balance_; }

  void setName(const std::string &n) { name_ = n; }

  private:
  std::string name_;
  double percentage_;
  double balance_{0.0};
  bool committed_;
};

#endif // PFMS_BUCKET_H
