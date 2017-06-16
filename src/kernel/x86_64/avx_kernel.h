#ifndef NANOBLAS_KERNEL_X86_64_AVX_KERNEL_H
#define NANOBLAS_KERNEL_X86_64_AVX_KERNEL_H

#include "kernel.h"

#define avx_kernel_fun APPEND_FTYPE(avx_kernel_fun)
void avx_kernel_fun(
		const FTYPE *restrict a_pack, const FTYPE *restrict b_pack,
		int k_len, int m_sub_len, int n_sub_len,
		FTYPE* restrict c, ptrdiff_t ldc, const sched_state_t *st);

#define avx_kernel APPEND_FTYPE(avx_kernel)
kernel_t avx_kernel = { &avx_kernel_fun,
#if USE_F64
	8
#endif
#if USE_F32
	4
#endif
};

#endif
