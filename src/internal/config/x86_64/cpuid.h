#include "internal/macro.h"

#define cpuid JOIN(NAMESPACE, cpuid)
void cpuid(int *eax, int *ebx, int *ecx, int *edx);
