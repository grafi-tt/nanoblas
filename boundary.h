#ifndef NANOBLAS_KERNEL_BOUNDARY_H
#define NANOBLAS_KERNEL_BOUNDARY_H

#include "const.h"

void boundary_kernel(
		int len, int m, int n
		FTYPE *restrict a_pack_cur, FTYPE *restrict b_pack_cur,
		size_t ldc, FTYPE *restrict c);

#endif
