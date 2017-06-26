#include "test_kernel.hpp"
#include "kernel/generic_kernel.h"
#include "kernel/x86_64/avx_kernel.h"

int main() {
	std::mt19937 gen(314159265);
	bool s = false;

	s = run_mult_test<float>(gen, nanoblas_f32_generic_kernel_4x4, 64, 3, 3);
	s = run_mult_test<float>(gen, nanoblas_f32_generic_kernel_6x4, 64);
	s = run_mult_test<float>(gen, nanoblas_f32_avx_kernel, 64);
	s = run_mult_test<float>(gen, nanoblas_f32_avx_kernel, 64, 5, 5);

	return s;
}
