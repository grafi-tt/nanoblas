#ifndef NANOBLAS_NANOBLAS_TYPES_HPP
#define NANOBLAS_NANOBLAS_TYPES_HPP

#include "nanoblas_types.h"

namespace nanoblas {

template<typename FTYPE>
struct _prepack_state_t_impl { };

template<>
struct _prepack_state_t_impl<float> {
	typedef nanoblas_f32_prepack_state_t ty;
};
template<>
struct _prepack_state_t_impl<double> {
	typedef nanoblas_f64_prepack_state_t ty;
};

template<typename FTYPE>
using prepack_state_t = typename _prepack_state_t_impl<FTYPE>::ty;


template<typename FTYPE>
struct _kernel_state_t_impl { };

template<>
struct _kernel_state_t_impl<float> {
	typedef nanoblas_f32_kernel_state_t ty;
};
template<>
struct _kernel_state_t_impl<double> {
	typedef nanoblas_f64_kernel_state_t ty;
};

template<typename FTYPE>
using kernel_state_t = typename _kernel_state_t_impl<FTYPE>::ty;


template<typename FTYPE>
struct _kernel_mult_t_impl { };

template<>
struct _kernel_mult_t_impl<float> {
	typedef nanoblas_f32_kernel_mult_t ty;
};
template<>
struct _kernel_mult_t_impl<double> {
	typedef nanoblas_f64_kernel_mult_t ty;
};

template<typename FTYPE>
using kernel_mult_t = typename _kernel_mult_t_impl<FTYPE>::ty;


template<typename FTYPE>
struct _kernel_pack_t_impl { };

template<>
struct _kernel_pack_t_impl<float> {
	typedef nanoblas_f32_kernel_pack_t ty;
};
template<>
struct _kernel_pack_t_impl<double> {
	typedef nanoblas_f64_kernel_pack_t ty;
};

template<typename FTYPE>
using kernel_pack_t = typename _kernel_pack_t_impl<FTYPE>::ty;


template<typename FTYPE>
struct _kernel_t_impl { };

template<>
struct _kernel_t_impl<float> {
	typedef nanoblas_f32_kernel_t ty;
};
template<>
struct _kernel_t_impl<double> {
	typedef nanoblas_f64_kernel_t ty;
};

template<typename FTYPE>
using kernel_t = typename _kernel_t_impl<FTYPE>::ty;

}

#endif
