#pragma once
#include <vector>
#include <string>
#include "record.h"

std::vector<Record> parse_csv(const std::string& filename);
