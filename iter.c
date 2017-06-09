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
