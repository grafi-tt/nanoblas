#ifndef NANOBLAS_TEST_TEST_KERNEL_HPP
#define NANOBLAS_TEST_TEST_KERNEL_HPP

#include <iostream>
#include <random>
#include <vector>
#include "nanoblas_kernel.hpp"
#include "lib/util.hpp"
#include "lib/reference_kernel.hpp"

template<typename FTYPE>
bool run_mult_test(std::mt19937 gen, const nanoblas::kernel_t<FTYPE> &kernel,
		int k_len, int m_slice_real_len, int n_slice_real_len, ptrdiff_t ldc) {
	std::uniform_real_distribution<FTYPE> dist(0, 1);

	std::vector<FTYPE> a_pack(kernel.m_slice_len * k_len + 32/sizeof(FTYPE));
	std::vector<FTYPE> b_pack(kernel.n_slice_len * k_len + 32/sizeof(FTYPE));
	std::vector<FTYPE> c_cur(ldc * m_slice_real_len);
	std::vector<FTYPE> d_cur(ldc * m_slice_real_len);

	for (auto &&v: a_pack) v = dist(gen);
	for (auto &&v: b_pack) v = dist(gen);
	std::mt19937 gen2 = gen;
	for (auto &&v: c_cur) v = dist(gen);
	for (auto &&v: d_cur) v = dist(gen2);

	const FTYPE *a_pack_aligned = reinterpret_cast<const FTYPE*>(
			reinterpret_cast<uintptr_t>(a_pack.data()) + (-reinterpret_cast<uintptr_t>(a_pack.data()) & 31));
	const FTYPE *b_pack_aligned = reinterpret_cast<const FTYPE*>(
			reinterpret_cast<uintptr_t>(b_pack.data()) + (-reinterpret_cast<uintptr_t>(b_pack.data()) & 31));

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
	nanoblas::kernel_state_t<FTYPE> st = {
		a_pack_aligned, b_pack_aligned, c_cur.data(), ldc * static_cast<int>(sizeof(FTYPE)),
		m_slice_real_len, n_slice_real_len, k_len, 0
	};
#pragma GCC diagnostic pop

	RK<FTYPE>::reference_kernel.m_slice_len = kernel.m_slice_len;
	RK<FTYPE>::reference_kernel.n_slice_len = kernel.n_slice_len;
	RK<FTYPE>::reference_kernel.max_sched_len_fun = kernel.max_sched_len_fun;

	RK<FTYPE>::reference_kernel.kernel_fun(&st);

	st.a_pack_cur = a_pack_aligned;
	st.b_pack_cur = b_pack_aligned;
	st.c_cur = d_cur.data();

	kernel.kernel_fun(&st);

	bool s = false;
	if (st.a_pack_cur != a_pack_aligned + kernel.m_slice_len * k_len) {
		std::cerr << "a_pack_cur is incremented by "<<st.a_pack_cur - a_pack_aligned<<", not "<<kernel.m_slice_len * k_len << std::endl;
		s = true;
	}
	if (st.b_pack_cur != b_pack_aligned + kernel.n_slice_len * k_len) {
		std::cerr << "b_pack_cur is incremented by "<<st.b_pack_cur - b_pack_aligned<<", not "<<kernel.n_slice_len * k_len << std::endl;
		s = true;
	}
	if (st.c_cur != d_cur.data() + kernel.n_slice_len) {
		std::cerr << "c_cur is incremented by "<<st.c_cur - d_cur.data()<<", not "<<kernel.n_slice_len << std::endl;
		s = true;
	}
	for (int i = 0; i < m_slice_real_len; i++) {
		for (ptrdiff_t j = 0; j < ldc; j++) {
			FTYPE x = c_cur[ldc*i+j];
			FTYPE y = d_cur[ldc*i+j];
			if (!mostly_equal(x, y)) {
				std::cerr <<
					"c_cur["<<i<<"]["<<j<<"] = "<<x<<"; " <<
					"d_cur["<<i<<"]["<<j<<"] = "<<y<<";" << std::endl;
				return true;
			}
		}
	}
	return s;
}

template<typename FTYPE>
bool run_mult_test(std::mt19937 gen, const nanoblas::kernel_t<FTYPE> &kernel,
		int k_len, int m_slice_real_len, int n_slice_real_len) {
	return run_mult_test<FTYPE>(gen, kernel, k_len, m_slice_real_len, n_slice_real_len, n_slice_real_len);
}

template<typename FTYPE>
bool run_mult_test(std::mt19937 gen, const nanoblas::kernel_t<FTYPE> &kernel, int k_len) {
	return run_mult_test<FTYPE>(gen, kernel, k_len, kernel.m_slice_len, kernel.n_slice_len);
}

#endif
