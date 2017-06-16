#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include "sched.h"
#include "sched/fixture.h"

void test_s1() {
	sched_state_t sched = s1;
	start_sched(&sched);
	assert(sched.k_sched_len == 16);
	assert(sched.mn_sched_len == 8);
}

void test_s3() {
	sched_state_t sched = s3;
	start_sched(&sched);
	assert(sched.k_sched_len == 15);
	assert(sched.mn_sched_len == 3);
}

int main() {
	test_s1();
	test_s3();
	return 0;
}
