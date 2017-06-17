#include "internal/iter.h"

void init_iter_by_blk_spec(size_t sum, int max_len, int unit_len, iter_t *it) {
	it->pos = 0;
	it->sum = sum;
	it->unit_len = unit_len;

	size_t num = sum / max_len;
	size_t rem = sum % max_len;
	if (rem == 0) {
		it->len = max_len;
		it->dec_pos = sum;
		return;
	}
	num++;

	size_t unit_num = sum / unit_len;
	size_t unit_rem = sum % unit_len;
	int unit_per_blk = unit_num / num;
	int unit_rem_blk = unit_num % num;

	size_t len = unit_per_blk * unit_len;
	if (unit_rem == 0) {
		it->len = len;
		it->dec_pos = sum;
		return;
	}

	it->len = ++len;
	it->dec_pos = len*unit_rem_blk;
}

void next(iter_t *it) {
	it->pos += it->len;
	if (it->pos == it->sum) {
		int rem = it->len % it->unit_len;
		if (rem) {
			it->len += it->unit_len - rem;
		}
	}
	if (it->pos == it->dec_pos)
		it->len -= it->unit_len;
	size_t d = it->sum - it->pos;
	if (d != 0 && d < (size_t)it->len)
		it->len = d;
}

void nest_next(nest_iter_t *nit) {
	for (int d = 0; d < nit->depth; d++) {
		iter_t *it = &nit->iters[d];
		next(it);
		if (it->pos < it->sum) return;
		it->pos = 0;
		it->len += it->unit_len;
		if (it->sum < (size_t)it->len)
			it->len = it->sum;
	}
	nit->is_end = 1;
}
