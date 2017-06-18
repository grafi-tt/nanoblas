#ifndef NANOBLAS_NANOBLAS_PREPACK_H
#define NANOBLAS_NANOBLAS_PREPACK_H

#include "nanoblas_types.h"

struct f32_prepack_state_t {
	const float *next_cur;
	float *next_pack_cur;
	int mn_sched_len;
	int k_sched_len;
	int mn_packed_len;
	int k_packed_len;
	int mn_next_len;
	int k_next_len;
	const ptrdiff_t interval_mn;
	const ptrdiff_t interval_k;
	const int unit_len;
};

struct f64_prepack_state_t {
	const double *next_cur;
	double *next_pack_cur;
	int mn_sched_len;
	int k_sched_len;
	int mn_packed_len;
	int k_packed_len;
	int mn_next_len;
	int k_next_len;
	const ptrdiff_t interval_mn;
	const ptrdiff_t interval_k;
	const int unit_len;
};

#endif
