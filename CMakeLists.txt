project(nanoblas C ASM-ATT)

cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

macro(my_targets_compile_setup targets)
	foreach (tgt IN LISTS targets)
		set_target_properties(tgt PROPERTIES C_STANDARD 99)
		set_target_properties(tgt PROPERTIES C_STANDARD_REQUIRED ON)
		if(${CMAKE_C_COMPILER_ID} MATCHES "GNU|Clang")
			target_compile_setup(tgt PRIVATE $<$<COMPILE_LANGUAGE:C>:-O2 -Werror -Wall -Wextra -Wpedantic>)
		endif()
	endforeach()
endmacro()

include_directories("src")
include(${CMAKE_SOURCE_DIR}/cmake/detect_arch.cmake)

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
my_targets_compile_setup(nanoblas_once nanoblas_f32 nanoblas_f64)
target_compile_definitions(nanoblas_f32 PRIVATE USE_F32)
target_compile_definitions(nanoblas_f64 PRIVATE USE_F64)

add_library(nanoblas
	$<TARGET_OBJECTS:nanoblas_once>
	$<TARGET_OBJECTS:nanoblas_f32>
	$<TARGET_OBJECTS:nanoblas_f64>
)

add_subdirectory(test)

macro(my_add_test test_name test_src)
	add_executable(${test_name} ${test_src})
	target_link_libraries(${test_name} nanoblas)
	my_targets_compile_setup(${test_name})
	add_test(NAME ${test_name} COMMAND ${test_name})
endmacro()

enable_testing()
foreach(test_src IN LISTS TESTS)
	string(REGEX REPLACE ".*/" "" test_name ${test_src})
	string(REGEX REPLACE "\\.[^/]*$" "" test_name ${test_name})
	if (${test_src} MATCHES "_f32|_f64|_fxx")
		my_add_test(${test_name} ${test_src})
	else()
		my_add_test(${test_name}_f32 ${test_src})
		target_compile_definitions(${test_name}_f32 PRIVATE USE_F32)
		my_add_test(${test_name}_f64 ${test_src})
		target_compile_definitions(${test_name}_f64 PRIVATE USE_F64)
	endif()
endforeach()
