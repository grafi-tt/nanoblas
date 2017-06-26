#include "nanoblas_kernel.h"
#include "nanoblas_prepack.h"
#include "kernel/generic_kernel.h"
#include "internal/macro.h"

#define generic_kernel_fun JOIN(NAMESPACE, FSIZE_PREFIX, generic_kernel_fun_, M_SLICE_LEN, x, N_SLICE_LEN)
__attribute__((optimize("unroll-loops")))
void generic_kernel_fun(kernel_state_t *kernel_st) {
	FTYPE c_buf[M_SLICE_LEN*N_SLICE_LEN];
	FTYPE *restrict c_buf_cur = c_buf;
	FTYPE *restrict c_cur = kernel_st->c_cur;

	const int m_slice_real_len = kernel_st->m_slice_real_len;
	const int n_slice_real_len = kernel_st->n_slice_real_len;

	/* load c */
	for (int i = 0; i < M_SLICE_LEN; i++) {
		for (int j = 0; j < N_SLICE_LEN; j++) {
			c_buf_cur[j] = i < m_slice_real_len && j < n_slice_real_len ? c_cur[j] : 0;
		}
		c_buf_cur += N_SLICE_LEN;
		c_cur = (FTYPE *)((char *)c_cur + kernel_st->ldc);
	}
	c_buf_cur = c_buf;
	c_cur = kernel_st->c_cur;

	const FTYPE *restrict a_pack_cur = kernel_st->a_pack_cur;
	const FTYPE *restrict b_pack_cur = kernel_st->b_pack_cur;
	int k_len = kernel_st->k_len;

	prepack_state_t *prepack_st = current_prepack_p(kernel_st);
	if (prepack_st == NULL) goto nopack_loop;

	const FTYPE *restrict next_cur = prepack_st->next_cur;
	FTYPE *restrict next_pack_cur  = prepack_st->next_pack_cur;

	int slice_pos = 0;
	int sched_len    = prepack_st->sched_len;
	const int next_slice_real_len = prepack_st->next_slice_real_len;
	const int slice_len           = prepack_st->slice_len;
	const ptrdiff_t interval_mn   = prepack_st->interval_mn;
	const ptrdiff_t proceed_k     = prepack_st->interval_k - interval_mn * slice_len;

	k_len -= sched_len * slice_len;

	do {
		for (int i = 0; i < M_SLICE_LEN; i++) {
			FTYPE v = a_pack_cur[i];
			for (int j = 0; j < N_SLICE_LEN; j++) {
				c_buf_cur[j] += v * b_pack_cur[j];
			}
			c_buf_cur += N_SLICE_LEN;
		}
		c_buf_cur = c_buf;
		a_pack_cur += M_SLICE_LEN;
		b_pack_cur += N_SLICE_LEN;
		/* pack */
		FTYPE v = (slice_pos < next_slice_real_len) ? *next_cur : 0;
		*next_pack_cur++ = v;
		next_cur = (const FTYPE *)((const char *)next_cur + interval_mn);
		int moving_up = ++slice_pos == slice_len;
		moving_up = -moving_up;
		sched_len += moving_up;
		next_cur = (const FTYPE *)((const char*)next_cur + (proceed_k & moving_up));
		/* ANDN instruction would be used on Haswell/Piledriver or newer*/
		slice_pos &= ~moving_up;
	} while (sched_len);

	/* save cur */
	prepack_st->next_cur      = next_cur;
	prepack_st->next_pack_cur = next_pack_cur;

	if (k_len == 0) goto loop_end;
	nopack_loop:
	do {
		for (int i = 0; i < M_SLICE_LEN; i++) {
			FTYPE v = a_pack_cur[i];
			for (int j = 0; j < N_SLICE_LEN; j++) {
				c_buf_cur[j] += v * b_pack_cur[j];
			}
			c_buf_cur += N_SLICE_LEN;
		}
		c_buf_cur = c_buf;
		a_pack_cur += M_SLICE_LEN;
		b_pack_cur += N_SLICE_LEN;
	} while (--k_len);
	loop_end:

	/* store c */
	for (int i = 0; i < M_SLICE_LEN; i++) {
		for (int j = 0; j < N_SLICE_LEN; j++) {
			if (i < m_slice_real_len && j < n_slice_real_len) {
				c_cur[j] = c_buf_cur[j];
			}
		}
		c_buf_cur += N_SLICE_LEN;
		c_cur = (FTYPE *)((char *)c_cur + kernel_st->ldc);
	}

	/* save cur */
	kernel_st->a_pack_cur = a_pack_cur;
	kernel_st->b_pack_cur = b_pack_cur;
	kernel_st->c_cur += N_SLICE_LEN;
}

#define generic_pack_fun JOIN(NAMESPACE, FSIZE_PREFIX, generic_pack_fun_, M_SLICE_LEN, x, N_SLICE_LEN)
__attribute__((optimize("unroll-loops")))
void generic_pack_fun(prepack_state_t *prepack_st) {
	const FTYPE *restrict next_cur = prepack_st->next_cur;
	FTYPE *restrict next_pack_cur  = prepack_st->next_pack_cur;

	const int next_slice_real_len = prepack_st->next_slice_real_len;
	const int slice_len           = prepack_st->slice_len;
	const ptrdiff_t interval_mn   = prepack_st->interval_mn;
	const ptrdiff_t proceed_k     = prepack_st->interval_k - interval_mn * slice_len;

	/* pack */
	if (slice_len == M_SLICE_LEN) {
		for (int i = 0; i < prepack_st->sched_len; i++) {
			for (int j = 0; j < M_SLICE_LEN; j++) {
				FTYPE v = (j < next_slice_real_len) ? *next_cur : 0;
				*next_pack_cur++ = v;
				next_cur = (const FTYPE *)((const char *)next_cur + interval_mn);
			}
			next_cur = (const FTYPE *)((const char*)next_cur + proceed_k);
		}
	} else {
		for (int i = 0; i < prepack_st->sched_len; i++) {
			for (int j = 0; j < N_SLICE_LEN; j++) {
				FTYPE v = (j < next_slice_real_len) ? *next_cur : 0;
				*next_pack_cur++ = v;
				next_cur = (const FTYPE *)((const char *)next_cur + interval_mn);
			}
			next_cur = (const FTYPE *)((const char*)next_cur + proceed_k);
		}
	}

	/* save cur */
	prepack_st->next_cur      = next_cur;
	prepack_st->next_pack_cur = next_pack_cur;
}

#define generic_max_sched_len_fun JOIN(NAMESPACE, FSIZE_PREFIX, generic_max_sched_len_fun_, M_SLICE_LEN, x, N_SLICE_LEN)
void generic_max_sched_len_fun(int k_len, int *a_max_sched_len, int *b_max_sched_len) {
	if (a_max_sched_len) *a_max_sched_len = k_len / M_SLICE_LEN;
	if (b_max_sched_len) *b_max_sched_len = k_len / N_SLICE_LEN;
}

#define generic_kernel JOIN(NAMESPACE, FSIZE_PREFIX, generic_kernel_, M_SLICE_LEN, x, N_SLICE_LEN)
kernel_t generic_kernel = {
	.kernel_fun        = &generic_kernel_fun,
	.pack_fun          = &generic_pack_fun,
	.max_sched_len_fun = &generic_max_sched_len_fun,
	.m_slice_len       = M_SLICE_LEN,
	.n_slice_len       = N_SLICE_LEN,
};
