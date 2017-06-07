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
		int len, int m, int n
		FTYPE *restrict a_pack_cur, FTYPE *restrict b_pack_cur,
		size_t ldc, FTYPE *restrict c) {

	FTYPE[UNIT_LEN*UNIT_LEN] c_buf;
	FTYPE *restrict c_buf_cur = c_buf;
	FTYPE *restrict c_cur = c;

	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			c_buf_cur[j] = c_cur[j];
		}
		c_buf_cur += n;
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

	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			c_cur[j] = c_buf_cur[j];
		}
		c_buf_cur += n;
		c_cur += ldc;
	}
}
