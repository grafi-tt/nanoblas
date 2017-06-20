#include "nanoblas_prepack.h"
#include "internal/prepack.h"
#include "internal/util.h"

int step_prepack(prepack_state_t *st) {
	int retval = 0;
	st->slice_packed_size += st->sched_size;
	if (st->slice_packed_size == st->slice_size) {
		retval = 1;
		st->next_cur = st->next_cur_bak + st->interval_mn_slice;
		st->next_cur_bak = st->next_cur;
		st->slice_packed_size = 0;
		st->mn_len_remained -= st->mn_slice_len_real;
		st->mn_slice_len_real = imin(st->mn_slice_len_virtual, st->mn_len_remained);
	}
	st->sched_size = imin(st->sched_max_size, st->slice_size - st->slice_packed_size);
	return retval;
}

void pack_all(prepack_state_t *st) {
	do {
		do {
			do {
				FTYPE v = (st->mn_slice_pos++ < st->mn_slice_real_len) ? *st->next_cur : 0;
				*st->next_pack_cur++ = v;
				st->next_cur += st->interval_mn;
				st->slice_packed_size++;
			} while (st->mn_slice_pos < st->mn_slice_len_virtual);
			st->mn_slice_pos = 0;
			st->next_cur += st->proceed_k;
		} while (st->slice_packed_size < st->slice_size);
		st->next_cur = st->next_pack_cur_back + st->interval_mn_slice;
		st->next_cur_bak = st->next_cur
		st->slice_packed_size = 0;
		st->mn_len_remained -= st->mn_slice_len_real;
		st->mn_slice_len_real = imin(st->mn_slice_len_virtual, st->mn_len_remained);
	} while (st->mn_len_remained);
}
