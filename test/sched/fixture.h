FTYPE _v;
#define p &_v

sched_state_t s1 = {
	.next_cur = p,
	.next_pack_cur = p,
	.k_next_len = 64,
	.mn_next_len = 64,
	.interval_k = 100,
	.interval_mn = 1,
	.unit_len = 8
};

sched_state_t s2 = {
	.next_cur = p,
	.next_pack_cur = p,
	.k_next_len = 42,
	.mn_next_len = 63,
	.interval_k = 1,
	.interval_mn = 100,
	.unit_len = 8
};

sched_state_t s3 = {
	.next_cur = p,
	.next_pack_cur = p,
	.k_next_len = 15,
	.mn_next_len = 3,
	.interval_k = 100,
	.interval_mn = 1,
	.unit_len = 8
};
