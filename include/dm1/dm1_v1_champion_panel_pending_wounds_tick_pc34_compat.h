#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_PENDING_WOUNDS_TICK_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_PENDING_WOUNDS_TICK_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DM1 V1 champion-panel pending-wounds tick contract.
 *
 * Source-locked to ReDMCSB Toolchains/Common/Source/CHAMPION.C
 * F0320_CHAMPION_ApplyAndDrawPendingDamageAndWounds:1720-1727:
 *
 *   L0971_ps_Champion = M516_CHAMPIONS;
 *   for (L0967_ui_ChampionIndex = C00_CHAMPION_FIRST;
 *        L0967_ui_ChampionIndex < G0305_ui_PartyChampionCount;
 *        L0967_ui_ChampionIndex++, L0971_ps_Champion++) {
 *       M008_SET(L0971_ps_Champion->Wounds,
 *                L0970_i_PendingWounds =
 *                    G0410_ai_ChampionPendingWounds[L0967_ui_ChampionIndex]);
 *       G0410_ai_ChampionPendingWounds[L0967_ui_ChampionIndex] = 0;
 *       if (!(L0968_ui_PendingDamage =
 *                 G0409_ai_ChampionPendingDamage[L0967_ui_ChampionIndex]))
 *           continue;
 *       G0409_ai_ChampionPendingDamage[L0967_ui_ChampionIndex] = 0;
 *       if (!(AL0969_i_Health = L0971_ps_Champion->CurrentHealth))
 *           continue;
 *
 * This gate pins the unique CHAMPION.C F0320:1721-1724 slice that the
 * companion dm1_v1_champion_panel_damage_indicator_pc34_compat (which
 * covers only the F0623 damage>0 path) and the
 * dm1_v1_champion_panel_damage_flash_decay_pc34_compat (which covers
 * only the post-damage C12 hide-damage-received timeline) explicitly
 * skip: the wound-application + pending-wound-reset ALWAYS execute
 * even when G0409_ai_ChampionPendingDamage[idx] == 0, and the damage
 * draw is skipped via the early `continue`.
 *
 * Three invariants pinned by this gate:
 *
 *   1. WOUNDS_APPLICATION_BEFORE_DAMAGE_DRAW
 *      M008_SET(Wounds, PendingWounds = ChampionPendingWounds[idx])
 *      (COMPILE.H:1042 #define M008_SET(value, mask) ((value) |= (mask)))
 *      ALWAYS runs, even when PendingDamage == 0.
 *
 *   2. PENDING_WOUNDS_RESET_BEFORE_DAMAGE_DRAW
 *      G0410_ai_ChampionPendingWounds[idx] = 0 runs immediately after
 *      the OR-in, so a follow-up F0320 tick always sees a fresh 0
 *      pending-wound buffer for this champion index.
 *
 *   3. ZERO_DAMAGE_EARLY_RETURN
 *      The early `continue` on !PendingDamage skips the
 *      G0409_ai_ChampionPendingDamage[idx] = 0 reset, skips the
 *      F0319 kill / nonlethal health-subtract branches, and skips the
 *      F0623_DrawDamageToChampion_F0320_sub call (the F0623 graphic +
 *      text-printer covered by dm1_v1_champion_panel_damage_indicator).
 *      It does NOT skip anything else — the wound OR-in and the
 *      pending-wound reset have already happened.
 *
 * Companion gates:
 *   - dm1_v1_champion_panel_damage_indicator_pc34_compat covers
 *     damage>0 only (the F0623 sub-routine at F0320:1743-1798).
 *   - dm1_v1_champion_panel_damage_flash_decay_pc34_compat covers
 *     the post-damage C12 hide-damage-received timeline at
 *     F0320:1758-1792 + TIMELINE.C F0254:1614-1637.
 *   - dm1_v1_champion_panel_hand_slot_refresh_pc34_compat covers
 *     the F0296 walk-order and inventory-owner ordinal skip.
 *   - dm1_v1_champion_panel_dead_member_hand_refresh_pc34_compat
 *     covers the dead-member branch on a separate redraw path.
 *
 * The Wounds field is a uint16 body-part bitmask per DEFS.H:679 +
 * DEFS.H:735-741 (MASK0x0001_WOUND_READY_HAND, MASK0x0002_WOUND_ACTION_HAND,
 * MASK0x0004_WOUND_HEAD, MASK0x0008_WOUND_TORSO, MASK0x0010_WOUND_LEGS,
 * MASK0x0020_WOUND_FEET). The pending-wound staging is performed by
 * F0321_CHAMPION_AddPendingDamageAndWounds_GetDamage at CHAMPION.C:1803
 * + CHAMPION.C:1910 (random bit-set) — that staging path is out of scope
 * for this gate.
 *
 * No real-asset / original-DOS pixel parity claim. Contract-only.
 */

#define DM1_V1_CPPWT_CHAMPION_COUNT_PC34 4
#define DM1_V1_CPPWT_PARTY_MIN_PC34 1

#define DM1_V1_CPPWT_WOUND_NONE_PC34          0x0000
#define DM1_V1_CPPWT_WOUND_READY_HAND_PC34    0x0001
#define DM1_V1_CPPWT_WOUND_ACTION_HAND_PC34   0x0002
#define DM1_V1_CPPWT_WOUND_HEAD_PC34          0x0004
#define DM1_V1_CPPWT_WOUND_TORSO_PC34         0x0008
#define DM1_V1_CPPWT_WOUND_LEGS_PC34          0x0010
#define DM1_V1_CPPWT_WOUND_FEET_PC34          0x0020

typedef enum DM1_V1_ChampionPanelPendingWoundsTickBranchPc34Compat {
    DM1_V1_CPPWT_BRANCH_NONE_PC34 = 0,
    DM1_V1_CPPWT_BRANCH_WOUND_ONLY_PC34 = 1,
    DM1_V1_CPPWT_BRANCH_WOUND_AND_DAMAGE_PC34 = 2,
    DM1_V1_CPPWT_BRANCH_DAMAGE_ONLY_PC34 = 3,
    DM1_V1_CPPWT_BRANCH_NO_OP_PC34 = 4
} DM1_V1_ChampionPanelPendingWoundsTickBranchPc34Compat;

typedef struct DM1_V1_ChampionPanelPendingWoundsTickEvidencePc34Compat {
    bool contract_only;
    const char *applier_function_anchor;
    const char *m008_set_macro_anchor;
    const char *defs_pending_wounds_anchor;
    const char *defs_pending_damage_anchor;
    const char *defs_wound_constants_anchor;
    const char *defs_champion_struct_anchor;
    const char *loop_bound_anchor;
    const char *zero_damage_continue_anchor;
    const char *complement_gate_damage_indicator;
    const char *complement_gate_damage_flash_decay;
    const char *no_real_asset_claim;
} DM1_V1_ChampionPanelPendingWoundsTickEvidencePc34Compat;

typedef struct DM1_V1_ChampionPanelPendingWoundsTickChampionPc34Compat {
    int index;
    uint16_t wounds;                  /* DEFS.H:679 CHAMPION.Wounds */
    uint16_t pending_wounds;          /* DEFS.H:5867 G0410_ai_ChampionPendingWounds[i] */
    int16_t pending_damage;           /* DEFS.H:5866 G0409_ai_ChampionPendingDamage[i] */
    bool invalid_input;               /* synthetic-only: out-of-bounds staging */
} DM1_V1_ChampionPanelPendingWoundsTickChampionPc34Compat;

typedef struct DM1_V1_ChampionPanelPendingWoundsTickStatePc34Compat {
    int party_champion_count;
    DM1_V1_ChampionPanelPendingWoundsTickChampionPc34Compat
        champions[DM1_V1_CPPWT_CHAMPION_COUNT_PC34];
} DM1_V1_ChampionPanelPendingWoundsTickStatePc34Compat;

typedef struct DM1_V1_ChampionPanelPendingWoundsTickStepResultPc34Compat {
    int champions_processed;
    int champions_with_wound_application;
    int champions_with_zero_damage_early_return;
    int champions_with_damage_draw_called;
    int champions_skipped_due_to_invalid_input;
    int first_invalid_champion_index;
} DM1_V1_ChampionPanelPendingWoundsTickStepResultPc34Compat;

typedef struct DM1_V1_ChampionPanelPendingWoundsTickChampionResultPc34Compat {
    int champion_index;
    DM1_V1_ChampionPanelPendingWoundsTickBranchPc34Compat branch;
    uint16_t wounds_before;
    uint16_t wounds_after;
    uint16_t pending_wounds_before;
    uint16_t pending_wounds_after;
    int16_t pending_damage_before;
    int16_t pending_damage_after;
    bool wound_application_occurred;
    bool pending_wound_reset_occurred;
    bool zero_damage_early_return_took_place;
    bool damage_draw_would_be_called;
} DM1_V1_ChampionPanelPendingWoundsTickChampionResultPc34Compat;

const DM1_V1_ChampionPanelPendingWoundsTickEvidencePc34Compat *
DM1_V1_ChampionPanelPendingWoundsTick_EvidencePc34Compat(void);

void DM1_V1_ChampionPanelPendingWoundsTick_InitStatePc34Compat(
    DM1_V1_ChampionPanelPendingWoundsTickStatePc34Compat *state);

int DM1_V1_ChampionPanelPendingWoundsTick_RunPartyLoopPc34Compat(
    DM1_V1_ChampionPanelPendingWoundsTickStatePc34Compat *state,
    DM1_V1_ChampionPanelPendingWoundsTickStepResultPc34Compat *out_step,
    DM1_V1_ChampionPanelPendingWoundsTickChampionResultPc34Compat
        out_per_champion[DM1_V1_CPPWT_CHAMPION_COUNT_PC34]);

#ifdef __cplusplus
}
#endif

#endif
