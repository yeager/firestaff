#include "dm1/dm1_v1_champion_panel_hud_damage_attribute_cascade_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static const DM1_V1_ChampionPanelHudDamageAttributeCascadeEvidencePc34Compat
    s_evidence = {
        true,
        "CHAMPION.C F0320_CHAMPION_ApplyAndDrawPendingDamageAndWounds:1720-1792",
        "CHAMPION.C F0320:1738 M008_SET MASK0x0100_STATISTICS",
        "CHAMPION.C F0320:1740 M008_SET MASK0x2000_WOUNDS when wounds != 0",
        "CHAMPION.C F0320:1724-1725 G0409[i] == 0 continue",
        "CHAMPION.C F0320:1726-1727 CurrentHealth == 0 continue",
        "CHAMDRAW.C F0292_CHAMPION_DrawState:755-1115",
        "CHAMDRAW.C F0292:757 M007_GET short-circuit on nine redraw bits",
        "CHAMDRAW.C F0292:898 M007_GET MASK0x0100_STATISTICS -> HP/Stam/Mana repaint",
        "CHAMDRAW.C F0292:937 M007_GET MASK0x2000_WOUNDS -> F0291 slot sweep",
        "CHAMDRAW.C F0292:1110 M009_CLEAR nine-bit Attributes mask",
        "DEFS.H:724-732 MASK0x0080_NAME_TITLE..MASK0x8000_ACTION_HAND",
        "contract-only F0320->F0292 attribute cascade; no bitmap sampling; "
        "without claiming real-asset parity"
    };

static const char s_source_evidence[] =
    "contract_only=1; CHAMPION.C F0320:1722 M008_SET Champion->Wounds |= "
    "pendingWounds then G0410[i]=0; CHAMPION.C F0320:1724-1725 skips the "
    "champion when G0409[i] is 0; CHAMPION.C F0320:1726-1727 skips the "
    "champion when CurrentHealth is 0 (F0319_CHAMPION_Kill owns that "
    "branch); CHAMPION.C F0320:1738 M008_SET Attributes MASK0x0100_STATISTICS "
    "after CurrentHealth -= damage; CHAMPION.C F0320:1740 M008_SET Attributes "
    "MASK0x2000_WOUNDS only when pendingWounds != 0; CHAMPION.C F0320:1778 "
    "calls F0623_DrawDamageToChampion_F0320_sub which calls F0292_CHAMPION_"
    "DrawState at line 698; CHAMDRAW.C F0292:757 short-circuits when none of "
    "MASK0x0080_NAME_TITLE | MASK0x0100_STATISTICS | MASK0x0200_LOAD | "
    "MASK0x0400_ICON | MASK0x0800_PANEL | MASK0x1000_STATUS_BOX | "
    "MASK0x2000_WOUNDS | MASK0x4000_VIEWPORT | MASK0x8000_ACTION_HAND are "
    "set; CHAMDRAW.C F0292:898 reads MASK0x0100_STATISTICS -> HP/Stam/Mana "
    "bar repaint; CHAMDRAW.C F0292:937 reads MASK0x2000_WOUNDS -> "
    "F0291_CHAMPION_DrawSlot sweep over C00..C05; CHAMDRAW.C F0292:1110 "
    "M009_CLEAR clears all nine Attributes bits so the next tick does "
    "not double-draw; without claiming real-asset parity.";

const DM1_V1_ChampionPanelHudDamageAttributeCascadeEvidencePc34Compat *
DM1_V1_ChampionPanelHudDamageAttributeCascade_EvidencePc34Compat(void)
{
    return &s_evidence;
}

const char *
DM1_V1_ChampionPanelHudDamageAttributeCascade_SourceEvidencePc34Compat(void)
{
    return s_source_evidence;
}

void DM1_V1_ChampionPanelHudDamageAttributeCascade_DefaultInputPc34Compat(
    DM1_V1_ChampionPanelHudDamageAttributeCascadeInputPc34Compat *input)
{
    if (!input) {
        return;
    }

    memset(input, 0, sizeof(*input));
    input->champion_index = 0;
    input->current_health = 100;
    input->pending_damage = 5;
    input->pending_wounds_mask = 0;
    input->alive = true;
    input->is_inventory_champion = false;
    input->party_is_resting = false;
}

static int cphudac_popcount_wound_mask(int wound_mask)
{
    int count = 0;
    int slot;

    /*
     * ReDMCSB CHAMPION.C F0321:1847-1851 walks the C00..C05 wound slot
     * range and counts the enabled bits to drive defense averaging and
     * F0313 armor lookups. This helper mirrors that count for the
     * synthetic gate; F0321 itself is owned by memory_combat_pc34_compat.
     */
    for (slot = DM1_V1_CPHUDAC_SLOT_READY_HAND_PC34;
         slot <= DM1_V1_CPHUDAC_SLOT_FEET_PC34;
         ++slot) {
        if (wound_mask & (1 << slot)) {
            ++count;
        }
    }
    return count;
}

static int cphudac_first_wound_slot(int wound_mask)
{
    int slot;
    for (slot = DM1_V1_CPHUDAC_SLOT_READY_HAND_PC34;
         slot <= DM1_V1_CPHUDAC_SLOT_FEET_PC34;
         ++slot) {
        if (wound_mask & (1 << slot)) {
            return slot;
        }
    }
    return -1;
}

static int cphudac_last_wound_slot(int wound_mask)
{
    int slot;
    for (slot = DM1_V1_CPHUDAC_SLOT_FEET_PC34;
         slot >= DM1_V1_CPHUDAC_SLOT_READY_HAND_PC34;
         --slot) {
        if (wound_mask & (1 << slot)) {
            return slot;
        }
    }
    return -1;
}

int DM1_V1_ChampionPanelHudDamageAttributeCascade_ApplyPc34Compat(
    const DM1_V1_ChampionPanelHudDamageAttributeCascadeInputPc34Compat *input,
    DM1_V1_ChampionPanelHudDamageAttributeCascadeResultPc34Compat *out_result)
{
    DM1_V1_ChampionPanelHudDamageAttributeCascadeInputPc34Compat local_input;
    int attributes_after;
    int wounds_after;
    int health_after;
    int wound_count;
    bool f0320_set_statistics;
    bool f0320_set_wounds;
    int redraw_kind;

    if (!out_result) {
        return 0;
    }

    memset(out_result, 0, sizeof(*out_result));
    out_result->contract_only = true;
    out_result->evidence = &s_evidence;
    out_result->wound_slot_redraw_first = -1;
    out_result->wound_slot_redraw_last = -1;

    if (!input) {
        DM1_V1_ChampionPanelHudDamageAttributeCascade_DefaultInputPc34Compat(
            &local_input);
        input = &local_input;
    }

    if (input->champion_index < 0 ||
        input->champion_index >= DM1_V1_CPHUDAC_CHAMPION_COUNT_PC34) {
        /*
         * CHAMPION.C F0320:1721 for-loop bound: champion_index is in
         * [0, partyChampionCount) which is bounded by the four
         * champion panel cells.
         */
        out_result->rejected_champion_index = true;
        return 0;
    }

    if (input->current_health < 0) {
        /*
         * CHAMPION.C F0320 reads CurrentHealth which the original code
         * maintains as a non-negative int16 (F0319 sets it to 0 on
         * death, F0315 heals it up to CurrentHealthMax).
         */
        out_result->rejected_negative_health = true;
        return 0;
    }

    out_result->valid = true;
    out_result->champion_index = input->champion_index;
    out_result->champion_ordinal = input->champion_index + 1;

    /*
     * CHAMPION.C F0320:1724-1725: when G0409[i] == 0 the iteration
     * continues and no attribute bits are set on this champion.
     */
    if (input->pending_damage == 0) {
        out_result->outcome =
            DM1_V1_CPHUDAC_OUTCOME_PENDING_DAMAGE_ZERO_PC34;
        out_result->skipped_pending_damage_zero = true;
        out_result->attributes_after_apply = DM1_V1_CPHUDAC_ATTRIBUTES_NONE_PC34;
        out_result->wound_bits_after_apply = 0;
        out_result->wound_count_after_apply = 0;
        out_result->health_after_apply = input->current_health;
        out_result->redraw = DM1_V1_CPHUDAC_REDRAW_NONE_PC34;
        out_result->f0292_will_short_circuit = true;
        return 1;
    }

    /*
     * CHAMPION.C F0320:1726-1727: when CurrentHealth == 0 the
     * champion is already dead (F0319_CHAMPION_Kill owns the kill
     * path); no attribute bits are set here.
     */
    if (!input->alive || input->current_health <= 0) {
        out_result->outcome = DM1_V1_CPHUDAC_OUTCOME_DEAD_CHAMPION_PC34;
        out_result->skipped_dead_champion = true;
        out_result->attributes_after_apply = DM1_V1_CPHUDAC_ATTRIBUTES_NONE_PC34;
        out_result->wound_bits_after_apply = 0;
        out_result->wound_count_after_apply = 0;
        out_result->health_after_apply = input->current_health;
        out_result->redraw = DM1_V1_CPHUDAC_REDRAW_NONE_PC34;
        out_result->f0292_will_short_circuit = true;
        return 1;
    }

    /*
     * CHAMPION.C F0320:1737: CurrentHealth -= pendingDamage (clamped
     * at 0 by the lethal branch above). When the resulting health
     * would be <= 0 the original F0320 enters F0319_CHAMPION_Kill;
     * the synthetic gate continues with CurrentHealth=0 and does NOT
     * set STATISTICS / WOUNDS (the kill branch owns the redraw).
     */
    health_after = input->current_health - input->pending_damage;
    if (health_after <= 0) {
        out_result->outcome = DM1_V1_CPHUDAC_OUTCOME_DEAD_CHAMPION_PC34;
        out_result->skipped_dead_champion = true;
        out_result->attributes_after_apply = DM1_V1_CPHUDAC_ATTRIBUTES_NONE_PC34;
        out_result->wound_bits_after_apply = 0;
        out_result->wound_count_after_apply = 0;
        out_result->health_after_apply = 0;
        out_result->redraw = DM1_V1_CPHUDAC_REDRAW_NONE_PC34;
        out_result->f0292_will_short_circuit = true;
        return 1;
    }

    /*
     * CHAMPION.C F0320:1722: M008_SET Champion->Wounds |= pendingWounds
     * then G0410[i] = 0; pendingWounds is a 6-bit mask for slots
     * C00..C05 (ready hand / action hand / head / torso / legs / feet).
     */
    wounds_after = input->pending_wounds_mask &
        ((1 << DM1_V1_CPHUDAC_WOUND_COUNT_MAX_PC34) - 1);

    /*
     * CHAMPION.C F0320:1738: M008_SET(Attributes, MASK0x0100_STATISTICS)
     * is unconditional after a nonlethal damage application.
     */
    f0320_set_statistics = true;
    attributes_after = DM1_V1_CPHUDAC_MASK0x0100_STATISTICS_PC34;

    /*
     * CHAMPION.C F0320:1740: M008_SET(Attributes, MASK0x2000_WOUNDS)
     * only when pendingWounds != 0.
     */
    f0320_set_wounds = (wounds_after != 0);
    if (f0320_set_wounds) {
        attributes_after |= DM1_V1_CPHUDAC_MASK0x2000_WOUNDS_PC34;
    }

    wound_count = cphudac_popcount_wound_mask(wounds_after);

    out_result->applied_damage = true;
    out_result->set_statistics_bit = f0320_set_statistics;
    out_result->set_wounds_bit = f0320_set_wounds;
    out_result->attributes_after_apply = attributes_after;
    out_result->wound_bits_after_apply = wounds_after;
    out_result->wound_count_after_apply = wound_count;
    out_result->health_after_apply = health_after;
    out_result->outcome = f0320_set_wounds
        ? DM1_V1_CPHUDAC_OUTCOME_DAMAGE_AND_WOUNDS_PC34
        : DM1_V1_CPHUDAC_OUTCOME_DAMAGE_ONLY_PC34;

    /*
     * CHAMDRAW.C F0292:898 reads MASK0x0100_STATISTICS to repaint
     * HP / Stamina / Mana bars, F0292:937 reads MASK0x2000_WOUNDS
     * to drive F0291 slot sweeps. With both bits set the synthetic
     * gate records REDRAW_STATISTICS_AND_WOUND_SLOTS; with only the
     * STATISTICS bit set it records REDRAW_STATISTICS.
     */
    if (f0320_set_statistics && f0320_set_wounds) {
        redraw_kind = DM1_V1_CPHUDAC_REDRAW_STATISTICS_AND_WOUND_SLOTS_PC34;
    } else if (f0320_set_statistics) {
        redraw_kind = DM1_V1_CPHUDAC_REDRAW_STATISTICS_PC34;
    } else {
        redraw_kind = DM1_V1_CPHUDAC_REDRAW_NONE_PC34;
    }
    out_result->redraw = redraw_kind;

    /*
     * CHAMDRAW.C F0292:757 short-circuit predicate (nine-bit mask).
     * A non-empty MASK0x0100_STATISTICS guarantees the redraw fires.
     */
    out_result->f0292_will_short_circuit =
        (attributes_after & 0xFF80u) == 0u;
    out_result->f0292_will_redraw_statistics = f0320_set_statistics;
    out_result->f0292_will_redraw_wounds =
        f0320_set_wounds &&
        ((attributes_after & DM1_V1_CPHUDAC_MASK0x2000_WOUNDS_PC34) != 0);

    /*
     * CHAMDRAW.C F0292:937 anchor: wound-slot redraw sweep iterates
     * C00..C05; the inventory champion walks C00..C05 with feet as
     * the topmost slot, the non-inventory champion walks C00..C01
     * (ready hand + action hand) per the IsInventoryChampion branch.
     */
    if (f0320_set_wounds) {
        if (input->is_inventory_champion) {
            out_result->wound_slot_redraw_first =
                cphudac_first_wound_slot(wounds_after);
            out_result->wound_slot_redraw_last =
                cphudac_last_wound_slot(wounds_after);
        } else {
            /*
             * Non-inventory champion: F0292 redraws the whole visible
             * hand-slot range C01..C00 when MASK0x2000_WOUNDS is set,
             * even if the actual wound bit lives on a hidden body slot.
             */
            out_result->wound_slot_redraw_first =
                DM1_V1_CPHUDAC_SLOT_READY_HAND_PC34;
            out_result->wound_slot_redraw_last =
                DM1_V1_CPHUDAC_SLOT_ACTION_HAND_PC34;
        }
    }

    /*
     * CHAMDRAW.C F0292:1110 M009_CLEAR clears all nine bits so the
     * next F0292 call on this champion does not double-draw.
     */
    out_result->f0292_will_clear_after_redraw = true;

    return 1;
}

int DM1_V1_ChampionPanelHudDamageAttributeCascade_RedrawPc34Compat(
    const DM1_V1_ChampionPanelHudDamageAttributeCascadeResultPc34Compat *state,
    DM1_V1_ChampionPanelHudDamageAttributeCascadeResultPc34Compat *out_result)
{
    if (!out_result) {
        return 0;
    }

    memset(out_result, 0, sizeof(*out_result));
    out_result->contract_only = true;
    out_result->evidence = &s_evidence;
    out_result->wound_slot_redraw_first = -1;
    out_result->wound_slot_redraw_last = -1;

    if (!state) {
        /*
         * CHAMDRAW.C F0292:757 short-circuit: when Attributes has
         * no redraw bits, F0292 returns without drawing or clearing.
         */
        out_result->outcome =
            DM1_V1_CPHUDAC_OUTCOME_PENDING_DAMAGE_ZERO_PC34;
        out_result->attributes_after_apply = DM1_V1_CPHUDAC_ATTRIBUTES_NONE_PC34;
        out_result->redraw = DM1_V1_CPHUDAC_REDRAW_NONE_PC34;
        out_result->f0292_will_short_circuit = true;
        out_result->valid = true;
        out_result->champion_index = -1;
        out_result->champion_ordinal = 0;
        return 1;
    }

    if (!state->valid) {
        out_result->rejected_null_output = true;
        return 0;
    }

    *out_result = *state;
    out_result->evidence = &s_evidence;

    /*
     * CHAMDRAW.C F0292:1110 M009_CLEAR clears all nine bits after
     * the redraw; the next F0292 call on this champion will
     * short-circuit at line 757.
     */
    out_result->attributes_after_apply = DM1_V1_CPHUDAC_ATTRIBUTES_NONE_PC34;
    out_result->f0292_will_clear_after_redraw = true;
    out_result->f0292_will_short_circuit = true;
    out_result->redraw = DM1_V1_CPHUDAC_REDRAW_FULL_REDRAW_MASK_PC34;
    return 1;
}
