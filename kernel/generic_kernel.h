#ifndef NANOBLAS_KERNEL_GENERIC_KERNEL_H
#define NANOBLAS_KERNEL_GENERIC_KERNEL_H

#include "kernel.h"

#define generic_kernel APPEND_FTYPE(generic_kernel)
void generic_kernel(
	const FTYPE *restrict a_pack, const FTYPE *restrict b_pack,
	int k_sched_len, int m_sched_len, size_t interval_k_in_a, size_t interval_m,
	FTYPE *restrict a_next_pack, const FTYPE *restrict a_next,
	size_t k_len_szt, FTYPE* restrict c, size_t ldc);

#endif
