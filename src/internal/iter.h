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

typedef struct {
	int depth;
	int is_end;
	iter_t iters[];
} nest_iter_t;

#ifdef __cplusplus
extern "C" {
#endif

#define iter_by_blk_spec JOIN(NAMESPACE, iter_by_blk_spec)
void iter_by_blk_spec(size_t len, int max_len, int slice_len, iter_t *it);

#define next JOIN(NAMESPACE, next)
void next(iter_t *it);

#define nest_next JOIN(NAMESPACE, nest_next)
void nest_next(nest_iter_t *nit);

#ifdef __cplusplus
}
#endif

#endif
