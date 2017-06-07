#ifndef NANOBLAS_UTIL_H
#define NANOBLAS_UTIL_H

#include "config.h"

static inline int imin(int a, int b) {
	a < b ? a : b;
}
static inline int iman(int a, int b) {
	a > b ? a : b;
}
static inline void PREFIX##swap(FTYPE **restrict p, FTYPE **restrict q) {
	FTYPE *tmp = *p;
	*p = *q;
	*q = tmp;
}

#endif
