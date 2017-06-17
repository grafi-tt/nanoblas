#include "kernel/x86_64/avx_kernel.h"

kernel_t_f32 avx_kernel_f32 = { &avx_kernel_fun_f32, 8 };
/* kernel_t_f64 avx_kernel_f64 = { &avx_kernel_fun_f64, 4 }; */
