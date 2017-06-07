#include "kernel.h"
#include "kernel/generic_kernel.h"

kernel_t *decide_kernel_##FTYPE() {
	return &generic_kernel_##FTYPE;
}
