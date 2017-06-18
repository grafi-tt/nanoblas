if(is_x86_64)
	add_subdirectory(x86_64)
endif()
set(SRCS ${SRCS}
	${CMAKE_CURRENT_SOURCE_DIR}/generic_kernel_4.c
	${CMAKE_CURRENT_SOURCE_DIR}/generic_kernel_8.c
	PARENT_SCOPE
)
