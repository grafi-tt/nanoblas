/* ebp: k_len                     (escape to stack; loaded from stack)      */
/* rsp: stack pointer                                                       */
/* rdi: a_pack                                                              */
/* rsi: b_pack                                                              */
/* edx: k_sched_len                                                         */
/* ecx: m_sched_len                                                         */
/* r8:  interval_k_in_a                                                     */
/* r9:  interval_m                                                          */
/* r10: a_next_pack               (loaded from stack)                       */
/* r11: a_next                    (loaded from stack)                       */
/* rax: tmp, at begin and end c   (loaded from stack)                       */
/* rbx: tmp, at begin and end ldc (escape to stack; loaded from stack)      */
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

	/* load */
	movq 32(%rsp), %r10
	movq 40(%rsp), %r11
	movl 48(%rsp), %ebp
	/* load c */
	movq 56(%rsp), %rax
	movq 64(%rsp), %rbx
	vmovaps (%rax), %ymm8
	leaq (%rax,%rbx,4), %rax
	vmovaps (%rax), %ymm9
	leaq (%rax,%rbx,4), %rax
	vmovaps (%rax), %ymm10
	leaq (%rax,%rbx,4), %rax
	vmovaps (%rax), %ymm11
	leaq (%rax,%rbx,4), %rax
	vmovaps (%rax), %ymm12
	leaq (%rax,%rbx,4), %rax
	vmovaps (%rax), %ymm13
	leaq (%rax,%rbx,4), %rax
	vmovaps (%rax), %ymm14
	leaq (%rax,%rbx,4), %rax
	vmovaps (%rax), %ymm15
	leaq (%rax,%rbx,4), %rax

	/* loop start */
.align 4
avx_kernel_loop:

	/* unroll 0 */
.macro unroll offset
	/* ymm1 = (a'[8:4], a'[8:4]); ymm0 = (a'[4:0], a'[4:0]) */
	vmovaps \offset(%rdi), %ymm0
	vperm2f128 $0x11, %ymm0, %ymm0, %ymm1
	vperm2f128 $0x00, %ymm0, %ymm0, %ymm0
	/* ymm4 = b' */
	vmovaps \offset(%rsi), %ymm4
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
.endm
	unroll 0
	/* termination check */
	subl $1, %ebp
	jz pack_restart

	/* unroll 1 */
	unroll 32
	/* proceed pointers */
	leaq 64(%rdi), %rdi
	leaq 64(%rsi), %rsi

pack:
	/* pack needed? */
	cmp %dh, %dl
	je pack_end
pack_do:
	/* load a[k_cnt,m_cnt] */
	movzbl %dh, %eax
	movzbl %ch, %ebx
	mulq %r8, %rax
	mulq %r9, %rbx
	addq %rbx, %rax
	movl (%r11,%rax,4), %r12d
	/* store a[k_cnt,m_cnt] */
	movzbl %dh, %eax
	movzbl %cl, %ebx
	mull %eax, %ebx
	movzbl %ch, %eax
	addl %ebx, %eax
	movl %r12d, (%r10,%rax,4)
	/* increment */
	addb $1, %ch
	/* eax (next_m_cnt == next_m_len) ? 1 : 0 */
	cmp %ch, %cl
	sete %al
	/* (next_k_cnt++, next_m_cnt = 0) if moving up */
	addb %al, %dh
	subb $1, %al
	andb %al, %ch
pack_end:
	/* termination check */
	subl $1, %ebp
	jnz avx_kernel_loop
pack_restart:
	/* restart packing until completed */
	addl $1, %ebp
	cmp %dh, %dl
	jne pack_do

	/* loop end */

	/* store c */
	movq 56(%rsp), %rax
	movq 64(%rsp), %rbx
	vmovaps %ymm8, (%rbx)
	leaq (%rax,%rbx,4), %rax
	vmovaps %ymm9, (%rax)
	leaq (%rax,%rbx,4), %rax
	vmovaps %ymm10, (%rax)
	leaq (%rax,%rbx,4), %rax
	vmovaps %ymm11, (%rax)
	leaq (%rax,%rbx,4), %rax
	vmovaps %ymm12, (%rax)
	leaq (%rax,%rbx,4), %rax
	vmovaps %ymm13, (%rax)
	leaq (%rax,%rbx,4), %rax
	vmovaps %ymm14, (%rax)
	leaq (%rax,%rbx,4), %rax
	vmovaps %ymm15, (%rax)
	leaq (%rax,%rbx,4), %rax

	/* return */
	popq %r12
	popq %rbx
	popq %rbp
	ret
