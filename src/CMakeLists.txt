add_subdirectory(kernel)
add_subdirectory(internal)
add_subdirectory(param)
set(SRCS ${SRCS}
	${CMAKE_CURRENT_SOURCE_DIR}/gemm.c
	PARENT_SCOPE
)
