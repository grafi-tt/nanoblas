#ifndef NANOBLAS_BOUNDARY_H
#define NANOBLAS_BOUNDARY_H

#include <stddef.h>
#include "const.h"

void pack_block(int k_len, int m_or_n_len, size_t interval_k, size_t interval_m_or_n,
		FTYPE *restrict pack, FTYPE *restrict a_or_b);

void boundary_kernel(
		int len, int m, int n
		FTYPE *restrict a_pack_cur, FTYPE *restrict b_pack_cur,
		size_t ldc, FTYPE *restrict c);

#endif
