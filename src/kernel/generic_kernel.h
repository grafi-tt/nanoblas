#ifndef NANOBLAS_KERNEL_GENERIC_KERNEL_H
#define NANOBLAS_KERNEL_GENERIC_KERNEL_H

#include "nanoblas_types.h"

#ifdef __cplusplus
extern "C" {
#endif

nanoblas_f32_kernel_fun_t nanoblas_f32_generic_kernel_fun_4x4;
nanoblas_f32_kernel_fun_t nanoblas_f32_generic_kernel_fun_6x4;
nanoblas_f64_kernel_fun_t nanoblas_f64_generic_kernel_fun_4x4;
nanoblas_f64_kernel_fun_t nanoblas_f64_generic_kernel_fun_6x4;

nanoblas_f32_kernel_t nanoblas_f32_generic_kernel_4x4;
nanoblas_f32_kernel_t nanoblas_f32_generic_kernel_6x4;
nanoblas_f64_kernel_t nanoblas_f64_generic_kernel_4x4;
nanoblas_f64_kernel_t nanoblas_f64_generic_kernel_6x4;

#ifdef __cplusplus
}
#endif

#endif