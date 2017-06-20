#include "kernel/x86_64/avx_kernel.h"
#include "internal/macro.h"

kernel_t nanoblas_f32_avx_kernel = {
	.fun = &nanoblas_f32_avx_kernel_fun,
	.m_slice_len = 12,
	.n_slice_len = 8,
};
