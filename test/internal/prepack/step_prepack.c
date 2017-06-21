#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include "nanoblas_prepack.h"
#include "internal/prepack.h"
#include "internal/prepack/fixture.h"

void s1_first() {
	prepack_state_t s1 = init_s1();
	s1.next_cur += 800;
	s1.next_pack_cur += 64;
	step_prepack(&s1);
	assert(s1.next_cur == s1_mtx + 800);
	assert(s1.next_pack_cur == s1_mtx_pack + 64);
	assert(s1.sched_size == 64);
	assert(s1.mn_slice_len_real == 8);
	assert(s1.next_cur_bak == s1_mtx);
	assert(s1.slice_packed_size == 64);
	assert(s1.mn_len_remained == 64);
}

void s1_chunk() {
	prepack_state_t s1 = init_s1();
	for (int i = 0; i < 8; i++) {
		s1.next_cur += 800;
		s1.next_pack_cur += 64;
		step_prepack(&s1);
	}
	assert(s1.next_cur == s1_mtx + 8);
	assert(s1.next_pack_cur == s1_mtx_pack + 512);
	assert(s1.sched_size == 64);
	assert(s1.mn_slice_len_real == 8);
	assert(s1.next_cur_bak == s1_mtx + 8);
	assert(s1.slice_packed_size == 0);
	assert(s1.mn_len_remained == 56);
}

void s1_end() {
	prepack_state_t s1 = init_s1();
	for (int i = 0; i < 64; i++) {
		s1.next_cur += 800;
		s1.next_pack_cur += 64;
		step_prepack(&s1);
	}
	assert(s1.next_cur == s1_mtx + 64);
	assert(s1.next_pack_cur == s1_mtx_pack + 4096);
	assert(s1.sched_size == 64);
	assert(s1.mn_slice_len_real == 0);
	assert(s1.next_cur_bak == s1_mtx + 64);
	assert(s1.slice_packed_size == 0);
	assert(s1.mn_len_remained == 0);
}

void s2_chunk() {
	prepack_state_t s2 = init_s2();
	for (int i = 0; i < 2; i++) {
		s2.next_cur += 16;
		s2.next_pack_cur += 128;
		step_prepack(&s2);
	}
	assert(s2.next_cur == s2_mtx + 32);
	assert(s2.next_pack_cur == s2_mtx_pack + 256);
	assert(s2.sched_size == 80);
	assert(s2.mn_slice_len_real == 8);
	assert(s2.next_cur_bak == s2_mtx);
	assert(s2.slice_packed_size == 256);
	assert(s2.mn_len_remained == 63);
	s2.next_cur += 10;
	s2.next_pack_cur += 80;
	step_prepack(&s2);
	assert(s2.next_cur == s2_mtx + 800);
	assert(s2.next_pack_cur == s2_mtx_pack + 336);
	assert(s2.sched_size == 128);
	assert(s2.mn_slice_len_real == 8);
	assert(s2.next_cur_bak == s2_mtx + 800);
	assert(s2.slice_packed_size == 0);
	assert(s2.mn_len_remained == 55);
}

void  s2_chunkend() {
	prepack_state_t s2 = init_s2();
	for (int i = 0; i < 7; i++) {
		for (int j = 0; j < 2; j++) {
			s2.next_cur += 16;
			s2.next_pack_cur += 128;
			step_prepack(&s2);
		}
		s2.next_cur += 10;
		s2.next_pack_cur += 80;
		step_prepack(&s2);
	}
	assert(s2.next_cur == s2_mtx + 5600);
	assert(s2.next_pack_cur == s2_mtx_pack + 2352);
	assert(s2.sched_size == 128);
	assert(s2.mn_slice_len_real == 7);
	assert(s2.next_cur_bak == s2_mtx + 5600);
	assert(s2.slice_packed_size == 0);
	assert(s2.mn_len_remained == 7);
}

void s2_end() {
	prepack_state_t s2 = init_s2();
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 2; j++) {
			s2.next_cur += 16;
			s2.next_pack_cur += 128;
			step_prepack(&s2);
		}
		s2.next_cur += 10;
		s2.next_pack_cur += 80;
		step_prepack(&s2);
	}
	assert(s2.next_cur == s2_mtx + 6300);
	assert(s2.next_pack_cur == s2_mtx_pack + 2646);
	assert(s2.sched_size == 128);
	assert(s2.mn_slice_len_real == 0);
	assert(s2.next_cur_bak == s2_mtx + 6300);
	assert(s2.slice_packed_size == 0);
	assert(s2.mn_len_remained == 0);
}

int main() {
	s1_first();
	s1_chunk();
	s1_end();
	s2_chunk();
	s2_chunkend();
	return 0;
}
