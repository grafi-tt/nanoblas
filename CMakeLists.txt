project(nanoblas C ASM-ATT)

cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

function(my_target_compile_setup)
	foreach (tgt IN LISTS ARGV)
		set_target_properties(${tgt} PROPERTIES C_STANDARD 99)
		set_target_properties(${tgt} PROPERTIES C_STANDARD_REQUIRED ON)
		target_include_directories(${tgt} PRIVATE ${PROJECT_SOURCE_DIR}/src)
		if(${CMAKE_C_COMPILER_ID} MATCHES "GNU|Clang")
			target_compile_options(${tgt} PRIVATE $<$<COMPILE_LANGUAGE:C>:-O2 -Werror -Wall -Wextra -Wpedantic>)
		endif()
	endforeach()
endfunction()

include(${PROJECT_SOURCE_DIR}/cmake/detect_arch.cmake)

add_subdirectory(src)

foreach(s IN LISTS SRCS)
	if (${s} MATCHES "_f32|_f64|_fxx")
		set(SRCS_ONCE ${SRCS_ONCE} ${s})
	else()
		set(SRCS_TWICE ${SRCS_TWICE} ${s})
	endif()
endforeach()

add_library(nanoblas_once OBJECT ${SRCS_ONCE})
add_library(nanoblas_f32 OBJECT ${SRCS_TWICE})
add_library(nanoblas_f64 OBJECT ${SRCS_TWICE})
my_target_compile_setup(nanoblas_once nanoblas_f32 nanoblas_f64)
target_compile_definitions(nanoblas_f32 PRIVATE USE_F32)
target_compile_definitions(nanoblas_f64 PRIVATE USE_F64)

add_library(nanoblas
	$<TARGET_OBJECTS:nanoblas_once>
	$<TARGET_OBJECTS:nanoblas_f32>
	$<TARGET_OBJECTS:nanoblas_f64>
)

enable_testing()
function(my_add_test test_name)
	if (${test_name} MATCHES "_f32|_f64|_fxx")
		add_executable(${test_name} ${ARGN})
		add_test(NAME ${test_name} COMMAND ${test_name})
		target_link_libraries(${test_name} nanoblas)
		my_target_compile_setup(${test_name})
		target_include_directories(${test_name} PRIVATE ${PROJECT_SOURCE_DIR}/test)
	else()
		my_add_test(${test_name}_f32 ${test_src} ${ARGN})
		target_compile_definitions(${test_name}_f32 PRIVATE USE_F32)
		my_add_test(${test_name}_f64 ${test_src} ${ARGN})
		target_compile_definitions(${test_name}_f64 PRIVATE USE_F64)
	endif()
endfunction()

add_subdirectory(test)
