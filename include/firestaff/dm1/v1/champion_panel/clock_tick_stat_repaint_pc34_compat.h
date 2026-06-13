#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_CLOCK_TICK_STAT_REPAINT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_CLOCK_TICK_STAT_REPAINT_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DM1 V1 champion-panel clock-tick stat repaint gate.
 *
 * Source-locked contract-only gate that pins the per-tick attribute
 * routing emitted by ReDMCSB CHAMPION.C F0331_CHAMPION_ApplyTimeEffects_CPSF
 * and the F0293_CHAMPION_DrawAllChampionStates sweep it terminates with.
 *
 * The lane name is `dm1_v1_auto_champion_panel_clock_tick_stat_repaint_gate`.
 * Existing pass700+ gates already cover the food/water recovery cycle
 * (test_dm1_v1_champion_panel_hud_food_water_recompute_pc34_compat) and
 * the close-inventory STATUS_BOX hook (same header), so this gate stays
 * narrow and focused on the *attribute* half of the clock-tick repaint:
 *
 *   - per-champion iteration: every alive non-candidate champion in
 *     the party (0..G0305_ui_PartyChampionCount - 1) gets
 *     MASK0x0100_STATISTICS set in M516_CHAMPIONS[i].Attributes on
 *     every tick,
 *   - PANEL set-routing: the MASK0x0800_PANEL attribute is added ONLY
 *     for the inventory champion (M000_INDEX_TO_ORDINAL(i) ==
 *     G0423_i_InventoryChampionOrdinal) AND only while the food/water
 *     poisoned panel (M565) or skills+statistics panel (C02) is open,
 *   - ICON direction re-sync: the MASK0x0400_ICON attribute is added
 *     when the direction has drifted and 60 ticks of creature quiet
 *     have elapsed (G0361_l_LastCreatureAttackTime < GameTime - 60),
 *   - the post-loop sweep: F0293_CHAMPION_DrawAllChampionStates is
 *     called exactly once at the END of F0331 with no per-champion
 *     F0293 call, and it is invoked with MASK0x0000_NONE so it
 *     repaints all 4 status boxes in one pass,
 *   - the skip rules: a dead champion (CurrentHealth == 0) is skipped,
 *     a champion whose ordinal matches G0299_ui_CandidateChampionOrdinal
 *     is skipped, and a champion index >= G0305_ui_PartyChampionCount
 *     is not visited at all,
 *   - the recovery-cycle cadence: 256 ticks active / 64 ticks resting
 *     trigger the C0..C6 statistic clamp loop, and the clamp formula
 *     is "current++ when below maximum" and
 *     "current -= current / maximum" when above maximum (the
 *     MEDIA240 PC 3.4+ formula, not the MEDIA002 PC 3.3 -1 step),
 *   - the deterministic 3-bit time criterion:
 *     (((GT & 0x80) + ((GT & 0x100) >> 2) + ((GT & 0x40) << 2)) >> 2)
 *     is computed ONCE at the top of the tick (not per-champion) and
 *     compared against Wisdom + (WizardSkill + PriestSkill) for mana
 *     regen and against Vitality + 12 for HP healing.
 *
 * Disjoint from: the food/water recovery cycle (hud_food_water_recompute),
 * the close-inventory STATUS_BOX hook, the food/water status-box
 * pixels, the portrait state redraw, the portrait box redraw states,
 * the F0354 inventory portrait box/blit, the hand-slot priority,
 * the action-hand slot priority, the spell-area overlay, the status
 * hand rotation, the second leader hand slot priority, the mouth/eye
 * poison warning, the wound handling, the ammunition compatibility,
 * the damage indicator, the status recompute, the action area routes,
 * the action area icon routes, the action area name routes, the
 * champion name hand routes, the champion names hands split, the
 * champion status slotbox, the champion panel HUD, the champion panel
 * HUD recompute, the champion panel bar pixels, the champion panel
 * pressing mouth/eye statusbox, the champion panel mouth/eye release,
 * the F0354 box variants, the F0292 portrait state redraw, the mirror
 * candidate C045 accept dead owner guard, the F0282 resurrection flow,
 * the chest scroll-wheel, the chest occupied-slot swap, and the
 * viewport F0107/F0108 family.
 */

#define FS_DM1_V1_CTS_CHAMPION_COUNT_PC34 4
#define FS_DM1_V1_CTS_STATISTIC_COUNT_PC34 7
#define FS_DM1_V1_CTS_STATISTIC_LUCK_PC34 0
#define FS_DM1_V1_CTS_STATISTIC_STRENGTH_PC34 1
#define FS_DM1_V1_CTS_STATISTIC_DEXTERITY_PC34 2
#define FS_DM1_V1_CTS_STATISTIC_WISDOM_PC34 3
#define FS_DM1_V1_CTS_STATISTIC_VITALITY_PC34 4
#define FS_DM1_V1_CTS_STATISTIC_ANTIMAGIC_PC34 5
#define FS_DM1_V1_CTS_STATISTIC_ANTIFIRE_PC34 6

#define FS_DM1_V1_CTS_STAT_CURRENT_PC34 1
#define FS_DM1_V1_CTS_STAT_MAXIMUM_PC34 0

#define FS_DM1_V1_CTS_PANEL_FOOD_WATER_POISONED_PC34 565
#define FS_DM1_V1_CTS_PANEL_SKILLS_AND_STATISTICS_PC34 2

#define FS_DM1_V1_CTS_ATTR_STATISTICS_PC34 0x0100u
#define FS_DM1_V1_CTS_ATTR_PANEL_PC34 0x0800u
#define FS_DM1_V1_CTS_ATTR_STATUS_BOX_PC34 0x1000u
#define FS_DM1_V1_CTS_ATTR_ICON_PC34 0x0400u
#define FS_DM1_V1_CTS_ATTR_LOAD_PC34 0x0200u
#define FS_DM1_V1_CTS_ATTR_ACTION_HAND_PC34 0x8000u

#define FS_DM1_V1_CTS_RECOVERY_PERIOD_ACTIVE_PC34 256
#define FS_DM1_V1_CTS_RECOVERY_PERIOD_RESTING_PC34 64

#define FS_DM1_V1_CTS_DIRECTION_RESYNC_QUIET_GAME_TIME_PC34 60
#define FS_DM1_V1_CTS_INVENTORY_CHAMPION_NONE_PC34 0
#define FS_DM1_V1_CTS_CANDIDATE_NONE_PC34 0

#define FS_DM1_V1_CTS_DRAW_ALL_NONE_PC34 0x0000u

typedef struct {
    int ordinal;
    int alive;
    int direction;
    int current_health;
    int maximum_health;
    int maximum_stamina;
    int current_stamina;
    int current_mana;
    int maximum_mana;
    int food;
    int water;
    int vitality_current;
    int wisdom_current;
    int wizard_skill;
    int priest_skill;
    int poison_event_count;
    int has_ekkhard_cross;
    int party_is_resting;
    uint16_t attributes;
} FsDm1V1CtsChampionPc34;

typedef struct {
    int party_champion_count;
    int inventory_champion_ordinal;
    int candidate_champion_ordinal;
    int last_creature_attack_time;
    int panel_content;
    int party_direction;
    FsDm1V1CtsChampionPc34 champions[FS_DM1_V1_CTS_CHAMPION_COUNT_PC34];
} FsDm1V1CtsTickInputPc34;

typedef struct {
    int valid;
    int time_criteria_3bit;
    int recovery_due_this_tick;
    int recovery_period;
    int statistics_attribute_set_count;
    int panel_attribute_set_index;
    int icon_attribute_set_count;
    int champion_skip_count;
    int dead_skipped;
    int candidate_skipped;
    int out_of_party_skipped;
    int draw_all_champion_states_invoked;
    uint16_t draw_all_champion_states_arg;
    int direction_resync_champion_index;
    int draw_state_calls_f0293_after_loop;
    int statistic_recovery_applied_count;
    int statistic_recovery_clamp_above_maximum_count;
    uint16_t attributes_before[FS_DM1_V1_CTS_CHAMPION_COUNT_PC34];
    uint16_t attributes_after[FS_DM1_V1_CTS_CHAMPION_COUNT_PC34];
    int champion_skipped_reason[FS_DM1_V1_CTS_CHAMPION_COUNT_PC34];
    int champion_attribute_dirty[FS_DM1_V1_CTS_CHAMPION_COUNT_PC34];
    int trace[8];
    uint32_t hash;
} FsDm1V1CtsTickResultPc34;

void fs_dm1_v1_cts_input_init_pc34(FsDm1V1CtsTickInputPc34 *input);

void fs_dm1_v1_cts_run_clock_tick_pc34(const FsDm1V1CtsTickInputPc34 *input,
                                       FsDm1V1CtsTickResultPc34 *result);

uint32_t fs_dm1_v1_cts_hash_pc34(const FsDm1V1CtsTickInputPc34 *input,
                                 const FsDm1V1CtsTickResultPc34 *result);

const char *fs_dm1_v1_cts_source_evidence_pc34(void);

const char *fs_dm1_v1_cts_non_overlap_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
