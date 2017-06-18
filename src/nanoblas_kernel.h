#ifndef NANOBLAS_NANOBLAS_KERNEL_H
#define NANOBLAS_NANOBLAS_KERNEL_H

#include "nanoblas_types.h"

struct f32_kernel_state_t {
	const float *a_pack_cur;
	const float *b_pack_cur;
	float *c_cur;
	int m_sub_len;
	int n_sub_len;
	const ptrdiff_t ldc;
	const int k_len;
};

struct f64_kernel_state_t {
	const double *a_pack_cur;
	const double *b_pack_cur;
	double *c_cur;
	int m_sub_len;
	int n_sub_len;
	const ptrdiff_t ldc;
	const int k_len;
};

#endif
