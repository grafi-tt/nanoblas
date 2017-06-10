#include <stdint.h>
#include "gemm.h"
#include "iter.h"
#include "kernel.h"
#include "pack.h"
#include "util.h"

void gemm(char transa, char transb, size_t m, size_t n, size_t k,
		FTYPE alpha, FTYPE *restrict a, ptrdiff_t lda,
		FTYPE *restrict b, ptrdiff_t ldb,
		FTYPE beta, FTYPE *restrict c, ptrdiff_t ldc) {

	/* get interval */
	const int trans_a = transa == 'T';
	const int trans_b = transb == 'T';
	const ptrdiff_t interval_k_in_a = trans_a ? lda : 1;
	const ptrdiff_t interval_k_in_b = trans_b ? 1 : ldb;
	const ptrdiff_t interval_m = trans_a ? 1 : lda;
	const ptrdiff_t interval_n = trans_b ? ldb : 1;
	/* get kernel */
	kernel_t *kernel = decide_kernel();

	FTYPE mem[4*BLK_K_LEN*BLK_MN_LEN+32/size(FTYPE)];
	FTYPE mem_ptr = mem;
	const int mem_offset = (uintptr_t)mem & (32/size(FTYPE)-1);
	if (mem_offset) mem_ptr += 32/size(FTYPE) - mem_offset;

	FTYPE *restrict a_pack      = mem_ptr;
	FTYPE *restrict a_next_pack = mem_ptr +   BLK_K_LEN*BLK_MN_LEN;
	FTYPE *restrict b_pack      = mem_ptr + 2*BLK_K_LEN*BLK_MN_LEN;
	FTYPE *restrict b_next_pack = mem_ptr + 3*BLK_K_LEN*BLK_MN_LEN;

	/* get iterator */
	iter_t _m_it, _n_it, _k_it;
	iter_by_blk_spec(m, BLK_MN_LEN, UNIT_LEN, &_m_it);
	iter_by_blk_spec(n, BLK_MN_LEN, UNIT_LEN, &_n_it);
	iter_by_blk_spec(k, BLK_K_LEN, UNIT_LEN, &_k_it);
	nest_iter_t it3 = {
		.depth = 2, .is_end = 0, .iters = { _m_it, _n_it, _k_it }
	};
	nest_iter_t it2 = {
		.depth = 2, .is_end = 0, .iters = { _n_it, _k_it }
	};
	iter_t *const m_it3 = &it3.iters[0], *const n_it3 = &it3.iters[1], *const k_it3 = &it3.iters[2];
	iter_t *const n_it2 = &it2.iters[0], *const k_it2 = &it2.iters[1];

	/* first packing */
	sched_state_t *sched_state_p = NULL;
	FTYPE *restrict a_pack_cur;
	FTYPE *restrict b_pack_cur;
	sched_state_t a_sched_state = {
		.next_cur = a;
		.next_pack_cur = a_next_pack,
		.k_next_len = k_it3->len,
		.mn_next_len = m_it3->len,
		.interval_k = interval_k_in_a,
		.interval_mn = interval_m,
		.unit_len = UNIT_LEN
	};
	sched_state_t b_sched_state = {
		.next_cur = b;
		.next_pack_cur = b_next_pack,
		.k_next_len = k_it2->len,
		.mn_next_len = n_it2->len,
		.interval_k = interval_k_in_b,
		.interval_mn = interval_n,
		.unit_len = UNIT_LEN
	};
	start_sched(&a_sched_state);
	all_steps_sched(&a_sched_state);
	start_sched(&b_sched_state);
	all_steps_sched(&b_sched_state);

	do {
		const int m_len = m_it3->len;
		const int n_len = n_it3->len;
		const int k_len = k_it3->len;
		const size_t m_end = m_it3->pos + m_len;
		const size_t n_end = n_it3->pos + n_len;

		nest_next(&it3);
		if (!it3.is_end) {
			fswap(a_pack, a_next_pack);
			a_pack_cur = a_pack;
			a_sched_state.next_cur = a + interval_m*m_it3->pos + interval_k_in_a*k_it3->pos;
			a_sched_state.next_pack_cur = a_next_pack;
			a_sched_state.k_next_len = k_it3->len;
			a_sched_state.mn_next_len = m_it3->len;
			start_sched(&a_sched_state);
			sched_state_p = &a_sched_state;
		}
		if (m_it3->pos == 0) {
			nest_next(&it2);
			if (!it2.is_end) {
				fswap(b_pack, b_next_pack);
				b_pack_cur = b_pack;
				b_sched_state.next_cur = b + interval_n*n_it2->pos + interval_k_in_b*k_it2->pos;
				b_sched_state.next_pack_cur = b_next_pack;
				b_sched_state.k_next_len = k_it2->len;
				b_sched_state.mn_next_len = n_it2->len;
				start_sched(&b_sched_state);
			}
		}

		for (size_t n_cur = n_pos; n_cur < n_end; n_cur += UNIT_LEN) {
			int n_sub_len = imin(UNIT_LEN, n_end - n_cur);
			for (size_t m_cur = m_pos; m_cur < m_end; m_cur += UNIT_LEN) {
				int m_sub_len = imin(UNIT_LEN, m_end - m_cur);
				kernel(a_pack_cur, b_pack_cur, k_len, m_sub_len, n_sub_Len,
						c + ldc*n_cur + m_cur, ldc, sched_state_p);
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
			}
		}
	} while (!it3.is_end);
}
