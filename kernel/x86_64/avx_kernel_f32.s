/* ebp: k_len                     (escape to stack; moved from reg)         */
/* rsp: stack pointer                                                       */
/* rdi: b_pack                    (init. k_len)                             */
/* rsi: a_pack                                                              */
/* edx: k_len_sched               (loaded from stack; init. b_pack)         */
/* ecx: m_len_sched                                                         */
/* r8:  a                         (loaded from stack; init. k_len_next )    */
/* r9:  trans_a ? lda :   1       (init. trans_a, which loaded from stack)  */
/* r10: trans_a ?   1 : lda       (init. lda, which loaded from stack)      */
/* r11: a_pack_next               (loaded from stack)                       */
/* rax: tmp, at begin and end ldc (loaded from stack)                       */
/* rbx: tmp, at begin and end c   (escape to stack; loaded from stack)      */
/* r12: tmp                       (escape to stack)                         */
/* r13: reserved                                                            */
/* r14: reserved                                                            */
/* r15: reserved                                                            */

.section .text

.align 4
.globl avx_kernel_f32
avx_kernel_f32:
	/* escape */
	pushq %rbp
	/* rsp = base pointer - 16 */
	pushq %rbx
	pushq %r12

	/* move some registers */
	movl %edi %ebp
	movl %rdx %rdi
	movl %r8 %edx
	/* load c */
	movq (%rsp,56), %rax
	movq (%rsp,64), %rbx
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
	movq (%rsp,32), %r10
	xorl %eax, %eax
	test %r9d, %r9d
	setnz %al
	negq %rax, %rax
	movq %r10 %r9
	andq %rax %r9
	notq %rax, %rax
	andq %rax, %r10
	/* load  other */
	movq (%rsp,40), %r11
	movq (%rsp,48), %r8

	/* loop start */
.align 4
avx_kernel_loop:

	/* unroll 0 */
.macro unroll offset
	/* ymm1 = (a'[8:4], a'[8:4]); ymm0 = (a'[4:0], a'[4:0]) */
	vmovaps \offset(%rsi), %ymm0
	vperm2f128 0x11, %ymm0, %ymm0, %ymm1
	vperm2f128 0x00, %ymm0, %ymm0, %ymm0
	/* ymm4 = b' */ */
	vmovpas \offset(%rdi) %ymm4
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
	unroll 0
	/* termination check */
	subl $1, %ebp
	jz pack_restart:

	/* unroll 1 */
	unroll 32
	/* proceed pointers */
	leaq 64(%rsi)
	leaq 64(%rdi)

pack:
	/* pack needed? */
	cmp %dl, %dh
	je pack_end
pack_do:
	/* load a[k_cnt,m_cnt] */
	movzbl %dl, %eax
	movzbl %cl, %ebx
	mulq %r9, %rax
	mulq %r10, %rbx
	addq %rbx, %rax
	movl (%r8,%rax,4), %r12d
	/* store a[k_cnt,m_cnt] */
	movzbl %dl, %eax
	movzbl %ch, %ebx
	mull %eax %ebx
	movzbl %cl %ebx
	addl %ebx %eax
	movl %r12d, (%r11,%rax,4)
	/* increment */
	addl $1, %cl
	/* eax (next_m_cnt == next_m_len) ? 1 : 0 */
	xorl %eax, %eax
	cmp %cl, %ch
	sete %al
	/* (next_k_cnt++, next_m_cnt = 0) if moving up */
	addl %eax, %edx
	subl $1, %eax
	andl %al, %cl
pack_end:
	/* termination check */
	subl $1, %edi
	jnz avx_kernel_loop
pack_restart:
	/* restart packing until completed */
	addl $1, %edi
	cmp %ecx, %r8d
	jne pack_do:

	/* loop end */

	/* store c */
	movq (%rsp,56), %rax
	movq (%rsp,64), %rbx
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
	popq %r12
	popq %rbx
	popq %rbp
	ret
