#ifndef NANOBLAS_KERNEL_H
#define NANOBLAS_KERNEL_H

#include <stddef.h>
#include "const.h"

typedef void (*kernel_t)(
	FTYPE *restrict a_pack, FTYPE *restrict b_pack,
	int k_sched_len, int m_sched_len, size_t interval_k_in_a, size_t interval_m,
	FTYPE *restrict a_next_pack, FTYPE *restrict a_next,
	/*for stack alignment*/ size_t k_len_szt, FTYPE* restrict c, size_t ldc) {

kernel_t *decide_kernel();

#endif
