#ifndef FIRESTAFF_DM2_V1_MOVE_2C1D_028C_H
#define FIRESTAFF_DM2_V1_MOVE_2C1D_028C_H

#include <stdint.h>

#include "dm2_v1_move_075f_1bc2.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM2_V1_MOVE_2C1D_028C_OUTCOME_NONE = 0,
    DM2_V1_MOVE_2C1D_028C_OUTCOME_BLOCKED,
    DM2_V1_MOVE_2C1D_028C_OUTCOME_ADVANCE,
    DM2_V1_MOVE_2C1D_028C_OUTCOME_STAIR_UP,
    DM2_V1_MOVE_2C1D_028C_OUTCOME_STAIR_DOWN
} DM2_V1_Move2c1d028cOutcome;

typedef struct {
    int valid;
    int source_state_unbound;
    int committed;
    int blocked;
    DM2_V1_Move2c1d028cOutcome outcome;
    DM2_V1_Move075f1bc2BlockReason block_reason;
    int level_before;
    int x_before;
    int y_before;
    int dir_before;
    int level_after;
    int x_after;
    int y_after;
    int dir_after;
    int target_square_type;
    int target_raw;
    int target_first_thing;
    int vertical_delta;
    int preserves_facing;
    int requires_post_step_chain;
    uint32_t commit_hash;
} DM2_V1_Move2c1d028cReceipt;

int dm2_v1_DM2_move_2c1d_028c_commit_receipt(
    const DM2_V1_Move075f1bc2Receipt *target,
    DM2_V1_Move2c1d028cReceipt *out);

const char *dm2_v1_DM2_move_2c1d_028c_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_MOVE_2C1D_028C_H */
