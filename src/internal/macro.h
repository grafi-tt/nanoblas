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
#define JOIN(...) JOIN_HELPER_1(__VA_ARGS__, ARITY_4, ARITY_3, ARITY_2, dummy)
#define JOIN_HELPER_1(v1, v2, v3, v4, arity, ...) JOIN_HELPER_2(v1, v2, v3, v4, arity)
#define JOIN_HELPER_2(v1, v2, v3, v4, arity)      JOIN_##arity(v1, v2, v3, v4)

#define JOIN_ARITY_4(v1, v2, v3, v4)  JOIN_OP_1(JOIN_ARITY_3(v1, v2, v3, dummy), v4)
#define JOIN_ARITY_3(v1, v2, v3, ...) JOIN_OP_1(JOIN_ARITY_2(v1, v2, dummy), v3)
#define JOIN_ARITY_2(v1, v2, ...)     JOIN_OP_1(v1, v2)

#define JOIN_OP_1(v1, v2) JOIN_OP_2(v1, v2)
#define JOIN_OP_2(v1, v2) v1##v2

#endif
