#ifndef NANOBLAS_INTERNAL_PREPACK_H
#define NANOBLAS_INTERNAL_PREPACK_H

#include "nanoblas_types.h"
#include "internal/macro.h"
#include "internal/util.h"

#ifdef __cplusplus
extern "C" {
#endif

#define prepack_state_new JOIN(NAMESPACE, PREFIX, prepack_state_new)
static inline prepack_state_t prepack_state_new(
		const FTYPE *next, FTYPE *next_pack,
		int mn_len, int k_len, int slice_len, int max_sched_size,
		ptrdiff_t interval_mn, ptrdiff_t interval_k) {
	prepack_state_t st = {
		.next_cur             = next,
		.next_pack_cur        = next_pack,
		.mn_slice_pos         = 0,
		.sched_size           = imin(max_sched_size, slice_len*k_len),
		.mn_slice_len_real    = imin(slice_len, mn_len),
		.mn_slice_len_virtual = slice_len,
		.interval_mn          = interval_mn,
		.proceed_k            = interval_k - slice_len*interval_mn,
		.next_cur_bak         = next,
		.slice_packed_size    = 0,
		.mn_len_remained      = mn_len,
		.slice_size           = slice_len*k_len,
		.max_sched_size       = max_sched_size,
		.interval_mn_slice    = slice_len*interval_mn,
	};
	return st;
}

#define restart_prepack JOIN(NAMESPACE, PREFIX, start_prepack)
static inline void restart_prepack(prepack_state_t *st,
		const FTYPE *next, int mn_len, int k_len, int max_sched_size) {
	st->next_cur = next;
	st->next_cur_bak = next;
	st->mn_len_remained = mn_len;
	st->mn_slice_len_real = imin(st->mn_slice_len_virtual, st->mn_len_remained);
	st->slice_size = st->mn_slice_len_virtual*k_len;
	st->max_sched_size = max_sched_size;
	st->sched_size = imin(st->max_sched_size, st->slice_size);
}

#define step_prepack JOIN(NAMESPACE, PREFIX, step_prepack)
int step_prepack(prepack_state_t *st);

#define pack_slice JOIN(NAMESPACE, PREFIX, pack_slice)
void pack_slice(prepack_state_t *st);

#define pack_all JOIN(NAMESPACE, PREFIX, pack_all)
void pack_all(prepack_state_t *st);

#ifdef __cplusplus
}
#endif

#endif
