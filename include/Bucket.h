//
// Created by Patrick Charlson on 25/4/2026.
//

#ifndef PFMS_BUCKET_H
#define PFMS_BUCKET_H

#include <string>


class Bucket {
public:
  Bucket(std::string name, double percentage, bool committed = false);

  const std::string& name() const { return name_; }
  double percentage() const { return percentage_; }
  double balance() const { return balance_; }
  bool committed() const { return committed_; }

  void setName(const std::string& n) { name_ = n; }
  void setPercentage(double p) { percentage_ = p; }
  void setCommitted(bool c) { committed_ = c; }

private:
  std::string name_;
  double percentage_;
  double balance_{0.0};
  bool committed_;
};

#endif // PFMS_BUCKET_H
