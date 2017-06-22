/**
 * \file
 * \brief Implementation of GEMM
 */

#include <stdbool.h>
#include <stdint.h>
#include "nanoblas.h"
#include "nanoblas_kernel.h"
#include "nanoblas_prepack.h"
#include "internal/iter.h"
#include "internal/macro.h"
#include "internal/prepack.h"
#include "internal/util.h"

typedef struct {
	const FTYPE *A_pos;
	const FTYPE *A_next_k;
	const FTYPE *B_pos;
	const FTYPE *B_next_k;
	const FTYPE *const C;
	const FTYPE *C_next;
	FTYPE *a_pack;
	FTYPE *a_next_pack;
	FTYPE *b_pack;
	FTYPE *b_next_pack;
	kernel_state_t kernel_st;
	prepack_state_t a_prepack_st;
	prepack_state_t b_prepack_st;
	prepack_state_t *current_prepack_st;
	int prepack_n_remained_len;
	iter_t *const m_it;
	iter_t *const n_it;
	iter_t *const k_it;
} gemm_state_t;

static void entry(gemm_state_t *st) {
	st->A_pos = st->A_next;
	st->B_pos = st->B_next;
	st->A_next_k += st->a_prepack_st.interval_k * st->k_it->len;
	st->B_next_k += st->b_prepack_st.interval_k * st->k_it->len;
	st->kernel_st->k_len = st->k_it->len;
}

static void n_step(gemm_state_t *st) {
	const int n_slice_len = st->b_prepack_st->mn_slice_len;
	const FSIZE *const b_next = st->B_pos + st->n_it->pos;

	while (st->prepack_n_remained_len > 0) {
		pack_all(&st->b_prepack_st);
		const FSIZE *b_next = st->b_prepack_st.next + st->b_prepack_st.interval_mn * n_slice_len;
		const n_slice_real_len = imin(n_slice_len, st->prepack_n_remained_len);
		advance_prepack(b_next, n_slice_real_len);
		st->prepack_n_remained_len -= n_slice_len;
	}
	restart_prepack_light(&st->b_prepack_st, st->B_pos + st->n_it->pos, imin(n_slice_len, st->n_it->len));

	fswap(&st->b_pack, &st->b_next_pack);
	kernel_st->b_pack_cur       = st->b_pack;
	b_prepack_st->next_pack_cur = st->b_next_pack;
}

static void n_last_step_before_k(gemm_state_t *st) {
	restart_prepack(&st->b_prepack_st, st->B_next_k, st->k_it.len, st->k_it.len);
}

static void m_step(gemm_state_t *st) {
	const size_t m_slice_len = st->a_prepack_st->mn_slice_len;
	st->kernel_st.c_cur = st->C_next;
	st->C_next += st->ldc * m_slice_len;

	if (st->current_prepack_st_p == &st->a_prepack_st) {
		pack_all(&st->a_prepack_st);
	}
	restart_prepack_light(&st->a_prepack_st, ) {
	}

	fswap(&st->a_pack, &st->a_next_pack);
	st->kernel_st.a_pack_cur       = st->a_pack;
	st->a_prepack_st.next_pack_cur = st->a_next_pack;

	st->a_prepack_st.mn_slice_real_len = st->m_it.len;
}

static void m_last_step_before_n(gemm_state_t *st) {
	restart_prepack_light(&st->a_prepack_st, st->A_pos, st->m_it->len);
	st->C_next = C + st->n_it->pos;
}

static void last_m_step_before_k(gemm_state_t *st) {
	restart_prepack(&st->a_prepack_st, st->A_next_k, st->m_it->len, st->k_it->len, st->k_it->len);
	st->C_next = C;
}

static void kernel_loop(gemm_state_t *st) {
#ifdef HAS_BUILTIN_PREFETCH
	size_t m_slice_len = st->a_prepack_st->mn_slice_len;
	const FSIZE *c_next_cur = st->C_next;
	for (int i = 0; i < m_slice_len; i++) {
		__builtin_prefetch(c_next_cur, 1, 0);
		c_next_cur += ldc;
	}
#endif

	size_t n_slice_len = st->b_prepack_st->mn_slice_len;
	for (size_t n_cur = st->n_pos; n_cur < st->n_it.pos; n_cur += n_slice_len) {

		st->kernel_st.n_slice_real_len = imin(n_slice_len, st->n_it.pos - n_cur);
		st->kernel_fun(st->kernel_st, *st->current_prepack_st_p);

		if (*st->current_prepack_st_p) {
			int s = step_prepack(*st->current_prepack_st_p);
			if (s) {
				if (*st->current_prepack_st_p == st->a_prepack_st) {
					*st->current_prepack_st_p = st->b_prepack_st;
				} else {
					prepack_n_remained_len -= b_prepack_st->mn_slice_real_len;
					b_prepack_st->mn_slice_real_len = imin(n_slice_len, prepack_n_remained_len);
				}
				if (b_prepack_st->next_cur == b_next_cur_end) {
					*st->current_prepack_st_p = NULL;
				}
			}
		}
	}
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
	FTYPE *b_next_pack = mem_aligned + 2*m_slice_len*blk_k_max_len + blk_n_max_len+blk_k_max_len;

	/* get interval */
	const ptrdiff_t interval_m = TransA ? 1 : lda;
	const ptrdiff_t interval_k_in_a = TransA ? lda : 1;
	const ptrdiff_t interval_k_in_b = TransB ? 1 : ldb;
	const ptrdiff_t interval_n = TransB ? ldb : 1;

	/* get iterator */
	iter_t m_it;
	iter_t n_it;
	iter_t k_it;
	iter_by_blk_spec(M, m_slice_len, m_slice_len, &m_it);
	iter_by_blk_spec(N, blk_n_max_len, n_slice_len, &n_it);
	iter_by_blk_spec(K, blk_k_max_len, 1, &k_it);

	gemm_state_t st = {
		/* .A_pos, */
		.A_next = A,
		/* .B_pos, */
		.B_next = B,
		.C = C,
		.C_next = C,
		.a_pack = a_pack,
		.a_next_pack = a_next_pack,
		.b_pack = b_pack,
		.b_next_pack = b_next_pack,
		.kernel_st = {
			/* .a_pack_cur, */
			/* .b_pack_cur, */
			/* .c_cur, */
			.ldc = ldc,
			/* .m_slice_real_len, */
			/* .n_slice_real_len, */
			/* .k_len, */
		},
		.a_prepack_st = {
			/* .next, */
			/* .next_pack_cur, */
			.mn_slice_pos = 0,
			/* .sched_size, */
			/* .mn_slice_real_len, */
			.mn_slice_len = m_slice_len,
			.interval_mn = interval_m,
			.proceed_k = interval_k_in_a - m_slice_len * interval_mn,
			/* .slice_size, */
			/* .max_sched_size, */
		},
		.b_prepack_st = {
			/* .next, */
			/* .next_pack_cur, */
			.mn_slice_pos = 0,
			/* .sched_size, */
			/* .mn_slice_real_len, */
			.mn_slice_len = n_slice_len,
			.interval_mn = interval_n,
			.proceed_k = interval_k_in_n - n_slice_len * interval_mn,
			/* .slice_size, */
			/* .max_sched_size, */
		},
		.current_prepack_state = &st.a_prepack_st;
		.m_it = &m_it,
		.n_it = &n_it,
		.k_it = &k_it,
	}

	do {
		entry(&st);
		next(&k_it);
		do {
			next(&n_it);
			n_step(&st);
			if (n_it.pos == 0) n_last_step_before_k(&st);
			do {
				next(&m_it);
				if (m_it.pos == 0) m_last_step_before_n(&st);
				if (m_it.pos == 0 && n_it.pos == 0) m_last_step_before_k(&st);
				kernel_loop(&st);
			} while (m_it.pos != 0);
		} while (n_it.pos != 0);
	} while (k_it.pos != 0);
}
