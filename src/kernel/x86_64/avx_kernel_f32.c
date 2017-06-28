#include "nanoblas_prepack.h"
#include "kernel/x86_64/avx_kernel.h"
#include "internal/macro.h"

void nanoblas_f32_avx_kernel_mult_asm(kernel_state_t *, void *, void *, kernel_state_t *);
void nanoblas_f32_avx_kernel_mult(kernel_state_t *kernel_st) {
	/* to support both of Windows and SysV calling conventions */
	nanoblas_f32_avx_kernel_mult_asm(kernel_st, NULL, NULL, kernel_st);
}

void nanoblas_f32_avx_kernel_pack_asm(prepack_state_t *, void *, void *, prepack_state_t *);
void nanoblas_f32_avx_kernel_pack(prepack_state_t *prepack_st) {
	do {
		nanoblas_f32_avx_kernel_pack_asm(prepack_st, NULL, NULL, prepack_st);
	} while (prepack_st->remained_len > 0);
}
