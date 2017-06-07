#include "kernel.h"

__attribute__((optimize("unroll-loops")))
void generic_kernel_##FTYPE(
	int k_len, FTYPE *restrict a_pack, FTYPE *restrict b_pack,
	int k_len_sched, int m_len_sched, int trans_a, size_t lda,
	FTYPE *restrict a_pack_next, FTYPE *restrict a,
	size_t ldc, FTYPE* restrict c) {

	int k_cnt = 0;
	int m_cnt = 0;

	FTYPE[UNIT_LEN*UNIT_LEN] c_buf;
	FTYPE *restrict c_buf_cur = c_buf;
	FTYPE *restrict c_cur = c;

	size_t lda_if_transa = transa ? lda : 0;
	size_t lda_if_not_transa = transa ? 0 : lda;

	for (int i = 0; i < UNIT_LEN; i++) {
		for (int j = 0; j < UNIT_LEN; j++) {
			c_buf_cur[j] = c_pack[j];
		}
		c_buf_cur += UNIT_LEN;
		c_pack += ldc;
	}
	c_buf_cur = c_buf;

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
		if (k_cnt != k_len_sched) {
			pack_do:
			a_pack[m_len_sched*k_cnt + m_len_sched] =
				a[k_cnt*lda_if_transa + m_cnt*lda_if_not_transa];
			int moving_up = m_cnt++ == m_len_sched;
			k_cnt += moving_up;
			m_cnt &= moving_up--;
		}
	} while (k_len-- != 0);
	/* restart packing until completed */
	k_len++;
	if (k_cnt != k_len_sched) goto pack_do;
}
