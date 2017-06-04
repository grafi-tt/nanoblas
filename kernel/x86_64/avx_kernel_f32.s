// edi: k_len
// esi: next_k_len
// edx: next_m_len
// ecx: next_k_cnt
// r8d: next_m_cnt
// r9: a_pack_next
// r10d: transa (escape to stack; load from stack)
// r11: lda (load from stack)
// r12: a (load from stack)
// r13: a_pack_cur (escape to stack; load from stack)
// r14: b_pack_cur (escape to stack; load from stack)
// rax: ldc (load from stack) or tmp
// rbx: c (escape to stack; load from stack) or tmp
// r15: reserved
// rbp: frame pointer
// rsp: stack pointer

.macro pack_load
// (eax, ebx) = (esi, edx) if not transa
// (ebx, eax) = (esi, edx) if transa
movl %esi %ebx
xorl %edx %ebx
andl %r10d %ebx
movl %ebx %eax
xorl %esi %eax
xorl %edx %ebx
// load a[m_len,k_len]
mulq %r11 %rax
addq %rbx %rax
movq (%r12,%rax,4) %rbx
.endm

.macro pack_store suffix
// store a[m_len,k_len]
movl %ecx %eax
mull %edx %eax
addl %r8d %eax
movq %rbx (%r13,%rax,4)
addl $1 %r8d
cmp %r8d %edx
jnz pack_store_end\suffix
xorl %r8d %r8d
addl $1 %ecx
pack_store_end\suffix
.endm

.macro unroll offset
	/* ymm9 = (a'[8:4], a'[8:4]); ymm8 = (a'[4:0], a'[4:0]) */
	vmovaps \offset(%[a_pack_cur_ptr]) %ymm8
	vperm2f128 %ymm8 %ymm8 %ymm9 0x11
	vperm2f128 %ymm8 %ymm8 %ymm8 0x00
	/* ymm12 = b' */
	vmovpas \offset(%[b_pack_cur_ptr]) %ymm12
	/* c[0,] += a'[0] * b'; c[1,] += a[1] * b' */
	vshufps %ymm8 %ymm10 $0x00
	vshufps %ymm8 %ymm11 $0x11
	vmulps %ymm10 %ymm12 %ymm14
	vmulps %ymm11 %ymm12 %ymm15
	vaddps %ymm0 %ymm14 %ymm0
	vaddps %ymm1 %ymm15 %ymm1
	/* c[2,] += a'[2] * b'; c[3,] += a'[3] * b' */
	vshufps %ymm8 %ymm10 $0x22
	vshufps %ymm8 %ymm11 $0x33
	vmulps %ymm10 %ymm12 %ymm14
	vmulps %ymm11 %ymm12 %ymm15
	vaddps %ymm0 %ymm14 %ymm2
	vaddps %ymm1 %ymm15 %ymm3
	/* c[4,] += a'[4] * b'; c[5,] += a'[5] * b' */
	vshufps %ymm8 %ymm10 $0x44
	vshufps %ymm8 %ymm11 $0x55
	vmulps %ymm10 %ymm12 %ymm14
	vmulps %ymm11 %ymm12 %ymm15
	vaddps %ymm0 %ymm14 %ymm4
	vaddps %ymm1 %ymm15 %ymm5
	/* c[6,] += a'[6] * b'; c[7,] += a'[7] * b' */
	vshufps %ymm8 %ymm10 $0x66
	vshufps %ymm8 %ymm11 $0x77
	vmulps %ymm10 %ymm12 %ymm14
	vmulps %ymm11 %ymm12 %ymm15
	vaddps %ymm0 %ymm14 %ymm6
	vaddps %ymm1 %ymm15 %ymm7
.endm

.section .text
.globl avx_kernel_f32
avx_kernel_f32:
	/* load c */
	vmovaps (%rbx) %ymm0
	leaq (%rbx,%rax,4) %rbx
	vmovaps (%rbx) %ymm1
	leaq (%rbx,%rax,4) %rbx
	vmovaps (%rbx) %ymm2
	leaq (%rbx,%rax,4) %rbx
	vmovaps (%rbx) %ymm3
	leaq (%rbx,%rax,4) %rbx
	vmovaps (%rbx) %ymm4
	leaq (%rbx,%rax,4) %rbx
	vmovaps (%rbx) %ymm5
	leaq (%rbx,%rax,4) %rbx
	vmovaps (%rbx) %ymm6
	leaq (%rbx,%rax,4) %rbx
	vmovaps (%rbx) %ymm7
	leaq (%rbx,%rax,4) %rbx
	/* reset c_ptr */
	leaq ($0,%rbx,8) %r14
	leaq (%rbx,%r14,-4) %rbx

	/* loop start */
avx_kernel_loop:
	/* unroll 0 */
	unroll $0
	/* termination check */
	subd $1, %edi
	jz avx_kernel_loop_half_end%=
	/* pack load step */
	pack_load
	/* unroll 1 */
	unroll $1
	/* proceed pointers */
	leaq 64(%[a_pack_cur_ptr])
	leaq 64(%[b_pack_cur_ptr])
	/* pack store step */
	pack_store loop
	/* termination check */
	subd $1, %edi
	jnz avx_kernel_loop
avx_kernel_loop_end:

	/* store c */
	vmovaps %ymm0 %rbx)
	leaq (%rbx,%rax,4) %rbx
	vmovaps %ymm1 %rbx)
	leaq (%rbx,%rax,4) %rbx
	vmovaps %ymm2 %rbx)
	leaq (%rbx,%rax,4) %rbx
	vmovaps %ymm3 %rbx)
	leaq (%rbx,%rax,4) %rbx
	vmovaps %ymm4 %rbx)
	leaq (%rbx,%rax,4) %rbx
	vmovaps %ymm5 %rbx)
	leaq (%rbx,%rax,4) %rbx
	vmovaps %ymm6 %rbx)
	leaq (%rbx,%rax,4) %rbx
	vmovaps %ymm7 %rbx)
	leaq (%rbx,%rax,4) %rbx
