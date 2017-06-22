#include "nanoblas_prepack.h"

/* s1 */
FTYPE s1_mtx[64*100];
FTYPE s1_mtx_pack[64*64];
FTYPE s1_mtx_pack_ans[64*64];

prepack_state_t init_s1() {
	int v = 0;
	for (int i = 0; i < 64; i++) {
		for (int j = 0; j < 64; j++) {
			s1_mtx[100*i+j] = v++;
		}
	}

	int l = 0;
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 64; j++) {
			for (int k = 0; k < 8; k++) {
				s1_mtx_pack_ans[l++] = 8*i + 64*j + k;
			}
		}
	}

	return prepack_state_new(s1_mtx, s1_mtx_pack, 64, 64, 8, 64, 1, 100);
}

/* s2 */
FTYPE s2_mtx[64*100];
FTYPE s2_mtx_pack[64*42];
FTYPE s2_mtx_pack_ans[64*42];

prepack_state_t init_s2() {
	int v = 0;
	for (int i = 0; i < 63; i++) {
		for (int j = 0; j < 42; j++) {
			s2_mtx[100*i+j] = v++;
		}
	}
	for (int j = 0; j < 42; j++) {
		s2_mtx[6300+j] = 7777;
	}

	int l = 0;
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 42; j++) {
			for (int k = 0; k < 8; k++) {
				s2_mtx_pack_ans[l++] = (i == 7 && k == 7) ? 0 : 336*i + j + 42*k;
			}
		}
	}

	return prepack_state_new(s2_mtx, s2_mtx_pack, 63, 42, 8, 128, 100, 1);
}

/*
prepack_state_t s2 = {
	.next_cur = s2_mtx,
	.next_pack_cur = s2_mtx_pack,
	.mn_next_len = 63,
	.k_next_len = 42,
	.interval_mn = 100,
	.interval_k = 1,
	.unit_len = 8,
};
*/

/* s3 */
/*
FTYPE s3_mtx[15*100];
void init_s3_mtx() {
	int v = 0;
	for (int i = 0; i < 15; i++) {
		for (int j = 0; j < 3; j++) {
			s3_mtx[100*i+j] = v++;
		}
	}
}

FTYPE s3_mtx_pack[8*15];
FTYPE s3_mtx_pack_ans[8*15];
void init_s3_mtx_pack_ans() {
	int l = 0;
	for (int i = 0; i < 15; i++) {
		for (int j = 0; j < 8; j++) {
			s3_mtx_pack_ans[l++] = j >= 3 ? 0 : 3*i + j;
		}
	}
}

prepack_state_t s3 = {
	.next_cur = s3_mtx,
	.next_pack_cur = s3_mtx_pack,
	.mn_next_len = 3,
	.k_next_len = 15,
	.interval_mn = 1,
	.interval_k = 100,
	.unit_len = 8,
};
*/