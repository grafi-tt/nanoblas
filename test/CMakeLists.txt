add_library(sfmt SFMT/SFMT.c)
target_compile_definitions(sfmt PUBLIC SFMT_MEXP=19937)
my_add_test(gemm_f32
	${CMAKE_CURRENT_SOURCE_DIR}/gemm_f32.c)
target_link_libraries(gemm_f32 sfmt)
#add_subdirectory(internal)
#add_subdirectory(kernel)
