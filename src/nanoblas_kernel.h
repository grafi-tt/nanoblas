#ifndef NANOBLAS_NANOBLAS_KERNEL_H
#define NANOBLAS_NANOBLAS_KERNEL_H

#include <stdint.h>
#include "nanoblas_types.h"
#include "nanoblas_prepack.h"

struct nanoblas_f32_kernel_state_t {
	const float *a_pack_cur;
	const float *b_pack_cur;
	float *c_buf;
	float *c_cur;
	float *c_next_cur;
	const ptrdiff_t ldc;
	int m_slice_real_len;
	int n_slice_real_len;
	int m_next_slice_real_len;
	int n_next_slice_real_len;
	int k_len;
	int current_prepack;
	float *a_pack;
	float *a_next_pack;
	float *b_pack;
	float *b_next_pack;
	union {
		struct {
			nanoblas_f32_prepack_state_t a;
			nanoblas_f32_prepack_state_t b;
		} sel;
		struct {
			const float *a_next_cur;
			float *a_next_pack_cur;
			ptrdiff_t interval_m;
			ptrdiff_t interval_k_in_a;
			const int m_slice_len;
			int a_dummy_int[5];
			const float *a_dummy_ptr;
			const float *b_next_cur;
			float *b_next_pack_cur;
			const ptrdiff_t interval_n;
			const ptrdiff_t interval_k_in_b;
			const int n_slice_len;
			int b_dummy_int[5];
			const float *b_dummy_ptr;
		} mem;
	} prepack;
};

struct _nanoblas_f32_kernel_state_former_t {
	const float *a_pack_cur;
	const float *b_pack_cur;
	float *c_buf;
	float *c_cur;
	float *c_next_cur;
	const ptrdiff_t ldc;
	int m_slice_real_len;
	int n_slice_real_len;
	int m_next_slice_real_len;
	int n_next_slice_real_len;
	int k_len;
	int current_prepack;
	float *a_pack;
	float *a_next_pack;
	float *b_pack;
	float *b_next_pack;
};

static const int nanoblas_f32_current_prepack_a =
	sizeof(struct _nanoblas_f32_kernel_state_former_t);

static const int nanoblas_f32_current_prepack_b =
	sizeof(struct _nanoblas_f32_kernel_state_former_t) + sizeof(nanoblas_f32_prepack_state_t);

static inline nanoblas_f32_prepack_state_t *
nanoblas_f32_current_prepack_p(nanoblas_f32_kernel_state_t *st) {
	return st->current_prepack == 0 ?
		NULL : (nanoblas_f32_prepack_state_t *)((char *)st + st->current_prepack);
}

struct nanoblas_f64_kernel_state_t {
	const double *a_pack_cur;
	const double *b_pack_cur;
	double *c_buf;
	double *c_cur;
	double *c_next_cur;
	const ptrdiff_t ldc;
	int m_slice_real_len;
	int n_slice_real_len;
	int m_next_slice_real_len;
	int n_next_slice_real_len;
	int k_len;
	int current_prepack;
	double *a_pack;
	double *a_next_pack;
	double *b_pack;
	double *b_next_pack;
	union {
		struct {
			nanoblas_f64_prepack_state_t a;
			nanoblas_f64_prepack_state_t b;
		} sel;
		struct {
			const double *a_next_cur;
			double *a_next_pack_cur;
			ptrdiff_t interval_m;
			ptrdiff_t interval_k_in_a;
			const int m_slice_len;
			int a_dummy_int[5];
			const double *a_dummy_ptr;
			const double *b_next_cur;
			double *b_next_pack_cur;
			ptrdiff_t interval_n;
			ptrdiff_t interval_k_in_b;
			const int n_slice_len;
			int b_dummy_int[5];
			const double *b_dummy_ptr;
		} mem;
	} prepack;
};

struct _nanoblas_f64_kernel_state_former_t {
	const double *a_pack_cur;
	const double *b_pack_cur;
	double *c_buf;
	double *c_cur;
	double *c_next_cur;
	const ptrdiff_t ldc;
	int m_slice_real_len;
	int n_slice_real_len;
	int m_next_slice_real_len;
	int n_next_slice_real_len;
	int k_len;
	int current_prepack;
	double *a_pack;
	double *a_next_pack;
	double *b_pack;
	double *b_next_pack;
};

static const int nanoblas_f64_current_prepack_a =
	sizeof(struct _nanoblas_f64_kernel_state_former_t);

static const int nanoblas_f64_current_prepack_b =
	sizeof(struct _nanoblas_f64_kernel_state_former_t) + sizeof(nanoblas_f64_prepack_state_t);

static inline nanoblas_f64_prepack_state_t *
nanoblas_f64_current_prepack_p(nanoblas_f64_kernel_state_t *st) {
	return st->current_prepack == 0 ?
		NULL : (nanoblas_f64_prepack_state_t *)((uintptr_t)st + st->current_prepack);
}

#endif
