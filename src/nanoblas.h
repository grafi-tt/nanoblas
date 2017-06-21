/**
 * @file nanoblas.h
 * @brief Nanoblas API definition
 */

#ifndef NANOBLAS_NANOBLAS_H
#define NANOBLAS_NANOBLAS_H

#include "cblas_enum.h"
#include "nanoblas_types.h"

/**
 * @defgroup NANOBLAS   Nanoblas data structure
 * \{ */

/**
 * @brief Definition of structs that contain all nanoblas's parameters.
 *
 * The struct can be initialized by nanoblas_init() function,
 * with automatically chosen parameters depending on the machine.
 * You can also change the parameters manually.
 *
 * If you create more than one structs, you can simultaneously use
 * multiple nanoblas with different configuration.
 */
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

/**
 * @brief Initalize nanoblas, with parameters suitable to your machine.
 */
void nanoblas_init(nanoblas_t *nb);

/** \} */

/**
 * @defgroup L3BLAS     Level3 BLAS functions
 * \{ */

/**
 * @defgroup GEMM       GEMM – General Matrix-Matrix Multiplication
 * \{ */

/**
 * \brief Compute @f$ C = \alpha AB + \beta C @f$ for single-precision floating point input.
 */
void nanoblas_sgemm(const nanoblas_t *nb,
		enum CBLAS_ORDER Order, enum CBLAS_TRANSPOSE TransA, enum CBLAS_TRANSPOSE TransB,
		size_t M, size_t N, size_t K,
		float alpha, const float *A, ptrdiff_t lda, const float *B, ptrdiff_t ldb,
		float beta, float *C, ptrdiff_t ldc);

/**
 * @brief Compute @f$ C = \alpha AB + \beta C @f$ for double-precision floating point input.
 */
void nanoblas_dgemm(const nanoblas_t *nb,
		enum CBLAS_ORDER Order, enum CBLAS_TRANSPOSE TransA, enum CBLAS_TRANSPOSE TransB,
		size_t M, size_t N, size_t K,
		double alpha, const double *A, ptrdiff_t lda, const double *B, ptrdiff_t ldb,
		double beta, double *C, ptrdiff_t ldc);

/** \} */

/** \} */

#ifdef __cplusplus
}
#endif

#endif
