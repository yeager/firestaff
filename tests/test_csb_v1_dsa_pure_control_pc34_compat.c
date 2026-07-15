/* CSBWin DSA pure-control and pure-stack opcode regression.
 * Source: Data.h DSAnoopCmd/DSAequalCmd; DSA.cpp EX_NOOP:574-591,
 * EX_EQUAL:1491-1515, STKOP_Loc2AbsCoord:3253-3268,
 * STKOP_BitCount:4832-4848 and STKOP_ParamFetch/ParamStore/VSET:
 * 2956-3044,4850-4887, plus STKOP_PartyDistance:4057-4072,
 * STKOP_TimeFetch:2512-2518, STKOP_ThisDSAId:4822-4828,
 * STKOP_WhoHasTalent:4363-4380, STKOP_CountInjury:4798-4817, and
 * STKOP_TalentsFetch:4243-4283. These
 * commands and STKOP_Fetch/Store:2473-2488 have no filter or world effect. */

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
    context.party_location_valid = 1;
    context.party_level = 5;
    context.party_x = 10;
    context.party_y = 12;
    context.game_time_valid = 1;
    context.game_time = 12345u;
    context.dsa_slave_thing_valid = 1;
    context.dsa_slave_thing = 0x8123u;
    context.party_champions_valid = 1;
    context.party_champion_count = 4;
    context.party_leader_index = 2;
    context.party_champion_talents[0] = 0x3u;
    context.party_champion_talents[1] = 0x1u;
    context.party_champion_talents[2] = 0x7u;
    context.party_champion_wounds[0] = 0x0003u;
    context.party_champion_wounds[1] = 0x000fu;
    context.party_champion_wounds[2] = 0x0006u;
    context.party_champion_wounds[3] = 0x8000u;
    context.party_champion_health[0] = 10;
    context.party_champion_health[1] = 0;
    context.party_champion_health[2] = 20;
    context.party_champion_health[3] = 30;
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
    uint16_t party_distance_same_level[] = {
        0x0786u, 0x14f4u, 0u, 0x0055u, 0x000du
    };
    uint16_t party_distance_other_level[] = {
        0x0786u, 0x08f4u, 0u, 0x0055u, 0x000du
    };
    uint16_t time_fetch[] = { 0x184bu, 0x000du };
    uint16_t this_dsa_id[] = { 0x0155u, 0x000du };
    uint16_t local_fetch_store[] = {
        0x0686u, 3u, 0x0686u, 0u, 0x0a0bu,
        0x0686u, 1u, 0x098bu, 0x0686u, 2u, 0x09cbu,
        0x0686u, 3u, 0x0686u, 0u, 0x0a4bu
    };
    uint16_t local_fetch_bad_index[] = { 0x0686u, 100u, 0x098bu };
    uint16_t who_has_talent[] = { 0x0686u, 1u, 0x1d4bu, 0x000du };
    uint16_t count_injury[] = {
        0x0686u, 15u, 0x0686u, 7u, 0x1a8bu, 0x000du
    };
    uint16_t talents_fetch_leader[] = {
        0x0686u, 4u, 0x0195u, 0x000du
    };
    uint16_t talents_fetch_missing[] = {
        0x0686u, 7u, 0x0195u, 0x000du
    };
    uint16_t talents_fetch_wing[] = {
        0x0786u, 0u, 1u, 0x0195u, 0x000du
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

    parameters[0] = 77u;
    check(run(&state, &action, party_distance_same_level,
              (int)(sizeof(party_distance_same_level) /
                    sizeof(party_distance_same_level[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 11u && execution.stack_depth == 0u,
          "PARTYDISTANCE returns source Manhattan distance on the party level");

    parameters[0] = 77u;
    check(run(&state, &action, party_distance_other_level,
              (int)(sizeof(party_distance_other_level) /
                    sizeof(party_distance_other_level[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 0xfffffffdu && execution.stack_depth == 0u,
          "PARTYDISTANCE returns negative source level distance off-level");

    parameters[0] = 77u;
    check(run(&state, &action, time_fetch,
              (int)(sizeof(time_fetch) / sizeof(time_fetch[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 12345u && execution.stack_depth == 0u,
          "TIME@ reads the runtime-owned CSBWin game clock");

    parameters[0] = 77u;
    check(run(&state, &action, this_dsa_id,
              (int)(sizeof(this_dsa_id) / sizeof(this_dsa_id[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 0x8123u && execution.stack_depth == 0u,
          "THIS_DSA_ID returns the verified source actuator Thing identity");

    parameters[0] = 10u;
    parameters[1] = 20u;
    parameters[2] = 30u;
    parameters[3] = 40u;
    check(run(&state, &action, local_fetch_store,
              (int)(sizeof(local_fetch_store) / sizeof(local_fetch_store[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 10u && parameters[1] == 20u &&
              parameters[2] == 20u && parameters[3] == 40u &&
              execution.stack_depth == 0u,
          "FETCH and STORE preserve CSBWin DSAVARS stack order");

    parameters[0] = 77u;
    check(run(&state, &action, local_fetch_bad_index,
              (int)(sizeof(local_fetch_bad_index) /
                    sizeof(local_fetch_bad_index[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_SOURCE_ILLEGAL &&
              parameters[0] == 77u,
          "FETCH rejects an out-of-bank source local without publication");

    parameters[0] = 77u;
    check(run(&state, &action, who_has_talent,
              (int)(sizeof(who_has_talent) / sizeof(who_has_talent[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 7u && execution.stack_depth == 0u,
          "WHO_HAS_TALENT returns the source party talent mask");

    parameters[0] = 77u;
    check(run(&state, &action, count_injury,
              (int)(sizeof(count_injury) / sizeof(count_injury[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 4u && execution.stack_depth == 0u,
          "COUNT_INJURY skips dead champions and counts selected source wounds");

    parameters[0] = 77u;
    check(run(&state, &action, talents_fetch_leader,
              (int)(sizeof(talents_fetch_leader) /
                    sizeof(talents_fetch_leader[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 0x7u && execution.stack_depth == 0u,
          "TALENTS@ resolves source hand character through the live party");

    parameters[0] = 77u;
    check(run(&state, &action, talents_fetch_missing,
              (int)(sizeof(talents_fetch_missing) /
                    sizeof(talents_fetch_missing[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 0u && execution.stack_depth == 0u,
          "TALENTS@ returns source zero for a missing in-party index");

    parameters[0] = 77u;
    check(run(&state, &action, talents_fetch_wing,
              (int)(sizeof(talents_fetch_wing) /
                    sizeof(talents_fetch_wing[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              parameters[0] == 77u,
          "TALENTS@ keeps unbound CSBWin wing records unavailable");

    state.imported_actions = NULL;
    state.imported_action_count = 0;
    csb_v1_chaos_cleanup(&state);
    return failures == 0 ? 0 : 1;
}
