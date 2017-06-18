#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include "nanoblas_prepack.h"
#include "internal/prepack.h"
#include "internal/prepack/fixture.h"

void test_s1() {
	start_prepack(&s1);
	assert(s1.k_sched_len == 16);
	assert(s1.mn_sched_len == 8);
}

void test_s3() {
	start_prepack(&s3);
	assert(s3.k_sched_len == 15);
	assert(s3.mn_sched_len == 3);
}

int main() {
	test_s1();
	test_s3();
	return 0;
}
