#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include "sched.h"

void normal_first() {
	FTYPE p[1];
	sched_state_t sched = {
		.next_cur = p,
		.next_pack_cur = p,
		.k_next_len = 64,
		.mn_next_len = 64,
		.interval_k = 100,
		.interval_mn = 1,
		.unit_len = 8
	};
	start_sched(&sched);
	step_sched(&sched);
	assert(sched.next_cur == p+1600);
	assert(sched.next_pack_cur == p+128);
	assert(sched.k_sched_len == 16);
	assert(sched.mn_sched_len == 8);
	assert(sched.k_packed_len == 16);
	assert(sched.mn_packed_len == 0);
}

void normal_chunk() {
	FTYPE p[1];
	sched_state_t sched = {
		.next_cur = p,
		.next_pack_cur = p,
		.k_next_len = 64,
		.mn_next_len = 64,
		.interval_k = 100,
		.interval_mn = 1,
		.unit_len = 8
	};
	start_sched(&sched);
	for (int i = 0; i < 4; i++) step_sched(&sched);
	assert(sched.next_cur == p+8);
	assert(sched.next_pack_cur == p+512);
	assert(sched.k_sched_len == 16);
	assert(sched.mn_sched_len == 8);
	assert(sched.k_packed_len == 0);
	assert(sched.mn_packed_len == 8);
}

int main() {
	normal_first();
	normal_chunk();
	return 0;
}
