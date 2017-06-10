#ifndef NANOBLAS_KERNEL_GENERIC_KERNEL_H
#define NANOBLAS_KERNEL_GENERIC_KERNEL_H

#include "kernel.h"

#define generic_kernel APPEND_FTYPE(generic_kernel)
void generic_kernel(
		const FTYPE *restrict a_pack, const FTYPE *restrict b_pack,
		int k_len, int m_sub_len, int n_sub_len,
		FTYPE* restrict c, ptrdiff_t ldc, const sched_state_t *st);

#endif
