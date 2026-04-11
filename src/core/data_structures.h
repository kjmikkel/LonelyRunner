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

// Result from the prime modular method.
// The lonely time is a / ((n+1) * prime), where n = number of runners.
struct PrimeModResult {
    bool found{false};
    int  prime{};  // p — the denominator factor
    int  a{};      // numerator: t = a / ((n+1)*prime)
};

enum class Algorithm { Geometric, Numerical, PrimeModular };

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
