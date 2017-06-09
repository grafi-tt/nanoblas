#include <stdlib.h>
#include "gemm.h"
#include "iter.h"
#include "kernel.h"
#include "pack.h"
#include "util.h"

static void decide_blk_len(int max_blk_len, size_t len,
		size_t *blk_num, int *restrict blk_len, size_t *restrict blk_ext_num, int *restrict blk_lack_len) {
}

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
	FTYPE *mem = (FTYPE*)malloc(sizeof(FTYPE)*4*BLK_K_LEN*BLK_MN_LEN);
	FTYPE *restrict a_pack      = mem;
	FTYPE *restrict a_next_pack = mem + BLK_K_LEN*BLK_MN_LEN;
	FTYPE *restrict b_pack      = mem + 2*BLK_LEN*BLK_MN_LEN;
	FTYPE *restrict b_next_pack = mem + 3*BLK_LEN*BLK_MN_LEN;
	/* pack A, B */
	pack_block(imin(k, BLK_LEN), imin(m, BLK_LEN), interval_k_in_a, interval_m, a_pack, a);
	pack_block(imin(k, BLK_LEN), imin(n, BLK_LEN), interval_k_in_n, interval_n, b_pack, b);

	/* get iterator */
	iter_t _m_it, _n_it, _k_it;
	iter_by_blk_spec(m, blk_mn_len, UNIT_LEN, _m_it);
	iter_by_blk_spec(n, blk_mn_len, UNIT_LEN, _n_it);
	iter_by_blk_spec(k, blk_k_len, UNIT_LEN, _k_it);
	nest_iter_t it3 = {
		.depth = 2, .is_end = 0, .iters = { _m_it, _n_it, _k_it }
	};
	nest_iter_t it2 = {
		.depth = 2, .is_end = 0, .iters = { _n_it, _k_it }
	};
	/* prepare scheduling */
	sched_state_t a_sched_state, b_sched_state;

	do {
		nest_next(&it3);
		if (!it3.is_end) {
			a_sched_state = {
				.next_cur = a + interval_m*n_it3->pos  + interval_k_in_a*k_it3->pos;
				.next_pack_cur = a_pack;
				.k_next_len = k_it3->len;
				.mn_next_len = m_it3->len;
			};
			start_sched(&a_sched_state);
			sched_state_p = &a_sched_state;
		}

		if (m_it3->pos == 0) {
			nest_next(&it2);
			if (!it2.is_end) {
				b_sched_state = {
					.next_cur = b + interval_n*n_it2->pos  + interval_k_in_b*k_it2->pos;
					.next_pack_cur = b_pack;
					.k_next_len = k_it2->len;
					.mn_next_len = n_it2->len;
				};
				start_sched(&b_sched_state);
			}
		}

		for (int n_pos = 0; n_pos < n_len; n_pos += UNIT_LEN) {
			int n_sub_len = imin(UNIT_LEN, n_len - n_pos);
			for (int m_pos = 0; m_pos < m_len; m_pos += UNIT_LEN) {
				int m_sub_len = imin(UNIT_LEN, m_len - m_pos);
				kernel(a_pack_cur, b_pack_cur, k_len, c_cur, ldc, sched_state_p);
				if (sched_state_p) {
					step_sched(sched_state_p);
					if (sched_state_p->k_sched_len == 0) {
						if (sched_state == &a_sched_state) {
							if (b_sched_state.k_sched_len != 0) {
								sched_state = &b_sched_state;
								goto sched_end;
							}
						}
						sched_state_p = NULL;
					}
				}
				sched_end:
				a_pack_cur += k_len * m_sub_len;
				b_pack_cur += k_len * n_sub_len;
				c_cur += m_sub_len;
			}
			c_cur += n_sub_len*ldc - m_len;
		}
	} while (!it3.is_end);
}
