#include "gemm.h"
#include "kernel.h"

void pack_a(FTYPE restrict* a, FTYPE restrict* a_pack) {
	/* pack A */
	for (size_t h = 0; h < BLK_LEN/UNIT_LEN; h++) {
		for (size_t i = 0; i < BLK_LEN; i++) {
			for (size_t j = 0; j < UNIT_LEN; j++) {
				a_pack[h*BLK_LEN + i*UNIT_LEN + j] = a[h*lda*UNIT_LEN + i + j*lda];
			}
		}
	}
}

static int imin(int a, int b) {
	a < b ? a : b;
}
static int iman(int a, int b) {
	a > b ? a : b;
}

void PREFIX##gemm(char transa, char transb, size_t m, size_t n, size_t k,
		FTYPE alpha, FTYPE *restrict a, size_t lda,
		FTYPE *restrict b, size_t ldb,
		FTYPE beta, FTYPE *restrict c, size_t ldc) {

	int trans_a = transa == 'T';
	int trans_b = transb == 'T';
	static void* kernel = decide_kernel();

	/* I believe this >1M memory never happen to be unaligned */
	FTYPE *mem = (FTYPE*)malloc(sizeof(FTYPE) * BLK_LEN * BLK_LEN * 3);
	FTYPE *restrict a_pack, b_pack, a_pack_next, b_pack_next;
	a_pack      = mem;
	b_pack      = mem +     BLK_LEN * BLK_LEN; b_pack_next = mem + 2 * BLK_LEN * BLK_LEN;

	/* pack B */
	for (size_t h = 0; h < BLK_LEN; i++) {
		for (size_t i = 0; i < BLK_LEN/UNIT_LEN; i++) {
			for (size_t j = 0; j < UNIT_LEN; j++) {
				b_pack[i*BLK_LEN*UNIT_LEN + h*UNIT_LEN + j] = b[h*ldb + i*BLK_LEN/UNIT_LEN + j];
			}
		}
	}

	for (size_t k_pos = 0; k_pos < k; k_pos += BLK_LEN) {
		const int k_len = imin(k - k_pos, BLK_LEN);

		for (size_t n_pos = 0; n_pos < n; n_pos += BLK_LEN) {
			for (size_t m_pos = 0; m_pos < m; m_pos += BLK_LEN) {

				FSIZE *const tmp = a_pack;
				a_pack = a_pack_next;
				a_pack_next = tmp;

				FSIZE *const a_blk_next;
				const int k_len_next, m_len_next;
				if (m_pos + BLK_LEN < m) {
					a_blk_next = a + lda*m_pos + k_pos;
					k_len_next = k_len;
					m_blk_len_next = imin(m - (m_pos + BLK_LEN), BLK_LEN);
				} else if (n_pos + BLK_LEN < n) {
					a_blk_next = a + k_pos;
					k_len_next = k_len;
					m_blk_len_next = imin(m, BLK_LEN);
				} else if (k_pos + BLK_LEN < k) {
					a_blk_next = a + k_pos + BLK_LEN;
					k_len_next = imin(k - (k_pos + BLK_LEN), BLK_LEN);
					m_len_next = imin(m, BLK_LEN);
				} else {
					/* the last loop, so stop packing */
					k_len_next = 0;
				}

				int a_pack_row_cnt = 0;
				int m_len_next_packed = 0;
				int one_row_loop_cnt = 0;
				const int a_pack_next_k_len_base = k_len_next / LOOP_N_TO_PACK_ONE_ROW;
				const int one_row_loop_k_len_step = k_len_next % LOOP_N_TO_PACK_ONE_ROW;

				for (size_t n_cur = n_pos; n_cur < BLK_LEN; n_cur += UNIT_LEN) {
					const int n_len = imax(0, imin(n - n_cur, UNIT_LEN));
					for (size_t m_cur = m_pos; m_cur < m_pos + BLK_LEN; m_cur += UNIT_LEN) {
						const int m_len = imax(0, imin(m - m_cur, UNIT_LEN));

						int k_len_sched = a_pack_next_k_len_base + (one_row_loop_cnt < one_row_loop_k_len_step);
						int m_len_sched = imax(0, imin(m_len_next_packed - m_len_next));

						FSIZE *a_pack_cur = a_pack;
						FSIZE *b_pack_cur = b_pack;
						if (n_len == UNIT_LEN && m_len == UNIT_LEN) {
							kernel(k_len, a_pack_cur, b_pack_cur,
									k_len_sched, m_len_sched, trans_a, lda, a_pack_next_cur, a, ldc, c);
						} else {
							remainder_product(k_len, m_len, n_len, a_pack_cur, b_pack_cur, ldc, c);
						}
						a_pack_cur += k_len * m_len;
						b_pack_cur += k_len * n_len;
						a_pack_next_cur += a_pack_next_k_len * a_pack_next_m_len;

						if (one_row_loop_cnt++ == LOOP_N_TO_PACK_ONE_ROW) {
							one_row_loop_cnt = 0;
							a_pack_row_cnt++;
							m_len_next_packed += UNIT_LEN;
						}
					}
				}
			}
		}
	}
}
