project(nanoblas C ASM-ATT)

cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

include_directories(".")
include("${CMAKE_SOURCE_DIR}/cmake/detect_arch.cmake")

add_subdirectory(kernel)
set(SRCS "${SRCS}"
	"${CMAKE_SOURCE_DIR}/boundary.c"
	"${CMAKE_SOURCE_DIR}/gemm.c"
)

string(REGEX REPLACE "[a-zA-Z0-9_./\\-]*_f64[a-zA-Z0-9_./\\-]*" "" SRCS_F32 "${SRCS}")
add_library(nanoblas_f32 OBJECT "${SRCS_F32}")
target_compile_definitions(nanoblas_f32 PRIVATE USE_F32)

string(REGEX REPLACE "[a-zA-Z0-9_./\\-]*_f32[a-zA-Z0-9_./\\-]*" "" SRCS_F64 "${SRCS}")
add_library(nanoblas_f64 OBJECT "${SRCS_F64}")
target_compile_definitions(nanoblas_f64 PRIVATE USE_F64)

add_library(nanoblas
	$<TARGET_OBJECTS:nanoblas_f32>
	$<TARGET_OBJECTS:nanoblas_f64>
)

set(TARGETS nanoblas_f32 nanoblas_f64 nanoblas)

set_target_properties(${TARGETS} PROPERTIES C_STANDARD 99)
set_target_properties(${TARGETS} PROPERTIES LINKER_LANGUAGE C)
if(CMAKE_C_COMPILER MATCHES "(gcc|clang)[^/\\]*$")
	target_compile_options(${TARGETS} PUBLIC "-O2 -Werror -Wall -Wextra -Wpedantic")
endif()