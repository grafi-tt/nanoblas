#ifndef NANOBLAS_TEST_TEST_KERNEL_MULT_HPP
#define NANOBLAS_TEST_TEST_KERNEL_MULT_HPP

#include <iostream>
#include <random>
#include <vector>
#include "nanoblas_kernel.hpp"
#include "lib/util.hpp"
#include "lib/reference_kernel.hpp"

using namespace nanoblas;

template<typename FTYPE>
bool run_mult_test(std::mt19937 gen, const kernel_t<FTYPE> &kernel,
		int k_len, int m_slice_real_len, int n_slice_real_len, int m_next_slice_real_len, int n_next_slice_real_len, ptrdiff_t ldc) {
	std::uniform_real_distribution<FTYPE> dist(0, 1);

	int m_slice_len = kernel.m_slice_len;
	int n_slice_len = kernel.n_slice_len;

	std::vector<FTYPE> a_pack(kernel.m_slice_len * k_len + 32/sizeof(FTYPE));
	std::vector<FTYPE> b_pack(kernel.n_slice_len * k_len + 32/sizeof(FTYPE));
	std::vector<FTYPE> c1_buf(kernel.m_slice_len * kernel.n_slice_len + 32/sizeof(FTYPE));
	std::vector<FTYPE> c2_buf(kernel.m_slice_len * kernel.n_slice_len + 32/sizeof(FTYPE));
	std::vector<FTYPE> c1_cur(ldc * m_slice_real_len);
	std::vector<FTYPE> c2_cur(ldc * m_slice_real_len);
	std::vector<FTYPE> c1_next_cur(ldc * m_next_slice_real_len);
	std::vector<FTYPE> c2_next_cur(ldc * m_next_slice_real_len);

	FTYPE *a_pack_aligned = aligned_ptr(a_pack);
	FTYPE *b_pack_aligned = aligned_ptr(b_pack);
	FTYPE *c1_buf_aligned = aligned_ptr(c1_buf);
	FTYPE *c2_buf_aligned = aligned_ptr(c2_buf);

	for (int l = 0; l < k_len; l++) {
		for (int i = 0; i < m_slice_len; i++) {
			a_pack_aligned[m_slice_len*l + i] = i < m_slice_real_len ? dist(gen) : 0;
		}
	}
	for (int l = 0; l < k_len; l++) {
		for (int j = 0; j < n_slice_len; j++) {
			b_pack_aligned[n_slice_len*l + j] = j < n_slice_real_len ? dist(gen) : 0;
		}
	}
	std::mt19937 gen2 = gen;
	for (int i = 0; i < kernel.m_slice_len; i++) {
		for (int j = 0; j < kernel.n_slice_len; j++) {
			if (i < m_slice_real_len && j < n_slice_real_len) {
				c1_buf_aligned[kernel.n_slice_len*i + j] = dist(gen);
				c2_buf_aligned[kernel.n_slice_len*i + j] = dist(gen2);
			} else {
				c1_buf_aligned[kernel.n_slice_len*i + j] = 0;
				c2_buf_aligned[kernel.n_slice_len*i + j] = 0;
			}
		}
	}

	for (auto &&v: c1_cur) v = dist(gen);
	for (auto &&v: c2_cur) v = dist(gen2);
	for (auto &&v: c1_next_cur) v = dist(gen);
	for (auto &&v: c2_next_cur) v = dist(gen2);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
	kernel_state_t<FTYPE> st = {
		a_pack_aligned, b_pack_aligned, c1_buf_aligned, c1_cur.data(), c1_next_cur.data(), ldc * static_cast<int>(sizeof(FTYPE)),
		m_slice_real_len, n_slice_real_len, m_next_slice_real_len, n_next_slice_real_len, k_len, 0
	};
#pragma GCC diagnostic pop

	reference_kernel_mult<FTYPE>(&st, kernel.m_slice_len, kernel.n_slice_len);

	st.a_pack_cur = a_pack_aligned;
	st.b_pack_cur = b_pack_aligned;
	st.c_buf = c2_buf_aligned;
	st.c_cur = c2_cur.data();
	st.c_next_cur = c2_next_cur.data();

	kernel.mult(&st);

	bool s = false;
	if (st.a_pack_cur != a_pack_aligned + kernel.m_slice_len * k_len) {
		std::cerr << "a_pack_cur is incremented by "<<st.a_pack_cur - a_pack_aligned<<", not "<<kernel.m_slice_len * k_len << std::endl;
		s = true;
	}
	if (st.b_pack_cur != b_pack_aligned + kernel.n_slice_len * k_len) {
		std::cerr << "b_pack_cur is incremented by "<<st.b_pack_cur - b_pack_aligned<<", not "<<kernel.n_slice_len * k_len << std::endl;
		s = true;
	}
	if (st.c_cur != c2_next_cur.data()) {
		std::cerr << "c_cur is not same as previous c_next_cur";
		s = true;
	}
	if (st.c_next_cur != c2_next_cur.data() + kernel.n_slice_len) {
		std::cerr << "c_cur is incremented by "<<st.c_cur - c2_cur.data()<<", not "<<kernel.n_slice_len << std::endl;
		s = true;
	}
	s = check_matrix(c1_cur, c2_cur, m_slice_real_len, ldc) || s;
	s = check_matrix(c1_next_cur, c2_next_cur, m_next_slice_real_len, ldc) || s;
	s = check_matrix(c1_buf_aligned, c2_buf_aligned, kernel.m_slice_len, kernel.n_slice_len) || s;
	return s;
}

template<typename FTYPE>
bool run_mult_test(std::mt19937 gen, const kernel_t<FTYPE> &kernel,
		int k_len, int m_slice_real_len, int n_slice_real_len, int m_next_slice_real_len, int n_next_slice_real_len) {
	return run_mult_test<FTYPE>(gen, kernel, k_len,
			m_slice_real_len, n_slice_real_len, m_next_slice_real_len, n_next_slice_real_len,
			n_slice_real_len + n_next_slice_real_len);
}

template<typename FTYPE>
bool run_mult_test(std::mt19937 gen, const kernel_t<FTYPE> &kernel, int k_len) {
	return run_mult_test<FTYPE>(gen, kernel, k_len, kernel.m_slice_len, kernel.n_slice_len, kernel.m_slice_len, kernel.n_slice_len);
}

#endif
