#ifdef NDEBUG
#undef NDEBUG
#endif

#define UNIT_LEN 8

#include <assert.h>
#include <stdio.h>
#include "kernel/generic_kernel.h"

static FTYPE a[8*64];
static FTYPE b[8*64];
static FTYPE c[8*8];
static FTYPE c_ans[8*8];
static FTYPE a_pack[8*64];
static FTYPE b_pack[8*64];

int main() {
	/* dummy data preparation */
	int v = 0;
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 64; j++) {
			a[i*64+j] = v++;
		}
	}
	for (int i = 0; i < 64; i++) {
		for (int j = 0; j < 8; j++) {
			b[i*8+j] = v++;
		}
	}
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			c[i*8+j] = -1;
			c_ans[i*8+j] = -1;
		}
	}
	/* pack data */
	sched_state_t a_sched = {
		.next_cur = a,
		.next_pack_cur = a_pack,
		.k_next_len = 64,
		.mn_next_len = 8,
		.interval_k = 1,
		.interval_mn = 64,
		.unit_len = 8
	};
	start_sched(&a_sched);
	pack_all(&a_sched);
	sched_state_t b_sched = {
		.next_cur = b,
		.next_pack_cur = b_pack,
		.k_next_len = 64,
		.mn_next_len = 8,
		.interval_k = 8,
		.interval_mn = 1,
		.unit_len = 8
	};
	start_sched(&b_sched);
	pack_all(&b_sched);
	/* run kernel */
	generic_kernel.fun(a_pack, b_pack, 64, 8, 8, c, 8, NULL);
	/* create ans */
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			for (int k = 0; k < 64; k++) {
				c_ans[8*i+j] += a[64*i+k] * b[8*k+j];
			}
		}
	}
	for (int i = 0; i < 8*8; i++) {
		/* printf("c[%d]=%f, c_ans[%d]=%f\n", i, c[i], i, c_ans[i]); */
		assert(c[i] == c_ans[i]);
	}
}
