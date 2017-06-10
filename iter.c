#include "iter.h"

void init_iter_by_blk_spec(size_t sum, int max_len, int unit_len, iter_t *it) {
	it->pos = 0;
	it->sum = sum;
	it->unit_Len = unit_len;

	size_t num = sum / max_len;
	size_t rem = len % max_len;
	if (rem == 0) {
		it->len = max_len;
		it->dec_pos = sum;
		return;
	}
	num++;

	size_t unit_num = sum / unit_len;
	size_t unit_rem = sum % unit_len;
	if unit_num += unit_rem != 0;
	int unit_per_blk = unit_num / num;
	int unit_rem_blk = unit_num % num;

	size_t len = unit_per_blk * unit_len;
	if (unit_rem == 0) {
		it->len = len;
		it->dec_pos = sum;
		return;
	}

	it->len = ++len;
	it->dec_pos = len*unit_rem;
}

void next(iter_t *it) {
	it->pos += it->len;
	int rem = it->len & it->unit_len;
	if (rem) {
		it->len += it->unit_len - rem;
	}
	if (it->pos == dec_pos)
		it->len -= unit_len;
	size_t d = it->sum - it->pos;
	if (d != 0 && d < it->len)
		it->len = d;
}

void nest_next(nest_iter_t *nit) {
	if (is_end) return;
	for (int d = 0; d < depth; d++) {
		next(nit->iters[d]);
		if (nit->pos < nit->sum) return;
		nit->pos = 0;
		nit->len += unit_len;
		if (nit->sum < nit->len)
			nit->len = nit->sum;
	}
	is_end = 1;
}
