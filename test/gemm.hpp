#ifndef NANOBLAS_TEST_GEMM_HPP
#define NANOBLAS_TEST_GEMM_HPP

#include <cstdint>
#include <chrono>
#include <functional>
#include <iostream>
#include <random>
#include <vector>
#include "lib/util.hpp"
#include "lib/reference_kernel.hpp"

template <typename FTYPE>
using impl_t = std::function<void(const FTYPE *, const FTYPE *, FTYPE *, size_t, size_t, size_t)>;

template<typename FTYPE>
bool run_test(std::mt19937 gen, const impl_t<FTYPE> &impl, size_t M, size_t N, size_t K) {
	std::uniform_real_distribution<FTYPE> dist(0, 1);

	std::vector<FTYPE> A(M*K);
	std::vector<FTYPE> B(K*N);
	std::vector<FTYPE> C(M*N);
	std::vector<FTYPE> D(M*N);

	for (auto &&v: A) v = dist(gen);
	for (auto &&v: B) v = dist(gen);

	auto t1s = std::chrono::high_resolution_clock::now();
	for (size_t m = 0; m < M; m++)
		for (size_t n = 0; n < N; n++)
			for (size_t k = 0; k < K; k++)
				C[N*m+n] += A[K*m+k] * B[N*k+n];
	auto t1e = std::chrono::high_resolution_clock::now();
	auto d1 = t1e - t1s;
	std::cout << "naive " <<
		std::chrono::duration_cast<std::chrono::milliseconds>(d1).count() << std::endl;

	auto t2s = std::chrono::high_resolution_clock::now();
	impl(A.data(), B.data(), D.data(), M, N, K);
	auto t2e = std::chrono::high_resolution_clock::now();
	auto d2 = t2e - t2s;
	std::cout << "impl " <<
		std::chrono::duration_cast<std::chrono::milliseconds>(d2).count() << std::endl;

	for (size_t i = 0; i < M; i++) {
		for (size_t j = 0; j < N; j++) {
			FTYPE x = C[N*i+j];
			FTYPE y = D[N*i+j];
			if (!mostly_equal(x, y)) {
				std::cerr <<
					"C["<<i<<"]["<<j<<"] = "<<x<<"; " <<
					"D["<<i<<"]["<<j<<"] = "<<y<<";" << std::endl;
				return true;
			}
		}
	}

	return false;
}

#endif
