add_subdirectory(config)
set(SRCS ${SRCS}
	${CMAKE_CURRENT_SOURCE_DIR}/prepack.c
	${CMAKE_CURRENT_SOURCE_DIR}/iter_fxx.c
	PARENT_SCOPE
)
