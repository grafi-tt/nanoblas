#include "kernel.h"

#ifdef USE_F32
#define UNIT_LEN 8
#endif
#ifdef USE_F64
#define UNIT_LEN 4
#endif

#include "kernel/generic_kernel.h"
#include "kernel/x86_64/avx_kernel.h"
#include "kernel/x86_64/cpuid.h"

kernel_t decide_kernel() {
	/* cache */
	static kernel_t kernel;
	if (kernel.fun) return kernel;
	/* use temporal variable for thread safety */
	kernel_t kernel_cand = generic_kernel;

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
	kernel_cand = avx_kernel;
	/* check fma */
	if (!(ecx && (1<<12))) goto decided;
	/* kernel_cand = &fma_kernel; */

decided:
	kernel = kernel_cand;
	return kernel;
}
