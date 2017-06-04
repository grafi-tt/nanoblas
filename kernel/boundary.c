#include "bondary_patch.h"

void boundary_kernel(
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
