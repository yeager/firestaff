#ifndef FIRESTAFF_CSB_V1_F2262_TIMER_A_EVENT_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F2262_TIMER_A_EVENT_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CSB_V1_F2262_TimerAEventState_PC34 {
    uint16_t wait_for_input_vblank_count;
    uint16_t wait_for_input_maximum_vblank_count;
    int stop_waiting_for_player_input;
    int fm_towns_timer_available;
    int fm_towns_volume_fade_available;
    int fm_towns_counter;
    int fm_towns_volume_fade_active;
    int fm_towns_volume;
    int fm_towns_volume_muted;
} CSB_V1_F2262_TimerAEventState_PC34;

typedef struct CSB_V1_F2262_TimerAEventReceipt_PC34 {
    int valid;
    int input_wait_advanced;
    int stop_waiting_set;
    int fm_towns_counter_advanced;
    int fm_towns_volume_advanced;
    int fm_towns_platform_branch_unavailable;
    uint16_t wait_for_input_vblank_count_before;
    uint16_t wait_for_input_vblank_count_after;
    uint16_t wait_for_input_maximum_vblank_count;
    int stop_waiting_before;
    int stop_waiting_after;
} CSB_V1_F2262_TimerAEventReceipt_PC34;

int csb_v1_f2262_timer_a_event_pc34(
    CSB_V1_F2262_TimerAEventState_PC34 *state,
    CSB_V1_F2262_TimerAEventReceipt_PC34 *out_receipt);

const char *csb_v1_f2262_timer_a_event_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
