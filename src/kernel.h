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
} sched_state_t_f32;

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
} sched_state_t_f64;

typedef void (kernel_fun_t_f32)(
		const float *restrict a_pack, const float *restrict b_pack,
		int k_len, int m_sub_len, int n_sub_len,
		float* restrict c, ptrdiff_t ldc, const sched_state_t_f32 *st);

typedef void (kernel_fun_t_f64)(
		const double *restrict a_pack, const double *restrict b_pack,
		int k_len, int m_sub_len, int n_sub_len,
		double* restrict c, ptrdiff_t ldc, const sched_state_t_f64 *st);

typedef struct {
	kernel_fun_t_f32 *fun;
	int unit_len;
} kernel_t_f32;

typedef struct {
	kernel_fun_t_f64 *fun;
	int unit_len;
} kernel_t_f64;

kernel_t_f32 decide_kernel_f32();
kernel_t_f64 decide_kernel_f64();

#endif
