#add_subdirectory(kernel)
set(TESTS ${TESTS}
	${CMAKE_CURRENT_SOURCE_DIR}/start_sched.c
	PARENT_SCOPE
)
