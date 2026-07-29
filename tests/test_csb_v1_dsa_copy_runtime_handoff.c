/* CSBWin DSA.cpp STKOP_Copy runtime transaction regression. */
#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

#define CSB_TEST_THING_TYPE_TEXTSTRING 2u

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

static void test_parameter_message_runtime_binding(void)
{
    /* STKOP_Message consumes (target, type, count, delay) and the live DSA
     * parameter stack.  This DB11 tail contains one genuine, source-sized
     * free node for the resulting two-parameter EXPOOL record. */
    uint8_t tail[2u * CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES] = { 0 };
    uint8_t tail_before[sizeof(tail)];
    uint16_t words[] = {
        0x0686u, 0x0088u, 0x0686u, 2u, 0x0686u, 2u,
        0x0686u, 5u, 0x0a8bu
    };
    uint16_t rejected_words[] = {
        0x0686u, 0x0088u, 0x0686u, 2u, 0x0686u, 2u,
        0x0686u, 5u, 0x0a8bu, 0x0000u
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

    memcpy(tail_before, profile.csbwin_appended_tail, sizeof(tail_before));
    profile.csbwin_num_timer = 0u;
    profile.csbwin_timer_queue_summary_count = 0u;
    profile.csbwin_timer_queue_summary_total = 0u;
    profile.csbwin_first_avail_timer = 1u;
    profile.csbwin_timers[0].function = DM1_EVENT_NONE;
    memset(&profile.timeline_queue, 0, sizeof(profile.timeline_queue));
    configure_action(&action, rejected_words,
                     (int)(sizeof(rejected_words) / sizeof(rejected_words[0])));
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
    check(csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
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
    test_textfetch_textsay_runtime_binding();
    test_parameter_message_runtime_binding();

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
