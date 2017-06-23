#include <cstdint>
#include <chrono>
#include <iostream>
#include <random>
#include <vector>

template<typename T>
constexpr T eps();

template<>
constexpr float eps() {
	return 1e-4;
}

template<>
constexpr double eps() {
	return 1e-8;
}

template<typename T>
bool mostly_equal(T x, T y) {
	return std::abs(x - y) / std::max(std::abs(x), std::abs(y)) < eps<T>();
}

template <typename T>
using impl_t = void (const T *, const T *, T *, size_t, size_t, size_t);

template<typename T>
bool run_test(std::mt19937 gen, impl_t<T> *impl, size_t M, size_t N, size_t K) {
	std::vector<T> A(M*K);
	std::vector<T> B(K*N);
	std::vector<T> C(M*N);
	std::vector<T> D(M*N);

	for (auto &&v: A) v = gen();
	for (auto &&v: B) v = gen();

	auto t1s = std::chrono::high_resolution_clock::now();
	for (size_t m = 0; m < M; m++)
		for (size_t n = 0; n < N; n++)
			for (size_t k = 0; k < K; k++)
				C[N*m+n] += A[K*m+k] * B[N*k+n];
	auto t1e = std::chrono::high_resolution_clock::now();
	auto d1 = t1e - t1s;
	std::cout << "naive " <<
		std::chrono::duration_cast<std::chrono::milliseconds>(d1).count() << std::endl;

	auto t2s = std::chrono::high_resolution_clock::now();
	impl(A.data(), B.data(), D.data(), M, N, K);
	auto t2e = std::chrono::high_resolution_clock::now();
	auto d2 = t2e - t2s;
	std::cout << "impl " <<
		std::chrono::duration_cast<std::chrono::milliseconds>(d2).count() << std::endl;

	for (size_t i = 0; i < M; i++) {
		for (size_t j = 0; j < N; j++) {
			T x = C[N*i+j];
			T y = D[N*i+j];
			if (!mostly_equal(x, y)) {
				std::cerr <<
					"C["<<i<<"]["<<j<<"] = "<<x<<"; " <<
					"D["<<i<<"]["<<j<<"] = "<<y<<";" << std::endl;
				return true;
			}
		}
	}

	return false;
}
