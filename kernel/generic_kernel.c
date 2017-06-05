#include "kernel.h"

__attribute__((optimize("unroll-loops")))
void generic_kernel_##FTYPE(
	int k_len, int next_m_cnt, int next_m_len, int next_k_cnt, int next_k_len,
	int transa, FTYPE *restrict a_pack_next, size_t lda,
	FTYPE *restrict a, FTYPE *restrict a_pack, FTYPE *restrict b_pack,
	size_t ldc, FTYPE* restrict c_pack) {

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
		if (next_k_cnt != next_k_len) {
			pack_do:
			a_pack[next_m_len*next_k_cnt + next_m_len] =
				a[next_k_cnt*lda_if_transa + next_m_cnt*lda_if_not_transa];
			int moving_up = next_m_cnt++ == next_m_len;
			next_k_cnt += moving_up;
			next_m_cnt &= moving_up--;
		}
	} while (k_len-- != 0);
	/* restart packing until completed */
	k_len++;
	if (next_k_cnt != next_k_len) goto pack_do;
}
