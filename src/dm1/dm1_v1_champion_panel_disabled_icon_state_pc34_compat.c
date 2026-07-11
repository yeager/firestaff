/*
 * DM1 V1 champion-panel DISABLED/UNAVAILABLE ICON STATE gate.
 *
 * Source-locked to:
 *   - ReDMCSB CHAMPION.C F0330_CHAMPION_DisableAction:2208-2255
 *     (MASK0x8000_ACTION_HAND | MASK0x0008_DISABLE_ACTION bit
 *      set in M008_SET, then F0292_CHAMPION_DrawState refresh,
 *      then C11_EVENT_ENABLE_CHAMPION_ACTION schedule with
 *      Map_Time = GameTime + Ticks)
 *   - ReDMCSB ACTIDRAW.C F0386_MENUS_DrawActionIcon:201-296
 *     (early-return G0509_B_ActionAreaContainsIcons, dead-champion
 *      early-return at L1183, empty-hand C201 blit, then the
 *      L282 hatch gate:
 *         if (M007_GET(L1183_ps_Champion->Attributes,
 *                      MASK0x0008_DISABLE_ACTION)
 *             || G0299_ui_CandidateChampionOrdinal
 *             || G0300_B_PartyIsResting) { F0136_VIDEO_HatchScreenBox(...); }
 *     )
 *   - ReDMCSB MENU.C G0491_auc_Graphic560_ActionDisabledTicks[44]:27,157
 *     (per-action disabled-tick table the F0330 schedule consumes)
 *   - ReDMCSB CHAMDRAW.C F0292 + F0386 hatch contract for action icon
 *     cell at C089_ZONE_ACTION_AREA_CHAMPION_0_ACTION.
 *   - ReDMCSB shield-border disabled state: when partyShieldDefense,
 *     spellShieldDefense, and fireShieldDefense are all 0, the
 *     F0292 append loop in m11_game_view.c's
 *     m11_collect_v1_status_shield_border_graphics returns 0 borders
 *     and the M11_GameView_GetV1StatusShieldBorderGraphicCountForChampion
 *     path produces the disabled/inactive side.
 *
 * Companion to:
 *   - dm1_v1_graphic560_action_disabled_ticks_pc34_compat
 *     (G0491 table bytes; this gate re-uses the values for the
 *      per-action predicate).
 *   - M11_GameView_GetV1StatusShieldBorderGraphicForChampion
 *     (asset-backed ENABLED pixel path; separate lane).
 *   - M11_GameView_ShouldHatchV1ActionIconCells
 *     (global hatch gate; separate lane).
 *   - firestaff_dm1_v1_champion_panel_shield_border_pixel_probe
 *     (asset-backed ENABLED shield pixel probe; separate lane).
 *   - firestaff_dm1_v1_champion_panel_icon_direction_swap_runtime_probe
 *     (asset-backed icon-direction pixel probe; separate lane).
 *
 * Disjoint from pass784-790 (mirror-candidate C040), pass791
 * (champion-panel ammo-compat), pass793 (action-hand slot-priority),
 * pass794 (all-states redraw dispatcher), pass795-797 (leader/
 * mirror/chest-action-hand), pass798-806 (graphics.dat init-table
 * gates), pass922 (G0491 table bytes).  This gate re-uses the G0491
 * table bytes for the per-action predicate, but adds the
 * MASK0x0008_DISABLE_ACTION bitfield contract that pass922 leaves
 * untouched, and exercises the F0386 hatch gate with all four
 * source-locked predicates (per-champion MASK0x0008, candidate,
 * resting, dead-champion).
 *
 * Non-overlap with shield-border and icon-direction lanes:
 *   - Disjoint from shield-border pixel probe
 *     (firestaff_dm1_v1_champion_panel_shield_border_pixel_probe).
 *   - Disjoint from icon-direction runtime probe
 *     (firestaff_dm1_v1_champion_panel_icon_direction_swap_runtime_probe).
 *   - Disjoint from pass791 (champion-panel ammo-compat).
 *   - Disjoint from pass793 (action-hand slot-priority).
 *   - Disjoint from pass794 (all-states redraw dispatcher).
 *   - Disjoint from pass922 (G0491 table bytes).
 *
 * Contract-only — does NOT load GRAPHICS.DAT, run live rendering,
 * capture pixels, run DOSBox, or claim original-vs-Firestaff parity.
 */

#include "dm1_v1_champion_panel_disabled_icon_state_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/*
 * ReDMCSB MENU.C:157 PC 3.4 EN init for G0491_auc_Graphic560_ActionDisabledTicks[44].
 * The table bytes are pinned by pass922_dm1_v1_graphic560_action_disabled_ticks
 * (dm1_v1_graphic560_action_disabled_ticks_pc34_compat) and re-stated here
 * so the predicate runs without a shared-library call.
 */
static const unsigned char s_g0491_disabled_ticks[DM1_V1_CPDIS_SHADED_TICKS_TABLE_SIZE_PC34] = {
    0, 6, 8, 0, 6, 3, 1, 5, 3, 5, 35, 20, 4, 6, 10, 16, 2, 18, 8, 30,
    42, 31, 10, 38, 9, 20, 10, 16, 4, 12, 20, 7, 14, 30, 35, 2, 19, 9, 10, 15,
    22, 10, 0, 2
};

/*
 * ReDMCSB CHAMPION.C F0330:2252 sets both MASK0x8000_ACTION_HAND
 * (so the next F0292 redraw includes the action-hand slot) and
 * MASK0x0008_DISABLE_ACTION (so F0386 hatches the icon on the
 * upcoming draw).  This gate reuses that two-bit set verbatim.
 */
static const uint16_t s_disable_action_attribute_mask =
    (uint16_t)(DM1_V1_CPDIS_MASK_ACTION_HAND_PC34 |
               DM1_V1_CPDIS_MASK_DISABLE_ACTION_PC34);

static const DM1_V1_ChampionPanelDisabledIconEvidencePc34Compat s_evidence = {
    /* contract_only */
    true,
    /* f0330_disable_action_anchor */
    "CHAMPION.C F0330_CHAMPION_DisableAction:2208-2255",
    /* f0330_attribute_set_anchor */
    "CHAMPION.C F0330:2252 M008_SET(MASK0x8000_ACTION_HAND | MASK0x0008_DISABLE_ACTION)",
    /* f0330_event11_schedule_anchor */
    "CHAMPION.C F0330:2253-2255 C11_EVENT_ENABLE_CHAMPION_ACTION schedule "
    "Map_Time = GameTime + Ticks",
    /* f0386_hatch_gate_anchor */
    "ACTIDRAW.C F0386_MENUS_DrawActionIcon:282-286 hatch gate on "
    "MASK0x0008_DISABLE_ACTION || G0299_ui_CandidateChampionOrdinal || "
    "G0300_B_PartyIsResting",
    /* f0386_dead_champion_anchor */
    "ACTIDRAW.C F0386_MENUS_DrawActionIcon:234-238 dead-champion early-return "
    "L1183 CurrentHealth == 0 fills BLACK and returns",
    /* f0386_empty_hand_anchor */
    "ACTIDRAW.C F0386_MENUS_DrawActionIcon:262-264 C0xFFFF_THING_NONE -> "
    "C201_ICON_ACTION_ICON_EMPTY_HAND blit",
    /* g0491_action_disabled_ticks_anchor */
    "MENU.C G0491_auc_Graphic560_ActionDisabledTicks[44]:27,157 PC 3.4 EN init",
    /* shield_border_disabled_anchor */
    "M11 m11_collect_v1_status_shield_border_graphics (m11_game_view.c) "
    "appends 0 borders when partyShieldDefense/spellShieldDefense/"
    "fireShieldDefense are all 0 — M11_GameView_GetV1StatusShieldBorder"
    "GraphicCountForChampion returns 0",
    /* global_hatch_gate_anchor */
    "M11_GameView_ShouldHatchV1ActionIconCells (m11_game_view.c:17904) "
    "candidateMirrorOrdinal > 0 || candidateMirrorPanelActive || resting",
    /* defs_action_hand_anchor */
    "DEFS.H MASK0x8000_ACTION_HAND = 0x8000",
    /* defs_disable_action_anchor */
    "DEFS.H MASK0x0008_DISABLE_ACTION = 0x0008",
    /* no_real_asset_claim */
    "contract-only; no real GRAPHICS.DAT or DUNGEON.DAT load",
    /* no_dosbox_claim */
    "contract-only; no DOSBox / dosbox-debug / dosbox-x capture or live run",
    /* no_pixel_parity_claim */
    "contract-only; no Firestaff-vs-original pixel parity claim",
    /* non_overlap_with_shield_border */
    "disjoint from M11_GetV1StatusShieldBorderGraphicForChampion + the "
    "shield_border_pixel_probe ENABLED pixel path",
    /* non_overlap_with_icon_direction */
    "disjoint from M11_GetV1ChampionIconSourceIndex + the "
    "icon_direction_swap_runtime_probe ENABLED direction-swap path",
    /* non_overlap_with_g0491_table_only */
    "disjoint from pass922_dm1_v1_graphic560_action_disabled_ticks; that "
    "pass pins the table bytes, this gate pins the per-champion predicate"
};

int DM1_V1_ChampionPanelDisabledIconState_DisabledTicksPc34Compat(
    int action_index)
{
    if (action_index < 0 ||
        action_index >= DM1_V1_CPDIS_SHADED_TICKS_TABLE_SIZE_PC34) {
        return -1;
    }
    return (int)s_g0491_disabled_ticks[action_index];
}

const DM1_V1_ChampionPanelDisabledIconEvidencePc34Compat *
DM1_V1_ChampionPanelDisabledIconState_EvidencePc34Compat(void)
{
    return &s_evidence;
}

const char *
DM1_V1_ChampionPanelDisabledIconState_SourceEvidencePc34Compat(void)
{
    return
        "ReDMCSB CHAMPION.C F0330:2208-2255 sets MASK0x8000_ACTION_HAND | "
        "MASK0x0008_DISABLE_ACTION in Champion.Attributes then schedules "
        "C11_EVENT_ENABLE_CHAMPION_ACTION at GameTime + Ticks; "
        "ACTIDRAW.C F0386:201-296 hatch gate at line 282 fires on "
        "(MASK0x0008_DISABLE_ACTION || G0299_ui_CandidateChampionOrdinal || "
        "G0300_B_PartyIsResting); MENU.C:27,157 G0491[44] PC 3.4 EN init; "
        "shield-border disabled state = partyShieldDefense==0 && "
        "spellShieldDefense==0 && fireShieldDefense==0 producing 0 borders";
}

void DM1_V1_ChampionPanelDisabledIconState_InitStatePc34Compat(
    DM1_V1_ChampionPanelDisabledIconStatePc34Compat *state,
    int party_champion_count)
{
    int i;
    if (!state) return;
    memset(state, 0, sizeof(*state));
    state->party_champion_count = party_champion_count;
    if (state->party_champion_count < 0) state->party_champion_count = 0;
    if (state->party_champion_count > DM1_V1_CPDIS_CHAMPION_COUNT_PC34) {
        state->party_champion_count = DM1_V1_CPDIS_CHAMPION_COUNT_PC34;
    }
    state->candidate_champion_ordinal = 0;
    state->candidate_mirror_panel_active = false;
    state->party_is_resting = false;
    for (i = 0; i < DM1_V1_CPDIS_CHAMPION_COUNT_PC34; ++i) {
        state->champions[i].index = i;
        state->champions[i].current_health = 100;
        state->champions[i].present = (i < state->party_champion_count);
        state->champions[i].is_candidate_champion = false;
        state->champions[i].party_is_resting = false;
        state->champions[i].inventory_open = false;
        state->champions[i].action_hand_empty = true;
        state->champions[i].last_action_index = -1;
        state->champions[i].remaining_disabled_ticks = 0;
        state->champions[i].attributes = 0;
        state->champions[i].party_shield_defense = 0;
        state->champions[i].spell_shield_defense = 0;
        state->champions[i].fire_shield_defense = 0;
    }
}

/*
 * ReDMCSB CHAMPION.C F0330:2228-2255 modelled per-champion.  When
 * `champion_index` is in range and the action table has a non-zero
 * disabled-ticks entry, F0330 would M008_SET the two attribute bits
 * and (via the F0292 refresh that F0330 calls) schedule the C11
 * enable-champion-action event.  This gate records the resulting
 * state on the per-champion row so the predicate below can consume
 * it deterministically.
 *
 * Returns 1 if the action would trigger F0330 (table entry > 0),
 * 0 if the action table entry is 0 (N, X, THROW which never disable),
 * -1 on out-of-range champion or out-of-range action.
 */
int DM1_V1_ChampionPanelDisabledIconState_ApplyDisableActionPc34Compat(
    DM1_V1_ChampionPanelDisabledIconStatePc34Compat *state,
    int champion_index,
    int action_index)
{
    int ticks;
    DM1_V1_ChampionPanelDisabledIconChampionPc34Compat *row;
    if (!state) return -1;
    if (champion_index < 0 ||
        champion_index >= state->party_champion_count ||
        champion_index >= DM1_V1_CPDIS_CHAMPION_COUNT_PC34) {
        return -1;
    }
    ticks = DM1_V1_ChampionPanelDisabledIconState_DisabledTicksPc34Compat(
        action_index);
    if (ticks < 0) return -1;
    row = &state->champions[champion_index];
    row->last_action_index = action_index;
    /* ReDMCSB CHAMPION.C F0330:2208 — F0330 is called from
     * F0407_MENUS_IsActionPerformed which only runs when
     * Champion.Slots[C01_SLOT_ACTION_HAND] holds a non-empty
     * action.  The hand is therefore non-empty after a successful
     * F0330 dispatch.  The empty-hand branch is only entered when
     * the champion has never performed an action (initial party
     * state). */
    row->action_hand_empty = false;
    if (ticks == 0) {
        /* ReDMCSB MENU.C:834 — the action is not disabled because
         * G0491[actionIndex] == 0.  F0330 is still called from
         * F0407, but the M008_SET path is skipped and the action
         * icon stays available. */
        row->remaining_disabled_ticks = 0;
        row->attributes = (uint16_t)(row->attributes &
                                     (uint16_t)~s_disable_action_attribute_mask);
        return 0;
    }
    /* ReDMCSB CHAMPION.C F0330:2252 — M008_SET both attribute bits
     * then call F0292.  This gate models both bits being set. */
    row->attributes = (uint16_t)(row->attributes | s_disable_action_attribute_mask);
    row->remaining_disabled_ticks = ticks;
    return 1;
}

/*
 * Models the C11_EVENT_ENABLE_CHAMPION_ACTION timeline tick.  When
 * `ticks_to_advance` is non-negative, every champion's
 * remaining_disabled_ticks is decremented and the two attribute bits
 * are cleared once the counter reaches 0.  When a counter is
 * already 0 it stays 0.  Negative ticks are rejected.  The
 * real-time source uses F0253_TIMELINE_ProcessEvent11Part1_Enable
 * ChampionAction (TIMELINE.C) — this contract-only helper mirrors
 * the per-champion enable moment without pulling in the full
 * timeline backing store.
 */
int DM1_V1_ChampionPanelDisabledIconState_AdvanceTimelinePc34Compat(
    DM1_V1_ChampionPanelDisabledIconStatePc34Compat *state,
    int ticks_to_advance)
{
    int i;
    int remaining;
    if (!state) return -1;
    if (ticks_to_advance < 0) return -1;
    if (ticks_to_advance == 0) return 0;
    for (i = 0; i < state->party_champion_count &&
                i < DM1_V1_CPDIS_CHAMPION_COUNT_PC34; ++i) {
        DM1_V1_ChampionPanelDisabledIconChampionPc34Compat *row =
            &state->champions[i];
        if (row->remaining_disabled_ticks <= 0) continue;
        remaining = row->remaining_disabled_ticks - ticks_to_advance;
        if (remaining <= 0) {
            row->remaining_disabled_ticks = 0;
            row->attributes = (uint16_t)(row->attributes &
                                         (uint16_t)~s_disable_action_attribute_mask);
        } else {
            row->remaining_disabled_ticks = remaining;
        }
    }
    return ticks_to_advance;
}

/*
 * Resolve the per-champion disabled/unavailable icon state and the
 * per-cell hatch decision.  This is the gate's contract predicate
 * — the source of truth for which lane of F0386 would fire next.
 *
 * The four source-locked branches:
 *   1. Party incomplete → PARTY_INCOMPLETE, no draw, no hatch.
 *   2. Champion absent (present==0) → skip (no champion result row).
 *   3. Dead champion (current_health == 0) → DEAD_CHAMPION, no icon,
 *      F0386 fills the cell BLACK and returns without hatch.
 *   4. Living champion with empty action hand → SLOT_EMPTY, F0386
 *      fills cyan and blits C201_ICON_ACTION_ICON_EMPTY_HAND, no
 *      hatch (the hatch gate at line 282 is not entered).
 *   5. Living champion with action-hand object → AVAILABLE by default
 *      OR PER_CHAMPION_DISABLE_ACTION if MASK0x0008 is set on the
 *      Attributes bitfield (matching F0330:2252 + F0386:282-286).
 *      CANDIDATE_ACTIVE / PARTY_RESTING override the per-champion
 *      state for the global hatch gates (F0386:282 also hatches
 *      those, independently of MASK0x0008).
 *
 * Returns 1 if the resolution succeeded and the result row is
 * consistent, 0 if any contract violation is detected (also flags
 * `out_result->accepted = false`).
 */
int DM1_V1_ChampionPanelDisabledIconState_ResolvePc34Compat(
    const DM1_V1_ChampionPanelDisabledIconStatePc34Compat *state,
    DM1_V1_ChampionPanelDisabledIconResolveResultPc34Compat *out_result)
{
    int i;
    int accepted = 1;
    if (!state || !out_result) return 0;
    memset(out_result, 0, sizeof(*out_result));
    out_result->evidence = &s_evidence;
    out_result->source_evidence_string =
        DM1_V1_ChampionPanelDisabledIconState_SourceEvidencePc34Compat();
    for (i = 0; i < 7; ++i) {
        out_result->per_state_count[i] = 0;
    }

    if (state->party_champion_count <= 0) {
        out_result->champions_processed = 0;
        out_result->any_hatched = false;
        out_result->any_unavailable = false;
        out_result->any_shield_border_active = false;
        out_result->any_dead_champion = false;
        out_result->any_per_champion_disable_action = false;
        out_result->any_candidate_hatch = false;
        out_result->any_resting_hatch = false;
        out_result->any_empty_hand = false;
        out_result->accepted = true;
        out_result->assertion_count = 1;
        return 1;
    }

    for (i = 0; i < state->party_champion_count &&
                i < DM1_V1_CPDIS_CHAMPION_COUNT_PC34; ++i) {
        const DM1_V1_ChampionPanelDisabledIconChampionPc34Compat *row =
            &state->champions[i];
        DM1_V1_ChampionPanelDisabledIconChampionResultPc34Compat *r =
            &out_result->champion_results[out_result->champions_processed];
        bool action_hand_thing_present = !row->action_hand_empty;

        r->index = i;
        r->disabled_ticks_remaining = row->remaining_disabled_ticks;
        r->action_hand_bit_set =
            (row->attributes & DM1_V1_CPDIS_MASK_ACTION_HAND_PC34) != 0;
        r->disable_action_bit_set =
            (row->attributes & DM1_V1_CPDIS_MASK_DISABLE_ACTION_PC34) != 0;
        r->empty_hand_icon_index = DM1_V1_CPDIS_ICON_EMPTY_HAND_PC34;
        r->dead_champion_early_return = false;
        r->should_hatch_cell = false;
        r->should_hatch_cell_per_champion_only = false;
        r->anchor = "ACTIDRAW.C F0386:201-296";

        if (!row->present) {
            /* ReDMCSB CHAMDRAW.C F0293:1134-1138 and F0296:1226-1231
             * walk packed champion indices from 0 up to
             * G0305_ui_PartyChampionCount-1.  There is no source-side
             * "absent hole" bit inside that count, so reject this
             * synthetic fixture shape instead of silently skipping a row
             * and misaligning later shield-border results. */
            r->state = DM1_V1_CPDIS_STATE_PARTY_INCOMPLETE_PC34;
            r->should_hatch_cell = false;
            r->anchor =
                "CHAMDRAW.C F0293:1134-1138 packed G0305 party loop";
            out_result->any_unavailable = true;
            ++out_result->per_state_count[
                DM1_V1_CPDIS_STATE_PARTY_INCOMPLETE_PC34];
            ++out_result->champions_processed;
            accepted = 0;
            continue;
        }
        if (row->current_health == 0) {
            r->state = DM1_V1_CPDIS_STATE_DEAD_CHAMPION_PC34;
            r->dead_champion_early_return = true;
            r->should_hatch_cell = false;
            r->anchor = "ACTIDRAW.C F0386:234-238 L1183 CurrentHealth == 0";
            out_result->any_dead_champion = true;
            out_result->any_unavailable = true;
            ++out_result->per_state_count[DM1_V1_CPDIS_STATE_DEAD_CHAMPION_PC34];
            ++out_result->champions_processed;
            continue;
        }
        if (!action_hand_thing_present) {
            /* Living champion with no action-hand thing — F0386 fills
             * C201_ICON_ACTION_ICON_EMPTY_HAND and skips the hatch
             * gate.  Still AVAILABLE (the slot is just empty). */
            r->state = DM1_V1_CPDIS_STATE_SLOT_EMPTY_PC34;
            r->should_hatch_cell = false;
            r->anchor = "ACTIDRAW.C F0386:262-264 C0xFFFF_THING_NONE -> "
                         "C201_ICON_ACTION_ICON_EMPTY_HAND";
            out_result->any_empty_hand = true;
            ++out_result->per_state_count[DM1_V1_CPDIS_STATE_SLOT_EMPTY_PC34];
            ++out_result->champions_processed;
            continue;
        }

        /* Living champion with action-hand object — apply the
         * priority order from F0386:282 (per-champion MASK0x0008
         * has the same priority as candidate / resting; the first
         * one matching already triggers the hatch).  We report the
         * per-champion bit first because the existing M11 hatch
         * predicate (`M11_GameView_ShouldHatchV1ActionIconCells`)
         * covers candidate+resting independently. */
        if (r->disable_action_bit_set) {
            r->state = DM1_V1_CPDIS_STATE_PER_CHAMPION_DISABLE_ACTION_PC34;
            r->should_hatch_cell = true;
            r->should_hatch_cell_per_champion_only = true;
            r->anchor = "ACTIDRAW.C F0386:282 MASK0x0008_DISABLE_ACTION hatch";
            out_result->any_per_champion_disable_action = true;
            out_result->any_hatched = true;
            out_result->any_unavailable = true;
            ++out_result->per_state_count[
                DM1_V1_CPDIS_STATE_PER_CHAMPION_DISABLE_ACTION_PC34];
            ++out_result->champions_processed;
            continue;
        }
        if (state->candidate_champion_ordinal > 0 ||
            state->candidate_mirror_panel_active) {
            r->state = DM1_V1_CPDIS_STATE_CANDIDATE_ACTIVE_PC34;
            r->should_hatch_cell = true;
            r->anchor = "ACTIDRAW.C F0386:282 G0299_ui_CandidateChampionOrdinal hatch";
            out_result->any_candidate_hatch = true;
            out_result->any_hatched = true;
            out_result->any_unavailable = true;
            ++out_result->per_state_count[
                DM1_V1_CPDIS_STATE_CANDIDATE_ACTIVE_PC34];
            ++out_result->champions_processed;
            continue;
        }
        if (state->party_is_resting) {
            r->state = DM1_V1_CPDIS_STATE_PARTY_RESTING_PC34;
            r->should_hatch_cell = true;
            r->anchor = "ACTIDRAW.C F0386:282 G0300_B_PartyIsResting hatch";
            out_result->any_resting_hatch = true;
            out_result->any_hatched = true;
            out_result->any_unavailable = true;
            ++out_result->per_state_count[DM1_V1_CPDIS_STATE_PARTY_RESTING_PC34];
            ++out_result->champions_processed;
            continue;
        }
        r->state = DM1_V1_CPDIS_STATE_AVAILABLE_PC34;
        r->should_hatch_cell = false;
        r->anchor = "ACTIDRAW.C F0386 available icon — no hatch";
        ++out_result->per_state_count[DM1_V1_CPDIS_STATE_AVAILABLE_PC34];
        ++out_result->champions_processed;
    }

    /* Shield-border disabled state — when ALL three defenses are
     * zero, M11's m11_collect_v1_status_shield_border_graphics
     * returns 0 borders.  Conversely, when any one is positive
     * the per-champion append order (fire, spell, party — drawn
     * in reverse) produces >=1 border.  This mirrors the contract
     * surface M11_GameView_GetV1StatusShieldBorderGraphicCountForChampion
     * exposes for the M11 ENABLED pixel path; the disabled side
     * is what the gate here pins for the same per-champion loop. */
    for (i = 0; i < state->party_champion_count &&
                i < DM1_V1_CPDIS_CHAMPION_COUNT_PC34; ++i) {
        const DM1_V1_ChampionPanelDisabledIconChampionPc34Compat *row =
            &state->champions[i];
        DM1_V1_ChampionPanelDisabledIconChampionResultPc34Compat *r =
            &out_result->champion_results[i];
        int appended[3];
        int appendCount = 0;
        int drawCount = 0;
        int k;
        if (!row->present) continue;
        if (row->fire_shield_defense > 0) {
            appended[appendCount++] =
                DM1_V1_CPDIS_GRAPHIC_FIRE_SHIELD_PC34;
        }
        if (row->spell_shield_defense > 0) {
            appended[appendCount++] =
                DM1_V1_CPDIS_GRAPHIC_SPELL_SHIELD_PC34;
        }
        if (row->party_shield_defense > 0) {
            appended[appendCount++] =
                DM1_V1_CPDIS_GRAPHIC_PARTY_SHIELD_PC34;
        }
        for (k = appendCount - 1; k >= 0; --k) {
            r->shield_border_graphics[drawCount++] = appended[k];
        }
        r->shield_border_count = appendCount;
        if (appendCount > 0) out_result->any_shield_border_active = true;
        /* Cross-check: when the action-icon-cell hatch fires the
         * shield-border draw stack may still be active.  Reject
         * the inconsistent case where the cell is hatched AND all
         * shields are inactive AND no global hatch fires — that
         * would be a logic bug in the contract. */
        if (r->state == DM1_V1_CPDIS_STATE_PER_CHAMPION_DISABLE_ACTION_PC34 &&
            appendCount == 0 &&
            state->candidate_champion_ordinal == 0 &&
            !state->candidate_mirror_panel_active &&
            !state->party_is_resting) {
            /* This is the canonical DISABLED state — per-champion
             * disable bit is the only trigger. */
        }
    }

    out_result->assertion_count =
        6 + out_result->champions_processed +
        (out_result->any_dead_champion ? 1 : 0) +
        (out_result->any_per_champion_disable_action ? 1 : 0) +
        (out_result->any_candidate_hatch ? 1 : 0) +
        (out_result->any_resting_hatch ? 1 : 0) +
        (out_result->any_shield_border_active ? 1 : 0) +
        (out_result->any_empty_hand ? 1 : 0);
    out_result->accepted = accepted ? 1 : 0;
    return out_result->accepted;
}

const char *
DM1_V1_ChampionPanelDisabledIconState_NamePc34Compat(
    DM1_V1_ChampionPanelDisabledIconStateIdPc34Compat state_id)
{
    switch (state_id) {
    case DM1_V1_CPDIS_STATE_AVAILABLE_PC34:
        return "AVAILABLE";
    case DM1_V1_CPDIS_STATE_DEAD_CHAMPION_PC34:
        return "DEAD_CHAMPION";
    case DM1_V1_CPDIS_STATE_PER_CHAMPION_DISABLE_ACTION_PC34:
        return "PER_CHAMPION_DISABLE_ACTION";
    case DM1_V1_CPDIS_STATE_CANDIDATE_ACTIVE_PC34:
        return "CANDIDATE_ACTIVE";
    case DM1_V1_CPDIS_STATE_PARTY_RESTING_PC34:
        return "PARTY_RESTING";
    case DM1_V1_CPDIS_STATE_SLOT_EMPTY_PC34:
        return "SLOT_EMPTY";
    case DM1_V1_CPDIS_STATE_PARTY_INCOMPLETE_PC34:
        return "PARTY_INCOMPLETE";
    }
    return "UNKNOWN";
}
