#include "blocking.h"

void pack_block(int k_len, int m_or_n_len, size_t interval_k, size_t interval_m_or_n,
		FTYPE *restrict pack, const FTYPE *restrict a_or_b) {
	for (int m_or_n_pos = 0; m_or_n_pos < m_or_n_len; m_or_n_pos += UNIT_LEN) {
		int m_or_n_sub_len = imin(m_or_n_len - m_or_n_pos, UNIT_LEN);
		for (int k_cur = 0; k_cur < k_len; k_cur++) {
			for (int m_or_n_cur = m_or_n_pos; m_or_n_cur < m_or_n_pos + m_or_n_sub_len; m_or_n_cur++) {
				*++pack = a_or_b[interval_k*k_cur + interval_m_or_n*m_or_n_cur];
			}
		}
		a_or_b += k_len * m_or_n_sub_len;
	}
}

sched_len_t start_sched(pack_state_t *st) {
	st->k_sched_len = imin(st->k_next_len - st->k_packed_len, 2*st->unit_len);
	st->mn_sched_len = imin(st->mn_packed_len - st->mn_next_len, st->unit_len);
}

sched_len_t step_sched(pack_st_t *st) {
	if (st->k_sched_len == 0) return;
	st->next_pack_cur += st->k_sched_len * st->mn_sched_Len;
	st->k_packed_len += st->k_sched_len;
	if (st->k_packed_len != st->k_next_len) {
		st->next_cur += st->k_sched_len*interval_k;
	} else {
		st->next_cur += -st->k_packed_len*interval_k + st->mn_sched_len*interval_mn;
		st->k_packed_len = 0;
		st->mn_packed_len += st->mn_sched_len;
		st->mn_sched_len = imin(st->mn_packed_len - st->mn_next_len, st->unit_len);
		if (st->mn_sched_len == 0) {
			st->k_sched_len == 0;
			return;
		}
	}
	st->k_sched_len = imin(st->k_next_len - st->k_packed_len, 2*st->unit_len);
}
