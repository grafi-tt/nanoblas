#include "nanoblas_kernel.h"
#include "nanoblas_prepack.h"
#include "kernel/generic_kernel.h"
#include "internal/macro.h"

#define generic_kernel_mult JOIN(NAMESPACE, FSIZE_PREFIX, generic_kernel_mult_, M_SLICE_LEN, x, N_SLICE_LEN)
void generic_kernel_mult (kernel_state_t *kernel_st) {
	FTYPE *restrict c_buf = kernel_st->c_buf;
	FTYPE *restrict c_buf_cur = c_buf;
	FTYPE *restrict c_cur = kernel_st->c_cur;
	FTYPE *restrict c_next_cur = kernel_st->c_next_cur;

	const int m_slice_real_len = kernel_st->m_slice_real_len;
	const int n_slice_real_len = kernel_st->n_slice_real_len;
	const int m_next_slice_real_len = kernel_st->m_next_slice_real_len;
	const int n_next_slice_real_len = kernel_st->n_next_slice_real_len;

	const FTYPE *restrict a_pack_cur = kernel_st->a_pack_cur;
	const FTYPE *restrict b_pack_cur = kernel_st->b_pack_cur;
	int k_len = kernel_st->k_len;

	prepack_state_t *prepack_st = current_prepack_p(kernel_st);
	if (prepack_st == NULL) goto nopack_loop;

	const FTYPE *restrict next_cur = prepack_st->next_cur;
	FTYPE *restrict next_pack_cur  = prepack_st->next_pack_cur;

	int slice_pos = 0;
	const int next_slice_real_len = prepack_st->next_slice_real_len;
	const int slice_len           = prepack_st->slice_len;
	const ptrdiff_t interval_mn   = prepack_st->interval_mn;
	const ptrdiff_t proceed_k     = prepack_st->interval_k - interval_mn * slice_len;

	int pack_len = prepack_st->remained_len;
	int max_pack_len = prepack_st->slice_len == M_SLICE_LEN ? k_len/M_SLICE_LEN : k_len/N_SLICE_LEN;
	prepack_st->remained_len -= max_pack_len;
	if (pack_len > max_pack_len) pack_len = max_pack_len;
	k_len -= pack_len * slice_len;

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
		next_cur = (const FTYPE *)((uintptr_t)next_cur + interval_mn);
		int moving_up = ++slice_pos == slice_len;
		moving_up = -moving_up;
		pack_len += moving_up;
		next_cur = (const FTYPE *)((uintptr_t)next_cur + (proceed_k & moving_up));
		/* ANDN instruction would be used on Haswell/Piledriver or newer*/
		slice_pos &= ~moving_up;
	} while (pack_len);

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

	/* store current c */
	for (int i = 0; i < M_SLICE_LEN; i++) {
		for (int j = 0; j < N_SLICE_LEN; j++) {
			if (i < m_slice_real_len && j < n_slice_real_len) {
				c_cur[j] = c_buf_cur[j];
			}
		}
		c_buf_cur += N_SLICE_LEN;
		c_cur = (FTYPE *)((uintptr_t)c_cur + kernel_st->ldc);
	}
	c_buf_cur = c_buf;

	/* load next c */
	for (int i = 0; i < M_SLICE_LEN; i++) {
		for (int j = 0; j < N_SLICE_LEN; j++) {
			if (i < m_next_slice_real_len && j < n_next_slice_real_len) {
				c_buf_cur[j] = c_next_cur[j];
			} else {
				c_buf_cur[j] = 0;
			}
		}
		c_buf_cur += N_SLICE_LEN;
		c_next_cur = (FTYPE *)((uintptr_t)c_next_cur + kernel_st->ldc);
	}

	/* save cur */
	kernel_st->a_pack_cur = a_pack_cur;
	kernel_st->b_pack_cur = b_pack_cur;
	kernel_st->c_cur = kernel_st->c_next_cur;
	kernel_st->c_next_cur += N_SLICE_LEN;
}

#define generic_kernel_pack JOIN(NAMESPACE, FSIZE_PREFIX, generic_kernel_pack_, M_SLICE_LEN, x, N_SLICE_LEN)
void generic_kernel_pack(prepack_state_t *prepack_st) {
	const FTYPE *restrict next_cur = prepack_st->next_cur;
	FTYPE *restrict next_pack_cur  = prepack_st->next_pack_cur;

	const int next_slice_real_len = prepack_st->next_slice_real_len;
	const int slice_len           = prepack_st->slice_len;
	const ptrdiff_t interval_mn   = prepack_st->interval_mn;
	const ptrdiff_t proceed_k     = prepack_st->interval_k - interval_mn * slice_len;

	/* pack */
	if (slice_len == M_SLICE_LEN) {
		for (int i = 0; i < prepack_st->remained_len; i++) {
			for (int j = 0; j < M_SLICE_LEN; j++) {
				FTYPE v = (j < next_slice_real_len) ? *next_cur : 0;
				*next_pack_cur++ = v;
				next_cur = (const FTYPE *)((const char *)next_cur + interval_mn);
			}
			next_cur = (const FTYPE *)((const char*)next_cur + proceed_k);
		}
	} else {
		for (int i = 0; i < prepack_st->remained_len; i++) {
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
	prepack_st->remained_len  = 0;
}
