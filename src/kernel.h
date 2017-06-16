#ifndef NANOBLAS_KERNEL_H
#define NANOBLAS_KERNEL_H

#include <stddef.h>
#include "sched.h"
#include "const.h"

#define kernel_fun_t APPEND_FTYPE(kernel_fun_t)
typedef void (kernel_fun_t)(
		const FTYPE *restrict a_pack, const FTYPE *restrict b_pack,
		int k_len, int m_sub_len, int n_sub_len,
		FTYPE* restrict c, ptrdiff_t ldc, const sched_state_t *st);

#define kernel_t APPEND_FTYPE(kernel_t)
typedef struct {
	kernel_fun_t *fun;
	int unit_len;
} kernel_t;

#define decide_kernel APPEND_FTYPE(decide_kernel)
kernel_t decide_kernel();

#endif
