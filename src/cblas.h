#ifndef NANOBLAS_CBLAS_H
#define NANOBLAS_CBLAS_H

#include <stddef.h>
#include "cblas_enum.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ===========================================================================
 * Prototypes for level 3 BLAS
 * ===========================================================================
 */

/* 
 * Routines with standard 4 prefixes (S, D, C, Z)
 */
void cblas_sgemm(
		const enum CBLAS_ORDER Order, const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_TRANSPOSE TransB,
		const size_t M, const size_t N, const size_t K,
		const float alpha, const float *A, const ptrdiff_t lda, const float *B, const ptrdiff_t ldb,
		const float beta, float *C, const ptrdiff_t ldc);

void cblas_dgemm(
		const enum CBLAS_ORDER Order, const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_TRANSPOSE TransB,
		const size_t M, const size_t N, const size_t K,
		const double alpha, const double *A, const ptrdiff_t lda, const double *B, const ptrdiff_t ldb,
		const double beta, double *C, const ptrdiff_t ldc);

#ifdef __cplusplus
}
#endif

#endif
