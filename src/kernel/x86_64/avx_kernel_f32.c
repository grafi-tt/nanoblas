#include "kernel/x86_64/avx_kernel.h"
#include "internal/macro.h"

void nanoblas_f32_avx_kernel_fun(kernel_state_t *kernel_st, prepack_state_t *prepack_st) {
	/* to support both of Windows and SysV calling conventions */
	nanoblas_f32_avx_kernel_asm(kernel_st, prepack_st, prepack_st, kernel_st);
}

kernel_t nanoblas_f32_avx_kernel = {
	.fun = &nanoblas_f32_avx_kernel_fun,
	.m_slice_len = 12,
	.n_slice_len = 8,
};
