#ifndef NANOBLAS_NANOBLAS_PREPACK_H
#define NANOBLAS_NANOBLAS_PREPACK_H

#include "nanoblas_types.h"

struct nanoblas_f32_prepack_state_t {
	const float *next_cur;
	float *next_pack_cur;
	int sched_size;
	int slice_size;
	int pos_in_width;
	int slice_width;
	int slice_packed_size;
	int remain_blk_size;
	const ptrdiff_t interval_mn;
	const ptrdiff_t interval_k;
	const int sched_max_size;
	const int unit_len;
};

struct nanoblas_f64_prepack_state_t {
	const double *next_cur;
	double *next_pack_cur;
	int sched_size;
	int slice_size;
	int pos_in_width;
	int slice_width;
	int slice_packed_size;
	int remain_blk_size;
	const ptrdiff_t interval_mn;
	const ptrdiff_t interval_k;
	const int sched_max_size;
	const int unit_len;
};

#endif
