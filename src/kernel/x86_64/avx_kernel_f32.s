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
	movq offset_interval_mn(%rcx), %r9
	movq offset_next_pack_cur(%rcx), %r10
	cmpq $4, %r9
	jne pack_trans

	// ymm0 will be the template of load mask
	// ymm1 is [1.0, ..., 1.0]
	// see comment in nanoblas_f32_avx_kernel_asm for detail, at the place creating mask to load C
	movl offset_sched_len(%rcx), %eax
	shll $4, %eax
	subl offset_next_slice_real_len(%rcx), %eax
	leaq loadmask(%rip), %rdx
	vmovups (%rdx, %rax, 4), %ymm0
	vmovaps (%rdx), %ymm1

	// pack data
	// ymm8 is the temporary

	vmaskmovps (%r8), %ymm0, %ymm8
	vmovaps %ymm8, (%r10)
	// +1
	vaddps %ymm1, %ymm0, %ymm5
	vaddps %ymm1, %ymm1, %ymm3
	addq %r9, %r8
	vmaskmovps (%r8), %ymm5, %ymm8
	vmovaps %ymm8, 32(%r10)
	// +2
	vaddps %ymm3, %ymm0, %ymm6
	addq %r9, %r8
	vmaskmovps (%r8), %ymm6, %ymm8
	vmovaps %ymm8, 64(%r10)
	// +3
	vaddps %ymm3, %ymm5, %ymm7
	vaddps %ymm3, %ymm3, %ymm3
	addq %r9, %r8
	vmaskmovps (%r8), %ymm7, %ymm8
	vmovaps %ymm8, 96(%r10)
	// incr r10
	addq $256, %r10
	// +4
	vaddps %ymm3, %ymm0, %ymm4
	addq %r9, %r8
	vmaskmovps (%r8), %ymm4, %ymm8
	vmovaps %ymm8, -128(%r10)
	// +5
	vaddps %ymm3, %ymm5, %ymm5
	addq %r9, %r8
	vmaskmovps (%r8), %ymm5, %ymm8
	vmovaps %ymm8, -96(%r10)
	// +6
	vaddps %ymm3, %ymm6, %ymm6
	addq %r9, %r8
	vmaskmovps (%r8), %ymm6, %ymm8
	vmovaps %ymm8, -64(%r10)
	// +7
	vaddps %ymm1, %ymm7, %ymm7
	addq %r9, %r8
	vmaskmovps (%r8), %ymm7, %ymm8
	vmovaps %ymm8, -32(%r10)

	movq %r8, offset_next_cur(%rcx)
	movq %r10, offset_next_pack_cur(%rcx)

	ret

.balign 16
pack_trans:
	movq offset_interval_k(%rcx), %r9

	// ymm0 will be the template of load mask
	// ymm1 is [1.0, ..., 1.0]
	// see comment in nanoblas_f32_avx_kernel_asm for detail, at the place creating mask to load C
	movl offset_next_slice_real_len(%rcx), %eax
	shll $6, %eax
	movl offset_sched_len(%rcx), %edx
	shll $2, %edx
	// update next_cur first
	leaq (%r8, %rdx), %r11
	movq %r11, offset_next_cur(%rcx)
	// get mask
	subl %edx, %eax
	leaq loadmask(%rip), %rdx
	vmovups (%rdx, %rax, 4), %ymm0
	vmovaps (%rdx), %ymm1

	// incr next_pack_cur
	leaq 256(%r10), %r11

	// load
	vmaskmovps (%r8), %ymm0, %ymm8
	// +1
	vaddps %ymm1, %ymm0, %ymm5
	vaddps %ymm1, %ymm1, %ymm3
	addq %r9, %r8
	vmaskmovps (%r8), %ymm5, %ymm9
	// +2
	vaddps %ymm3, %ymm0, %ymm6
	addq %r9, %r8
	vmaskmovps (%r8), %ymm6, %ymm10
	// +3
	vaddps %ymm3, %ymm5, %ymm7
	vaddps %ymm3, %ymm3, %ymm3
	addq %r9, %r8
	vmaskmovps (%r8), %ymm7, %ymm11
	// +4
	vaddps %ymm3, %ymm0, %ymm4
	addq %r9, %r8
	vmaskmovps (%r8), %ymm4, %ymm12
	// +5
	vaddps %ymm3, %ymm5, %ymm5
	addq %r9, %r8
	vmaskmovps (%r8), %ymm5, %ymm13
	// +6
	vaddps %ymm3, %ymm6, %ymm6
	addq %r9, %r8
	vmaskmovps (%r8), %ymm6, %ymm14
	// +7
	vaddps %ymm1, %ymm7, %ymm7
	addq %r9, %r8
	vmaskmovps (%r8), %ymm7, %ymm15

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

.balign 16
store_c:
	// moved here for better alignment
	movq %r8, offset_a_pack_cur(%rcx)

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
	addq $32, offset_c_cur(%rcx)

	// if required, do packing
	movl offset_current_prepack(%rcx), %ecx
	addq %rcx, %rdx
	testl %ecx, %ecx
	jnz nanoblas_f32_avx_pack_asm

	ret
