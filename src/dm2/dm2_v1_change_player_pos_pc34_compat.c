/* DM2_CHANGE_PLAYER_POS — c_hero.cpp:4037.
 * Kept in the production DM2 source set because the M11 runtime calls it. */
#include "dm2_v1_change_player_pos_pc34_compat.h"

/* DM2_CHANGE_PLAYER_POS — c_hero.cpp:4037 */
void dm2_v1_change_player_pos(
    int16_t packed_pos, const DM2_V1_ChangePlayerPosState *state,
    const DM2_V1_ChangePlayerPosCallbacks *cb, void *ctx)
{
    int source_slot;
    int target_slot;
    int source_hero;
    int target_hero;
    int source_position;
    int target_position;
    int8_t facing;

    if (!cb || !state || !cb->get_player_at_position ||
        !cb->get_party_facing)
        return;

    facing = cb->get_party_facing(ctx);
    if (facing < 0 || facing > 3)
        return;
    target_slot = packed_pos & 0x7fff;
    if ((packed_pos & (int16_t)0x8000) != 0)
        target_slot &= 3;
    /* Source UI events 125..129 encode slots 0..4. Slot 4 wraps to
     * formation position 0 for lookup but remains distinct in the pending
     * slot comparison performed by CHANGE_PLAYER_POS. */
    if (target_slot < 0 || target_slot > 4)
        return;

    target_position = (target_slot + facing) & 3;
    target_hero = cb->get_player_at_position(ctx, target_position);
    if (!state->drag_in_progress) {
        /* CHANGE_PLAYER_POS first paints the selected champion as the
         * platform cursor and records v1e00b0/v1e00b8. The formation itself
         * remains untouched until the destination event arrives. */
        if (target_hero >= 0 && cb->begin_drag)
            cb->begin_drag(ctx, target_slot);
        return;
    }

    source_slot = state->drag_source_position;
    if (source_slot < 0 || source_slot > 4)
        return;
    if (!cb->clear_drag || !cb->set_position ||
        (((packed_pos & (int16_t)0x8000) == 0) && !cb->set_direction))
        return;
    source_position = (source_slot + facing) & 3;
    source_hero = cb->get_player_at_position(ctx, source_position);

    /* The source clears v1e00b8 before looking up the dragged hero or
     * mutating either party position. */
    cb->clear_drag(ctx);
    if (source_hero < 0)
        return;

    if ((packed_pos & (int16_t)0x8000) == 0)
        cb->set_direction(ctx, source_hero, facing);
    if (source_slot == target_slot)
        return;

    if (target_hero >= 0)
        cb->set_position(ctx, target_hero, (int8_t)source_position);
    cb->set_position(ctx, source_hero, (int8_t)target_position);
}
