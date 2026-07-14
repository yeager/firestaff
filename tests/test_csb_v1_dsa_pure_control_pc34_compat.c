/* CSBWin DSA pure-control and pure-stack opcode regression.
 * Source: Data.h DSAnoopCmd/DSAequalCmd; DSA.cpp EX_NOOP:574-591,
 * EX_EQUAL:1491-1515, STKOP_Loc2AbsCoord:3253-3268,
 * STKOP_BitCount:4832-4848 and STKOP_ParamFetch/ParamStore/VSET:
 * 2956-3044,4850-4887. These commands have no filter or world effect. */

#include "csb_v1_chaos_magic_pc34_compat.h"

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

static CSB_V1_CSBWinDSAStackResult run(
    CSB_V1_ChaosMagicState *state, CSB_V1_DSAImportedAction *action,
    uint16_t *words, int word_count, uint32_t *parameters,
    CSB_V1_CSBWinDSAStackExecution *out_execution)
{
    CSB_V1_CSBWinDSAStackContext context;

    memset(&context, 0, sizeof(context));
    action->program_words = words;
    action->program_word_count = word_count;
    context.parameters = parameters;
    context.parameter_count = 4;
    return csb_v1_csbwin_dsa_execute_authenticated_stack_action(
        state, 7, 1u, 0, &context, out_execution);
}

int main(void)
{
    uint16_t noop_relative[] = { 0xffc3u };
    uint16_t noop_extended[] = { 0x8003u, 0xfffeu };
    uint16_t equal_true[] = {
        0x0686u, 5u, 0x0686u, 5u, 0x0048u, 0x080du
    };
    uint16_t equal_false[] = {
        0x0686u, 5u, 0x0686u, 6u, 0x0048u, 0x080du
    };
    uint16_t equal_underflow[] = { 0x0008u };
    uint16_t question_true[] = { 0x0686u, 1u, 0x0049u };
    uint16_t question_false[] = { 0x0686u, 0u, 0x03c9u };
    uint16_t question_extended[] = { 0x0686u, 1u, 0x0389u, 0xfffeu };
    uint16_t question_jump[] = { 0x0686u, 1u, 0x0849u, 2u };
    uint16_t loc2abscoord[] = {
        0x0786u, 0x9629u, 0x0002u, 0x0c0bu,
        0x00cdu, 0x008du, 0x004du, 0x000du
    };
    uint16_t bitcount[] = {
        0x0786u, 0x0f0fu, 0xf0f0u, 0x1b0bu, 0x000du
    };
    uint16_t bitcount_underflow[] = { 0x1b0bu };
    uint16_t parameter_round_trip[] = {
        0x0686u, 4u, 0x0686u, 0u, 0x0a0bu,
        0x0686u, 1u, 0x0686u, 0u, 0x0686u, 3u, 0x0215u,
        0x0686u, 3u, 0x0686u, 0u, 0x0a4bu
    };
    uint16_t parameter_constant[] = {
        0x0686u, 107u, 0x0686u, 0u, 0x0686u, 4u, 0x0215u,
        0x0686u, 4u, 0x0686u, 0u, 0x0a4bu
    };
    uint16_t parameter_bad_variable_span[] = {
        0x0686u, 4u, 0x0686u, 98u, 0x0a0bu
    };
    uint32_t parameters[4] = { 77u, 0u, 0u, 0u };
    CSB_V1_DSAImportedAction action;
    CSB_V1_ChaosMagicState state;
    CSB_V1_CSBWinDSAStackExecution execution;

    memset(&action, 0, sizeof(action));
    memset(&execution, 0, sizeof(execution));
    csb_v1_chaos_init(&state);
    action.dsa_id = 7u;
    action.state_index = 1u;
    action.column = 0u;
    state.imported_actions = &action;
    state.imported_action_count = 1;

    check(run(&state, &action, noop_relative,
              (int)(sizeof(noop_relative) / sizeof(noop_relative[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              execution.next_state == -1 && execution.words_consumed == 1u &&
              execution.stack_depth == 0u && parameters[0] == 77u,
          "NOOP preserves a signed inline next-state without mutation");

    check(run(&state, &action, noop_extended,
              (int)(sizeof(noop_extended) / sizeof(noop_extended[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              execution.next_state == 65534 && execution.words_consumed == 2u &&
              execution.stack_depth == 0u && parameters[0] == 77u,
          "NOOP MAXSTATE consumes its exact raw extension word");

    check(run(&state, &action, equal_true,
              (int)(sizeof(equal_true) / sizeof(equal_true[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 1u && execution.next_state == 1 &&
              execution.stack_depth == 0u,
          "EQUAL stores one for equal source stack words");

    parameters[0] = 77u;
    check(run(&state, &action, equal_false,
              (int)(sizeof(equal_false) / sizeof(equal_false[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 0u && execution.next_state == 1 &&
              execution.stack_depth == 0u,
          "EQUAL stores zero for unequal source stack words");

    parameters[0] = 77u;
    check(run(&state, &action, equal_underflow,
              (int)(sizeof(equal_underflow) / sizeof(equal_underflow[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_MALFORMED &&
              parameters[0] == 77u,
          "EQUAL stack underflow rejects without parameter publication");

    parameters[0] = 77u;
    check(run(&state, &action, question_true,
              (int)(sizeof(question_true) / sizeof(question_true[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              execution.next_state == 1 && execution.stack_depth == 0u &&
              parameters[0] == 77u,
          "QUESTION consumes a true source condition without side effects");

    check(run(&state, &action, question_false,
              (int)(sizeof(question_false) / sizeof(question_false[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              execution.next_state == -1 && execution.stack_depth == 0u &&
              parameters[0] == 77u,
          "QUESTION consumes a false source condition without side effects");

    check(run(&state, &action, question_extended,
              (int)(sizeof(question_extended) / sizeof(question_extended[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              execution.next_state == 65534 && execution.words_consumed == 4u,
          "QUESTION MAXSTATE consumes its exact raw extension word");

    check(run(&state, &action, question_jump,
              (int)(sizeof(question_jump) / sizeof(question_jump[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              parameters[0] == 77u,
          "QUESTION jump branch remains closed without an action-chain owner");

    parameters[0] = parameters[1] = parameters[2] = parameters[3] = 77u;
    check(run(&state, &action, loc2abscoord,
              (int)(sizeof(loc2abscoord) / sizeof(loc2abscoord[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 37u && parameters[1] == 17u &&
              parameters[2] == 9u && parameters[3] == 2u &&
              execution.stack_depth == 0u,
          "LOC2ABSCOORD decodes an original packed location in source order");

    parameters[0] = 77u;
    check(run(&state, &action, bitcount,
              (int)(sizeof(bitcount) / sizeof(bitcount[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 16u && execution.stack_depth == 0u,
          "BITCOUNT consumes and counts all 32 original source bits");

    parameters[0] = 77u;
    check(run(&state, &action, bitcount_underflow,
              (int)(sizeof(bitcount_underflow) /
                    sizeof(bitcount_underflow[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_MALFORMED &&
              parameters[0] == 77u,
          "BITCOUNT underflow rejects without parameter publication");

    parameters[0] = 10u;
    parameters[1] = 20u;
    parameters[2] = 30u;
    parameters[3] = 40u;
    check(run(&state, &action, parameter_round_trip,
              (int)(sizeof(parameter_round_trip) /
                    sizeof(parameter_round_trip[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 20u && parameters[1] == 30u &&
              parameters[2] == 40u && parameters[3] == 40u &&
              execution.stack_depth == 0u,
          "PARAM@ VSET PARAM! preserves source variable and parameter order");

    parameters[0] = 10u;
    parameters[1] = 20u;
    parameters[2] = 30u;
    parameters[3] = 40u;
    check(run(&state, &action, parameter_constant,
              (int)(sizeof(parameter_constant) /
                    sizeof(parameter_constant[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 7u && parameters[1] == 7u &&
              parameters[2] == 7u && parameters[3] == 7u,
          "VSET positive constant writes a bounded source DSAVARS range");

    parameters[0] = 10u;
    parameters[1] = 20u;
    parameters[2] = 30u;
    parameters[3] = 40u;
    check(run(&state, &action, parameter_bad_variable_span,
              (int)(sizeof(parameter_bad_variable_span) /
                    sizeof(parameter_bad_variable_span[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_SOURCE_ILLEGAL &&
              parameters[0] == 10u && parameters[1] == 20u &&
              parameters[2] == 30u && parameters[3] == 40u,
          "PARAM@ rejects an out-of-bank DSAVARS span without publication");

    state.imported_actions = NULL;
    state.imported_action_count = 0;
    csb_v1_chaos_cleanup(&state);
    return failures == 0 ? 0 : 1;
}
