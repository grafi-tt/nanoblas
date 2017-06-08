#include "iter.h"

void init_iter_by_blk_spec(size_t len, int max_blk_len, int unit_len, iter_t *it) {
	size_t unit_num = len / unit_len;
	size_t unit_rem = len % unit_len;
	if (unit_rem == 0) {
		*blk_lack_len = 0;
	} else {
		*blk_lack_len = unit_len - unit_rem;
		len += unit_len - unit_rem;
		unit_num++;
	}
	*blk_num = len / max_blk_len;
	size_t blk_rem = len % max_blk_len;
	if (blk_rem == 0) {
		*blk_len = max_blk_len;
		*blk_ext_num = blk_num;
		return;
	}
	*blk_num++;
	int unit_per_blk = unit_num / blk_num;
	int unit_rem_blk = unit_num % blk_num;
	*blk_len = unit_per_blk * unit_len;
	*blk_ext_num = unit_rem_blk;
}

void next(iter_t *it) {
	it->blk_cnt++;
	current_pos += current_len;
	current_len = blk_len +
		(it->blk_cnt < it->blk_ext_num)*unit_len -
		(it->blk_cnt+1 == it->blk_num)*it->blk_lack_len;
}

void nest_next(nest_iter_t *nit) {
	if (is_end) return;
	for (int d = 0; d < depth; d++) {
		next(nit->iters[d]);
		if (nit->blk_cnt < nit->blk_num) return;
		nit->blk_cnt = 0;
		nit->current_pos = 0;
		nit->current_len = 0;
	}
	is_end = 1;
}

sched_len_t start_sched(pack_state_t *st) {
	st->k_sched_len = imin(st->k_next_len - st->k_packed_len, 2*st->unit_len);
	st->mn_sched_len = imin(st->mn_packed_len - st->mn_next_len, st->unit_len);
}

sched_len_t step_sched(pack_st_t *st) {
	if (st->k_sched_len == 0) return;
	st->next_pack_cur += st->k_sched_len * st->mn_sched_Len;
	st->k_packed_len += st->k_sched_len;
	if (st->k_packed_len != st->k_next_len) {
		st->next_cur += st->k_sched_len*interval_k;
	} else {
		st->next_cur += -st->k_packed_len*interval_k + st->mn_sched_len*interval_mn;
		st->k_packed_len = 0;
		st->mn_packed_len += st->mn_sched_len;
		st->mn_sched_len = imin(st->mn_packed_len - st->mn_next_len, st->unit_len);
		if (st->mn_sched_len == 0) {
			st->k_sched_len == 0;
			return;
		}
	}
	st->k_sched_len = imin(st->k_next_len - st->k_packed_len, 2*st->unit_len);
}
