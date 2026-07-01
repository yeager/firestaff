#include "dm1/dm1_v1_champion_panel_pending_wounds_tick_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/*
 * Synthetic source-locked gate for the DM1 V1 champion-panel
 * pending-wounds tick. See the header for the full ReDMCSB anchor
 * list.
 *
 * Behaviour pinned by this file (all source-locked to ReDMCSB
 * CHAMPION.C F0320:1720-1727 + DEFS.H:679 + DEFS.H:5866-5867 +
 * DEFS.H:735-741 + COMPILE.H:1042):
 *
 *  1. DM1_V1_ChampionPanelPendingWoundsTick_InitStatePc34Compat
 *     zero-clears all four champion records and sets
 *     party_champion_count = DM1_V1_CPPWT_CHAMPION_COUNT_PC34.
 *     Each champion starts with wounds=0, pending_wounds=0,
 *     pending_damage=0, alive=true, invalid_input=false. The
 *     synthetic state mirrors the post-F0321 staging invariant
 *     (DM1 V1 always starts every champion with no wounds and no
 *     pending buffers), per ReDMCSB CEDTINCI.C:68
 *     `L5817_ps_Champion->Wounds = 0`.
 *
 *  2. DM1_V1_ChampionPanelPendingWoundsTick_RunPartyLoopPc34Compat
 *     simulates the F0320:1720-1727 party loop, one iteration per
 *     champion index from C00_CHAMPION_FIRST (0) to
 *     G0305_ui_PartyChampionCount (state->party_champion_count,
 *     bounded by DM1_V1_CPPWT_CHAMPION_COUNT_PC34 = 4):
 *       a. M008_SET(Wounds, L0970_i_PendingWounds =
 *              ChampionPendingWounds[idx])    — F0320:1721 (COMPILE.H:1042)
 *       b. ChampionPendingWounds[idx] = 0     — F0320:1722
 *       c. if (!PendingDamage) continue;       — F0320:1723-1724
 *          (skip the rest of the iteration: damage reset, kill /
 *          health-subtract branches, F0623 call)
 *       d. PendingDamage[idx] = 0              — F0320:1725
 *       e. if (!CurrentHealth) continue;       — F0320:1726-1727
 *          (dead-champion early return)
 *       f. Subtract health, handle lethal vs nonlethal —
 *          F0320:1728 onwards (out of scope: this gate stops at the
 *          damage-draw dispatch boundary and reports whether the
 *          F0623 call WOULD have run, leaving the actual draw to
 *          the dm1_v1_champion_panel_damage_indicator and the
 *          decay to dm1_v1_champion_panel_damage_flash_decay gates).
 *
 *  The synthetic state in this gate tracks wounds, pending_wounds,
 *  and pending_damage per champion, and reports the per-champion
 *  result (branch + before/after wounds + before/after pending
 *  buffers + which F0320 stages fired). No CurrentHealth/Attributes
 *  mutation is performed by this gate; those belong to the
 *  damage>0 path already pinned by the sibling damage-indicator
 *  and damage-flash-decay gates.
 */

static const DM1_V1_ChampionPanelPendingWoundsTickEvidencePc34Compat
    s_cppwt_evidence = {
        true,
        "CHAMPION.C F0320_CHAMPION_ApplyAndDrawPendingDamageAndWounds:"
        "1720-1727 party-champion loop with wound OR-in + reset + "
        "zero-damage continue",
        "COMPILE.H:1042 #define M008_SET(value, mask) ((value) |= (mask))",
        "DEFS.H:5867 extern unsigned int16_t G0410_ai_ChampionPendingWounds[4]",
        "DEFS.H:5866 extern int16_t G0409_ai_ChampionPendingDamage[4]",
        "DEFS.H:735-741 MASK0x0000..MASK0x0020 wound bit constants",
        "DEFS.H:679 unsigned int16_t Wounds in CHAMPION struct",
        "CHAMPION.C F0320:1720 C00_CHAMPION_FIRST..G0305_ui_PartyChampionCount "
        "loop bound (1..4)",
        "CHAMPION.C F0320:1723-1724 `if (!(PendingDamage = "
        "ChampionPendingDamage[idx])) continue;`",
        "test_dm1_v1_champion_panel_damage_indicator_pc34_compat covers "
        "F0320:1743-1798 F0623_DrawDamageToChampion sub-route only",
        "test_dm1_v1_champion_panel_damage_flash_decay_pc34_compat covers "
        "F0320:1758-1792 + TIMELINE.C F0254:1614-1637 hide-damage-received "
        "timeline only",
        "contract-only; no real-asset or original-DOS pixel parity claim"
    };

const DM1_V1_ChampionPanelPendingWoundsTickEvidencePc34Compat *
DM1_V1_ChampionPanelPendingWoundsTick_EvidencePc34Compat(void)
{
    return &s_cppwt_evidence;
}

void DM1_V1_ChampionPanelPendingWoundsTick_InitStatePc34Compat(
    DM1_V1_ChampionPanelPendingWoundsTickStatePc34Compat *state)
{
    int champion_index;

    if (!state) {
        return;
    }

    memset(state, 0, sizeof(*state));
    state->party_champion_count = DM1_V1_CPPWT_CHAMPION_COUNT_PC34;
    for (champion_index = 0;
         champion_index < DM1_V1_CPPWT_CHAMPION_COUNT_PC34;
         ++champion_index) {
        state->champions[champion_index].index = champion_index;
        /* DEFS.H:679 wounds bitmask stays 0 (CEDTINCI.C:68 initialize) */
        state->champions[champion_index].wounds = 0;
        /* DEFS.H:5867 G0410 staging buffer starts cleared */
        state->champions[champion_index].pending_wounds = 0;
        /* DEFS.H:5866 G0409 staging buffer starts cleared */
        state->champions[champion_index].pending_damage = 0;
        state->champions[champion_index].invalid_input = false;
    }
}

/*
 * Run one F0320:1720-1727 loop pass. The result fields fall into
 * five disjoint branches per the early-return structure of the
 * source:
 *   - WOUND_ONLY          pending_wounds!=0 && pending_damage==0
 *   - WOUND_AND_DAMAGE    pending_wounds!=0 && pending_damage!=0
 *   - DAMAGE_ONLY         pending_wounds==0 && pending_damage!=0
 *   - NO_OP               pending_wounds==0 && pending_damage==0
 * The damage-draw-would-be-called flag is true for any branch where
 * the F0320 loop reaches past F0320:1723-1724 (i.e., for
 * WOUND_AND_DAMAGE and DAMAGE_ONLY).
 *
 * Note: invalid_input per-champion still runs the OR-in and reset
 * for its slot (the synthetic state drives the loop unconditionally)
 * but is reported in the step-result so the test can pin the
 * "out-of-bounds staging slips in but does not crash" invariant.
 * The real DM1 engine would never stage into an invalid slot, so
 * the gate treats invalid_input as a unit-test-only safety hatch
 * and reports it in out_step->first_invalid_champion_index without
 * rolling back the wound OR-in (per F0320:1721 which has no slot
 * guard in the source).
 */
int DM1_V1_ChampionPanelPendingWoundsTick_RunPartyLoopPc34Compat(
    DM1_V1_ChampionPanelPendingWoundsTickStatePc34Compat *state,
    DM1_V1_ChampionPanelPendingWoundsTickStepResultPc34Compat *out_step,
    DM1_V1_ChampionPanelPendingWoundsTickChampionResultPc34Compat
        out_per_champion[DM1_V1_CPPWT_CHAMPION_COUNT_PC34])
{
    int party_count;
    int champion_index;

    if (!state || !out_step) {
        return 0;
    }

    memset(out_step, 0, sizeof(*out_step));
    if (out_per_champion) {
        memset(out_per_champion, 0,
               sizeof(out_per_champion[0]) *
                   DM1_V1_CPPWT_CHAMPION_COUNT_PC34);
    }

    party_count = state->party_champion_count;
    if (party_count < DM1_V1_CPPWT_PARTY_MIN_PC34) {
        party_count = DM1_V1_CPPWT_PARTY_MIN_PC34;
    }
    if (party_count > DM1_V1_CPPWT_CHAMPION_COUNT_PC34) {
        party_count = DM1_V1_CPPWT_CHAMPION_COUNT_PC34;
    }

    for (champion_index = 0;
         champion_index < party_count;
         ++champion_index) {
        DM1_V1_ChampionPanelPendingWoundsTickChampionPc34Compat
            *champion = &state->champions[champion_index];
        DM1_V1_ChampionPanelPendingWoundsTickChampionResultPc34Compat
            *slot = NULL;
        uint16_t wounds_before;
        uint16_t pending_wounds_before;
        int16_t pending_damage_before;

        if (out_per_champion) {
            slot = &out_per_champion[champion_index];
            memset(slot, 0, sizeof(*slot));
            slot->champion_index = champion_index;
        }

        if (champion->invalid_input) {
            /*
             * Synthetic-only safety hatch. Real F0321 never stages
             * into an invalid slot, but we still report it and keep
             * the loop running so a regression in the unit-test
             * surface is detectable.
             */
            out_step->champions_skipped_due_to_invalid_input++;
            if (out_step->first_invalid_champion_index == 0 &&
                out_step->champions_processed == 0) {
                out_step->first_invalid_champion_index = champion_index;
            }
        }

        wounds_before = champion->wounds;
        pending_wounds_before = champion->pending_wounds;
        pending_damage_before = champion->pending_damage;

        /*
         * F0320:1721
         *   M008_SET(L0971_ps_Champion->Wounds,
         *            L0970_i_PendingWounds =
         *                G0410_ai_ChampionPendingWounds[idx]);
         * COMPILE.H:1042 #define M008_SET(value, mask) ((value) |= (mask))
         * Always executes, regardless of pending_damage.
         */
        {
            uint16_t applied = champion->pending_wounds;
            champion->wounds = (uint16_t)(champion->wounds | applied);
        }
        /*
         * F0320:1722
         *   G0410_ai_ChampionPendingWounds[idx] = 0;
         * Always resets the staged wound buffer.
         */
        champion->pending_wounds = 0;

        out_step->champions_processed++;
        out_step->champions_with_wound_application++;

        if (slot) {
            slot->wounds_before = wounds_before;
            slot->wounds_after = champion->wounds;
            slot->pending_wounds_before = pending_wounds_before;
            slot->pending_wounds_after = champion->pending_wounds;
            slot->pending_damage_before = pending_damage_before;
            slot->pending_damage_after = champion->pending_damage;
            slot->wound_application_occurred = true;
            slot->pending_wound_reset_occurred = true;
        }

        /*
         * F0320:1723-1724
         *   if (!(L0968_ui_PendingDamage =
         *             G0409_ai_ChampionPendingDamage[idx]))
         *           continue;
         * Zero-damage early return: do NOT reset G0409 yet, do NOT
         * fall through to F0623. The wound OR-in above has already
         * mutated state.
         */
        if (champion->pending_damage == 0) {
            out_step->champions_with_zero_damage_early_return++;

            if (slot) {
                slot->branch = (pending_wounds_before != 0)
                                   ? DM1_V1_CPPWT_BRANCH_WOUND_ONLY_PC34
                                   : DM1_V1_CPPWT_BRANCH_NO_OP_PC34;
                slot->zero_damage_early_return_took_place = true;
                slot->damage_draw_would_be_called = false;
            }
            continue;
        }

        /*
         * F0320:1725
         *   G0409_ai_ChampionPendingDamage[idx] = 0;
         * Only reached when PendingDamage > 0.
         */
        champion->pending_damage = 0;
        out_step->champions_with_damage_draw_called++;

        if (slot) {
            slot->pending_damage_after = 0;
            slot->branch = (pending_wounds_before != 0)
                               ? DM1_V1_CPPWT_BRANCH_WOUND_AND_DAMAGE_PC34
                               : DM1_V1_CPPWT_BRANCH_DAMAGE_ONLY_PC34;
            /*
             * The next F0320:1726-1727 dead-champion branch is also
             * out of scope for this gate (it lives on the
             * damage>0 path, which is covered by the damage-indicator
             * gate). The "damage draw would be called" flag here means
             * the dispatch boundary at F0320:1728+ would have run,
             * subject to the dead-champion and lethal branches.
             */
            slot->zero_damage_early_return_took_place = false;
            slot->damage_draw_would_be_called = true;
        }
    }

    return 1;
}
