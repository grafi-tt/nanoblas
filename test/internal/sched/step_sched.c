#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include "internal/sched.h"
#include "internal/sched/fixture.h"

void s1_first() {
	sched_state_t sched = s1;
	start_sched(&sched);
	step_sched(&sched);
	assert(sched.next_cur == p+1600);
	assert(sched.next_pack_cur == p+128);
	assert(sched.k_sched_len == 16);
	assert(sched.mn_sched_len == 8);
	assert(sched.k_packed_len == 16);
	assert(sched.mn_packed_len == 0);
}

void s1_chunk() {
	sched_state_t sched = s1;
	start_sched(&sched);
	for (int i = 0; i < 4; i++) step_sched(&sched);
	assert(sched.next_cur == p+8);
	assert(sched.next_pack_cur == p+512);
	assert(sched.k_sched_len == 16);
	assert(sched.mn_sched_len == 8);
	assert(sched.k_packed_len == 0);
	assert(sched.mn_packed_len == 8);
}

void s1_end() {
	sched_state_t sched = s1;
	start_sched(&sched);
	for (int i = 0; i < 32; i++) step_sched(&sched);
	assert(sched.next_cur == p+64);
	assert(sched.next_pack_cur == p+4096);
	assert(sched.mn_sched_len == 0);
	assert(sched.mn_packed_len == 64);
}

void s2_chunk() {
	sched_state_t sched = s2;
	start_sched(&sched);
	for (int i = 0; i < 2; i++) step_sched(&sched);
	assert(sched.next_cur == p+32);
	assert(sched.next_pack_cur == p+256);
	assert(sched.k_sched_len == 10);
	assert(sched.mn_sched_len == 8);
	assert(sched.k_packed_len == 32);
	assert(sched.mn_packed_len == 0);
	step_sched(&sched);
	assert(sched.next_cur == p+800);
	assert(sched.next_pack_cur == p+336);
	assert(sched.k_sched_len == 16);
	assert(sched.mn_sched_len == 8);
	assert(sched.k_packed_len == 0);
	assert(sched.mn_packed_len == 8);
}

void  s2_chunkend() {
	sched_state_t sched = s2;
	start_sched(&sched);
	for (int i = 0; i < 21; i++) step_sched(&sched);
	assert(sched.next_cur == p+5600);
	assert(sched.next_pack_cur == p+2352);
	assert(sched.k_sched_len == 16);
	assert(sched.mn_sched_len == 7);
	assert(sched.k_packed_len == 0);
	assert(sched.mn_packed_len == 56);
}

void s2_end() {
	sched_state_t sched = s2;
	start_sched(&sched);
	for (int i = 0; i < 24; i++) step_sched(&sched);
	assert(sched.next_cur == p+6300);
	assert(sched.next_pack_cur == p+2646);
	assert(sched.mn_sched_len == 0);
	assert(sched.mn_packed_len == 63);
}

int main() {
	s1_first();
	s1_chunk();
	s1_end();
	s2_chunk();
	s2_chunkend();
	return 0;
}
