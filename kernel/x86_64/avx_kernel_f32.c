static void avx_kernel(
		int len, int a_pack_next_cnt, float *restrict a_pack_next,
		size_t lda, float *restrict a, float *restrict a_pack_cur,
		float *restrict b_pack_cur,
		size_t ldc, float *restrict c) {

	__asm__(
		/* load c */
		"vmovaps (%[c_ptr]) %%ymm0\n\t"
		"leaq (%[c_ptr],%[ldc],4) %[c_ptr]\n\t"
		"vmovaps (%[c_ptr]) %%ymm1\n\t"
		"leaq (%[c_ptr],%[ldc],4) %[c_ptr]\n\t"
		"vmovaps (%[c_ptr]) %%ymm2\n\t"
		"leaq (%[c_ptr],%[ldc],4) %[c_ptr]\n\t"
		"vmovaps (%[c_ptr]) %%ymm3\n\t"
		"leaq (%[c_ptr],%[ldc],4) %[c_ptr]\n\t"
		"vmovaps (%[c_ptr]) %%ymm4\n\t"
		"leaq (%[c_ptr],%[ldc],4) %[c_ptr]\n\t"
		"vmovaps (%[c_ptr]) %%ymm5\n\t"
		"leaq (%[c_ptr],%[ldc],4) %[c_ptr]\n\t"
		"vmovaps (%[c_ptr]) %%ymm6\n\t"
		"leaq (%[c_ptr],%[ldc],4) %[c_ptr]\n\t"
		"vmovaps (%[c_ptr]) %%ymm7\n\t"
		"leaq (%[c_ptr],%[ldc],4) %[c_ptr]\n\t"
		/* reset c_ptr */
		"leaq ($0,%[c_ptr],8) %%r15\n\t"
		"leaq (%[c_ptr],%%r15,-4) %[c_ptr]\n\t"

		/* loop start */
		"avx_kernel_loop%=:"

		/* unroll 0 */

		/* ymm9 = (a'[8:4], a'[8:4]); ymm8 = (a'[4:0], a'[4:0]) */
		"vmovaps (%[a_pack_cur_ptr]) %%ymm8\n\t"
		"vperm2f128 %%ymm8 %%ymm8 %%ymm9 0x11\n\t"
		"vperm2f128 %%ymm8 %%ymm8 %%ymm8 0x00\n\t"
		/* ymm12 = b' */
		"vmovpas (%[b_pack_cur_ptr]) %%ymm12\n\t"
		/* c[0,] += a'[0] * b'; c[1,] += a[1] * b' */
		"vshufps %%ymm8 %%ymm10 $0x00\n\t"
		"vshufps %%ymm8 %%ymm11 $0x11\n\t"
		"vmulps %%ymm10 %%ymm12 %%ymm14\n\t"
		"vmulps %%ymm11 %%ymm12 %%ymm15\n\t"
		"vaddps %%ymm0 %%ymm14 %%ymm0\n\t"
		"vaddps %%ymm1 %%ymm15 %%ymm1\n\t"
		/* c[2,] += a'[2] * b'; c[3,] += a'[3] * b' */
		"vshufps %%ymm8 %%ymm10 $0x22\n\t"
		"vshufps %%ymm8 %%ymm11 $0x33\n\t"
		"vmulps %%ymm10 %%ymm12 %%ymm14\n\t"
		"vmulps %%ymm11 %%ymm12 %%ymm15\n\t"
		"vaddps %%ymm0 %%ymm14 %%ymm2\n\t"
		"vaddps %%ymm1 %%ymm15 %%ymm3\n\t"
		/* c[4,] += a'[4] * b'; c[5,] += a'[5] * b' */
		"vshufps %%ymm8 %%ymm10 $0x44\n\t"
		"vshufps %%ymm8 %%ymm11 $0x55\n\t"
		"vmulps %%ymm10 %%ymm12 %%ymm14\n\t"
		"vmulps %%ymm11 %%ymm12 %%ymm15\n\t"
		"vaddps %%ymm0 %%ymm14 %%ymm4\n\t"
		"vaddps %%ymm1 %%ymm15 %%ymm5\n\t"
		/* c[6,] += a'[6] * b'; c[7,] += a'[7] * b' */
		"vshufps %%ymm8 %%ymm10 $0x66\n\t"
		"vshufps %%ymm8 %%ymm11 $0x77\n\t"
		"vmulps %%ymm10 %%ymm12 %%ymm14\n\t"
		"vmulps %%ymm11 %%ymm12 %%ymm15\n\t"
		"vaddps %%ymm0 %%ymm14 %%ymm6\n\t"
		"vaddps %%ymm1 %%ymm15 %%ymm7\n\t"

		/* termination check */
		"subd $1, %[len]\n\t"
		"jz avx_kernel_loop_half_end%=\n\t"

		/* unroll 1 */

		/* ymm9 = (a'[8:4], a'[8:4]); ymm8 = (a'[4:0], a'[4:0]) */
		"vmovaps 32(%[a_pack_cur_ptr]) %%ymm8\n\t"
		"vperm2f128 %%ymm8 %%ymm8 %%ymm9 $0x11\n\t"
		"vperm2f128 %%ymm8 %%ymm8 %%ymm8 $0x00\n\t"
		/* ymm12 = b' */
		"vmovpas 32(%[b_pack_cur_ptr]) %%ymm12\n\t"
		/* c[0,] += a'[0] * b'; c[1,] += a[1] * b' */
		"vshufps %%ymm8 %%ymm10 $0x00\n\t"
		"vshufps %%ymm8 %%ymm11 $0x11\n\t"
		"vmulps %%ymm10 %%ymm12 %%ymm14\n\t"
		"vmulps %%ymm11 %%ymm12 %%ymm15\n\t"
		"vaddps %%ymm0 %%ymm14 %%ymm0\n\t"
		"vaddps %%ymm1 %%ymm15 %%ymm1\n\t"
		/* c[2,] += a'[2] * b'; c[3,] += a'[3] * b' */
		"vshufps %%ymm8 %%ymm10 $0x22\n\t"
		"vshufps %%ymm8 %%ymm11 $0x33\n\t"
		"vmulps %%ymm10 %%ymm12 %%ymm14\n\t"
		"vmulps %%ymm11 %%ymm12 %%ymm15\n\t"
		"vaddps %%ymm0 %%ymm14 %%ymm2\n\t"
		"vaddps %%ymm1 %%ymm15 %%ymm3\n\t"
		/* c[4,] += a'[4] * b'; c[5,] += a'[5] * b' */
		"vshufps %%ymm8 %%ymm10 $0x44\n\t"
		"vshufps %%ymm8 %%ymm11 $0x55\n\t"
		"vmulps %%ymm10 %%ymm12 %%ymm14\n\t"
		"vmulps %%ymm11 %%ymm12 %%ymm15\n\t"
		"vaddps %%ymm0 %%ymm14 %%ymm4\n\t"
		"vaddps %%ymm1 %%ymm15 %%ymm5\n\t"
		/* c[6,] += a'[6] * b'; c[7,] += a'[7] * b' */
		"vshufps %%ymm8 %%ymm10 $0x66\n\t"
		"vshufps %%ymm8 %%ymm11 $0x77\n\t"
		"vmulps %%ymm10 %%ymm12 %%ymm14\n\t"
		"vmulps %%ymm11 %%ymm12 %%ymm15\n\t"
		"vaddps %%ymm0 %%ymm14 %%ymm6\n\t"
		"vaddps %%ymm1 %%ymm15 %%ymm7\n\t"

		/* proceed pointers */
		"leaq 64(%[a_pack_cur_ptr])\n\t"
		"leaq 64(%[b_pack_cur_ptr])\n\t"
		/* pack step */
#include "pack.c"

		/* termination check */
		"subd $1, %[len]\n\t"
		"jnz avx_kernel_loop%=\n\t"
		":avx_kernel_loop_end%=\n\t"

		/* store c */
		"vmovaps %%ymm0 %[c_ptr])\n\t"
		"leaq (%[c_ptr],%[ldc],4) %[c_ptr]\n\t"
		"vmovaps %%ymm1 %[c_ptr])\n\t"
		"leaq (%[c_ptr],%[ldc],4) %[c_ptr]\n\t"
		"vmovaps %%ymm2 %[c_ptr])\n\t"
		"leaq (%[c_ptr],%[ldc],4) %[c_ptr]\n\t"
		"vmovaps %%ymm3 %[c_ptr])\n\t"
		"leaq (%[c_ptr],%[ldc],4) %[c_ptr]\n\t"
		"vmovaps %%ymm4 %[c_ptr])\n\t"
		"leaq (%[c_ptr],%[ldc],4) %[c_ptr]\n\t"
		"vmovaps %%ymm5 %[c_ptr])\n\t"
		"leaq (%[c_ptr],%[ldc],4) %[c_ptr]\n\t"
		"vmovaps %%ymm6 %[c_ptr])\n\t"
		"leaq (%[c_ptr],%[ldc],4) %[c_ptr]\n\t"
		"vmovaps %%ymm7 %[c_ptr])\n\t"
		"leaq (%[c_ptr],%[ldc],4) %[c_ptr]\n\t"

		: [len] "r" (len), [a_pack_next_cnt] "+r",
		  [a_pack_cur_ptr] "+r" (a_pack_cur),
		  [b_pack_cur_ptr] "+r" (b_pack_cur)
		: [a_pack_next_ptr] "r",
		  [lda] "r" (lda), [a_ptr] "r" (a),
		  [ldc] "r" (ldc), [c_ptr] "r" (c)
		: "%ymm0", "%ymm1", "%ymm2",  "%ymm3",  "%ymm4",  "%ymm5",  "%ymm6",  "%ymm7",
		  "%ymm8", "%ymm9", "%ymm10", "%ymm11", "%ymm12", "%ymm13", "%ymm14", "%ymm15",
		  "%r14", "r15"
	);
}
