#include <iostream>
#include "test_kernel_mult.hpp"
#include "kernel/generic_kernel.hpp"
#include "kernel/x86_64/avx_kernel.hpp"

template <typename FTYPE>
bool run_kernel_mult_test_set(const std::string &name, nanoblas::kernel_t<FTYPE> kernel) {
	std::mt19937 gen(314159265);
	bool s = false;
	s = run_kernel_mult_test<FTYPE>(gen, name, kernel, 64) || s;
	s = run_kernel_mult_test<FTYPE>(gen, name, kernel, 67) || s;
	s = run_kernel_mult_test<FTYPE>(gen, name, kernel, 64, 5, 5, 4, 4) || s;
	s = run_kernel_mult_test<FTYPE>(gen, name, kernel, 30, 1, 1, 3, 3) || s;
	return s;
}

int main() {
	bool s = false;
	s = run_kernel_mult_test_set<float>("f32_generic_4x4", nanoblas::get_generic_kernel_4x4<float>()) || s;
	s = run_kernel_mult_test_set<float>("f32_generic_6x4", nanoblas::get_generic_kernel_6x4<float>()) || s;
	s = run_kernel_mult_test_set<double>("f64_generic_4x4", nanoblas::get_generic_kernel_4x4<double>()) || s;
	s = run_kernel_mult_test_set<double>("f64_generic_6x4", nanoblas::get_generic_kernel_6x4<double>()) || s;
	s = run_kernel_mult_test_set<float>("f32_avx", nanoblas::get_avx_kernel()) || s;
	return s;
}
