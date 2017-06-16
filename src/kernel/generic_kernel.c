#include "generic_kernel.h"

__attribute__((optimize("unroll-loops")))
void generic_kernel_fun(
		const FTYPE *restrict a_pack, const FTYPE *restrict b_pack,
		int k_len, int m_sub_len, int n_sub_len,
		FTYPE* restrict c, ptrdiff_t ldc, const sched_state_t *st) {

	FTYPE c_buf[UNIT_LEN*UNIT_LEN];
	FTYPE *restrict c_buf_cur = c_buf;
	FTYPE *restrict c_cur = c;

	/* load c */
	for (int i = 0; i < m_sub_len; i++) {
		for (int j = 0; j < n_sub_len; j++) {
			c_buf_cur[j] = c_cur[j];
		}
		c_buf_cur += UNIT_LEN;
		c_cur += ldc;
	}
	c_buf_cur = c_buf;
	c_cur = c;

	if (st == NULL) goto nopack_loop;

	FTYPE *restrict a_next_cur = st->next_cur;
	FTYPE *restrict a_next_pack_cur = st->next_pack_cur;
	int k_sched_len = st->k_sched_len;
	int mn_sched_len = st->mn_sched_len;
	ptrdiff_t interval_m = st->interval_mn;
	ptrdiff_t proceed_k = -UNIT_LEN*interval_m + st->interval_k;

	int mn_cnt = 0;
	k_len -= UNIT_LEN*k_sched_len;

	do {
		for (int i = 0; i < UNIT_LEN; i++) {
			FTYPE v = a_pack[i];
			for (int j = 0; j < UNIT_LEN; j++) {
				c_buf_cur[j] = v * b_pack[j];
			}
			c_buf_cur += UNIT_LEN;
		}
		c_buf_cur = c_buf;
		a_pack += UNIT_LEN;
		b_pack += UNIT_LEN;
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
			FTYPE v = a_pack[i];
			for (int j = 0; j < UNIT_LEN; j++) {
				c_buf_cur[j] += v * b_pack[j];
			}
			c_buf_cur += UNIT_LEN;
		}
		c_buf_cur = c_buf;
		a_pack += UNIT_LEN;
		b_pack += UNIT_LEN;
	} while (--k_len);
	loop_end:

	/* store c */
	for (int i = 0; i < m_sub_len; i++) {
		for (int j = 0; j < n_sub_len; j++) {
			c_cur[j] = c_buf_cur[j];
		}
		c_buf_cur += UNIT_LEN;
		c_cur += ldc;
	}
}

kernel_t generic_kernel = { &generic_kernel_fun, UNIT_LEN };
