// offsets in kernel_staet_t struct
.set offset_a_pack_cur,             0
.set offset_b_pack_cur,             8
.set offset_c_buf,                  16
.set offset_c_cur,                  24
.set offset_c_next_cur,             32
.set offset_ldc,                    40
.set offset_m_slice_real_len,       48
.set offset_n_slice_real_len,       52
.set offset_m_next_slice_real_len,  56
.set offset_n_next_slice_real_len,  60
.set offset_k_len,                  64
.set offset_current_prepack,        68

// offsets in prepack_state_t
.set offset_next_cur,            0
.set offset_next_pack_cur,       8
.set offset_interval_mn,         16
.set offset_interval_k,          24
.set offset_slice_len,           32
.set offset_remained_len,        36
.set offset_next_slice_real_len, 40

.text
.balign 32
// [-1.0, ..., -1.0]
.rept 8
.float -1
.endr
loadmask:
// [0.0, ..., 0.0]
.rept 8
.float 0
.endr

.macro mult_load0
vmaskmovps (%r8), %ymm0, %ymm1
vmovaps %ymm1, -128(%rsi)
.endm
.macro mult_load1
leaq (%r8, %r9), %r14
cmpl $1, %eax
cmovle %r12, %r14
vmaskmovps (%r14), %ymm0, %ymm1
vmovaps %ymm1, -96(%rsi)
.endm
.macro mult_load2
leaq (%r8, %r9, 2), %r14
cmpl $2, %eax
cmovle %r12, %r14
vmaskmovps (%r14), %ymm0, %ymm1
vmovaps %ymm1, -64(%rsi)
.endm
.macro mult_load3
addq %r11, %r8
cmpl $3, %eax
cmovle %r12, %r8
vmaskmovps (%r8), %ymm0, %ymm1
vmovaps %ymm1, -32(%rsi)
.endm
.macro mult_load4
movq %r10, %r14
cmpl $4, %eax
cmovle %r12, %r14
vmaskmovps (%r14), %ymm0, %ymm1
vmovaps %ymm1, (%rsi)
.endm
.macro mult_load5
leaq (%r10, %r9), %r14
cmpl $5, %eax
cmovle %r12, %r14
vmaskmovps (%r14), %ymm0, %ymm1
vmovaps %ymm1, 32(%rsi)
.endm
.macro mult_load6
leaq (%r10, %r9, 2), %r14
cmpl $6, %eax
cmovle %r12, %r14
vmaskmovps (%r14), %ymm0, %ymm1
vmovaps %ymm1, 64(%rsi)
.endm
.macro mult_load7
addq %r11, %r10
cmpl $7, %eax
cmovle %r12, %r10
vmaskmovps (%r10), %ymm0, %ymm1
vmovaps %ymm1, 96(%rsi)
.endm

.macro mult_store_c0
vmaskmovps %ymm8, %ymm0, (%r8)
.endm
.macro mult_store_c1
leaq (%r8, %r9), %r14
movq %r14, %r15
cmpl $1, %eax
cmovle %r12, %r15
vmaskmovps %ymm9, %ymm0, (%r15)
.endm
.macro mult_store_c2
addq %r9, %r14
cmpl $2, %eax
cmovle %r12, %r14
vmaskmovps %ymm10, %ymm0, (%r14)
.endm
.macro mult_store_c3
addq %r11, %r8
cmpl $3, %eax
cmovle %r12, %r8
vmaskmovps %ymm11, %ymm0, (%r8)
.endm
.macro mult_store_c4
movq %r10, %r14
cmpl $4, %eax
cmovle %r12, %r14
vmaskmovps %ymm12, %ymm0, (%r14)
.endm
.macro mult_store_c5
leaq (%r10, %r9), %r14
movq %r14, %r15
cmpl $5, %eax
cmovle %r12, %r15
vmaskmovps %ymm13, %ymm0, (%r15)
.endm
.macro mult_store_c6
addq %r9, %r14
cmpl $6, %eax
cmovle %r12, %r14
vmaskmovps %ymm14, %ymm0, (%r14)
.endm
.macro mult_store_c7
addq %r11, %r10
cmpl $7, %eax
cmovle %r12, %r10
vmaskmovps %ymm15, %ymm0, (%r10)
.endm

.globl _nanoblas_f32_avx_kernel_mult
.globl nanoblas_f32_avx_kernel_mult
.type _nanoblas_f32_avx_kernel_mult, function
.type nanoblas_f32_avx_kernel_mult, function
.size _nanoblas_f32_avx_kernel_mult, kernel_pack - kernel_mult
.size nanoblas_f32_avx_kernel_mult, kernel_pack - kernel_mult
_nanoblas_f32_avx_kernel_mult:
nanoblas_f32_avx_kernel_mult:
// 32bytes aligned
kernel_mult:
	// stack layout
	// | temporary (256bytes) | 0 (32bytes) | old r15, r14, r13, r12 | some padding | old rbp | ret addr |
	//                                         64bytes aligned here -+              +- current rbp
	pushq %rbp
	movq %rsp, %rbp
	andq $-64, %rsp
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
	subq $288, %rsp
	vxorps %ymm0, %ymm0, %ymm0
	movl $128, %ecx
	vmovaps %ymm0, (%rsp, %rcx, 2)

	// rcx: a_cur, rdx: b_cur, rsi: c_buf
	// shifted by 128 for shorter instruction encoding
	movl %ecx, %edx
	movq %rcx, %rsi
	addq offset_a_pack_cur(%rdi), %rcx
	addq offset_b_pack_cur(%rdi), %rdx
	addq offset_c_buf(%rdi), %rsi
	// loop_len_remained: eax
	movl offset_k_len(%rdi), %eax
	// update cur now
	movl %eax, %r15d
	shll $5, %r15d
	addq %r15, offset_a_pack_cur(%rdi)
	addq %r15, offset_b_pack_cur(%rdi)

	// r8: c_next_cur
	// r9: ldc
	movq offset_c_next_cur(%rdi), %r8
	movq offset_ldc(%rdi), %r9
	// r10: c_next_cur + 4*ldc
	// r11: 3*ldc
	leaq (%r9, %r9), %r11
	leaq (%r11, %r11), %r10
	addq %r9, %r11
	addq %r8, %r10

	// defaults to no packing
	xorl %r15d, %r15d
	// if <16, skip prefetch or packing, and inspect where to jump
	cmpq $12, %rax
	jl mult_fallback

	// r12: c_next_cur
	// r13: ldc
	// r14: c_next_cur + 4*ldc
	movq %r8, %r12
	movq %r9, %r13
	movq %r10, %r14
	// packing check
	movl offset_current_prepack(%rdi), %r15d
	testl %r15d, %r15d
	// if no packing required, r15 will be kept 0
	jz mult_prefetch

	// prepare for packing
	addq %rdi, %r15
	// r12: next_cur
	movq offset_next_cur(%r15), %r12
	// r13: interval_m (if a)
	movq offset_interval_mn(%r15), %r13
	cmpq $4, %r13
	// r13: interval_k (if b)
	cmove offset_interval_k(%r15), %r13
	// r14: next_cur + 4*interval_m_or_k
	// r15: 3*interval_m_or_k
	leaq (%r13, %r13), %r15
	leaq (%r15, %r15), %r14
	addq %r13, %r15
	addq %r12, %r14

	subl $2, %eax

// 16bytes aligned
mult_prefetch:
	// ymm6 = (a[8:4], a[8:4]); ymm5 = (a[4:0], a[4:0])
	vmovaps -128(%rcx), %ymm5
	vinsertf128 $0, -128+16(%rcx), %ymm5, %ymm6
	vinsertf128 $1, -128(%rcx), %ymm5, %ymm5
	// ymm7 = b
	vmovaps -128(%rdx), %ymm7
	// ymm3 = (a'[8:4], a'[8:4]); ymm2 = (a'[4:0], a'[4:0])
	vmovaps -128+32(%rcx), %ymm2
	vinsertf128 $0, -128+48(%rcx), %ymm2, %ymm3
	vinsertf128 $1, -128+32(%rcx), %ymm2, %ymm2
	// ymm4 = b'
	vmovaps -128+32(%rdx), %ymm4

	prefetcht1 (%r8)
	prefetcht1 (%r8, %r9)
	prefetcht1 (%r8, %r9, 2)
	prefetcht1 (%r8, %r11)
	prefetcht1 (%r10)
	prefetcht1 (%r10, %r9)
	prefetcht1 (%r10, %r9, 2)
	prefetcht1 (%r10, %r11)

.macro mult_prefetch_macro cnt=0, times
	.if (\cnt % 2 == 0)
		.if (\cnt != 0)
			// ymm6 = (a[8:4], a[8:4]); ymm5 = (a[4:0], a[4:0])
			vmovaps 32*\cnt-128(%rcx), %ymm5
			vinsertf128 $0, 32*\cnt-128+16(%rcx), %ymm5, %ymm6
			vinsertf128 $1, 32*\cnt-128(%rcx), %ymm5, %ymm5
			// ymm7 = b
			vmovaps 32*\cnt-128(%rdx), %ymm7
			// ymm3 = (a'[8:4], a'[8:4]); ymm2 = (a'[4:0], a'[4:0])
			vmovaps 32*\cnt-128+32(%rcx), %ymm2
			vinsertf128 $0, 32*\cnt-128+48(%rcx), %ymm2, %ymm3
			vinsertf128 $1, 32*\cnt-128+32(%rcx), %ymm2, %ymm2
			// ymm4 = b'
			vmovaps 32*\cnt-128+32(%rdx), %ymm4
		.endif
		.set yal, %ymm5
		.set yah, %ymm6
		.set yb, %ymm7
	.else
		.set yal, %ymm2
		.set yah, %ymm3
		.set yb, %ymm4
	.endif

	// ymm1 = [a[0], ..., a[0]]
	vshufps $0x00, yal, yal, %ymm1
	// C[0,] += a[0] * b
	.if (\cnt == 0)
	vmulps %ymm1, yb, %ymm8
	vaddps -128(%rsi), %ymm8, %ymm8
	.else
	vmulps %ymm1, yb, %ymm1
	vaddps %ymm1, %ymm8, %ymm8
	.endif
	// ymm1 = [a[1], ..., a[1]]
	vshufps $0x55, yal, yal, %ymm1
	// C[1,] += a[1] * b
	.if (\cnt == 0)
	vmulps %ymm1, yb, %ymm9
	vaddps -96(%rsi), %ymm9, %ymm9
	.else
	vmulps %ymm1, yb, %ymm1
	vaddps %ymm1, %ymm9, %ymm9
	.endif
	// ymm1 = [a[2], ..., a[2]]
	vshufps $0xAA, yal, yal, %ymm1
	// C[2,] += a[2] * b
	.if (\cnt == 0)
	vmulps %ymm1, yb, %ymm10
	vaddps -64(%rsi), %ymm10, %ymm10
	.else
	vmulps %ymm1, yb, %ymm1
	vaddps %ymm1, %ymm10, %ymm10
	.endif
	// ymm1 = [a[3], ..., a[3]]
	vshufps $0xFF, yal, yal, %ymm1
	// C[3,] += a[3] * b
	.if (\cnt == 0)
	vmulps %ymm1, yb, %ymm11
	vaddps -32(%rsi), %ymm11, %ymm11
	.else
	vmulps %ymm1, yb, %ymm1
	vaddps %ymm1, %ymm11, %ymm11
	.endif
	// ymm1 = [a[4], ..., a[4]]
	vshufps $0x00, yah, yah, %ymm1
	// C[4,] += a[4] * b
	.if (\cnt == 0)
	vmulps %ymm1, yb, %ymm12
	vaddps (%rsi), %ymm12, %ymm12
	.else
	vmulps %ymm1, yb, %ymm1
	vaddps %ymm1, %ymm12, %ymm12
	.endif
	// ymm1 = [a[5], ..., a[5]]
	vshufps $0x55, yah, yah, %ymm1
	// C[5,] += a[5] * b
	.if (\cnt == 0)
	vmulps %ymm1, yb, %ymm13
	vaddps 32(%rsi), %ymm13, %ymm13
	.else
	vmulps %ymm1, yb, %ymm1
	vaddps %ymm1, %ymm13, %ymm13
	.endif
	// ymm1 = [a[6], ..., a[6]]
	vshufps $0xAA, yah, yah, %ymm1
	// C[6,] += a[6] * b
	.if (\cnt == 0)
	vmulps %ymm1, yb, %ymm14
	vaddps 64(%rsi), %ymm14, %ymm14
	.else
	vmulps %ymm1, yb, %ymm1
	vaddps %ymm1, %ymm14, %ymm14
	.endif
	// ymm1 = [a[7], ..., a[7]]
	vshufps $0xFF, yah, yah, %ymm1
	// C[7,] += a[7] * b
	.if (\cnt == 0)
	vmulps %ymm1, yb, %ymm15
	vaddps 96(%rsi), %ymm15, %ymm15
	.else
	vmulps %ymm1, yb, %ymm1
	vaddps %ymm1, %ymm15, %ymm15
	.endif

	.if (\cnt+1)-\times
	mult_prefetch_macro (\cnt+1), \times
	.else
	addq $32*\times, %rcx
	addq $32*\times, %rdx
	.endif
.endm
	mult_prefetch_macro times=8

	prefetcht1 (%r12)
	prefetcht1 (%r12, %r13)
	prefetcht1 (%r12, %r13, 2)
	prefetcht1 (%r12, %r15)
	prefetcht1 (%r14)
	prefetcht1 (%r14, %r13)
	prefetcht1 (%r14, %r13, 2)
	prefetcht1 (%r14, %r15)

	// r12 holds 0 address
	leaq 256(%rsp), %r12
	// check eax
	subl $10, %eax
	jz mult_load_c

// 16bytes aligned
mult_main: // 32 bytes sequence
	// prepare for duff's device
	pushq %rax
	negl %eax
	andl $7, %eax
	// displacement multiplied by -32: rcx, rdx
	shll $5, %eax
	subq %rax, %rcx
	subq %rax, %rdx
	// loop length is 32*4
	leaq mult_duffs_loop(%rip), %r14
	leaq (%r14, %rax, 4), %r14
	popq %rax
	pushq %r14
	// jump to loop
	jmp *(%rsp)

mult_duffs_loop: // 32 bytes sequences
.macro mult_duffs_macro cnt=0, times
	//.balign 16
	// ymm6 = (a[8:4], a[8:4]); ymm5 = (a[4:0], a[4:0])
	.if \cnt == 4
	// vmovaps 0(%rcx), %ymm5
	.byte 0xc5, 0xfc, 0x28, 0x69, 0x00
	.else
	vmovaps 32*\cnt-128(%rcx), %ymm5
	.endif

	vinsertf128 $0, 32*\cnt-128+16(%rcx), %ymm5, %ymm6
	.if \cnt == 4
	// vinsertf128 $1, 0(%rcx), %ymm5, %ymm5
	.byte 0xc4, 0xe3, 0x55, 0x18, 0x69, 0x00, 0x01
	.else
	vinsertf128 $1, 32*\cnt-128(%rcx), %ymm5, %ymm5
	.endif

	// ymm7 = b
	.if \cnt == 4
	// vmovaps 0(%rdx), %ymm7
	.byte 0xc5, 0xfc, 0x28, 0x7a, 0x00
	.else
	vmovaps 32*\cnt-128(%rdx), %ymm7
	.endif

	.if (\cnt+1)==\times
	addq $32*\times, %rcx
	addq $32*\times, %rdx
	.endif

	// ymm1 = [a[0], ..., a[0]]
	vshufps $0x00, %ymm5, %ymm5, %ymm1
	// C[0,] += a[0] * b
	vmulps %ymm1, %ymm7, %ymm1
	vaddps %ymm1, %ymm8, %ymm8
	// ymm1 = [a[1], ..., a[1]]
	vshufps $0x55, %ymm5, %ymm5, %ymm1
	// C[1,] += a[1] * b
	vmulps %ymm1, %ymm7, %ymm1
	vaddps %ymm1, %ymm9, %ymm9
	// ymm1 = [a[2], ..., a[2]]
	vshufps $0xAA, %ymm5, %ymm5, %ymm1
	// C[2,] += a[2] * b
	vmulps %ymm1, %ymm7, %ymm1
	vaddps %ymm1, %ymm10, %ymm10
	// ymm1 = [a[3], ..., a[3]]
	vshufps $0xFF, %ymm5, %ymm5, %ymm1
	// C[3,] += a[3] * b
	vmulps %ymm1, %ymm7, %ymm1
	vaddps %ymm1, %ymm11, %ymm11
	// ymm1 = [a[4], ..., a[4]]
	vshufps $0x00, %ymm6, %ymm6, %ymm1
	// C[4,] += a[4] * b
	vmulps %ymm1, %ymm7, %ymm1
	vaddps %ymm1, %ymm12, %ymm12
	// ymm1 = [a[5], ..., a[5]]
	vshufps $0x55, %ymm6, %ymm6, %ymm1
	// C[5,] += a[5] * b
	vmulps %ymm1, %ymm7, %ymm1
	vaddps %ymm1, %ymm13, %ymm13
	// ymm1 = [a[6], ..., a[6]]
	vshufps $0xAA, %ymm6, %ymm6, %ymm1
	// C[6,] += a[6] * b
	vmulps %ymm1, %ymm7, %ymm1
	vaddps %ymm1, %ymm14, %ymm14
	// ymm1 = [a[7], ..., a[7]]
	vshufps $0xFF, %ymm6, %ymm6, %ymm1
	// C[7,] += a[7] * b
	vmulps %ymm1, %ymm7, %ymm1
	vaddps %ymm1, %ymm15, %ymm15

	.if (\cnt+1)-\times
	mult_duffs_macro (\cnt+1), \times
	.endif
.endm
	mult_duffs_macro times=8
	subl $8, %eax
	jg mult_duffs_loop

	popq %r14

mult_load_c:
	// fix cur
	movl $128, %eax
	subq %rax, %rcx
	subq %rax, %rdx

	// create mask to load next C
	// ymm0 will be hi[0, ..., 0, -1, ..., -1]lo
	//    #(8 - n_slice_real_len) #n_slice_rean_len
	movl offset_n_next_slice_real_len(%rdi), %r13d
	negq %r13
	leaq loadmask(%rip), %r14
	vmovups (%r14, %r13, 4), %ymm0
	// load limit
	movl offset_m_next_slice_real_len(%rdi), %eax

	jmp mult_load

// 16bytes aligned
mult_pack:
	// current_prepack
	movl offset_current_prepack(%rdi), %r11d
	addq %rdi, %r11

	// eax: min(remained_len, 8)
	movl $8, %esi
	movl offset_remained_len(%r11), %eax
	cmpl %esi, %eax
	cmovg %esi, %eax
	// update remained_len now
	subl %esi, offset_remained_len(%r11)

	// r13: next_pack_cur+128
	movl $128, %r13d
	addq offset_next_pack_cur(%r11), %r13
	// update next_pack_cur now
	movl %eax, %esi
	shll $5, %esi
	addq %rsi, offset_next_pack_cur(%r11)

	// r8: next_cur
	movq offset_next_cur(%r11), %r8
	// r9: interval_k (no trans)
	movq offset_interval_k(%r11), %r9
	// update next_cur now
	leaq (%r8, %r9, 8), %r10
	movq %r10, offset_next_cur(%r11)

	// no more packing
	xorl %r15d, %r15d
	// r9: interval_mn (need trans)
	cmpq $4, %r9
	cmove offset_interval_mn(%r11), %r9
	// if trans required, set 1
	sete %r15b
	// eax: min(remained_len, 8), r10d: next_slice_real_len,  rsi: next_pack_cur+128 (no trans)
	// eax: next_slice_real_len,  r10d: min(remained_len, 8), rsi: rsp+128  (trans)
	movl %eax, %r10d
	cmovne offset_next_slice_real_len(%r11), %r10d
	cmove offset_next_slice_real_len(%r11), %eax
	leaq 128(%rsp), %rsi
	cmovne %r13, %rsi

	// create mask to load
	negq %r10
	leaq loadmask(%rip), %r11
	vmovups (%r11, %r10, 4), %ymm0

	// r10: next_cur + 4*interval_mn_or_k
	// r11: 3*interval_mn_or_k
	leaq (%r9, %r9), %r11
	leaq (%r11, %r11), %r10
	addq %r9, %r11
	addq %r8, %r10

// 1 (mod 16) byte aligned
mult_load:
.macro mult_2fetch_macro
	// ymm6 = (a[8:4], a[8:4]); ymm5 = (a[4:0], a[4:0])
	vmovaps (%rcx), %ymm5
	vinsertf128 $0, 16(%rcx), %ymm5, %ymm6
	vinsertf128 $1, (%rcx), %ymm5, %ymm5
	// ymm7 = b
	vmovaps (%rdx), %ymm7
	// ymm3 = (a'[8:4], a'[8:4]); ymm2 = (a'[4:0], a'[4:0])
	vmovaps 32(%rcx), %ymm2
	vinsertf128 $0, 48(%rcx), %ymm2, %ymm3
	vinsertf128 $1, 32(%rcx), %ymm2, %ymm2
	// ymm4 = b'
	vmovaps 32(%rdx), %ymm4
.endm
	mult_2fetch_macro

	mult_load0
	mult_load1
	mult_load2
	mult_load3
	mult_load4
	mult_load5
	mult_load6
	mult_load7

.macro mult_2fma_macro cnt=0
	.if (\cnt == 0)
		.set yal, %ymm5
		.set yah, %ymm6
		.set yb, %ymm7
	.else
		.set yal, %ymm2
		.set yah, %ymm3
		.set yb, %ymm4
	.endif

	// ymm1 = [a[0], ..., a[0]]
	vshufps $0x00, yal, yal, %ymm1
	// C[0,] += a[0] * b
	vmulps %ymm1, yb, %ymm1
	vaddps %ymm1, %ymm8, %ymm8
	// ymm1 = [a[1], ..., a[1]]
	vshufps $0x55, yal, yal, %ymm1
	// C[1,] += a[1] * b
	vmulps %ymm1, yb, %ymm1
	vaddps %ymm1, %ymm9, %ymm9
	// ymm1 = [a[2], ..., a[2]]
	vshufps $0xAA, yal, yal, %ymm1
	// C[2,] += a[2] * b
	vmulps %ymm1, yb, %ymm1
	vaddps %ymm1, %ymm10, %ymm10
	// ymm1 = [a[3], ..., a[3]]
	vshufps $0xFF, yal, yal, %ymm1
	// C[3,] += a[3] * b
	vmulps %ymm1, yb, %ymm1
	vaddps %ymm1, %ymm11, %ymm11
	// ymm1 = [a[4], ..., a[4]]
	vshufps $0x00, yah, yah, %ymm1
	// C[4,] += a[4] * b
	vmulps %ymm1, yb, %ymm1
	vaddps %ymm1, %ymm12, %ymm12
	// ymm1 = [a[5], ..., a[5]]
	vshufps $0x55, yah, yah, %ymm1
	// C[5,] += a[5] * b
	vmulps %ymm1, yb, %ymm1
	vaddps %ymm1, %ymm13, %ymm13
	// ymm1 = [a[6], ..., a[6]]
	vshufps $0xAA, yah, yah, %ymm1
	// C[6,] += a[6] * b
	vmulps %ymm1, yb, %ymm1
	vaddps %ymm1, %ymm14, %ymm14
	// ymm1 = [a[7], ..., a[7]]
	vshufps $0xFF, yah, yah, %ymm1
	// C[7,] += a[7] * b
	vmulps %ymm1, yb, %ymm1
	vaddps %ymm1, %ymm15, %ymm15

	.if \cnt == 0
	mult_2fma_macro 1
	.else
	addq $64, %rcx
	addq $64, %rdx
	.endif
.endm
	mult_2fma_macro

	// packing checking
	cmpq $1, %r15
	jl mult_store_c
	jg mult_pack

mult_trans:
	movq %r13, %rcx

	// transpose algorithm is from
	// IA-32/64 Optimization Manual 11.11.2

	// lo
	vmovaps -128(%rsi), %xmm0
	vinsertf128 $1, (%rsi), %ymm0, %ymm0
	vmovaps -96(%rsi), %xmm1
	vinsertf128 $1, 32(%rsi), %ymm1, %ymm1
	vunpcklpd %ymm1, %ymm0, %ymm4
	vunpckhpd %ymm1, %ymm0, %ymm5

	vmovaps -64(%rsi), %xmm2
	vinsertf128 $1, 64(%rsi), %ymm2, %ymm2
	vmovaps -32(%rsi), %xmm3
	vinsertf128 $1, 96(%rsi), %ymm3, %ymm3
	vunpcklpd %ymm3, %ymm2, %ymm6
	vunpckhpd %ymm3, %ymm2, %ymm7

	vshufps $0x88, %ymm6, %ymm4, %ymm0
	vmovaps %ymm0, -128(%rcx)
	vshufps $0xDD, %ymm6, %ymm4, %ymm1
	vmovaps %ymm1, -96(%rcx)
	vshufps $0x88, %ymm7, %ymm5, %ymm2
	vmovaps %ymm2, -64(%rcx)
	vshufps $0xDD, %ymm7, %ymm5, %ymm3
	vmovaps %ymm3, -32(%rcx)

	// hi
	vmovaps -112(%rsi), %xmm0
	vinsertf128 $1, 16(%rsi), %ymm0, %ymm0
	vmovaps -80(%rsi), %xmm1
	vinsertf128 $1, 48(%rsi), %ymm1, %ymm1
	vunpcklpd %ymm1, %ymm0, %ymm4
	vunpckhpd %ymm1, %ymm0, %ymm5

	vmovaps -48(%rsi), %xmm2
	vinsertf128 $1, 80(%rsi), %ymm2, %ymm2
	vmovaps -16(%rsi), %xmm3
	vinsertf128 $1, 112(%rsi), %ymm3, %ymm3
	vunpcklpd %ymm3, %ymm2, %ymm6
	vunpckhpd %ymm3, %ymm2, %ymm7

	vshufps $0x88, %ymm6, %ymm4, %ymm0
	vmovaps %ymm0, (%rcx)
	vshufps $0xDD, %ymm6, %ymm4, %ymm1
	vmovaps %ymm1, 32(%rcx)
	vshufps $0x88, %ymm7, %ymm5, %ymm2
	vmovaps %ymm2, 64(%rcx)
	vshufps $0xDD, %ymm7, %ymm5, %ymm3
	vmovaps %ymm3, 96(%rcx)

// 16 bytes aligned
mult_store_c:
	// r8: c_next_cur
	// r9: ldc
	movq offset_c_cur(%rdi), %r8
	movq offset_ldc(%rdi), %r9
	// r10: c_next_cur + 4*ldc
	// r11: 3*ldc
	leaq (%r9, %r9), %r11
	leaq (%r11, %r11), %r10
	addq %r9, %r11
	addq %r8, %r10

	// update c_cur, c_next_cur now
	movq offset_c_next_cur(%rdi), %r14
	movq %r14, offset_c_cur(%rdi)
	addq $32, %r14
	movq %r14, offset_c_next_cur(%rdi)

	// create mask to store C
	// ymm0 will be hi[0, ..., 0, -1, ..., -1]lo
	//    #(8 - n_slice_real_len) #n_slice_rean_len
	movl offset_n_slice_real_len(%rdi), %eax
	negq %rax
	leaq loadmask(%rip), %r14
	vmovups (%r14, %rax, 4), %ymm0
	// load limit
	movl offset_m_slice_real_len(%rdi), %eax

	// store
	mult_store_c0
	mult_store_c1
	mult_store_c2
	mult_store_c3
	mult_store_c4
	mult_store_c5
	mult_store_c6
	mult_store_c7

	// return
	addq $288, %rsp
	popq %r15
	popq %r14
	popq %r13
	popq %r12
	movq %rbp, %rsp
	popq %rbp
	ret

.balign 16
mult_fallback:
	// load c
	vmovaps -128(%rsi), %ymm8
	vmovaps  -96(%rsi), %ymm9
	vmovaps  -64(%rsi), %ymm10
	vmovaps  -32(%rsi), %ymm11
	vmovaps     (%rsi), %ymm12
	vmovaps   32(%rsi), %ymm13
	vmovaps   64(%rsi), %ymm14
	vmovaps   96(%rsi), %ymm15
	// r12 holds 0 address
	leaq 256(%rsp), %r12

	subl $8, %eax
	jg mult_main
	je mult_load_c

	// %eax is loop_len-8, [-7,-1]
	negl %eax
	// loop length is 32*4
	// first iteration is truncated
	shll $5, %eax
	subl $32, %eax
	leaq mult_duffs_through(%rip), %r14
	shll $2, %eax
	addq %rax, %r14
	// jump to loop
	pushq %r14
.byte 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00
	jmp *(%rsp)
	// 16bytes aligned here

mult_duffs_through: // 32bytes sequences
	mult_duffs_macro cnt=1, times=8
	popq %r14

	// create mask to load next C
	// ymm0 will be hi[0, ..., 0, -1, ..., -1]lo
	//    #(8 - n_slice_real_len) #n_slice_rean_len
	movl offset_n_next_slice_real_len(%rdi), %eax
	negq %rax
	leaq loadmask(%rip), %r14
	vmovups (%r14, %rax, 4), %ymm0
	// load limit
	movl offset_m_next_slice_real_len(%rdi), %eax

	mult_load0
	mult_load1
	mult_load2
	mult_load3
	mult_load4
	mult_load5
	mult_load6
	mult_load7

	jmp mult_store_c

.balign 16
.globl _nanoblas_f32_avx_kernel_pack
.globl nanoblas_f32_avx_kernel_pack
.type _nanoblas_f32_avx_kernel_pack, function
.type nanoblas_f32_avx_kernel_pack, function
.size _nanoblas_f32_avx_kernel_pack, kernel_end - kernel_pack
.size nanoblas_f32_avx_kernel_pack, kernel_end - kernel_pack
_nanoblas_f32_avx_kernel_pack:
nanoblas_f32_avx_kernel_pack:
kernel_pack:
	movq offset_next_cur(%rdi), %r8
	movq offset_next_pack_cur(%rdi), %r9
	movq offset_interval_k(%rdi), %rcx
	movl offset_remained_len(%rdi), %eax
	movl $0, offset_remained_len(%rdi)

	// check trans
	cmpl $4, %ecx
	je pack_trans

	// ymm0 will the load mask
	// see comment in nanoblas_f32_avx_kernel_asm for detail, at the place creating mask to load C
	movl offset_next_slice_real_len(%rdi), %esi
	negq %rsi
	leaq loadmask(%rip), %rdx
	vmovups (%rdx, %rsi, 4), %ymm0

	// loop (LSD)
pack_no_trans_loop:
	vmaskmovps (%r8), %ymm0, %ymm1
	vmovaps %ymm1, (%r9)
	addq %rcx, %r8
	addq $32, %r9
	subl $1, %eax
	jnz pack_no_trans_loop

	movq %r9, offset_next_pack_cur(%rdi)

	ret

.balign 16
pack_trans:
	// space to put garbage
	subq $80, %rsp

	// load interval
	movq offset_interval_mn(%rdi), %rcx
	// load limit
	movl offset_next_slice_real_len(%rdi), %edx

	// scale remained_len by 4
	addl %eax, %eax
	addl %eax, %eax
	// update next_pack_cur first
	leaq (%r9, %rax, 8), %r10
	movq %r10, offset_next_pack_cur(%rdi)
	// rdi is no longer required

.balign 16
pack_trans_loop:
	// ymm0 will be the load mask
	// see comment in nanoblas_f32_avx_kernel_asm for detail, at the place creating mask to load C
	movl $32, %esi
	cmpl %esi, %eax
	cmovl %eax, %esi
	negq %rsi
	leaq loadmask(%rip), %rdi
	vmovups (%rdi, %rsi), %ymm0

	// ymm8 = pack[0,]
	vmaskmovps (%r8), %ymm0, %ymm8
	prefetcht1 64(%r8)
	// ymm9 = pack[1,]
	leaq (%r8, %rcx), %r10
	movq %r10, %rsi
	cmpl $1, %edx
	cmovle %rdi, %rsi
	vmaskmovps (%rsi), %ymm0, %ymm9
	prefetcht1 64(%rsi)
	// ymm10 = pack[2,]
	addq %rcx, %r10
	movq %r10, %rsi
	cmpl $2, %edx
	cmovle %rdi, %rsi
	prefetcht1 64(%rsi)
	vmaskmovps (%rsi), %ymm0, %ymm10
	// ymm11 = pack[3,]
	addq %rcx, %r10
	movq %r10, %rsi
	cmpl $3, %edx
	cmovle %rdi, %rsi
	prefetcht1 64(%rsi)
	vmaskmovps (%rsi), %ymm0, %ymm11
	// ymm12 = pack[4,]
	addq %rcx, %r10
	movq %r10, %rsi
	cmpl $4, %edx
	cmovle %rdi, %rsi
	prefetcht1 64(%rsi)
	vmaskmovps (%rsi), %ymm0, %ymm12
	// ymm13 = pack[5,]
	addq %rcx, %r10
	movq %r10, %rsi
	cmpl $5, %edx
	cmovle %rdi, %rsi
	prefetcht1 64(%rsi)
	vmaskmovps (%rsi), %ymm0, %ymm13
	// ymm14 = pack[6,]
	addq %rcx, %r10
	movq %r10, %rsi
	cmpl $6, %edx
	cmovle %rdi, %rsi
	prefetcht1 64(%rsi)
	vmaskmovps (%rsi), %ymm0, %ymm14
	// ymm15 = pack[7,]
	addq %rcx, %r10
	cmpl $7, %edx
	cmovle %rdi, %r10
	vmaskmovps (%r10), %ymm0, %ymm15
	prefetcht1 64(%r10)

	// update next_cur
	addq $32, %r8
	// incr next_pack_cur
	leaq 256(%r9), %r10

	// transpose algorithm is from
	// IA-32/64 Optimization Manual 11.11.1

	// unpack lo
	vunpcklps %ymm9, %ymm8, %ymm0
	vunpcklps %ymm11, %ymm10, %ymm1
	vunpcklps %ymm13, %ymm12, %ymm2
	vunpcklps %ymm15, %ymm14, %ymm3
	// unpack hi
	vunpckhps %ymm9, %ymm8, %ymm4
	vunpckhps %ymm11, %ymm10, %ymm5
	vunpckhps %ymm13, %ymm12, %ymm6
	vunpckhps %ymm15, %ymm14, %ymm7

	// shuf lo
	vshufps $0x4E, %ymm1, %ymm0, %ymm8
	vshufps $0x4E, %ymm3, %ymm2, %ymm9
	// first half hi4 lo0
	vblendps $0x33, %ymm0, %ymm8, %ymm10
	// second half hi4 lo0
	vblendps $0x33, %ymm2, %ymm9, %ymm11
	// 0
	vperm2f128 $0x20, %ymm11, %ymm10, %ymm12
	vmovaps %ymm12, (%r9)
	// 4
	vperm2f128 $0x31, %ymm11, %ymm10, %ymm13
	vmovaps %ymm13, -128(%r10)
	// first half hi5 lo1
	vblendps $0xCC, %ymm1, %ymm8, %ymm10
	// second half hi5 lo1
	vblendps $0xCC, %ymm3, %ymm9, %ymm11
	// 1
	vperm2f128 $0x20, %ymm11, %ymm10, %ymm14
	vmovaps %ymm14, 32(%r9)
	// 5
	vperm2f128 $0x31, %ymm11, %ymm10, %ymm15
	vmovaps %ymm15, -96(%r10)

	// shuf hi
	vshufps $0x4E, %ymm5, %ymm4, %ymm8
	vshufps $0x4E, %ymm7, %ymm6, %ymm9
	// first half hi6 lo2
	vblendps $0x33, %ymm4, %ymm8, %ymm10
	// second half hi6 lo2
	vblendps $0x33, %ymm6, %ymm9, %ymm11
	// 2
	vperm2f128 $0x20, %ymm11, %ymm10, %ymm12
	vmovaps %ymm12, 64(%r9)
	// 6
	vperm2f128 $0x31, %ymm11, %ymm10, %ymm13
	vmovaps %ymm13, -64(%r10)
	// first half hi7 lo3
	vblendps $0xCC, %ymm5, %ymm8, %ymm10
	// second half hi7 lo3
	vblendps $0xCC, %ymm7, %ymm9, %ymm11
	// 3
	vperm2f128 $0x20, %ymm11, %ymm10, %ymm14
	vmovaps %ymm14, 96(%r9)
	// 7
	vperm2f128 $0x31, %ymm11, %ymm10, %ymm15
	vmovaps %ymm15, -32(%r10)

	// loop
	subl $32, %eax
	movq %r10, %r9
	jg pack_trans_loop

	addq $80, %rsp
	ret

.balign 16
kernel_end:
