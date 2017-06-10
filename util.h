#ifndef NANOBLAS_UTIL_H
#define NANOBLAS_UTIL_H

#include "const.h"

static inline int imin(int a, int b) {
	return a < b ? a : b;
}
static inline int imax(int a, int b) {
	return a > b ? a : b;
}
#define fswap APPEND_FTYPE(fswap)
static inline void fswap(FTYPE *restrict *restrict p, FTYPE *restrict *restrict q) {
	FTYPE *tmp = *p;
	*p = *q;
	*q = tmp;
}

#endif
