#ifndef NANOBLAS_KERNEL_GENERIC_KERNEL_H
#define NANOBLAS_KERNEL_GENERIC_KERNEL_H

#include "nanoblas_types.h"

#ifdef __cplusplus
extern "C" {
#endif

nanoblas_f32_kernel_mult_t nanoblas_f32_generic_kernel_mult_4x4;
nanoblas_f32_kernel_mult_t nanoblas_f32_generic_kernel_mult_6x4;
nanoblas_f64_kernel_mult_t nanoblas_f64_generic_kernel_mult_4x4;
nanoblas_f64_kernel_mult_t nanoblas_f64_generic_kernel_mult_6x4;

nanoblas_f32_kernel_pack_t nanoblas_f32_generic_kernel_pack_4x4;
nanoblas_f32_kernel_pack_t nanoblas_f32_generic_kernel_pack_6x4;
nanoblas_f64_kernel_pack_t nanoblas_f64_generic_kernel_pack_4x4;
nanoblas_f64_kernel_pack_t nanoblas_f64_generic_kernel_pack_6x4;

static const nanoblas_f32_kernel_t nanoblas_f32_generic_kernel_4x4 = {
	&nanoblas_f32_generic_kernel_mult_4x4,
	&nanoblas_f32_generic_kernel_pack_4x4,
	4, 4, 1
};
static const nanoblas_f64_kernel_t nanoblas_f64_generic_kernel_4x4 = {
	&nanoblas_f64_generic_kernel_mult_4x4,
	&nanoblas_f64_generic_kernel_pack_4x4,
	4, 4, 1
};
static const nanoblas_f32_kernel_t nanoblas_f32_generic_kernel_6x4 = {
	&nanoblas_f32_generic_kernel_mult_6x4,
	&nanoblas_f32_generic_kernel_pack_6x4,
	6, 4, 1
};
static const nanoblas_f64_kernel_t nanoblas_f64_generic_kernel_6x4 = {
	&nanoblas_f64_generic_kernel_mult_6x4,
	&nanoblas_f64_generic_kernel_pack_6x4,
	6, 4, 1
};

#ifdef __cplusplus
}
#endif

#endif
