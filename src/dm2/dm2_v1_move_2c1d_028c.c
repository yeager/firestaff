#include "dm2_v1_move_2c1d_028c.h"

#include <string.h>

int dm2_v1_DM2_move_2c1d_028c_commit_receipt(
    const DM2_V1_Move075f1bc2Receipt *target,
    DM2_V1_Move2c1d028cReceipt *out)
{
    (void)target;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->source_state_unbound = 1;
    out->blocked = 1;
    out->outcome = DM2_V1_MOVE_2C1D_028C_OUTCOME_BLOCKED;
    out->block_reason = DM2_V1_MOVE_075F_1BC2_BLOCK_SOURCE_STATE_UNBOUND;
    return 0;
}

const char *dm2_v1_DM2_move_2c1d_028c_source_evidence(void)
{
    return "skproject SKULLWIN/c_move.cpp:2914 DM2_move_2c1d_028c: "
           "finds a party member adjacent to the supplied position and returns "
           "that hero index or -1; it does not commit movement. The party owner "
           "and candidate-position helper are unbound, so this adapter rejects "
           "instead of committing a fabricated dungeon move.";
}
