#include "geometric.h"
#include "numerical.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <vector>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size < 1) return 0;

    // Interpret bytes as speeds 1..256 (no zeros), cap at 8 runners
    std::vector<int> speeds;
    speeds.reserve(std::min(size, size_t{8}));
    for (size_t i = 0; i < size && speeds.size() < 8; ++i)
        speeds.push_back(static_cast<int>(data[i]) + 1);

    // Conjecture requires strictly increasing speeds
    std::sort(speeds.begin(), speeds.end());
    speeds.erase(std::unique(speeds.begin(), speeds.end()), speeds.end());
    if (speeds.empty()) return 0;

    auto gr = geometric_method(speeds);
    auto nr = numerical_method(speeds);

    // Any result that claims success must actually satisfy the lonely condition
    if (gr) assert(is_valid(*gr, speeds));
    if (nr) assert(is_valid(*nr, speeds));

    // Geometric is the complete verifier — if it finds nothing, numerical cannot
    // legitimately claim a found solution
    if (!gr && nr) assert(!nr->found);

    return 0;
}
