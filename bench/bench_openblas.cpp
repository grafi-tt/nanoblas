#include <openblas/cblas.h>
#include "bench.hpp"

void openblas_sgemm_op(const float *A, const float *B, float *C,
		size_t M, size_t N, size_t K, size_t lda, size_t ldb, size_t ldc) {
	cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, M, N, K, 1, A, lda, B, ldb, 1, C, ldc);
}

int main() {
	openblas_set_num_threads(1);
	run_sgemm_test_set("openblas", openblas_sgemm_op);
	return 0;
}
