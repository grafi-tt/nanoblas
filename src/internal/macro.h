#ifndef NANOBLAS_INTERNAL_MACRO_H
#define NANOBLAS_INTERNAL_MACRO_H

/**
 * @defgroup INTERNAL   Internally used functions / macros
 * \{
 * @defgroup MACRO      Internally used macros
 * \{
 */

/**
 * @name Fragments of symbol / type names
 *       Typically they are concatinated to the “real” name by JOIN() macro.
 * \{ */

/**
 * @brief All symbols exported are prefixed with this namespace string.
 *
 *        For public API, the namespace is explicitly prefixed on the header files.
 *        In source codes or internal headers, it is made implicit by using this macro.
 */
#define NAMESPACE nanoblas_

/**
 * @brief It will expand to `f32_` or `f64_`, depending on which of USE_F32 or USE_F64 is defined.
 */
#define FSIZE_PREFIX
/**
 * @brief It will expand to `float` or `double`, depending on which of USE_F32 or USE_F64 is defined.
 */
#define FTYPE
/**
 * @brief It will expand to `s` or `d`, depending on which of USE_F32 or USE_F64 is defined.
 */
#define SIGN

/** \cond MACRO_DETAIL */
#undef FSIZE_PREFIX
#undef FTYPE
#undef SIGN

#ifdef USE_F32
#define FSIZE_PREFIX f32_
#define FTYPE float
#define SIGN s
#endif

#ifdef USE_F64
#define FSIZE_PREFIX f64_
#define FTYPE double
#define SIGN d
#endif
/* \endcond */
/** \} */

/**
 * @name Code manipulation
 * \{ */

/**
 * @brief Expand and concatinate given tokens.
 * \hideinitializer
 */
#define JOIN(...) \
	JOIN_HELPER_1(__VA_ARGS__, ARITY_9, ARITY_8, ARITY_7, ARITY_6, ARITY_5, ARITY_4, ARITY_3, ARITY_2, dummy)
 /** \cond MACRO_DETAIL */
#define JOIN_HELPER_1(v1, v2, v3, v4, v5, v6, v7, v8, v9, arity, ...) \
	JOIN_HELPER_2(v1, v2, v3, v4, v5, v6, v7, v8, v9, arity)
#define JOIN_HELPER_2(v1, v2, v3, v4, v5, v6, v7, v8, v9, arity) \
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
/** \endcond */

/** \} */

/**
 * @name Type aliases
 * \{ */
#define kernel_t         JOIN(NAMESPACE, FSIZE_PREFIX, kernel_t)
#define kernel_fun_t     JOIN(NAMESPACE, FSIZE_PREFIX, kernel_fun_t)
#define kernel_state_t   JOIN(NAMESPACE, FSIZE_PREFIX, kernel_state_t)
#define prepack_state_t  JOIN(NAMESPACE, FSIZE_PREFIX, prepack_state_t)
/** \} */

/**
 * @name Member name aliases
 * \{ */
#define kernel    JOIN(FSIZE_PREFIX, kernel)
#define blk_n_max_len JOIN(FSIZE_PREFIX, blk_n_max_len)
#define blk_k_max_len JOIN(FSIZE_PREFIX, blk_k_max_len)
/** \} */

/** \}\} */

#endif
