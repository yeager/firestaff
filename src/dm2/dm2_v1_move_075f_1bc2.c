#include "dm2_v1_move_075f_1bc2.h"

#include <string.h>

int dm2_v1_DM2_move_075f_1bc2_target_receipt(
    const DM2_V1_DungeonData *dungeon,
    int level,
    int x,
    int y,
    int facing_dir,
    int move_dir,
    DM2_V1_Move075f1bc2Receipt *out)
{
    (void)dungeon;
    (void)level;
    (void)x;
    (void)y;
    (void)facing_dir;
    (void)move_dir;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->blocked = 1;
    out->source_state_unbound = 1;
    out->block_reason = DM2_V1_MOVE_075F_1BC2_BLOCK_SOURCE_STATE_UNBOUND;
    return 0;
}

const char *dm2_v1_DM2_move_075f_1bc2_source_evidence(void)
{
    return "skproject SKULLWIN/c_move.cpp:2861 DM2_move_075f_1bc2: "
           "builds one of four player-position candidates from party "
           "coordinates and DM2_RANDBIT; it is not a dungeon target/collision "
           "routine. The party-position and RNG owners are unbound, so this "
           "adapter rejects instead of inventing a move result.";
}
