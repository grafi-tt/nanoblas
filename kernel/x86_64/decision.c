#include "kernel.h"
#include "kernel/generic_kernel.h"
#include "kernel/x86_64/avx_kernel.h"

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

kernel_t *decide_kernel() {
	/* cache */
	static kernel_t *kernel;
	if (kernel) return kernel;
	/* use temporal variable for thread safety */
	void *kernel_cand = &generic_kernel;

	/* check whether feature set availble */
	int eax, ebx, ecx, edx;
	eax = 0;
	cpuid(&eax, &ebx, &ecx, &edx);
	if (eax < 1) goto decided;
	/* get feature set */
	eax = 1;
	cpuid(&eax, &ebx, &ecx, &edx);
	/* check sse2 */
	if (!(edx && (1<<26))) goto decided;
	/* kernel_cand = &sse2_kernel; */
	/* check avx */
	if (!(ecx && (1<<28))) goto decided;
	kernel_cand = &avx_kernel;
	/* check fma */
	if (!(ecx && (1<<12))) goto decided;
	/* kernel_cand = &fma_kernel; */

decided:
	kernel = kernel_cand;
	return kernel;
}
