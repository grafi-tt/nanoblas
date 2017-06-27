#include <iostream>
#include "test_kernel.hpp"
#include "kernel/generic_kernel.h"
#include "kernel/x86_64/avx_kernel.h"

int main() {
	std::mt19937 gen(314159265);
	bool s = false;

	std::cout << "generic 4x4" << std::endl;
	s = run_mult_test<float>(gen, nanoblas_f32_generic_kernel_4x4, 64, 3, 3);
	std::cout << "generic 6x4" << std::endl;
	s = run_mult_test<float>(gen, nanoblas_f32_generic_kernel_6x4, 64);
	std::cout << "avx" << std::endl;
	s = run_mult_test<float>(gen, nanoblas_f32_avx_kernel, 64);
	std::cout << "avx, duff's device" << std::endl;
	s = run_mult_test<float>(gen, nanoblas_f32_avx_kernel, 67);
	std::cout << "avx with 5x5 real len" << std::endl;
	s = run_mult_test<float>(gen, nanoblas_f32_avx_kernel, 64, 5, 5);

	return s;
}
