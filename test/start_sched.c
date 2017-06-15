#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include "sched.h"

void normal() {
	sched_state_t sched = {
		.k_next_len = 128,
		.mn_next_len = 128,
		.unit_len = 8
	};
	start_sched(&sched);
	assert(sched.k_sched_len == 16);
	assert(sched.mn_sched_len == 8);
}

void corner() {
	sched_state_t sched = {
		.k_next_len = 15,
		.mn_next_len = 3,
		.unit_len = 8
	};
	start_sched(&sched);
	assert(sched.k_sched_len == 15);
	assert(sched.mn_sched_len == 3);
}

int main() {
	normal();
	corner();
	return 0;
}
