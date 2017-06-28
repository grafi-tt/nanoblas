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
.set offset_next_slice_real_len, 36
.set offset_sched_len,           40

.section .text
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

.globl nanoblas_f32_avx_kernel_asm
nanoblas_f32_avx_kernel_asm:
	// create mask to load C
	// ymm0 will be [0, ..., 0, -1, ..., -1]
	/   #(8 - n_slice_real_len) #n_slice_rean_len
	movl offset_n_slice_real_len(%rcx), %eax
	negq %rax
	leaq loadmask(%rip), %rdx
	vmovups (%rdx, %rax, 4), %ymm0

	// load limit
	movl offset_m_slice_real_len(%rcx), %eax

	// load C
	movq offset_c_cur(%rcx), %r8
	movq offset_ldc(%rcx), %r11
	// backup C addressing
	movq %r8, %r10
	// [ymm8^T, ..., ymm15^T]^T = C (8x8)
	vmaskmovps (%r8), %ymm0, %ymm8
	// constant
	mov $1, %edx
	// -1
	addq %r11, %r8
	movq %r8, %r9
	subl %edx, %eax
	cmovle %rsp, %r9
	vmaskmovps (%r9), %ymm0, %ymm9
	// -2
	addq %r11, %r8
	movq %r8, %r9
	subl %edx, %eax
	cmovle %rsp, %r9
	vmaskmovps (%r9), %ymm0, %ymm10
	// -3
	addq %r11, %r8
	movq %r8, %r9
	subl %edx, %eax
	cmovle %rsp, %r9
	vmaskmovps (%r9), %ymm0, %ymm11
	// -4
	addq %r11, %r8
	movq %r8, %r9
	subl %edx, %eax
	cmovle %rsp, %r9
	vmaskmovps (%r9), %ymm0, %ymm12
	// -5
	addq %r11, %r8
	movq %r8, %r9
	subl %edx, %eax
	cmovle %rsp, %r9
	vmaskmovps (%r9), %ymm0, %ymm13
	// -6
	addq %r11, %r8
	movq %r8, %r9
	subl %edx, %eax
	cmovle %rsp, %r9
	vmaskmovps (%r9), %ymm0, %ymm14
	// -7
	addq %r11, %r8
	subl %edx, %eax
	cmovle %rsp, %r8
	vmaskmovps (%r8), %ymm0, %ymm15

	// loop_len_remained: eax
	movl offset_k_len(%rcx), %eax
	// prepare for duff's device
	xorl %edx, %edx
	subl %eax, %edx
	andl $7, %edx
	// scale by 16 first
	shll $4, %edx
	// prepare for displacement (r9)
	leaq -128(%rdx, %rdx), %r9
	// loop length is 16*9
	// jump address: rdx
	leal (%edx, %edx, 8), %edx
	leaq avx_kernel_nopack_loop(%rip), %r8
	addq %r8, %rdx
	// displacement multiplied by 32, and incremented by 128 for shorter instruction encoding
	negq %r9
	// a_pack_cur: r8
	// b_pack_cur: r9
	movq offset_a_pack_cur(%rcx), %r8
	addq %r9, %r8
	addq offset_b_pack_cur(%rcx), %r9
	// push jump address
	pushq %rdx
	// jump to loop
	nop
	jmp *(%rsp)
	// 16 bytes alinged here

avx_kernel_nopack_loop:

.macro unroll cnt=0, times
	.balign 16
	// ymm3 = (a[8:4], a[8:4]); ymm2 = (a[4:0], a[4:0])
	vmovaps 32*\cnt-128(%r8), %ymm2
	vinsertf128 $0, 32*\cnt-128+16(%r8), %ymm2, %ymm3
	vinsertf128 $1, 32*\cnt-128(%r8), %ymm2, %ymm2
	// ymm5 = b
	vmovaps 32*\cnt-128(%r9), %ymm5

	// ymm4 = [a[0], ..., a[0]]
	vshufps $0x00, %ymm2, %ymm2, %ymm4
	// C[0,] += a[0] * b
	vmulps %ymm4, %ymm5, %ymm4
	vaddps %ymm8, %ymm4, %ymm8
	// ymm4 = [a[1], ..., a[1]]
	vshufps $0x55, %ymm2, %ymm2, %ymm4
	// C[1,] += a[1] * b
	vmulps %ymm4, %ymm5, %ymm4
	vaddps %ymm9, %ymm4, %ymm9
	// ymm4 = [a[2], ..., a[2]]
	vshufps $0xAA, %ymm2, %ymm2, %ymm4
	// C[2,] += a[2] * b
	vmulps %ymm4, %ymm5, %ymm4
	vaddps %ymm10, %ymm4, %ymm10
	// ymm4 = [a[3], ..., a[3]]
	vshufps $0xFF, %ymm2, %ymm2, %ymm4
	// C[3,] += a[3] * b
	vmulps %ymm4, %ymm5, %ymm4
	vaddps %ymm11, %ymm4, %ymm11
	// ymm4 = [a[4], ..., a[4]]
	vshufps $0x00, %ymm3, %ymm3, %ymm4
	// C[4,] += a[4] * b
	vmulps %ymm4, %ymm5, %ymm4
	vaddps %ymm12, %ymm4, %ymm12
	// ymm4 = [a[5], ..., a[5]]
	vshufps $0x55, %ymm3, %ymm3, %ymm4
	// C[5,] += a[5] * b
	vmulps %ymm4, %ymm5, %ymm4
	vaddps %ymm13, %ymm4, %ymm13
	// ymm4 = [a[6], ..., a[6]]
	vshufps $0xAA, %ymm3, %ymm3, %ymm4
	// C[6,] += a[6] * b
	vmulps %ymm4, %ymm5, %ymm4
	vaddps %ymm14, %ymm4, %ymm14
	// ymm4 = [a[7], ..., a[7]]
	vshufps $0xFF, %ymm3, %ymm3, %ymm4
	// C[7,] += a[7] * b
	vmulps %ymm4, %ymm5, %ymm4
	vaddps %ymm15, %ymm4, %ymm15

	.if (\cnt+1)-\times
	unroll (\cnt+1), \times
	.else
	addq $32*\times, %r8
	addq $32*\times, %r9
	subl $\times, %eax
	.endif
.endm

	unroll times=8
	jg avx_kernel_nopack_loop
	addq $8, %rsp
	// loop end

	// update cursor
	addq $-128, %r8
	addq $-128, %r9
	movq %r9, offset_b_pack_cur(%rcx)

.balign 16
store_c:
	// moved here for better alignment
	movq %r8, offset_a_pack_cur(%rcx)
	// load limit again
	movl offset_m_slice_real_len(%rcx), %eax

	// space to put garbage
	subq $32, %rsp
	// store c
	// C is already on r10
	// ldc is already on r11
	vmaskmovps %ymm8, %ymm0, (%r10)
	// +1
	addq %r11, %r10
	movq %r10, %rdx
	subl $1, %eax
	cmovle %rsp, %rdx
	vmaskmovps %ymm9, %ymm0, (%rdx)
	// +2
	addq %r11, %r10
	movq %r10, %rdx
	subl $1, %eax
	cmovle %rsp, %rdx
	vmaskmovps %ymm10, %ymm0, (%rdx)
	// +3
	addq %r11, %r10
	movq %r10, %rdx
	subl $1, %eax
	cmovle %rsp, %rdx
	vmaskmovps %ymm11, %ymm0, (%rdx)
	// +4
	addq %r11, %r10
	movq %r10, %rdx
	subl $1, %eax
	cmovle %rsp, %rdx
	vmaskmovps %ymm12, %ymm0, (%rdx)
	// +5
	addq %r11, %r10
	movq %r10, %rdx
	subl $1, %eax
	cmovle %rsp, %rdx
	vmaskmovps %ymm13, %ymm0, (%rdx)
	// +6
	addq %r11, %r10
	movq %r10, %rdx
	subl $1, %eax
	cmovle %rsp, %rdx
	vmaskmovps %ymm14, %ymm0, (%rdx)
	// +7
	addq %r11, %r10
	subl $1, %eax
	cmovle %rsp, %r10
	vmaskmovps %ymm15, %ymm0, (%r10)
	// restore rsp before jump
	addq $32, %rsp

	// update c_cur
	addq $32, offset_c_cur(%rcx)

	// if required, do packing
	movl offset_current_prepack(%rcx), %edx
	addq %rdx, %rcx
	testl %edx, %edx
	jnz nanoblas_f32_avx_pack_asm

	ret

.balign 16
.globl nanoblas_f32_avx_pack_asm
nanoblas_f32_avx_pack_asm:
	movq offset_next_cur(%rcx), %r8
	movq offset_interval_k(%rcx), %r9
	movq offset_next_pack_cur(%rcx), %r10
	cmpq $4, %r9
	je pack_trans

	// ymm0 will the load mask
	// see comment in nanoblas_f32_avx_kernel_asm for detail, at the place creating mask to load C
	movl offset_next_slice_real_len(%rcx), %eax
	negq %rax
	leaq loadmask(%rip), %rdx
	vmovups (%rdx, %rax, 4), %ymm0

	// load limit
	movl offset_sched_len(%rcx), %eax

	// pack data
	// rdx is zero
	xorl %edx, %edx
	// ymm8 is the temporary
	mov %r9, %r11
	vmaskmovps (%r8), %ymm0, %ymm8
	vmovaps %ymm8, (%r10)
	// +1
	subl $1, %eax
	cmovle %rdx, %r9
	addq %r9, %r8
	vmaskmovps (%r8), %ymm0, %ymm8
	vmovaps %ymm8, 32(%r10)
	// +2
	subl $1, %eax
	cmovle %rdx, %r9
	addq %r9, %r8
	vmaskmovps (%r8), %ymm0, %ymm8
	vmovaps %ymm8, 64(%r10)
	// +3
	subl $1, %eax
	cmovle %rdx, %r9
	addq %r9, %r8
	vmaskmovps (%r8), %ymm0, %ymm8
	vmovaps %ymm8, 96(%r10)
	// incr r10
	addq $256, %r10
	// +4
	subl $1, %eax
	cmovle %rdx, %r9
	addq %r9, %r8
	vmaskmovps (%r8), %ymm0, %ymm8
	vmovaps %ymm8, -128(%r10)
	// +5
	subl $1, %eax
	cmovle %rdx, %r9
	addq %r9, %r8
	vmaskmovps (%r8), %ymm0, %ymm8
	vmovaps %ymm8, -96(%r10)
	// +6
	subl $1, %eax
	cmovle %rdx, %r9
	addq %r9, %r8
	vmaskmovps (%r8), %ymm0, %ymm8
	vmovaps %ymm8, -64(%r10)
	// +7
	subl $1, %eax
	cmovle %rdx, %r9
	addq %r9, %r8
	vmaskmovps (%r8), %ymm0, %ymm8
	vmovaps %ymm8, -32(%r10)
	// correct next_cur ptr
	addq %r11, %r8
	// save cur
	movq %r8, offset_next_cur(%rcx)
	movq %r10, offset_next_pack_cur(%rcx)

	ret

.balign 16
pack_trans:
	// space to put garbage
	subq $32, %rsp

	movq offset_interval_mn(%rcx), %r9

	// ymm0 will be the load mask
	// see comment in nanoblas_f32_avx_kernel_asm for detail, at the place creating mask to load C
	movl offset_sched_len(%rcx), %eax
	addl %eax, %eax
	addl %eax, %eax
	// update next_cur first
	leaq (%r8, %rax), %r11
	movq %r11, offset_next_cur(%rcx)
	// get mask
	negq %rax
	leaq loadmask(%rip), %rdx
	vmovups (%rdx, %rax), %ymm0

	// incr next_pack_cur
	leaq 256(%r10), %r11

	// load limit
	movl offset_next_slice_real_len(%rcx), %eax

	// load
	vmaskmovps (%r8), %ymm0, %ymm8
	// -1
	addq %r9, %r8
	movq %r8, %rdx
	subl $1, %eax
	cmovle %rsp, %rdx
	vmaskmovps (%rdx), %ymm0, %ymm9
	// -2
	addq %r9, %r8
	movq %r8, %rdx
	subl $1, %eax
	cmovle %rsp, %rdx
	vmaskmovps (%rdx), %ymm0, %ymm10
	// -3
	addq %r9, %r8
	movq %r8, %rdx
	subl $1, %eax
	cmovle %rsp, %rdx
	vmaskmovps (%rdx), %ymm0, %ymm11
	// -4
	addq %r9, %r8
	movq %r8, %rdx
	subl $1, %eax
	cmovle %rsp, %rdx
	vmaskmovps (%rdx), %ymm0, %ymm12
	// -5
	addq %r9, %r8
	movq %r8, %rdx
	subl $1, %eax
	cmovle %rsp, %rdx
	vmaskmovps (%rdx), %ymm0, %ymm13
	// -6
	addq %r9, %r8
	movq %r8, %rdx
	subl $1, %eax
	cmovle %rsp, %rdx
	vmaskmovps (%rdx), %ymm0, %ymm14
	// -7
	addq %r9, %r8
	subl $1, %eax
	cmovle %rsp, %r8
	vmaskmovps (%r8), %ymm0, %ymm15

	// save cur
	movq %r11, offset_next_pack_cur(%rcx)

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
	vmovaps %ymm12, (%r10)
	// 4
	vperm2f128 $0x31, %ymm11, %ymm10, %ymm13
	vmovaps %ymm13, -128(%r11)
	// first half hi5 lo1
	vblendps $0x33, %ymm8, %ymm1, %ymm10
	// second half hi5 lo1
	vblendps $0x33, %ymm9, %ymm3, %ymm11
	// 1
	vperm2f128 $0x20, %ymm11, %ymm10, %ymm14
	vmovaps %ymm14, 32(%r10)
	// 5
	vperm2f128 $0x31, %ymm11, %ymm10, %ymm15
	vmovaps %ymm15, -96(%r11)

	// shuf hi
	vshufps $0x4E, %ymm5, %ymm4, %ymm8
	vshufps $0x4E, %ymm7, %ymm6, %ymm9
	// first half hi6 lo2
	vblendps $0xCC, %ymm8, %ymm4, %ymm10
	// second half hi6 lo2
	vblendps $0xCC, %ymm9, %ymm6, %ymm11
	// 2
	vperm2f128 $0x20, %ymm11, %ymm10, %ymm12
	vmovaps %ymm12, 64(%r10)
	// 6
	vperm2f128 $0x31, %ymm11, %ymm10, %ymm13
	vmovaps %ymm13, -64(%r11)
	// first half hi7 lo3
	vblendps $0x33, %ymm8, %ymm5, %ymm10
	// second half hi7 lo3
	vblendps $0x33, %ymm9, %ymm7, %ymm11
	// 3
	vperm2f128 $0x20, %ymm11, %ymm10, %ymm14
	vmovaps %ymm14, 96(%r10)
	// 7
	vperm2f128 $0x31, %ymm11, %ymm10, %ymm15
	vmovaps %ymm15, -32(%r11)

	addq $32, %rsp
	ret
