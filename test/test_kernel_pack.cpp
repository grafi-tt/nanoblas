#include <iostream>
#include "test_kernel_pack.hpp"
#include "kernel/generic_kernel.h"
#include "kernel/x86_64/avx_kernel.h"

int main() {
	std::mt19937 gen(314159265);
	bool s = false;

	std::cout << "=== pack test ===" << std::endl;
	std::cout << "generic 4x4 no trans" << std::endl;
	s = run_pack_test<float>(gen, nanoblas_f32_generic_kernel_4x4, 64, 4, 1, 10);
	std::cout << "generic 4x4 trans" << std::endl;
	s = run_pack_test<float>(gen, nanoblas_f32_generic_kernel_4x4, 64, 4, 10, 1);
	std::cout << "generic 6x4 no trans" << std::endl;
	s = run_pack_test<float>(gen, nanoblas_f32_generic_kernel_6x4, 64, 6, 1, 10);
	std::cout << "generic 6x4 trans" << std::endl;
	s = run_pack_test<float>(gen, nanoblas_f32_generic_kernel_6x4, 64, 6, 10, 1);
	std::cout << "avx, no trans" << std::endl;
	s = run_pack_test<float>(gen, nanoblas_f32_avx_kernel, 8, 8, 1, 100);
	std::cout << "avx, trans" << std::endl;
	s = run_pack_test<float>(gen, nanoblas_f32_avx_kernel, 8, 8, 100, 1);
	std::cout << "avx, no trans, unalign device" << std::endl;
	s = run_pack_test<float>(gen, nanoblas_f32_avx_kernel, 35, 8, 1, 100);

	return s;
}
