#ifndef NANOBLAS_TEST_TEST_GEMM_HPP
#define NANOBLAS_TEST_TEST_GEMM_HPP

#include <cstdint>
#include <functional>
#include <iostream>
#include <random>
#include <vector>
#include "lib/util.hpp"

namespace nanoblas {

template<typename FTYPE>
using impl_t = std::function<void(const FTYPE *, const FTYPE *, FTYPE *, size_t, size_t, size_t, size_t, size_t, size_t)>;

template<typename FTYPE>
bool run_test(std::mt19937 gen, const impl_t<FTYPE> &impl, size_t M, size_t N, size_t K, size_t lda, size_t ldb, size_t ldc) {
	std::uniform_real_distribution<FTYPE> dist(0, 1);

	std::vector<FTYPE> A((M+2)*lda);
	std::vector<FTYPE> B((K+2)*ldb);
	std::vector<FTYPE> C((M+2)*ldc);
	std::vector<FTYPE> D((M+2)*ldc);

	for (auto &&v: A) v = dist(gen);
	for (auto &&v: B) v = dist(gen);
	std::mt19937 gen2 = gen;
	for (auto &&v: C) v = dist(gen);
	for (auto &&v: D) v = dist(gen2);

	for (size_t m = 0; m < M; m++)
		for (size_t n = 0; n < N; n++)
			for (size_t k = 0; k < K; k++)
				C[ldc*(m+1)+n] += A[lda*(m+1)+k] * B[ldb*(k+1)+n];

	impl(A.data()+lda, B.data()+ldb, D.data()+ldc, M, N, K, lda, ldb, ldc);

	for (size_t i = 0; i < M+2; i++) {
		for (size_t j = 0; j < N; j++) {
			FTYPE x = C[ldc*i+j];
			FTYPE y = D[ldc*i+j];
			if (!mostly_equal(x, y)) {
				std::cerr <<
					"C["<<(long long)i-1<<"]["<<j<<"] = "<<x<<"; " <<
					"D["<<(long long)i-1<<"]["<<j<<"] = "<<y<<";" << std::endl;
				return true;
			}
		}
	}

	return false;
}

template<typename FTYPE>
bool run_test(std::mt19937 gen, const impl_t<FTYPE> &impl, size_t M, size_t N, size_t K) {
	return run_test(gen, impl, M, N, K, K, N, N);
}

}

#endif
