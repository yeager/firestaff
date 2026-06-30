#include "dm1/dm1_v1_champion_panel_pending_damage_apply_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/*
 * Synthetic source-locked gate for the DM1 V1 F0320 pre-damage mutation
 * slice. See the header for the full ReDMCSB anchor list.
 *
 * Behaviour pinned by this file (all source-locked to ReDMCSB
 * CHAMPION.C F0320:1719-1740 + F0319:1552-1609 + DEFS.H:725,729-730
 * + COMPILE.H:1042 + BASE.C F0201 / TIMELINE.C F0254 / CHAMDRAW.C F0623
 * / CHAMDRAW.C F0292 surface conventions):
 *
 *  1. DM1_V1_ChampionPanelPendingDamageApply_BuildPc34Compat mutates a
 *     synthetic champion state once for the chosen champion index:
 *       - Reads input.attributes_before as the baseline
 *         M008_SET anchor.
 *       - Applies pending_wounds to champion->Wounds (recorded as
 *         wounds_applied) before the pending-damage guards. This is
 *         distinct from the Attributes bit MASK0x2000_WOUNDS, which
 *         F0320 only sets later on the live, non-lethal branch.
 *       - If pending_damage == 0: SKIPPED_NO_PENDING. The synthetic
 *         champion is unchanged; the F0320 outer loop simply
 *         continues to the next champion (F0320:1723-1725).
 *       - If current_health == 0 (or was_alive_before == false):
 *         SKIPPED_DEAD. F0320:1728-1729 early-continues; no draw or
 *         attribute mutation occurs. The previous damage fields are
 *         still wiped so the next iteration of F0320 sees a clean
 *         state.
 *       - (current_health - pending_damage) <= 0:
 *         KILLED_BY_F0319 (F0320:1733-1734). The synthetic recorder
 *         sets current_health_after = 0, alive_after = false,
 *         f0319_kill_called = true,
 *         and attributes_after = attributes_before |
 *         MASK0x1000_STATUS_BOX (F0319:1574 marks the status box
 *         for redraw on the next tick). F0623 + C12 schedule never
 *         run on this iteration; status_box_redraw_dispatched = true,
 *         mouse_screen_update_enabled = false,
 *         mouse_screen_update_disabled = false.
 *       - else: HEALTH_UPDATED. current_health_after =
 *         current_health_before - pending_damage,
 *         attributes_after = (attributes_before |
 *         MASK0x0100_STATISTICS) [F0320:1737 STATISTICS always set
 *         on the non-lethal branch],
 *         and if wounds_applied != 0 also MASK0x2000_WOUNDS
 *         [F0320:1738-1739 conditional].
 *         F0623 + C12 schedule run AFTER this gate returns; that is
 *         captured by mouse_screen_update_enabled = true (F0320:1741
 *         F0077_MOUSE_EnableScreenUpdate_CPSE for MEDIA009 PC 3.4).
 *       - attributes_orbit_mask is the set of bits the synthetic
 *         gate is allowed to set: MASK0x0100_STATISTICS |
 *         MASK0x2000_WOUNDS | MASK0x1000_STATUS_BOX. Pre-existing
 *         bits outside that orbit (e.g. MASK0x0200_LOAD,
 *         MASK0x0400_ICON, MASK0x0800_PANEL,
 *         MASK0x8000_ACTION_HAND) survive the apply unchanged.
 */

static const DM1_V1_ChampionPanelPendingDamageApplyEvidencePc34Compat
    s_evidence = {
        true,
        "CHAMPION.C F0320_CHAMPION_ApplyAndDrawPendingDamageAndWounds:1689-1800",
        "CHAMPION.C F0319_CHAMPION_Kill:1552-1609",
        "DEFS.H:729 MASK0x1000_STATUS_BOX",
        "COMPILE.H:1042 M008_SET(value, mask) ((value) |= (mask))",
        "DEFS.H:725 MASK0x0100_STATISTICS",
        "DEFS.H:729 MASK0x1000_STATUS_BOX",
        "DEFS.H:730 MASK0x2000_WOUNDS",
        "CHAMPION.C F0320:1719-1722 wounds apply + pending zero",
        "CHAMPION.C F0320:1728-1729 dead champion skip",
        "CHAMPION.C F0320:1731-1734 lethal branch + F0319 dispatch",
        "CHAMPION.C F0320:1735-1737 non-lethal health subtract + STATISTICS",
        "CHAMPION.C F0320:1738-1739 conditional WOUNDS set",
        "contract-only F0320 pre-damage mutation slice; no bitmap sampling",
        "without claiming real-asset parity"
    };

static const char s_source_evidence[] =
    "contract_only=1; CHAMPION.C F0320:1719-1720 enters the loop with "
    "L0967_ui_ChampionIndex=C00_CHAMPION_FIRST and L0971_ps_Champion="
    "M516_CHAMPIONS, looping while L0967_ui_ChampionIndex < "
    "G0305_ui_PartyChampionCount; F0320:1720 applies "
    "M008_SET(L0971_ps_Champion->Wounds, L0970_i_PendingWounds = "
    "G0410_ai_ChampionPendingWounds[L0967_ui_ChampionIndex]) and "
    "F0320:1722 zeros G0410[L0967_ui_ChampionIndex]; F0320:1723-1725 "
    "continues on (L0968_ui_PendingDamage = G0409[L0967]) == 0 and "
    "F0320:1726 zeros G0409[L0967]; F0320:1728-1729 continues on "
    "AL0969_i_Health == L0971_ps_Champion->CurrentHealth == 0 "
    "(already-dead champion); F0320:1731-1732 computes "
    "(currentHealth - pendingDamage) and F0320:1733-1734 calls "
    "F0319_CHAMPION_Kill(L0967_ui_ChampionIndex) when the result is "
    "<= 0; F0319:1569 sets CurrentHealth = 0 and F0319:1574 "
    "M008_SETs MASK0x1000_STATUS_BOX (the post-kill status-box "
    "redraw flag) and F0319:1592-1609 drops a C05_JUNK_BONES thing "
    "with ChargeCount = championIndex and DoNotDiscard=1; F0320:1735 "
    "writes back AL0969_i_Health to CurrentHealth, F0320:1737 always "
    "M008_SETs MASK0x0100_STATISTICS in Attributes on the "
    "non-lethal branch, and F0320:1738-1739 also M008_SETs "
    "MASK0x2000_WOUNDS when L0970_i_PendingWounds != 0; "
    "DEFS.H:725/729/730 document the bit masks; "
    "COMPILE.H:1042 expands M008_SET as ((value) |= (mask)); "
    "the pre-damage slice runs BEFORE F0623 (post-damage draw) and "
    "BEFORE the C12 schedule (F0320:1758-1792 F0238/F0236 timeline "
    "event), so F0319-dispatched champions never reach the F0623 "
    "box / C12 hide-damage event for that iteration; "
    "without claiming real-asset parity.";

const DM1_V1_ChampionPanelPendingDamageApplyEvidencePc34Compat *
DM1_V1_ChampionPanelPendingDamageApply_EvidencePc34Compat(void)
{
    return &s_evidence;
}

const char *
DM1_V1_ChampionPanelPendingDamageApply_SourceEvidencePc34Compat(void)
{
    return s_source_evidence;
}

void DM1_V1_ChampionPanelPendingDamageApply_DefaultInputPc34Compat(
    DM1_V1_ChampionPanelPendingDamageApplyInputPc34Compat *input)
{
    if (!input) {
        return;
    }

    memset(input, 0, sizeof(*input));
    input->champion_index = 0;
    input->pending_damage = 0;
    input->pending_wounds = 0;
    input->current_health = 100;
    input->was_alive_before = true;
    input->attributes_before = DM1_V1_CPDA_ATTR_NONE_PC34;
}

int DM1_V1_ChampionPanelPendingDamageApply_BuildPc34Compat(
    const DM1_V1_ChampionPanelPendingDamageApplyInputPc34Compat *input,
    DM1_V1_ChampionPanelPendingDamageApplyResultPc34Compat *out_result)
{
    DM1_V1_ChampionPanelPendingDamageApplyInputPc34Compat local_input;

    if (!out_result) {
        return 0;
    }

    memset(out_result, 0, sizeof(*out_result));
    out_result->contract_only = true;
    out_result->evidence = &s_evidence;
    out_result->attributes_orbit_mask =
        DM1_V1_CPDA_ATTR_STATISTICS_PC34 |
        DM1_V1_CPDA_ATTR_WOUNDS_PC34 |
        DM1_V1_CPDA_ATTR_STATUS_BOX_PC34;

    if (!input) {
        DM1_V1_ChampionPanelPendingDamageApply_DefaultInputPc34Compat(
            &local_input);
        input = &local_input;
    }

    out_result->champion_index = input->champion_index;
    out_result->champion_ordinal = input->champion_index + 1;
    out_result->pending_damage = input->pending_damage;
    out_result->pending_wounds = input->pending_wounds;
    out_result->current_health_before = input->current_health;
    out_result->was_alive_before = input->was_alive_before;
    out_result->attributes_before = input->attributes_before;

    /*
     * ReDMCSB CHAMPION.C F0320:1720-1721 leader guard.
     * Active party bound = DM1_V1_CPDA_CHAMPION_COUNT_PC34 (the four
     * champion panel cells; the exact G0305_ui_PartyChampionCount
     * bound only matters for a 4-deep party which is the canonical
     * DM1 PC 3.4 maximum).
     */
    if (input->champion_index < 0 ||
        input->champion_index >= DM1_V1_CPDA_CHAMPION_COUNT_PC34) {
        out_result->rejected_index = true;
        out_result->outcome =
            DM1_V1_CPDA_OUTCOME_REJECTED_INDEX_PC34;
        out_result->current_health_after = input->current_health;
        out_result->alive_after = input->was_alive_before;
        out_result->attributes_after = input->attributes_before;
        return 0;
    }

    if (input->pending_damage < 0 ||
        input->pending_damage > DM1_V1_CPDA_PENDING_DAMAGE_MAX_PC34) {
        /*
         * ReDMCSB CHAMPION.C F0320:1723-1725 reads G0409 as the
         * pending-damage accumulator; the synthetic gate mirrors the
         * documented int16 cap so callers don't have to model signed
         * wraparound separately.
         */
        out_result->rejected_pending_damage = true;
        out_result->outcome =
            DM1_V1_CPDA_OUTCOME_REJECTED_PENDING_DAMAGE_PC34;
        out_result->current_health_after = input->current_health;
        out_result->alive_after = input->was_alive_before;
        out_result->attributes_after = input->attributes_before;
        return 0;
    }

    /*
     * ReDMCSB CHAMPION.C F0320:1723-1725: no pending damage in the
     * accumulator -> F0320 continues to the next champion and the
         * CURRENT champion's Wounds are applied (F0320:1720-1722).
         * The Attributes bit MASK0x2000_WOUNDS is not reached because
         * it lives in the later non-lethal damage branch.
     */
    out_result->wounds_applied = input->pending_wounds;
    out_result->pending_damage_applied = 0;

    if (input->pending_damage == 0) {
        /*
         * ReDMCSB CHAMPION.C F0320:1723-1725 zero-pending-damage
         * early-return: the synthetic champion survives with wounds
         * applied to champion->Wounds, but no Attributes bit flips
         * because the STATISTICS/WOUNDS attribute updates live after
         * the pending_damage and current_health guards.
         */
        out_result->outcome =
            DM1_V1_CPDA_OUTCOME_SKIPPED_NO_PENDING_PC34;
        out_result->current_health_after = input->current_health;
        out_result->alive_after = input->was_alive_before;
        out_result->attributes_after = input->attributes_before;
        out_result->valid = true;
        return 1;
    }

    /*
     * ReDMCSB CHAMPION.C F0320:1728-1729 dead-champion skip:
     * if CurrentHealth == 0 (or the caller's was_alive_before is
     * false), F0320 early-continues without applying or drawing
     * anything for that iteration. The pending damage is still
     * wiped (F0320:1726 G0409[i] = 0), wounds are still mounted
     * (F0320:1720-1722), and the later Attributes bits are not reached.
     */
    if (input->current_health <= 0 || !input->was_alive_before) {
        out_result->outcome =
            DM1_V1_CPDA_OUTCOME_SKIPPED_DEAD_PC34;
        out_result->current_health_after = 0;
        out_result->alive_after = false;
        out_result->attributes_after = input->attributes_before;
        out_result->valid = true;
        return 1;
    }

    /*
     * ReDMCSB CHAMPION.C F0320:1731-1737 + F0319:1552-1609.
     * Lethal: (currentHealth - pendingDamage) <= 0 ->
     * F0319_CHAMPION_Kill dispatch. CurrentHealth = 0 (F0319:1569),
     * MASK0x1000_STATUS_BOX set (F0319:1574), bones dropped
     * (F0319:1592-1609 with ChargeCount = championIndex,
     * DoNotDiscard=1).
     *
     * The synthetic recorder does not invoke the real
     * F0319_CHAMPION_Kill function - it records the dispatch
     * decision so a focused unit/probe can verify the boundary
     * contract without driving the full inventory / drop-all /
     * imagery coverage of F0319.
     */
    if (input->pending_damage >= input->current_health) {
        out_result->outcome =
            DM1_V1_CPDA_OUTCOME_KILLED_BY_F0319_PC34;
        out_result->f0319_kill_called = true;
        out_result->status_box_redraw_dispatched = true;
        out_result->pending_damage_applied = input->pending_damage;
        out_result->current_health_after = 0;
        out_result->alive_after = false;
        out_result->attributes_after =
            input->attributes_before | DM1_V1_CPDA_ATTR_STATUS_BOX_PC34;
        out_result->valid = true;
        return 1;
    }

    /*
     * ReDMCSB CHAMPION.C F0320:1735-1739 non-lethal branch.
     * currentHealth -= pendingDamage (F0320:1736),
     * M008_SET(Attributes, MASK0x0100_STATISTICS) always
     * (F0320:1737), and conditionally M008_SET(Attributes,
     * MASK0x2000_WOUNDS) when pendingWounds != 0 (F0320:1738-1739).
     * Post-mutation, F0623 / C12 schedule run on this iteration,
     * so mouse_screen_update_enabled = true (F0320:1741
     * F0077_MOUSE_EnableScreenUpdate_CPSE for MEDIA009 PC 3.4).
     */
    out_result->outcome =
        DM1_V1_CPDA_OUTCOME_HEALTH_UPDATED_PC34;
    out_result->pending_damage_applied = input->pending_damage;
    out_result->current_health_after =
        input->current_health - input->pending_damage;
    out_result->alive_after = true;
    out_result->attributes_after =
        input->attributes_before |
        DM1_V1_CPDA_ATTR_STATISTICS_PC34 |
        ((input->pending_wounds != 0)
             ? DM1_V1_CPDA_ATTR_WOUNDS_PC34
             : DM1_V1_CPDA_ATTR_NONE_PC34);
    out_result->mouse_screen_update_enabled = true;
    out_result->valid = true;
    return 1;
}
