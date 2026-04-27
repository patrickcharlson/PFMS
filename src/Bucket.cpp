//
// Created by Patrick Charlson on 26/4/2026.
//

#include "../include/Bucket.h"

#include "Account.h"

#include <string>

Bucket::Bucket(std::string name, double percentage, bool committed) :
    name_(std::move(name)), percentage_(percentage), committed_(committed) {}
