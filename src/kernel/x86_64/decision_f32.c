#include "kernel.h"

#include "kernel/generic_kernel.h"
#include "kernel/x86_64/avx_kernel.h"
#include "kernel/x86_64/cpuid.h"

kernel_t_f32 decide_kernel() {
	kernel_t_f32 kernel_cand = generic_kernel_8_f32;

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
	kernel_cand = avx_kernel_f32;
	/* check fma */
	if (!(ecx && (1<<12))) goto decided;
	/* kernel_cand = &fma_kernel; */

decided:
	return kernel_cand;
}
