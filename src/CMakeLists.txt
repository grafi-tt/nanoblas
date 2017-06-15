add_subdirectory(kernel)
set(SRCS ${SRCS}
	${CMAKE_CURRENT_SOURCE_DIR}/sched.c
	${CMAKE_CURRENT_SOURCE_DIR}/gemm.c
	${CMAKE_CURRENT_SOURCE_DIR}/iter_fxx.c
	PARENT_SCOPE
)
