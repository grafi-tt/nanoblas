#include <stdlib.h>
#include "gemm.h"
#include "boundary.h"
#include "kernel.h"
#include "util.h"

void gemm(char transa, char transb, size_t m, size_t n, size_t k,
		FTYPE alpha, FTYPE *restrict a, size_t lda,
		FTYPE *restrict b, size_t ldb,
		FTYPE beta, FTYPE *restrict c, size_t ldc) {

	/* get interval */
	int trans_a = transa == 'T';
	int trans_b = transb == 'T';
	size_t interval_k_in_a = trans_a ? lda : 1;
	size_t interval_k_in_b = trans_b ? 1 : ldb;
	size_t interval_m = trans_a ? 1 : lda;
	size_t interval_n = trans_b ? ldb : 1;
	/* get kernel */
	kernel_t *kernel = decide_kernel();
	/* I believe this >1M memory never happen to be unaligned */
	FTYPE *mem = (FTYPE*)malloc(sizeof(FTYPE)*3*BLK_LEN*BLK_LEN);
	FTYPE *restrict a_pack = mem;
	FTYPE *restrict a_next_pack = mem + BLK_LEN*BLK_LEN;
	FTYPE *restrict b_pack = mem + 2*BLK_LEN*BLK_LEN;
	/* pack A */
	pack_block(imin(k, BLK_LEN), imin(m, BLK_LEN), interval_k_in_a, interval_m, a_pack, a);

	/* block loop k */
	for (size_t k_pos = 0; k_pos < k; k_pos += BLK_LEN) {
		const int k_len = imin(k - k_pos, BLK_LEN);
		const FTYPE *a_pos = a + interval_k_in_a * k_pos;
		const FTYPE *a_pos_bak = a_pos;
		const FTYPE *b_pos = b + interval_k_in_b * k_pos;
		/* block loop n */
		for (size_t n_pos = 0; n_pos < n; n_pos += BLK_LEN) {
			const int n_len = imin(BLK_LEN, n - n_pos);
			/* pack B */
			pack_block(k_len, n_len, interval_k_in_b, interval_n, b_pack, b_pos);
			/* block loop m */
			for (size_t m_pos = 0; m_pos < m; m_pos += BLK_LEN) {
				const int m_len = imin(BLK_LEN, n - n_pos);
				/* schedule packing of a */
				FTYPE *a_next_pack_cur = a_next_pack;
				const FTYPE *a_next_pos;
				/*const*/ int k_next_len, m_next_len;
				if (m_pos + BLK_LEN < m) {
					a_next_pos = a_pos + interval_m*m_len;
					k_next_len = k_len;
					m_next_len = imin(BLK_LEN, m - (m_pos + BLK_LEN));
				} else if (n_pos + BLK_LEN < n) {
					a_next_pos = a_pos_bak;
					k_next_len = k_len;
					m_next_len = imin(BLK_LEN, m);
				} else if (k_pos + BLK_LEN < k) {
					a_next_pos = a_pos_bak + interval_k_in_a*k_len;
					k_next_len = imin(BLK_LEN, k - (k_pos + BLK_LEN));
					m_next_len = imin(BLK_LEN, m);
				} else {
					/* the last loop, so stop packing */
					k_next_len = 0;
					m_next_len = 0;
				}
				int m_sched_cnt = 0;
				/* unit loop B */
				for (size_t n_cur = n_pos; n_cur < n_pos + BLK_LEN; n_cur += UNIT_LEN) {
					const int n_sub_len = imax(imin(UNIT_LEN, n - n_cur), 0);
					/* get schedule */
					int m_sched_len = imin(UNIT_LEN, m_next_len - m_sched_cnt);
					int k_sched_cnt = 0;
					const FTYPE *a_next_cur = a_next_pos + interval_m*m_sched_cnt;
					/* unit loop A */
					for (size_t m_cur = m_pos; m_cur < m_pos + BLK_LEN; m_cur += UNIT_LEN) {
						const int m_sub_len = imax(imin(UNIT_LEN, m - m_cur), 0);
						FTYPE *a_pack_cur = a_pack;
						FTYPE *b_pack_cur = b_pack;
						/* get schedule */
						int k_sched_len = imin(UNIT_LEN, k_next_len - k_sched_cnt);
						/* run kernel */
						if (n_sub_len == UNIT_LEN && m_sub_len == UNIT_LEN) {
							kernel(a_pack_cur, b_pack_cur,
									k_sched_len, m_sched_len, interval_k_in_a, interval_m,
									a_next_pack_cur, a_next_cur, k_len, c, ldc);
						} else {
							boundary_kernel(m_len, n_len, a_pack_cur, b_pack_cur,
									k_sched_len, m_sched_len, interval_k_in_a, interval_m,
									a_next_pack_cur, a_next_cur, k_len, c, ldc);
						}
						a_pack_cur += k_len * m_len;
						b_pack_cur += k_len * n_len;
						k_sched_cnt += k_sched_len;
						a_next_pack_cur += k_sched_len * m_sched_len;
						a_next_cur += interval_k_in_a*k_sched_len;
					}
					m_sched_cnt += m_sched_len;
				}
				a_pos += m_len;
				fswap(&a_pack, &a_next_pack);
			}
			b_pos += n_len;
		}
	}
}
