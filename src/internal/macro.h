#ifndef NANOBLAS_INTERNAL_MACRO_H
#define NANOBLAS_INTERNAL_MACRO_H


#define NAMESPACE nanoblas_

#ifdef USE_F32
#define PREFIX f32_
#define FTYPE float
#define SIGN s
#endif

#ifdef USE_F64
#define PREFIX f64_
#define FTYPE double
#define SIGN d
#endif

/* types */
#define kernel_t         JOIN(NAMESPACE, PREFIX, kernel_t)
#define kernel_fun_t     JOIN(NAMESPACE, PREFIX, kernel_fun_t)
#define kernel_state_t   JOIN(NAMESPACE, PREFIX, kernel_state_t)
#define prepack_state_t  JOIN(NAMESPACE, PREFIX, prepack_state_t)

/* members of nanoblas_t */
#define kernel    JOIN(PREFIX, kernel)
#define blk_m_len JOIN(PREFIX, blk_m_len)
#define blk_n_len JOIN(PREFIX, blk_n_len)
#define blk_k_len JOIN(PREFIX, blk_k_len)

/* join */
#define JOIN(...) \
	JOIN_HELPER_1(__VA_ARGS__, ARITY_9, ARITY_8, ARITY_7, ARITY_6, ARITY_5, ARITY_4, ARITY_3, ARITY_2, dummy)
#define JOIN_HELPER_1(v1, v2, v3, v4, v5, v6, v7, v8, v9, arity, ...) \
	JOIN_HELPER_2(v1, v2, v3, v4, v5, v6, v7, v8, v9, arity)
#define JOIN_HELPER_2(v1, v2, v3, v4, v5, v6, v7, v8, v9 arity) \
	JOIN_##arity(v1, v2, v3, v4, v5, v6, v7, v8, v9)

#define JOIN_ARITY_9(v1, v2, v3, v4, v5, v6, v7, v8, v9) \
	JOIN_OP_1(v1, JOIN_ARITY_8(v2, v3, v4, v5, v6, v7, v8, v9, dummy))
#define JOIN_ARITY_8(v1, v2, v3, v4, v5, v6, v7, v8, ...) \
	JOIN_OP_1(v1, JOIN_ARITY_7(v2, v3, v4, v5, v6, v7, v8, dummy))
#define JOIN_ARITY_7(v1, v2, v3, v4, v5, v6, v7, ...) \
	JOIN_OP_1(v1, JOIN_ARITY_6(v2, v3, v4, v5, v6, v7, dummy))
#define JOIN_ARITY_6(v1, v2, v3, v4, v5, v6, ...) \
	JOIN_OP_1(v1, JOIN_ARITY_5(v2, v3, v4, v5, v6, dummy))
#define JOIN_ARITY_5(v1, v2, v3, v4, v5, ...) \
	JOIN_OP_1(v1, JOIN_ARITY_4(v2, v3, v4, v5, dummy))
#define JOIN_ARITY_4(v1, v2, v3, v4, ...) \
	JOIN_OP_1(v1, JOIN_ARITY_3(v2, v3, v4, dummy))
#define JOIN_ARITY_3(v1, v2, v3, ...) \
	JOIN_OP_1(v1, JOIN_ARITY_2(v2, v3, dummy))
#define JOIN_ARITY_2(v1, v2, ...) \
	JOIN_OP_1(v1, v2)

#define JOIN_OP_1(v1, v2) JOIN_OP_2(v1, v2)
#define JOIN_OP_2(v1, v2) v1##v2

#endif
