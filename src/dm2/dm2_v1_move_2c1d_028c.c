#include "dm2_v1_move_2c1d_028c.h"

#include <string.h>

static uint32_t dm2_move_2c1d_028c_hash_step(uint32_t hash, uint32_t value)
{
    hash ^= value;
    return hash * 16777619u;
}

static DM2_V1_Move2c1d028cOutcome dm2_move_2c1d_028c_outcome(
    const DM2_V1_Move075f1bc2Receipt *target)
{
    if (!target || !target->accepted) {
        return DM2_V1_MOVE_2C1D_028C_OUTCOME_BLOCKED;
    }
    if (target->vertical_kind == DM2_V1_MOVE_075F_1BC2_VERTICAL_UP) {
        return DM2_V1_MOVE_2C1D_028C_OUTCOME_STAIR_UP;
    }
    if (target->vertical_kind == DM2_V1_MOVE_075F_1BC2_VERTICAL_DOWN) {
        return DM2_V1_MOVE_2C1D_028C_OUTCOME_STAIR_DOWN;
    }
    return DM2_V1_MOVE_2C1D_028C_OUTCOME_ADVANCE;
}

int dm2_v1_DM2_move_2c1d_028c_commit_receipt(
    const DM2_V1_Move075f1bc2Receipt *target,
    DM2_V1_Move2c1d028cReceipt *out)
{
    uint32_t hash = 2166136261u;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!target || !target->valid) return 0;

    out->valid = 1;
    out->outcome = dm2_move_2c1d_028c_outcome(target);
    out->block_reason = target->block_reason;
    out->level_before = target->level;
    out->x_before = target->from_x;
    out->y_before = target->from_y;
    out->dir_before = target->from_dir;
    out->level_after = target->level;
    out->x_after = target->from_x;
    out->y_after = target->from_y;
    out->dir_after = target->from_dir;
    out->target_square_type = target->target_square_type;
    out->target_raw = target->target_raw;
    out->target_first_thing = target->target_first_thing;
    out->preserves_facing = 1;

    if (target->accepted && !target->blocked) {
        out->committed = 1;
        out->x_after = target->to_x;
        out->y_after = target->to_y;
        if (target->vertical_kind == DM2_V1_MOVE_075F_1BC2_VERTICAL_UP) {
            out->vertical_delta = -1;
            out->requires_post_step_chain = 1;
        } else if (target->vertical_kind ==
                   DM2_V1_MOVE_075F_1BC2_VERTICAL_DOWN) {
            out->vertical_delta = 1;
            out->requires_post_step_chain = 1;
        } else if (target->target_first_thing >= 0) {
            out->requires_post_step_chain = 1;
        }
    } else {
        out->blocked = 1;
        out->outcome = DM2_V1_MOVE_2C1D_028C_OUTCOME_BLOCKED;
    }

    hash = dm2_move_2c1d_028c_hash_step(hash, (uint32_t)out->outcome);
    hash = dm2_move_2c1d_028c_hash_step(hash, (uint32_t)out->block_reason);
    hash = dm2_move_2c1d_028c_hash_step(hash, (uint32_t)out->level_before);
    hash = dm2_move_2c1d_028c_hash_step(hash, (uint32_t)out->x_before);
    hash = dm2_move_2c1d_028c_hash_step(hash, (uint32_t)out->y_before);
    hash = dm2_move_2c1d_028c_hash_step(hash, (uint32_t)out->dir_before);
    hash = dm2_move_2c1d_028c_hash_step(hash, (uint32_t)out->level_after);
    hash = dm2_move_2c1d_028c_hash_step(hash, (uint32_t)out->x_after);
    hash = dm2_move_2c1d_028c_hash_step(hash, (uint32_t)out->y_after);
    hash = dm2_move_2c1d_028c_hash_step(hash, (uint32_t)out->dir_after);
    hash = dm2_move_2c1d_028c_hash_step(hash, (uint32_t)out->target_square_type);
    hash = dm2_move_2c1d_028c_hash_step(hash, (uint32_t)out->target_raw);
    hash = dm2_move_2c1d_028c_hash_step(hash, (uint32_t)out->target_first_thing);
    hash = dm2_move_2c1d_028c_hash_step(hash, (uint32_t)out->vertical_delta);
    hash = dm2_move_2c1d_028c_hash_step(
        hash, (uint32_t)out->requires_post_step_chain);
    hash = dm2_move_2c1d_028c_hash_step(hash, (uint32_t)out->committed);
    if (hash == 0u) return 0;
    out->commit_hash = hash;
    return 1;
}

const char *dm2_v1_DM2_move_2c1d_028c_source_evidence(void)
{
    return "skproject SKULLWIN/c_move.cpp:2914 DM2_move_2c1d_028c: "
           "bounded post-target movement receipt. It consumes the "
           "DM2_move_075f_1bc2 target-cell result, preserves party facing, "
           "advances only for accepted targets, records blocked/no-mutation "
           "outcomes, and marks stairs or target thing links as post-step "
           "chain work instead of fabricating sensors, teleports, or record "
           "mutation.";
}
