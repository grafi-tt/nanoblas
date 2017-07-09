#ifndef NANOBLAS_NANOBLAS_INTERNAL_KERNEL_H
#define NANOBLAS_NANOBLAS_INTERNAL_KERNEL_H

#include "nanoblas_kernel.h"
#include "internal/macro.h"
#include "internal/prepack.h"
#include "internal/util.h"

#ifdef __cplusplus
extern "C" {
#endif

#define swap_a_pack JOIN(NAMESPACE, FSIZE_PREFIX, swap_a_pack)
static inline void swap_a_pack(kernel_state_t *st, kernel_pack_t *kernel_pack) {
	st->current_prepack = current_prepack_a;
	pack_slice(current_prepack_p(st), kernel_pack);
	fswap(&st->a_pack, &st->a_next_pack);
	st->a_pack_cur                  = st->a_pack;
	st->prepack.mem.a_next_pack_cur = st->a_next_pack;
}

#define swap_b_pack JOIN(NAMESPACE, FSIZE_PREFIX, swap_b_pack)
static inline void swap_b_pack(kernel_state_t *st, kernel_pack_t *kernel_pack) {
	st->current_prepack = current_prepack_b;
	pack_all(current_prepack_p(st), kernel_pack);
	fswap(&st->b_pack, &st->b_next_pack);
	st->b_pack_cur                  = st->b_pack;
	st->prepack.mem.b_next_pack_cur = st->b_next_pack;
}

#define kernel_state_new JOIN(NAMESPACE, FSIZE_PREFIX, kernel_state_new)
static inline kernel_state_t kernel_state_new(
		const FTYPE *a, const FTYPE *b, const FTYPE *c,
		size_t interval_m, size_t interval_n, size_t interval_k_in_a, size_t interval_k_in_b, size_t ldc,
		FTYPE *a_pack, FTYPE *a_next_pack, FTYPE *b_pack, FTYPE *b_next_pack, FTYPE *c_buf,
		int m_slice_len, int n_slice_len,
		int m_slice_len_limit, int n_slice_len_limit, int k_len) {

	kernel_state_t st = {
		.a_pack      = a_pack,
		.a_next_pack = a_next_pack,
		.b_pack      = b_pack,
		.b_next_pack = b_next_pack,
		.c_buf       = c_buf,
		.ldc         = sizeof(FTYPE) * ldc,
		.prepack.sel.a = prepack_state_new(
				a, a_next_pack, m_slice_len, m_slice_len_limit, k_len, interval_m, interval_k_in_a),
		.prepack.sel.b = prepack_state_new(
				b, b_next_pack, n_slice_len, n_slice_len_limit, k_len, interval_n, interval_k_in_b),
	};
	st.m_next_slice_real_len = st.prepack.sel.a.next_slice_real_len;
	st.n_next_slice_real_len = st.prepack.sel.b.next_slice_real_len;

	for (int i = 0; i < m_slice_len; i++)
		for (int j = 0; j < n_slice_len; j++)
			st.c_buf[m_slice_len*i + j] =
				i < st.m_next_slice_real_len && j < st.n_next_slice_real_len ? c[ldc*i + j] : 0;
	return st;
}

#define update_prepack JOIN(NAMESPACE, FSIZE_PREFIX, update_prepack)
static inline void update_prepack(kernel_state_t *st) {
	if (st->current_prepack != 0) {
		step_prepack(current_prepack_p(st));
		if (is_packed(current_prepack_p(st))) {
			if (st->current_prepack != current_prepack_b) {
				st->current_prepack = current_prepack_b;
			}
			if (is_packed(current_prepack_p(st))) {
				st->current_prepack = 0;
			}
		}
	}
}

#ifdef __cplusplus
}
#endif

#endif
