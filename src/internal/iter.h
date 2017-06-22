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
} iter_t;

#ifdef __cplusplus
extern "C" {
#endif

#define iter_by_blk_spec JOIN(NAMESPACE, iter_by_blk_spec)
void iter_by_blk_spec(size_t len, int max_len, int slice_len, iter_t *it);

#define next JOIN(NAMESPACE, next)
void next(iter_t *it) {
	it->pos += it->len;
	if (it->pos == it->dec_pos)
		it->len -= it->slice_len;
	size_t distance = it->sum - it->pos;
	if (distance == 0) {
		it->pos = 0;
		it->len = it->base_len;
	} else if (distance < (size_t)it->len) {
		it->len = distance;
	}
}

#ifdef __cplusplus
}
#endif

#endif
