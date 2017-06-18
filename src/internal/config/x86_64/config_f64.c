#include "internal/config.h"
#include "kernel/x86_64/cpuid.h"
#include "kernel/generic_kernel.h"
#include "kernel/x86_64/avx_kernel.h"

f64_kernel_t f64_decide_kernel() {
	f64_kernel_t kernel_cand = f64_generic_kernel_4;

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
	/* kernel_cand = avx_kernel_f64; */
	/* check fma */
	if (!(ecx && (1<<12))) goto decided;
	/* kernel_cand = &fma_kernel; */

decided:
	return kernel_cand;
}
