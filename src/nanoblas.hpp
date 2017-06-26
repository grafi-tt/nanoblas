#ifndef NANOBLAS_NANOBLAS_HPP
#define NANOBLAS_NANOBLAS_HPP

#include "nanoblas.h"

namespace nanoblas {

template<typename FTYPE>
void gemm(const nanoblas_t &nb,
		enum CBLAS_ORDER Order, enum CBLAS_TRANSPOSE TransA, enum CBLAS_TRANSPOSE TransB,
		size_t M, size_t N, size_t K,
		FTYPE alpha, const FTYPE *A, ptrdiff_t lda, const FTYPE *B, ptrdiff_t ldb,
		FTYPE beta, FTYPE *C, ptrdiff_t ldc);

template<>
void gemm<float>(const nanoblas_t &nb,
		enum CBLAS_ORDER Order, enum CBLAS_TRANSPOSE TransA, enum CBLAS_TRANSPOSE TransB,
		size_t M, size_t N, size_t K,
		float alpha, const float *A, ptrdiff_t lda, const float *B, ptrdiff_t ldb,
		float beta, float *C, ptrdiff_t ldc) {
	nanoblas_sgemm(&nb, Order, TransA, TransB, M, N, K, alpha, A, lda, B, ldb, beta, C, ldc);
}

template<>
void gemm<double>(const nanoblas_t &nb,
		enum CBLAS_ORDER Order, enum CBLAS_TRANSPOSE TransA, enum CBLAS_TRANSPOSE TransB,
		size_t M, size_t N, size_t K,
		double alpha, const double *A, ptrdiff_t lda, const double *B, ptrdiff_t ldb,
		double beta, double *C, ptrdiff_t ldc) {
	nanoblas_dgemm(&nb, Order, TransA, TransB, M, N, K, alpha, A, lda, B, ldb, beta, C, ldc);
}

}

#endif
