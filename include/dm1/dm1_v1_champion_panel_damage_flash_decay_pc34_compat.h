#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_DAMAGE_FLASH_DECAY_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_DAMAGE_FLASH_DECAY_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DM1 V1 champion-panel damage FLASH DECAY contract.
 *
 * Source-locked to ReDMCSB Toolchains/Common/Source/CHAMPION.C F0320:1758-1792
 * (post-F0623 schedule of C12_EVENT_HIDE_DAMAGE_RECEIVED with GameTime+5),
 * Toolchains/Common/Source/TIMELINE.C F0254:1614-1637 (C12 handler that
 * either draws the inventory portrait over the C179..C185 damage zone or
 * sets MASK0x0080_NAME_TITLE + F0292_CHAMPION_DrawState for the non-
 * inventory champion to redraw the name over the C167..C173 damage zone),
 * Toolchains/Common/Source/TIMELINE.C F0261:1933-1934 (C12 case dispatch),
 * and Toolchains/Common/Source/CHAMDRAW.C F0292:771,792-815 (the
 * MASK0x1000_STATUS_BOX / C151..C182 status-box zone overdraw that
 * incidentally erases the C167..C173 / C179..C185 damage graphic when
 * the next tick fires F0292 for the flagged champion).
 *
 * Companion to test_dm1_v1_champion_panel_damage_indicator_pc34_compat
 * (which pins the per-tick F0623 flash) — the present gate pins the
 * temporal decay (flash on tick T, erased on tick T+5 by the C12 event,
 * or earlier if MASK0x1000_STATUS_BOX is set on that champion).
 *
 * The flash decay is a 5-tick window:
 *   tick T   : F0320 sees G0409[i] != 0, applies damage. If the
 *              resulting CurrentHealth is > 0, F0320 calls F0623 to draw
 *              the C015/C016 graphic at C167..C173 (small, non-inventory)
 *              or C179..C185 (big, inventory) zone, then schedules a
 *              C12_EVENT_HIDE_DAMAGE_RECEIVED event with
 *              Map_Time.G0313_ul_GameTime = GameTime + 5 and
 *              A.A.Priority = championIndex, storing the event index
 *              in champion->HideDamageReceivedEventIndex.
 *              If the damage is lethal, F0320 takes the F0319 kill branch
 *              before F0623/C12, so no damage graphic or C12 event is
 *              created for that hit.
 *   tick T+1..T+4 : damage graphic visible, G0409[i] is 0.
 *   tick T+5   : F0261 case C12 fires F0254 (championIndex) which
 *              erases the damage by drawing the inventory portrait
 *              (inventory champion) or by MASK0x0080_NAME_TITLE +
 *              F0292_CHAMPION_DrawState (non-inventory champion), then
 *              resets HideDamageReceivedEventIndex to -1.
 *   tick >T+5  : damage fully decayed.
 *
 * Reschedule edge: if HideDamageReceivedEventIndex != -1 (a previous
 * hit already has a pending C12), F0320 does NOT add a new event —
 * it fixes the placement of the existing event to the new GameTime+5
 * via F0235_TIMELINE_GetIndex + F0236_TIMELINE_FixPlacement.
 *
 * Dead-champion edge: F0254 early-returns at line 1624 when a previously
 * staged C12 targets a champion whose CurrentHealth is now 0. Lethal damage
 * itself does not stage that C12; F0319_CHAMPION_Kill instead marks the
 * status-box redraw path (CHAMPION.C:1574 M008_SET MASK0x1000_STATUS_BOX).
 *
 * No real-asset / original-DOS pixel parity claim. Contract-only.
 */

#define DM1_V1_CPDFD_CHAMPION_COUNT_PC34 4
#define DM1_V1_CPDFD_TIMELINE_NONE_PC34 (-1)
#define DM1_V1_CPDFD_HIDE_DELAY_TICKS_PC34 5

#define DM1_V1_CPDFD_GFX_DAMAGE_SMALL_PC34 15
#define DM1_V1_CPDFD_GFX_DAMAGE_BIG_PC34 16
#define DM1_V1_CPDFD_ZONE_DAMAGE_SMALL_FIRST_PC34 167
#define DM1_V1_CPDFD_ZONE_DAMAGE_BIG_FIRST_PC34 179

#define DM1_V1_CPDFD_EVENT_HIDE_DAMAGE_RECEIVED_PC34 12

typedef enum DM1_V1_ChampionPanelDamageFlashDecayTickKindPc34Compat {
    DM1_V1_CPDFD_TICK_NONE_PC34 = 0,
    DM1_V1_CPDFD_TICK_PENDING_FLASH_PC34 = 1,
    DM1_V1_CPDFD_TICK_C12_HIDE_DAMAGE_PC34 = 2,
    DM1_V1_CPDFD_TICK_STATUS_BOX_OVERDRAW_PC34 = 3
} DM1_V1_ChampionPanelDamageFlashDecayTickKindPc34Compat;

typedef struct DM1_V1_ChampionPanelDamageFlashDecayEvidencePc34Compat {
    bool contract_only;
    const char *applier_function_anchor;
    const char *hide_event_handler_anchor;
    const char *timeline_dispatch_anchor;
    const char *status_box_overdraw_anchor;
    const char *defs_event_anchor;
    const char *defs_hide_anchor;
    const char *defs_zone_anchor;
    const char *defs_graphic_anchor;
    const char *reschedule_anchor;
    const char *kill_status_box_anchor;
    const char *no_real_asset_claim;
} DM1_V1_ChampionPanelDamageFlashDecayEvidencePc34Compat;

typedef struct DM1_V1_ChampionPanelDamageFlashDecayChampionPc34Compat {
    int index;
    int current_health;
    bool alive;
    bool is_inventory_champion;
    int damage_just_applied;
    int damage_visible;
    int hide_damage_received_event_index;
    int damage_zone_index;
    int damage_graphic_index;
} DM1_V1_ChampionPanelDamageFlashDecayChampionPc34Compat;

typedef struct DM1_V1_ChampionPanelDamageFlashDecayTimelineEventPc34Compat {
    int event_index;
    int event_type;
    int map_index;
    int64_t fire_time;
    int64_t scheduled_time;
    int champion_priority;
    int slot; /* unused; present for future parallelism with other C-codes */
} DM1_V1_ChampionPanelDamageFlashDecayTimelineEventPc34Compat;

typedef struct DM1_V1_ChampionPanelDamageFlashDecayStatePc34Compat {
    int64_t game_time;
    int party_champion_count;
    int inventory_champion_ordinal;
    bool inventory_open;
    bool mask0x1000_status_box_dirty;
    bool mask0x1000_status_box_dirty_for_champion[DM1_V1_CPDFD_CHAMPION_COUNT_PC34];
    DM1_V1_ChampionPanelDamageFlashDecayChampionPc34Compat
        champions[DM1_V1_CPDFD_CHAMPION_COUNT_PC34];
    /* synthetic timeline backing store for the C12 hide-damage event */
    int next_timeline_event_index;
    int pending_timeline_event_count;
    DM1_V1_ChampionPanelDamageFlashDecayTimelineEventPc34Compat
        timeline_events[16];
} DM1_V1_ChampionPanelDamageFlashDecayStatePc34Compat;

typedef struct DM1_V1_ChampionPanelDamageFlashDecayTickPc34Compat {
    int64_t game_time;
    int champion_index;
    DM1_V1_ChampionPanelDamageFlashDecayTickKindPc34Compat kind;
    const char *anchor;
} DM1_V1_ChampionPanelDamageFlashDecayTickPc34Compat;

typedef struct DM1_V1_ChampionPanelDamageFlashDecayStepResultPc34Compat {
    int tick_count;
    bool applied_pending_damage;
    bool scheduled_new_event;
    bool rescheduled_existing_event;
    bool fired_c12_hide_event;
    bool inventory_portrait_overdraw_occurred;
    bool non_inventory_name_overdraw_occurred;
    bool dead_champion_early_return;
    bool status_box_dirty_overdraw_occurred;
    int64_t next_game_time;
    int damage_visible_after;
    int champion_index;
} DM1_V1_ChampionPanelDamageFlashDecayStepResultPc34Compat;

typedef struct DM1_V1_ChampionPanelDamageFlashDecayRunResultPc34Compat {
    int tick_count;
    int total_applied;
    int total_scheduled_new;
    int total_rescheduled_existing;
    int total_c12_hide_fired;
    int total_inventory_portrait_overdraws;
    int total_non_inventory_name_overdraws;
    int total_status_box_overdraws;
    int total_dead_champion_early_returns;
    int64_t final_game_time;
    /* per-tick trace (capped) */
    int trace_count;
    DM1_V1_ChampionPanelDamageFlashDecayTickPc34Compat
        trace[64];
    bool damage_visible_for_champion[DM1_V1_CPDFD_CHAMPION_COUNT_PC34];
} DM1_V1_ChampionPanelDamageFlashDecayRunResultPc34Compat;

const DM1_V1_ChampionPanelDamageFlashDecayEvidencePc34Compat *
DM1_V1_ChampionPanelDamageFlashDecay_EvidencePc34Compat(void);

void DM1_V1_ChampionPanelDamageFlashDecay_InitStatePc34Compat(
    DM1_V1_ChampionPanelDamageFlashDecayStatePc34Compat *state);

int DM1_V1_ChampionPanelDamageFlashDecay_TickPc34Compat(
    DM1_V1_ChampionPanelDamageFlashDecayStatePc34Compat *state,
    int champion_index_with_pending_damage,
    int pending_damage,
    DM1_V1_ChampionPanelDamageFlashDecayStepResultPc34Compat *out_step);

int DM1_V1_ChampionPanelDamageFlashDecay_AdvanceTimelinePc34Compat(
    DM1_V1_ChampionPanelDamageFlashDecayStatePc34Compat *state,
    int ticks,
    DM1_V1_ChampionPanelDamageFlashDecayRunResultPc34Compat *out_run);

int DM1_V1_ChampionPanelDamageFlashDecay_OverdrawStatusBoxPc34Compat(
    DM1_V1_ChampionPanelDamageFlashDecayStatePc34Compat *state,
    int champion_index);

int DM1_V1_ChampionPanelDamageFlashDecay_FlushRemainingPc34Compat(
    DM1_V1_ChampionPanelDamageFlashDecayStatePc34Compat *state,
    DM1_V1_ChampionPanelDamageFlashDecayRunResultPc34Compat *out_run);

int DM1_V1_ChampionPanelDamageFlashDecay_BuildResultPc34Compat(
    const DM1_V1_ChampionPanelDamageFlashDecayStatePc34Compat *state,
    int champion_index,
    DM1_V1_ChampionPanelDamageFlashDecayStepResultPc34Compat *out_step);

#ifdef __cplusplus
}
#endif

#endif
