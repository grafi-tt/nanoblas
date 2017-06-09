#ifndef NANOBLAS_ITER_H
#define NANOBLAS_ITER_H

#include <stddef.h>

typedef struct {
	size_t cnt;
	size_t pos;
	int len;
	const int unit_len;
	const size_t blk_num;
	const size_t ext_num;
	const int blk_len;
	const int lack_len;
} iter_t;

typedef struct {
	int depth;
	int is_end;
	iter_t iters[];
} nest_iter_t;

void iter_by_blk_spec(size_t len, int max_blk_len, int unit_len, iter_t *it);
void next(iter_t *it);
void nest_next(nest_iter_t *nit);

#endif
