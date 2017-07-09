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

.macro mult_load0
movq %r10, %r12
leaq (%r12, %r9), %r13
leaq (%r9, %r9), %r10
vmaskmovps (%r8), %ymm0, %ymm1
vmovaps %ymm1, (%rsp)
.endm
.macro mult_load1
addq %r8, %r9
cmpl $1, %eax
cmovle %rsi, %r9
vmaskmovps (%r9), %ymm0, %ymm1
vmovaps %ymm1, 32(%rsp)
.endm
.macro mult_load2
leaq (%r12, %r10), %r14
addq %r8, %r10
cmpl $2, %eax
cmovle %rsi, %r10
vmaskmovps (%r10), %ymm0, %ymm1
vmovaps %ymm1, 64(%rsp)
.endm
.macro mult_load3
leaq (%r12, %r11), %r15
addq %r8, %r11
cmpl $3, %eax
cmovle %rsi, %r11
vmaskmovps (%r11), %ymm0, %ymm1
vmovaps %ymm1, 96(%rsp)
.endm
.macro mult_load4
cmpl $4, %eax
cmovle %rsi, %r12
vmaskmovps (%r12), %ymm0, %ymm1
vmovaps %ymm1, -128(%rsi)
.endm
.macro mult_load5
cmpl $5, %eax
cmovle %rsi, %r13
vmaskmovps (%r13), %ymm0, %ymm1
vmovaps %ymm1, -96(%rsi)
.endm
.macro mult_load6
cmpl $6, %eax
cmovle %rsi, %r14
vmaskmovps (%r14), %ymm0, %ymm1
vmovaps %ymm1, -64(%rsi)
.endm
.macro mult_load7
cmpl $7, %eax
cmovle %rsi, %r15
vmaskmovps (%r15), %ymm0, %ymm1
vmovaps %ymm1, -32(%rsi)
.endm

.macro mult_store0
vaddps (%rsp), %ymm8, %ymm8
vmaskmovps %ymm8, %ymm0, (%r8)
.endm
.macro mult_store1
vaddps 32(%rsp), %ymm9, %ymm9
vmaskmovps %ymm9, %ymm0, (%r9)
.endm
.macro mult_store2
vaddps 64(%rsp), %ymm10, %ymm10
vmaskmovps %ymm10, %ymm0, (%r10)
.endm
.macro mult_store3
vaddps 96(%rsp), %ymm11, %ymm11
vmaskmovps %ymm11, %ymm0, (%r11)
.endm
.macro mult_store4
vaddps -128(%rsi), %ymm12, %ymm12
vmaskmovps %ymm12, %ymm0, (%r12)
.endm
.macro mult_store5
vaddps -96(%rsi), %ymm13, %ymm13
vmaskmovps %ymm13, %ymm0, (%r13)
.endm
.macro mult_store6
vaddps -64(%rsi), %ymm14, %ymm14
vmaskmovps %ymm14, %ymm0, (%r14)
.endm
.macro mult_store7
vaddps -32(%rsi), %ymm15, %ymm15
vmaskmovps %ymm15, %ymm0, (%r15)
.endm

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
	vmovaps %ymm0, 256(%rsp)

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
	// if <16, skip prefetch, and inspect where to jump
	subl $16, %eax
	jl mult_check_fallback
	// prefetch & store

mult_prefetch:
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
	vaddps %ymm6, %ymm8, %ymm8
	.endif
	// ymm6 = [a[1], ..., a[1]]
	vshufps $0x55, %ymm4, %ymm4, %ymm6
	// C[1,] += a[1] * b
	.if (\cnt == 0)
	vmulps %ymm6, %ymm7, %ymm9
	.else
	vmulps %ymm6, %ymm7, %ymm6
	vaddps %ymm6, %ymm9, %ymm9
	.endif
	// ymm6 = [a[2], ..., a[2]]
	vshufps $0xAA, %ymm4, %ymm4, %ymm6
	// C[2,] += a[2] * b
	.if (\cnt == 0)
	vmulps %ymm6, %ymm7, %ymm10
	.else
	vmulps %ymm6, %ymm7, %ymm6
	vaddps %ymm6, %ymm10, %ymm10
	.endif
	// ymm6 = [a[3], ..., a[3]]
	vshufps $0xFF, %ymm4, %ymm4, %ymm6
	// C[3,] += a[3] * b
	.if (\cnt == 0)
	vmulps %ymm6, %ymm7, %ymm11
	.else
	vmulps %ymm6, %ymm7, %ymm6
	vaddps %ymm6, %ymm11, %ymm11
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
	vaddps %ymm6, %ymm12, %ymm12
	.endif
	// ymm6 = [a[5], ..., a[5]]
	vshufps $0x55, %ymm5, %ymm5, %ymm6
	// C[5,] += a[5] * b
	.if (\cnt == 0)
	vmulps %ymm6, %ymm7, %ymm13
	.else
	vmulps %ymm6, %ymm7, %ymm6
	vaddps %ymm6, %ymm13, %ymm13
	.endif
	// ymm6 = [a[6], ..., a[6]]
	vshufps $0xAA, %ymm5, %ymm5, %ymm6
	// C[6,] += a[6] * b
	.if (\cnt == 0)
	vmulps %ymm6, %ymm7, %ymm14
	.else
	vmulps %ymm6, %ymm7, %ymm6
	vaddps %ymm6, %ymm14, %ymm14
	.endif
	// ymm6 = [a[7], ..., a[7]]
	vshufps $0xFF, %ymm5, %ymm5, %ymm6
	// C[7,] += a[7] * b
	.if (\cnt == 0)
	vmulps %ymm6, %ymm7, %ymm15
	.else
	vmulps %ymm6, %ymm7, %ymm6
	vaddps %ymm6, %ymm15, %ymm15
	.endif

	.if (\cnt+1)-\times
	mult_prefetch_macro (\cnt+1), \times
	.else
	addq $32*\times, %rcx
	addq $32*\times, %rdx
	.endif
.endm
	mult_prefetch_macro times=8

	test %eax, %eax
	jz mult_epilogue

.balign 16
mult_main: // 32 bytes sequence
	// prepare for duff's device
	pushq %rax
	negl %eax
	andl $7, %eax
	// displacement multiplied by -32: rcx, rdx
	shlq $5, %rax
	subq %rax, %rcx
	subq %rax, %rdx
	// loop length is 32*4
	leaq mult_duffs_loop(%rip), %rsi
	leaq (%rsi, %rax, 4), %rsi
	popq %rax
	pushq %rsi
	// jump to loop
	jmp *(%rsp)

mult_duffs_loop: // 32 bytes sequences
.macro mult_duffs_macro cnt=0, times
	//.balign 16
	// ymm5 = (a[8:4], a[8:4]); ymm4 = (a[4:0], a[4:0])
	.if \cnt == 4
	// vmovaps 0(%rcx), %ymm7
	.byte 0xc5, 0xfc, 0x28, 0x61, 0x00
	.else
	vmovaps 32*\cnt-128(%rcx), %ymm4
	.endif

	vinsertf128 $0, 32*\cnt-128+16(%rcx), %ymm4, %ymm5
	.if \cnt == 4
	// vinsertf128 $1, 0(%rcx), %ymm4, %ymm4
	.byte 0xc4, 0xe3, 0x5d, 0x18, 0x61, 0x00, 0x01
	.else
	vinsertf128 $1, 32*\cnt-128(%rcx), %ymm4, %ymm4
	.endif

	// ymm7 = b
	.if \cnt == 4
	// vmovaps 0(%rdx), %ymm7
	.byte 0xc5, 0xfc, 0x28, 0x7a, 0x00
	.else
	vmovaps 32*\cnt-128(%rdx), %ymm7
	.endif

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

mult_epilogue:
	// rsi holds 0 address
	leaq 256(%rsp), %rsi

	// create mask to load C
	// ymm0 will be hi[0, ..., 0, -1, ..., -1]lo
	//    #(8 - n_slice_real_len) #n_slice_rean_len
	movl offset_n_slice_real_len(%rdi), %eax
	shll $2, %eax
	pushq %rax
	leaq loadmask(%rip), %rax
	subq (%rsp), %rax
	vmovups (%rax), %ymm0
	popq %rax

	// load limit
	movl offset_m_slice_real_len(%rdi), %eax

.macro mult_load_macro cnt=0, times
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
	// load
	.if \cnt == 0
	mult_load0
	.elseif \cnt == 1
	mult_load2
	.elseif \cnt == 2
	mult_load4
	.elseif \cnt == 3
	mult_load6
	.endif
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
	// load
	.if \cnt == 0
	mult_load1
	.elseif \cnt == 1
	mult_load3
	.elseif \cnt == 2
	mult_load5
	.elseif \cnt == 3
	mult_load7
	.endif
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
	mult_load_macro (\cnt+1), \times
	.else
	addq $32*\times, %rcx
	addq $32*\times, %rdx
	.endif
.endm
mult_load_macro times=8

	// update cursor
	addq $-128, %rcx
	addq $-128, %rdx
	movq %rcx, offset_a_pack_cur(%rdi)
	movq %rdx, offset_b_pack_cur(%rdi)

	// add to C; store
	mult_store0
	mult_store1
	mult_store2
	mult_store3
	mult_store4
	mult_store5
	mult_store6
	mult_store7

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
mult_check_fallback:
	vxorps %ymm8, %ymm8, %ymm8
	vxorps %ymm9, %ymm9, %ymm9
	vxorps %ymm10, %ymm10, %ymm10
	vxorps %ymm11, %ymm11, %ymm11
	vxorps %ymm12, %ymm12, %ymm12
	vxorps %ymm13, %ymm13, %ymm13
	vxorps %ymm14, %ymm14, %ymm14
	vxorps %ymm15, %ymm15, %ymm15

	subl $-8, %eax
	jg mult_main
	je mult_epilogue

.balign 16
mult_fallback: // 32bytes sequence
	// %eax is loop_len-8, [-7,-1]
	negl %eax
	// displacement multiplied by -32: rcx, rdx
	shll $5, %eax
	subq %rax, %rcx
	subq %rax, %rdx
	// first loop is truncated
	subq $32, %rax
	// loop length is 32*4
	leaq mult_duffs_through(%rip), %rsi
	shlq $2, %rax
	addq %rax, %rsi
	// jump to loop
	jmp *(%rsp)

mult_duffs_through: // 32bytes sequences
	mult_duffs_macro cnt=1, times=8

mult_load_store_c:
	// update cursor
	addq $-128, %rcx
	addq $-128, %rdx
	movq %rcx, offset_a_pack_cur(%rdi)
	movq %rdx, offset_b_pack_cur(%rdi)

	// rsi holds 0 address
	leaq 256(%rsp), %rsi

	// create mask to load C
	// ymm0 will be hi[0, ..., 0, -1, ..., -1]lo
	//    #(8 - n_slice_real_len) #n_slice_rean_len
	movl offset_n_slice_real_len(%rdi), %eax
	shll $2, %eax
	pushq %rax
	leaq loadmask(%rip), %rax
	subq (%rsp), %rax
	vmovups (%rax), %ymm0
	popq %rax

	// load limit
	movl offset_m_slice_real_len(%rdi), %eax

	// load C[0,]
	mult_load0
	// ymm8 += C[0,]; store C[0,]
	mult_store0
	// load C[1,]
	mult_load1
	// ymm9 += C[1,]; store C[1,]
	mult_store1
	// load C[2,]
	mult_load2
	// ymm10 += C[2,]; store C[2,]
	mult_store2
	// load C[3,]
	mult_load3
	// ymm11 += C[3,]; store C[3,]
	mult_store3
	// load C[4,]
	mult_load4
	// ymm12 += C[4,]; store C[4,]
	mult_store4
	// load C[5,]
	mult_load5
	// ymm13 += C[5,]; store C[5,]
	mult_store5
	// load C[6,]
	mult_load6
	// ymm14 += C[6,]; storeC[6,]
	mult_store6
	// load C[7,]
	mult_load7
	// ymm15 += C[7,]; store C[7,]
	mult_store7

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
