#include "kernel/x86_64/avx_kernel.h"
#include "kernel/generic_kernel.h"
#include "nanoblas.h"
#include "SFMT/SFMT.h"

int main() {
	sfmt_t sfmt;
	sfmt_init_gen_rand(&sfmt, 314159265);
	float A[512*512];
	float B[512*512];
	float C[512*512];
	float D[512*512];

	for (int i = 0; i < 512*512; i++) {
		A[i] = sfmt_genrand_real1(&sfmt);
		B[i] = sfmt_genrand_real1(&sfmt);
		C[i] = 0;
		D[i] = 0;
	}
	for (int m = 0; m < 512; m++)
		for (int n = 0; n < 512; n++)
			for (int k = 0; k < 512; k++)
				C[512*m+n] += A[512*m+k] * B[512*k+n];
	printf("%f %f\n", C[0], C[12345]);

	nanoblas_t nb;
	nanoblas_init(&nb);
	printf("%d\n", nb.f32_kernel.fun == nanoblas_f32_avx_kernel_fun);

	nb.f32_kernel = nanoblas_f32_generic_kernel_4x4;
	nanoblas_sgemm(&nb, CblasRowMajor, CblasNoTrans, CblasNoTrans, 512, 512, 512, 1, A, 512, B, 512, 1, D, 512);
	printf("%f %f\n", D[1], D[12345]);

	return 0;
}
