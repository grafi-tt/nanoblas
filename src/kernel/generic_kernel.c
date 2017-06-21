#include "nanoblas_kernel.h"
#include "nanoblas_prepack.h"
#include "kernel/generic_kernel.h"
#include "internal/macro.h"

#define generic_kernel_fun JOIN(NAMESPACE, PREFIX, generic_kernel_fun_, SLICE_M_LEN, x, SLICE_N_LEN)
__attribute__((optimize("unroll-loops")))
void generic_kernel_fun(kernel_state_t *kernel_st, prepack_state_t *prepack_st) {

	FTYPE c_buf[M_SLICE_LEN*N_SLICE_LEN];
	FTYPE *restrict c_buf_cur = c_buf;
	FTYPE *restrict c_cur = kernel_st->c_cur;

	/* load c */
	for (int i = 0; i < M_SLICE_LEN; i++) {
		for (int j = 0; j < N_SLICE_LEN; j++) {
			c_buf_cur[j] = c_cur[j];
		}
		c_buf_cur += N_SLICE_LEN;
		c_cur += kernel_st->ldc;
	}
	c_buf_cur = c_buf;
	c_cur = kernel_st->c_cur;

	const FTYPE *restrict a_pack_cur = kernel_st->a_pack_cur;
	const FTYPE *restrict b_pack_cur = kernel_st->b_pack_cur;
	int k_len = kernel_st->k_len;

	if (prepack_st == NULL) goto nopack_loop;

	const FTYPE *restrict next_cur = prepack_st->next_cur;
	FTYPE *restrict next_pack_cur  = prepack_st->next_pack_cur;

	int mn_slice_pos = prepack_st->mn_slice_pos;
	int sched_size   = prepack_st->sched_size;
	const int mn_slice_len_real    = prepack_st->mn_slice_len_real;
	const int mn_slice_len_virtual = prepack_st->mn_slice_len_virtual;
	const ptrdiff_t interval_mn    = prepack_st->interval_mn;
	const ptrdiff_t proceed_k      = prepack_st->proceed_k;

	k_len -= sched_size;

	do {
		for (int i = 0; i < M_SLICE_LEN; i++) {
			FTYPE v = a_pack_cur[i];
			for (int j = 0; j < N_SLICE_LEN; j++) {
				c_buf_cur[j] = v * b_pack_cur[j];
			}
			c_buf_cur += N_SLICE_LEN;
		}
		c_buf_cur = c_buf;
		a_pack_cur += M_SLICE_LEN;
		b_pack_cur += N_SLICE_LEN;
		/* pack */
		FTYPE v = (mn_slice_pos < mn_slice_len_real) ? *next_cur : 0;
		*next_pack_cur++ = v;
		next_cur += interval_mn;
		int moving_up = mn_slice_pos++ == mn_slice_len_virtual;
		moving_up = -moving_up;
		next_cur += proceed_k & moving_up;
		/* ANDN instruction would be used on Haswell/Piledriver or newer*/
		mn_slice_pos &= ~moving_up;
	} while (--sched_size);

	/* save cur */
	prepack_st->next_cur      = next_cur;
	prepack_st->next_pack_cur = next_pack_cur;
	prepack_st->mn_slice_pos  = mn_slice_pos;

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
	for (int i = 0; i < kernel_st->m_slice_len; i++) {
		for (int j = 0; j < kernel_st->n_slice_len; j++) {
			c_cur[j] = c_buf_cur[j];
		}
		c_buf_cur += N_SLICE_LEN;
		c_cur += kernel_st->ldc;
	}

	/* save cur */
	kernel_st->a_pack_cur = a_pack_cur;
	kernel_st->b_pack_cur = b_pack_cur;
	kernel_st->c_cur += N_SLICE_LEN;
}

#define generic_kernel JOIN(NAMESPACE, PREFIX, generic_kernel_, SLICE_M_LEN, x, SLICE_N_LEN)
kernel_t generic_kernel = { &generic_kernel_fun, M_SLICE_LEN, N_SLICE_LEN };
