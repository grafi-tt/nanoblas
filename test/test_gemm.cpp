#include "nanoblas.hpp"
#include "test_gemm.hpp"
#include "kernel/generic_kernel.h"
#include "lib/reference_kernel.hpp"

using namespace nanoblas;

template<typename FTYPE>
impl_t<FTYPE> get_gemm_impl(const nanoblas_t &nb) {
	return [=](const FTYPE *A, const FTYPE *B, FTYPE *C, size_t M, size_t N, size_t K, size_t lda, size_t ldb, size_t ldc) {
		gemm<FTYPE>(nb, CblasRowMajor, CblasNoTrans, CblasNoTrans, M, N, K, 1, A, lda, B, ldb, 1, C, ldc);
	};
}

bool run_test_set(std::mt19937 gen, nanoblas_t nb) {
	auto sgemm_impl = get_gemm_impl<float>(nb);
	auto dgemm_impl = get_gemm_impl<double>(nb);

	bool s = false;
	s = run_test(gen, sgemm_impl, 200, 200, 200) || s;
	s = run_test(gen, dgemm_impl, 200, 200, 200) || s;
	s = run_test(gen, sgemm_impl, 4, 128, 64) || s;
	s = run_test(gen, dgemm_impl, 4, 128, 64) || s;
	s = run_test(gen, sgemm_impl, 512, 512, 512) || s;
	s = run_test(gen, dgemm_impl, 512, 512, 512) || s;
	s = run_test(gen, sgemm_impl, 200, 200, 200, 300, 300, 300) || s;
	s = run_test(gen, dgemm_impl, 200, 200, 200, 300, 300, 300) || s;
	s = run_test(gen, sgemm_impl, 123, 456, 789) || s;
	s = run_test(gen, dgemm_impl, 123, 456, 789) || s;

	return s;
}

int main() {
	std::mt19937 gen(314159265);

	nanoblas_t nb;
	nb.f32_blk_n_max_len = 128;
	nb.f32_blk_k_max_len = 128;
	nb.f64_blk_n_max_len = 128;
	nb.f64_blk_k_max_len = 128;

	std::cout << "reference 4x4" << std::endl;
	nb.f32_kernel = get_reference_kernel<float, 4, 4>();
	nb.f64_kernel = get_reference_kernel<double, 4, 4>();

	bool s = false;
	s = run_test_set(gen, nb) || s;

	std::cout << "generic 4x4" << std::endl;
	nb.f32_kernel = nanoblas_f32_generic_kernel_4x4;
	nb.f64_kernel = nanoblas_f64_generic_kernel_4x4;

	s = run_test_set(gen, nb) || s;

	std::cout << "generic 6x4" << std::endl;
	nb.f32_kernel = nanoblas_f32_generic_kernel_6x4;
	nb.f64_kernel = nanoblas_f64_generic_kernel_6x4;

	s = run_test_set(gen, nb) || s;

	return s;
}
