#ifndef NANOBLAS_PACK_H
#define NANOBLAS_PACK_H

#include <stddef.h>
#include "const.h"

#define pack_block APPEND_FTYPE(pack_block)
void pack_block(int k_len, int m_or_n_len, size_t interval_k, size_t interval_m_or_n,
		FTYPE *restrict pack, const FTYPE *restrict a_or_b);

#define pack_state_t APPEND_FTYPE(pack_state_t)
typedef struct {
	FTYPE *next_cur;
	FTYPE *next_pack_cur;
	const int k_next_len;
	const int mn_next_len;
	const size_t interval_k;
	const size_t interval_mn;
	int k_sched_len;
	int mn_sched_len;
	int k_packed_len;
	int mn_packed_len;
} pack_state_t;

#define start_sched APPEND_FTYPE(start_sched)
sched_len_t start_sched(pack_state_t *st);

#define step_sched APPEND_FTYPE(step_sched)
sched_len_t step_sched(pack_state_t *st);

#endif
