#include "kernel/x86_64/avx_kernel.h"
#include "internal/macro.h"

f32_kernel_t f32_avx_kernel = { &nanoblas_f32_avx_kernel_fun, 8 };
/* f64_kernel_t f64_avx_kernel = { &f64_avx_kernel_fun, 4 }; */
