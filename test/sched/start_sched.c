#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include "sched.h"
#include "sched/fixture.h"

void test_s1() {
	start_sched(&s1);
	assert(s1.k_sched_len == 16);
	assert(s1.mn_sched_len == 8);
}

void test_s3() {
	start_sched(&s3);
	assert(s3.k_sched_len == 15);
	assert(s3.mn_sched_len == 3);
}

int main() {
	test_s1();
	test_s3();
	return 0;
}
