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

typedef void nanoblas_f32_kernel_fun_t(nanoblas_f32_kernel_state_t *, nanoblas_f32_prepack_state_t *);
typedef void nanoblas_f64_kernel_fun_t(nanoblas_f64_kernel_state_t *, nanoblas_f64_prepack_state_t *);

typedef struct nanoblas_f32_kernel_t {
	nanoblas_f32_kernel_fun_t *fun;
	int m_slice_len;
	int n_slice_len;
} nanoblas_f32_kernel_t;
typedef struct nanoblas_f64_kernel_t {
	nanoblas_f64_kernel_fun_t *fun;
	int m_slice_len;
	int n_slice_len;
} nanoblas_f64_kernel_t;

#endif
