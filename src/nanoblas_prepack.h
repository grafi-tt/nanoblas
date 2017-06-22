#ifndef NANOBLAS_NANOBLAS_PREPACK_H
#define NANOBLAS_NANOBLAS_PREPACK_H

#include "nanoblas_types.h"

struct nanoblas_f32_prepack_state_t {
	/* read/written by kernel */
	const float *next;
	float *next_pack_cur;
	int mn_slice_pos;
	/* read by kernel */
	int sched_size;
	int mn_slice_real_len;
	const int mn_slice_len;
	const ptrdiff_t interval_mn;
	const ptrdiff_t proceed_k;
	/* internal */
	int packed_size;
	int size;
	int max_sched_size;
};

struct nanoblas_f64_prepack_state_t {
	/* read/written by kernel */
	const double *next;
	double *next_pack_cur;
	int mn_slice_pos;
	/* read by kernel */
	int sched_size;
	int mn_slice_real_len;
	const int mn_slice_len;
	const ptrdiff_t interval_mn;
	const ptrdiff_t proceed_k;
	/* internal */
	int packed_size;
	int size;
	int max_sched_size;
};

#endif
