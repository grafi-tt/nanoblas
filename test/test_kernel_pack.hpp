#ifndef NANOBLAS_TEST_TEST_KERNEL_PACK_HPP
#define NANOBLAS_TEST_TEST_KERNEL_PACK_HPP

#include <iostream>
#include <random>
#include <vector>
#include "nanoblas_kernel.hpp"
#include "lib/util.hpp"
#include "lib/reference_kernel.hpp"

using namespace nanoblas;

template<typename FTYPE>
bool run_pack_test(std::mt19937 gen, const kernel_t<FTYPE> &kernel,
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
	prepack_state_t<FTYPE> st = {
		mtx.data(),
		ans_pack_aligned,
		interval_mn * static_cast<int>(sizeof(FTYPE)),
		interval_k * static_cast<int>(sizeof(FTYPE)),
		packing_slice_len,
		len,
		packing_slice_real_len,
	};
#pragma GCC diagnostic pop

	reference_kernel_pack<FTYPE>(&st);

	st.next_cur = mtx.data();
	st.next_pack_cur = tst_pack_aligned;
	st.remained_len = len;

	kernel.pack(&st);

	bool s = false;
	if (st.next_pack_cur != tst_pack_aligned + packing_slice_len * len) {
		std::cerr << "next_pack_cur is incremented by "<<st.next_pack_cur - tst_pack_aligned<<", not "<<packing_slice_len * len << std::endl;
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
bool run_pack_test(std::mt19937 gen, const kernel_t<FTYPE> &kernel,
		int len, int packing_slice_len,
		ptrdiff_t interval_mn, ptrdiff_t interval_k) {
	return run_pack_test<FTYPE>(gen, kernel, len, packing_slice_len, packing_slice_len, interval_mn, interval_k);
}

#endif
