//
// Created by Patrick Charlson on 25/4/2026.
//

#ifndef PFMS_SHA256_H
#define PFMS_SHA256_H
#include <string>
#include <cstdint>

class Sha256 {
public:
  static std::string hash(const std::string& input);

private:
  static void transform(uint32_t state[8], const uint8_t block[64]);
};

#endif // PFMS_SHA256_H
