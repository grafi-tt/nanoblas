#ifndef NANOBLAS_KERNEL_GENERIC_KERNEL_HPP
#define NANOBLAS_KERNEL_GENERIC_KERNEL_HPP

#include "nanoblas_types.hpp"
#include "kernel/generic_kernel.h"

namespace nanoblas {

template<typename T>
constexpr kernel_t<T> get_generic_kernel_4x4();

template<>
constexpr kernel_t<float> get_generic_kernel_4x4<float>() {
	 return {
		&nanoblas_f32_generic_kernel_mult_4x4,
		&nanoblas_f32_generic_kernel_pack_4x4,
		4, 4, 1
	};
}
template<>
constexpr kernel_t<double> get_generic_kernel_4x4<double>() {
	return {
		&nanoblas_f64_generic_kernel_mult_4x4,
		&nanoblas_f64_generic_kernel_pack_4x4,
		4, 4, 1
	};
}

template<typename T>
constexpr kernel_t<T> get_generic_kernel_6x4();

template<>
constexpr kernel_t<float> get_generic_kernel_6x4<float>() {
	 return {
		&nanoblas_f32_generic_kernel_mult_6x4,
		&nanoblas_f32_generic_kernel_pack_6x4,
		6, 4, 1
	};
}
template<>
constexpr kernel_t<double> get_generic_kernel_6x4<double>() {
	return {
		&nanoblas_f64_generic_kernel_mult_6x4,
		&nanoblas_f64_generic_kernel_pack_6x4,
		6, 4, 1
	};
}

}

#endif
