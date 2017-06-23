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
#include "internal/prepack.h"
#include "internal/util.h"

typedef struct {
	const FTYPE *A_pos;
	const FTYPE *A_next_k;
	const FTYPE *B_next_k;
	FTYPE *const C;
	FTYPE *C_next;
	const ptrdiff_t interval_k_in_a;
	const ptrdiff_t interval_k_in_b;
	FTYPE *a_pack;
	FTYPE *a_next_pack;
	FTYPE *b_pack;
	FTYPE *b_next_pack;
	kernel_fun_t *const kernel_fun;
	kernel_state_t kernel_st;
	prepack_state_t a_prepack_st;
	prepack_state_t b_prepack_st;
	prepack_state_t *current_prepack_st_p;
	int prepack_n_remained_len;
	size_t n_pos;
	circular_iter_t *const m_it;
	circular_iter_t *const n_it;
	circular_iter_t *const k_it;
} gemm_state_t;

static void before_k_step(gemm_state_t *st) {
	st->A_pos = st->A_next_k;
	st->A_next_k += st->interval_k_in_a * st->k_it->len;
	st->B_next_k += st->interval_k_in_b * st->k_it->len;
	st->kernel_st.k_len = st->k_it->len;
}

static void before_n_step(gemm_state_t *st) {
	st->n_pos = st->n_it->pos;
}

static void n_step(gemm_state_t *st) {
	const int n_slice_len = st->b_prepack_st.mn_slice_len;

	while (st->prepack_n_remained_len > 0) {
		pack_all(&st->b_prepack_st);
		st->prepack_n_remained_len -= n_slice_len;
		limit_prepack(&st->b_prepack_st, st->prepack_n_remained_len);
	}
	st->prepack_n_remained_len = st->n_it->len;
	limit_prepack(&st->b_prepack_st, st->prepack_n_remained_len);

	fswap(&st->b_pack, &st->b_next_pack);
	st->kernel_st.b_pack_cur       = st->b_pack;
	st->b_prepack_st.next_pack_cur = st->b_next_pack;
}

static void last_n_step_before_k(gemm_state_t *st) {
	reset_prepack(&st->b_prepack_st, st->B_next_k, st->n_it->len, st->k_it->len, st->k_it->len);
}

static void last_n_step(gemm_state_t *st) {
	st->prepack_n_remained_len = 0;
}

static void before_m_step(gemm_state_t *st) {
	st->kernel_st.m_slice_real_len = st->m_it->len;
}

static void m_step(gemm_state_t *st) {
	if (st->current_prepack_st_p == &st->a_prepack_st) {
		pack_all(&st->a_prepack_st);
	}
	limit_prepack(&st->a_prepack_st, st->m_it->len);

	fswap(&st->a_pack, &st->a_next_pack);
	st->kernel_st.a_pack_cur       = st->a_pack;
	st->a_prepack_st.next_pack_cur = st->a_next_pack;

	st->current_prepack_st_p = &st->a_prepack_st;

	const int m_slice_len = st->a_prepack_st.mn_slice_len;
	st->kernel_st.c_cur = st->C_next;
	st->C_next += st->kernel_st.ldc * m_slice_len;
}

static void last_m_step_before_n(gemm_state_t *st) {
	restart_prepack(&st->a_prepack_st, st->A_pos, st->m_it->len);
	st->C_next = st->C + st->n_it->pos;
}

static void last_m_step_before_k(gemm_state_t *st) {
	reset_prepack(&st->a_prepack_st, st->A_next_k, st->m_it->len, st->k_it->len, st->k_it->len);
	st->C_next = st->C;
}

static void last_m_step(gemm_state_t *st) {
	st->current_prepack_st_p = NULL;
}

static void kernel_loop(gemm_state_t *st) {
#ifdef HAS_BUILTIN_PREFETCH
	const int m_slice_len = st->a_prepack_st.mn_slice_len;
	const FTYPE *c_next_cur = st->C_next;
	for (int i = 0; i < m_slice_len; i++) {
		__builtin_prefetch(c_next_cur, 1, 0);
		c_next_cur += st->kernel_st.ldc;
	}
#endif

	size_t n_end = st->n_it->pos;
	if (n_end == 0) n_end = st->n_it->sum;
	const int n_slice_len = st->b_prepack_st.mn_slice_len;
	for (size_t n_cur = st->n_pos; n_cur < n_end; n_cur += n_slice_len) {
		st->kernel_st.n_slice_real_len = imin(n_slice_len, (int)(n_end - n_cur));
		st->kernel_fun(&st->kernel_st, st->current_prepack_st_p);
		st->kernel_st.a_pack_cur = st->a_pack;

		if (st->current_prepack_st_p) {
			if (step_prepack(st->current_prepack_st_p)) {
				if (st->current_prepack_st_p == &st->a_prepack_st) {
					st->current_prepack_st_p = &st->b_prepack_st;
				} else {
					st->prepack_n_remained_len -= st->b_prepack_st.mn_slice_real_len;
					limit_prepack(&st->b_prepack_st, st->prepack_n_remained_len);
				}
				if (st->prepack_n_remained_len <= 0) {
					st->current_prepack_st_p = NULL;
				}
			}
		}
	}
	st->kernel_st.b_pack_cur = st->b_pack;
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
	kernel_fun_t *const kernel_fun = kernel.fun;

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
	circular_iter_t n_it = blk_spec_iter(N, blk_n_max_len, n_slice_len);
	circular_iter_t k_it = blk_spec_iter(K, blk_k_max_len, 1);

	gemm_state_t st = {
		/* .A_pos, */
		.A_next_k = A,
		.B_next_k = B,
		.C = C,
		.C_next = C,
		.interval_k_in_a = interval_k_in_a,
		.interval_k_in_b = interval_k_in_b,
		.a_pack = a_pack,
		.a_next_pack = a_next_pack,
		.b_pack = b_pack,
		.b_next_pack = b_next_pack,
		.kernel_fun = kernel_fun,
		.kernel_st = {
			/* .a_pack_cur, */
			/* .b_pack_cur, */
			/* .c_cur, */
			.ldc = ldc,
			/* .m_slice_real_len, */
			/* .n_slice_real_len, */
			/* .k_len, */
		},
		.a_prepack_st = prepack_state_new(
				A, a_next_pack, m_slice_len, M,
				k_it.len, k_it.len, interval_m, interval_k_in_a),
		.b_prepack_st = prepack_state_new(
				B, b_next_pack, n_slice_len, n_it.len,
				k_it.len, k_it.len, interval_n, interval_k_in_b),
		.current_prepack_st_p = &st.a_prepack_st,
		.prepack_n_remained_len = n_it.len,
		/* .n_pos, */
		.m_it = &m_it,
		.n_it = &n_it,
		.k_it = &k_it,
	};

	do {
		before_k_step(&st);
		next(&k_it);
		do {
			before_n_step(&st);
			next(&n_it);
			n_step(&st);
			if (n_it.pos == 0) last_n_step_before_k(&st);
			if (n_it.pos == 0 && k_it.pos == 0) last_n_step(&st);
			do {
				before_m_step(&st);
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
