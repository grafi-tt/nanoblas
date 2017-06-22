#ifndef NANOBLAS_NANOBLAS_KERNEL_H
#define NANOBLAS_NANOBLAS_KERNEL_H

#include "nanoblas_types.h"

struct nanoblas_f32_kernel_state_t {
	const float *a_pack_cur;
	const float *b_pack_cur;
	float *c_cur;
	const ptrdiff_t ldc;
	int m_slice_real_len;
	int n_slice_real_len;
	int k_len;
};

struct nanoblas_f64_kernel_state_t {
	const double *a_pack_cur;
	const double *b_pack_cur;
	double *c_cur;
	const ptrdiff_t ldc;
	int m_slice_real_len;
	int n_slice_real_len;
	int k_len;
};

#endif
