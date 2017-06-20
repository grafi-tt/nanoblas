#ifndef NANOBLAS_NANOBLAS_H
#define NANOBLAS_NANOBLAS_H

#include "cblas_enum.h"
#include "nanoblas_types.h"

typedef struct nanoblas_t {
	nanoblas_f32_kernel_t f32_kernel;
	nanoblas_f64_kernel_t f64_kernel;
	size_t f32_blk_n_max_len;
	size_t f32_blk_k_max_len;
	size_t f64_blk_n_max_len;
	size_t f64_blk_k_max_len;
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
		enum CBLAS_ORDER Order, enum CBLAS_TRANSPOSE TransA, enum CBLAS_TRANSPOSE TransB,
		size_t M, size_t N, size_t K,
		float alpha, const float *A, ptrdiff_t lda, const float *B, ptrdiff_t ldb,
		float beta, float *C, ptrdiff_t ldc);

void nanoblas_dgemm(const nanoblas_t *nb,
		enum CBLAS_ORDER Order, enum CBLAS_TRANSPOSE TransA, enum CBLAS_TRANSPOSE TransB,
		size_t M, size_t N, size_t K,
		double alpha, const double *A, ptrdiff_t lda, const double *B, ptrdiff_t ldb,
		double beta, double *C, ptrdiff_t ldc);

#ifdef __cplusplus
}
#endif

#endif
