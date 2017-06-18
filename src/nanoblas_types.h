#ifndef NANOBLAS_NANOBLAS_TYPES_H
#define NANOBLAS_NANOBLAS_TYPES_H

#include <stddef.h>

struct f32_prepack_state_t;
typedef struct f32_prepack_state_t f32_prepack_state_t;
struct f64_prepack_state_t;
typedef struct f64_prepack_state_t f64_prepack_state_t;

struct f32_kernel_state_t;
typedef struct f32_kernel_state_t f32_kernel_state_t;
struct f64_kernel_state_t;
typedef struct f64_kernel_state_t f64_kernel_state_t;

typedef void f32_kernel_fun_t(f32_kernel_state_t *, f32_prepack_state_t *);
typedef void f64_kernel_fun_t(f64_kernel_state_t *, f64_prepack_state_t *);

typedef struct f32_kernel_t {
	f32_kernel_fun_t *fun;
	int unit_len;
} f32_kernel_t;
typedef struct f64_kernel_t {
	f64_kernel_fun_t *fun;
	int unit_len;
} f64_kernel_t;

#endif
