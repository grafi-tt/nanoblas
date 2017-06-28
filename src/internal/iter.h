#ifndef NANOBLAS_INTERNAL_ITER_H
#define NANOBLAS_INTERNAL_ITER_H

#include <stddef.h>
#include "internal/macro.h"

typedef struct {
	size_t pos;
	int len;
	int base_len;
	int slice_len;
	size_t sum;
	size_t dec_pos;
} circular_iter_t;

#ifdef __cplusplus
extern "C" {
#endif

#define simple_iter JOIN(NAMESPACE, simple_iter)
static inline circular_iter_t simple_iter(size_t sum, int base_len) {
	circular_iter_t it = {
		.pos = 0,
		.len = (size_t)base_len < sum ? base_len : (int)sum,
		.base_len = base_len,
		.sum = sum,
		.slice_len = base_len,
		.dec_pos = sum,
	};
	return it;
}

#define blk_spec_iter JOIN(NAMESPACE, blk_spec_iter)
circular_iter_t blk_spec_iter(size_t sum, int max_len, int slice_len);

#define next JOIN(NAMESPACE, next)
static inline int next(circular_iter_t *it) {
	it->pos += it->len;
	if (it->pos == it->dec_pos)
		it->len -= it->slice_len;
	size_t distance = it->sum - it->pos;
	if (distance == 0) {
		it->pos = 0;
		it->len = it->base_len;
		return 2;
	} else if (distance < (size_t)it->len) {
		it->len = distance;
		return 1;
	}
	return 0;
}

#ifdef __cplusplus
}
#endif

#endif
