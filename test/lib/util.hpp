#ifndef NANOBLAS_TEST_LIB_UTIL_HPP
#define NANOBLAS_TEST_LIB_UTIL_HPP

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

#endif
