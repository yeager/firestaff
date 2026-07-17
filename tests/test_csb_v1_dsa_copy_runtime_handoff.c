/* CSBWin DSA.cpp STKOP_Copy runtime transaction regression. */
#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

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
