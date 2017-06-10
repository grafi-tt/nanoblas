if(is_x86_64)
	add_subdirectory(x86_64)
else()
	add_subdirectory(other)
endif()
set(SRCS ${SRCS}
	${CMAKE_CURRENT_SOURCE_DIR}/generic_kernel.c
	PARENT_SCOPE
)
set(TYPE_INDEPENDENT_SRCS ${TYPE_INDEPENDENT_SRCS}
	PARENT_SCOPE
)
