#ifndef NANOBLAS_KERNEL_X86_64_AVX_KERNEL_H
#define NANOBLAS_KERNEL_X86_64_AVX_KERNEL_H

#include "nanoblas_types.h"

#ifdef __cplusplus
extern "C" {
#endif

f32_kernel_fun_t f32_avx_kernel_fun;
/* f64_kernel_fun_t f64_avx_kernel_fun; */

f32_kernel_t f32_avx_kernel;
/* f64_kernel_t f64_avx_kernel; */

#ifdef __cplusplus
}
#endif

#endif
