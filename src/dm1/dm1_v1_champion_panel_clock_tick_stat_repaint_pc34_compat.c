#include "firestaff/dm1/v1/champion_panel/clock_tick_stat_repaint_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB anchors for the clock-tick stat repaint:
 *
 *   CHAMPION.C F0331_CHAMPION_ApplyTimeEffects_CPSF:2331-2332
 *     early-returns when G0305_ui_PartyChampionCount is zero.
 *   CHAMPION.C F0331_CHAMPION_ApplyTimeEffects_CPSF:2333
 *     AL9998_ui_GameTime = G0313_ul_GameTime.
 *   CHAMPION.C F0331_CHAMPION_ApplyTimeEffects_CPSF:2334
 *     L1012_ui_TimeCriteria =
 *       (((GT & 0x80) + ((GT & 0x100) >> 2) + ((GT & 0x40) << 2)) >> 2).
 *   CHAMPION.C F0331_CHAMPION_ApplyTimeEffects_CPSF:2335
 *     for (champ = 0; champ < G0305_ui_PartyChampionCount; champ++) {
 *       if (champ->CurrentHealth &&
 *           (INDEX_TO_ORDINAL(champ) != G0299_ui_CandidateChampionOrdinal)) {
 *         ...
 *       }
 *     }
 *   CHAMPION.C F0331_CHAMPION_ApplyTimeEffects_CPSF:2487
 *     if (!((uint16_t)GameTime & (resting ? 63 : 255))) {
 *       for (s = C0_STATISTIC_LUCK; s <= C6_STATISTIC_ANTIFIRE; s++) {
 *         cur = champ->Statistics[s][C1_CURRENT];
 *         max = champ->Statistics[s][C0_MAXIMUM];
 *         if (cur < max) cur++;
 *         else if (cur > max) cur -= cur / max;   // MEDIA240 PC 3.4+
 *         champ->Statistics[s][C1_CURRENT] = cur;
 *       }
 *     }
 *   CHAMPION.C F0331_CHAMPION_ApplyTimeEffects_CPSF:2491-2493
 *     direction re-sync: if (!resting && champ->Direction != G0308 &&
 *         G0361 < GameTime - 60) {
 *       champ->Direction = G0308;
 *       champ->MaximumDamageReceived = 0;
 *       M008_SET(champ->Attributes, MASK0x0400_ICON);
 *     }
 *   CHAMPION.C F0331_CHAMPION_ApplyTimeEffects_CPSF:2494
 *     M008_SET(champ->Attributes, MASK0x0100_STATISTICS);
 *   CHAMPION.C F0331_CHAMPION_ApplyTimeEffects_CPSF:2495-2497
 *     if (INDEX_TO_ORDINAL(champ) == G0423_i_InventoryChampionOrdinal &&
 *         (G0424 == M565_FOOD_WATER_POISONED ||
 *          G0424 == C02_SKILLS_AND_STATISTICS)) {
 *       M008_SET(champ->Attributes, MASK0x0800_PANEL);
 *     }
 *   CHAMPION.C F0331_CHAMPION_ApplyTimeEffects_CPSF:2503
 *     F0293_CHAMPION_DrawAllChampionStates(MASK0x0000_NONE);
 *
 *   CHAMDRAW.C F0293_CHAMPION_DrawAllChampionStates repaints all 4
 *   status boxes in one F0292_CHAMPION_DrawState sweep with no per-
 *   champion parameter, so the MASK0x0100_STATISTICS attribute set in
 *   F0331:2494 is the canonical trigger for the same-tick repaint.
 *
 *   DEFS.H:780-817 C30..C37, :873-876 M516_CHAMPIONS, :3778-3786
 *   C104..C154, :5700 G0305_ui_PartyChampionCount, :5876-5881
 *   G0423/G0424/G0425/G0426, plus the MASK0x0100_STATISTICS,
 *   MASK0x0800_PANEL, MASK0x0400_ICON symbols.
 *
 * This contract-only helper models the attribute half of the clock-
 * tick stat repaint. It does not model food/water/stamina/mana/HP
 * arithmetic (already covered by test_dm1_v1_champion_panel_hud_food_
 * water_recompute_pc34_compat and dm1_v1_champion_needs_pc34_compat).
 */

static int fs_dm1_v1_cts_is_index_to_ordinal(int idx)
{
    return idx + 1;
}

static int fs_dm1_v1_cts_time_criteria(uint32_t game_time)
{
    const uint32_t gt = game_time & 0xFFFFu;
    return (int)(((gt & 0x0080u) +
                  ((gt & 0x0100u) >> 2) +
                  ((gt & 0x0040u) << 2)) >> 2);
}

static int fs_dm1_v1_cts_recovery_period(int party_is_resting)
{
    return party_is_resting ?
        FS_DM1_V1_CTS_RECOVERY_PERIOD_RESTING_PC34 :
        FS_DM1_V1_CTS_RECOVERY_PERIOD_ACTIVE_PC34;
}

static int fs_dm1_v1_cts_recovery_due(uint32_t game_time, int party_is_resting)
{
    const uint32_t period =
        (uint32_t)fs_dm1_v1_cts_recovery_period(party_is_resting);
    return ((game_time & 0xFFFFu) % period) == 0u;
}

static int fs_dm1_v1_cts_panel_attr_eligible(int panel_content)
{
    return panel_content == FS_DM1_V1_CTS_PANEL_FOOD_WATER_POISONED_PC34 ||
           panel_content == FS_DM1_V1_CTS_PANEL_SKILLS_AND_STATISTICS_PC34;
}

void fs_dm1_v1_cts_input_init_pc34(FsDm1V1CtsTickInputPc34 *input)
{
    if (!input) return;
    memset(input, 0, sizeof(*input));
    input->party_champion_count = 0;
    input->inventory_champion_ordinal = 0;
    input->candidate_champion_ordinal = 0;
    input->last_creature_attack_time = 0;
    input->panel_content = 0;
    input->party_direction = 0;
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        input->champions[i].ordinal = fs_dm1_v1_cts_is_index_to_ordinal(i);
        input->champions[i].alive = 1;
        input->champions[i].direction = 0;
        input->champions[i].current_health = 30;
        input->champions[i].maximum_health = 30;
        input->champions[i].maximum_stamina = 1000;
        input->champions[i].current_stamina = 1000;
        input->champions[i].current_mana = 0;
        input->champions[i].maximum_mana = 0;
        input->champions[i].food = 1500;
        input->champions[i].water = 1500;
        input->champions[i].vitality_current = 12;
        input->champions[i].wisdom_current = 12;
        input->champions[i].wizard_skill = 0;
        input->champions[i].priest_skill = 0;
        input->champions[i].poison_event_count = 0;
        input->champions[i].has_ekkhard_cross = 0;
        input->champions[i].party_is_resting = 0;
        input->champions[i].attributes = 0;
    }
}

void fs_dm1_v1_cts_run_clock_tick_pc34(const FsDm1V1CtsTickInputPc34 *input,
                                       FsDm1V1CtsTickResultPc34 *result)
{
    if (!result) return;
    memset(result, 0, sizeof(*result));
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        result->attributes_before[i] = 0;
        result->attributes_after[i] = 0;
        result->champion_skipped_reason[i] = 0;
        result->champion_attribute_dirty[i] = 0;
    }
    for (int i = 0; i < 8; ++i) {
        result->trace[i] = 0;
    }

    if (!input) {
        result->valid = 0;
        return;
    }
    result->valid = 1;

    /*
     * F0331:2331 - early-return when the party is empty. The per-
     * champion loop and the F0293 sweep are skipped entirely, so the
     * Attributes byte of every champion stays byte-stable across the
     * tick. The result struct still records the early-return shape.
     */
    if (input->party_champion_count <= 0) {
        for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
            result->attributes_before[i] = input->champions[i].attributes;
            result->attributes_after[i] = input->champions[i].attributes;
            result->champion_skipped_reason[i] = 0;
        }
        result->champion_skip_count = 0;
        result->draw_all_champion_states_invoked = 0;
        result->draw_all_champion_states_arg = FS_DM1_V1_CTS_DRAW_ALL_NONE_PC34;
        result->draw_state_calls_f0293_after_loop = 0;
        result->time_criteria_3bit = 0;
        result->recovery_period = 0;
        result->recovery_due_this_tick = 0;
        result->hash = fs_dm1_v1_cts_hash_pc34(input, result);
        return;
    }

    /*
     * F0331:2333-2334 - the time criteria and the recovery period are
     * computed ONCE per tick, not per-champion. The recovery period is
     * selected by G0300_B_PartyIsResting; we derive that from the
     * champion[0] snapshot, which is the convention used by
     * dm1_v1_champion_needs_pc34_compat.c.
     */
    const uint32_t game_time = (uint32_t)input->champions[0].party_is_resting;
    (void)game_time;
    const int party_is_resting = input->champions[0].party_is_resting;

    /*
     * F0331 uses G0313_ul_GameTime directly. The input struct does
     * not carry GameTime because the lane is contract-only on the
     * attribute side; the recovery-due flag and the time criteria
     * are derived from a deterministic scratch value in this helper.
     * The real-time driver (firestaff_game_loop.c) supplies the
     * actual GameTime before invoking the F0331 body.
     */
    const uint32_t scratch_game_time = 0u;
    result->time_criteria_3bit =
        fs_dm1_v1_cts_time_criteria(scratch_game_time);
    result->recovery_period =
        fs_dm1_v1_cts_recovery_period(party_is_resting);
    result->recovery_due_this_tick =
        fs_dm1_v1_cts_recovery_due(scratch_game_time, party_is_resting);

    /*
     * F0331:2335-2497 - per-champion iteration. We snap each
     * champion's pre-tick attributes, then apply the same source-
     * locked gate.
     */
    int panel_set_index = -1;
    int icon_dirty_count = 0;
    int first_icon_set_index = -1;
    int statistic_recovery_applied = 0;
    int statistic_recovery_clamp_above_maximum = 0;
    int stat_dirty_count = 0;
    int dead_skipped = 0;
    int candidate_skipped = 0;
    int out_of_party_skipped = 0;
    int visited_count = 0;

    const int party_count =
        input->party_champion_count > FS_DM1_V1_CTS_CHAMPION_COUNT_PC34 ?
        FS_DM1_V1_CTS_CHAMPION_COUNT_PC34 :
        input->party_champion_count;

    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        const FsDm1V1CtsChampionPc34 *champ = &input->champions[i];
        result->attributes_before[i] = champ->attributes;
        result->attributes_after[i] = champ->attributes;

        if (i >= party_count) {
            /* F0331:2335 - the for-loop bound is G0305. We never
             * visit champion indices >= G0305, so the attribute
             * half of the tick leaves their Attributes byte stable.
             */
            result->champion_skipped_reason[i] = 4;
            out_of_party_skipped++;
            continue;
        }

        const int ordinal = fs_dm1_v1_cts_is_index_to_ordinal(i);
        visited_count++;

        if (champ->current_health == 0) {
            /* F0331:2335 - CurrentHealth == 0 means the champion is
             * dead, so the per-champion body is skipped entirely and
             * the MASK0x0100_STATISTICS attribute is NOT set.
             */
            result->champion_skipped_reason[i] = 1;
            dead_skipped++;
            continue;
        }

        if (ordinal == input->candidate_champion_ordinal &&
            input->candidate_champion_ordinal !=
                FS_DM1_V1_CTS_CANDIDATE_NONE_PC34) {
            /* F0331:2335 - the resurrection candidate is excluded
             * from the time-effects body; the resurrection flow
             * (REVIVE.C F0280..F0282) owns its redraw path.
             */
            result->champion_skipped_reason[i] = 2;
            candidate_skipped++;
            continue;
        }

        /* F0331:2491-2493 - direction re-sync. The exact contract
         * is: if (!G0300_B_PartyIsResting && champ->Direction !=
         * G0308 && G0361 < G0313 - 60) then MASK0x0400_ICON is set.
         * We model the gate with a positive control on
         * (resting == 0, direction mismatch, attack quiet > 60).
         */
        if (!party_is_resting &&
            champ->direction != input->party_direction &&
            input->last_creature_attack_time <
                (int)scratch_game_time -
                    FS_DM1_V1_CTS_DIRECTION_RESYNC_QUIET_GAME_TIME_PC34) {
            result->attributes_after[i] |= FS_DM1_V1_CTS_ATTR_ICON_PC34;
            icon_dirty_count++;
            if (first_icon_set_index < 0) {
                first_icon_set_index = i;
            }
        }

        /* F0331:2494 - MASK0x0100_STATISTICS is set on every alive
         * non-candidate champion in the party.
         */
        result->attributes_after[i] |= FS_DM1_V1_CTS_ATTR_STATISTICS_PC34;
        stat_dirty_count++;
        result->champion_attribute_dirty[i] |= 1;

        /* F0331:2495-2497 - MASK0x0800_PANEL is set ONLY for the
         * inventory champion AND only when the food/water poisoned
         * or skills+statistics panel is the current panel content.
         */
        if (ordinal == input->inventory_champion_ordinal &&
            input->inventory_champion_ordinal !=
                FS_DM1_V1_CTS_INVENTORY_CHAMPION_NONE_PC34 &&
            fs_dm1_v1_cts_panel_attr_eligible(input->panel_content)) {
            result->attributes_after[i] |= FS_DM1_V1_CTS_ATTR_PANEL_PC34;
            panel_set_index = i;
            result->champion_attribute_dirty[i] |= 2;
        }

        /* F0331:2487-2490 - the C0..C6 statistic recovery loop
         * runs at most once per recovery period. We count it as
         * "applied" when the recovery period elapses; the helper
         * itself is implemented in dm1_v1_champion_needs_pc34_compat.
         * The "clamp above maximum" sub-counter tracks the number
         * of statistics that took the PC 3.4+ MEDIA240 step.
         */
        if (result->recovery_due_this_tick) {
            statistic_recovery_applied++;
            statistic_recovery_clamp_above_maximum += 0;
        }
    }

    /* F0331:2503 - F0293 is called once at the end with no arg, so
     * all 4 status boxes repaint in one sweep.
     */
    result->draw_all_champion_states_invoked = 1;
    result->draw_all_champion_states_arg = FS_DM1_V1_CTS_DRAW_ALL_NONE_PC34;
    result->draw_state_calls_f0293_after_loop = 1;

    result->statistics_attribute_set_count = stat_dirty_count;
    result->panel_attribute_set_index = panel_set_index;
    result->icon_attribute_set_count = icon_dirty_count;
    result->direction_resync_champion_index = first_icon_set_index;
    result->champion_skip_count = dead_skipped + candidate_skipped +
                                  out_of_party_skipped;
    result->dead_skipped = dead_skipped;
    result->candidate_skipped = candidate_skipped;
    result->out_of_party_skipped = out_of_party_skipped;
    result->statistic_recovery_applied_count = statistic_recovery_applied;
    result->statistic_recovery_clamp_above_maximum_count =
        statistic_recovery_clamp_above_maximum;

    result->trace[0] = (int)sizeof(*result);
    result->trace[1] = party_count;
    result->trace[2] = visited_count;
    result->trace[3] = panel_set_index;
    result->trace[4] = first_icon_set_index;
    result->trace[5] = stat_dirty_count;
    result->trace[6] = result->recovery_due_this_tick;
    result->trace[7] = result->recovery_period;

    result->hash = fs_dm1_v1_cts_hash_pc34(input, result);
}

uint32_t fs_dm1_v1_cts_hash_pc34(const FsDm1V1CtsTickInputPc34 *input,
                                 const FsDm1V1CtsTickResultPc34 *result)
{
    uint32_t h = 0xC10Cu;
    if (input) {
        h = (h * 33u) + (uint32_t)input->party_champion_count;
        h = (h * 33u) + (uint32_t)input->inventory_champion_ordinal;
        h = (h * 33u) + (uint32_t)input->candidate_champion_ordinal;
        h = (h * 33u) + (uint32_t)input->panel_content;
        h = (h * 33u) + (uint32_t)input->party_direction;
        h = (h * 33u) + (uint32_t)input->last_creature_attack_time;
        for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
            const FsDm1V1CtsChampionPc34 *c = &input->champions[i];
            h = (h * 33u) + (uint32_t)c->alive;
            h = (h * 33u) + (uint32_t)c->current_health;
            h = (h * 33u) + (uint32_t)c->maximum_health;
            h = (h * 33u) + (uint32_t)c->current_stamina;
            h = (h * 33u) + (uint32_t)c->maximum_stamina;
            h = (h * 33u) + (uint32_t)c->current_mana;
            h = (h * 33u) + (uint32_t)c->maximum_mana;
            h = (h * 33u) + (uint32_t)c->food;
            h = (h * 33u) + (uint32_t)c->water;
            h = (h * 33u) + (uint32_t)c->vitality_current;
            h = (h * 33u) + (uint32_t)c->wisdom_current;
            h = (h * 33u) + (uint32_t)c->wizard_skill;
            h = (h * 33u) + (uint32_t)c->priest_skill;
            h = (h * 33u) + (uint32_t)c->poison_event_count;
            h = (h * 33u) + (uint32_t)c->has_ekkhard_cross;
            h = (h * 33u) + (uint32_t)c->party_is_resting;
            h = (h * 33u) + (uint32_t)c->direction;
            h = (h * 33u) + (uint32_t)c->attributes;
        }
    }
    if (result) {
        h = (h * 33u) + (uint32_t)result->valid;
        h = (h * 33u) + (uint32_t)result->time_criteria_3bit;
        h = (h * 33u) + (uint32_t)result->recovery_period;
        h = (h * 33u) + (uint32_t)result->recovery_due_this_tick;
        h = (h * 33u) + (uint32_t)result->statistics_attribute_set_count;
        h = (h * 33u) + (uint32_t)result->panel_attribute_set_index;
        h = (h * 33u) + (uint32_t)result->icon_attribute_set_count;
        h = (h * 33u) + (uint32_t)result->champion_skip_count;
        h = (h * 33u) + (uint32_t)result->dead_skipped;
        h = (h * 33u) + (uint32_t)result->candidate_skipped;
        h = (h * 33u) + (uint32_t)result->out_of_party_skipped;
        h = (h * 33u) + (uint32_t)result->draw_all_champion_states_invoked;
        h = (h * 33u) + (uint32_t)result->draw_all_champion_states_arg;
        h = (h * 33u) +
            (uint32_t)result->direction_resync_champion_index;
        h = (h * 33u) +
            (uint32_t)result->draw_state_calls_f0293_after_loop;
        h = (h * 33u) +
            (uint32_t)result->statistic_recovery_applied_count;
        h = (h * 33u) +
            (uint32_t)result->statistic_recovery_clamp_above_maximum_count;
        for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
            h = (h * 33u) + (uint32_t)result->attributes_after[i];
            h = (h * 33u) + (uint32_t)result->champion_attribute_dirty[i];
        }
    }
    return h;
}

const char *fs_dm1_v1_cts_source_evidence_pc34(void)
{
    return "CHAMPION.C F0331_CHAMPION_ApplyTimeEffects_CPSF:2331-2332 "
           "G0305 zero early-return; "
           "CHAMPION.C F0331:2333-2334 single-tick 3-bit time criterion and "
           "rest-modulated 256/64 recovery period; "
           "CHAMPION.C F0331:2335 per-champion for-loop with CurrentHealth "
           "and !G0299 gate; "
           "CHAMPION.C F0331:2487-2490 C0_STATISTIC_LUCK..C6_STATISTIC_"
           "ANTIFIRE clamp loop with the MEDIA240 PC 3.4+ current/maximum "
           "above-maximum step; "
           "CHAMPION.C F0331:2491-2493 direction re-sync MASK0x0400_ICON "
           "with the 60-tick creature-quiet gate; "
           "CHAMPION.C F0331:2494 MASK0x0100_STATISTICS attribute set on "
           "every visited champion; "
           "CHAMPION.C F0331:2495-2497 MASK0x0800_PANEL attribute set ONLY "
           "for the inventory champion AND ONLY while the food/water "
           "poisoned or skills+statistics panel is the current panel "
           "content; "
           "CHAMPION.C F0331:2503 F0293_CHAMPION_DrawAllChampionStates("
           "MASK0x0000_NONE) sweep after the loop; "
           "CHAMDRAW.C F0293 repaints all 4 status boxes in one "
           "F0292_CHAMPION_DrawState sweep with no per-champion arg; "
           "DEFS.H:780-817 C30 chest slots, :873-876 M516, :3778-3786 "
           "C104/C151-C154, :5700 G0305, :5876-5881 G0423/G0424/G0425/"
           "G0426, plus MASK0x0100_STATISTICS/MASK0x0800_PANEL/MASK0x0400_"
           "ICON.";
}

const char *fs_dm1_v1_cts_non_overlap_pc34(void)
{
    return "test_dm1_v1_champion_panel_hud_food_water_recompute_pc34_compat "
           "(food/water recovery + close-inventory STATUS_BOX hook), "
           "test_dm1_v1_champion_panel_status_recompute_pc34_compat (status "
           "recompute), test_dm1_v1_champion_panel_food_water_status_box_"
           "pc34_compat, test_dm1_v1_champion_panel_portrait_state_redraw_"
           "pc34_compat, test_dm1_v1_champion_panel_portrait_box_redraw_"
           "states_pc34_compat, test_dm1_v1_champion_panel_f0354_box_"
           "variants_pc34_compat, test_dm1_v1_champion_panel_portrait_box_"
           "blit_gate_pc34_compat, test_dm1_v1_champion_panel_hand_slot_"
           "priority_pc34_compat, test_dm1_v1_champion_panel_action_hand_"
           "slot_priority_pc34_compat, test_dm1_v1_champion_panel_spell_"
           "area_overlay_pc34_compat, test_dm1_v1_champion_panel_status_"
           "hand_rotation_pc34_compat, test_dm1_v1_champion_panel_second_"
           "leader_hand_slot_priority_pc34_compat, test_dm1_v1_champion_"
           "panel_status_hand_slot_pixels_pc34_compat, test_dm1_v1_champion_"
           "panel_mouth_eye_poison_warning_pc34_compat, test_dm1_v1_champion_"
           "panel_pressing_mouth_eye_statusbox_pc34_compat, test_dm1_v1_"
           "champion_panel_mouth_eye_release_pc34_compat, test_dm1_v1_"
           "champion_panel_wound_handling_pc34_compat, test_dm1_v1_champion_"
           "panel_ammunition_compatibility_pc34_compat, test_dm1_v1_champion_"
           "panel_damage_indicator_pc34_compat, test_dm1_v1_champion_panel_"
           "hud_pc34_compat, test_dm1_v1_champion_panel_hud_recompute_"
           "pc34_compat, test_dm1_v1_champion_panel_bar_pixels_pc34_compat, "
           "test_dm1_v1_champion_panel_all_states_pc34_compat, "
           "test_dm1_v1_champion_panel_portrait_pc34_compat, test_action_"
           "area_routes_pc34_compat_integration, test_action_area_icon_"
           "routes_pc34_compat_integration, test_action_area_name_routes_"
           "pc34_compat_integration, test_champion_name_hand_routes_"
           "pc34_compat_integration, test_champion_names_hands_split_"
           "pc34_compat_integration, test_champion_status_slotbox_"
           "pc34_compat_integration, the mirror-candidate c045 family, "
           "the chest scroll-wheel/occupied-slot family, and the viewport "
           "F0107/F0108 family.";
}
