/* CSBWin DSA.cpp STKOP_Copy runtime transaction regression. */
#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

#define CSB_TEST_THING_TYPE_TEXTSTRING 2u

typedef struct {
    int prepare_count;
    int commit_count;
    int32_t selector;
    uint32_t value;
} IndirectCharacterStore;

static int indirect_prepare_character_store(void *user,
                                            int32_t selector,
                                            uint32_t values[59],
                                            uint32_t word_count)
{
    IndirectCharacterStore *store = user;
    if (!store || !values || word_count != 1u) return -1;
    ++store->prepare_count;
    store->selector = selector;
    store->value = values[0];
    return 1;
}

static int indirect_commit_character_store(void *user,
                                           int32_t selector,
                                           const uint32_t values[59],
                                           uint32_t word_count)
{
    IndirectCharacterStore *store = user;
    if (!store || !values || word_count != 1u) return 0;
    ++store->commit_count;
    store->selector = selector;
    store->value = values[0];
    return 1;
}

static void check(int condition, const char *message)
{
    if (condition) printf("PASS: %s\n", message);
    else {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static void configure_action(CSB_V1_DSAImportedAction *action,
                             uint16_t *words, int word_count)
{
    memset(action, 0, sizeof(*action));
    action->dsa_id = 19u;
    action->state_index = 2u;
    action->column = 0u;
    action->program_words = words;
    action->program_word_count = word_count;
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

static uint32_t read_le32(const uint8_t *bytes, size_t offset)
{
    return (uint32_t)bytes[offset] |
           ((uint32_t)bytes[offset + 1u] << 8) |
           ((uint32_t)bytes[offset + 2u] << 16) |
           ((uint32_t)bytes[offset + 3u] << 24);
}

static uint32_t fnv1a32(const uint8_t *bytes, size_t size)
{
    uint32_t hash = 2166136261u;
    size_t i;

    for (i = 0u; i < size; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

typedef struct {
    int calls;
    int32_t cloud_type;
    int32_t size;
    uint32_t location;
} CloudStore;

static int commit_cloud(void *user, int32_t cloud_type, int32_t size,
                        uint32_t location)
{
    CloudStore *store = user;
    if (!store) return 0;
    ++store->calls;
    store->cloud_type = cloud_type;
    store->size = size;
    store->location = location;
    return 1;
}

static void test_create_cloud_transaction(void)
{
    uint16_t cloud_words[] = {
        0x0686u, 0x0203u, 0x0686u, 7u, 0x0686u, 23u, 0x110bu
    };
    uint16_t invalid_type_words[] = {
        0x0686u, 0x0203u, 0x0686u, 99u, 0x0686u, 23u, 0x110bu
    };
    uint16_t rollback_words[] = {
        0x0686u, 0x0203u, 0x0686u, 7u, 0x0686u, 23u, 0x110bu, 0u
    };
    uint16_t indirect_words[] = { 0x168bu };
    uint32_t indirect_parameters[] = {
        /* EX_AMPERSAND pushes P5, P4, P3: direct CreateCloud therefore
         * receives its source pop order (size, type, location). */
        81u, 3u, 23u, 7u, 0x0203u, 0u, 0u
    };
    CSB_V1_ChaosMagicState state;
    CSB_V1_DSAImportedAction action;
    CSB_V1_CSBWinDSAStackContext context;
    CSB_V1_CSBWinDSAStackExecution execution;
    CloudStore store;

    memset(&state, 0, sizeof(state));
    memset(&context, 0, sizeof(context));
    memset(&store, 0, sizeof(store));
    configure_action(&action, cloud_words,
                     (int)(sizeof(cloud_words) / sizeof(cloud_words[0])));
    state.imported_actions = &action;
    state.imported_action_count = 1;
    context.create_cloud = commit_cloud;
    context.dungeon_user = &store;
    check(csb_v1_csbwin_dsa_execute_authenticated_stack_action(
              &state, action.dsa_id, action.state_index, 0, &context,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              store.calls == 1 && store.cloud_type == 7 && store.size == 23 &&
              store.location == 0x0203u && execution.create_cloud_count == 1u,
          "STKOP_CreateCloud commits the exact source request after acceptance");

    configure_action(&action, indirect_words,
                     (int)(sizeof(indirect_words) / sizeof(indirect_words[0])));
    context.parameters = indirect_parameters;
    context.parameter_count = (int)(sizeof(indirect_parameters) /
                                    sizeof(indirect_parameters[0]));
    store.calls = 0;
    check(csb_v1_csbwin_dsa_execute_authenticated_stack_action(
              &state, action.dsa_id, action.state_index, 0, &context,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              store.calls == 1 && store.cloud_type == 7 && store.size == 23 &&
              store.location == 0x0203u && execution.create_cloud_count == 1u,
          "STKOP_I_Indirect expands I_CreateCloud through the same transaction");

    context.parameters = NULL;
    context.parameter_count = 0;

    configure_action(&action, invalid_type_words,
                     (int)(sizeof(invalid_type_words) / sizeof(invalid_type_words[0])));
    store.calls = 0;
    check(csb_v1_csbwin_dsa_execute_authenticated_stack_action(
              &state, action.dsa_id, action.state_index, 0, &context,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK && store.calls == 0,
          "STKOP_CreateCloud preserves CSBWin's silent illegal-type no-op");

    configure_action(&action, rollback_words,
                     (int)(sizeof(rollback_words) / sizeof(rollback_words[0])));
    store.calls = 0;
    check(csb_v1_csbwin_dsa_execute_authenticated_stack_action(
              &state, action.dsa_id, action.state_index, 0, &context,
              &execution) != CSB_V1_CSBWIN_DSA_STACK_OK && store.calls == 0,
          "STKOP_CreateCloud rolls back when a later source word is rejected");
}

static void test_parameter_message_runtime_binding(void)
{
    /* STKOP_Message consumes (target, type, count, delay) and the live DSA
     * parameter stack.  This DB11 tail contains one genuine, source-sized
     * free node for the resulting two-parameter EXPOOL record. */
    uint8_t tail[2u * CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES] = { 0 };
    uint8_t empty_tail[2u * CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES] = { 0 };
    uint8_t tail_before[sizeof(tail)];
    uint16_t words[] = {
        0x0686u, 0x0088u, 0x0686u, 2u, 0x0686u, 2u,
        0x0686u, 5u, 0x0a8bu
    };
    uint16_t rejected_words[] = {
        0x0686u, 0x0088u, 0x0686u, 2u, 0x0686u, 2u,
        0x0686u, 5u, 0x0a8bu, 0x0000u
    };
    uint16_t zero_words[] = {
        0x0686u, 0x0088u, 0x0686u, 2u, 0x0686u, 0u,
        0x0686u, 5u, 0x0a8bu
    };
    CSB_V1_RuntimeProfile profile;
    CSB_V1_CSBWinDSAFilterStackRunnerContext runner;
    CSB_V1_DSAImportedAction action;
    CSB_V1_CSBWinDSARuntimeExecutionReceipt_PC34 receipt;
    int parameters[] = { 0x11223344, 0x55667788 };

    /* EXPOOL::enlarge layout: block 64 holds 4-word nodes, and the free-list
     * header for a four-word write points to its first node at 65. */
    put_le16(tail, 64u * 4u + 2u, 4u);
    put_le32(tail, 4u * 4u, 65u);

    csb_v1_runtime_init(&profile, NULL);
    profile.csbwin_extended_features_valid = 1;
    profile.csbwin_body_runtime_summary_valid = 1;
    profile.csbwin_timer_summary_count = 6u;
    profile.csbwin_timer_summary_total = 6u;
    profile.csbwin_max_timers = 6u;
    profile.csbwin_first_avail_timer = 0u;
    profile.csbwin_appended_tail_valid = 1;
    profile.csbwin_appended_tail_size = sizeof(tail);
    profile.csbwin_appended_tail_preserved_size = sizeof(tail);
    memcpy(profile.csbwin_appended_tail, tail, sizeof(tail));
    profile.csbwin_appended_tail_fnv1a = fnv1a32(tail, sizeof(tail));
    configure_action(&action, words,
                     (int)(sizeof(words) / sizeof(words[0])));
    profile.csbwin_extended_dsa_state.imported_actions = &action;
    profile.csbwin_extended_dsa_state.imported_action_count = 1;
    memset(&runner, 0, sizeof(runner));
    runner.programs = &profile.csbwin_extended_dsa_state;
    runner.dsa_id = action.dsa_id;
    runner.state_index = action.state_index;

    memset(&receipt, 0, sizeof(receipt));
    check(csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
              &profile, &runner, &action, parameters, 2, NULL) == 1 &&
              profile.csbwin_num_timer == 1u &&
              profile.csbwin_timer_queue_summary_count == 1u &&
              profile.csbwin_timer_queue[0] == 0u &&
              profile.csbwin_timers[0].function == 101u &&
              profile.csbwin_timers[0].time == 5u &&
              profile.csbwin_timers[0].ubyte5 == 0u &&
              profile.csbwin_timers[0].ubyte6 == 4u &&
              profile.csbwin_timers[0].ubyte7 == 8u &&
              profile.csbwin_timers[0].ubyte8 == 0u &&
              profile.csbwin_timers[0].ubyte9 == 2u &&
              profile.csbwin_parameter_message_sequence == 1u &&
              read_le32(profile.csbwin_appended_tail, 37u * 4u) == 65u &&
              read_le32(profile.csbwin_appended_tail, 66u * 4u) ==
                  0x01000000u &&
              read_le32(profile.csbwin_appended_tail, 67u * 4u) ==
                  0x11223344u &&
              read_le32(profile.csbwin_appended_tail, 68u * 4u) ==
                  0x55667788u &&
              csb_v1_runtime_get_last_csbwin_dsa_execution_receipt_pc34(
                  &profile, &receipt) == 1 &&
              receipt.parameter_message_created_count == 1u &&
              receipt.last_parameter_message_timer_index == 0u &&
              receipt.last_parameter_message_queue_slot == 0u &&
              receipt.last_parameter_message_tail_fnv1a ==
                  profile.csbwin_appended_tail_fnv1a,
          "STKOP_Message commits one CSBWin timer, EXPOOL record and receipt");

    /* EXPOOL::Write with NumParam == 0 remains a live two-word record, not
     * SetSkin-style deletion. This separate source-shaped free list has only
     * two-word nodes, so accepting it with a synthetic payload would fail. */
    put_le16(empty_tail, 64u * 4u + 2u, 2u);
    put_le32(empty_tail, 2u * 4u, 65u);
    csb_v1_runtime_init(&profile, NULL);
    profile.csbwin_extended_features_valid = 1;
    profile.csbwin_body_runtime_summary_valid = 1;
    profile.csbwin_timer_summary_count = 6u;
    profile.csbwin_timer_summary_total = 6u;
    profile.csbwin_max_timers = 6u;
    profile.csbwin_appended_tail_valid = 1;
    profile.csbwin_appended_tail_size = sizeof(empty_tail);
    profile.csbwin_appended_tail_preserved_size = sizeof(empty_tail);
    memcpy(profile.csbwin_appended_tail, empty_tail, sizeof(empty_tail));
    profile.csbwin_appended_tail_fnv1a = fnv1a32(empty_tail,
                                                  sizeof(empty_tail));
    configure_action(&action, zero_words,
                     (int)(sizeof(zero_words) / sizeof(zero_words[0])));
    profile.csbwin_extended_dsa_state.imported_actions = &action;
    profile.csbwin_extended_dsa_state.imported_action_count = 1;
    memset(&runner, 0, sizeof(runner));
    runner.programs = &profile.csbwin_extended_dsa_state;
    runner.dsa_id = action.dsa_id;
    runner.state_index = action.state_index;
    memset(&receipt, 0, sizeof(receipt));
    check(csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
              &profile, &runner, &action, NULL, 0, NULL) == 1 &&
              profile.csbwin_num_timer == 1u &&
              profile.csbwin_timers[0].function == 101u &&
              read_le32(profile.csbwin_appended_tail, 37u * 4u) == 65u &&
              read_le32(profile.csbwin_appended_tail, 66u * 4u) ==
                  0x01000000u &&
              csb_v1_runtime_get_last_csbwin_dsa_execution_receipt_pc34(
                  &profile, &receipt) == 1 &&
              receipt.parameter_message_created_count == 1u,
          "zero-parameter STKOP_Message retains its source-sized EXPOOL record");

    /* Restore the four-word free node and exercise rollback after MESSAGE
     * itself has staged its timer and DB11 record. */
    csb_v1_runtime_init(&profile, NULL);
    profile.csbwin_extended_features_valid = 1;
    profile.csbwin_body_runtime_summary_valid = 1;
    profile.csbwin_timer_summary_count = 6u;
    profile.csbwin_timer_summary_total = 6u;
    profile.csbwin_max_timers = 6u;
    profile.csbwin_appended_tail_valid = 1;
    profile.csbwin_appended_tail_size = sizeof(tail);
    profile.csbwin_appended_tail_preserved_size = sizeof(tail);
    memcpy(profile.csbwin_appended_tail, tail, sizeof(tail));
    profile.csbwin_appended_tail_fnv1a = fnv1a32(tail, sizeof(tail));
    configure_action(&action, rejected_words,
                     (int)(sizeof(rejected_words) / sizeof(rejected_words[0])));
    profile.csbwin_extended_dsa_state.imported_actions = &action;
    profile.csbwin_extended_dsa_state.imported_action_count = 1;
    memset(&runner, 0, sizeof(runner));
    runner.programs = &profile.csbwin_extended_dsa_state;
    runner.dsa_id = action.dsa_id;
    runner.state_index = action.state_index;
    memcpy(tail_before, profile.csbwin_appended_tail, sizeof(tail_before));
    check(csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
              &profile, &runner, &action, parameters, 2, NULL) == 0 &&
              profile.csbwin_num_timer == 0u &&
              memcmp(profile.csbwin_appended_tail, tail_before,
                     sizeof(tail_before)) == 0,
          "later unsupported DSA word rolls back the timer and EXPOOL write");
}

static void test_textfetch_textsay_runtime_binding(void)
{
    /* One source-shaped DB2 and its F0507 text words.  TEXT@ is deliberately
     * independent of DB2 visibility in CSBWin DSA.cpp. */
    uint8_t raw[128] = { 0 };
    uint16_t text_words[] = {
        0x0686u, (uint16_t)(CSB_TEST_THING_TYPE_TEXTSTRING << 10),
        0x0686u, 0u, 0x19cbu,
        0x0686u, 0u, 0x0686u, 4u, 0x1a0bu
    };
    uint16_t text_then_unknown_words[] = {
        0x0686u, (uint16_t)(CSB_TEST_THING_TYPE_TEXTSTRING << 10),
        0x0686u, 0u, 0x19cbu,
        0x0686u, 0u, 0x0686u, 4u, 0x1a0bu, 0x0000u
    };
    CSB_V1_DungeonData dungeon;
    CSB_V1_RuntimeProfile profile;
    CSB_V1_CSBWinDSAFilterStackRunnerContext runner;
    CSB_V1_DSAImportedAction action;
    CSB_V1_CSBWinDSARuntimeExecutionReceipt_PC34 receipt;

    memset(&dungeon, 0, sizeof(dungeon));
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    dungeon.text_data_base = 104;
    dungeon.text_word_count = 2;
    dungeon.thing_data_bases[CSB_TEST_THING_TYPE_TEXTSTRING] = 100;
    dungeon.thing_type_counts[CSB_TEST_THING_TYPE_TEXTSTRING] = 1;
    put_le16(raw, 100u, 0xfffeu);
    put_le16(raw, 102u, 0u);
    /* H,E,L and the ReDMCSB F0507 end marker. */
    put_le16(raw, 104u, (uint16_t)((7u << 10) | (4u << 5) | 11u));
    put_le16(raw, 106u, (uint16_t)(31u << 10));

    csb_v1_runtime_init(&profile, NULL);
    profile.dungeon_handle = &dungeon;
    profile.csbwin_extended_features_valid = 1;
    configure_action(&action, text_words,
                     (int)(sizeof(text_words) / sizeof(text_words[0])));
    profile.csbwin_extended_dsa_state.imported_actions = &action;
    profile.csbwin_extended_dsa_state.imported_action_count = 1;
    memset(&runner, 0, sizeof(runner));
    runner.programs = &profile.csbwin_extended_dsa_state;
    runner.dsa_id = action.dsa_id;
    runner.state_index = action.state_index;

    memset(&receipt, 0, sizeof(receipt));
    check(csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
              &profile, &runner, &action, NULL, 0, NULL) == 1 &&
              profile.csbwin_text_message_receipt.valid &&
              strcmp(profile.csbwin_text_message_receipt.text, "HEL") == 0 &&
              csb_v1_runtime_get_last_csbwin_dsa_execution_receipt_pc34(
                  &profile, &receipt) == 1 && receipt.text_message_changed &&
              receipt.text_message_after.valid &&
              strcmp(receipt.text_message_after.text, "HEL") == 0,
          "TEXT@ and TEXTSAY publish a real DB2/F0507 text receipt");

    memset(&profile.csbwin_text_message_receipt, 0,
           sizeof(profile.csbwin_text_message_receipt));
    configure_action(&action, text_then_unknown_words,
                     (int)(sizeof(text_then_unknown_words) /
                           sizeof(text_then_unknown_words[0])));
    check(csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
              &profile, &runner, &action, NULL, 0, NULL) == 0 &&
              !profile.csbwin_text_message_receipt.valid &&
              profile.csbwin_text_message_receipt.text[0] == '\0',
          "failed later DSA text opcode cannot publish a DB2 text receipt");
}

static void test_copyteleporter_runtime_receipt(void)
{
    uint8_t raw[96] = { 0 };
    uint16_t words[] = { 0x0284u, 0u, 0x0020u };
    CSB_V1_DungeonData dungeon;
    CSB_V1_RuntimeProfile profile;
    CSB_V1_CSBWinDSAFilterStackRunnerContext runner;
    CSB_V1_DSAImportedAction action;
    CSB_V1_CSBWinDSARuntimeExecutionReceipt_PC34 receipt;
    CSB_V1_CSBWinDSACoreProgramReceipt core;

    memset(&dungeon, 0, sizeof(dungeon));
    /* Two real byte-map teleporter squares: column starts at 60, map at 64,
     * first-Thing table at 66, and two DB1 records at 70/76. */
    raw[60] = 0u; raw[61] = 0u;
    raw[62] = 1u; raw[63] = 0u;
    raw[64] = 0xbcu;
    raw[65] = 0xb0u;
    raw[66] = 0u; raw[67] = 4u;
    raw[68] = 1u; raw[69] = 4u;
    raw[70] = 0xffu; raw[71] = 0xffu;
    raw[76] = 0xffu; raw[77] = 0xffu;
    raw[72] = 0x15u; raw[73] = 0x6cu;
    raw[74] = 0x00u; raw[75] = 0x35u;
    raw[78] = 0x00u; raw[79] = 0x00u;
    raw[80] = 0x00u; raw[81] = 0x00u;
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 2;
    dungeon.level_heights[0] = 1;
    dungeon.level_offsets[0] = 64;
    dungeon.square_bytes = 1;
    dungeon.square_first_thing_base = 66;
    dungeon.square_first_thing_count = 2;
    dungeon.thing_data_bases[1] = 70;
    dungeon.thing_type_counts[1] = 2;

    csb_v1_runtime_init(&profile, NULL);
    profile.dungeon_handle = &dungeon;
    profile.csbwin_extended_features_valid = 1;
    configure_action(&action, words,
                     (int)(sizeof(words) / sizeof(words[0])));
    profile.csbwin_extended_dsa_state.imported_actions = &action;
    profile.csbwin_extended_dsa_state.imported_action_count = 1;
    memset(&runner, 0, sizeof(runner));
    runner.programs = &profile.csbwin_extended_dsa_state;
    runner.dsa_id = action.dsa_id;
    runner.state_index = action.state_index;

    memset(&receipt, 0, sizeof(receipt));
    memset(&core, 0, sizeof(core));
    check(csb_v1_csbwin_dsa_verify_authenticated_core_program(
              &profile.csbwin_extended_dsa_state, action.dsa_id,
              action.state_index, 0, &core) == CSB_V1_CSBWIN_DSA_CORE_OK &&
              core.valid && core.requires_runtime_owner &&
              csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
              &profile, &runner, &action, NULL, 0, NULL) == 1 &&
              raw[65] == raw[64] && memcmp(raw + 78, raw + 72, 4u) == 0 &&
              csb_v1_runtime_get_last_csbwin_dsa_execution_receipt_pc34(
                  &profile, &receipt) == 1 &&
              receipt.teleporter_copy_count == 1u &&
              receipt.last_teleporter_copy_source_location == 0u &&
              receipt.last_teleporter_copy_destination_location == 0x0020u &&
              memcmp(receipt.last_teleporter_copy_source_before,
                     receipt.last_teleporter_copy_destination_after,
                     sizeof(receipt.last_teleporter_copy_source_before)) == 0,
          "COPYTELEPORTER publishes source-owned DB1/CELLFLAG pre/post receipt");
    raw[78] ^= 1u;
    check(csb_v1_runtime_get_last_csbwin_dsa_execution_receipt_pc34(
              &profile, &receipt) == 0,
          "COPYTELEPORTER receipt rejects destination DB1 drift");
}

static void test_teleport_party_runtime_receipt(void)
{
    uint8_t raw[72] = { 0 };
    uint16_t words[] = { 0x0686u, 0x0020u, 0x0f8bu };
    CSB_V1_DungeonData dungeon;
    CSB_V1_RuntimeProfile profile;
    CSB_V1_CSBWinDSAFilterStackRunnerContext runner;
    CSB_V1_DSAImportedAction action;
    CSB_V1_CSBWinDSARuntimeExecutionReceipt_PC34 receipt;
    CSB_V1_CSBWinDSACoreProgramReceipt core;

    memset(&dungeon, 0, sizeof(dungeon));
    /* One loaded two-cell level. LOCATIONREL 0x0020 is (p=0,l=0,x=1,y=0). */
    raw[60] = 0u; raw[61] = 0u;
    raw[62] = 1u; raw[63] = 0u;
    raw[64] = 0x20u;
    raw[65] = 0x20u;
    raw[66] = 0xfeu; raw[67] = 0xffu;
    raw[68] = 0xfeu; raw[69] = 0xffu;
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 2;
    dungeon.level_heights[0] = 1;
    dungeon.level_offsets[0] = 64;
    dungeon.square_bytes = 1;
    dungeon.square_first_thing_base = 66;
    dungeon.square_first_thing_count = 2;

    csb_v1_runtime_init(&profile, NULL);
    profile.dungeon_handle = &dungeon;
    profile.csbwin_extended_features_valid = 1;
    profile.party_state_valid = 1;
    configure_action(&action, words,
                     (int)(sizeof(words) / sizeof(words[0])));
    profile.csbwin_extended_dsa_state.imported_actions = &action;
    profile.csbwin_extended_dsa_state.imported_action_count = 1;
    memset(&runner, 0, sizeof(runner));
    runner.programs = &profile.csbwin_extended_dsa_state;
    runner.dsa_id = action.dsa_id;
    runner.state_index = action.state_index;

    memset(&receipt, 0, sizeof(receipt));
    memset(&core, 0, sizeof(core));
    check(csb_v1_csbwin_dsa_verify_authenticated_core_program(
              &profile.csbwin_extended_dsa_state, action.dsa_id,
              action.state_index, 0, &core) == CSB_V1_CSBWIN_DSA_CORE_OK &&
              core.valid && core.requires_runtime_owner &&
              csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
              &profile, &runner, &action, NULL, 0, NULL) == 1 &&
              profile.current_level == 0 && profile.party_z == 0 &&
              profile.party_x == 1 && profile.party_y == 0 &&
              profile.party_dir == 0 && profile.party_state.PartyMapX == 1 &&
              profile.party_state.PartyMapY == 0 &&
              csb_v1_runtime_get_last_csbwin_dsa_execution_receipt_pc34(
                  &profile, &receipt) == 1 &&
              receipt.teleport_party_count == 1u &&
              receipt.last_teleport_party_destination == 0x0020u,
          "TELEPORTPARTY requires runtime ownership and atomically publishes LOCATIONREL");
    profile.party_x = 0;
    check(csb_v1_runtime_get_last_csbwin_dsa_execution_receipt_pc34(
              &profile, &receipt) == 0,
          "TELEPORTPARTY receipt rejects post-publication party-pose drift");
}

static void test_indirect_local_variable_char_store(void)
{
    /* CSBWin DSA.cpp EX_AMPERSAND I_Indirect format. P3=count, P4=DSAVARS
     * index and P5=character selector become the direct CHAR! stack words;
     * P7 writes the selected action-local DSAVARS entry before CHAR! runs. */
    uint16_t words[] = { 0x168bu };
    uint32_t parameters[] = {
        85u, 3u, 1u, 0u, 1u, 1u, 0x12345678u, 0u
    };
    CSB_V1_DSAImportedAction action;
    CSB_V1_ChaosMagicState state;
    CSB_V1_CSBWinDSAStackContext context;
    CSB_V1_CSBWinDSAStackExecution execution;
    IndirectCharacterStore store;

    memset(&action, 0, sizeof(action));
    csb_v1_chaos_init(&state);
    memset(&context, 0, sizeof(context));
    memset(&store, 0, sizeof(store));
    action.dsa_id = 23u;
    action.state_index = 1u;
    action.program_words = words;
    action.program_word_count = (int)(sizeof(words) / sizeof(words[0]));
    state.imported_actions = &action;
    state.imported_action_count = 1;
    context.parameters = parameters;
    context.parameter_count = (int)(sizeof(parameters) / sizeof(parameters[0]));
    context.dungeon_user = &store;
    context.prepare_character_store = indirect_prepare_character_store;
    context.set_character_info = indirect_commit_character_store;

    check(csb_v1_csbwin_dsa_execute_authenticated_stack_action(
              &state, 23, 1u, 0, &context, &execution) ==
              CSB_V1_CSBWIN_DSA_STACK_OK &&
              store.prepare_count == 1 && store.commit_count == 1 &&
              store.selector == 1 && store.value == 0x12345678u &&
              execution.stack_depth == 0u,
          "STKOP_I_Indirect applies action-local DSAVARS before CHAR!");
}

static void test_monster_group_timer_and_item16_runtime_binding(void)
{
    uint8_t raw[144] = { 0 };
    const uint16_t sensor = (uint16_t)(THING_TYPE_SENSOR << 10);
    const uint16_t group = (uint16_t)(THING_TYPE_GROUP << 10);
    uint16_t delete_words[] = {
        0x0686u, 0u, 0x0686u, 0u, 0x17cbu
    };
    uint16_t insert_words[] = {
        0x0686u, 0u, 0x0686u, 0x0fu, 0x180bu
    };
    CSB_V1_DungeonData dungeon;
    CSB_V1_RuntimeProfile profile;
    CSB_V1_CSBWinDSAFilterStackRunnerContext runner;
    CSB_V1_DSAImportedAction action;
    CSB_V1_CSBWinDSARuntimeExecutionReceipt_PC34 receipt;

    memset(&dungeon, 0, sizeof(dungeon));
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 1;
    dungeon.level_heights[0] = 1;
    dungeon.level_offsets[0] = 80;
    dungeon.square_bytes = 1;
    dungeon.square_first_thing_base = 90;
    dungeon.square_first_thing_count = 1;
    dungeon.thing_data_bases[THING_TYPE_SENSOR] = 100;
    dungeon.thing_type_counts[THING_TYPE_SENSOR] = 1;
    dungeon.thing_data_bases[THING_TYPE_GROUP] = 108;
    dungeon.thing_type_counts[THING_TYPE_GROUP] = 1;
    raw[80] = 0x10u;
    put_le16(raw, 60u, 0u);
    put_le16(raw, 90u, sensor);
    put_le16(raw, 100u, group);
    put_le16(raw, 108u, THING_ENDOFLIST);
    raw[112] = 3u;
    raw[113] = 0x08u;
    put_le16(raw, 114u, 0x0111u);
    put_le16(raw, 116u, 0x0222u);
    put_le16(raw, 122u, 0x0026u); /* Two creatures and CSBWin fear state. */

    csb_v1_runtime_init(&profile, NULL);
    profile.dungeon_handle = &dungeon;
    profile.csbwin_extended_features_valid = 1;
    profile.csbwin_body_runtime_summary_valid = 1;
    profile.csbwin_item16_summary_count = 1u;
    profile.csbwin_item16_summary_total = 1u;
    profile.csbwin_item16[0].valid = 1;
    profile.csbwin_item16[0].monster_index = group;
    profile.csbwin_item16[0].current_x = 0u;
    profile.csbwin_item16[0].current_y = 0u;
    profile.csbwin_item16[0].single_monster_status[0] = 0x41u;
    profile.csbwin_item16[0].single_monster_status[1] = 0x72u;
    check(csb_v1_runtime_materialize_csbwin_item16_summaries(&profile) == 1,
          "DELMON runtime fixture materializes its CSBWin ITEM16 owner");
    profile.csbwin_timer_summary_count = 4u;
    profile.csbwin_timer_summary_total = 4u;
    profile.csbwin_max_timers = 4u;
    profile.csbwin_num_timer = 2u;
    profile.csbwin_timer_queue_summary_count = 2u;
    profile.csbwin_timer_queue_summary_total = 2u;
    profile.csbwin_timer_queue[0] = 0u;
    profile.csbwin_timer_queue[1] = 1u;
    profile.csbwin_first_avail_timer = 2u;
    profile.csbwin_timers[0].valid = 1;
    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].time = 10u;
    profile.csbwin_timers[0].function = 33u;
    profile.csbwin_timers[0].level = 0u;
    profile.csbwin_timers[1].valid = 1;
    profile.csbwin_timers[1].source_index = 1u;
    profile.csbwin_timers[1].time = 11u;
    profile.csbwin_timers[1].function = 34u;
    profile.csbwin_timers[1].level = 0u;
    profile.csbwin_timers[2].function = DM1_EVENT_NONE;
    profile.csbwin_timers[3].function = DM1_EVENT_NONE;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 2,
          "DELMON runtime fixture materializes CSBWin A0/A1 timers");
    configure_action(&action, delete_words,
                     (int)(sizeof(delete_words) / sizeof(delete_words[0])));
    profile.csbwin_extended_dsa_state.imported_actions = &action;
    profile.csbwin_extended_dsa_state.imported_action_count = 1;
    memset(&runner, 0, sizeof(runner));
    runner.programs = &profile.csbwin_extended_dsa_state;
    runner.dsa_id = action.dsa_id;
    runner.state_index = action.state_index;
    check(csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
              &profile, &runner, &action, NULL, 0, NULL) == 1 &&
              ((raw[122] >> 5) & 3u) == 0u &&
              profile.csbwin_num_timer == 1u &&
              profile.timeline_queue.eventCount == 1 &&
              profile.csbwin_timers[1].function == 33u &&
              profile.csbwin_item16[0].single_monster_status[0] == 0x72u &&
              profile.csbwin_runtime_item16[0].single_monster_status[0] ==
                  0x72u &&
              csb_v1_runtime_get_last_csbwin_dsa_execution_receipt_pc34(
                  &profile, &receipt) == 1 &&
              receipt.monster_group_mutation_count == 1u &&
              receipt.last_monster_group_location == 0u &&
              !receipt.last_monster_group_insert &&
              receipt.last_monster_group_thing == group &&
              receipt.last_monster_group_record_fnv1a != 0u,
          "DELMON compacts C04, A/B timer ownership, and ITEM16 together");

    configure_action(&action, insert_words,
                     (int)(sizeof(insert_words) / sizeof(insert_words[0])));
    check(csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
              &profile, &runner, &action, NULL, 0, NULL) == 1 &&
              ((raw[122] >> 5) & 3u) == 1u &&
              profile.csbwin_num_timer == 2u &&
              profile.timeline_queue.eventCount == 2 &&
              profile.csbwin_item16[0].single_monster_status[1] == 0x72u &&
              profile.csbwin_runtime_item16[0].single_monster_status[1] ==
                  0x72u &&
              csb_v1_runtime_get_last_csbwin_dsa_execution_receipt_pc34(
                  &profile, &receipt) == 1 &&
              receipt.monster_group_mutation_count == 1u &&
              receipt.last_monster_group_insert &&
              receipt.last_monster_group_thing == group &&
              receipt.last_monster_group_record_fnv1a != 0u,
          "INSMON clones source A0/B0 and ITEM16 state for the appended creature");
}

int main(void)
{
    const uint16_t source = (uint16_t)(CSB_V1_THING_TYPE_ACTUATOR << 10);
    const uint16_t destination = (uint16_t)(source | 1u);
    uint16_t copy_words[] = {
        0x0686u, source, 0x0686u, destination, 0x130bu
    };
    uint16_t copy_then_unknown_words[] = {
        0x0686u, source, 0x0686u, destination, 0x130bu, 0x0000u
    };
    uint16_t indirect_copy_words[] = { 0x168bu };
    uint32_t indirect_copy_parameters[] = {
        87u, 2u, destination, source, 0u, 0u
    };
    uint16_t modify_message_words[] = {
        0x0686u, 2u, 0x0686u, 9u, 0x0686u, 7u, 0x0295u
    };
    uint8_t raw[16] = {
        0x00u, 0x00u, 'A', 'B', 'C', 'D', 'E', 'F',
        0x00u, 0x00u, 'g', 'h', 'i', 'j', 'k', 'l'
    };
    CSB_V1_DungeonData dungeon;
    CSB_V1_RuntimeProfile profile;
    CSB_V1_CSBWinDSAFilterStackRunnerContext runner;
    CSB_V1_DSAImportedAction action;
    CSB_V1_CSBWinDSARuntimeExecutionReceipt_PC34 receipt;

    test_copyteleporter_runtime_receipt();
    test_teleport_party_runtime_receipt();
    test_textfetch_textsay_runtime_binding();
    test_parameter_message_runtime_binding();
    test_indirect_local_variable_char_store();
    test_create_cloud_transaction();
    test_monster_group_timer_and_item16_runtime_binding();

    memset(&dungeon, 0, sizeof(dungeon));
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    dungeon.thing_data_bases[CSB_V1_THING_TYPE_ACTUATOR] = 0;
    dungeon.thing_type_counts[CSB_V1_THING_TYPE_ACTUATOR] = 2;
    csb_v1_runtime_init(&profile, NULL);
    profile.dungeon_handle = &dungeon;
    profile.csbwin_extended_features_valid = 1;
    configure_action(&action, copy_words,
                     (int)(sizeof(copy_words) / sizeof(copy_words[0])));
    profile.csbwin_extended_dsa_state.imported_actions = &action;
    profile.csbwin_extended_dsa_state.imported_action_count = 1;
    memset(&runner, 0, sizeof(runner));
    runner.programs = &profile.csbwin_extended_dsa_state;
    runner.dsa_id = 19;
    runner.state_index = 2u;

    check(csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
              &profile, &runner, &action, NULL, 0, NULL) == 1 &&
              memcmp(raw + 2, raw + 10, 6u) == 0 &&
              runner.last_execution.actuator_copy_count == 1u &&
              runner.last_execution.last_actuator_copy_source_thing == source &&
              runner.last_execution.last_actuator_copy_destination_thing ==
                  destination,
          "STKOP_Copy commits a source-owned DB3 pair through the runtime candidate");

    memcpy(raw + 2, "ABCDEF", 6u);
    memcpy(raw + 10, "ghijkl", 6u);
    configure_action(&action, indirect_copy_words,
                     (int)(sizeof(indirect_copy_words) /
                           sizeof(indirect_copy_words[0])));
    check(csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
              &profile, &runner, &action, (int *)indirect_copy_parameters,
              (int)(sizeof(indirect_copy_parameters) /
                    sizeof(indirect_copy_parameters[0])), NULL) == 1 &&
              memcmp(raw + 2, raw + 10, 6u) == 0 &&
              runner.last_execution.actuator_copy_count == 1u,
          "STKOP_I_Indirect expands I_Copy through the same DB3 transaction");

    memcpy(raw + 2, "ABCDEF", 6u);
    memcpy(raw + 10, "ghijkl", 6u);
    configure_action(&action, copy_then_unknown_words,
                     (int)(sizeof(copy_then_unknown_words) /
                           sizeof(copy_then_unknown_words[0])));
    check(csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
              &profile, &runner, &action, NULL, 0, NULL) == 0 &&
              memcmp(raw + 2, "ABCDEF", 6u) == 0 &&
              memcmp(raw + 10, "ghijkl", 6u) == 0,
          "unknown DSA action leaves source and destination DB3 bytes unmodified");

    configure_action(&action, modify_message_words,
                     (int)(sizeof(modify_message_words) /
                           sizeof(modify_message_words[0])));
    runner.timer_type_modifiers_valid = 1;
    runner.timer_type_modifiers[0] = 0u;
    runner.timer_type_modifiers[1] = 1u;
    runner.timer_type_modifiers[2] = 2u;
    check(csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
              &profile, &runner, &action, NULL, 0, NULL) == 1 &&
              csb_v1_runtime_get_last_csbwin_dsa_execution_receipt_pc34(
                  &profile, &receipt) == 1 &&
              receipt.timer_type_modifiers_valid &&
              receipt.timer_type_modifiers[0] == 3u &&
              receipt.timer_type_modifiers[1] == 3u &&
              receipt.timer_type_modifiers[2] == 2u,
          "STKOP_ModifyMessage publishes only its authenticated timer-scope receipt");

    /* The dungeon is caller-owned stack storage; the process exits after this
     * focused receipt check, so do not hand it to the owning cleanup path. */
    return failures == 0 ? 0 : 1;
}
