#pragma once
#include <vector>
#include <chrono>
#include <string>

enum class PointType { START, END, FINAL };

struct EventPoint {
    int local_position{};
    int number_of_runners{};
    int rounds{};           // int (not unsigned) — fixes signed/unsigned bug #4
    int speed{};
    int runner_number{};
    PointType type{PointType::START};
};

struct GeoResult {
    bool found{false};
    EventPoint point{};
};

struct NumResult {
    bool found{false};
    int k1{}, k2{}, a{};
};

enum class Algorithm { Geometric, Numerical };

struct RangeConfig {
    int start_value{1};
    int end_value{100};
    int num_runners{3};
    int start_max_value{3};
    bool pre_test{false};
    Algorithm algorithm{Algorithm::Geometric};
};

struct RangeResult {
    enum class Status { Clean, ViolationFound, Cancelled };
    Status status{Status::Clean};
    std::vector<int> speeds{};
    std::chrono::microseconds elapsed{};
};
