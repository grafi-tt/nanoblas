#ifndef NANOBLAS_NANOBLAS_PREPACK_H
#define NANOBLAS_NANOBLAS_PREPACK_H

#include "nanoblas_types.h"

struct nanoblas_f32_prepack_state_t {
	/* read/written by kernel */
	const float *next_cur;
	float *next_pack_cur;
	int mn_slice_pos;
	/* read by kernel */
	int sched_size;
	int mn_slice_len_real;
	const int mn_slice_len_virtual;
	const ptrdiff_t interval_mn;
	const ptrdiff_t proceed_k;
	/* internal */
	const float *next_cur_bak;
	int slice_packed_size;
	int mn_len_remained;
	int slice_size;
	int max_sched_size;
	const ptrdiff_t interval_mn_slice;
};

struct nanoblas_f64_prepack_state_t {
	/* read/written by kernel */
	const double *next_cur;
	double *next_pack_cur;
	int mn_slice_pos;
	/* read by kernel */
	int sched_size;
	int mn_slice_len_real;
	const int mn_slice_len_virtual;
	const ptrdiff_t interval_mn;
	const ptrdiff_t proceed_k;
	/* internal */
	const double *next_cur_bak;
	int slice_packed_size;
	int mn_len_remained;
	int slice_size;
	int max_sched_size;
	const ptrdiff_t interval_mn_slice;
};

#endif
