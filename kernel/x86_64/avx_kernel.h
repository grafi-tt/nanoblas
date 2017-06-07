#include "kernel.h"

void avx_kernel_##FTYPE(
	int k_len, FTYPE *restrict a_pack, FTYPE *restrict b_pack,
	int k_len_sched, int m_len_sched, int trans_a, size_t lda,
	FTYPE *restrict a_pack_next, FTYPE *restrict a,
	size_t ldc, FTYPE* restrict c);
