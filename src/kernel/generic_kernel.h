#ifndef NANOBLAS_KERNEL_GENERIC_KERNEL_H
#define NANOBLAS_KERNEL_GENERIC_KERNEL_H

#include "kernel.h"

#define APPEND_UNIT_LEN(name) APPEND_UNIT_LEN_HELPER1(name, UNIT_LEN)
#define APPEND_UNIT_LEN_HELPER1(name, len) APPEND_UNIT_LEN_HELPER2(name, len)
#define APPEND_UNIT_LEN_HELPER2(name, len) name##_##len

#define generic_kernel_fun APPEND_FTYPE(APPEND_UNIT_LEN(generic_kernel_fun))
void generic_kernel_fun(
		const FTYPE *restrict a_pack, const FTYPE *restrict b_pack,
		int k_len, int m_sub_len, int n_sub_len,
		FTYPE* restrict c, ptrdiff_t ldc, const sched_state_t *st);

#define generic_kernel APPEND_FTYPE(APPEND_UNIT_LEN(generic_kernel))
kernel_t generic_kernel;

#endif
