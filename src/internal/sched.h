#ifndef NANOBLAS_INTERNAL_SCHED_H
#define NANOBLAS_INTERNAL_SCHED_H

#include <stddef.h>
#include "kernel.h"
#include "internal/macro.h"

#define start_sched ADD_FTYPE(start_sched)
void start_sched(sched_state_t *st);

#define step_sched ADD_FTYPE(step_sched)
void step_sched(sched_state_t *st);

#define pack_all ADD_FTYPE(pack_all)
void pack_all(sched_state_t *st);

#endif
