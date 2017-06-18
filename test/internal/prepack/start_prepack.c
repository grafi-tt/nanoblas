#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include "nanoblas_prepack.h"
#include "internal/prepack.h"
#include "internal/prepack/fixture.h"

void test_s1() {
	prepack_state_t sched = s1;
	start_prepack(&sched);
	assert(sched.k_sched_len == 16);
	assert(sched.mn_sched_len == 8);
}

void test_s3() {
	prepack_state_t sched = s3;
	start_prepack(&sched);
	assert(sched.k_sched_len == 15);
	assert(sched.mn_sched_len == 3);
}

int main() {
	test_s1();
	test_s3();
	return 0;
}
