#include <stdbool.h>
#include <stdint.h>
#include "nanoblas.h"
#include "nanoblas_kernel.h"
#include "nanoblas_prepack.h"
#include "internal/iter.h"
#include "internal/macro.h"
#include "internal/prepack.h"
#include "internal/util.h"

#define gemm ADD_PREFIX(gemm)
void gemm_do(nanoblas_t *nb,
		const bool is_transposed, const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_TRANSPOSE TransB,
		const size_t M, const size_t N, const size_t K,
		const FTYPE alpha, const FTYPE *A, const ptrdiff_t lda, const FTYPE *B, const ptrdiff_t ldb,
		const FTYPE beta, FTYPE *C, const ptrdiff_t ldc) {

	(void)alpha;
	(void)beta;

	nanoblas_init(nb);

	/* get interval */
	const ptrdiff_t interval_m = TransA ? 1 : lda;
	const ptrdiff_t interval_k_in_a = TransA ? lda : 1;
	const ptrdiff_t interval_k_in_b = TransB ? 1 : ldb;
	const ptrdiff_t interval_n = TransB ? ldb : 1;

	/* get block size */
	const size_t blk_m_len = is_transposed ? nb->blk_n_len : nb->blk_m_len;
	const size_t blk_n_len = is_transposed ? nb->blk_m_len : nb->blk_n_len;
	const size_t blk_k_len = nb->blk_k_len;

	/* get kernel */
	kernel_t kernel = nb->kernel;
	int unit_len = kernel.unit_len;
	kernel_fun_t *kernel_fun = kernel.fun;

	/* allocate pack (VLA) */
	FTYPE mem[2*blk_m_len*blk_k_len + 2*blk_k_len*blk_n_len + 32/sizeof(FTYPE)];
	FTYPE *mem_ptr = mem;
	const int mem_offset = ((uintptr_t)mem & 32)/sizeof(FTYPE);
	if (mem_offset) mem_ptr += 32/sizeof(FTYPE) - mem_offset;
	FTYPE *restrict a_pack      = mem_ptr;
	FTYPE *restrict a_next_pack = mem_ptr +   blk_m_len*blk_k_len;
	FTYPE *restrict b_pack      = mem_ptr + 2*blk_m_len*blk_k_len;
	FTYPE *restrict b_next_pack = mem_ptr + 2*blk_m_len*blk_k_len + blk_k_len*blk_n_len;

	/* get iterator */
	iter_t _m_it, _n_it, _k_it;
	iter_by_blk_spec(M, blk_m_len, unit_len, &_m_it);
	iter_by_blk_spec(N, blk_n_len, unit_len, &_n_it);
	iter_by_blk_spec(K, blk_k_len, unit_len, &_k_it);
	union {
		char mem[sizeof(nest_iter_t) + sizeof(iter_t)*3];
		nest_iter_t nit;
	} it3_union = { .nit = { .depth = 3, .is_end = 0 } };
	union {
		char mem[sizeof(nest_iter_t) + sizeof(iter_t)*2];
		nest_iter_t nit;
	} it2_union = { .nit = { .depth = 2, .is_end = 0 } };
	nest_iter_t it3 = it3_union.nit;
	it3.iters[0] = _m_it;
	it3.iters[1] = _n_it;
	it3.iters[2] = _k_it;
	nest_iter_t it2 = it2_union.nit;
	it3.iters[0] = _n_it;
	it3.iters[1] = _k_it;
	iter_t *const m_it3 = &it3.iters[0], *const n_it3 = &it3.iters[1], *const k_it3 = &it3.iters[2];
	iter_t *const n_it2 = &it2.iters[0], *const k_it2 = &it2.iters[1];

	/* first packing */
	prepack_state_t *current_prepack_st_p = NULL;
	FTYPE *restrict a_pack_cur = a_pack;
	FTYPE *restrict b_pack_cur = b_pack;
	prepack_state_t a_prepack_st = {
		.next_cur = A,
		.next_pack_cur = a_next_pack,
		.k_next_len = k_it3->len,
		.mn_next_len = m_it3->len,
		.interval_k = interval_k_in_a,
		.interval_mn = interval_m,
		.unit_len = unit_len,
	};
	prepack_state_t b_prepack_st = {
		.next_cur = B,
		.next_pack_cur = b_next_pack,
		.k_next_len = k_it2->len,
		.mn_next_len = n_it2->len,
		.interval_k = interval_k_in_b,
		.interval_mn = interval_n,
		.unit_len = unit_len,
	};
	start_prepack(&a_prepack_st);
	pack_all(&a_prepack_st);
	start_prepack(&b_prepack_st);
	pack_all(&b_prepack_st);

	do {
		const int m_len = m_it3->len;
		const int n_len = n_it3->len;
		const int k_len = k_it3->len;
		const size_t m_pos = m_it3->pos;
		const size_t n_pos = n_it3->pos;
		const size_t m_end = m_it3->pos + m_len;
		const size_t n_end = n_it3->pos + n_len;

		nest_next(&it3);
		if (!it3.is_end) {
			fswap(&a_pack, &a_next_pack);
			a_prepack_st.next_cur = A + interval_m*m_it3->pos + interval_k_in_a*k_it3->pos,
			a_prepack_st.next_pack_cur = a_next_pack,
			a_prepack_st.mn_packed_len = 0;
			a_prepack_st.k_packed_len = 0;
			a_prepack_st.mn_next_len = m_it3->len,
			a_prepack_st.k_next_len = k_it3->len,
			start_prepack(&a_prepack_st);
			current_prepack_st_p = &a_prepack_st;
			a_pack_cur = a_pack;
		}
		if (m_it3->pos == 0) {
			nest_next(&it2);
			if (!it2.is_end) {
				fswap(&b_pack, &b_next_pack);
				b_prepack_st.next_cur =  B + interval_k_in_b*k_it2->pos + interval_n*n_it2->pos,
				b_prepack_st.next_pack_cur = b_next_pack,
				b_prepack_st.mn_packed_len = 0;
				b_prepack_st.k_packed_len = 0;
				b_prepack_st.mn_next_len = n_it2->len,
				b_prepack_st.k_next_len = k_it2->len,
				start_prepack(&b_prepack_st);
				b_pack_cur = b_pack;
			}
		}

		kernel_state_t kernel_st = {
			.a_pack_cur = a_pack_cur,
			.b_pack_cur = b_pack_cur,
			.ldc = ldc,
			.k_len = k_len,
		};

		for (size_t n_cur = n_pos; n_cur < n_end; n_cur += unit_len) {
			kernel_st.n_sub_len = imin(unit_len, n_end - n_cur);

			for (size_t m_cur = m_pos; m_cur < m_end; m_cur += unit_len) {
				kernel_st.m_sub_len = imin(unit_len, m_end - m_cur);
				kernel_st.c_cur = C + ldc*m_cur + n_cur;

				kernel_fun(&kernel_st, current_prepack_st_p);

				if (current_prepack_st_p) {
					step_prepack(current_prepack_st_p);
					if (current_prepack_st_p->mn_sched_len == 0) {
						if (current_prepack_st_p == &a_prepack_st) {
							if (b_prepack_st.mn_sched_len != 0) {
								current_prepack_st_p = &b_prepack_st;
								goto b_prepack_decided;
							}
						}
						current_prepack_st_p = NULL;
					}
				}
				b_prepack_decided: ;
			}
		}
	} while (!it3.is_end);

}

#define gemm ADD_PREFIX(gemm)
void gemm(nanoblas_t *nb,
		const enum CBLAS_ORDER Order, const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_TRANSPOSE TransB,
		const size_t M, const size_t N, const size_t K,
		const FTYPE alpha, const FTYPE *A, const ptrdiff_t lda, const FTYPE *B, const ptrdiff_t ldb,
		const FTYPE beta, FTYPE *C, const ptrdiff_t ldc) {

	if (Order == CblasRowMajor) {
		gemm_do(nb, false, TransA, TransB, M, N, K, alpha, A, lda, B, ldb, beta, C, ldc);
	} else {
		gemm_do(nb, true, TransB, TransA, N, M, K, alpha, B, ldb, A, lda, beta, C, ldc);
	}
}
