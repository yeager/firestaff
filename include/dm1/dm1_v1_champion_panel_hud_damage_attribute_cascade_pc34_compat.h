#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_HUD_DAMAGE_ATTRIBUTE_CASCADE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_HUD_DAMAGE_ATTRIBUTE_CASCADE_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DM1 V1 champion-panel HUD damage attribute-cascade contract.
 *
 * Source-locked to ReDMCSB Toolchains/Common/Source/CHAMPION.C F0320
 * :1720-1740 (apply pending damage / wounds, then M008_SET
 * MASK0x0100_STATISTICS, M008_SET MASK0x2000_WOUNDS when wounds > 0)
 * and Toolchains/Common/Source/CHAMDRAW.C F0292_CHAMPION_DrawState
 * :757 (M007_GET short-circuit on any of MASK0x0080_NAME_TITLE |
 * MASK0x0100_STATISTICS | MASK0x0200_LOAD | MASK0x0400_ICON |
 * MASK0x0800_PANEL | MASK0x1000_STATUS_BOX | MASK0x2000_WOUNDS |
 * MASK0x4000_VIEWPORT | MASK0x8000_ACTION_HAND),
 * :898 (M007_GET MASK0x0100_STATISTICS triggers redraw of the
 * champion's HP/Stamina/Mana bar triplet),
 * :937 (M007_GET MASK0x2000_WOUNDS triggers the wound-slot F0291
 * redraw sweep over C00..C05 with the inventory-vs-non-inventory
 * foot/action-hand anchor),
 * and :1110 (the F0292 tail M009_CLEAR that clears all nine
 * champion-panel Attributes bits after the redraw so the next tick
 * does not double-draw).
 *
 * Companion to:
 *   - test_dm1_v1_champion_panel_damage_indicator_pc34_compat
 *     (which pins the per-tick F0623_DrawDamageToChampion_F0320_sub
 *      draw of the C015/C016 damage graphic at C167..C173/C179..C185)
 *   - test_dm1_v1_champion_panel_damage_flash_decay_pc34_compat
 *     (which pins the F0320 C12 schedule + F0254 timeline hide
 *      with GameTime+5)
 * The present gate pins the *attribute bitfield cascade* between
 * F0320 (producer: sets Attributes MASK0x0100_STATISTICS and
 * MASK0x2000_WOUNDS) and F0292 (consumer: reads them, draws, and
 * clears all nine bits at the tail) - this contract is the bridge
 * that turns a damage event into the next-tick HP/Stamina/Mana bar
 * repaint + wound-slot repaint without re-firing the damage flash.
 *
 * The contract surface:
 *   pendingDamage == 0                -> champion skipped (continue);
 *                                        no attribute bits set.
 *   currentHealth == 0 (dead)         -> F0319_CHAMPION_Kill branch;
 *                                        no STATISTICS/WOUNDS bits
 *                                        set on the surviving path.
 *   nonlethal damage + wounds == 0    -> Attributes |= MASK0x0100_STATISTICS;
 *                                        Attributes has NO MASK0x2000_WOUNDS.
 *   nonlethal damage + wounds > 0     -> Attributes |= MASK0x0100_STATISTICS;
 *                                        Attributes |= MASK0x2000_WOUNDS.
 *   F0292 short-circuit on Attributes -> returns immediately when
 *                                        none of the nine redraw bits
 *                                        are set (CHAMDRAW.C:757).
 *   F0292 redraw consumes the bits    -> M009_CLEAR clears the entire
 *                                        nine-bit mask at F0292:1110 so
 *                                        a second F0292 call on the
 *                                        same tick does not double-draw.
 *
 * No real-asset / original-DOS pixel parity claim. Contract-only.
 * Disjoint from damage_indicator (F0623 draw), damage_flash_decay
 * (F0320 C12 schedule + F0254 hide), and the per-stat
 * clock_tick_stat_repaint / mana_bar_repaint / food_water_status_box
 * gates (which all read an already-cleared Attributes field and
 * re-set their own subset of bits).
 */

#define DM1_V1_CPHUDAC_CHAMPION_COUNT_PC34 4
#define DM1_V1_CPHUDAC_ATTRIBUTES_NONE_PC34 0x0000
#define DM1_V1_CPHUDAC_MASK0x0080_NAME_TITLE_PC34 0x0080
#define DM1_V1_CPHUDAC_MASK0x0100_STATISTICS_PC34 0x0100
#define DM1_V1_CPHUDAC_MASK0x0200_LOAD_PC34 0x0200
#define DM1_V1_CPHUDAC_MASK0x0400_ICON_PC34 0x0400
#define DM1_V1_CPHUDAC_MASK0x0800_PANEL_PC34 0x0800
#define DM1_V1_CPHUDAC_MASK0x1000_STATUS_BOX_PC34 0x1000
#define DM1_V1_CPHUDAC_MASK0x2000_WOUNDS_PC34 0x2000
#define DM1_V1_CPHUDAC_MASK0x4000_VIEWPORT_PC34 0x4000
#define DM1_V1_CPHUDAC_MASK0x8000_ACTION_HAND_PC34 0x8000
#define DM1_V1_CPHUDAC_MASK0x0100_STATISTICS_DAMAGE_TRIGGER_PC34 0x0100

#define DM1_V1_CPHUDAC_WOUND_COUNT_MAX_PC34 6
#define DM1_V1_CPHUDAC_SLOT_READY_HAND_PC34 0
#define DM1_V1_CPHUDAC_SLOT_ACTION_HAND_PC34 1
#define DM1_V1_CPHUDAC_SLOT_FEET_PC34 5

typedef enum DM1_V1_ChampionPanelHudDamageAttributeCascadeOutcomePc34Compat {
    DM1_V1_CPHUDAC_OUTCOME_NONE_PC34 = 0,
    DM1_V1_CPHUDAC_OUTCOME_PENDING_DAMAGE_ZERO_PC34 = 1,
    DM1_V1_CPHUDAC_OUTCOME_DEAD_CHAMPION_PC34 = 2,
    DM1_V1_CPHUDAC_OUTCOME_DAMAGE_ONLY_PC34 = 3,
    DM1_V1_CPHUDAC_OUTCOME_DAMAGE_AND_WOUNDS_PC34 = 4
} DM1_V1_ChampionPanelHudDamageAttributeCascadeOutcomePc34Compat;

typedef enum DM1_V1_ChampionPanelHudDamageAttributeCascadeRedrawPc34Compat {
    DM1_V1_CPHUDAC_REDRAW_NONE_PC34 = 0,
    DM1_V1_CPHUDAC_REDRAW_STATISTICS_PC34 = 1,
    DM1_V1_CPHUDAC_REDRAW_WOUND_SLOTS_PC34 = 2,
    DM1_V1_CPHUDAC_REDRAW_STATISTICS_AND_WOUND_SLOTS_PC34 = 3,
    DM1_V1_CPHUDAC_REDRAW_FULL_REDRAW_MASK_PC34 = 4
} DM1_V1_ChampionPanelHudDamageAttributeCascadeRedrawPc34Compat;

typedef struct DM1_V1_ChampionPanelHudDamageAttributeCascadeEvidencePc34Compat {
    bool contract_only;
    const char *producer_function_anchor;
    const char *producer_set_statistics_anchor;
    const char *producer_set_wounds_anchor;
    const char *producer_skip_zero_anchor;
    const char *producer_skip_dead_anchor;
    const char *consumer_function_anchor;
    const char *consumer_short_circuit_anchor;
    const char *consumer_statistics_redraw_anchor;
    const char *consumer_wounds_redraw_anchor;
    const char *consumer_clear_anchor;
    const char *wound_slot_anchor_anchor;
    const char *no_real_asset_claim;
} DM1_V1_ChampionPanelHudDamageAttributeCascadeEvidencePc34Compat;

typedef struct DM1_V1_ChampionPanelHudDamageAttributeCascadeInputPc34Compat {
    int champion_index;
    int current_health;
    int pending_damage;
    int pending_wounds_mask; /* bits 0..5 = C00..C05 slot wound bits */
    bool alive;
    bool is_inventory_champion;
    bool party_is_resting;
} DM1_V1_ChampionPanelHudDamageAttributeCascadeInputPc34Compat;

typedef struct DM1_V1_ChampionPanelHudDamageAttributeCascadeResultPc34Compat {
    bool valid;
    bool contract_only;
    bool rejected_null_output;
    bool rejected_champion_index;
    bool rejected_negative_health;
    bool skipped_pending_damage_zero;
    bool skipped_dead_champion;
    bool applied_damage;
    bool set_statistics_bit;
    bool set_wounds_bit;
    bool f0292_will_short_circuit;
    bool f0292_will_redraw_statistics;
    bool f0292_will_redraw_wounds;
    bool f0292_will_clear_after_redraw;
    int champion_index;
    int champion_ordinal;
    int attributes_after_apply;
    int wound_bits_after_apply;
    int wound_count_after_apply;
    int health_after_apply;
    int wound_slot_redraw_first;
    int wound_slot_redraw_last;
    DM1_V1_ChampionPanelHudDamageAttributeCascadeOutcomePc34Compat outcome;
    DM1_V1_ChampionPanelHudDamageAttributeCascadeRedrawPc34Compat redraw;
    const DM1_V1_ChampionPanelHudDamageAttributeCascadeEvidencePc34Compat *evidence;
} DM1_V1_ChampionPanelHudDamageAttributeCascadeResultPc34Compat;

const DM1_V1_ChampionPanelHudDamageAttributeCascadeEvidencePc34Compat *
DM1_V1_ChampionPanelHudDamageAttributeCascade_EvidencePc34Compat(void);

const char *
DM1_V1_ChampionPanelHudDamageAttributeCascade_SourceEvidencePc34Compat(void);

void DM1_V1_ChampionPanelHudDamageAttributeCascade_DefaultInputPc34Compat(
    DM1_V1_ChampionPanelHudDamageAttributeCascadeInputPc34Compat *input);

int DM1_V1_ChampionPanelHudDamageAttributeCascade_ApplyPc34Compat(
    const DM1_V1_ChampionPanelHudDamageAttributeCascadeInputPc34Compat *input,
    DM1_V1_ChampionPanelHudDamageAttributeCascadeResultPc34Compat *out_result);

int DM1_V1_ChampionPanelHudDamageAttributeCascade_RedrawPc34Compat(
    const DM1_V1_ChampionPanelHudDamageAttributeCascadeResultPc34Compat *state,
    DM1_V1_ChampionPanelHudDamageAttributeCascadeResultPc34Compat *out_result);

#ifdef __cplusplus
}
#endif

#endif
