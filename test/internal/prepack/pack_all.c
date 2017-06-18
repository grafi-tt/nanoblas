#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include "nanoblas_prepack.h"
#include "internal/prepack.h"
#include "internal/prepack/fixture.h"

void test_s1() {
	init_s1_mtx();
	init_s1_mtx_pack_ans();
	start_prepack(&s1);
	pack_all(&s1);
	for (int i = 0; i < 64*64; i++) {
		/* printf("mtx_pack[%d]=%f, mtx_pack_ans[%d]=%f\n", i, mtx_pack[i], i, mtx_pack_ans[i]); */
		assert(s1_mtx_pack[i] == s1_mtx_pack_ans[i]);
	}
}

void test_s2() {
	init_s2_mtx();
	init_s2_mtx_pack_ans();
	start_prepack(&s2);
	pack_all(&s2);
	for (int i = 0; i < 64*42; i++) {
		/* printf("mtx_pack[%d]=%f, mtx_pack_ans[%d]=%f\n", i, mtx_pack[i], i, mtx_pack_ans[i]); */
		assert(s2_mtx_pack[i] == s2_mtx_pack_ans[i]);
	}
}

void test_s3() {
	init_s3_mtx();
	init_s3_mtx_pack_ans();
	start_prepack(&s3);
	pack_all(&s3);
	for (int i = 0; i < 8*15; i++) {
		/* printf("mtx_pack[%d]=%f, mtx_pack_ans[%d]=%f\n", i, mtx_pack[i], i, mtx_pack_ans[i]); */
		assert(s3_mtx_pack[i] == s3_mtx_pack_ans[i]);
	}
}

int main() {
	test_s1();
	test_s2();
	test_s3();
	return 0;
}
