#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include "nanoblas_prepack.h"
#include "internal/prepack.h"
#include "internal/prepack/fixture.h"

void s1_first() {
	prepack_state_t st = s1;
	start_prepack(&st);
	step_prepack(&st);
	assert(st.next_cur == s1_mtx+1600);
	assert(st.next_pack_cur == s1_mtx_pack+128);
	assert(st.k_sched_len == 16);
	assert(st.mn_sched_len == 8);
	assert(st.k_packed_len == 16);
	assert(st.mn_packed_len == 0);
}

void s1_chunk() {
	prepack_state_t st = s1;
	start_prepack(&st);
	for (int i = 0; i < 4; i++) step_prepack(&st);
	assert(st.next_cur == s1_mtx+8);
	assert(st.next_pack_cur == s1_mtx_pack+512);
	assert(st.k_sched_len == 16);
	assert(st.mn_sched_len == 8);
	assert(st.k_packed_len == 0);
	assert(st.mn_packed_len == 8);
}

void s1_end() {
	prepack_state_t st = s1;
	start_prepack(&st);
	for (int i = 0; i < 32; i++) step_prepack(&st);
	assert(st.next_cur == s1_mtx+64);
	assert(st.next_pack_cur == s1_mtx_pack+4096);
	assert(st.mn_sched_len == 0);
	assert(st.mn_packed_len == 64);
}

void s2_chunk() {
	prepack_state_t st = s2;
	start_prepack(&st);
	for (int i = 0; i < 2; i++) step_prepack(&st);
	assert(st.next_cur == s2_mtx+32);
	assert(st.next_pack_cur == s2_mtx_pack+256);
	assert(st.k_sched_len == 10);
	assert(st.mn_sched_len == 8);
	assert(st.k_packed_len == 32);
	assert(st.mn_packed_len == 0);
	step_prepack(&st);
	assert(st.next_cur == s2_mtx+800);
	assert(st.next_pack_cur == s2_mtx_pack+336);
	assert(st.k_sched_len == 16);
	assert(st.mn_sched_len == 8);
	assert(st.k_packed_len == 0);
	assert(st.mn_packed_len == 8);
}

void  s2_chunkend() {
	prepack_state_t st = s2;
	start_prepack(&st);
	for (int i = 0; i < 21; i++) step_prepack(&st);
	assert(st.next_cur == s2_mtx+5600);
	assert(st.next_pack_cur == s2_mtx_pack+2352);
	assert(st.k_sched_len == 16);
	assert(st.mn_sched_len == 7);
	assert(st.k_packed_len == 0);
	assert(st.mn_packed_len == 56);
}

void s2_end() {
	prepack_state_t st = s2;
	start_prepack(&st);
	for (int i = 0; i < 24; i++) step_prepack(&st);
	assert(st.next_cur == s2_mtx+6300);
	assert(st.next_pack_cur == s2_mtx_pack+2646);
	assert(st.mn_sched_len == 0);
	assert(st.mn_packed_len == 63);
}

int main() {
	s1_first();
	s1_chunk();
	s1_end();
	s2_chunk();
	s2_chunkend();
	return 0;
}
