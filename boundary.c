#include "bondary.h"
#include "const.h"
#include "util.h"

void pack_block_##FTYPE(int k_len, int m_or_n_len, size_t interval_k, size_t interval_m_or_n,
		FTYPE *restrict pack, FTYPE *restrict a_or_b) {
	for (int m_or_n_pos = 0; m_or_n_pos < m_or_n_len; m_or_n_pos += UNIT_LEN) {
		m_or_n_sub_len = imin(m_or_n_len - m_or_n_pos, UNIT_LEN);
		for (int k_cur = 0; k_cur < k_len; k_cur++) {
			for (int m_or_n_cur = m_or_n_pos; m_or_n_cur < m_or_n_pos + m_or_n_sub_len; m_or_n_cur++) {
				*++pack = a_or_b[interval_k*k_cur + interval_m_or_n*m_or_n_cur];
			}
		}
		a_or_b += k_len * m_or_n_sub_len;
	}
}

void boundary_kernel_##FTYPE(
		int m_sub_len, int n_sub_len, FTYPE *restrict a_pack, FTYPE *restrict b_pack,
		int k_sched_len, int m_sched_len, size_t interval_k_in_a, size_t interval_m,
		FTYPE *restrict a_next_pack, FTYPE *restrict a_next,
		size_t k_len_szt, FTYPE *restrict c, size_t ldc) {

	int k_len = k_len_szt;

	FTYPE[UNIT_LEN*UNIT_LEN] c_buf;
	FTYPE *restrict c_buf_cur = c_buf;
	FTYPE *restrict c_cur = c;

	for (int i = 0; i < m_sub_len; i++) {
		for (int j = 0; j < n_sub_len; j++) {
			c_buf_cur[j] = c_cur[j];
		}
		c_buf_cur += n_sub_len;
		c_cur += ldc;
	}
	c_buf_cur = c_buf;
	c_cur = c;

	for (int k = 0; k < len; k++) {
		for (int i = 0; i < m; i++) {
			FTYPE v = a_pack_cur[i];
			for (int j = 0; j < n; j++) {
				c_buf_cur[j] = v * b_pack_cur[j];
			}
			c_buf_cur += n;
		}
		a_pack_cur += m;
		b_pack_cur += n;
		c_buf_cur = c_buf;
	}

	for (int i = 0; i < m_sub_len; i++) {
		for (int j = 0; j < n_sub_len; j++) {
			c_cur[j] = c_buf_cur[j];
		}
		c_buf_cur += n_sub_len;
		c_cur += ldc;
	}

	pack_block_##FTYPE(k_sched_len, m_sched_len, interval_k_in_a, interval_m, a_next_pack, a_next);
}
