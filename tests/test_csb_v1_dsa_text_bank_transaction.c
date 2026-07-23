/* CSBWin DSA.cpp TEXT@ / GLOBALTEXT! / CHARNAME@@ transaction regression. */
#include "csb_v1_chaos_magic_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

typedef struct {
    int source_available;
    int global_write_count;
    uint32_t global_index;
    char global_text[1001];
} TextOwner;

static void check(int condition, const char *message)
{
    if (condition) printf("PASS: %s\n", message);
    else {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static int read_text(void *user, uint32_t object_index, char *out,
                     size_t out_capacity)
{
    TextOwner *owner = user;

    if (!owner || !out || out_capacity == 0u) return -1;
    if (!owner->source_available) return -1;
    if (object_index != 0x2au) return 0;
    snprintf(out, out_capacity, "SOURCE DB2 TEXT");
    return 1;
}

static int read_character_name(void *user, uint16_t fingerprint, char *out,
                               size_t out_capacity)
{
    (void)user;
    if (!out || out_capacity == 0u) return -1;
    if (fingerprint != 0x33u) return 0;
    snprintf(out, out_capacity, "HALK");
    return 1;
}

static int set_global_text(void *user, uint32_t global_index,
                           const char *text)
{
    TextOwner *owner = user;

    if (!owner || !text) return 0;
    ++owner->global_write_count;
    owner->global_index = global_index;
    snprintf(owner->global_text, sizeof(owner->global_text), "%s", text);
    return 1;
}

static void configure_action(CSB_V1_DSAImportedAction *action,
                             uint16_t *words, int word_count)
{
    memset(action, 0, sizeof(*action));
    action->dsa_id = 37u;
    action->state_index = 3u;
    action->column = 0u;
    action->program_words = words;
    action->program_word_count = word_count;
}

int main(void)
{
    /* Load object 42, local slot 1, TEXT@; then populate that same slot from
     * fingerprint 51 and atomically write it to global text slot 7. */
    uint16_t accepted_words[] = {
        0x0686u, 0x002au, 0x0686u, 1u, 0x19cbu,
        0x0686u, 0x0033u, 0x0686u, 1u, 0x1e8bu,
        0x0686u, 1u, 0x0686u, 7u, 0x1e4bu
    };
    uint16_t rejected_words[] = {
        0x0686u, 0x002au, 0x0686u, 1u, 0x19cbu,
        0x0686u, 1u, 0x0686u, 7u, 0x1e4bu, 0x0000u
    };
    CSB_V1_ChaosMagicState state;
    CSB_V1_DSAImportedAction action;
    CSB_V1_CSBWinDSAFilterStackRunnerContext runner;
    TextOwner owner;

    memset(&state, 0, sizeof(state));
    memset(&runner, 0, sizeof(runner));
    memset(&owner, 0, sizeof(owner));
    owner.source_available = 1;
    configure_action(&action, accepted_words,
                     (int)(sizeof(accepted_words) / sizeof(accepted_words[0])));
    state.imported_actions = &action;
    state.imported_action_count = 1;
    runner.programs = &state;
    runner.dsa_id = action.dsa_id;
    runner.state_index = action.state_index;
    runner.read_text = read_text;
    runner.read_character_name = read_character_name;
    runner.set_global_text = set_global_text;
    runner.text_user = &owner;

    check(csb_v1_csbwin_dsa_run_authenticated_filter_stack_action(
              &action, NULL, 0, NULL, &runner) == 1 &&
              owner.global_write_count == 1 && owner.global_index == 7u &&
              strcmp(owner.global_text, "HALK") == 0 &&
              runner.last_execution.global_text_store_count == 1u &&
              runner.last_execution.last_global_text_store_index == 7u,
          "TEXT@, CHARNAME@@ and GLOBALTEXT! commit one authenticated text-bank write");

    owner.global_write_count = 0;
    owner.global_index = 0u;
    owner.global_text[0] = '\0';
    configure_action(&action, rejected_words,
                     (int)(sizeof(rejected_words) / sizeof(rejected_words[0])));
    check(csb_v1_csbwin_dsa_run_authenticated_filter_stack_action(
              &action, NULL, 0, NULL, &runner) == 0 &&
              owner.global_write_count == 0,
          "later rejected bytecode rolls back staged GLOBALTEXT! publication");

    owner.source_available = 0;
    configure_action(&action, accepted_words,
                     (int)(sizeof(accepted_words) / sizeof(accepted_words[0])));
    check(csb_v1_csbwin_dsa_run_authenticated_filter_stack_action(
              &action, NULL, 0, NULL, &runner) == 0 &&
              owner.global_write_count == 0,
          "TEXT@ refuses an unavailable real DB2 owner without synthetic text");

    return failures == 0 ? 0 : 1;
}
