//
// Created by Patrick Charlson on 26/4/2026.
//

#include "../include/Bucket.h"

#include "../include/Account.h"

#include <string>

Bucket::Bucket(std::string name, const double percentage, const bool committed) :
    name_(std::move(name)), percentage_(percentage), committed_(committed) {}
