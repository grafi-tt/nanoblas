#include "kernel/x86_64/cpuid.h"

void cpuid(int *eax, int *ebx, int *ecx, int *edx) {
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
