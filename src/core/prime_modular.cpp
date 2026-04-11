#include "prime_modular.h"
#include "prime.h"
#include <algorithm>

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

// Verify that the stored time t = a / ((n+1)*prime) satisfies the lonely
// condition for every runner.
//
// A runner with speed v is lonely at t iff its distance from the start point
// on the unit circle is >= 1/(n+1).  In integer arithmetic with denominator
// Q = (n+1)*prime:
//   min(a*v mod Q, Q - a*v mod Q) * (n+1) >= Q
// which simplifies to:
//   min(a*v mod Q, Q - a*v mod Q) >= prime
bool is_valid(const PrimeModResult& result, std::span<const int> speeds) {
    if (!result.found) return false;
    const int  n = static_cast<int>(speeds.size());
    const long Q = static_cast<long>(n + 1) * result.prime;
    for (int v : speeds) {
        long pos  = (static_cast<long long>(result.a) * v) % Q;
        long dist = std::min(pos, Q - pos);
        if (dist * (n + 1) < Q) return false;
    }
    return true;
}

// For each prime p <= max_prime, check all numerators a in {1,...,(n+1)*p - 1}.
// Time t = a / ((n+1)*p) satisfies the lonely condition for runner v iff
//   min(a*v mod Q, Q - a*v mod Q) >= p   (where Q = (n+1)*p)
// because the threshold 1/(n+1) = p/Q in the same integer units.
//
// We iterate primes in ascending order and return the first solution found.
// Using long long for the inner multiply avoids overflow for large speeds.
std::optional<PrimeModResult> prime_modular_method(std::span<const int> speeds,
                                                   int max_prime) {
    const int n = static_cast<int>(speeds.size());
    if (n == 0) return std::nullopt;

    // prime_sieve(k) returns all primes strictly less than k
    const auto primes = prime_sieve(max_prime + 1);

    for (int p : primes) {
        const long Q = static_cast<long>(n + 1) * p;

        for (long a = 1; a < Q; ++a) {
            bool valid = true;
            for (int v : speeds) {
                long pos  = static_cast<long>(
                    (static_cast<long long>(a) * v) % Q);
                long dist = std::min(pos, Q - pos);
                if (dist * (n + 1) < Q) { valid = false; break; }
            }
            if (valid) return PrimeModResult{true, p, static_cast<int>(a)};
        }
    }

    return std::nullopt;
}
