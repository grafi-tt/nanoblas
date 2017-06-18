#ifndef NANOBLAS_INTERNAL_PREPACK_H
#define NANOBLAS_INTERNAL_PREPACK_H

#include "nanoblas_types.h"
#include "internal/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

#define start_prepack ADD_FTYPE(start_prepack)
void start_prepack(prepack_state_t *st);

#define step_prepack ADD_FTYPE(step_prepack)
void step_prepack(prepack_state_t *st);

#define pack_all ADD_FTYPE(pack_all)
void pack_all(prepack_state_t *st);

#ifdef __cplusplus
}
#endif

#endif
