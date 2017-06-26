#include "nanoblas.hpp"
#include "gemm.hpp"
#include "kernel/generic_kernel.h"
#include "lib/reference_kernel.hpp"

template<typename FTYPE>
void max_sched_len_fun1(int len, int *a_max_sched_len, int *b_max_sched_len) {
	if (a_max_sched_len) *a_max_sched_len = len / RK<FTYPE>::reference_kernel.m_slice_len;
	if (b_max_sched_len) *b_max_sched_len = len / RK<FTYPE>::reference_kernel.n_slice_len;
}

template<typename FTYPE>
impl_t<FTYPE> get_gemm_impl(const nanoblas_t &nb) {
	return [=](const FTYPE *A, const FTYPE *B, FTYPE *C, size_t M, size_t N, size_t K) {
		nanoblas::gemm<FTYPE>(nb, CblasRowMajor, CblasNoTrans, CblasNoTrans, M, N, K, 1, A, K, B, N, 1, C, N);
	};
}

bool run_test_set(std::mt19937 gen, nanoblas_t nb) {
	auto sgemm_impl = get_gemm_impl<float>(nb);
	auto dgemm_impl = get_gemm_impl<double>(nb);

	bool s = false;
	s = run_test(gen, sgemm_impl, 4, 128, 64) || s;
	s = run_test(gen, dgemm_impl, 4, 128, 64) || s;
	s = run_test(gen, sgemm_impl, 512, 512, 512) || s;
	s = run_test(gen, dgemm_impl, 512, 512, 512) || s;

	return s;
}

int main() {

	std::mt19937 gen(314159265);

	nanoblas_t nb;
	nb.f32_blk_n_max_len = 128;
	nb.f32_blk_k_max_len = 128;
	nb.f64_blk_n_max_len = 128;
	nb.f64_blk_k_max_len = 128;

	RK<float>::reference_kernel.m_slice_len = 4;
	RK<float>::reference_kernel.n_slice_len = 4;
	RK<float>::reference_kernel.max_sched_len_fun = max_sched_len_fun1<float>;
	RK<double>::reference_kernel.m_slice_len = 4;
	RK<double>::reference_kernel.n_slice_len = 4;
	RK<double>::reference_kernel.max_sched_len_fun = max_sched_len_fun1<double>;

	nb.f32_kernel = RK<float>::reference_kernel;
	nb.f64_kernel = RK<double>::reference_kernel;

	bool s = false;
	s = run_test_set(gen, nb) || s;

	nb.f32_kernel = nanoblas_f32_generic_kernel_4x4;
	nb.f64_kernel = nanoblas_f64_generic_kernel_4x4;

	s = run_test_set(gen, nb) || s;

	return s;
}
