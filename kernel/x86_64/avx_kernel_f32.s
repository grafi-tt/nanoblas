// edi: k_len
// esi: next_k_len
// edx: next_m_len
// ecx: next_k_cnt
// r8d: next_m_cnt
// r9: transa -> transa ? lda : 0
// r10: a_pack_next (load from stack)
// r11: lda (load from stack)
// r12: a (load from stack)
// r13: a_pack_cur (escape to stack; load from stack)
// r14: b_pack_cur (escape to stack; load from stack)
// rax: ldc (load from stack) or tmp
// rbx: c (escape to stack; load from stack) or tmp
// r15: transa ? 0 : lda (escape to stack)
// rbp: frame pointer
// rsp: stack pointer

.section .text
.globl avx_kernel_f32
avx_kernel_f32:
	push %rbp
	movq %rsp, %rbp

	// escape
	pushq %r13
	pushq %r14
	pushq %rbx
	pushq %r15
	// load
	movq (%rbp,16), %r10
	movq (%rbp,24), %r11
	movq (%rbp,32), %r12
	movq (%rbp,40), %r13
	movq (%rbp,48), %r14

	// lda
	xorq %rax, %rax
	test %r9d, %r9d
	setz %al
	negq %eax, %r15
	notq %r15, %r9
	andq %r11, %r15
	andq %r11, %r9

	// load c
	movq (%rbp,56), %rax
	movq (%rbp,64), %rbx
	vmovaps (%rbx), %ymm0
	leaq (%rbx,%rax,4), %rbx
	vmovaps (%rbx), %ymm1
	leaq (%rbx,%rax,4), %rbx
	vmovaps (%rbx), %ymm2
	leaq (%rbx,%rax,4), %rbx
	vmovaps (%rbx), %ymm3
	leaq (%rbx,%rax,4), %rbx
	vmovaps (%rbx), %ymm4
	leaq (%rbx,%rax,4), %rbx
	vmovaps (%rbx), %ymm5
	leaq (%rbx,%rax,4), %rbx
	vmovaps (%rbx), %ymm6
	leaq (%rbx,%rax,4), %rbx
	vmovaps (%rbx), %ymm7
	leaq (%rbx,%rax,4), %rbx

	// loop start
.align 5
avx_kernel_loop:

	// unroll 0
.macro unroll offset
	// ymm9 = (a'[8:4], a'[8:4]); ymm8 = (a'[4:0], a'[4:0]) */
	vmovaps \offset(%r13), %ymm8
	vperm2f128 %ymm8, %ymm8, %ymm9, 0x11
	vperm2f128 %ymm8, %ymm8, %ymm8, 0x00
	// ymm12 = b' */
	vmovpas \offset(%r14) %ymm12
	// c[0,] += a'[0] * b'; c[1,] += a[1] * b'
	vshufps $0x00, %ymm8, %ymm10
	vshufps $0x11, %ymm8, %ymm11
	vmulps %ymm10, %ymm12, %ymm14
	vmulps %ymm11, %ymm12, %ymm15
	vaddps %ymm0, %ymm14, %ymm0
	vaddps %ymm1, %ymm15, %ymm1
	// c[2,] += a'[2] * b'; c[3,] += a'[3] * b'
	vshufps $0x22, %ymm8, %ymm10
	vshufps $0x33, %ymm8, %ymm11
	vmulps %ymm10, %ymm12, %ymm14
	vmulps %ymm11, %ymm12, %ymm15
	vaddps %ymm2, %ymm14, %ymm2
	vaddps %ymm3, %ymm15, %ymm3
	// c[4,] += a'[4] * b'; c[5,] += a'[5] * b'
	vshufps $0x44, %ymm8, %ymm10
	vshufps $0x55, %ymm8, %ymm11
	vmulps %ymm10, %ymm12, %ymm14
	vmulps %ymm11, %ymm12, %ymm15
	vaddps %ymm4, %ymm14, %ymm4
	vaddps %ymm5, %ymm15, %ymm5
	// c[6,] += a'[6] * b'; c[7,] += a'[7] * b'
	vshufps $0x66, %ymm8, %ymm10
	vshufps $0x77, %ymm8, %ymm11
	vmulps %ymm10, %ymm12, %ymm14
	vmulps %ymm11, %ymm12, %ymm15
	vaddps %ymm6, %ymm14, %ymm6
	vaddps %ymm7, %ymm15, %ymm7
.endm
	unroll $0
	// termination check
	subd $1, %edi
	jz pack

	// unroll 1
	unroll $32
	/* proceed pointers */
	leaq 64(%r13)
	leaq 64(%r14)

pack:
	// pack needed?
	cmp %r8d, %edx
	jz pack_end
	// load a[m_len,k_len]
	movq %rcx, %rax
	movq %r8, %rbx
	mulq %r9, %rax
	mulq %r15, %rbx
	addq %rbx, %rax
	movq (%r12,%rax,4), %rbx
	// store a[m_len,k_len]
	movl %ecx, %eax
	mull %edx, %eax
	addl %r8d, %eax
	movq %rbx, (%r13,%rax,4)
	// increment
	addl $1, %r8d
	// eax (next_m_cnt == next_m_len) ? 1 : 0
	xorl %eax, %eax
	cmp %r8d, %edx
	sete %al
	// (next_k_cnt++, next_m_cnt = 0) if moving up
	addl %eax, %ecx
	subl $1, %eax
	andl %eax, %edx
pack_end:
	// termination check
	subl $1, %edi
	jnz avx_kernel_loop
	// restart packing until completed
	addl $1, %edi
	cmp %r8d, %edx
	jnz pack

avx_kernel_loop_end:

	// store c
	movq (%rbp,56), %rax
	movq (%rbp,64), %rbx
	vmovaps %ymm0, (%rbx)
	leaq (%rbx,%rax,4), %rbx
	vmovaps %ymm1, (%rbx)
	leaq (%rbx,%rax,4), %rbx
	vmovaps %ymm2, (%rbx)
	leaq (%rbx,%rax,4), %rbx
	vmovaps %ymm3, (%rbx)
	leaq (%rbx,%rax,4), %rbx
	vmovaps %ymm4, (%rbx)
	leaq (%rbx,%rax,4), %rbx
	vmovaps %ymm5, (%rbx)
	leaq (%rbx,%rax,4), %rbx
	vmovaps %ymm6, (%rbx)
	leaq (%rbx,%rax,4), %rbx
	vmovaps %ymm7, (%rbx)
	leaq (%rbx,%rax,4), %rbx

	// return
	popq %r15
	popq %rbx
	popq %r14
	popq %r13
	movq %rbp, %rsp
	popq %rbp
	ret
