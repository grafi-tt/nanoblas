#ifndef NANOBLAS_BOUNDARY_H
#define NANOBLAS_BOUNDARY_H

#include <stddef.h>
#include "const.h"

void pack_block_##FTYPE(int k_len, int m_or_n_len, size_t interval_k, size_t interval_m_or_n,
		FTYPE *restrict pack, FTYPE *restrict a_or_b);

void boundary_kernel_##FTYPE(
		int m_sub_len, int n_sub_len, FTYPE *restrict a_pack, FTYPE *restrict b_pack,
		int k_sched_len, int m_sched_len, int interval_k_in_a, int interval_m,
		FTYPE *restrict a_next_pack, FTYPE *restrict a_next,
		size_t k_len_szt, size_t ldc, FTYPE *restrict c);

#endif
