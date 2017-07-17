#ifndef NANOBLAS_TEST_LIB_UTIL_HPP
#define NANOBLAS_TEST_LIB_UTIL_HPP

#include <cstdint>
#include <iostream>
#include <vector>

template<typename T>
constexpr T eps();
template<>
constexpr float eps() {
	return 1e-4;
}
template<>
constexpr double eps() {
	return 1e-8;
}

template<typename T>
bool mostly_equal(T x, T y) {
	return std::abs(x - y) / std::max(std::max(std::abs(x), std::abs(y)), eps<T>()) < eps<T>();
}

template<typename FTYPE>
FTYPE *aligned_ptr(std::vector<FTYPE> &d, int alignment=32) {
	return reinterpret_cast<FTYPE *>(
			reinterpret_cast<uintptr_t>(d.data()) + (-reinterpret_cast<uintptr_t>(d.data()) & (alignment-1)));
}

template <typename T>
bool check_matrix_impl(const std::string &A_name, const std::string &B_name, T A, T B,
		long long M, long long N, long long ld, long long mshift) {
	bool s = false;
	for (long long i = 0; i < M; i++) {
		for (long long j = 0; j < N; j++) {
			auto x = A[ld*i+j];
			auto y = B[ld*i+j];
			if (!mostly_equal(x, y)) {
				s = true;
				std::cout <<
					A_name<<"["<<M-mshift<<"]["<<j<<"] = "<<x<<"; " <<
					B_name<<"["<<M-mshift<<"]["<<j<<"] = "<<y<<";" << std::endl;
			}
		}
	}
	return s;
}
template <typename T>
bool check_matrix_impl(const std::string &A_name, const std::string &B_name, T A, T B,
		long long M, long long N, long long ld) {
	return check_matrix_impl(A_name, B_name, A, B, M, N, ld, 0);
}
template <typename T>
bool check_matrix_impl(const std::string &A_name, const std::string &B_name, T A, T B,
		long long M, long long N) {
	return check_matrix_impl(A_name, B_name, A, B, M, N, N);
}
#define check_matrix(A, B, ...) check_matrix_impl(#A, #B, A, B, __VA_ARGS__)

template <typename T>
bool check_ptr_impl(const std::string &p_name, const T *p, const T *base_p, ptrdiff_t d) {
	ptrdiff_t actual_d = p - base_p;
	if (actual_d != d) {
		std::cout << p_name<<" is incremented by "<<actual_d<<", not "<<d << std::endl;
		return true;
	} else {
		return false;
	}
}
#define check_ptr(p, ...) check_ptr_impl(#p, p, __VA_ARGS__)

#endif
