#ifndef NANOBLAS_KERNEL_X86_64_AVX_KERNEL_H
#define NANOBLAS_KERNEL_X86_64_AVX_KERNEL_H

#include "nanoblas_types.h"

#ifdef __cplusplus
extern "C" {
#endif

nanoblas_f32_kernel_fun_t nanoblas_f32_avx_kernel_fun;
extern nanoblas_f32_kernel_t nanoblas_f32_avx_kernel;

#ifdef __cplusplus
}
#endif

#endif
