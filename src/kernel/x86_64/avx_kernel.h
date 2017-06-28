#ifndef NANOBLAS_KERNEL_X86_64_AVX_KERNEL_H
#define NANOBLAS_KERNEL_X86_64_AVX_KERNEL_H

#include "nanoblas_types.h"

#ifdef __cplusplus
extern "C" {
#endif

nanoblas_f32_kernel_mult_t nanoblas_f32_avx_kernel_mult;
nanoblas_f32_kernel_pack_t nanoblas_f32_avx_kernel_pack;

const nanoblas_f32_kernel_t nanoblas_f32_avx_kernel = {
	&nanoblas_f32_avx_kernel_mult,
	&nanoblas_f32_avx_kernel_pack,
	8, 8, 8
};

#ifdef __cplusplus
}
#endif

#endif
