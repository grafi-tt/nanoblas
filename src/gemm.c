/**
 * \file
 * \brief Implementation of GEMM
 */

#include <stdint.h>
#include "nanoblas.h"
#include "nanoblas_kernel.h"
#include "nanoblas_prepack.h"
#include "internal/macro.h"
#include "internal/iter.h"
#include "internal/kernel.h"
#include "internal/prepack.h"
#include "internal/util.h"

typedef struct {
	const FTYPE *const A;
	const FTYPE *A_pos;
	const FTYPE *A_next_k;
	const FTYPE *const B;
	const FTYPE *B_next_k;
	FTYPE *const C;
	FTYPE *C_next;
	kernel_mult_t *const kernel_mult;
	kernel_pack_t *const kernel_pack;
	kernel_state_t kernel_st;
	circular_iter_t *const m_it;
	circular_iter_t *const m_it_bak;
	circular_iter_t *const n_it;
	circular_iter_t *const n_it_bak;
	circular_iter_t *const k_it;
	circular_iter_t *const k_it_bak;
} gemm_state_t;

static void k_step(gemm_state_t *st) {
	st->A_pos = (const FTYPE *)((uintptr_t)
			st->A + st->kernel_st.prepack.mem.interval_k_in_a * st->k_it_bak->pos);
	st->A_next_k = (const FTYPE *)((uintptr_t)
			st->A + st->kernel_st.prepack.mem.interval_k_in_a * st->k_it->pos);
	st->B_next_k = (const FTYPE *)((uintptr_t)
			st->B + st->kernel_st.prepack.mem.interval_k_in_b * st->k_it->pos);
}

static void n_step(gemm_state_t *st) {
	swap_b_pack(&st->kernel_st, st->kernel_pack);
	limit_prepack(current_prepack_p(&st->kernel_st), st->n_it->len);
}

static void last_n_step_before_k(gemm_state_t *st) {
	swap_b_pack(&st->kernel_st, st->kernel_pack);
	reset_prepack(current_prepack_p(&st->kernel_st), st->B_next_k, st->n_it->len, st->k_it->len);
}

static void last_n_step(gemm_state_t *st) {
	swap_b_pack(&st->kernel_st, st->kernel_pack);
	set_packed(current_prepack_p(&st->kernel_st));
}

static void m_step(gemm_state_t *st) {
	swap_a_pack(&st->kernel_st, st->kernel_pack);
	proceed_prepack(current_prepack_p(&st->kernel_st), st->m_it->len);

	st->kernel_st.c_cur = st->C_next;
	const int m_slice_len = st->kernel_st.prepack.mem.m_slice_len;
	st->C_next = (FTYPE *)((uintptr_t)st->C_next + st->kernel_st.ldc * m_slice_len);
}

static void last_m_step_before_n(gemm_state_t *st) {
	swap_a_pack(&st->kernel_st, st->kernel_pack);
	restart_prepack(current_prepack_p(&st->kernel_st), st->A_pos, st->m_it->len);

	st->kernel_st.c_cur = st->C_next;
	st->C_next = st->C + st->n_it->pos;
}

static void last_m_step_before_k(gemm_state_t *st) {
	swap_a_pack(&st->kernel_st, st->kernel_pack);
	reset_prepack(current_prepack_p(&st->kernel_st), st->A_next_k, st->m_it->len, st->k_it->len);

	st->kernel_st.c_cur = st->C_next;
	st->C_next = st->C;
}

static void last_m_step(gemm_state_t *st) {
	swap_a_pack(&st->kernel_st, st->kernel_pack);
	st->kernel_st.current_prepack = 0;

	st->kernel_st.c_cur = st->C_next;
}

static void kernel_loop(gemm_state_t *st) {
#ifdef HAS_BUILTIN_PREFETCH
	const int m_slice_len = st->kernel_st.prepack.mem.m_slice_len;
	const FTYPE *c_next_cur = st->C_next;
	for (int i = 0; i < m_slice_len; i++) {
		__builtin_prefetch(c_next_cur, 1, 0);
		c_next_cur = (FTYPE *)((uintptr_t)c_next_cur + st->kernel_st.ldc);
	}
#endif

	const size_t n_pos = st->n_it_bak->pos;
	const size_t n_end = n_pos + st->n_it_bak->len;
	const int n_slice_len = st->kernel_st.prepack.mem.n_slice_len;

	for (size_t n_cur = n_pos; n_cur < n_end; n_cur += n_slice_len) {
		set_kernel_info(&st->kernel_st, st->m_it_bak->len, (int)(n_end - n_cur), st->k_it_bak->len);

		st->kernel_mult(&st->kernel_st);

		st->kernel_st.a_pack_cur = st->kernel_st.a_pack;
		update_prepack(&st->kernel_st);
	}
	st->kernel_st.b_pack_cur = st->kernel_st.b_pack;
}

/**
 * \brief gemm
 */
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

	/* get kernel */
	kernel_t kernel = nb->kernel;
	const int m_slice_len = kernel.m_slice_len;
	const int n_slice_len = kernel.n_slice_len;
	const int k_unit_len = kernel.k_unit_len;

	/* get block size */
	const size_t blk_n_max_len = ((nb->blk_n_max_len - 1)/n_slice_len + 1) * n_slice_len;
	const size_t blk_k_max_len = ((nb->blk_k_max_len - 1)/k_unit_len + 1) * k_unit_len;

	/* allocate pack (VLA) */
	FTYPE mem[2*m_slice_len*blk_k_max_len + 2*blk_n_max_len*blk_k_max_len + 32/sizeof(FTYPE)];
	FTYPE *mem_aligned = (FTYPE *)((uintptr_t)mem + (-(uintptr_t)mem & 31));
	FTYPE *a_pack      = mem_aligned;
	FTYPE *a_next_pack = mem_aligned +   m_slice_len*blk_k_max_len;
	FTYPE *b_pack      = mem_aligned + 2*m_slice_len*blk_k_max_len;
	FTYPE *b_next_pack = mem_aligned + 2*m_slice_len*blk_k_max_len + blk_n_max_len*blk_k_max_len;

	/* get interval */
	const ptrdiff_t interval_m      = TransA == CblasNoTrans ? lda : 1;
	const ptrdiff_t interval_k_in_a = TransA == CblasNoTrans ? 1 : lda;
	const ptrdiff_t interval_k_in_b = TransB == CblasNoTrans ? ldb : 1;
	const ptrdiff_t interval_n      = TransB == CblasNoTrans ? 1 : ldb;

	/* get iterator */
	circular_iter_t m_it = simple_iter(M, m_slice_len);
	circular_iter_t m_it_bak;
	circular_iter_t n_it = blk_spec_iter(N, blk_n_max_len, n_slice_len);
	circular_iter_t n_it_bak;
	circular_iter_t k_it = blk_spec_iter(K, blk_k_max_len, k_unit_len);
	circular_iter_t k_it_bak;

	gemm_state_t st = {
		.A = A,
		.B = B,
		.C = C,
		.C_next = C,
		.kernel_mult = kernel.mult,
		.kernel_pack = kernel.pack,
		.kernel_st = kernel_state_new(
				A, B, interval_m, interval_n, interval_k_in_a, interval_k_in_b, ldc,
				a_pack, a_next_pack, b_pack, b_next_pack, m_slice_len, n_slice_len,
				m_it.len, n_it.len, k_it.len),
		.m_it     = &m_it,
		.m_it_bak = &m_it_bak,
		.n_it     = &n_it,
		.n_it_bak = &n_it_bak,
		.k_it     = &k_it,
		.k_it_bak = &k_it_bak,
	};

	do {
		k_it_bak = k_it;
		next(&k_it);
		k_step(&st);
		do {
			n_it_bak = n_it;
			if (!next(&n_it))
				n_step(&st);
			else if (k_it.pos != 0)
				last_n_step_before_k(&st);
			else
				last_n_step(&st);
			do {
				m_it_bak = m_it;
				if (!next(&m_it))
					m_step(&st);
				else if (n_it.pos != 0)
					last_m_step_before_n(&st);
				else if (k_it.pos != 0)
					last_m_step_before_k(&st);
				else
					last_m_step(&st);
				kernel_loop(&st);
			} while (m_it.pos != 0);
		} while (n_it.pos != 0);
	} while (k_it.pos != 0);
}
