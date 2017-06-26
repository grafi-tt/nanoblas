#ifndef NANOBLAS_NANOBLAS_FTYPEYPES_HPP
#define NANOBLAS_NANOBLAS_FTYPEYPES_HPP

#include "nanoblas_types.h"

template<typename FTYPE> struct _nanoblas_prepack_state_t_impl { };
template<typename FTYPE>
using nanoblas_prepack_state_t = typename _nanoblas_prepack_state_t_impl<FTYPE>::ty;

template<>
struct _nanoblas_prepack_state_t_impl<float> {
	typedef nanoblas_f32_prepack_state_t ty;
};

template<>
struct _nanoblas_prepack_state_t_impl<double> {
	typedef nanoblas_f64_prepack_state_t ty;
};


template<typename FTYPE> struct _nanoblas_kernel_state_t_impl { };
template<typename FTYPE>
using nanoblas_kernel_state_t = typename _nanoblas_kernel_state_t_impl<FTYPE>::ty;

template<>
struct _nanoblas_kernel_state_t_impl<float> {
	typedef nanoblas_f32_kernel_state_t ty;
};

template<>
struct _nanoblas_kernel_state_t_impl<double> {
	typedef nanoblas_f64_kernel_state_t ty;
};


template<typename FTYPE> struct _nanoblas_kernel_fun_t_impl { };
template<typename FTYPE>
using nanoblas_kernel_fun_t = typename _nanoblas_kernel_fun_t_impl<FTYPE>::ty;

template<>
struct _nanoblas_kernel_fun_t_impl<float> {
	typedef nanoblas_f32_kernel_fun_t ty;
};

template<>
struct _nanoblas_kernel_fun_t_impl<double> {
	typedef nanoblas_f64_kernel_fun_t ty;
};


template<typename FTYPE> struct _nanoblas_pack_fun_t_impl { };
template<typename FTYPE>
using nanoblas_pack_fun_t = typename _nanoblas_pack_fun_t_impl<FTYPE>::ty;

template<>
struct _nanoblas_pack_fun_t_impl<float> {
	typedef nanoblas_f32_pack_fun_t ty;
};

template<>
struct _nanoblas_pack_fun_t_impl<double> {
	typedef nanoblas_f64_pack_fun_t ty;
};


template<typename FTYPE> struct _nanoblas_kernel_t_impl { };
template<typename FTYPE>
using nanoblas_kernel_t = typename _nanoblas_kernel_t_impl<FTYPE>::ty;

template<>
struct _nanoblas_kernel_t_impl<float> {
	typedef nanoblas_f32_kernel_t ty;
};

template<>
struct _nanoblas_kernel_t_impl<double> {
	typedef nanoblas_f64_kernel_t ty;
};

#endif
