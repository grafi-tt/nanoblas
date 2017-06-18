#ifndef NANOBLAS_INTERNAL_MACRO_H
#define NANOBLAS_INTERNAL_MACRO_H

typedef float f32;
typedef double f64;

#if defined(USE_F32) ^ defined(USE_F64)

#ifdef USE_F32
#define FTYPE f32
#define PREFIX nanoblas_s
#endif

#ifdef USE_F64
#define FTYPE f64
#define PREFIX nanoblas_d
#endif

#else
#error please specify one of USE_F32 or USE_F64
#endif

#define ADD_FTYPE(name) ADD_FTYPE_HELPER1(name, FTYPE)
#define ADD_FTYPE_HELPER1(name, typ) ADD_FTYPE_HELPER2(name, typ)
#define ADD_FTYPE_HELPER2(name, typ) typ##_##name

#define ADD_PREFIX(name) ADD_PREFIX_HELPER1(name, PREFIX)
#define ADD_PREFIX_HELPER1(name, typ) ADD_PREFIX_HELPER2(name, typ)
#define ADD_PREFIX_HELPER2(name, typ) typ##name

/* types */
#define kernel_t ADD_FTYPE(kernel_t)
#define kernel_fun_t ADD_FTYPE(kernel_fun_t)
#define kernel_state_t ADD_FTYPE(kernel_state_t)
#define prepack_state_t ADD_FTYPE(prepack_state_t)

/* members of nanoblas_t */
#define kernel ADD_FTYPE(kernel)
#define blk_m_len ADD_FTYPE(blk_m_len)
#define blk_n_len ADD_FTYPE(blk_n_len)
#define blk_k_len ADD_FTYPE(blk_k_len)

#endif
