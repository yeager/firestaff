#ifndef FIRESTAFF_DM2_V1_GLOBAL_EFFECT_TIMER_HELPERS_H
#define FIRESTAFF_DM2_V1_GLOBAL_EFFECT_TIMER_HELPERS_H

#include "dm2_v1_timeline.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_V1_TIMER_TYPE_GLOBAL_EFFECT_0E 0x0eu

typedef struct {
    int handled;
    int source_locked;
    int valid;
    int blocked;
    int mutated;
    uint8_t timer_type;
    uint8_t effect_index;
    uint8_t effect_before;
    uint8_t effect_after;
    uint32_t game_tick;
    uint32_t dispatched_count;
    uint32_t remaining_timer_count;
    const char *symbol;
    const char *source_path;
} DM2_V1_GlobalEffectTimerReceipt;

void dm2_v1_global_effect_timer_receipt_clear(
    DM2_V1_GlobalEffectTimerReceipt *receipt);

int dm2_v1_PROCESS_TIMER_0E(
    const DM2_V1_SourceTimer *timer,
    uint8_t *global_effects,
    size_t global_effect_count,
    DM2_V1_GlobalEffectTimerReceipt *out_receipt);

int dm2_v1_PROCEED_GLOBAL_EFFECT_TIMERS(
    DM2_V1_SourceTimerQueue *queue,
    uint32_t game_tick,
    uint8_t *global_effects,
    size_t global_effect_count,
    DM2_V1_GlobalEffectTimerReceipt *out_receipt);

const char *dm2_v1_global_effect_timer_helpers_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_GLOBAL_EFFECT_TIMER_HELPERS_H */
