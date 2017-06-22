#ifndef NANOBLAS_INTERNAL_PREPACK_H
#define NANOBLAS_INTERNAL_PREPACK_H

#include "nanoblas_types.h"
#include "internal/macro.h"
#include "internal/util.h"

#ifdef __cplusplus
extern "C" {
#endif

#define step_prepack JOIN(NAMESPACE, PREFIX, step_prepack)
static inline int step_prepack(prepack_state_t *st) {
	st->packed_size += st->sched_size;
	if (st->packed_size == st->size) {
		st->next_cur = st->next_bak + st->interval_mn * st->mn_slice_len;
		st->next_bak = st->next_cur;
		st->packed_size = 0;
		st->sched_size = st->max_sched_size;
		return 1;
	} else {
		st->sched_size = imin(st->max_sched_size, st->size - st->packed_size);
		return 0;
	}
}

#define limit_prepack JOIN(NAMESPACE, PREFIX, limit_prepack)
static inline void limit_prepack(prepack_state_t *st, int mn_slice_real_len_limit) {
	st->mn_slice_real_len = imin(st->mn_slice_len, mn_slice_real_len_limit);
}

#define restart_prepack JOIN(NAMESPACE, PREFIX, restart_prepack)
static inline void restart_prepack(prepack_state_t *st,
		const FTYPE *next, int mn_slice_real_len_limit) {
	limit_prepack(st, mn_slice_real_len_limit);
	st->next_cur = next;
	st->next_bak = next;
}

#define reset_prepack JOIN(NAMESPACE, PREFIX, reset_prepack)
static inline void reset_prepack(prepack_state_t *st,
		const FTYPE *next, int mn_slice_real_len_limit, int k_len, int max_sched_size) {
	restart_prepack(st, next, mn_slice_real_len_limit);
	st->size = st->mn_slice_len * k_len;
	st->max_sched_size = max_sched_size;
	st->sched_size = imin(st->max_sched_size, st->size);
}

#define prepack_state_new JOIN(NAMESPACE, PREFIX, prepack_state_new)
static inline prepack_state_t prepack_state_new(
		const FTYPE *next, FTYPE *next_pack, int slice_len, int mn_slice_real_len_limit,
		int k_len, int max_sched_size, ptrdiff_t interval_mn, ptrdiff_t interval_k) {
	prepack_state_t st = {
		.next_pack_cur = next_pack,
		.mn_slice_pos = 0,
		.mn_slice_len = slice_len,
		.interval_mn = interval_mn,
		.proceed_k = interval_k - slice_len * interval_mn,
		.packed_size = 0,
	};
	reset_prepack(&st, next, mn_slice_real_len_limit, k_len, max_sched_size);
	return st;
}

#define pack_all JOIN(NAMESPACE, PREFIX, pack_all)
static inline void pack_all(prepack_state_t *st) {
	const FTYPE *restrict next_cur = st->next_cur;
	FTYPE *restrict next_pack_cur = st->next_pack_cur;

	do {
		do {
			FTYPE v = (st->mn_slice_pos++ < st->mn_slice_real_len) ? *st->next_cur : 0;
			*st->next_pack_cur++ = v;
			st->next_cur += st->interval_mn;
			st->packed_size++;
		} while (st->mn_slice_pos < st->mn_slice_len);
		st->mn_slice_pos = 0;
		st->next_cur += st->proceed_k;
	} while (st->packed_size < st->size);
	st->next_cur = st->next_bak + st->interval_mn * st->mn_slice_len;
	st->next_bak = st->next_cur;
	st->packed_size = 0;
	st->sched_size = st->max_sched_size;

	st->next_cur = next_cur;
	st->next_pack_cur = next_pack_cur;
}

#ifdef __cplusplus
}
#endif

#endif
