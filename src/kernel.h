#ifndef NANOBLAS_KERNEL_H
#define NANOBLAS_KERNEL_H

#include <stddef.h>

typedef struct {
	float *next_cur;
	float *next_pack_cur;
	int k_next_len;
	int mn_next_len;
	const ptrdiff_t interval_k;
	const ptrdiff_t interval_mn;
	int k_sched_len;
	int mn_sched_len;
	int k_packed_len;
	int mn_packed_len;
	const int unit_len;
} f32_sched_state_t;

typedef struct {
	double *next_cur;
	double *next_pack_cur;
	int k_next_len;
	int mn_next_len;
	const ptrdiff_t interval_k;
	const ptrdiff_t interval_mn;
	int k_sched_len;
	int mn_sched_len;
	int k_packed_len;
	int mn_packed_len;
	const int unit_len;
} f64_sched_state_t;

typedef void (f32_kernel_fun_t)(
		const float *restrict a_pack, const float *restrict b_pack,
		int k_len, int m_sub_len, int n_sub_len,
		float* restrict c, ptrdiff_t ldc, const f32_sched_state_t *st);

typedef void (f64_kernel_fun_t)(
		const double *restrict a_pack, const double *restrict b_pack,
		int k_len, int m_sub_len, int n_sub_len,
		double* restrict c, ptrdiff_t ldc, const f64_sched_state_t *st);

typedef struct {
	f32_kernel_fun_t *fun;
	int unit_len;
} f32_kernel_t;

typedef struct {
	f64_kernel_fun_t *fun;
	int unit_len;
} f64_kernel_t;

f32_kernel_t f32_decide_kernel();
f64_kernel_t f64_decide_kernel();

#endif
