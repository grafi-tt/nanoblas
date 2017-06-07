cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

set_target_properties(nanoblas PROPERTIES C_STANDARD 99)
if(CMAKE_C_COMPILER MATCHES "(gcc|clang)[^/\\]*$")
	target_compile_options(nanoblas PUBLIC "-O2 -Werror -Wall -Wextra -Wpedantic")
endif()

include(detect_arch)
set(SRCS
	boundary.c
	gemm.c
)
add_subdirectory(kernel)

string(REGEX REPLACE "[^ \n\t]*_f64[^ \n\t]*" "" SRCS_F32 "${SRCS}")
target_compile_definitions(nanoblas_f32 PRIVATE USE_F32)
add_library(nanoblas_f32 OBJECT $SRCS)

string(REGEX REPLACE "[^ \n\t]*_f32[^ \n\t]*" "" SRCS_F64 "${SRCS}")
target_compile_definitions(nanoblas_f64 PRIVATE USE_F64)
add_library(nanoblas_f64 OBJECT $SRCS)

add_library(nanoblas
	$<TARGET_OBJECTS:nanoblas_f32>
	$<TARGET_OBJECTS:nanoblas_f64>
)
