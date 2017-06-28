#ifndef NANOBLAS_NANOBLAS_TYPES_H
#define NANOBLAS_NANOBLAS_TYPES_H

#include <stddef.h>

struct nanoblas_f32_prepack_state_t;
typedef struct nanoblas_f32_prepack_state_t nanoblas_f32_prepack_state_t;
struct nanoblas_f64_prepack_state_t;
typedef struct nanoblas_f64_prepack_state_t nanoblas_f64_prepack_state_t;

struct nanoblas_f32_kernel_state_t;
typedef struct nanoblas_f32_kernel_state_t nanoblas_f32_kernel_state_t;
struct nanoblas_f64_kernel_state_t;
typedef struct nanoblas_f64_kernel_state_t nanoblas_f64_kernel_state_t;

typedef void nanoblas_f32_kernel_mult_t(nanoblas_f32_kernel_state_t *);
typedef void nanoblas_f64_kernel_mult_t(nanoblas_f64_kernel_state_t *);
typedef void nanoblas_f32_kernel_pack_t(nanoblas_f32_prepack_state_t *);
typedef void nanoblas_f64_kernel_pack_t(nanoblas_f64_prepack_state_t *);

/**
 * \addtogroup NANOBLAS
 * @{ */

typedef struct nanoblas_f32_kernel_t {
	nanoblas_f32_kernel_mult_t *mult;
	nanoblas_f32_kernel_pack_t *pack;
	int m_slice_len;
	int n_slice_len;
	int k_unit_len;
} nanoblas_f32_kernel_t;
typedef struct nanoblas_f64_kernel_t {
	nanoblas_f64_kernel_mult_t *mult;
	nanoblas_f64_kernel_pack_t *pack;
	int m_slice_len;
	int n_slice_len;
	int k_unit_len;
} nanoblas_f64_kernel_t;

/** @} */

#endif
