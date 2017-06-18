#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include <stdio.h>
#include "nanoblas_kernel.h"
#include "nanoblas_prepack.h"
#include "kernel/generic_kernel.h"
#include "internal/macro.h"
#include "internal/prepack.h"

#define generic_kernel_8 JOIN(NAMESPACE, PREFIX, generic_kernel_8)

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
	prepack_state_t a_prepack_st = {
		.next_cur = a,
		.next_pack_cur = a_pack,
		.mn_next_len = 8,
		.k_next_len = 64,
		.interval_mn = 64,
		.interval_k = 1,
		.unit_len = 8
	};
	start_prepack(&a_prepack_st);
	pack_all(&a_prepack_st);
	prepack_state_t b_prepack_st = {
		.next_cur = b,
		.next_pack_cur = b_pack,
		.mn_next_len = 8,
		.k_next_len = 64,
		.interval_mn = 1,
		.interval_k = 8,
		.unit_len = 8
	};
	start_prepack(&b_prepack_st);
	pack_all(&b_prepack_st);
	/* run kernel */
	kernel_state_t kernel_st = {
		.a_pack_cur = a_pack,
		.b_pack_cur = b_pack,
		.c_cur = c,
		.m_sub_len = 8,
		.n_sub_len = 8,
		.ldc = 8,
		.k_len = 64,
	};
	generic_kernel_8.fun(&kernel_st, NULL);
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
