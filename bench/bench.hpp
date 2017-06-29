#include <cstdint>
#include <chrono>
#include <functional>
#include <iostream>
#include <random>
#include <string>
#include <vector>

template<typename FTYPE>
using impl_t = std::function<void(const FTYPE *, const FTYPE *, FTYPE *, size_t, size_t, size_t, size_t, size_t, size_t)>;

template<typename FTYPE>
void run_test(std::mt19937 gen, const impl_t<FTYPE> &impl, const std::string &name,
		size_t M, size_t N, size_t K, size_t lda, size_t ldb, size_t ldc) {

	std::cout << name << std::endl;
	std::cout << "("<<M<<","<<K<<") x ("<<K<<","<<N<<"); lda: "<<lda<<", ldb: "<<ldb<<", ldc: "<<ldc << std::endl;

	std::uniform_real_distribution<FTYPE> dist(0, 1);

	std::vector<FTYPE> A(M*lda);
	std::vector<FTYPE> B(K*ldb);
	std::vector<FTYPE> C(M*ldc);

	for (auto &&v: A) v = dist(gen);
	for (auto &&v: B) v = dist(gen);
	for (auto &&v: C) v = dist(gen);

	auto t1 = std::chrono::high_resolution_clock::now();
	impl(A.data(), B.data(), C.data(), M, N, K, lda, ldb, ldc);
	auto t2 = std::chrono::high_resolution_clock::now();
	auto d = t2 - t1;
	std::cout << "elapsed time: " <<
		std::chrono::duration_cast<std::chrono::microseconds>(d).count() << "us" << std::endl;
}
template<typename FTYPE>
void run_test(const std::string &name, std::mt19937 gen, const impl_t<FTYPE> &impl, size_t M, size_t N, size_t K) {
	run_test<FTYPE>(gen, impl, name, M, N, K, K, N, N);
}

void run_sgemm_test_set(const std::string &name, impl_t<float> impl) {
	std::mt19937 gen(314159265);
	std::string new_name = "sgemm "+name;
	run_test<float>(new_name, gen, impl, 128, 128, 128);
	run_test<float>(new_name, gen, impl, 256, 256, 256);
	run_test<float>(new_name, gen, impl, 512, 512, 512);
	run_test<float>(new_name, gen, impl, 768, 768, 768);
	run_test<float>(new_name, gen, impl, 1024, 1024, 1024);
	run_test<float>(new_name, gen, impl, 1536, 1536, 1536);
	run_test<float>(new_name, gen, impl, 2048, 2048, 2048);
}
