#include "prime.h"
#include <numeric>

std::vector<int> prime_sieve(int up_to) {
    if (up_to < 2) return {};

    std::vector<int> sieve(up_to + 1);
    std::iota(sieve.begin(), sieve.end(), 0);
    sieve[0] = sieve[1] = 0;

    for (int f = 2; f * f <= up_to; ++f) {
        if (sieve[f] == 0) continue;
        for (int m = f + f; m <= up_to; m += f)
            sieve[m] = 0;
    }

    std::vector<int> primes;
    for (int i = 2; i <= up_to; ++i)
        if (sieve[i] != 0) primes.push_back(i);
    return primes;
}
