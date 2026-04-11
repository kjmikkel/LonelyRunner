#pragma once
#include "data_structures.h"
#include <optional>
#include <span>

// Search for a lonely time of the form t = a / ((n+1)*p) for primes p up
// to max_prime.  This denominator family is the one studied by Rosenfeld
// (arXiv:2509.14111) and Trakulthongchai (arXiv:2511.22427) in their proofs
// for 8–10 runners: a counterexample's speed-product must be divisible by
// primes for which no tuple covers the full range with this denominator.
//
// Here we use it in the forward direction: given a specific speed set, check
// every rational t = a / ((n+1)*p) for small primes p.  If any satisfies the
// lonely condition for all runners, it is returned immediately.
//
// Returns nullopt if no solution is found within the prime limit.  Unlike the
// geometric method this search is not exhaustive: a lonely time may exist with
// a denominator not of this form.
std::optional<PrimeModResult> prime_modular_method(std::span<const int> speeds,
                                                   int max_prime = 200);

bool is_valid(const PrimeModResult& result, std::span<const int> speeds);
