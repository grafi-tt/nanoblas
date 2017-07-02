#include <nanoblas.hpp>
#include <kernel/x86_64/avx_kernel.hpp>
#include "bench.hpp"

template<typename FTYPE>
impl_t<FTYPE> get_gemm_impl(const nanoblas_t &nb) {
	return [=](const FTYPE *A, const FTYPE *B, FTYPE *C, size_t M, size_t N, size_t K, size_t lda, size_t ldb, size_t ldc) {
		nanoblas::gemm<FTYPE>(nb, CblasRowMajor, CblasNoTrans, CblasNoTrans, M, N, K, 1, A, lda, B, ldb, 1, C, ldc);
	};
}

int main() {
	nanoblas_t nb;
	nanoblas_init(&nb);
	nb.f32_kernel = nanoblas::get_avx_kernel();

	/*
	nb.f32_blk_k_max_len = 128;
	nb.f32_blk_n_max_len = 128;
	run_sgemm_test_set("nanoblas avx k128 n128", get_gemm_impl<float>(nb));
	*/

	nb.f32_blk_k_max_len = 256;
	nb.f32_blk_n_max_len = 128;
	run_sgemm_test_set("nanoblas avx k256 n128", get_gemm_impl<float>(nb));

	/*
	nb.f32_blk_k_max_len = 128;
	nb.f32_blk_n_max_len = 256;
	run_sgemm_test_set("nanoblas avx k128 n256", get_gemm_impl<float>(nb));

	nb.f32_blk_k_max_len = 256;
	nb.f32_blk_n_max_len = 256;
	run_sgemm_test_set("nanoblas avx k256 n256", get_gemm_impl<float>(nb));
	*/
}
