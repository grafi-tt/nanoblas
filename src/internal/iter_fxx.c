#include "internal/iter.h"

void init_iter_by_blk_spec(size_t sum, int max_len, int slice_len, iter_t *it) {
	it->pos = 0;
	it->sum = sum;
	it->slice_len = slice_len;

	size_t num = sum / max_len;
	size_t rem = sum % max_len;
	if (rem == 0) {
		it->len = max_len;
		it->dec_pos = sum;
		return;
	}
	num++;

	size_t slice_num = sum / slice_len;
	size_t slice_rem = sum % slice_len;
	if (slice_rem != 0) slice_num++;
	int slice_per_blk = slice_num / num;
	int slice_rem_blk = slice_num % num;

	size_t len = slice_per_blk * slice_len;
	if (slice_rem_blk == 0) {
		it->dec_pos = sum;
	} else {
		len += slice_len;
		it->dec_pos = len * slice_rem_blk;
	}
	it->len = len;
	it->base_len = len;
}
