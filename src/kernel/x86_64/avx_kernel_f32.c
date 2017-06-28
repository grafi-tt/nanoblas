#include "nanoblas_prepack.h"
#include "kernel/x86_64/avx_kernel.h"
#include "internal/macro.h"

void nanoblas_f32_avx_kernel_asm(kernel_state_t *, void *, void *, kernel_state_t *);
void nanoblas_f32_avx_kernel_fun(kernel_state_t *kernel_st) {
	/* to support both of Windows and SysV calling conventions */
	nanoblas_f32_avx_kernel_asm(kernel_st, NULL, NULL, kernel_st);
}

void nanoblas_f32_avx_pack_asm(prepack_state_t *, void *, void *, prepack_state_t *);
void nanoblas_f32_avx_pack_fun(prepack_state_t *prepack_st) {
	int sched_len = prepack_st->sched_len;
	prepack_st->sched_len = 8;
	while (sched_len > 8) {
		nanoblas_f32_avx_pack_asm(prepack_st, NULL, NULL, prepack_st);
		sched_len -= 8;
	}
	prepack_st->sched_len = sched_len;
	nanoblas_f32_avx_pack_asm(prepack_st, NULL, NULL, prepack_st);
}

void nanoblas_f32_avx_max_sched_len_fun(int k_len, int *a_max_sched_len, int *b_max_sched_len) {
	(void)k_len;
	if (a_max_sched_len) *a_max_sched_len = 64;
	if (b_max_sched_len) *b_max_sched_len = 64;
}

kernel_t nanoblas_f32_avx_kernel = {
	.kernel_fun        = &nanoblas_f32_avx_kernel_fun,
	.pack_fun          = &nanoblas_f32_avx_pack_fun,
	.max_sched_len_fun = &nanoblas_f32_avx_max_sched_len_fun,
	.m_slice_len       = 8,
	.n_slice_len       = 8,
};
