add_library(sfmt SFMT/SFMT.c)
my_add_test(gemm_f32
	${CMAKE_CURRENT_SOURCE_DIR}/gemm_f32.c)
target_link_libraries(gemm_f32 sfmt)
target_compile_definitions(gemm_f32 PRIVATE SFMT_MEXP=19937)
#add_subdirectory(internal)
#add_subdirectory(kernel)
