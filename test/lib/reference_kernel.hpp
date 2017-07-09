#ifndef NANOBLAS_TEST_LIB_REFERENCE_KERNEL_HPP
#define NANOBLAS_TEST_LIB_REFERENCE_KERNEL_HPP

#include "nanoblas_kernel.hpp"

using namespace nanoblas;

template<typename FTYPE>
void reference_kernel_pack(prepack_state_t<FTYPE> *prepack_st) {
	for (int i = 0; i < prepack_st->remained_len; i++) {
		for (int j = 0; j < prepack_st->slice_len; j++) {
			const FTYPE *next_cur = reinterpret_cast<const FTYPE *>(
					reinterpret_cast<uintptr_t>(prepack_st->next_cur) +
					prepack_st->interval_k * i + prepack_st->interval_mn * j);
			FTYPE v = (j < prepack_st->next_slice_real_len) ? *next_cur : 0;
			prepack_st->next_pack_cur[prepack_st->slice_len * i + j] = v;
		}
	}
	prepack_st->next_cur = reinterpret_cast<const FTYPE *>(
			reinterpret_cast<uintptr_t>(prepack_st->next_cur) +
			prepack_st->interval_k * prepack_st->remained_len);
	prepack_st->next_pack_cur += prepack_st->slice_len * prepack_st->remained_len;
	prepack_st->remained_len = 0;
}

template<typename FTYPE>
void reference_kernel_mult(kernel_state_t<FTYPE> *kernel_st, int m_slice_len, int n_slice_len) {
	// acc to buf
	for (int l = 0; l < kernel_st->k_len; l++) {
		for (int i = 0; i < m_slice_len; i++) {
			for (int j = 0; j < n_slice_len; j++) {
				kernel_st->c_buf[n_slice_len*i + j] +=
					kernel_st->a_pack_cur[m_slice_len*l + i] * kernel_st->b_pack_cur[n_slice_len*l + j];
			}
		}
	}
	// store
	for (int i = 0; i < m_slice_len; i++) {
		for (int j = 0; j < n_slice_len; j++) {
			if (i < kernel_st->m_slice_real_len && j < kernel_st->n_slice_real_len) {
				FTYPE *c_cur = reinterpret_cast<FTYPE *>(
						reinterpret_cast<uintptr_t>(kernel_st->c_cur) + kernel_st->ldc * i) + j;
				*c_cur = kernel_st->c_buf[n_slice_len*i + j];
			}
		}
	}
	// load
	for (int i = 0; i < m_slice_len; i++) {
		for (int j = 0; j < n_slice_len; j++) {
			if (i < kernel_st->m_next_slice_real_len && j < kernel_st->n_next_slice_real_len) {
				FTYPE *c_next_cur = reinterpret_cast<FTYPE *>(
						reinterpret_cast<uintptr_t>(kernel_st->c_next_cur) + kernel_st->ldc * i) + j;
				kernel_st->c_buf[n_slice_len*i + j] = *c_next_cur;
			} else {
				kernel_st->c_buf[n_slice_len*i + j] = 0;
			}
		}
	}

	kernel_st->a_pack_cur += m_slice_len * kernel_st->k_len;
	kernel_st->b_pack_cur += n_slice_len * kernel_st->k_len;
	kernel_st->c_cur = kernel_st->c_next_cur;
	kernel_st->c_next_cur += n_slice_len;

	if (current_prepack_p<FTYPE>(kernel_st) != nullptr) {
		reference_kernel_pack<FTYPE>(current_prepack_p<FTYPE>(kernel_st));
	}
}

template<typename FTYPE, int m_slice_len, int n_slice_len>
void reference_kernel_mult(kernel_state_t<FTYPE> *kernel_st) {
	return reference_kernel_mult<FTYPE>(kernel_st, m_slice_len, n_slice_len);
}

template<typename FTYPE, int m_slice_len, int n_slice_len, int k_unit_len = 1>
constexpr kernel_t<FTYPE> get_reference_kernel() {
	return {
		&reference_kernel_mult<FTYPE, m_slice_len, n_slice_len>,
		&reference_kernel_pack<FTYPE>,
		m_slice_len,
		n_slice_len,
		k_unit_len,
	};
}

#endif
