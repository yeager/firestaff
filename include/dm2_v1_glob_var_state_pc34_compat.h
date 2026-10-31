#ifndef FIRESTAFF_DM2_V1_GLOB_VAR_STATE_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_GLOB_VAR_STATE_PC34_COMPAT_H

#include <stdint.h>

/* skproject dm2global.cpp — three-tier global variable storage. */
typedef struct {
    uint8_t bit_vars[8];
    uint8_t byte_vars[64];
    int16_t word_vars[192];
} DM2_V1_GlobVarState;

void dm2_v1_glob_var_init(DM2_V1_GlobVarState *state);
int32_t dm2_v1_get_glob_var(
    const DM2_V1_GlobVarState *state, uint16_t index);
int32_t dm2_v1_update_glob_var_direct(
    DM2_V1_GlobVarState *state,
    int16_t var_idx, int16_t mode, int16_t value);

#endif /* FIRESTAFF_DM2_V1_GLOB_VAR_STATE_PC34_COMPAT_H */
