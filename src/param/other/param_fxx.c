#include "nanoblas.h"
#include "kernel/generic_kernel.h"

void nanoblas_init(nanoblas_t *nb) {
	nb->f32_kernel = f32_generic_kernel_6x4;
	nb->f64_kernel = f64_generic_kernel_4x4;

	nb->f32_blk_n_max_len = 128;
	nb->f32_blk_k_max_len = 128;
	nb->f64_blk_n_max_len = 64;
	nb->f64_blk_k_max_len = 128;
}
