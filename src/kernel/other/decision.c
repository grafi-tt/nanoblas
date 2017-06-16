#include "kernel.h"
#include "kernel/generic_kernel.h"

kernel_t decide_kernel() {
#ifdef USE_F32
	return generic_kernel_8_f32;
#endif
#ifdef USE_F64
	return generic_kernel_4_f64;
#endif
}
