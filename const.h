#ifndef NANOBLAS_CONST_H
#define NANOBLAS_CONST_H

typedef float f32;
typedef double f64;

#if defined(USE_F32) ^ defined(USE_F64)

#ifdef USE_F32
#define FTYPE f32
#define BLK_SZ 96
#define PREFIX s
#endif

#ifdef USE_F64
#define FTYPE f64
#define BLK_SZ 128
#define PREFIX d
#endif

#else
#error please specify one of USE_F32 or USE_F64
#endif

#endif
