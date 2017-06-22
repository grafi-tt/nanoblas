#include "nanoblas.h"
#include "internal/macro.h"
#include "kernel/generic_kernel.h"
#include "kernel/x86_64/avx_kernel.h"

#define cpuid JOIN(NAMESPACE, cpuid)
static void cpuid(int *eax, int *ebx, int *ecx, int *edx) {
	__asm__(
		"movl %0, %%eax\n\t"
		"cpuid\n\t"
		"movl %%eax, %0\n\t"
		"movl %%ebx, %1\n\t"
		"movl %%ecx, %2\n\t"
		"movl %%edx, %3\n\t"
	: "+mr" (*eax), "=rm" (*ebx), "=rm" (*ecx), "=rm" (*edx)
	:
	: "%eax", "%ebx", "%ecx", "%edx"
	);
}

void nanoblas_init(nanoblas_t *nb) {
	nb->f32_kernel = nanoblas_f32_generic_kernel_6x4;
	nb->f64_kernel = nanoblas_f64_generic_kernel_4x4;

	nb->f32_blk_n_max_len = 128;
	nb->f32_blk_k_max_len = 128;
	nb->f64_blk_n_max_len = 64;
	nb->f64_blk_k_max_len = 128;

	/* check whether feature set availble */
	int eax, ebx, ecx, edx;
	eax = 0;
	cpuid(&eax, &ebx, &ecx, &edx);
	if (eax < 1) return;
	/* get feature set */
	eax = 1;
	cpuid(&eax, &ebx, &ecx, &edx);
	/* check sse2 */
	if (!(edx && (1<<26))) return;
	/* check avx */
	if (!(ecx && (1<<28))) return;
	nb->f32_kernel = nanoblas_f32_avx_kernel;
	/* check fma */
	/* if (!(ecx && (1<<12))) return; */
	/* kernel_cand = &fma_kernel; */
}
