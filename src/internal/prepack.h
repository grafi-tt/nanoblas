#ifndef NANOBLAS_INTERNAL_PREPACK_H
#define NANOBLAS_INTERNAL_PREPACK_H

#include "nanoblas_types.h"
#include "internal/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

#define prepack_state_new JOIN(NAMESPACE, PREFIX, start_prepack)
static inline prepack_state_t prepack_state_new(
		const FTYPE *next, FTYPE *next_pack,
		int mn_len, int k_len, int unit_len, int max_sched_size,
		ptrdiff_t interva_mn, ptrdiff_t interval_k) {

	return {
		.next_cur = next,
		.next_pack_cur = next_pack,
		.mn_slice_pos = 0,
		.sched_size = imin(max_sched_size, unit_len*k_len),
		.mn_slice_len_virtual = unit_len,
		.mn_slice_len_real = imin(unit_len, mn_len),
		.proceed_k = interval_k - unit_len*interval_mn,
		.next_pack_cur_bak = next_pack,
		.slice_packed_size = 0,
		.mn_len_remained = mn_len,
		.slice_size = unit_len*k_len,
		.max_sched_size = max_sched_size,
		.interval_m = interval_mn,
		.interval_k = interval_k,
	}
}

#define step_prepack JOIN(NAMESPACE, PREFIX, step_prepack)
void step_prepack(prepack_state_t *st);

#define pack_all JOIN(NAMESPACE, PREFIX, pack_all)
void pack_all(prepack_state_t *st);

#ifdef __cplusplus
}
#endif

#endif
