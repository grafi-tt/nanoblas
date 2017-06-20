#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>

#include <assert.h>
#include <stdio.h>
#include "nanoblas_kernel.h"
#include "nanoblas_prepack.h"
#include "kernel/generic_kernel.h"
#include "internal/macro.h"

#include "internal/prepack/fixture.h"

#define generic_kernel_8 JOIN(NAMESPACE, PREFIX, generic_kernel_8)

static FTYPE a[8*4096];
static FTYPE b[8*4096];
static FTYPE c[8*8];
static kernel_state_t kst = {
	.a_pack_cur = a,
	.b_pack_cur = b,
	.c_cur = c,
	.m_sub_len = 8,
	.n_sub_ne = 8,
	.ldc = 8,
	.k_len = 4096,
}

int test_s1() {
	kernel_state_t kst_cpy = kst;
	init_s1_mtx();
	start_prepack(&s1);
	generic_kernel_8(&kst_cpy, &s1);
	for (int i = 0; i < 64*64; i++) {
		/* printf("mtx_pack[%d]=%f, mtx_pack_ans[%d]=%f\n", i, mtx_pack[i], i, mtx_pack_ans[i]); */
		assert(s1_mtx_pack[i] == s1_mtx_pack_ans[i]);
	}
}

int main() {
	test_s1();
	return 0;
}
