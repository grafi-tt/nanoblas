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

void FTYPE_PREFIX ## gemm(char transa, char transb, size_t m, size_t n, size_t k,
		FTYPE alpha, FTYPE *restrict a, size_t lda,
		FTYPE *restrict b, size_t ldb,
		FTYPE beta, FTYPE *restrict c, size_t ldc) {

	static void* kernel = decide_kernel();

	/* I believe this >1M memory never happen to be unaligned */
	FTYPE *mem = (FTYPE*)malloc(sizeof(FTYPE) * BLK_LEN * BLK_LEN * 3);
	FTYPE *restrict a_pack, b_pack, a_pack_next, b_pack_next;
	a_pack      = mem;
	b_pack      = mem +     BLK_LEN * BLK_LEN;
	b_pack_next = mem + 2 * BLK_LEN * BLK_LEN;


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

				FSIZE *tmp = a_pack;
				a_pack = a_pack_next;
				a_pack_next = tmp;

				int a_pack_next_cnt;
				FSIZE *a_blk_next;
				if (m_pos + BLK_LEN < m) {
					a_pack_next_cnt = imin(m - (m_pos + BLK_LEN), BLK_LEN) * k_len;
					a_blk_next = a + lda*m_pos + k_pos;
				} else if (n_pos + BLK_LEN < n) {
					a_pack_next_cnt = imin(m, BLK_LEN) * k_len;
					a_blk_next = a + k_pos;
				} else if (k_pos + BLK_LEN < k) {
					a_pack_next_cnt = imin(m, BLK_LEN) * imin(k - (k_pos + BLK_LEN), BLK_LEN);
					a_blk_next = a + k_pos + BLK_LEN;
				} else {
					a_pack_next_cnt = 0;
					/* packing won't perform */
				}

				size_t m_cur, n_cur;
				float *a_pack_cur = a_pack;
				float *b_pack_cur = b_pack;
				for (n_cur = n_pos; n_cur < imin(n, n_pos + BLK_LEN); n_cur += UNIT_LEN) {
					const int n_len = imin(n - n_cur, UNIT_LEN);
					for (m_cur = m_pos; m_cur < imin(m, m_pos + BLK_LEN); m_cur += UNIT_LEN) {
						const int m_len = imin(m - m_cur, UNIT_LEN);
						if (n_len == UNIT_LEN && m_len == UNIT_LEN) {
							a_pack_next_cnt =
								kernel(k_len, a_pack_next_cnt, a_pack_next, lda, a,
										a_pack_cur, b_pack_cur, ldc, c);
						} else {
							remainder_product(k_len, m_len, n_len, a_pack_cur, b_pack_cur, ldc, c);
						}
						a_pack_cur += k_len * m_len;
						b_pack_cur += k_len * n_len;
					}
				}
			}
		}
	}
}
