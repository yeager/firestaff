#include "csb_v1_f0806_entrance_loop_runtime_handoff_pc34_compat.h"

#include <string.h>

static int csb_v1_f0806_entrance_draw_matches_pc34(
    const CSB_V1_StartEndEntranceBoundaryReceipt_PC34 *receipt)
{
    return receipt && receipt->valid &&
        (receipt->accepted_stage_mask &
         CSB_V1_STARTEND_F0439_DRAW_ENTRANCE_PC34) &&
        receipt->real_asset_matched &&
        receipt->host_view_consumed &&
        receipt->host_draw_consumed &&
        receipt->draw_consumes_receipt_only &&
        receipt->no_synthetic_payloads &&
        receipt->no_fallback_graphics &&
        receipt->route_wrappers_retired;
}

static int csb_v1_f0806_credits_route_matches_pc34(
    const CSB_V1_F0806_EntranceLoopFacts_PC34 *facts)
{
    const CSB_V1_StartEndEntranceBoundaryReceipt_PC34 *receipt;

    if (!facts || facts->credits_command_loops_before_exit == 0) {
        return 1;
    }

    receipt = &facts->draw_credits;
    return facts->credits_command_loops_before_exit > 0 &&
        receipt->valid &&
        (receipt->accepted_stage_mask &
         CSB_V1_STARTEND_F0442_DRAW_CREDITS_PC34) &&
        receipt->real_asset_matched &&
        receipt->host_view_consumed &&
        receipt->host_draw_consumed &&
        receipt->draw_consumes_receipt_only &&
        receipt->no_synthetic_payloads &&
        receipt->no_fallback_graphics &&
        receipt->route_wrappers_retired;
}

static CSB_V1_F0806_EntranceLoopDecision_PC34 csb_v1_f0806_decision_pc34(
    const CSB_V1_F0806_EntranceLoopFacts_PC34 *facts)
{
    if (!facts) return CSB_V1_F0806_ENTRANCE_LOOP_DECISION_NONE_PC34;

    if (facts->final_new_game_mode ==
        CSB_V1_F0806_MODE_LOAD_SAVED_GAME_PC34) {
        return CSB_V1_F0806_ENTRANCE_LOOP_DECISION_LOAD_SAVED_GAME_PC34;
    }

    if (facts->final_new_game_mode == CSB_V1_F0806_MODE_LOAD_DUNGEON_PC34 &&
        facts->bonus_dungeon_selected) {
        return CSB_V1_F0806_ENTRANCE_LOOP_DECISION_LOAD_BONUS_DUNGEON_PC34;
    }

    if (facts->final_new_game_mode == CSB_V1_F0806_MODE_LOAD_DUNGEON_PC34) {
        return CSB_V1_F0806_ENTRANCE_LOOP_DECISION_LOAD_DUNGEON_PC34;
    }

    return CSB_V1_F0806_ENTRANCE_LOOP_DECISION_NONE_PC34;
}

static int csb_v1_f0806_final_command_matches_pc34(
    const CSB_V1_F0806_EntranceLoopFacts_PC34 *facts,
    CSB_V1_F0806_EntranceLoopDecision_PC34 decision)
{
    if (decision == CSB_V1_F0806_ENTRANCE_LOOP_DECISION_LOAD_SAVED_GAME_PC34) {
        return facts->final_command == CSB_V1_F0806_MODE_LOAD_SAVED_GAME_PC34;
    }
    if (decision == CSB_V1_F0806_ENTRANCE_LOOP_DECISION_LOAD_DUNGEON_PC34) {
        return facts->final_command ==
            CSB_V1_F0806_COMMAND_ENTRANCE_ENTER_DUNGEON_PC34;
    }
    if (decision ==
        CSB_V1_F0806_ENTRANCE_LOOP_DECISION_LOAD_BONUS_DUNGEON_PC34) {
        return facts->final_command ==
            CSB_V1_F0806_COMMAND_ENTRANCE_ENTER_BONUS_DUNGEON_PC34;
    }
    return 0;
}

static int csb_v1_f0806_requires_door_animation_pc34(
    CSB_V1_F0806_EntranceLoopDecision_PC34 decision)
{
    return decision == CSB_V1_F0806_ENTRANCE_LOOP_DECISION_LOAD_DUNGEON_PC34 ||
        decision == CSB_V1_F0806_ENTRANCE_LOOP_DECISION_LOAD_BONUS_DUNGEON_PC34;
}

void csb_v1_f0806_entrance_loop_receipt_init_pc34(
    CSB_V1_F0806_EntranceLoopReceipt_PC34 *receipt)
{
    if (receipt) memset(receipt, 0, sizeof(*receipt));
}

int F0806_F0806_ENTRANCE_int(
    const CSB_V1_F0806_EntranceLoopFacts_PC34 *facts,
    CSB_V1_F0806_EntranceLoopReceipt_PC34 *out_receipt)
{
    const char *evidence =
        csb_v1_f0806_entrance_loop_source_evidence_pc34();
    CSB_V1_F0806_EntranceLoopDecision_PC34 decision =
        csb_v1_f0806_decision_pc34(facts);
    int requires_door = csb_v1_f0806_requires_door_animation_pc34(decision);

    csb_v1_f0806_entrance_loop_receipt_init_pc34(out_receipt);
    if (!facts || !facts->valid ||
        !facts->entrance_assets_bound ||
        !facts->entrance_input_tables_bound ||
        !facts->entrance_music_started ||
        !facts->draw_entrance_each_outer_loop ||
        !facts->pointer_shown_before_wait ||
        !facts->input_discarded_before_wait ||
        !facts->waiting_mode_set_before_queue ||
        !facts->command_queue_processed_until_exit ||
        facts->credits_command_loops_before_exit < 0 ||
        decision == CSB_V1_F0806_ENTRANCE_LOOP_DECISION_NONE_PC34 ||
        !csb_v1_f0806_final_command_matches_pc34(facts, decision) ||
        !facts->post_selection_switch_sound_played ||
        facts->post_selection_delay_ticks != 20 ||
        !facts->temporary_entrance_memory_released ||
        !facts->no_synthetic_input ||
        !facts->no_synthetic_graphics_bytes ||
        !facts->no_fallback_visuals ||
        !facts->no_legacy_entrance_wrapper ||
        !csb_v1_f0806_entrance_draw_matches_pc34(&facts->draw_entrance) ||
        !csb_v1_f0806_credits_route_matches_pc34(facts) ||
        !facts->micro_dungeon.valid ||
        !facts->micro_dungeon.no_loaded_dungeon_substitute ||
        !facts->micro_dungeon.no_synthetic_viewport_pixels ||
        !facts->micro_dungeon.no_legacy_micro_dungeon_wrapper ||
        (requires_door && (!facts->door_animation_step.valid ||
                           !facts->door_animation_step.bitplanes_consumed ||
                           !facts->door_animation_step.runtime_coupling_consumed ||
                           !facts->door_animation_step.draw_consumes_receipt_only ||
                           !facts->door_animation_step.input_consumes_receipt_only ||
                           !facts->door_animation_step.no_legacy_animation_wrapper ||
                           !facts->door_animation_step.no_synthetic_visuals)) ||
        (!requires_door && facts->door_animation_step.valid)) {
        if (out_receipt) {
            out_receipt->no_synthetic_input = 1;
            out_receipt->no_synthetic_graphics_bytes = 1;
            out_receipt->no_fallback_visuals = 1;
            out_receipt->source_evidence = evidence;
        }
        return 0;
    }

    out_receipt->valid = 1;
    out_receipt->decision = decision;
    out_receipt->final_new_game_mode = facts->final_new_game_mode;
    out_receipt->final_command = facts->final_command;
    out_receipt->bonus_dungeon_selected = facts->bonus_dungeon_selected;
    out_receipt->entrance_draw_consumed = 1;
    out_receipt->credits_route_consumed =
        facts->credits_command_loops_before_exit > 0;
    out_receipt->micro_dungeon_consumed = 1;
    out_receipt->door_animation_consumed = requires_door;
    out_receipt->waiting_loop_source_locked = 1;
    out_receipt->command_queue_source_locked = 1;
    out_receipt->temporary_entrance_memory_released = 1;
    out_receipt->no_synthetic_input = 1;
    out_receipt->no_synthetic_graphics_bytes = 1;
    out_receipt->no_fallback_visuals = 1;
    out_receipt->no_legacy_entrance_wrapper = 1;
    out_receipt->source_evidence = evidence;
    return 1;
}

const char *csb_v1_f0806_entrance_loop_source_evidence_pc34(void)
{
    return "ReDMCSB ENTRANCE.C:721-826/850-903 F0806_F0806_ENTRANCE_int "
           "loads the real C002-C005 entrance assets, sets entrance input "
           "tables and music, redraws F0439 around the wait loop, discards "
           "input before setting C099_MODE_WAITING_ON_ENTRANCE, processes "
           "commands until C000/C001, loops M567 credits through F0442, plays "
           "the switch sound, delays 20 ticks, opens doors only for nonzero "
           "G0298_B_NewGame, and releases the temporary entrance allocation";
}
