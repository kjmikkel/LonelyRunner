#include "util.h"
#include "numerical.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <stdexcept>
#include <chrono>
#include <ctime>
using json = nlohmann::json;

std::vector<int> read_speeds_from_json(const std::filesystem::path& path) {
    std::ifstream f(path);
    if (!f) throw std::runtime_error("Cannot open: " + path.string());
    json j = json::parse(f);
    return j.get<std::vector<int>>();
}

void save_result(const std::filesystem::path& path,
                 const RangeResult& result,
                 const std::vector<int>& speeds,
                 const std::string& algorithm,
                 int numerator, int denominator) {
    json j;
    j["version"]      = 1;
    j["type"]         = (result.status == RangeResult::Status::ViolationFound)
                        ? "violation_candidate" : "solution";
    j["speeds"]       = speeds;
    j["num_runners"]  = static_cast<int>(speeds.size());
    j["algorithm"]    = algorithm;
    j["result_found"] = (result.status != RangeResult::Status::ViolationFound);
    j["time"]         = {{"numerator", numerator}, {"denominator", denominator}};

    auto now  = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&t));
    j["timestamp"] = buf;

    std::ofstream out(path);
    if (!out) throw std::runtime_error("Cannot write: " + path.string());
    out << j.dump(2);
}

bool verify_result(const std::filesystem::path& path) {
    std::ifstream f(path);
    if (!f) throw std::runtime_error("Cannot open: " + path.string());
    json j = json::parse(f);

    std::vector<int> speeds = j["speeds"].get<std::vector<int>>();
    int num = j["time"]["numerator"];
    int den = j["time"]["denominator"];
    if (den == 0) return false;

    // Reconstruct a NumResult using k1=den, k2=0 for is_valid (canonical form)
    NumResult nr;
    nr.found = true;
    nr.a     = num;
    nr.k1    = den;
    nr.k2    = 0;
    return is_valid(nr, speeds);
}
