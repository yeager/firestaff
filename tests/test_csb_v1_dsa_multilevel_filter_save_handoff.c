/*
 * CSBWin multi-level movement-filter runtime/save handoff regression.
 * Source: CSBWin Monster.cpp:3079-3176, 3222-3370; DSA.cpp:1244-1312;
 * SaveGame.cpp global-variable EXPOOL records. Local EXPOOL fixture only.
 */

#include "csb_v1_monster_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures;

static void check(int condition, const char *message)
{
    if (condition) printf("PASS: %s\n", message);
    else { fprintf(stderr, "FAIL: %s\n", message); ++g_failures; }
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
    uint16_t store_first[] = { 0x0686u, 0x55aau, 0x0054u };
    uint16_t store_second[] = { 0x0686u, 0x66bbu, 0x0094u };
    CSB_V1_DSAImportedAction actions[2];
    CSB_V1_RuntimeDSAMovementFilterRequest requests[2];
    CSB_V1_RuntimeProfile profile;
    CSB_V1_RuntimeDSAMovementFilterStackAdapter adapter;
    CSB_V1_DSAFilterRuntime filter;
    uint8_t tail[CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES];
    const uint8_t *payload = NULL;
    size_t payload_size = 0u;
    const uint32_t record_id = (5u << 24) | (4u << 16);
    const uint32_t bucket = 32u + ((record_id * 0xbb40e62du) >> 27);
    int flags[2] = { 1, 2 };

    memset(actions, 0, sizeof(actions));
    memset(requests, 0, sizeof(requests));
    memset(&profile, 0, sizeof(profile));
    memset(&adapter, 0, sizeof(adapter));
    memset(&filter, 0, sizeof(filter));
    memset(tail, 0, sizeof(tail));
    csb_v1_chaos_init(&profile.csbwin_extended_dsa_state);
    actions[0].dsa_id = 9u;
    actions[0].state_index = 4u;
    actions[0].program_words = store_first;
    actions[0].program_word_count = 3;
    actions[1].dsa_id = 10u;
    actions[1].state_index = 2u;
    actions[1].program_words = store_second;
    actions[1].program_word_count = 3;
    profile.csbwin_extended_features_valid = 1;
    profile.csbwin_extended_dsa_state.imported_actions = actions;
    profile.csbwin_extended_dsa_state.imported_action_count = 2;
    profile.csbwin_global_variables_valid = 1;
    profile.csbwin_global_variable_count = 16u;
    put_le16(tail, 2u, 18u);
    put_le32(tail, (size_t)bucket * 4u, 1u);
    put_le32(tail, 2u * 4u, record_id);
    profile.csbwin_appended_tail_valid = 1;
    profile.csbwin_appended_tail_size = sizeof(tail);
    profile.csbwin_appended_tail_preserved_size = sizeof(tail);
    memcpy(profile.csbwin_appended_tail, tail, sizeof(tail));

    requests[0].binding.dsa_id = 9u;
    requests[0].binding.location.level = 2;
    requests[0].state_index = 4u;
    requests[0].action_ordinal = 0;
    requests[0].master_location = 0x10001u;
    requests[1].binding.dsa_id = 10u;
    requests[1].binding.location.level = 7;
    requests[1].state_index = 2u;
    requests[1].action_ordinal = 0;
    requests[1].master_location = 0x20002u;

    check(csb_v1_runtime_bind_csbwin_movement_filter_stack_runtime_multi(
              &profile, requests, 2u, 6, &filter, &adapter) == 1 &&
              adapter.runner_count == 2 && filter.loaded_level == 6 &&
              filter.movement_filter_dsa_id[2] == 9 &&
              filter.movement_filter_dsa_id[7] == 10 &&
              filter.movement_filter_dsa_id[0] == -1,
          "two resolved source levels install as one authenticated callback");

    check(csb_v1_dsa_filter_movement_preprocess_live(
              2, 1, 2, 3, 2, 4, 5, flags, &filter) == 1 &&
              csb_v1_dsa_filter_movement_preprocess_live(
                  7, 6, 7, 8, 7, 9, 10, flags, &filter) == 1 &&
              filter.loaded_level == 6 &&
              adapter.runners[0].execution_count == 1 &&
              adapter.runners[1].execution_count == 1 &&
              profile.csbwin_global_variables[1] == 0x55aau &&
              profile.csbwin_global_variables[2] == 0x66bbu &&
              csb_v1_runtime_locate_csbwin_appended_expool_record(
                  &profile, record_id, &payload, &payload_size) == 1 &&
              payload_size == 64u && payload[4] == 0xaau &&
              payload[5] == 0x55u && payload[8] == 0xbbu &&
              payload[9] == 0x66u,
          "each level dispatches its authenticated action and original EXPOOL word");

    requests[1].binding.location.level = 2;
    check(csb_v1_runtime_bind_csbwin_movement_filter_stack_runtime_multi(
              &profile, requests, 2u, 6, &filter, &adapter) == 0 &&
              adapter.runner_count == 2 && filter.movement_filter_dsa_id[2] == 9 &&
              filter.movement_filter_dsa_id[7] == 10,
          "duplicate source level rejects without replacing the live callback");

    profile.csbwin_extended_dsa_state.imported_actions = NULL;
    profile.csbwin_extended_dsa_state.imported_action_count = 0;
    csb_v1_chaos_cleanup(&profile.csbwin_extended_dsa_state);
    return g_failures == 0 ? 0 : 1;
}
