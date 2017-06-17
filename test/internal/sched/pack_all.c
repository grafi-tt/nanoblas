#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include "internal/sched.h"
#include "internal/sched/fixture.h"

static FTYPE mtx[64*100];
static FTYPE mtx_pack[64*64];
static FTYPE mtx_pack_ans[64*64];

void test_s1() {
	int v = 0;
	for (int i = 0; i < 64; i++) {
		for (int j = 0; j < 64; j++) {
			mtx[100*i+j] = v++;
		}
	}
	int l = 0;
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 64; j++) {
			for (int k = 0; k < 8; k++) {
				mtx_pack_ans[l++] = 8*i + 64*j + k;
			}
		}
	}
	sched_state_t sched = s1;
	sched.next_cur = mtx;
	sched.next_pack_cur = mtx_pack;
	start_sched(&sched);
	pack_all(&sched);
	for (int i = 0; i < 64*64; i++) {
		/* printf("mtx_pack[%d]=%f, mtx_pack_ans[%d]=%f\n", i, mtx_pack[i], i, mtx_pack_ans[i]); */
		assert(mtx_pack[i] == mtx_pack_ans[i]);
	}
}

void test_s2() {
	int v = 0;
	for (int i = 0; i < 63; i++) {
		for (int j = 0; j < 42; j++) {
			mtx[100*i+j] = v++;
		}
	}
	int l = 0;
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 42; j++) {
			for (int k = 0; k < 8; k++) {
				mtx_pack_ans[l++] = (i == 7 && k == 7) ? 0 : 336*i + j + 42*k;
			}
		}
	}
	sched_state_t sched = s2;
	sched.next_cur = mtx;
	sched.next_pack_cur = mtx_pack;
	start_sched(&sched);
	pack_all(&sched);
	for (int i = 0; i < 42*64; i++) {
		/* printf("mtx_pack[%d]=%f, mtx_pack_ans[%d]=%f\n", i, mtx_pack[i], i, mtx_pack_ans[i]); */
		assert(mtx_pack[i] == mtx_pack_ans[i]);
	}
}

int main() {
	test_s1();
	test_s2();
	return 0;
}
