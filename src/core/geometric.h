#pragma once
#include "data_structures.h"
#include <optional>
#include <span>

std::optional<GeoResult> geometric_method(std::span<const int> speeds);
