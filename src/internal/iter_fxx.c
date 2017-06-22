#include "internal/iter.h"

circular_iter_t blk_spec_iter(size_t sum, int max_len, int slice_len) {
	circular_iter_t it = {
		.pos = 0,
		.sum = sum,
		.slice_len = slice_len,
	};

	size_t num = sum / max_len;
	size_t rem = sum % max_len;
	if (rem == 0) {
		it.len = max_len;
		it.base_len = max_len;
		it.dec_pos = sum;
	} else {
		num++;
		size_t slice_num = sum / slice_len;
		size_t slice_rem = sum % slice_len;
		if (slice_rem != 0) slice_num++;
		int slice_per_blk = slice_num / num;
		int slice_rem_blk = slice_num % num;

		size_t len = slice_per_blk * slice_len;
		if (slice_rem_blk == 0) {
			it.dec_pos = sum;
		} else {
			len += slice_len;
			it.dec_pos = len * slice_rem_blk;
		}
		it.len = (size_t)len < sum ? len : sum;
		it.base_len = len;
	}
	return it;
}
