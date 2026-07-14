#include <limits.h>

#include "redmcsb_f0756_evaluate_memory_requirements_pc34_compat.h"

int redmcsb_f0756_evaluate_memory_requirements_pc34_compat(
    struct redmcsb_f0756_memory_requirements_pc34_compat *state,
    int no_more_memory_profile_to_test,
    int32_t byte_count)
{
    int16_t map_index;
    int16_t party_map_index_backup;
    int32_t available_memory_for_sounds;

    /* ReDMCSB STARTUP2.C:1091-1118, MEDIA728 PC 3.4 route. */
    party_map_index_backup = *state->party_map_index;
    *state->evaluating_memory_requirements = 1U;
    available_memory_for_sounds = INT32_MAX;
    for (map_index = 0; map_index < state->map_count; map_index++) {
        state->process_map(state->user, map_index);
        if (*state->available_memory_for_sounds - byte_count <
            available_memory_for_sounds) {
            available_memory_for_sounds =
                *state->available_memory_for_sounds - byte_count;
        }
    }
    if (available_memory_for_sounds > *state->needed_memory) {
        *state->needed_memory = available_memory_for_sounds;
    }
    if (no_more_memory_profile_to_test != 0 &&
        available_memory_for_sounds < 0) {
        state->out_of_memory(
            state->user, (-*state->needed_memory + 1023) >> 10);
        state->endgame(state->user);
    }
    *state->evaluating_memory_requirements = 0U;
    *state->party_map_index = party_map_index_backup;
    return available_memory_for_sounds >= 0;
}

const char *redmcsb_f0756_evaluate_memory_requirements_source_evidence_pc34(void)
{
    return "ReDMCSB WIP20210206 STARTUP2.C:1081-1120 defines "
           "F0756_EvaluateMemoryRequirements: process every map while "
           "G2136 is set, retain the minimum G2138 minus profile byte count, "
           "raise G2134 when needed, report/endgame on final negative memory, "
           "then restore G0309 and clear G2136.";
}
