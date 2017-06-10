/* ebp: sched_k_len               (escape to stack; loaded from stack)      */
/* rsp: stack pointer                                                       */
/* rdi: a_pack                                                              */
/* rsi: b_pack                                                              */
/* edx: k_len -> sched_m_len -> k_len                                       */
/* ecx: m_sub_len -> sched_m_cnt                                            */
/* r8:  n_sub_len -> a_next                                                 */
/* r9:  c -> a_next_pack -> c                                               */
/* r10: ldc -> proceed_k -> ldc                                             */
/* r11: interval_m                                                          */
/* r12: k_len                     (escape to stack)                         */
/* rax: tmp                                                                 */
/* rbx: tmp                       (escape to stack)                         */
/* r13: reserved                                                            */
/* r14: reserved                                                            */
/* r15: reserved                                                            */

.section .text

.align 4
.globl avx_kernel_f32
avx_kernel_f32:
	/* escape */
	pushq %rbp
	pushq %rbx
	pushq %r12
	pushq %r9
	/* create mask template */
	mov $0x10000000, %eax
	sll $23, %ecx
	add %ecx, %eax
	pushl %eax
	vbroadcastps (%rsp), %xmm0
	movaps %xmm0, %xmm1
	/* shift template */
	leal 2(%r8,%r8,4), %rax
	jmp short, %rax
	/* 40 bytes until here */
	/* loop */
	psrldq $4, %xmm0
	psrldq $4, %xmm0
	psrldq $4, %xmm0
	psrldq $4, %xmm0
	psrldq $4, %xmm1
	psrldq $4, %xmm1
	psrldq $4, %xmm1
	psrldq $4, %xmm1
	/* 80 bytes until here */
	/* cocnat template */
	vinsertf128 $0, $ymm1, $ymm0, $ymm0
	/* backup template */
	movaps %ymm0, %ymm5

	/* mask updater for n-dim */
	/* IEEE754 trick */
	mov $1, %eax
	sra $23, %eax
	movl %eax, (%rsp)
	vbroadcastps (%rsp), %ymm1

	/* ldc */
	movq 36(%rsp), %r10
	/* load c  */
	vmaskmovps %ymm0, (%rax), %ymm8
	vaddps %ymm1, %ymm0, %ymm0
	vaddps %ymm1, %ymm1, %ymm1
	leaq (%r9,%r10,4), %r9
	vmaskmovps %ymm0, (%r9), %ymm9
	vaddps %ymm1, %ymm0, %ymm0
	vaddps %ymm1, %ymm1, %ymm1
	leaq (%r9,%r10,4), %r9
	vmaskmovps %ymm0, (%r9), %ymm10
	vaddps %ymm1, %ymm0, %ymm0
	vaddps %ymm1, %ymm1, %ymm1
	leaq (%r9,%r10,4), %r9
	vmaskmovps %ymm0, (%r9), %ymm11
	vaddps %ymm1, %ymm0, %ymm0
	vaddps %ymm1, %ymm1, %ymm1
	leaq (%r9,%r10,4), %r9
	vmaskmovps %ymm0, (%r9), %ymm12
	vaddps %ymm1, %ymm0, %ymm0
	vaddps %ymm1, %ymm1, %ymm1
	leaq (%r9,%r10,4), %r9
	vmaskmovps %ymm0, (%r9), %ymm13
	vaddps %ymm1, %ymm0, %ymm0
	vaddps %ymm1, %ymm1, %ymm1
	leaq (%r9,%r10,4), %r9
	vmaskmovps %ymm0, (%r9), %ymm14
	leaq (%r9,%r10,4), %r9
	vmaskmovps %ymm0, (%r9), %ymm15
	vaddps %ymm1, %ymm0, %ymm0
	vaddps %ymm1, %ymm1, %ymm1
	leaq (%r9,%r10,4), %r9

	/* load sched state */
	mov 44(%rsp),%r8
	test %r8, %r8
	jz avx_kernel_nopack_loop
	/* adjust and escape k_len */
	mov 40(%r8), %ebp
	leal (%edx,%ebp,-8), %r12d
	/* load vals */
	mov 44(%r8), %edx
	mov 32(%r8), %r11
	mov 24(%r8), %r10
	mov 8(%r8), %r9
	mov (%r8), %r8
	/* calculate proceed_k */
	leaq (%r10,%r11,-8), %r10

.align 4
avx_kernel_pack_loop:
	/* ymm1 = (a'[8:4], a'[8:4]); ymm0 = (a'[4:0], a'[4:0]) */
	vmovaps (%rdi), %ymm0
	vperm2f128 $0x11, %ymm0, %ymm0, %ymm1
	vperm2f128 $0x00, %ymm0, %ymm0, %ymm0
	/* ymm4 = b' */
	vmovaps (%rsi), %ymm4
	/* c[0,] += a'[0] * b'; c[1,] += a[1] * b' */
	vshufps $0x00, %ymm0, %ymm0, %ymm2
	vshufps $0x55, %ymm0, %ymm0, %ymm3
	vmulps %ymm2, %ymm4, %ymm6
	vmulps %ymm3, %ymm4, %ymm7
	vaddps %ymm8, %ymm6, %ymm8
	vaddps %ymm9, %ymm7, %ymm9
	/* c[2,] += a'[2] * b'; c[3,] += a'[3] * b' */
	vshufps $0xAA, %ymm0, %ymm0, %ymm2
	vshufps $0xFF, %ymm0, %ymm0, %ymm3
	vmulps %ymm2, %ymm4, %ymm6
	vmulps %ymm3, %ymm4, %ymm7
	vaddps %ymm10, %ymm6, %ymm10
	vaddps %ymm11, %ymm7, %ymm11
	/* c[4,] += a'[4] * b'; c[5,] += a'[5] * b' */
	vshufps $0x00, %ymm1, %ymm1, %ymm2
	vshufps $0x55, %ymm1, %ymm1, %ymm3
	vmulps %ymm2, %ymm4, %ymm6
	vmulps %ymm3, %ymm4, %ymm7
	vaddps %ymm12, %ymm6, %ymm12
	vaddps %ymm13, %ymm7, %ymm13
	/* c[6,] += a'[6] * b'; c[7,] += a'[7] * b' */
	vshufps $0xAA, %ymm1, %ymm1, %ymm2
	vshufps $0xFF, %ymm1, %ymm1, %ymm3
	vmulps %ymm2, %ymm4, %ymm6
	vmulps %ymm3, %ymm4, %ymm7
	vaddps %ymm14, %ymm6, %ymm14
	vaddps %ymm15, %ymm7, %ymm15
	/* increment a_pack and b_pack */
	leaq 32(%rdi), %rdi
	leaq 32(%rsi), %rsi

	/* scheduled pack */
	xorl %eax, %eax
	cmpl %edx, %ecx
	jge after_load
		movl (%r8), %eax
	after_load:
	movl %eax, (%r9)
	leaq (%r8,%r11,4), %r8
	addq $4, %r9
	add $1, %edx
	xorl %eax, %eax
	cmp $8, %edx
	sete %al
	addq $-1, %rax
	andl %eax, %ecx
	notq %rax, %rax
	movq %r10, %rbx
	andq %rax, %rbx
	leaq (%r8,%rbx,4), %r8
	addl %eax, %ebp
	jnz avx_kernel_pack_loop
	/* restore k_len */
	mov %r12d, %edx

.align 4
avx_kernel_nopack_loop:
	/* ymm1 = (a'[8:4], a'[8:4]); ymm0 = (a'[4:0], a'[4:0]) */
	vmovaps (%rdi), %ymm0
	vperm2f128 $0x11, %ymm0, %ymm0, %ymm1
	vperm2f128 $0x00, %ymm0, %ymm0, %ymm0
	/* ymm4 = b' */
	vmovaps (%rsi), %ymm4
	/* c[0,] += a'[0] * b'; c[1,] += a[1] * b' */
	vshufps $0x00, %ymm0, %ymm0, %ymm2
	vshufps $0x55, %ymm0, %ymm0, %ymm3
	vmulps %ymm2, %ymm4, %ymm6
	vmulps %ymm3, %ymm4, %ymm7
	vaddps %ymm8, %ymm6, %ymm8
	vaddps %ymm9, %ymm7, %ymm9
	/* c[2,] += a'[2] * b'; c[3,] += a'[3] * b' */
	vshufps $0xAA, %ymm0, %ymm0, %ymm2
	vshufps $0xFF, %ymm0, %ymm0, %ymm3
	vmulps %ymm2, %ymm4, %ymm6
	vmulps %ymm3, %ymm4, %ymm7
	vaddps %ymm10, %ymm6, %ymm10
	vaddps %ymm11, %ymm7, %ymm11
	/* c[4,] += a'[4] * b'; c[5,] += a'[5] * b' */
	vshufps $0x00, %ymm1, %ymm1, %ymm2
	vshufps $0x55, %ymm1, %ymm1, %ymm3
	vmulps %ymm2, %ymm4, %ymm6
	vmulps %ymm3, %ymm4, %ymm7
	vaddps %ymm12, %ymm6, %ymm12
	vaddps %ymm13, %ymm7, %ymm13
	/* c[6,] += a'[6] * b'; c[7,] += a'[7] * b' */
	vshufps $0xAA, %ymm1, %ymm1, %ymm2
	vshufps $0xFF, %ymm1, %ymm1, %ymm3
	vmulps %ymm2, %ymm4, %ymm6
	vmulps %ymm3, %ymm4, %ymm7
	vaddps %ymm14, %ymm6, %ymm14
	vaddps %ymm15, %ymm7, %ymm15
	/* increment a_pack and b_pack */
	leaq 32(%rdi), %rdi
	leaq 32(%rsi), %rsi
	subl $8, %edx
	jnz avx_kernel_nopack_loop
avx_kernel_loop_end:

	/* load template */
	vbroadcastps (%rsp), %ymm1
	addq $4, %rsp
	/* c, ldc */
	popq %r9
	movq 24(%rsp), %r10
	/* store c */
	vmaskmovps %ymm5, (%r9), %ymm8
	vaddps %ymm1, %ymm5, %ymm5
	vaddps %ymm1, %ymm1, %ymm1
	leaq (%r9,%r10,4), %r9
	vmaskmovps %ymm5, (%r9), %ymm9
	vaddps %ymm1, %ymm5, %ymm5
	vaddps %ymm1, %ymm1, %ymm1
	leaq (%r9,%r10,4), %r9
	vmaskmovps %ymm5, (%r9), %ymm10
	vaddps %ymm1, %ymm5, %ymm5
	vaddps %ymm1, %ymm1, %ymm1
	leaq (%r9,%r10,4), %r9
	vmaskmovps %ymm5, (%r9), %ymm11
	vaddps %ymm1, %ymm5, %ymm5
	vaddps %ymm1, %ymm1, %ymm1
	leaq (%r9,%r10,4), %r9
	vmaskmovps %ymm5, (%r9), %ymm12
	vaddps %ymm1, %ymm5, %ymm5
	vaddps %ymm1, %ymm1, %ymm1
	leaq (%r9,%r10,4), %r9
	vmaskmovps %ymm5, (%r9), %ymm13
	vaddps %ymm1, %ymm5, %ymm5
	vaddps %ymm1, %ymm1, %ymm1
	leaq (%r9,%r10,4), %r9
	vmaskmovps %ymm5, (%r9), %ymm14
	leaq (%r9,%r10,4), %r9
	vmaskmovps %ymm5, (%r9), %ymm15
	vaddps %ymm1, %ymm5, %ymm5
	vaddps %ymm1, %ymm1, %ymm1
	leaq (%r9,%r10,4), %r9

	/* return */
	popq %r12
	popq %rbx
	popq %rbp
	ret
