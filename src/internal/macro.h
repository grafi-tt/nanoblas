#ifndef NANOBLAS_INTERNAL_MACRO_H
#define NANOBLAS_INTERNAL_MACRO_H

typedef float f32;
typedef double f64;

#if defined(USE_F32) ^ defined(USE_F64)

#ifdef USE_F32
#define FTYPE f32
#define BLK_MN_LEN 80
#define BLK_K_LEN 64
#define PREFIX s
#endif

#ifdef USE_F64
#define FTYPE f64
#define BLK_MN_LEN 40
#define BLK_K_LEN 64
#define PREFIX d
#endif

#else
#error please specify one of USE_F32 or USE_F64
#endif

#endif

#define ADD_FTYPE(name) ADD_FTYPE_HELPER1(name, FTYPE)
#define ADD_FTYPE_HELPER1(name, typ) ADD_FTYPE_HELPER2(name, typ)
#define ADD_FTYPE_HELPER2(name, typ) typ##_##name

#define ADD_PREFIX(name) ADD_PREFIX_HELPER1(name, PREFIX)
#define ADD_PREFIX_HELPER1(name, typ) ADD_PREFIX_HELPER2(name, typ)
#define ADD_PREFIX_HELPER2(name, typ) typ##name

#define kernel_t ADD_FTYPE(kernel_t)
#define kernel_fun_t ADD_FTYPE(kernel_fun_t)
#define sched_state_t ADD_FTYPE(sched_state_t)
#define decide_kernel ADD_FTYPE(decide_kernel)
