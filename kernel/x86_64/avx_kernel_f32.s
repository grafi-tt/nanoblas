/* rbp: tmp                                                                 */
/* rsp: stack pointer                                                       */
/* edi: k_len                                                               */
/* esi: next_m_cnt                                                          */
/* edx: next_m_len                                                          */
/* ecx: next_k_cnt                                                          */
/* r8d: next_k_len                                                          */
/* r9: a_pack_next                                                          */
/* r10: transa -> transa ? lda : 0 (load from stack)                        */
/* r11: lda -> transa ? 0 : lda (load from stack)                           */
/* r12: a (escape to stack, load from stack)                                */
/* rax: ldc -> tmp -> a_pack_cur -> ldc (load from stack)                   */
/* rbx: c -> b_pack_cur -> c (escape to stack; load from stack)             */
/* r13: tmp (escape to stack)                                               */
/* r14: reserved                                                            */
/* r15: reserved                                                            */

.section .text

.align 4
.globl avx_kernel_f32
avx_kernel_f32:
	/* escape */
	pushq %rbp
	/* rsp = base pointer - 24 */
	pushq %rbx
	pushq %r12
	pushq %r13

	/* load c */
	movq (%rsp,80), %rax
	movq (%rsp,88), %rbx
	vmovaps (%rbx), %ymm8
	leaq (%rbx,%rax,4), %rbx
	vmovaps (%rbx), %ymm9
	leaq (%rbx,%rax,4), %rbx
	vmovaps (%rbx), %ymm10
	leaq (%rbx,%rax,4), %rbx
	vmovaps (%rbx), %ymm11
	leaq (%rbx,%rax,4), %rbx
	vmovaps (%rbx), %ymm12
	leaq (%rbx,%rax,4), %rbx
	vmovaps (%rbx), %ymm13
	leaq (%rbx,%rax,4), %rbx
	vmovaps (%rbx), %ymm14
	leaq (%rbx,%rax,4), %rbx
	vmovaps (%rbx), %ymm15
	leaq (%rbx,%rax,4), %rbx
	/* lda */
	movq (%rsp,40), %r10
	movq (%rsp,48), %r11
	xorl %eax, %eax
	test %r10d, %r10d
	setnz %al
	negq %rax, %rax
	movq %r11 %r10
	andq %rax %r10
	notq %rax, %rax
	andq %rax, %r11
	/* load  other */
	movq (%rsp,56), %r12
	movq (%rsp,64), %rax
	movq (%rsp,72), %rbx

	/* loop start */
.align 4
avx_kernel_loop:

	/* unroll 0 */
.macro unroll offset
	/* ymm1 = (a'[8:4], a'[8:4]); ymm0 = (a'[4:0], a'[4:0]) */ */
	vmovaps \offset(%rax), %ymm0
	vperm2f128 0x11, %ymm0, %ymm0, %ymm1
	vperm2f128 0x00, %ymm0, %ymm0, %ymm0
	/* ymm4 = b' */ */
	vmovpas \offset(%r13) %ymm4
	/* c[0,] += a'[0] * b'; c[1,] += a[1] * b' */
	vshufps $0x00, %ymm0, %ymm2
	vshufps $0x55, %ymm0, %ymm3
	vmulps %ymm2, %ymm4, %ymm6
	vmulps %ymm3, %ymm4, %ymm7
	vaddps %ymm8, %ymm6, %ymm8
	vaddps %ymm9, %ymm7, %ymm9
	/* c[2,] += a'[2] * b'; c[3,] += a'[3] * b' */
	vshufps $0xAA, %ymm0, %ymm2
	vshufps $0xFF, %ymm0, %ymm3
	vmulps %ymm2, %ymm4, %ymm6
	vmulps %ymm3, %ymm4, %ymm7
	vaddps %ymm10, %ymm6, %ymm10
	vaddps %ymm11, %ymm7, %ymm11
	/* c[4,] += a'[4] * b'; c[5,] += a'[5] * b' */
	vshufps $0x00, %ymm1, %ymm2
	vshufps $0x55, %ymm1, %ymm3
	vmulps %ymm2, %ymm4, %ymm6
	vmulps %ymm3, %ymm4, %ymm7
	vaddps %ymm12, %ymm6, %ymm12
	vaddps %ymm13, %ymm7, %ymm13
	/* c[6,] += a'[6] * b'; c[7,] += a'[7] * b' */
	vshufps $0xAA, %ymm1, %ymm2
	vshufps $0xFF, %ymm1, %ymm3
	vmulps %ymm2, %ymm4, %ymm6
	vmulps %ymm3, %ymm4, %ymm7
	vaddps %ymm14, %ymm6, %ymm14
	vaddps %ymm15, %ymm7, %ymm15
.endm
	unroll $0
	/* termination check */
	subl $1, %edi
	jz pack_restart:

	/* unroll 1 */
	unroll $32
	/* proceed pointers */
	leaq 64(%r12)
	leaq 64(%r13)

pack:
	/* pack needed? */
	cmp %ecx, %r8d
	je pack_end
pack_do:
	/* load a[m_len,k_len] */
	movl %ecx, %ebp
	movl %esi, %r13d
	mulq %r10, %rbp
	mulq %r11, %r13
	addq %r13, %rbp
	movq (%r12,%rbp,4), %r13
	/* store a[m_len,k_len] */
	movl %ecx, %ebp
	mull %edx, %ebp
	addl %esi, %ebp
	movq %r13, (%rax,%rbp,4)
	/* increment */
	addl $1, %esi
	/* eax (next_m_cnt == next_m_len) ? 1 : 0 */
	xorl %ebp, %ebp
	cmp %esi, %edx
	sete %bpl
	/* (next_k_cnt++, next_m_cnt = 0) if moving up */
	addl %ebp, %ecx
	subl $1, %ebp
	andl %ebp, %esi
pack_end:
	/* termination check */
	subl $1, %edi
	jnz avx_kernel_loop
pack_restart:
	/* restart packing until completed */
	addl $1, %edi
	cmp %ecx, %r8d
	jnz pack_do:

	/* loop end */

	/* store c */
	movq (%rsp,80), %rax
	movq (%rsp,88), %rbx
	vmovaps %ymm8, (%rbx)
	leaq (%rbx,%rax,4), %rbx
	vmovaps %ymm9, (%rbx)
	leaq (%rbx,%rax,4), %rbx
	vmovaps %ymm10, (%rbx)
	leaq (%rbx,%rax,4), %rbx
	vmovaps %ymm11, (%rbx)
	leaq (%rbx,%rax,4), %rbx
	vmovaps %ymm12, (%rbx)
	leaq (%rbx,%rax,4), %rbx
	vmovaps %ymm13, (%rbx)
	leaq (%rbx,%rax,4), %rbx
	vmovaps %ymm14, (%rbx)
	leaq (%rbx,%rax,4), %rbx
	vmovaps %ymm15, (%rbx)
	leaq (%rbx,%rax,4), %rbx

	/* return */
	popq %r13
	popq %r12
	popq %rbx
	popq %rbp
	ret
