#include "nanoblas.h"
#include "internal/config.h"

void nanoblas_init(nanoblas_t *nb) {
	nb->f32_kernel = f32_decide_kernel();
	nb->f64_kernel = f64_decide_kernel();
	nb->f32_blk_m_len = 80;
	nb->f32_blk_n_len = 80;
	nb->f32_blk_k_len = 64;
	nb->f64_blk_m_len = 40;
	nb->f64_blk_n_len = 40;
	nb->f64_blk_k_len = 64;
}
