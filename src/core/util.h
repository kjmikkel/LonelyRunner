#pragma once
#include "data_structures.h"
#include <filesystem>
#include <vector>
#include <string>

std::vector<int> read_speeds_from_json(const std::filesystem::path& path);

void save_result(const std::filesystem::path& path,
                 const RangeResult& result,
                 const std::vector<int>& speeds,
                 const std::string& algorithm,
                 int numerator, int denominator);

bool verify_result(const std::filesystem::path& path);
