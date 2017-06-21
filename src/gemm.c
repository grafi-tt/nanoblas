#include <stdbool.h>
#include <stdint.h>
#include "nanoblas.h"
#include "nanoblas_kernel.h"
#include "nanoblas_prepack.h"
#include "internal/iter.h"
#include "internal/macro.h"
#include "internal/prepack.h"
#include "internal/util.h"

#define gemm JOIN(NAMESPACE, SIGN, gemm)
void gemm(const nanoblas_t *nb,
		enum CBLAS_ORDER Order, enum CBLAS_TRANSPOSE TransA, enum CBLAS_TRANSPOSE TransB,
		size_t M, size_t N, size_t K,
		FTYPE alpha, const FTYPE *A, ptrdiff_t lda, const FTYPE *B, ptrdiff_t ldb,
		FTYPE beta, FTYPE *C, ptrdiff_t ldc) {

	(void)alpha;
	(void)beta;

	if (Order == CblasColMajor) {
		enum CBLAS_TRANSPOSE tmp_tr = TransA;
		TransA = TransB;
		TransB = tmp_tr;
		size_t tmp_s = M;
		M = N;
		N = tmp_s;
		ptrdiff_t tmp_pd = lda;
		lda = ldb;
		ldb = tmp_pd;
	}

	/* get interval */
	const ptrdiff_t interval_m = TransA ? 1 : lda;
	const ptrdiff_t interval_k_in_a = TransA ? lda : 1;
	const ptrdiff_t interval_k_in_b = TransB ? 1 : ldb;
	const ptrdiff_t interval_n = TransB ? ldb : 1;

	/* get block size */
	const size_t blk_n_max_len = nb->blk_n_max_len;
	const size_t blk_k_max_len = nb->blk_k_max_len;

	/* get kernel */
	kernel_t kernel = nb->kernel;
	const int m_slice_len = kernel.m_slice_len;
	const int n_slice_len = kernel.n_slice_len;
	kernel_fun_t *const kernel_fun = kernel.fun;

	/* allocate pack (VLA) */
	FTYPE mem[2*m_slice_len*blk_k_max_len + 2*blk_n_max_len*blk_k_max_len + 32/sizeof(FTYPE)];
	int mem_offset = ((uintptr_t)mem & 32)/sizeof(FTYPE);
	FTYPE *mem_aligned = mem_offset ? mem + 32/sizeof(FTYPE) - mem_offset : mem;
	FTYPE *a_pack      = mem_aligned;
	FTYPE *a_next_pack = mem_aligned +   m_slice_len*blk_k_max_len;
	FTYPE *b_pack      = mem_aligned + 2*m_slice_len*blk_k_max_len;
	FTYPE *b_next_pack = mem_aligned + 2*m_slice_len*blk_k_max_len + blk_n_max_len+blk_k_max_len;

	/* get iterator */
	union {
		char mem[sizeof(nest_iter_t) + sizeof(iter_t)*2];
		nest_iter_t it2;
	} it2_union = { .it2 = { .depth = 2, .is_end = 0 } };
	nest_iter_t *const it2 = &it2_union.it2;
	iter_t *const n_it2 = &it2->iters[0];
	iter_t *const k_it2 = &it2->iters[1];
	iter_by_blk_spec(N, blk_n_max_len, n_slice_len, n_it2);
	iter_by_blk_spec(K, blk_k_max_len, 1, k_it2);

	/* preparing for pack swapping; other members are filled later */
	kernel_state_t kernel_st = {
		.a_pack_cur = a_pack,
		.b_pack_cur = b_pack,
		.ldc = ldc,
	};

	prepack_state_t *current_prepack_st_p = NULL;
	prepack_state_t a_prepack_st = prepack_state_new(
			A, a_next_pack, M, k_it2->len, m_slice_len, k_it2->len, interval_m, interval_k_in_a);
	prepack_state_t b_prepack_st = prepack_state_new(
			B, b_next_pack, n_it2->len, k_it2->len, n_slice_len, k_it2->len, interval_n, interval_k_in_b);

	do {
		kernel_st.c_cur = C + n_it2->pos;
		kernel_st.k_len = k_it2->len;

		if (b_prepack_st.mn_slice_len_real != 0) {
			pack_all(&b_prepack_st);
		}
		fswap(&b_pack, &b_next_pack);
		kernel_st.b_pack_cur       = b_pack;
		b_prepack_st.next_pack_cur = b_next_pack;

		nest_next(it2);
		if (!it2->is_end) {
			restart_prepack(&b_prepack_st,
					B + interval_k_in_b*k_it2->pos + interval_n*n_it2->pos, n_it2->len, k_it2->len, k_it2->len);
		}

		int m_slice_next_len = a_prepack_st.mn_slice_len_real;
		do {
			if (current_prepack_st_p == &a_prepack_st) {
				pack_slice(&a_prepack_st);
			}
			fswap(&a_pack, &a_next_pack);
			kernel_st.a_pack_cur       = a_pack;
			a_prepack_st.next_pack_cur = a_next_pack;

			kernel_st.m_slice_len = m_slice_next_len;
			m_slice_next_len = a_prepack_st.mn_slice_len_real;

			if (m_slice_next_len == 0) {
				if (it2->is_end) {
					current_prepack_st_p = NULL;
				} else {
					restart_prepack(&a_prepack_st,
							A + interval_k_in_a*k_it2->pos, M, k_it2->len, k_it2->len);
				}
			}

			size_t n_pos = n_it2->pos;
			size_t n_end = n_it2->pos + n_it2->len;
			for (size_t n_cur = n_pos; n_cur < n_end; n_cur += n_slice_len) {
				kernel_st.n_slice_len = imin(n_slice_len, n_end - n_cur);
				kernel_fun(&kernel_st, current_prepack_st_p);
				if (current_prepack_st_p) {
					int s = step_prepack(current_prepack_st_p);
					if (s) {
						current_prepack_st_p =
							b_prepack_st.mn_slice_len_real == 0 ? NULL : &b_prepack_st;
					}
				}
			}
		} while (m_slice_next_len != 0);
	} while (!it2->is_end);
}
