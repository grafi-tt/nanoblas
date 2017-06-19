#include "nanoblas_prepack.h"
#include "internal/prepack.h"
#include "internal/util.h"

void start_prepack(prepack_state_t *st) {
	st->k_sched_len = imin(st->k_sched_max_len, st->k_next_len);
	st->mn_sched_len = imin(st->unit_len, st->mn_next_len);
}

void step_prepack(prepack_state_t *st) {
	st->next_cur += st->k_sched_len*st->interval_k;
	st->next_pack_cur += st->k_sched_len * st->mn_sched_len;
	st->k_packed_len += st->k_sched_len;
	if (st->k_packed_len == st->k_next_len) {
		st->next_cur += -st->k_packed_len*st->interval_k + st->mn_sched_len*st->interval_mn;
		st->k_packed_len = 0;
		st->mn_packed_len += st->mn_sched_len;
		st->mn_sched_len = imin(st->unit_len, st->mn_next_len - st->mn_packed_len);
		if (st->mn_sched_len == 0) return;
	}
	st->k_sched_len = imin(st->k_sched_max_len, st->k_next_len - st->k_packed_len);
}

void pack_all(prepack_state_t *st) {
	int mn_cnt = 0;
	const ptrdiff_t proceed_k = st->interval_k - st->unit_len*st->interval_mn;
	while (st->mn_packed_len < st->mn_next_len) {
		while (st->k_packed_len < st->k_next_len) {
			while (mn_cnt < st->unit_len) {
				FTYPE v = (mn_cnt < st->mn_sched_len) ? *st->next_cur : 0;
				*st->next_pack_cur++ = v;
				st->next_cur += st->interval_mn;
				mn_cnt++;
			}
			mn_cnt = 0;
			st->next_cur += proceed_k;
			st->k_packed_len++;
		}
		st->next_cur += -st->k_packed_len*st->interval_k + st->mn_sched_len*st->interval_mn;
		st->k_packed_len = 0;
		st->mn_packed_len += st->mn_sched_len;
		st->mn_sched_len = imin(st->unit_len, st->mn_next_len - st->mn_packed_len);
	}
	st->k_sched_len = 0;
}
