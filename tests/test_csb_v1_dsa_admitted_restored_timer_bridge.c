#include "csb_v1_csbwin_dsa_runtime_admission_pc34_compat.h"
#include "m11_game_view.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static void put_le16(uint8_t *bytes, size_t offset, uint16_t value)
{
    bytes[offset] = (uint8_t)value;
    bytes[offset + 1u] = (uint8_t)(value >> 8);
}

static void put_le32(uint8_t *bytes, size_t offset, uint32_t value)
{
    bytes[offset] = (uint8_t)value;
    bytes[offset + 1u] = (uint8_t)(value >> 8);
    bytes[offset + 2u] = (uint8_t)(value >> 16);
    bytes[offset + 3u] = (uint8_t)(value >> 24);
}

static uint32_t fnv1a32(const uint8_t *bytes, size_t size)
{
    uint32_t hash = 2166136261u;
    size_t i;
    for (i = 0u; i < size; ++i) hash = (hash ^ bytes[i]) * 16777619u;
    return hash;
}

int main(void)
{
    uint8_t raw[128] = { 0 };
    uint8_t tail[2u * CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES] = { 0 };
    uint16_t store_global[] = { 0x0686u, 0x55aau, 0x0054u };
    uint16_t message_then_unowned_sound[] = {
        0x0b41u, 2u, 0u, 0x114bu
    };
    uint16_t message[] = { 0x0b41u, 2u, 0u };
    uint16_t false_pit[] = {
        0x0686u, 0u, 0x0686u, 1u, 0x084bu
    };
    uint16_t dynamic_jump_gear[] = {
        0x0686u, 0u, 0x0686u, 9u, 0x188bu
    };
    uint16_t comparison_store[] = {
        0x0686u, 0x55aau, 0x0686u, 0x55aau, 0x014bu, 0x0054u
    };
    /* Target state 9 returns to source state 1: the real LocalState=2 save
     * route therefore has no invented persistent-state write to perform. */
    uint16_t direct_jump[] = { 0x814cu, 0xfff8u };
    const uint32_t global_record_id = (5u << 24) | (4u << 16);
    const uint32_t global_bucket = 32u +
        ((global_record_id * 0xbb40e62du) >> 27);
    CSB_V1_DungeonData dungeon;
    CSB_V1_BootProfile boot;
    CSB_V1_DSAImportedAction actions[2];
    CSB_V1_CSBWin512TimerSummary timer;
    CSB_V1_CSBWinDSASaveRuntimeHandoffReceipt_PC34 handoff;
    CSB_V1_StartupRuntimeAssetSession_PC34 session;
    CSB_V1_CSBWinDSARestoredTimerExecutionReceipt_PC34 bridge;
    CSB_V1_CSBWinDSARuntimeChainReceipt_PC34 chain;
    CSB_V1_DSAFilterLocation location;
    CSB_V1_RuntimeDSAFilterBinding binding;
    CSB_V1_DSAFilterRuntime filter;
    CSB_V1_RuntimeDSAFilterStackAdapter adapter;
    M11_GameViewState view;
    int timeline_count_before;

    memset(&dungeon, 0, sizeof(dungeon));
    memset(&boot, 0, sizeof(boot));
    memset(actions, 0, sizeof(actions));
    memset(&timer, 0, sizeof(timer));
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 1;
    dungeon.level_heights[0] = 1;
    dungeon.level_offsets[0] = 80;
    dungeon.square_bytes = 1;
    dungeon.square_first_thing_base = 62;
    dungeon.square_first_thing_count = 1;
    dungeon.text_data_base = 104;
    dungeon.text_word_count = 2;
    dungeon.thing_data_bases[CSB_V1_THING_TYPE_ACTUATOR] = 90;
    dungeon.thing_type_counts[CSB_V1_THING_TYPE_ACTUATOR] = 1;
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    raw[80] = 0x10u;
    put_le16(raw, 62u, (uint16_t)(CSB_V1_THING_TYPE_ACTUATOR << 10));
    put_le16(raw, 90u, 0xfffeu);
    put_le16(raw, 92u, 0x012fu);
    put_le16(raw, 96u, 1u);

    csb_v1_runtime_init(&boot.runtime, NULL);
    boot.assets_verified = 1;
    boot.dungeon_verified = 1;
    boot.runtime.dungeon_handle = &dungeon;
    snprintf(boot.dungeon_md5, sizeof(boot.dungeon_md5), "%s",
             "6695d2acebce49f95db1d8f3a5c733de");
    boot.runtime.csbwin_body_runtime_summary_valid = 1;
    boot.runtime.csbwin_extended_features_valid = 1;
    boot.runtime.csbwin_extended_level_index_present = 1;
    boot.runtime.csbwin_extended_level_dsa_index[0][2] = 7u;
    boot.runtime.csbwin_extended_level_dsa_index[1][2] = 7u;
    boot.runtime.csbwin_extended_level_dsa_index[2][2] = 7u;
    boot.runtime.csbwin_global_variables_valid = 1;
    boot.runtime.csbwin_global_variable_count = 16u;
    put_le16(tail, 2u, 3u);
    put_le16(tail, 64u * 4u + 2u, 18u);
    put_le32(tail, (size_t)global_bucket * 4u, 65u);
    put_le32(tail, 65u * 4u, 0u);
    put_le32(tail, 66u * 4u, global_record_id);
    boot.runtime.csbwin_appended_tail_valid = 1;
    boot.runtime.csbwin_appended_tail_size = sizeof(tail);
    boot.runtime.csbwin_appended_tail_preserved_size = sizeof(tail);
    memcpy(boot.runtime.csbwin_appended_tail, tail, sizeof(tail));
    boot.runtime.csbwin_appended_tail_fnv1a = fnv1a32(tail, sizeof(tail));
    actions[0].dsa_id = 7u;
    actions[0].state_index = 1u;
    actions[0].column = 0u;
    actions[0].program_words = store_global;
    actions[0].program_word_count = 3;
    actions[1].dsa_id = 7u;
    actions[1].state_index = 9u;
    actions[1].column = 0u;
    actions[1].program_words = direct_jump;
    actions[1].program_word_count = 2;
    boot.runtime.csbwin_extended_dsa_state.imported_actions = actions;
    boot.runtime.csbwin_extended_dsa_state.imported_action_count = 2;
    boot.runtime.csbwin_extended_dsa_state.imported_headers[7].valid = 1;
    boot.runtime.csbwin_extended_dsa_state.imported_headers[7].local_state = 2u;
    boot.runtime.csbwin_extended_dsa_state.imported_headers[7].state_slot_count = 2u;
    timer.valid = 1;
    timer.source_index = 0u;
    timer.function = 6u;
    timer.time = boot.runtime.game_time;
    boot.runtime.csbwin_timer_summary_count = 1u;
    boot.runtime.csbwin_timer_summary_total = 1u;
    boot.runtime.csbwin_timer_queue_summary_count = 1u;
    boot.runtime.csbwin_timer_queue_summary_total = 1u;
    boot.runtime.csbwin_max_timers = 1u;
    boot.runtime.csbwin_num_timer = 1u;
    boot.runtime.csbwin_first_avail_timer = 1u;
    boot.runtime.csbwin_timers[0] = timer;
    boot.runtime.csbwin_timer_queue[0] = 0u;
    memset(&session, 0, sizeof(session));
    session.valid = session.real_asset_matched = session.full_startup_ready = 1;
    session.rejects_legacy_wrappers = 1;
    session.generation = 4u;
    session.source_tick = 12u;
    session.csbStartupPackageIdentity = 0x5cb00001u;
    memset(&handoff, 0, sizeof(handoff));
    handoff.valid = handoff.source_admission_consumed = 1;
    handoff.save_handoff_consumed = handoff.dungeon_identity_current = 1;
    handoff.startup_session_consumed = handoff.runtime_chain_consumed = 1;
    handoff.save_fnv1a = 0x11u;
    handoff.gameblock_fnv1a = 0x22u;
    handoff.source_admission_hash = handoff.handoff_hash = 0x44u;
    handoff.startup_session_generation = session.generation;
    handoff.startup_source_tick = session.source_tick;
    handoff.runtime_game_time = boot.runtime.game_time;
    handoff.live_timer_event_count = 1u;
    handoff.imported_action_count = 2;
    snprintf(handoff.dungeon_md5, sizeof(handoff.dungeon_md5), "%s",
             boot.dungeon_md5);

    memset(&view, 0, sizeof(view));
    view.active = 1;
    view.sourceKind = M11_GAME_SOURCE_CSB_BOOT;
    view.csbBootProfile = &boot;
    view.csbStartupRuntimeAssetSession = &session;
    view.csbStartupExpectedPackageIdentity = session.csbStartupPackageIdentity;

    memset(&location, 0, sizeof(location));
    location.level = 0;
    location.x = 0;
    location.y = 0;
    location.position = 0;
    location.actuator_thing = (uint16_t)(CSB_V1_THING_TYPE_ACTUATOR << 10);
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&boot.runtime) == 1,
          "fixture materializes restored TimerQueue before bridge consumption");
    check(csb_v1_runtime_csbwin_dsa_runtime_chain_receipt_pc34(
              &boot.runtime, &chain) && chain.live_timer_event_count == 1u,
          "fixture retains one source-owned queued timer chain");
    check(csb_v1_csbwin_dsa_execute_restored_timer_pc34(
              &boot, &handoff, &session, &location, 0u, &bridge) &&
              bridge.valid && bridge.action_executed &&
              bridge.timer_function == 6u && bridge.timer_index == 0u &&
              bridge.variable_core && !bridge.timer_core &&
              bridge.timer_owner_hash != 0u &&
              csb_v1_csbwin_dsa_restored_timer_receipt_current_pc34(
                  &boot, &handoff, &session, &bridge) &&
              boot.runtime.csbwin_global_variables[1] == 0x55aau,
          "admitted restored timer executes only its verified local-state action");
    boot.runtime.csbwin_timers[0].function = 7u;
    check(!csb_v1_csbwin_dsa_restored_timer_receipt_current_pc34(
              &boot, &handoff, &session, &bridge),
          "timer-owner function drift invalidates the published receipt");
    boot.runtime.csbwin_timers[0].function = 6u;
    boot.runtime.csbwin_timers[0].ubyte9 = 1u;
    check(!csb_v1_csbwin_dsa_restored_timer_receipt_current_pc34(
              &boot, &handoff, &session, &bridge),
          "timer-owner action drift invalidates the published receipt");
    boot.runtime.csbwin_timers[0].ubyte9 = 0u;
    boot.runtime.csbwin_timers[0].ubyte6 = 1u;
    check(!csb_v1_csbwin_dsa_restored_timer_receipt_current_pc34(
              &boot, &handoff, &session, &bridge),
          "timer-owner location drift invalidates the published receipt");
    boot.runtime.csbwin_timers[0].ubyte6 = 0u;
    check(csb_v1_csbwin_dsa_restored_timer_receipt_current_pc34(
              &boot, &handoff, &session, &bridge),
          "unaltered saved TIMER owner round-trips through currentness");
    actions[0].program_words = message;
    actions[0].program_word_count = (int)(sizeof(message) / sizeof(message[0]));
    check(csb_v1_csbwin_dsa_execute_restored_timer_pc34(
              &boot, &handoff, &session, &location, 0u, &bridge) &&
              bridge.timer_core && bridge.message_core &&
              bridge.timer_scheduled_count == 1u &&
              bridge.last_scheduled_target_location == 0u &&
              csb_v1_csbwin_dsa_restored_timer_receipt_current_pc34(
                  &boot, &handoff, &session, &bridge),
          "restored timer binds CSBWin MESSAGE queue side effect to save identity");
    bridge.last_scheduled_event_type++;
    check(!csb_v1_csbwin_dsa_restored_timer_receipt_current_pc34(
              &boot, &handoff, &session, &bridge),
          "queued MESSAGE event drift invalidates the restored save receipt");
    actions[0].program_words = store_global;
    actions[0].program_word_count = 3;
    actions[0].program_words = comparison_store;
    actions[0].program_word_count = (int)(sizeof(comparison_store) /
                                          sizeof(comparison_store[0]));
    boot.runtime.csbwin_global_variables[1] = 0u;
    check(csb_v1_csbwin_dsa_execute_restored_timer_pc34(
              &boot, &handoff, &session, &location, 0u, &bridge) &&
              bridge.comparison_core && bridge.arithmetic_core &&
              bridge.action_program_fnv1a != 0u &&
              boot.runtime.csbwin_global_variables[1] == 1u &&
              csb_v1_csbwin_dsa_restored_timer_receipt_current_pc34(
                  &boot, &handoff, &session, &bridge),
          "restored timer binds CSBWin comparison program bytes to save identity");
    comparison_store[3] = 0x55abu;
    check(!csb_v1_csbwin_dsa_restored_timer_receipt_current_pc34(
              &boot, &handoff, &session, &bridge),
          "comparison program-word drift invalidates the restored save receipt");
    comparison_store[3] = 0x55aau;
    actions[0].program_words = store_global;
    actions[0].program_word_count = 3;
    actions[0].program_words = message_then_unowned_sound;
    actions[0].program_word_count = (int)(sizeof(message_then_unowned_sound) /
                                          sizeof(message_then_unowned_sound[0]));
    timeline_count_before = boot.runtime.timeline_queue.eventCount;
    check(!csb_v1_csbwin_dsa_execute_restored_timer_pc34(
              &boot, &handoff, &session, &location, 0u, &bridge) &&
              boot.runtime.timeline_queue.eventCount == timeline_count_before,
          "restored MESSAGE cannot publish a timer before a later unowned opcode");
    actions[0].program_words = store_global;
    actions[0].program_word_count = (int)(sizeof(store_global) /
                                          sizeof(store_global[0]));
    boot.runtime.csbwin_global_variables[1] = 0u;
    check(M11_GameView_BindCSBDSASaveRuntimeHandoff(&view, &handoff) == 1 &&
              M11_GameView_ExecuteCSBDSARestoredTimer(&view, &location, 0u) == 1 &&
              view.csbDsaRestoredTimerTransactionRequired &&
              view.csbDsaRestoredTimerTransactionReceipt.valid &&
              view.csbDsaRestoredTimerTransactionReceipt.queue_slot == 0u &&
              view.csbDsaRestoredTimerTransactionReceipt.timer_index == 0u &&
              boot.runtime.csbwin_global_variables[1] == 0x55aau,
          "M11 commits only the source-ordered admitted CSBWin timer action");
    boot.runtime.csbwin_global_variables[1] = 0u;
    session.source_tick++;
    check(M11_GameView_ExecuteCSBDSARestoredTimer(&view, &location, 0u) == 0 &&
              boot.runtime.csbwin_global_variables[1] == 0u,
          "M11 rejects stale session identity before the CSBWin timer runner");
    session.source_tick = handoff.startup_source_tick;
    check(M11_GameView_ExecuteCSBDSARestoredTimer(&view, &location, 1u) == 0 &&
              boot.runtime.csbwin_global_variables[1] == 0u,
          "M11 rejects an undeclared TimerQueue slot without a fallback action");
    session.source_tick++;
    check(!csb_v1_csbwin_dsa_execute_restored_timer_pc34(
              &boot, &handoff, &session, &location, 0u, &bridge) &&
              boot.runtime.csbwin_global_variables[1] == 0u,
          "session drift invalidates the restored timer bridge before execution");
    session.source_tick = handoff.startup_source_tick;
    boot.runtime.game_time++;
    check(!csb_v1_csbwin_dsa_execute_restored_timer_pc34(
              &boot, &handoff, &session, &location, 0u, &bridge),
          "tick drift rejects an out-of-order restored timer");
    boot.runtime.game_time = handoff.runtime_game_time;
    memset(&binding, 0, sizeof(binding));
    binding.dsa_selector = 2u;
    binding.dsa_id = 7u;
    binding.actuator_identity_valid = 1;
    binding.location.level = 1;
    check(csb_v1_csbwin_dsa_bind_restored_movement_filter_pc34(
              &boot, &handoff, &session, &binding, 1u, 0, 0u, 1,
              &filter, &adapter, &bridge) && filter.movement_filter_dsa_id[1] == 7,
          "admitted movement filter binds its declared level-one route");
    binding.location.level = 2;
    check(csb_v1_csbwin_dsa_bind_restored_movement_filter_pc34(
              &boot, &handoff, &session, &binding, 1u, 0, 0u, 2,
              &filter, &adapter, &bridge) && filter.movement_filter_dsa_id[2] == 7,
          "admitted movement filter keeps the second source level distinct");
    actions[0].program_words = dynamic_jump_gear;
    actions[0].program_word_count = (int)(sizeof(dynamic_jump_gear) /
                                          sizeof(dynamic_jump_gear[0]));
    check(csb_v1_csbwin_dsa_execute_restored_timer_pc34(
              &boot, &handoff, &session, &location, 0u, &bridge) &&
              bridge.dynamic_transfer_count == 1u &&
              bridge.dynamic_transfer_state == 9u &&
              bridge.dynamic_transfer_column == 0u &&
              !bridge.dynamic_transfer_gosub &&
              csb_v1_csbwin_dsa_restored_timer_receipt_current_pc34(
                  &boot, &handoff, &session, &bridge),
          "restored timer binds JumpGear's exact imported target to save identity");
    direct_jump[0] = 0x0006u;
    check(!csb_v1_csbwin_dsa_restored_timer_receipt_current_pc34(
              &boot, &handoff, &session, &bridge),
          "dynamic transfer target drift invalidates the restored save receipt");
    direct_jump[0] = 0x814cu;
    actions[0].program_words = store_global;
    actions[0].program_word_count = (int)(sizeof(store_global) /
                                          sizeof(store_global[0]));
    store_global[0] = 0xffffu;
    check(!csb_v1_csbwin_dsa_execute_restored_timer_pc34(
              &boot, &handoff, &session, &location, 0u, &bridge) &&
              boot.runtime.csbwin_global_variables[1] == 0u,
          "unknown opcode body cannot cross the restored-timer admission bridge");
    raw[80] = 0x60u; /* CSBWin roomPIT with false-pit flag clear. */
    actions[0].program_words = false_pit;
    actions[0].program_word_count = (int)(sizeof(false_pit) / sizeof(false_pit[0]));
    check(csb_v1_csbwin_dsa_execute_restored_timer_pc34(
              &boot, &handoff, &session, &location, 0u, &bridge) &&
              bridge.dungeon_mutation_core && bridge.runtime_dungeon_changed &&
              bridge.cell_store_count == 1u && bridge.false_pit_count == 1u &&
              bridge.last_false_pit_location == 0u && raw[80] == 0x61u &&
              csb_v1_csbwin_dsa_restored_timer_receipt_current_pc34(
                  &boot, &handoff, &session, &bridge),
          "restored timer binds CSBWin FalsePit CELLFLAG mutation to loaded dungeon");
    raw[80] ^= 0x04u;
    check(!csb_v1_csbwin_dsa_restored_timer_receipt_current_pc34(
              &boot, &handoff, &session, &bridge),
          "post-mutation DUNGEON.DAT drift invalidates the restored save receipt");
    return failures ? 1 : 0;
}
