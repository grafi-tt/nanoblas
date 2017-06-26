#ifndef NANOBLAS_NANOBLAS_KERNEL_HPP
#define NANOBLAS_NANOBLAS_KERNEL_HPP

#include "nanoblas_kernel.h"
#include "nanoblas_types.hpp"

namespace nanoblas {

template<typename FTYPE>
constexpr int current_prepack_a();

template<>
constexpr int current_prepack_a<float>() {
	return sizeof(struct _nanoblas_f32_kernel_state_former_t);
}

template<>
constexpr int current_prepack_a<double>() {
	return sizeof(struct _nanoblas_f64_kernel_state_former_t);
}


template<typename FTYPE>
constexpr int current_prepack_b();

template<>
constexpr int current_prepack_b<float>() {
	return sizeof(struct _nanoblas_f32_kernel_state_former_t) + sizeof(nanoblas_f32_prepack_state_t);
}

template<>
constexpr int current_prepack_b<double>() {
	return sizeof(struct _nanoblas_f64_kernel_state_former_t) + sizeof(nanoblas_f64_prepack_state_t);
}

template<typename FTYPE>
static inline prepack_state_t<FTYPE> *current_prepack_p(kernel_state_t<FTYPE> *st) {
	return st->current_prepack == 0 ?
		nullptr : reinterpret_cast<prepack_state_t<FTYPE> *>(reinterpret_cast<uintptr_t>(st) + st->current_prepack);
}

}

#endif
