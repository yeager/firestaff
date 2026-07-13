/*
 * CSBWin movement-filter runtime/save handoff regression.
 *
 * Source:
 *   CSBWin Monster.cpp:3079-3176 movement filter lookup per loaded level
 *   CSBWin Monster.cpp:3222-3370 ProcessDSAFilter seven-word movement ABI
 *   CSBWin DSA.cpp:1244-1312 GLOBALSTORE
 *   CSBWin SaveGame.cpp global-variable EXPOOL records
 *
 * The EXPOOL bytes below are a local format fixture, not a real-save corpus.
 */

#include "csb_v1_monster_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures;

static void check(int condition, const char *message)
{
    if (condition) {
        printf("PASS: %s\n", message);
    } else {
        fprintf(stderr, "FAIL: %s\n", message);
        ++g_failures;
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

int main(void)
{
    uint16_t ignored_dsa_zero[] = { 0x0686u, 0x1111u, 0x0054u };
    uint16_t store_global[] = { 0x0686u, 0x55aau, 0x0054u };
    CSB_V1_DSAImportedAction actions[2];
    CSB_V1_RuntimeProfile profile;
    CSB_V1_RuntimeDSAFilterBinding binding;
    CSB_V1_RuntimeDSAFilterStackAdapter adapter;
    CSB_V1_DSAFilterRuntime filter;
    uint8_t tail[CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES];
    const uint8_t *global_payload = NULL;
    size_t global_payload_size = 0u;
    const uint32_t global_record_id = (5u << 24) | (4u << 16);
    const uint32_t global_bucket = 32u +
        ((global_record_id * 0xbb40e62du) >> 27);
    int flags[2] = { 17, 23 };

    memset(actions, 0, sizeof(actions));
    memset(&profile, 0, sizeof(profile));
    memset(&binding, 0, sizeof(binding));
    memset(&adapter, 0, sizeof(adapter));
    memset(&filter, 0, sizeof(filter));
    memset(tail, 0, sizeof(tail));
    csb_v1_chaos_init(&profile.csbwin_extended_dsa_state);

    actions[0].dsa_id = 0u;
    actions[0].state_index = 1u;
    actions[0].program_words = ignored_dsa_zero;
    actions[0].program_word_count = (int)(sizeof(ignored_dsa_zero) /
                                          sizeof(ignored_dsa_zero[0]));
    actions[1].dsa_id = 9u;
    actions[1].state_index = 4u;
    actions[1].program_words = store_global;
    actions[1].program_word_count = (int)(sizeof(store_global) /
                                          sizeof(store_global[0]));
    profile.csbwin_extended_features_valid = 1;
    profile.csbwin_extended_dsa_state.imported_actions = actions;
    profile.csbwin_extended_dsa_state.imported_action_count = 2;
    profile.csbwin_global_variables_valid = 1;
    profile.csbwin_global_variable_count = 16u;
    put_le16(tail, 2u, 18u);
    put_le32(tail, (size_t)global_bucket * 4u, 1u);
    put_le32(tail, 1u * 4u, 0u);
    put_le32(tail, 2u * 4u, global_record_id);
    profile.csbwin_appended_tail_valid = 1;
    profile.csbwin_appended_tail_size = sizeof(tail);
    profile.csbwin_appended_tail_preserved_size = sizeof(tail);
    memcpy(profile.csbwin_appended_tail, tail, sizeof(tail));
    binding.dsa_id = 9u;
    binding.location.level = 3;

    check(csb_v1_runtime_bind_csbwin_movement_filter_stack_runtime(
              &profile, &binding, 4u, 0, 0x12345u, 6, &filter,
              &adapter) == 1 && filter.loaded_level == 6 &&
              filter.movement_filter_dsa_id[3] == 9 &&
              filter.movement_filter_state[3] == 4u &&
              filter.movement_filter_action[3] == 0 &&
              filter.movement_filter_dsa_id[0] == -1 &&
              filter.movement_filter_action[0] == -1 &&
              filter.attack_filter_dsa_id == -1 &&
              filter.attack_filter_action == -1,
          "resolved movement filter installs only its source level slot");

    check(csb_v1_dsa_filter_movement_preprocess_live(
              3, 11, 12, 0x1234, 3, 5, 6, flags, &filter) == 1 &&
              filter.loaded_level == 6 && adapter.runner.execution_count == 1 &&
              profile.csbwin_global_variables[1] == 0x55aau &&
              flags[0] == 17 && flags[1] == 23 &&
              csb_v1_runtime_locate_csbwin_appended_expool_record(
                  &profile, global_record_id, &global_payload,
                  &global_payload_size) == 1 && global_payload_size == 64u &&
              global_payload[4] == 0xaau && global_payload[5] == 0x55u,
          "seven-word movement callback restores level and publishes GLOBALSTORE");

    check(csb_v1_dsa_filter_movement_preprocess_live(
              0, 1, 2, 3, 0, 4, 5, flags, &filter) == 0 &&
              adapter.runner.execution_count == 1 &&
              profile.csbwin_global_variables[1] == 0x55aau,
          "unbound movement levels reject instead of dispatching DSA zero");

    profile.csbwin_extended_dsa_state.imported_actions = NULL;
    profile.csbwin_extended_dsa_state.imported_action_count = 0;
    csb_v1_chaos_cleanup(&profile.csbwin_extended_dsa_state);
    return g_failures == 0 ? 0 : 1;
}
