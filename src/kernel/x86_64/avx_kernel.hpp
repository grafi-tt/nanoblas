#ifndef NANOBLAS_KERNEL_X86_64_AVX_KERNEL_HPP
#define NANOBLAS_KERNEL_X86_64_AVX_KERNEL_HPP

#include "nanoblas_types.hpp"
#include "kernel/x86_64/avx_kernel.h"

namespace nanoblas {

constexpr kernel_t<float> get_avx_kernel() {
	return {
		&nanoblas_f32_avx_kernel_mult,
		&nanoblas_f32_avx_kernel_pack,
		8, 8, 8
	};
}

}

#endif
