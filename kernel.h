#ifndef NANOBLAS_KERNEL_H
#define NANOBLAS_KERNEL_H

#include <stddef.h>
#include "const.h"

typedef void (*kernel_t)(
	int k_len, FTYPE *restrict a_pack, FTYPE *restrict b_pack,
	int k_len_sched, int m_len_sched, int trans_a, size_t lda,
	FTYPE *restrict a_pack_next, FTYPE *restrict a,
	size_t ldc, FTYPE* restrict c) {

kernel_t *decide_kernel();

#define LOOP_N_TO_PACK_ONE_ROW (BLK_LEN / UNIT_LEN)

#endif
