#ifndef NANOBLAS_CONST_H
#define NANOBLAS_CONST_H

typedef float f32;
typedef double f64;

#if defined(USE_F32) ^ defined(USE_F64)

#ifdef USE_F32
#define FTYPE f32
#define BLK_MN_LEN 128
#define BLK_K_LEN 128
#define UNIT_LEN 8
#define PREFIX s
#endif

#ifdef USE_F64
#define FTYPE f64
#define BLK_MN_LEN 64
#define BLK_K_LEN 128
#define UNIT_LEN 8
#define PREFIX d
#endif

#else
#error please specify one of USE_F32 or USE_F64
#endif

#endif

#define APPEND_FTYPE(name) APPEND_FTYPE_HELPER1(name, FTYPE)
#define APPEND_FTYPE_HELPER1(name, typ) APPEND_FTYPE_HELPER2(name, typ)
#define APPEND_FTYPE_HELPER2(name, typ) name##_##typ

#define ADD_PREFIX(name) ADD_PREFIX_HELPER1(name, PREFIX)
#define ADD_PREFIX_HELPER1(name, typ) ADD_PREFIX_HELPER2(name, typ)
#define ADD_PREFIX_HELPER2(name, typ) typ##name
