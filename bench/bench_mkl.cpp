#include <mkl.h>
#include <mkl_cblas.h>
#include "bench.hpp"

void mkl_sgemm_op(const float *A, const float *B, float *C,
		size_t M, size_t N, size_t K, size_t lda, size_t ldb, size_t ldc) {
	cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, M, N, K, 1, A, lda, B, ldb, 1, C, ldc);
}

int main() {
	mkl_set_num_threads(1);
	run_sgemm_test_set("mkl", mkl_sgemm_op);
	return 0;
}
