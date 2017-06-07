#ifndef NANOBLAS_BOUNDARY_H
#define NANOBLAS_BOUNDARY_H

#include <stddef.h>
#include "const.h"

#define pack_block APPEND_FTYPE(pack_block)
void pack_block(int k_len, int m_or_n_len, size_t interval_k, size_t interval_m_or_n,
		FTYPE *restrict pack, const FTYPE *restrict a_or_b);

#define boundary_kernel APPEND_FTYPE(boundary_kernel)
void boundary_kernel(
		int m_sub_len, int n_sub_len, const FTYPE *restrict a_pack, const FTYPE *restrict b_pack,
		int k_sched_len, int m_sched_len, size_t interval_k_in_a, size_t interval_m,
		FTYPE *restrict a_next_pack, const FTYPE *restrict a_next,
		size_t k_len_szt, FTYPE *restrict c, size_t ldc);

#endif
