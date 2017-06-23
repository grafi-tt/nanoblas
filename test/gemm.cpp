#include "nanoblas.h"
#include "kernel/generic_kernel.h"
#include "gemm.hpp"

void sgemm_impl(const float *A, const float *B, float *C, size_t M, size_t N, size_t K) {
	nanoblas_t nb;
	nanoblas_init(&nb);
	nb.f32_kernel = nanoblas_f32_generic_kernel_4x4;
	nanoblas_sgemm(&nb, CblasRowMajor, CblasNoTrans, CblasNoTrans, M, N, K, 1, A, K, B, N, 1, C, N);
}

void dgemm_impl(const double *A, const double *B, double *C, size_t M, size_t N, size_t K) {
	nanoblas_t nb;
	nanoblas_init(&nb);
	nb.f32_kernel = nanoblas_f32_generic_kernel_4x4;
	nanoblas_dgemm(&nb, CblasRowMajor, CblasNoTrans, CblasNoTrans, M, N, K, 1, A, K, B, N, 1, C, N);
}

int main() {
	std::mt19937 gen(314159265);

	bool s = false;
	s = run_test(gen, &sgemm_impl, 8, 128, 64) || s;
	s = run_test(gen, &dgemm_impl, 8, 128, 64) || s;

	return s;
}
