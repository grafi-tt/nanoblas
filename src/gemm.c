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
	FTYPE *C_cur;
	FTYPE *C_next;
	kernel_fun_t *const kernel_fun;
	pack_fun_t *const pack_fun;
	max_sched_len_fun_t *const max_sched_len_fun;
	kernel_state_t kernel_st;
	circular_iter_t *const m_it;
	circular_iter_t *const m_it_bak;
	circular_iter_t *const n_it;
	circular_iter_t *const n_it_bak;
	circular_iter_t *const k_it;
	circular_iter_t *const k_it_bak;
} gemm_state_t;

static void k_step(gemm_state_t *st) {
	st->A_pos = (const FTYPE *)((const char *)
			st->A + st->kernel_st.prepack.mem.interval_k_in_a * st->k_it_bak->pos);
	st->A_next_k = (const FTYPE *)((const char *)
			st->A + st->kernel_st.prepack.mem.interval_k_in_a * st->k_it->pos);
	st->B_next_k = (const FTYPE *)((const char *)
			st->B + st->kernel_st.prepack.mem.interval_k_in_b * st->k_it->pos);
}

static void n_step(gemm_state_t *st) {
	swap_b_pack(&st->kernel_st, st->pack_fun);
}

static void last_n_step_before_k(gemm_state_t *st) {
	int b_max_sched_len;
	st->max_sched_len_fun(st->k_it->len, NULL, &b_max_sched_len);
	reset_prepack(current_prepack_p(&st->kernel_st), st->B_next_k, st->n_it->len, st->k_it->len, b_max_sched_len);
}

static void last_n_step(gemm_state_t *st) {
	set_slice_packed(current_prepack_p(&st->kernel_st));
}

static void m_step(gemm_state_t *st) {
	swap_a_pack(&st->kernel_st, st->pack_fun);

	const int m_slice_len = st->kernel_st.prepack.mem.m_slice_len;
	st->C_cur = st->C_next;
	st->C_next = (FTYPE *)((char *)st->C_next + st->kernel_st.ldc * m_slice_len);
}

static void last_m_step_before_n(gemm_state_t *st) {
	restart_prepack(current_prepack_p(&st->kernel_st), st->A_pos, st->m_it->len);
	st->C_next = st->C + st->n_it->pos;
}

static void last_m_step_before_k(gemm_state_t *st) {
	int a_max_sched_len;
	st->max_sched_len_fun(st->k_it->len, &a_max_sched_len, NULL);
	reset_prepack(current_prepack_p(&st->kernel_st), st->A_next_k, st->m_it->len, st->k_it->len, a_max_sched_len);
	st->C_next = st->C;
}

static void last_m_step(gemm_state_t *st) {
	st->kernel_st.current_prepack = 0;
}

static void kernel_loop(gemm_state_t *st) {
#ifdef HAS_BUILTIN_PREFETCH
	const int m_slice_len = st->kernel_st.prepack.mem.m_slice_len;
	const FTYPE *c_next_cur = st->C_next;
	for (int i = 0; i < m_slice_len; i++) {
		__builtin_prefetch(c_next_cur, 1, 0);
		c_next_cur += st->kernel_st.ldc;
	}
#endif

	const size_t n_pos = st->n_it_bak->pos;
	const size_t n_end = n_pos + st->n_it_bak->len;
	const int n_slice_len = st->kernel_st.prepack.mem.n_slice_len;

	for (size_t n_cur = n_pos; n_cur < n_end; n_cur += n_slice_len) {
		set_kernel_info(&st->kernel_st, st->C_cur, st->m_it_bak->len, (int)(n_end - n_cur), st->k_it_bak->len);

		st->kernel_fun(&st->kernel_st);

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

	/* get block size */
	const size_t blk_n_max_len = nb->blk_n_max_len;
	const size_t blk_k_max_len = nb->blk_k_max_len;

	/* allocate pack (VLA) */
	FTYPE mem[2*m_slice_len*blk_k_max_len + 2*blk_n_max_len*blk_k_max_len + 32/sizeof(FTYPE)];
	int mem_offset = ((uintptr_t)mem & 32)/sizeof(FTYPE);
	FTYPE *mem_aligned = mem_offset ? mem + 32/sizeof(FTYPE) - mem_offset : mem;
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
	circular_iter_t k_it = blk_spec_iter(K, blk_k_max_len, 1);
	circular_iter_t k_it_bak;

	/* get max scheduling length */
	int a_max_sched_len, b_max_sched_len;
	kernel.max_sched_len_fun(k_it.len, &a_max_sched_len, &b_max_sched_len);

	gemm_state_t st = {
		.A = A,
		.B = B,
		.C = C,
		.C_next = C,
		.kernel_fun        = kernel.kernel_fun,
		.pack_fun          = kernel.pack_fun,
		.max_sched_len_fun = kernel.max_sched_len_fun,
		.kernel_st = kernel_state_new(
				A, B, interval_m, interval_n, interval_k_in_a, interval_k_in_b, ldc,
				a_pack, a_next_pack, b_pack, b_next_pack, m_slice_len, n_slice_len,
				m_it.len, n_it.len, k_it.len, a_max_sched_len, b_max_sched_len),
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
			next(&n_it);
			n_step(&st);
			if (n_it.pos == 0) last_n_step_before_k(&st);
			if (n_it.pos == 0 && k_it.pos == 0) last_n_step(&st);
			do {
				m_it_bak = m_it;
				next(&m_it);
				m_step(&st);
				if (m_it.pos == 0) last_m_step_before_n(&st);
				if (m_it.pos == 0 && n_it.pos == 0) last_m_step_before_k(&st);
				if (m_it.pos == 0 && n_it.pos == 0 && k_it.pos == 0) last_m_step(&st);
				kernel_loop(&st);
			} while (m_it.pos != 0);
		} while (n_it.pos != 0);
	} while (k_it.pos != 0);
}
