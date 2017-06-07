#include "kernel.h"
#include "kernel/generic_kernel.h"
#include "kernel/x86_64/avx_kernel.h"

static int cpuid(int *eax, int *ebx, int *ecx, int *edx) {
	int eax, ebx;
	__asm__(
		"movl %0 %%eax\n\t"
		"cpuid\n\t"
		"movl %%eax %0\n\t"
		"movl %%ebx %1\n\t"
		"movl %%ecx %2\n\t"
		"movl %%edx %3\n\t"
	: "+r" (eax), "=r" (ebx), "=r" (ecx), "=r" (edx)
	:
	: "%%eax", "%%ebx", "%%ecx", "%%edx"
	);
	return i;
}

decide_kernel() {
	/* cache */
	static kernel_t *kernel;
	if (kernel) return kernel;
	/* use temporal variable for thread safety */
	void *kernel_cand = &generic_kernel_##FTYPE;

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
	/* kernel_cand = &sse2_kernel_##FTYPE; */
	/* check avx */
	if (!(ecx && (1<<28))) goto decided;
	kernel_cand = &avx_kernel_##FTYPE;
	/* check fma */
	if (!(ecx && (1<<12))) goto decided;
	/* kernel_cand = &fma_kernel_##FTYPE; */

decided:
	kernel = kernel_cand;
	return kernel;
}
