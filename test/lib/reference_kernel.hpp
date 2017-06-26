#ifndef NANOBLAS_TEST_LIB_REFERENCE_KERNEL_HPP
#define NANOBLAS_TEST_LIB_REFERENCE_KERNEL_HPP

#include "nanoblas_kernel.hpp"

template<typename FTYPE>
struct RK {
	static void reference_pack_fun(nanoblas::prepack_state_t<FTYPE> *prepack_st) {
		for (int i = 0; i < prepack_st->sched_len; i++) {
			for (int j = 0; j < prepack_st->slice_len; j++) {
				const FTYPE *next_cur = reinterpret_cast<const FTYPE *>(
						reinterpret_cast<const char *>(prepack_st->next_cur) +
						prepack_st->interval_k * i + prepack_st->interval_mn * j);
				FTYPE v = (j < prepack_st->next_slice_real_len) ? *next_cur : 0;
				prepack_st->next_pack_cur[prepack_st->slice_len * i + j] = v;
			}
		}
		prepack_st->next_cur = reinterpret_cast<const FTYPE *>(
				reinterpret_cast<const char *>(prepack_st->next_cur) +
				prepack_st->interval_k * prepack_st->sched_len);
		prepack_st->next_pack_cur += prepack_st->slice_len * prepack_st->sched_len;
	}

	static void reference_kernel_fun(nanoblas::kernel_state_t<FTYPE> *kernel_st) {
		const int m_slice_len = reference_kernel.m_slice_len;
		const int n_slice_len = reference_kernel.n_slice_len;

		for (int k = 0; k < kernel_st->k_len; k++) {
			for (int m = 0; m < m_slice_len; m++) {
				for (int n = 0; n < n_slice_len; n++) {
					FTYPE *c_cur = reinterpret_cast<FTYPE *>(
							reinterpret_cast<char *>(kernel_st->c_cur) + kernel_st->ldc * m) + n;
					*c_cur += kernel_st->a_pack_cur[m_slice_len * k + m] *
							kernel_st->b_pack_cur[n_slice_len * k + n];
				}
			}
		}
		kernel_st->a_pack_cur += m_slice_len * kernel_st->k_len;
		kernel_st->b_pack_cur += n_slice_len * kernel_st->k_len;
		kernel_st->c_cur += n_slice_len;

		if (nanoblas::current_prepack_p<FTYPE>(kernel_st) != nullptr) {
			reference_pack_fun(nanoblas::current_prepack_p<FTYPE>(kernel_st));
		}
	}

	static thread_local nanoblas::kernel_t<FTYPE> reference_kernel;
};

template<typename FTYPE>
thread_local nanoblas::kernel_t<FTYPE> RK<FTYPE>::reference_kernel = {
	&RK<FTYPE>::reference_kernel_fun,
	&RK<FTYPE>::reference_pack_fun,
	0,
	0,
	0,
};

#endif
