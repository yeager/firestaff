#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_PENDING_DAMAGE_APPLY_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_PENDING_DAMAGE_APPLY_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DM1 V1 champion-panel pending damage APPLY gate.
 *
 * Source-locked contract-only gate for the
 * F0320_CHAMPION_ApplyAndDrawPendingDamageAndWounds pre-damage mutation
 * slice (CHAMPION.C F0320:1719-1740). The companion `damage_indicator`
 * gate pins the post-damage draw
 * (F0623) box-geometry + text-stride branch (F0320:1741-1768) and the
 * `damage_flash_decay` gate pins the C12 schedule + tick-5 erasure
 * (F0320:1758-1792, F0254:1614-1637). This gate covers the **mutation**
 * half - the M008_SET / CurrentHealth subtract / F0319 dispatch decided
 * before the F0623 box / C12 schedule ever fires:
 *
 *   - M008_SET(champion->Wounds, pendingWounds[i]) (F0320:1720),
 *     pendingWounds[i] = 0 (F0320:1722),
 *   - if pendingDamage[i] == 0 -> continue (F0320:1723-1725),
 *     pendingDamage[i] = 0 (F0320:1726),
 *   - if champion->CurrentHealth == 0 -> continue (F0320:1728-1729)
 *     (already dead; F0320 never resurrects in this slice),
 *   - if (CurrentHealth - pendingDamage) <= 0 ->
 *     F0319_CHAMPION_Kill (F0320:1733-1734) which sets
 *     CurrentHealth = 0, MASK0x1000_STATUS_BOX (F0319:1574),
 *     drops bones (F0319:1602-1609), clears inventory / drop-all-objects,
 *     and surfaces the kill to F0884 step 5;
 *     the F0623 box / C12 schedule never runs on this iteration;
 *   - else CurrentHealth -= pendingDamage (F0320:1736),
 *     M008_SET(Attributes, MASK0x0100_STATISTICS) (F0320:1737),
 *     if pendingWoundsApplied != 0 ->
 *     M008_SET(Attributes, MASK0x2000_WOUNDS) (F0320:1738-1739).
 *
 * The synthetic gate below is intentionally narrow:
 *   - one champion at a time (the F0320 outer-loop iteration is
 *     single-champion for testing; multi-champion sequencing is
 *     covered by feeding each in turn),
 *   - tracks but does not call F0319_CHAMPION_Kill directly (it
 *     dispatches the decision and the synthetic recorder records it),
 *   - rejects input.champion_index outside [0, 4) and pending_damage
 *     outside [0, 32767], mirroring the F0623:int16 / F0320:1719
 *     active-party bounds,
 *   - never reads real GRAPHICS.DAT / DUNGEON.DAT, never claims
 *     real-asset parity, never produces original-DOS pixel pairs.
 *
 * Disjoint from:
 *   - dm1_v1_champion_panel_damage_indicator_pc34_compat (post-damage
 *     F0623 box geometry + text stride),
 *   - dm1_v1_champion_panel_damage_flash_decay_pc34_compat
 *     (post-damage C12 schedule + tick-5 erase),
 *   - dm1_apply_pending_damage (combat_pc34_compat.c -- the
 *     combat-only M516 mutation without the attribute mask flips),
 *   - F0737_COMBAT_ApplyDamageToChampion_Compat
 *     (combat result -> champ mutation side),
 *   - F0319_CHAMPION_Kill itself (the synthesizer dispatches the
 *     decision and records the call, but does not implement F0319
 *     bones / drop-all / leader-hand / imagery coverage),
 *   - grp02 f0737/f0738/f0739 source-lock gates (combat resolution /
 *     result serialization), and
 *   - clock_tick_stat_repaint (F0331 per-tick attribute set,
 *     independent from F0320 damage-time MASK0x0100_STATISTICS).
 */

#define DM1_V1_CPDA_CHAMPION_COUNT_PC34 4
#define DM1_V1_CPDA_PENDING_DAMAGE_MAX_PC34 32767
#define DM1_V1_CPDA_PENDING_WOUNDS_MAX_PC34 65535

#define DM1_V1_CPDA_ATTR_NONE_PC34 0x0000u
#define DM1_V1_CPDA_ATTR_STATISTICS_PC34 0x0100u
#define DM1_V1_CPDA_ATTR_LOAD_PC34 0x0200u
#define DM1_V1_CPDA_ATTR_ICON_PC34 0x0400u
#define DM1_V1_CPDA_ATTR_PANEL_PC34 0x0800u
#define DM1_V1_CPDA_ATTR_STATUS_BOX_PC34 0x1000u
#define DM1_V1_CPDA_ATTR_WOUNDS_PC34 0x2000u
#define DM1_V1_CPDA_ATTR_ACTION_HAND_PC34 0x8000u

typedef enum DM1_V1_ChampionPanelPendingDamageApplyOutcomePc34Compat {
    DM1_V1_CPDA_OUTCOME_NONE_PC34 = 0,
    DM1_V1_CPDA_OUTCOME_HEALTH_UPDATED_PC34 = 1,
    DM1_V1_CPDA_OUTCOME_KILLED_BY_F0319_PC34 = 2,
    DM1_V1_CPDA_OUTCOME_SKIPPED_DEAD_PC34 = 3,
    DM1_V1_CPDA_OUTCOME_SKIPPED_NO_PENDING_PC34 = 4,
    DM1_V1_CPDA_OUTCOME_REJECTED_INDEX_PC34 = 5,
    DM1_V1_CPDA_OUTCOME_REJECTED_PENDING_DAMAGE_PC34 = 6
} DM1_V1_ChampionPanelPendingDamageApplyOutcomePc34Compat;

typedef struct DM1_V1_ChampionPanelPendingDamageApplyEvidencePc34Compat {
    bool contract_only;
    const char *applier_function_anchor;
    const char *kill_function_anchor;
    const char *status_box_redraw_anchor;
    const char *m008_set_anchor;
    const char *defs_mask_anchor;
    const char *defs_status_box_anchor;
    const char *defs_wounds_anchor;
    const char *loop_init_anchor;
    const char *dead_champion_skip_anchor;
    const char *lethal_branch_anchor;
    const char *nonlethal_branch_anchor;
    const char *conditional_wounds_anchor;
    const char *scope_note;
    const char *no_real_asset_claim;
} DM1_V1_ChampionPanelPendingDamageApplyEvidencePc34Compat;

typedef struct DM1_V1_ChampionPanelPendingDamageApplyInputPc34Compat {
    int champion_index;
    int pending_damage;
    int pending_wounds;
    int current_health;
    bool was_alive_before;
    uint32_t attributes_before;
} DM1_V1_ChampionPanelPendingDamageApplyInputPc34Compat;

typedef struct DM1_V1_ChampionPanelPendingDamageApplyResultPc34Compat {
    bool valid;
    bool contract_only;
    bool rejected_index;
    bool rejected_pending_damage;
    int champion_index;
    int champion_ordinal;
    int pending_damage;
    int pending_wounds;
    int pending_damage_applied;
    int wounds_applied;
    int current_health_before;
    int current_health_after;
    bool was_alive_before;
    bool alive_after;
    uint32_t attributes_before;
    uint32_t attributes_after;
    uint32_t attributes_orbit_mask;
    bool f0319_kill_called;
    bool status_box_redraw_dispatched;
    bool mouse_screen_update_enabled;
    bool mouse_screen_update_disabled;
    DM1_V1_ChampionPanelPendingDamageApplyOutcomePc34Compat outcome;
    const DM1_V1_ChampionPanelPendingDamageApplyEvidencePc34Compat *evidence;
} DM1_V1_ChampionPanelPendingDamageApplyResultPc34Compat;

const DM1_V1_ChampionPanelPendingDamageApplyEvidencePc34Compat *
DM1_V1_ChampionPanelPendingDamageApply_EvidencePc34Compat(void);

const char *
DM1_V1_ChampionPanelPendingDamageApply_SourceEvidencePc34Compat(void);

void DM1_V1_ChampionPanelPendingDamageApply_DefaultInputPc34Compat(
    DM1_V1_ChampionPanelPendingDamageApplyInputPc34Compat *input);

int DM1_V1_ChampionPanelPendingDamageApply_BuildPc34Compat(
    const DM1_V1_ChampionPanelPendingDamageApplyInputPc34Compat *input,
    DM1_V1_ChampionPanelPendingDamageApplyResultPc34Compat *out_result);

#ifdef __cplusplus
}
#endif

#endif
