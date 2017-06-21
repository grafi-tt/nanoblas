try_compile(is_x86_64
	${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_LIST_DIR}/detect_x86_64.c)
try_compile(has_builtin_prefetch
	${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_LIST_DIR}/detect_builtin_prefetch.c)
