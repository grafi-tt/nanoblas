#include "iter.h"

void next(iter_t *it) {
	it->blk_cnt++;
	current_pos += current_len;
	current_len = blk_len +
		(it->blk_cnt < it->blk_ext_num)*unit_len -
		(it->blk_cnt+1 == it->blk_num)*it->blk_lack_len;
}

void iter_by_blk_spec(size_t len, int max_blk_len, int unit_len, iter_t *it) {
	size_t unit_num = len / UNIT_LEN;
	size_t unit_rem = len / UNIT_LEN;
	if (unit_rem == 0) {
		*blk_lack_len = 0;
	} else {
		*blk_lack_len = UNIT_LEN - unit_rem;
		len += UNIT_LEN - unit_rem;
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

void mnk_iter(size_t m, size_t n, size_t k, int blk_k_len, int blk_mn_len, int unit_len, nest_iter_t *nit) {
	nit->isend = 0;
	iter_by_blk_spec(m, blk_mn_len, unit_len, &nit->iters[0]);
	iter_by_blk_spec(n, blk_mn_len, unit_len, &nit->iters[1]);
	iter_by_blk_spec(k, blk_k_len, unit_len, &nit->iters[2]);
}
