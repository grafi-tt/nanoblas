#ifndef NANOBLAS_NANOBLAS_H
#define NANOBLAS_NANOBLAS_H

#include "cblas_enum.h"
#include "nanoblas_types.h"

typedef struct nanoblas_t {
	nanoblas_f32_kernel_t f32_kernel;
	nanoblas_f64_kernel_t f64_kernel;
	size_t f32_blk_m_len;
	size_t f32_blk_n_len;
	size_t f32_blk_k_len;
	size_t f64_blk_m_len;
	size_t f64_blk_n_len;
	size_t f64_blk_k_len;
} nanoblas_t;

#ifdef __cplusplus
extern "C" {
#endif

void nanoblas_init(nanoblas_t *nb);

/*
 * ===========================================================================
 * Prototypes for level 3 BLAS
 * ===========================================================================
 */

/* 
 * Routines with standard 4 prefixes (S, D, C, Z)
 */
void nanoblas_sgemm(const nanoblas_t *nb,
		const enum CBLAS_ORDER Order, const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_TRANSPOSE TransB,
		const size_t M, const size_t N, const size_t K,
		const float alpha, const float *A, const ptrdiff_t lda, const float *B, const ptrdiff_t ldb,
		const float beta, float *C, const ptrdiff_t ldc);

void nanoblas_dgemm(const nanoblas_t *nb,
		const enum CBLAS_ORDER Order, const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_TRANSPOSE TransB,
		const size_t M, const size_t N, const size_t K,
		const double alpha, const double *A, const ptrdiff_t lda, const double *B, const ptrdiff_t ldb,
		const double beta, double *C, const ptrdiff_t ldc);

#ifdef __cplusplus
}
#endif

#endif
