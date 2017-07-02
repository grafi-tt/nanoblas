#ifndef NANOBLAS_INTERNAL_ITER_H
#define NANOBLAS_INTERNAL_ITER_H

#include <stdio.h>
#include <stddef.h>
#include "internal/macro.h"

typedef struct {
	size_t pos;
	int len;
	int base_len;
	int slice_len;
	size_t sum;
} circular_iter_t;

#ifdef __cplusplus
extern "C" {
#endif

#define blk_spec_iter JOIN(NAMESPACE, blk_spec_iter)
static inline circular_iter_t blk_spec_iter(size_t sum, int max_len, int slice_len) {
	circular_iter_t it = {
		.pos = 0,
		.sum = sum,
		.slice_len = slice_len,
	};
	size_t blk_num = (sum - 1) / (size_t)max_len + 1;
	size_t slice_num = (sum - 1) / (size_t)slice_len + 1;
	int slice_num_in_blk = (slice_num - 1) / blk_num + 1;

	it.base_len = slice_num_in_blk * slice_len;
	it.len = (size_t)it.base_len < sum ? it.base_len : (int)sum;
	return it;
}

#define simple_iter JOIN(NAMESPACE, simple_iter)
static inline circular_iter_t simple_iter(size_t sum, int base_len) {
	circular_iter_t it = {
		.pos = 0,
		.len = (size_t)base_len < sum ? base_len : (int)sum,
		.base_len = base_len,
		.slice_len = base_len,
		.sum = sum,
	};
	return it;
}

#define blk_spec_iter JOIN(NAMESPACE, blk_spec_iter)
circular_iter_t blk_spec_iter(size_t sum, int max_len, int slice_len);

#define next JOIN(NAMESPACE, next)
static inline int next(circular_iter_t *it) {
	it->pos += it->len;
	size_t distance = it->sum - it->pos;
	if (distance == 0) {
		it->pos = 0;
		it->len = it->base_len;
		return 1;
	} else if (distance < (size_t)it->len) {
		it->len = distance;
	}
	return 0;
}

#ifdef __cplusplus
}
#endif

#endif
