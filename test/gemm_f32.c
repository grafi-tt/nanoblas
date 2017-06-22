#include "kernel/x86_64/avx_kernel.h"
#include "kernel/generic_kernel.h"
#include "nanoblas.h"
#include "SFMT/SFMT.h"

#define M 512
#define N 512
#define K 512

int main() {
	sfmt_t sfmt;
	sfmt_init_gen_rand(&sfmt, 314159265);
	float A[M*K];
	float B[K*N];
	float C[M*N];
	float D[M*N];

	for (int i = 0; i < M*K; i++) {
		A[i] = sfmt_genrand_real1(&sfmt);
	}
	for (int i = 0; i < K*N; i++) {
		B[i] = sfmt_genrand_real1(&sfmt);
	}
	for (int i = 0; i < M*N; i++) {
		C[i] = 0;
		D[i] = 0;
	}
	for (int m = 0; m < M; m++)
		for (int n = 0; n < N; n++)
			for (int k = 0; k < K; k++)
				C[N*m+n] += A[K*m+k] * B[N*k+n];
	printf("%f %f\n", C[1], C[300]);

	nanoblas_t nb;
	nanoblas_init(&nb);
	printf("%d\n", nb.f32_kernel.fun == nanoblas_f32_avx_kernel_fun);

	nb.f32_kernel = nanoblas_f32_generic_kernel_4x4;
	nanoblas_sgemm(&nb, CblasRowMajor, CblasNoTrans, CblasNoTrans, M, N, K, 1, A, K, B, N, 1, D, N);
	printf("%f %f\n", D[1], D[300]);

	return 0;
}
