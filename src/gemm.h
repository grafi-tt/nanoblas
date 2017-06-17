#ifndef NANOBLAS_GEMM_H
#define NANOBLAS_GEMM_H

#include <stddef.h>

void sgemm(
		char transa, char transb, size_t m, size_t n, size_t k,
		float alpha, float *restrict a, ptrdiff_t lda,
		float *restrict b, ptrdiff_t ldb,
		float beta, float *restrict c, ptrdiff_t ldc);

void dgemm(
		char transa, char transb, size_t m, size_t n, size_t k,
		double alpha, double *restrict a, ptrdiff_t lda,
		double *restrict b, ptrdiff_t ldb,
		double beta, double *restrict c, ptrdiff_t ldc);

#endif
