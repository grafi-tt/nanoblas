if(is_x86_64)
	add_subdirectory(x86_64)
else()
	add_subdirectory(other)
endif()
set(SRCS ${SRCS} PARENT_SCOPE)
