#ifndef NANOBLAS_KERNEL_X86_64_AVX_KERNEL_H
#define NANOBLAS_KERNEL_X86_64_AVX_KERNEL_H

#include "kernel.h"

#define avx_kernel APPEND_FTYPE(avx_kernel)
void avx_kernel(
	const FTYPE *restrict a_pack, const FTYPE *restrict b_pack,
	int k_sched_len, int m_sched_len, size_t interval_k_in_a, size_t interval_m,
	FTYPE *restrict a_next_pack, const FTYPE *restrict a_next,
	size_t k_len_szt, FTYPE* restrict c, size_t ldc);

#endif
