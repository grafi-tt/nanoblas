#include "kernel.h"

__attribute__((optimize("unroll-loops")))
void generic_kernel_##FTYPE(
	FTYPE *restrict a_pack, FTYPE *restrict b_pack,
	int k_sched_len, int m_sched_len, size_t interval_k_in_a, size_t interval_m,
	FTYPE *restrict a_next_pack, FTYPE *restrict a_next,
	size_t k_len_szt, size_t ldc, FTYPE* restrict c) {

	int k_len = k_len_szt;

	int k_cnt = 0;
	int m_cnt = 0;

	FTYPE[UNIT_LEN*UNIT_LEN] c_buf;
	FTYPE *restrict c_buf_cur = c_buf;
	FTYPE *restrict c_cur = c;

	for (int i = 0; i < UNIT_LEN; i++) {
		for (int j = 0; j < UNIT_LEN; j++) {
			c_buf_cur[j] = c_cur[j];
		}
		c_buf_cur += UNIT_LEN;
		c_cur += ldc;
	}
	c_buf_cur = c_buf;
	c_cur = c;

	do {
		/* unroll 0 */
		for (int i = 0; i < UNIT_LEN; i++) {
			FTYPE v = a_pack[i];
			for (int j = 0; j < UNIT_LEN; j++) {
				c_buf_cur[j] = v * b_pack[j];
			}
			c_buf_cur += UNIT_LEN;
		}
		c_buf_cur = c_buf;
		if (k_len-- == 0) break;

		/* unroll 1 */
		for (int i = 0; i < UNIT_LEN; i++) {
			FTYPE v = a_pack[UNIT_LEN+i];
			for (int j = 0; j < UNIT_LEN; j++) {
				c_buf_cur[j] = v * b_pack[UNIT_LEN+j];
			}
			c_buf_cur += UNIT_LEN;
		}
		c_buf_cur = c_buf;
		a_pack += 2*UNIT_LEN;
		b_pack += 2*UNIT_LEN;

		/* pack */
		pack:
		if (k_cnt != k_sched_len) {
			pack_do:
			a_pack[m_sched_len*k_cnt + m_cnt] =
				a[interval_k_in_a*k_cnt + interval_m*m_cnt];
			int moving_up = m_cnt++ == m_sched_len;
			k_cnt += moving_up;
			m_cnt &= moving_up--;
		}
	} while (k_len-- != 0);
	/* restart packing until completed */
	k_len++;
	if (k_cnt != k_sched_len) goto pack_do;

	for (int i = 0; i < UNIT_LEN; i++) {
		for (int j = 0; j < UNIT_LEN; j++) {
			c_cur[j] = c_buf_cur[j];
		}
		c_buf_cur += UNIT_LEN;
		c_cur += ldc;
	}
}
