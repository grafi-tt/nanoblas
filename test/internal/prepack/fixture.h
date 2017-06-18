#include "nanoblas_prepack.h"

FTYPE _v;
#define p &_v

prepack_state_t s1 = {
	.next_cur = p,
	.next_pack_cur = p,
	.mn_next_len = 64,
	.k_next_len = 64,
	.interval_mn = 1,
	.interval_k = 100,
	.unit_len = 8,
};

prepack_state_t s2 = {
	.next_cur = p,
	.next_pack_cur = p,
	.mn_next_len = 63,
	.k_next_len = 42,
	.interval_mn = 100,
	.interval_k = 1,
	.unit_len = 8,
};

prepack_state_t s3 = {
	.next_cur = p,
	.next_pack_cur = p,
	.mn_next_len = 3,
	.k_next_len = 15,
	.interval_mn = 1,
	.interval_k = 100,
	.unit_len = 8,
};
