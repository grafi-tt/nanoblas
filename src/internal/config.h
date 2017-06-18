#ifndef NANOBLAS_INTERNAL_CONFIG_H
#define NANOBLAS_INTERNAL_CONFIG_H

#include "nanoblas_types.h"
#include "internal/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

#define f32_decide_kernel JOIN(NAMESPACE, f32_decide_kernel)
f32_kernel_t f32_decide_kernel();

#define f64_decide_kernel JOIN(NAMESPACE, f64_decide_kernel)
f64_kernel_t f64_decide_kernel();

#ifdef __cplusplus
}
#endif

#endif
