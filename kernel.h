#ifndef NANOBLAS_KERNEL_H
#define NANOBLAS_KERNEL_H

#include <stddef.h>
#include "config.h"

typedef void (*kernel_t)(
	int k_len, int next_k_len, int next_m_len, int next_k_cnt, int next_m_cnt,
	int transa, FTYPE *restrict a_pack_next, size_t lda,
	FTYPE *restrict a, FTYPE *restrict a_pack, FTYPE *restrict b_pack,
	size_t ldc, FTYPE* restrict c_pack);

kernel_t *decide_kernel();

#endif
