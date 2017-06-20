#include <stdbool.h>
#include <stdint.h>
#include "nanoblas.h"
#include "nanoblas_kernel.h"
#include "nanoblas_prepack.h"
#include "internal/iter.h"
#include "internal/macro.h"
#include "internal/prepack.h"
#include "internal/util.h"

#define gemm_do JOIN(NAMESPACE, SIGN, gemm_do)
void gemm_do(const nanoblas_t *nb,
		const bool is_transposed, const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_TRANSPOSE TransB,
		const size_t M, const size_t N, const size_t K,
		const FTYPE alpha, const FTYPE *A, const ptrdiff_t lda, const FTYPE *B, const ptrdiff_t ldb,
		const FTYPE beta, FTYPE *C, const ptrdiff_t ldc) {

	(void)alpha;
	(void)beta;

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
	FTYPE *a_pack      = mem_ptr;
	FTYPE *a_next_pack = mem_ptr +   blk_m_len*blk_k_len;
	FTYPE *b_pack      = mem_ptr + 2*blk_m_len*blk_k_len;
	FTYPE *b_next_pack = mem_ptr + 2*blk_m_len*blk_k_len + blk_k_len*blk_n_len;

	/* get iterator */
	iter_t _n_it, _k_it;
	iter_by_blk_spec(N, blk_n_len, unit_len, &_n_it);
	iter_by_blk_spec(K, blk_k_len, unit_len, &_k_it);
	union {
		char mem[sizeof(nest_iter_t) + sizeof(iter_t)*2];
		nest_iter_t nit;
	} it2_union = { .nit = { .depth = 2, .is_end = 0 } };
	nest_iter_t it2 = it2_union.nit;
	it2.iters[0] = _n_it;
	it2.iters[1] = _k_it;
	iter_t *const n_it2 = &it2.iters[0], *const k_it2 = &it2.iters[1];

	/* first packing */
	kernel_state_t kernel_st = {
		.a_pack_cur = a_pack_next,
		.b_pack_cur = b_pack_next,
		.ldc = ldc,
		.k_len = k_len,
	};

	prepack_state_t *current_prepack_st_p = NULL;
	prepack_state_t b_prepack_st = prepack_state_new(
			B, b_pack, n_it2->len, k_it2->len, unit_len, k_it2->len, interval_n, interval_k_in_b);
	prepack_state_t a_prepack_st = prepack_state_new(
			A, a_pack, M, k_it2->len, unit_len, k_it2->len, interval_m, interval_k_in_a);

	do {
		kernel_st.c_cur = C + n_it2->cur;

		if (b_prepack_st.mn_sched_len != 0) {
			pack_all(&b_prepack_st);
		}
		kernel_st.b_pack_cur       = b_pack_next;
		b_prepack_st.next_pack_cur = b_pack;
		b_pack      = kernel_st.b_pack_cur;
		b_pack_next = b_prepack_st.next_pack_cur;

		nest_next(&it2);
		if (!it2.is_end) {
			restart_prepack(&b_prepack_st,
					B + interval_k_in_b*k_it2->pos + interval_n*n_it2->pos, n_it2->len, k_it2->len, k_it2->len);
		}

		do {
			if (current_prepack_st_p == &a_prepack_st) {
				pack_slice(&a_prepack_st);
			}
			kernel_st.a_pack_cur       = a_pack_next;
			a_prepack_st.next_pack_cur = a_pack;
			a_pack      = kernel_st.a_pack_cur;
			a_pack_next = a_prepack_st.next_pack_cur;
			current_prepack_st_p = &a_prepack_st;

			if (!a_prepack_st.mn_len_remained) {
				if (it2.is_end) {
					current_prepack_st_p = NULL;
				} else {
					restart_prepack(&a_prepack_st,
							A + interval_k_in_a*k_it2->pos, M, k_it2->len, k_it2->len);
				}
			}

			for (size_t n_cur = n_pos; n_cur < n_end; n_cur += unit_len) {
				kernel_st.n_sub_len = imin(unit_len, n_end - n_cur);
				kernel_fun(&kernel_st, current_prepack_st_p);
				if (current_prepack_st_p) {
					int s = step_prepack(current_prepack_st_p);
					if (s) {
						current_prepack_st_p = b_prepack_st.mn_len_remained ? b_prepack_st : NULL;
					}
				}
			}
		} while (m_slice_next_len);
	} while (!it2.is_end);

}

#define gemm JOIN(NAMESPACE, SIGN, gemm)
void gemm(const nanoblas_t *nb,
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
