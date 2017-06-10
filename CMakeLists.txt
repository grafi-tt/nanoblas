project(nanoblas C ASM-ATT)

cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

include_directories(".")
include(${CMAKE_SOURCE_DIR}/cmake/detect_arch.cmake)

add_subdirectory(kernel)
set(SRCS ${SRCS}
	${CMAKE_SOURCE_DIR}/sched.c
	${CMAKE_SOURCE_DIR}/gemm.c
)
set(TYPE_INDEPENDENT_SRCS ${TYPE_INDEPENDENT_SRCS}
	${CMAKE_SOURCE_DIR}/iter.c
)

string(REGEX REPLACE "[a-zA-Z0-9_./\\-]*_f64[a-zA-Z0-9_./\\-]*" "" SRCS_F32 "${SRCS}")
add_library(nanoblas_f32 OBJECT ${SRCS_F32})
target_compile_definitions(nanoblas_f32 PRIVATE USE_F32)

string(REGEX REPLACE "[a-zA-Z0-9_./\\-]*_f32[a-zA-Z0-9_./\\-]*" "" SRCS_F64 "${SRCS}")
add_library(nanoblas_f64 OBJECT ${SRCS_F64})
target_compile_definitions(nanoblas_f64 PRIVATE USE_F64)

add_library(nanoblas
	$<TARGET_OBJECTS:nanoblas_f32>
	$<TARGET_OBJECTS:nanoblas_f64>
	${TYPE_INDEPENDENT_SRCS}
)

set_target_properties(nanoblas_f32 nanoblas_f64 nanoblas PROPERTIES C_STANDARD 99)
if(${CMAKE_C_COMPILER_ID} MATCHES "GNU|Clang")
	target_compile_options(nanoblas_f32 PUBLIC $<$<COMPILE_LANGUAGE:C>:-O2 -Werror -Wall -Wextra -Wpedantic>)
	target_compile_options(nanoblas_f64 PUBLIC $<$<COMPILE_LANGUAGE:C>:-O2 -Werror -Wall -Wextra -Wpedantic>)
	target_compile_options(nanoblas     PUBLIC $<$<COMPILE_LANGUAGE:C>:-O2 -Werror -Wall -Wextra -Wpedantic>)
endif()
