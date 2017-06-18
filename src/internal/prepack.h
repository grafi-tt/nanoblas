#ifndef NANOBLAS_INTERNAL_PREPACK_H
#define NANOBLAS_INTERNAL_PREPACK_H

#include "nanoblas_types.h"
#include "internal/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

#define start_prepack JOIN(NAMESPACE, PREFIX, start_prepack)
void start_prepack(prepack_state_t *st);

#define step_prepack JOIN(NAMESPACE, PREFIX, step_prepack)
void step_prepack(prepack_state_t *st);

#define pack_all JOIN(NAMESPACE, PREFIX, pack_all)
void pack_all(prepack_state_t *st);

#ifdef __cplusplus
}
#endif

#endif
