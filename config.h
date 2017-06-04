#ifndef NANOBLAS_CONFIG_H
#define NANOBLAS_CONFIG_H

#if defined(USE_F32) ^ defined(USE_F64)

#ifdef USE_F32
#define FTYPE double
#define BLK_SZ 96
#endif

#ifdef USE_F64
#define FTYPE float
#define BLK_SZ 128
#endif

#else
#error please specify one of USE_F32 or USE_F64
#endif

#endif
