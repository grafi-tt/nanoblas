set(SRCS "${SRCS}"
	"${CMAKE_CURRENT_SOURCE_DIR}/decision.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/avx_kernel_f32.s"
	"${CMAKE_CURRENT_SOURCE_DIR}/avx_kernel_f64.s"
	PARENT_SCOPE
)
