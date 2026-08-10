#include "csb_v1_f0070_champion_formation_pc34_compat.h"

#include <string.h>

static int champion_at_cell(const CsbV1F0070ChampionFormationStatePc34 *state,
                            int cell)
{
    int champion;
    for (champion = 0; champion < state->champion_count; ++champion) {
        if (state->cell[champion] == (uint8_t)cell) return champion;
    }
    return -1;
}

static int state_is_valid(const CsbV1F0070ChampionFormationStatePc34 *state)
{
    int champion;
    if (!state || state->champion_count < 1 ||
        state->champion_count > CSB_V1_F0070_CHAMPION_CAPACITY_PC34 ||
        state->party_direction < 0 || state->party_direction > 3 ||
        state->held_icon_ordinal > CSB_V1_F0070_CHAMPION_CAPACITY_PC34) {
        return 0;
    }
    for (champion = 0; champion < state->champion_count; ++champion) {
        int other;
        if (state->cell[champion] > 3U || state->direction[champion] > 3U)
            return 0;
        for (other = champion + 1; other < state->champion_count; ++other) {
            if (state->cell[champion] == state->cell[other]) return 0;
        }
    }
    return 1;
}

int csb_v1_f0070_champion_formation_click_pc34(
    CsbV1F0070ChampionFormationStatePc34 *state,
    int target_icon_index,
    CsbV1F0070ChampionFormationReceiptPc34 *out_receipt)
{
    CsbV1F0070ChampionFormationStatePc34 next;
    int target_cell;
    int target_champion;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->source_evidence =
        csb_v1_f0070_champion_formation_source_evidence_pc34();
    if (!state_is_valid(state) || target_icon_index < 0 || target_icon_index > 3)
        return 0;

    next = *state;
    target_cell = (target_icon_index + next.party_direction) & 3;
    target_champion = champion_at_cell(&next, target_cell);
    out_receipt->target_icon_index = target_icon_index;
    out_receipt->target_champion_index = target_champion;

    if (next.held_icon_ordinal == 0U) {
        /* IO.C:2400-2407: only an occupied target may become the pointer. */
        if (target_champion < 0) return 0;
        next.held_icon_ordinal = (unsigned int)target_icon_index + 1U;
        out_receipt->accepted = 1;
        out_receipt->picked_up = 1;
        out_receipt->source_icon_suppressed = 1;
        out_receipt->source_icon_index = target_icon_index;
        out_receipt->source_champion_index = target_champion;
        *state = next;
        return 1;
    }

    {
        int source_icon_index = (int)next.held_icon_ordinal - 1;
        int source_cell = (source_icon_index + next.party_direction) & 3;
        int source_champion = champion_at_cell(&next, source_cell);

        /* IO.C:2585-2599 reads this champion before mutating any cell. */
        if (source_champion < 0) return 0;
        out_receipt->source_icon_index = source_icon_index;
        out_receipt->source_champion_index = source_champion;
        next.held_icon_ordinal = 0U;
        next.direction[source_champion] = (uint8_t)next.party_direction;
        next.attributes[source_champion] |= CSB_V1_F0070_ATTRIBUTE_ICON_DIRTY_PC34;

        if (source_icon_index == target_icon_index) {
            /* IO.C:2600-2608 redraws the source icon but does not move it. */
        } else if (target_champion >= 0) {
            /* IO.C:2609-2616 moves the target into the vacated source cell. */
            next.cell[target_champion] = (uint8_t)source_cell;
            next.attributes[target_champion] |=
                CSB_V1_F0070_ATTRIBUTE_ICON_DIRTY_PC34;
            out_receipt->swapped_with_occupant = 1;
        } else {
            /* IO.C:2617-2625 clears the former icon zone; no host raster. */
            out_receipt->moved_to_empty_cell = 1;
            out_receipt->source_icon_cleared = 1;
        }
        if (source_icon_index != target_icon_index)
            next.cell[source_champion] = (uint8_t)target_cell;
        out_receipt->accepted = 1;
        out_receipt->released = 1;
        *state = next;
        return 1;
    }
}

const char *csb_v1_f0070_champion_formation_source_evidence_pc34(void)
{
    return "ReDMCSB COMMAND.C F0380:2164-2170 dispatches C125-C128 directly "
           "to IO.C F0070; IO.C F0070:2395-2647 picks up C028 icon state and "
           "IO.C:2585-2633 updates M516_CHAMPIONS Cell/Direction/0x0400 ICON. "
           "F0378 handles only C081 panel clicks and is not in this route.";
}
