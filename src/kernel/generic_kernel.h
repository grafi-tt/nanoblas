#ifndef NANOBLAS_KERNEL_GENERIC_KERNEL_H
#define NANOBLAS_KERNEL_GENERIC_KERNEL_H

#include "nanoblas_types.h"

#ifdef __cplusplus
extern "C" {
#endif

f32_kernel_fun_t f32_generic_kernel_fun_4;
f32_kernel_fun_t f32_generic_kernel_fun_8;
f64_kernel_fun_t f64_generic_kernel_fun_4;
f64_kernel_fun_t f64_generic_kernel_fun_8;

f32_kernel_t f32_generic_kernel_4;
f32_kernel_t f32_generic_kernel_8;
f64_kernel_t f64_generic_kernel_4;
f64_kernel_t f64_generic_kernel_8;

#ifdef __cplusplus
}
#endif

#endif
