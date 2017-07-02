project(nanoblas C CXX ASM-ATT)

cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

if(${CMAKE_C_COMPILER_ID} MATCHES "GNU|Clang")
	set (CMAKE_C_FLAGS_DEBUG "-pg")
	set (CMAKE_CXX_FLAGS_DEBUG "-pg")
endif()

function(nanoblas_target_setup)
	foreach (tgt IN LISTS ARGV)
		target_include_directories(${tgt} PRIVATE ${PROJECT_SOURCE_DIR}/src)
		set_target_properties(${tgt} PROPERTIES C_STANDARD 99)
		set_target_properties(${tgt} PROPERTIES C_STANDARD_REQUIRED ON)
		if(${CMAKE_C_COMPILER_ID} MATCHES "GNU|Clang")
			target_compile_options(${tgt} PRIVATE $<$<COMPILE_LANGUAGE:C>:-O3 -Werror -Wall -Wextra -Wpedantic -std=c99>)
		endif()
		if (has_builtin_prefetch)
			target_compile_definitions(${tgt} PRIVATE HAS_BUILTIN_PREFETCH)
		endif()
		set_property(TARGET ${tgt} PROPERTY POSITION_INDEPENDENT_CODE ON)
	endforeach()
endfunction()

include(${PROJECT_SOURCE_DIR}/cmake/detect_arch.cmake)

add_subdirectory(src)

foreach(s IN LISTS SRCS)
	if (${s} MATCHES "_f32")
		set(SRCS_F32 ${SRCS_F32} ${s})
	elseif(${s} MATCHES "_f64")
		set(SRCS_F64 ${SRCS_F64} ${s})
	elseif(${s} MATCHES "_fxx")
		set(SRCS_FXX ${SRCS_FXX} ${s})
	else()
		set(SRCS_TWICE ${SRCS_TWICE} ${s})
	endif()
endforeach()

add_library(nanoblas_fxx OBJECT ${SRCS_FXX})
add_library(nanoblas_f32 OBJECT ${SRCS_TWICE} ${SRCS_F32})
add_library(nanoblas_f64 OBJECT ${SRCS_TWICE} ${SRCS_F64})
nanoblas_target_setup(nanoblas_fxx nanoblas_f32 nanoblas_f64)
target_compile_definitions(nanoblas_f32 PRIVATE USE_F32)
target_compile_definitions(nanoblas_f64 PRIVATE USE_F64)

add_library(nanoblas SHARED
	$<TARGET_OBJECTS:nanoblas_fxx>
	$<TARGET_OBJECTS:nanoblas_f32>
	$<TARGET_OBJECTS:nanoblas_f64>
)
target_include_directories(nanoblas INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>)


function(misc_target_setup)
	foreach (tgt IN LISTS ARGV)
		set_target_properties(${tgt} PROPERTIES C_STANDARD 99)
		set_target_properties(${tgt} PROPERTIES C_STANDARD_REQUIRED ON)
		set_target_properties(${tgt} PROPERTIES CXX_STANDARD 11)
		set_target_properties(${tgt} PROPERTIES CXX_STANDARD_REQUIRED ON)
		if(${CMAKE_C_COMPILER_ID} MATCHES "GNU|Clang")
			target_compile_options(${tgt} PRIVATE $<$<COMPILE_LANGUAGE:C>:-O2 -Werror -Wall -Wextra -Wpedantic -std=c99>)
			target_compile_options(${tgt} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-O2 -Werror -Wall -Wextra -Wpedantic -std=c++11>)
		endif()
	endforeach()
endfunction()

option(ENABLE_TEST "enable test target" ON)
option(ENABLE_BENCH "enable benchmark target" ON)
if (ENABLE_TEST)
	enable_testing()
	add_subdirectory(test)
endif()
if (ENABLE_BENCH)
	add_subdirectory(bench)
endif()
