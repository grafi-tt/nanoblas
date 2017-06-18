#include "kernel/x86_64/avx_kernel.h"
#include "internal/macro.h"

kernel_t nanoblas_f32_avx_kernel = { &nanoblas_f32_avx_kernel_fun, 8 };
