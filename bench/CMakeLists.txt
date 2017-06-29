find_package(PkgConfig)

add_executable(bench_nanoblas ${CMAKE_CURRENT_SOURCE_DIR}/bench_nanoblas.cpp)
target_link_libraries(bench_nanoblas nanoblas)

add_executable(bench_openblas ${CMAKE_CURRENT_SOURCE_DIR}/bench_openblas.cpp)
pkg_check_modules(OPENBLAS REQUIRED openblas-threads)
target_link_libraries(bench_openblas ${OPENBLAS_LIBRARIES})
target_include_directories(bench_openblas PRIVATE ${OPENBLAS_INCLUDE_DIRS})

add_executable(bench_mkl ${CMAKE_CURRENT_SOURCE_DIR}/bench_mkl.cpp)
set(MKL_DIR /opt/intel/mkl)
target_link_libraries(bench_mkl
	${MKL_DIR}/lib/intel64/libmkl_intel_lp64.so
	${MKL_DIR}/lib/intel64/libmkl_sequential.so
	${MKL_DIR}/lib/intel64/libmkl_core.so)
target_include_directories(bench_mkl PRIVATE ${MKL_DIR}/include)

misc_target_setup(bench_nanoblas bench_openblas bench_mkl)
