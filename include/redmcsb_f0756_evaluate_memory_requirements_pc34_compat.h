/*
 * ReDMCSB STARTUP2.C F0756_EvaluateMemoryRequirements, PC 3.4 route.
 *
 * This portable boundary keeps the source routine's state changes and callback
 * order without supplying a synthetic memory manager or UI.
 */
#ifndef FIRESTAFF_REDMCSB_F0756_EVALUATE_MEMORY_REQUIREMENTS_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0756_EVALUATE_MEMORY_REQUIREMENTS_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*redmcsb_f0756_process_map_pc34_compat_fn)(void *user,
                                                           int16_t map_index);
typedef void (*redmcsb_f0756_out_of_memory_pc34_compat_fn)(void *user,
                                                             int32_t kib_needed);
typedef void (*redmcsb_f0756_endgame_pc34_compat_fn)(void *user);

/*
 * Pointer fields correspond to STARTUP2.C globals G0309, G2134, G2136, and
 * G2138. process_map must update available_memory_for_sounds for each map.
 */
struct redmcsb_f0756_memory_requirements_pc34_compat {
    int16_t map_count;
    int16_t *party_map_index;
    int32_t *available_memory_for_sounds;
    int32_t *needed_memory;
    uint8_t *evaluating_memory_requirements;
    redmcsb_f0756_process_map_pc34_compat_fn process_map;
    redmcsb_f0756_out_of_memory_pc34_compat_fn out_of_memory;
    redmcsb_f0756_endgame_pc34_compat_fn endgame;
    void *user;
};

/*
 * Mirrors F0756. The fatal callbacks are invoked only for the source's
 * no-more-profiles/negative-memory case; if endgame returns, source cleanup
 * still occurs before the boolean result is returned.
 */
int redmcsb_f0756_evaluate_memory_requirements_pc34_compat(
    struct redmcsb_f0756_memory_requirements_pc34_compat *state,
    int no_more_memory_profile_to_test,
    int32_t byte_count);

const char *redmcsb_f0756_evaluate_memory_requirements_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
