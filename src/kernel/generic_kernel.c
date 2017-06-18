#include "nanoblas_kernel.h"
#include "nanoblas_prepack.h"
#include "kernel/generic_kernel.h"
#include "internal/macro.h"

#define generic_kernel_fun JOIN(NAMESPACE, PREFIX, generic_kernel_fun_, UNIT_LEN)
__attribute__((optimize("unroll-loops")))
void generic_kernel_fun(kernel_state_t *kernel_st, prepack_state_t *prepack_st) {

	FTYPE c_buf[UNIT_LEN*UNIT_LEN];
	FTYPE *restrict c_buf_cur = c_buf;
	FTYPE *restrict c_cur = kernel_st->c_cur;

	/* load c */
	for (int i = 0; i < kernel_st->m_sub_len; i++) {
		for (int j = 0; j < kernel_st->n_sub_len; j++) {
			c_buf_cur[j] = c_cur[j];
		}
		c_buf_cur += UNIT_LEN;
		c_cur += kernel_st->ldc;
	}
	c_buf_cur = c_buf;
	c_cur = kernel_st->c_cur;

	const FTYPE *restrict a_pack_cur = kernel_st->a_pack_cur;
	const FTYPE *restrict b_pack_cur = kernel_st->b_pack_cur;
	int k_len = kernel_st->k_len;

	if (prepack_st == NULL) goto nopack_loop;

	const FTYPE *restrict a_next_cur = prepack_st->next_cur;
	FTYPE *restrict a_next_pack_cur = prepack_st->next_pack_cur;
	int k_sched_len = prepack_st->k_sched_len;
	int mn_sched_len = prepack_st->mn_sched_len;
	ptrdiff_t interval_m = prepack_st->interval_mn;
	ptrdiff_t proceed_k = -UNIT_LEN*interval_m + prepack_st->interval_k;

	int mn_cnt = 0;
	k_len -= UNIT_LEN*k_sched_len;

	do {
		for (int i = 0; i < UNIT_LEN; i++) {
			FTYPE v = a_pack_cur[i];
			for (int j = 0; j < UNIT_LEN; j++) {
				c_buf_cur[j] = v * b_pack_cur[j];
			}
			c_buf_cur += UNIT_LEN;
		}
		c_buf_cur = c_buf;
		a_pack_cur += UNIT_LEN;
		b_pack_cur += UNIT_LEN;
		/* pack */
		FTYPE v = (mn_cnt < mn_sched_len) ? *a_next_cur : 0;
		*a_next_pack_cur++ = v;
		a_next_cur += interval_m;
		int moving_up = mn_cnt++ == UNIT_LEN;
		moving_up--;
		mn_cnt &= moving_up;
		moving_up = ~moving_up;
		a_next_cur += proceed_k & moving_up;
		k_sched_len += moving_up;
	} while (k_sched_len);
	if (k_len == 0) goto loop_end;

	nopack_loop:
	do {
		for (int i = 0; i < UNIT_LEN; i++) {
			FTYPE v = a_pack_cur[i];
			for (int j = 0; j < UNIT_LEN; j++) {
				c_buf_cur[j] += v * b_pack_cur[j];
			}
			c_buf_cur += UNIT_LEN;
		}
		c_buf_cur = c_buf;
		a_pack_cur += UNIT_LEN;
		b_pack_cur += UNIT_LEN;
	} while (--k_len);
	loop_end:

	/* save cur */
	kernel_st->a_pack_cur = a_pack_cur;
	kernel_st->b_pack_cur = b_pack_cur;

	/* store c */
	for (int i = 0; i < kernel_st->m_sub_len; i++) {
		for (int j = 0; j < kernel_st->n_sub_len; j++) {
			c_cur[j] = c_buf_cur[j];
		}
		c_buf_cur += UNIT_LEN;
		c_cur += kernel_st->ldc;
	}
}

#define generic_kernel JOIN(NAMESPACE, PREFIX, generic_kernel_, UNIT_LEN)
kernel_t generic_kernel = { &generic_kernel_fun, UNIT_LEN };
