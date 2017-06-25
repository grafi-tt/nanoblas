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

typedef void nanoblas_f32_kernel_fun_t(nanoblas_f32_kernel_state_t *);
typedef void nanoblas_f64_kernel_fun_t(nanoblas_f64_kernel_state_t *);
typedef void nanoblas_f32_pack_fun_t(nanoblas_f32_prepack_state_t *);
typedef void nanoblas_f64_pack_fun_t(nanoblas_f64_prepack_state_t *);
typedef void nanoblas_f32_max_sched_len_fun_t(int k_len, int *a_max_sched_len, int *b_max_sched_len);
typedef void nanoblas_f64_max_sched_len_fun_t(int k_len, int *a_max_sched_len, int *b_max_sched_len);

/**
 * \addtogroup NANOBLAS
 * @{ */

typedef struct nanoblas_f32_kernel_t {
	nanoblas_f32_kernel_fun_t        *kernel_fun;
	nanoblas_f32_pack_fun_t          *pack_fun;
	nanoblas_f32_max_sched_len_fun_t *max_sched_len_fun;
	int m_slice_len;
	int n_slice_len;
} nanoblas_f32_kernel_t;
typedef struct nanoblas_f64_kernel_t {
	nanoblas_f64_kernel_fun_t        *kernel_fun;
	nanoblas_f64_pack_fun_t          *pack_fun;
	nanoblas_f64_max_sched_len_fun_t *max_sched_len_fun;
	int m_slice_len;
	int n_slice_len;
} nanoblas_f64_kernel_t;

/** @} */

#endif
