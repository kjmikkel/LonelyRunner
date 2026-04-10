#pragma once
#include "data_structures.h"
#include <optional>
#include <span>

std::optional<NumResult> numerical_method(std::span<const int> speeds,
                                          bool find_maximum = false);
bool is_valid(const NumResult& result, std::span<const int> speeds);
bool is_valid(const GeoResult& result, std::span<const int> speeds);
bool check_for_solution(std::span<const int> speeds);
