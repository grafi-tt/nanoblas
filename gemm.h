#ifndef NANOBLAS_GEMM_H
#define NANOBLAS_GEMM_H

#include <stddef.h>
#include "const.h"

#define gemm ADD_PREFIX(gemm)
void gemm(
		char transa, char transb, size_t m, size_t n, size_t k,
		FTYPE alpha, FTYPE *restrict a, ptrdiff_t lda,
		FTYPE *restrict b, ptrdiff_t ldb,
		FTYPE beta, FTYPE *restrict c, ptrdiff_t ldc);

#endif
