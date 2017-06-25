#ifndef NANOBLAS_NANOBLAS_PREPACK_H
#define NANOBLAS_NANOBLAS_PREPACK_H

#include "nanoblas_types.h"

struct nanoblas_f32_prepack_state_t {
	/* read/written by kernel */
	const float *next_cur;
	float *next_pack_cur;
	/* read by kernel */
	ptrdiff_t interval_mn;
	ptrdiff_t interval_k;
	const int slice_len;
	int next_slice_real_len;
	int sched_len;
	/* internal */
	int len;
	int packed_len;
	int remained_next_slice_len;
	int max_sched_len;
	int dummy;
	const float *next_bak;
};

struct nanoblas_f64_prepack_state_t {
	/* read/written by kernel */
	const double *next_cur;
	double *next_pack_cur;
	/* read by kernel */
	ptrdiff_t interval_mn;
	ptrdiff_t interval_k;
	const int slice_len;
	int next_slice_real_len;
	int sched_len;
	/* internal */
	int len;
	int packed_len;
	int max_sched_len;
	int remained_next_slice_len;
	int dummy;
	const double *next_bak;
};

#endif
