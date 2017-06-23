#ifndef NANOBLAS_INTERNAL_UTIL_H
#define NANOBLAS_INTERNAL_UTIL_H

#include "internal/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline int imin(int a, int b) {
	return a < b ? a : b;
}
static inline int imax(int a, int b) {
	return a > b ? a : b;
}
#define fswap JOIN(FSIZE_PREFIX, fswap)
static inline void fswap(FTYPE *restrict *restrict p, FTYPE *restrict *restrict q) {
	FTYPE *tmp = *p;
	*p = *q;
	*q = tmp;
}

#ifdef __cplusplus
}
#endif

#endif
