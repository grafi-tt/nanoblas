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
loadmask:
// [1.0, ..., 1.0]
.rept 8
.float 1
.endr
// [-1.0, ..., -1.0, 0, ..., 0]
.rept 8
.float -1
.endr
.rept 8
.float 0
.endr
// [-2.0, ..., -2.0, 0, ..., 0]
.rept 8
.float -2
.endr
.rept 8
.float 0
.endr
// [-3.0, ..., -3.0, 0, ..., 0]
.rept 8
.float -3
.endr
.rept 8
.float 0
.endr
// [-4.0, ..., -4.0, 0, ..., 0]
.rept 8
.float -4
.endr
.rept 8
.float 0
.endr
// [-5.0, ..., -5.0, 0, ..., 0]
.rept 8
.float -5
.endr
.rept 8
.float 0
.endr
// [-6.0, ..., -6.0, 0, ..., 0]
.rept 8
.float -6
.endr
.rept 8
.float 0
.endr
// [-7.0, ..., -7.0, 0, ..., 0]
.rept 8
.float -7
.endr
.rept 8
.float 0
.endr
// [-8.0, ..., -8.0, 0, ..., 0]
.rept 8
.float -8
.endr
.rept 8
.float 0
.endr

.globl nanoblas_f32_avx_pack_asm
nanoblas_f32_avx_pack_asm:
	movq offset_next_cur(%rcx), %r8
	movq offset_interval_mn(%rcx), %rdx
	cmpq $4, %rdx
	jne pack_trans

	// ymm0 is reserved by nanoblas_avx_kernel_asm
	// ymm1 is anyway [1.0, ..., 1.0]
	// ymm2 will be the template of load mask
	// see comment in nanoblas_f32_avx_kernel_asm for detail, at the place creating mask to load C
	movl offset_sched_len(%rcx), %eax
	shll $4, %eax
	subl offset_next_slice_real_len(%rcx), %eax
	leaq loadmask(%rip), %r9
	vmovups (%r9, %rax, 4), %ymm2
	vmovaps (%r9), %ymm1

	// pack data
	// ymm3 is the temporary
	movq offset_next_pack_cur(%rcx), %r9

	vmaskmovps (%r8), %ymm2, %ymm3
	vmovaps %ymm3, (%r9)
	vaddps %ymm1, %ymm2, %ymm2
	addq %rdx, %r8
	vmaskmovps (%r8), %ymm2, %ymm3
	vmovaps %ymm3, 32(%r9)
	vaddps %ymm1, %ymm2, %ymm2
	addq %rdx, %r8
	vmaskmovps (%r8), %ymm2, %ymm3
	vmovaps %ymm3, 64(%r9)
	vaddps %ymm1, %ymm2, %ymm2
	addq %rdx, %r8
	vmaskmovps (%r8), %ymm2, %ymm3
	vmovaps %ymm3, 96(%r9)
	vaddps %ymm1, %ymm2, %ymm2
	addq %rdx, %r8
	vmaskmovps (%r8), %ymm2, %ymm3
	vmovaps %ymm3, 128(%r9)
	vaddps %ymm1, %ymm2, %ymm2
	addq %rdx, %r8
	vmaskmovps (%r8), %ymm2, %ymm3
	vmovaps %ymm3, 160(%r9)
	vaddps %ymm1, %ymm2, %ymm2
	addq %rdx, %r8
	vmaskmovps (%r8), %ymm2, %ymm3
	vmovaps %ymm3, 192(%r9)
	vaddps %ymm1, %ymm2, %ymm2
	addq %rdx, %r8
	vmaskmovps (%r8), %ymm2, %ymm3
	vmovaps %ymm3, 224(%r9)
	vaddps %ymm1, %ymm2, %ymm2

	ret

.balign 16
pack_trans:
	movq offset_interval_k(%rcx), %rdx

	// ymm0 is reserved by nanoblas_avx_kernel_asm
	// ymm1 is anyway [1.0, ..., 1.0]
	// ymm2 will be the template of load mask
	// see comment in nanoblas_f32_avx_kernel_asm for detail, at the place creating mask to load C
	movl offset_next_slice_real_len(%rcx), %eax
	shll $4, %eax
	subl offset_sched_len(%rcx), %eax
	leaq loadmask(%rip), %r9
	vmovups (%r9, %rax, 4), %ymm2
	vmovaps (%r9), %ymm1

	// align stack
	movq offset_next_pack_cur(%rcx), %r9
	// rcx is not used anymore
	movq %rsp, %rcx
	andq $-32, %rsp
	subq $256, %rsp

	// transpose algorithm is from
	// Optimization Manual 11.11.2

	// copy data to buf, and do first step of transpose
	// ymm3 is the temporary
	// ymm4-ymm7 are working
	// note that ymm8-ymm15 are reserved by nanoblas_avx_kernel_asm
	vmaskmovps (%r8), %ymm2, %ymm4
	vmovaps %ymm4, (%rsp)
	vaddps %ymm1, %ymm2, %ymm2
	addq %rdx, %r8
	vmaskmovps (%r8), %ymm2, %ymm5
	vmovaps %ymm5, 32(%rsp)
	vaddps %ymm1, %ymm2, %ymm2
	addq %rdx, %r8
	vmaskmovps (%r8), %ymm2, %ymm6
	vmovaps %ymm6, 64(%rsp)
	vaddps %ymm1, %ymm2, %ymm2
	addq %rdx, %r8
	vmaskmovps (%r8), %ymm2, %ymm7
	vmovaps %ymm7, 96(%rsp)
	vaddps %ymm1, %ymm2, %ymm2
	addq %rdx, %r8

	vmaskmovps (%r8), %ymm2, %ymm3
	vinsertf128 $1, %xmm3, %ymm4, %ymm4
	vmovaps %ymm3, 128(%rsp)
	vaddps %ymm1, %ymm2, %ymm2
	addq %rdx, %r8
	vmaskmovps (%r8), %ymm2, %ymm3
	vinsertf128 $1, %xmm3, %ymm5, %ymm5
	vmovaps %ymm3, 160(%rsp)
	vaddps %ymm1, %ymm2, %ymm2
	addq %rdx, %r8
	vmaskmovps (%r8), %ymm2, %ymm3
	vinsertf128 $1, %xmm3, %ymm6, %ymm6
	vmovaps %ymm3, 192(%rsp)
	vaddps %ymm1, %ymm2, %ymm2
	addq %rdx, %r8
	vmaskmovps (%r8), %ymm2, %ymm3
	vinsertf128 $1, %xmm3, %ymm7, %ymm7
	vmovaps %ymm3, 224(%rsp)
	vaddps %ymm1, %ymm2, %ymm2

	// second step
	vunpcklpd %ymm5, %ymm4, %ymm2
	vunpckhpd %ymm5, %ymm4, %ymm3
	vunpcklpd %ymm7, %ymm6, %ymm4
	vunpckhpd %ymm7, %ymm6, %ymm5

	vshufps $0x88, %ymm4, %ymm2, %ymm6
	vmovaps %ymm6, (%r9)
	vshufps $0xDD, %ymm4, %ymm2, %ymm6
	vmovaps %ymm6, 32(%r9)
	vshufps $0x88, %ymm5, %ymm3, %ymm6
	vmovaps %ymm6, 64(%r9)
	vshufps $0xDD, %ymm5, %ymm3, %ymm6
	vmovaps %ymm6, 96(%r9)

	// third step
	vmovaps 16(%rsp), %xmm4
	vinsertf128 $1, 144(%rsp), %ymm4, %ymm4
	vmovaps 48(%rsp), %xmm5
	vinsertf128 $1, 176(%rsp), %ymm5, %ymm5
	vunpcklpd %ymm5, %ymm4, %ymm2
	vunpckhpd %ymm5, %ymm4, %ymm3

	vmovaps 80(%rsp), %xmm6
	vinsertf128 $1, 208(%rsp), %ymm6, %ymm6
	vmovaps 112(%rsp), %xmm7
	vinsertf128 $1, 240(%rsp), %ymm7, %ymm7
	vunpcklpd %ymm7, %ymm6, %ymm4
	vunpckhpd %ymm7, %ymm6, %ymm5

	// fourth step
	vshufps $0x88, %ymm4, %ymm2, %ymm6
	vmovaps %ymm6, 128(%r9)
	vshufps $0xDD, %ymm4, %ymm2, %ymm6
	vmovaps %ymm6, 160(%r9)
	vshufps $0x88, %ymm5, %ymm3, %ymm6
	vmovaps %ymm6, 192(%r9)
	vshufps $0xDD, %ymm5, %ymm3, %ymm6
	vmovaps %ymm6, 224(%r9)

	movq %rcx, %rsp
	ret

.balign 16
.globl nanoblas_f32_avx_kernel_asm
nanoblas_f32_avx_kernel_asm:
	// create mask to load C
	// ymm0 will be [0, ..., 0, -m_slice_real_len, ..., -m_slice_real_len]
	//          #(8 - n_slice_real_len)     #n_slice_rean_len
	movl offset_m_slice_real_len(%rcx), %eax
	shll $4, %eax
	subl offset_n_slice_real_len(%rcx), %eax
	leaq loadmask(%rip), %rdx
	vmovups (%rdx, %rax, 4), %ymm0
	// ymm1 will be [1.0, ..., 1.0]
	vmovaps (%rdx), %ymm1

	// load C
	movq offset_c_cur(%rcx), %r10
	movq offset_ldc(%rcx), %r11
	// [ymm8^T, ..., ymm15^T]^T = C (8x8)
	vmaskmovps (%r10), %ymm0, %ymm8
	// +1
	vaddps %ymm1, %ymm0, %ymm5
	vaddps %ymm1, %ymm1, %ymm3
	addq %r11, %r10
	vmaskmovps (%r10), %ymm5, %ymm9
	// +2
	vaddps %ymm3, %ymm0, %ymm6
	addq %r11, %r10
	vmaskmovps (%r10), %ymm6, %ymm10
	// +3
	vaddps %ymm3, %ymm5, %ymm7
	vaddps %ymm3, %ymm3, %ymm3
	addq %r11, %r10
	vmaskmovps (%r10), %ymm7, %ymm11
	// +4
	vaddps %ymm3, %ymm0, %ymm4
	addq %r11, %r10
	vmaskmovps (%r10), %ymm4, %ymm12
	// +5
	vaddps %ymm3, %ymm5, %ymm5
	addq %r11, %r10
	vmaskmovps (%r10), %ymm5, %ymm13
	// +6
	vaddps %ymm3, %ymm6, %ymm6
	addq %r11, %r10
	vmaskmovps (%r10), %ymm6, %ymm14
	// +7
	vaddps %ymm3, %ymm7, %ymm7
	addq %r11, %r10
	vmaskmovps (%r10), %ymm7, %ymm15
	// prepare for store
	movq offset_c_cur(%rcx), %r10

	// loop_len_remained: eax
	movl offset_k_len(%rcx), %eax
	// prepare for duff's device
	// notice that edx is NULL
	subl %eax, %edx
	andl $7, %edx
	// scale by 16 first
	shll $4, %edx
	// prepare for displacement (r9)
	leaq (%rdx, %rdx), %r9
	addq $-128, %r9
	// loop length is 16*9
	// jump address: rdx
	leal (%edx, %edx, 8), %edx
	leaq avx_kernel_nopack_loop(%rip), %r8
	addq %r8, %rdx
	// displacement multiplied by 32, and shift it by 128 for shorter instruction encoding
	negq %r9
	// a_pack_cur: r8
	// b_pack_cur: r9
	movq offset_a_pack_cur(%rcx), %r8
	addq %r9, %r8
	addq offset_b_pack_cur(%rcx), %r9
	// push jump address
	pushq %rdx
	// backup rcx for packing
	movq %rcx, %rdx
	// jump to loop
.byte 0x0f, 0x1f, 0x44, 0x00, 0x00
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

	// if required, do packing
	movl offset_current_prepack(%rcx), %ecx
	testl %ecx, %ecx
	jz store_c
	addq %rdx, %rcx
	call nanoblas_f32_avx_pack_asm

.balign 16
store_c:
	// moved here for better alignment
	movq %r8, offset_a_pack_cur(%rdx)

	// store c
	// ldc is already on r11
	vmaskmovps %ymm8, %ymm0, (%r10)
	// +1
	vaddps %ymm1, %ymm0, %ymm5
	vaddps %ymm1, %ymm1, %ymm3
	addq %r11, %r10
	vmaskmovps %ymm9, %ymm5, (%r10)
	// +2
	vaddps %ymm3, %ymm0, %ymm6
	addq %r11, %r10
	vmaskmovps %ymm10, %ymm6, (%r10)
	// +3
	vaddps %ymm3, %ymm5, %ymm7
	vaddps %ymm3, %ymm3, %ymm3
	addq %r11, %r10
	vmaskmovps %ymm11, %ymm7, (%r10)
	// +4
	vaddps %ymm3, %ymm0, %ymm4
	addq %r11, %r10
	vmaskmovps %ymm12, %ymm4, (%r10)
	// +5
	vaddps %ymm3, %ymm5, %ymm5
	addq %r11, %r10
	vmaskmovps %ymm13, %ymm5, (%r10)
	// +6
	vaddps %ymm3, %ymm6, %ymm6
	addq %r11, %r10
	vmaskmovps %ymm14, %ymm6, (%r10)
	// +7
	vaddps %ymm3, %ymm7, %ymm7
	addq %r11, %r10
	vmaskmovps %ymm15, %ymm7, (%r10)

	// update c_cur
	add $32, offset_c_cur(%rdx)

	ret
