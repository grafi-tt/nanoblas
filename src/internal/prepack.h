#ifndef NANOBLAS_INTERNAL_PREPACK_H
#define NANOBLAS_INTERNAL_PREPACK_H

#include "nanoblas_types.h"
#include "internal/macro.h"
#include "internal/util.h"

#ifdef __cplusplus
extern "C" {
#endif

#define set_packed JOIN(NAMESPACE, FSIZE_PREFIX, set_packed)
static inline void set_packed(prepack_state_t *st) {
	st->remained_next_slice_len = 0;
}

#define is_packed JOIN(NAMESPACE, FSIZE_PREFIX, is_packed)
static inline int is_packed(prepack_state_t *st) {
	return st->remained_next_slice_len <= 0;
}

#define pack_all JOIN(NAMESPACE, FSIZE_PREFIX, pack_all)
static inline void pack_all(prepack_state_t *st, pack_fun_t *pack_fun) {
	if (!is_packed(st)) {
		do {
			st->sched_len = st->len - st->packed_len;
			pack_fun(st);

			st->next_cur = (const FTYPE *)((const char *)st->next_bak + st->interval_mn * st->slice_len);
			st->next_bak = st->next_cur;
			st->remained_next_slice_len -= st->slice_len;
			st->next_slice_real_len = imin(st->slice_len, st->remained_next_slice_len);

			st->packed_len = 0;
		} while (!is_packed(st));
		st->sched_len = imin(st->max_sched_len, st->len);
	}
}

#define pack_slice JOIN(NAMESPACE, FSIZE_PREFIX, pack_slice)
static inline void pack_slice(prepack_state_t *st, pack_fun_t *pack_fun) {
	if (!is_packed(st)) {
		st->sched_len = st->len - st->packed_len;
		pack_fun(st);

		st->next_cur = (const FTYPE *)((const char *)st->next_bak + st->interval_mn * st->slice_len);
		st->next_bak = st->next_cur;
		st->remained_next_slice_len -= st->slice_len;
		st->next_slice_real_len = imin(st->slice_len, st->remained_next_slice_len);

		st->packed_len = 0;
		st->sched_len = imin(st->max_sched_len, st->len);
	}
}

#define step_prepack JOIN(NAMESPACE, FSIZE_PREFIX, step_prepack)
static inline void step_prepack(prepack_state_t *st) {
	st->packed_len += st->sched_len;
	if (st->packed_len == st->len) {
		st->next_cur = (const FTYPE *)((const char *)st->next_bak + st->interval_mn * st->slice_len);
		st->next_bak = st->next_cur;
		st->remained_next_slice_len -= st->slice_len;
		st->next_slice_real_len = imin(st->slice_len, st->remained_next_slice_len);

		st->packed_len = 0;
		st->sched_len = imin(st->max_sched_len, st->len);
	} else {
		st->sched_len = imin(st->max_sched_len, st->len - st->packed_len);
	}
}

#define limit_prepack JOIN(NAMESPACE, FSIZE_PREFIX, limit_prepack)
static inline void limit_prepack(prepack_state_t *st, int sum_next_slice_len) {
	st->remained_next_slice_len = sum_next_slice_len;
	st->next_slice_real_len = imin(st->slice_len, st->remained_next_slice_len);
}

#define restart_prepack JOIN(NAMESPACE, FSIZE_PREFIX, restart_prepack)
static inline void restart_prepack(prepack_state_t *st,
		const FTYPE *next, int sum_next_slice_len) {
	limit_prepack(st, sum_next_slice_len);
	st->next_cur = next;
	st->next_bak = next;
}

#define reset_prepack JOIN(NAMESPACE, FSIZE_PREFIX, reset_prepack)
static inline void reset_prepack(prepack_state_t *st,
		const FTYPE *next, int sum_next_slice_len, int k_next_len, int max_sched_len) {
	restart_prepack(st, next, sum_next_slice_len);
	st->len = k_next_len;
	st->max_sched_len = max_sched_len;
	st->sched_len = imin(st->max_sched_len, st->len);
}

#define prepack_state_new JOIN(NAMESPACE, FSIZE_PREFIX, prepack_state_new)
static inline prepack_state_t prepack_state_new(
		const FTYPE *next, FTYPE *next_pack, int slice_len, int sum_next_slice_len,
		int k_next_len, int max_sched_len, ptrdiff_t interval_mn, ptrdiff_t interval_k) {
	prepack_state_t st = {
		.next_pack_cur = next_pack,
		.interval_mn = sizeof(FTYPE) * interval_mn,
		.interval_k = sizeof(FTYPE) * interval_k,
		.slice_len = slice_len,
		.packed_len = 0,
	};
	reset_prepack(&st, next, sum_next_slice_len, k_next_len, max_sched_len);
	return st;
}

#ifdef __cplusplus
}
#endif

#endif
