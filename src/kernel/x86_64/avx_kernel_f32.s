// offsets in kernel_staet_t struct
.set offset_a_pack_cur,        0
.set offset_b_pack_cur,        8
.set offset_c_cur,             16
.set offset_ldc,               24
.set offset_m_slice_real_len,  32
.set offset_n_slice_real_len,  36
.set offset_k_len,             40
.set offset_current_prepack,   44

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

.globl _nanoblas_f32_avx_kernel_mult
.globl nanoblas_f32_avx_kernel_mult
.type _nanoblas_f32_avx_kernel_mult, function
.type nanoblas_f32_avx_kernel_mult, function
.size _nanoblas_f32_avx_kernel_mult, kernel_pack - kernel_mult
.size nanoblas_f32_avx_kernel_mult, kernel_pack - kernel_mult
_nanoblas_f32_avx_kernel_mult:
nanoblas_f32_avx_kernel_mult:
kernel_mult:
	// stack layout
	// | 0 (32bytes) | temporary (32bytes) | old r15, r14, r13, r12 | some padding | old rbp | ret addr |
	//                                        64bytes aligned here -+              +- current rbp
	pushq %rbp
	movq %rsp, %rbp
	andq $-64, %rsp
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
	subq $64, %rsp
	vxorps %ymm0, %ymm0, %ymm0
	vmovaps %ymm0, (%rsp)

	// rcx: a_cur, rdx: b_cur
	// shifted by 128 for shorter instruction encoding
	movl $128, %ecx
	movl %ecx, %edx
	addq offset_a_pack_cur(%rdi), %rcx
	addq offset_b_pack_cur(%rdi), %rdx
	// loop_len_remained: eax
	movl offset_k_len(%rdi), %eax

	// r8: c_cur
	// r9: ldc
	movq offset_c_cur(%rdi), %r8
	movq offset_ldc(%rdi), %r9
	// update c_cur now
	leaq 32(%r8), %rsi
	movq %rsi, offset_c_cur(%rdi)

	// r10: c_cur + 4*ldc
	// r11: 3*ldc
	leaq (%r9, %r9), %r11
	leaq (%r11, %r11), %r10
	addq %r9, %r11
	addq %r8, %r10

mult_prologue:
	// if <8, skip prefetch
	cmpl $8, %eax
	jl mult_clear_ymm
	subl $8, %eax

.macro mult_prefetch_macro cnt=0, times
	// ymm5 = (a[8:4], a[8:4]); ymm4 = (a[4:0], a[4:0])
	vmovaps 32*\cnt-128(%rcx), %ymm4
	vinsertf128 $0, 32*\cnt-128+16(%rcx), %ymm4, %ymm5
	vinsertf128 $1, 32*\cnt-128(%rcx), %ymm4, %ymm4
	// ymm7 = b
	vmovaps 32*\cnt-128(%rdx), %ymm7

	// ymm6 = [a[0], ..., a[0]]
	vshufps $0x00, %ymm4, %ymm4, %ymm6
	// C[0,] += a[0] * b
	.if (\cnt == 0)
	vmulps %ymm6, %ymm7, %ymm8
	.else
	vmulps %ymm6, %ymm7, %ymm6
	vaddps %ymm8, %ymm6, %ymm8
	.endif
	// ymm6 = [a[1], ..., a[1]]
	vshufps $0x55, %ymm4, %ymm4, %ymm6
	// C[1,] += a[1] * b
	.if (\cnt == 0)
	vmulps %ymm6, %ymm7, %ymm9
	.else
	vmulps %ymm6, %ymm7, %ymm6
	vaddps %ymm9, %ymm6, %ymm9
	.endif
	// ymm6 = [a[2], ..., a[2]]
	vshufps $0xAA, %ymm4, %ymm4, %ymm6
	// C[2,] += a[2] * b
	.if (\cnt == 0)
	vmulps %ymm6, %ymm7, %ymm10
	.else
	vmulps %ymm6, %ymm7, %ymm6
	vaddps %ymm10, %ymm6, %ymm10
	.endif
	// ymm6 = [a[3], ..., a[3]]
	vshufps $0xFF, %ymm4, %ymm4, %ymm6
	// C[3,] += a[3] * b
	.if (\cnt == 0)
	vmulps %ymm6, %ymm7, %ymm11
	.else
	vmulps %ymm6, %ymm7, %ymm6
	vaddps %ymm11, %ymm6, %ymm11
	.endif
	// prefetch
	.if (\cnt == 0)
	prefetcht1 (%r8)
	.elseif (\cnt == 1)
	prefetcht1 (%r8, %r9)
	.elseif (\cnt == 2)
	prefetcht1 (%r8, %r9, 2)
	.elseif (\cnt == 3)
	prefetcht1 (%r8, %r11)
	.elseif (\cnt == 4)
	prefetcht1 (%r10)
	.elseif (\cnt == 5)
	prefetcht1 (%r10, %r9)
	.elseif (\cnt == 6)
	prefetcht1 (%r10, %r9, 2)
	.elseif (\cnt == 7)
	prefetcht1 (%r10, %r11)
	.endif
	// ymm6 = [a[4], ..., a[4]]
	vshufps $0x00, %ymm5, %ymm5, %ymm6
	// C[4,] += a[4] * b
	.if (\cnt == 0)
	vmulps %ymm6, %ymm7, %ymm12
	.else
	vmulps %ymm6, %ymm7, %ymm6
	vaddps %ymm12, %ymm6, %ymm12
	.endif
	// ymm6 = [a[5], ..., a[5]]
	vshufps $0x55, %ymm5, %ymm5, %ymm6
	// C[5,] += a[5] * b
	.if (\cnt == 0)
	vmulps %ymm6, %ymm7, %ymm13
	.else
	vmulps %ymm6, %ymm7, %ymm6
	vaddps %ymm13, %ymm6, %ymm13
	.endif
	// ymm6 = [a[6], ..., a[6]]
	vshufps $0xAA, %ymm5, %ymm5, %ymm6
	// C[6,] += a[6] * b
	.if (\cnt == 0)
	vmulps %ymm6, %ymm7, %ymm14
	.else
	vmulps %ymm6, %ymm7, %ymm6
	vaddps %ymm14, %ymm6, %ymm14
	.endif
	// ymm6 = [a[7], ..., a[7]]
	vshufps $0xFF, %ymm5, %ymm5, %ymm6
	// C[7,] += a[7] * b
	.if (\cnt == 0)
	vmulps %ymm6, %ymm7, %ymm15
	.else
	vmulps %ymm6, %ymm7, %ymm6
	vaddps %ymm15, %ymm6, %ymm15
	.endif

	.if (\cnt+1)-\times
	mult_prefetch_macro (\cnt+1), \times
	.else
	addq $32*\times, %rcx
	addq $32*\times, %rdx
	.endif
.endm
	mult_prefetch_macro times=8

.balign 16
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
	leaq mult_duffs_loop(%rip), %rsi
	leaq (%rsi, %rax, 4), %rsi
	popq %rax
	pushq %rsi
	// jump to loop
	nop
	jmp *(%rsp)

mult_duffs_loop:
.macro mult_duffs_macro cnt=0, times
	.balign 16
	// ymm5 = (a[8:4], a[8:4]); ymm4 = (a[4:0], a[4:0])
	vmovaps 32*\cnt-128(%rcx), %ymm4
	vinsertf128 $0, 32*\cnt-128+16(%rcx), %ymm4, %ymm5
	vinsertf128 $1, 32*\cnt-128(%rcx), %ymm4, %ymm4
	// ymm7 = b
	vmovaps 32*\cnt-128(%rdx), %ymm7

	// ymm6 = [a[0], ..., a[0]]
	vshufps $0x00, %ymm4, %ymm4, %ymm6
	// C[0,] += a[0] * b
	vmulps %ymm6, %ymm7, %ymm6
	vaddps %ymm6, %ymm8, %ymm8
	// ymm6 = [a[1], ..., a[1]]
	vshufps $0x55, %ymm4, %ymm4, %ymm6
	// C[1,] += a[1] * b
	vmulps %ymm6, %ymm7, %ymm6
	vaddps %ymm6, %ymm9, %ymm9
	// ymm6 = [a[2], ..., a[2]]
	vshufps $0xAA, %ymm4, %ymm4, %ymm6
	// C[2,] += a[2] * b
	vmulps %ymm6, %ymm7, %ymm6
	vaddps %ymm6, %ymm10, %ymm10
	// ymm6 = [a[3], ..., a[3]]
	vshufps $0xFF, %ymm4, %ymm4, %ymm6
	// C[3,] += a[3] * b
	vmulps %ymm6, %ymm7, %ymm6
	vaddps %ymm6, %ymm11, %ymm11
	// ymm6 = [a[4], ..., a[4]]
	vshufps $0x00, %ymm5, %ymm5, %ymm6
	// C[4,] += a[4] * b
	vmulps %ymm6, %ymm7, %ymm6
	vaddps %ymm6, %ymm12, %ymm12
	// ymm6 = [a[5], ..., a[5]]
	vshufps $0x55, %ymm5, %ymm5, %ymm6
	// C[5,] += a[5] * b
	vmulps %ymm6, %ymm7, %ymm6
	vaddps %ymm6, %ymm13, %ymm13
	// ymm6 = [a[6], ..., a[6]]
	vshufps $0xAA, %ymm5, %ymm5, %ymm6
	// C[6,] += a[6] * b
	vmulps %ymm6, %ymm7, %ymm6
	vaddps %ymm6, %ymm14, %ymm14
	// ymm6 = [a[7], ..., a[7]]
	vshufps $0xFF, %ymm5, %ymm5, %ymm6
	// C[7,] += a[7] * b
	vmulps %ymm6, %ymm7, %ymm6
	vaddps %ymm6, %ymm15, %ymm15

	.if (\cnt+1)-\times
	mult_duffs_macro (\cnt+1), \times
	.else
	addq $32*\times, %rcx
	addq $32*\times, %rdx
	.endif
.endm
	mult_duffs_macro times=8
	subl $8, %eax
	jg mult_duffs_loop
	popq %rsi

	// update cursor
	addq $-128, %rcx
	addq $-128, %rdx
	movq %rcx, offset_a_pack_cur(%rdi)
	movq %rdx, offset_b_pack_cur(%rdi)

mult_epilogue:
	// create mask to load C
	// ymm0 will be hi[0, ..., 0, -1, ..., -1]lo
	//    #(8 - n_slice_real_len) #n_slice_rean_len
	movl offset_n_slice_real_len(%rdi), %eax
	negq %rax
	leaq loadmask(%rip), %rsi
	vmovups (%rsi, %rax, 4), %ymm0

	// load limit
	movl offset_m_slice_real_len(%rdi), %eax

	// load C
	// [ymm8^T, ..., ymm15^T]^T = C (8x8)
	// C[0,]
	vmaskmovps (%r8), %ymm0, %ymm1
	vaddps %ymm8, %ymm1, %ymm8
	vmaskmovps %ymm8, %ymm0, (%r8)
	// ymm9 = C[1,]
	movq %r10, %r12
	leaq (%r12, %r9), %r13
	leaq (%r9, %r9), %r10
	addq %r8, %r9
	cmpl $1, %eax
	cmovle %rsp, %r9
	vmaskmovps (%r9), %ymm0, %ymm1
	vaddps %ymm9, %ymm1, %ymm9
	vmaskmovps %ymm9, %ymm0, (%r9)
	// ymm10 = c[2,]
	leaq (%r12, %r10), %r14
	addq %r8, %r10
	cmpl $2, %eax
	cmovle %rsp, %r10
	vmaskmovps (%r10), %ymm0, %ymm1
	vaddps %ymm10, %ymm1, %ymm10
	vmaskmovps %ymm10, %ymm0, (%r10)
	// ymm11 = c[3,]
	leaq (%r12, %r11), %r15
	addq %r8, %r11
	cmpl $3, %eax
	cmovle %rsp, %r11
	vmaskmovps (%r11), %ymm0, %ymm1
	vaddps %ymm11, %ymm1, %ymm11
	vmaskmovps %ymm11, %ymm0, (%r11)
	// ymm12 = c[4,]
	cmpl $4, %eax
	cmovle %rsp, %r12
	vmaskmovps (%r12), %ymm0, %ymm1
	vaddps %ymm12, %ymm1, %ymm12
	vmaskmovps %ymm12, %ymm0, (%r12)
	// ymm13 = c[5,]
	cmpl $5, %eax
	cmovle %rsp, %r13
	vmaskmovps (%r13), %ymm0, %ymm1
	vaddps %ymm13, %ymm1, %ymm13
	vmaskmovps %ymm13, %ymm0, (%r13)
	// ymm14 = c[6,]
	cmpl $6, %eax
	cmovle %rsp, %r14
	vmaskmovps (%r14), %ymm0, %ymm1
	vaddps %ymm14, %ymm1, %ymm14
	vmaskmovps %ymm14, %ymm0, (%r14)
	// ymm15 = c[7,]
	cmpl $7, %eax
	cmovle %rsp, %r15
	vmaskmovps (%r15), %ymm0, %ymm1
	vaddps %ymm15, %ymm1, %ymm15
	vmaskmovps %ymm15, %ymm0, (%r15)

	// return
	addq $64, %rsp
	popq %r15
	popq %r14
	popq %r13
	popq %r12
	movq %rbp, %rsp
	popq %rbp
	ret

.balign 16
mult_clear_ymm:
	vxorps %ymm8, %ymm8, %ymm8
	vxorps %ymm9, %ymm9, %ymm9
	vxorps %ymm10, %ymm10, %ymm10
	vxorps %ymm11, %ymm11, %ymm11
	vxorps %ymm12, %ymm12, %ymm12
	vxorps %ymm13, %ymm13, %ymm13
	vxorps %ymm14, %ymm14, %ymm14
	vxorps %ymm15, %ymm15, %ymm15
	jmp mult_main


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
.balign 16
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
	vblendps $0xCC, %ymm8, %ymm0, %ymm10
	// second half hi4 lo0
	vblendps $0xCC, %ymm9, %ymm2, %ymm11
	// 0
	vperm2f128 $0x20, %ymm11, %ymm10, %ymm12
	vmovaps %ymm12, (%r9)
	// 4
	vperm2f128 $0x31, %ymm11, %ymm10, %ymm13
	vmovaps %ymm13, -128(%r10)
	// first half hi5 lo1
	vblendps $0x33, %ymm8, %ymm1, %ymm10
	// second half hi5 lo1
	vblendps $0x33, %ymm9, %ymm3, %ymm11
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
	vblendps $0xCC, %ymm8, %ymm4, %ymm10
	// second half hi6 lo2
	vblendps $0xCC, %ymm9, %ymm6, %ymm11
	// 2
	vperm2f128 $0x20, %ymm11, %ymm10, %ymm12
	vmovaps %ymm12, 64(%r9)
	// 6
	vperm2f128 $0x31, %ymm11, %ymm10, %ymm13
	vmovaps %ymm13, -64(%r10)
	// first half hi7 lo3
	vblendps $0x33, %ymm8, %ymm5, %ymm10
	// second half hi7 lo3
	vblendps $0x33, %ymm9, %ymm7, %ymm11
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
