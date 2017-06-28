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
				s = true;
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


template<typename FTYPE>
bool run_pack_test(std::mt19937 gen, const nanoblas::kernel_t<FTYPE> &kernel,
		int len, int packing_slice_len, int packing_slice_real_len,
		ptrdiff_t interval_mn, ptrdiff_t interval_k) {
	std::uniform_real_distribution<FTYPE> dist(0, 1);


	std::vector<FTYPE> mtx(std::max(packing_slice_len * interval_mn, len * interval_k));
	std::vector<FTYPE> ans_pack(packing_slice_len * (len + 8) + 32/sizeof(FTYPE));
	std::vector<FTYPE> tst_pack(packing_slice_len * (len + 8) + 32/sizeof(FTYPE));

	for (auto &&v: mtx) v = dist(gen);
	std::mt19937 gen2 = gen;
	for (auto &&v: ans_pack) v = dist(gen);
	for (auto &&v: tst_pack) v = dist(gen2);

	FTYPE *ans_pack_aligned = reinterpret_cast<FTYPE*>(
			reinterpret_cast<uintptr_t>(ans_pack.data()) + (-reinterpret_cast<uintptr_t>(ans_pack.data()) & 31));
	FTYPE *tst_pack_aligned = reinterpret_cast<FTYPE*>(
			reinterpret_cast<uintptr_t>(tst_pack.data()) + (-reinterpret_cast<uintptr_t>(tst_pack.data()) & 31));

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
	nanoblas::prepack_state_t<FTYPE> st = {
		mtx.data(),
		ans_pack_aligned,
		interval_mn * static_cast<int>(sizeof(FTYPE)),
		interval_k * static_cast<int>(sizeof(FTYPE)),
		packing_slice_len,
		packing_slice_real_len,
		len,
	};
#pragma GCC diagnostic pop

	RK<FTYPE>::reference_kernel.m_slice_len = kernel.m_slice_len;
	RK<FTYPE>::reference_kernel.n_slice_len = kernel.n_slice_len;
	RK<FTYPE>::reference_kernel.max_sched_len_fun = kernel.max_sched_len_fun;

	RK<FTYPE>::reference_kernel.pack_fun(&st);

	st.next_cur = mtx.data();
	st.next_pack_cur = tst_pack_aligned;

	kernel.pack_fun(&st);

	bool s = false;
	if (st.next_cur != mtx.data() + interval_k * len) {
		std::cerr << "next_cur is incremented by "<<st.next_cur - mtx.data()<<", not "<<interval_k * len << std::endl;
		s = true;
	}
	for (int i = 0; i < len; i++) {
		for (int j = 0; j < packing_slice_len; j++) {
			FTYPE x = ans_pack_aligned[packing_slice_len*i+j];
			FTYPE y = tst_pack_aligned[packing_slice_len*i+j];
			if (!mostly_equal(x, y)) {
				std::cerr <<
					"ans_pack["<<i<<"]["<<j<<"] = "<<x<<"; " <<
					"tst_pack["<<i<<"]["<<j<<"] = "<<y<<";" << std::endl;
				s = true;
			}
		}
	}
	return s;
}

template<typename FTYPE>
bool run_pack_test(std::mt19937 gen, const nanoblas::kernel_t<FTYPE> &kernel,
		int len, int packing_slice_len,
		ptrdiff_t interval_mn, ptrdiff_t interval_k) {
	return run_pack_test<FTYPE>(gen, kernel, len, packing_slice_len, packing_slice_len, interval_mn, interval_k);
}

#endif
