/* ReDMCSB IBMIO.C F8123/F8124 PC 3.4 sound-completion routes. */
#ifndef FIRESTAFF_REDMCSB_F8123_SOUND_COMPLETION_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F8123_SOUND_COMPLETION_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    REDMCSB_F8123_SOUND_NONE_PC34 = 1,
    REDMCSB_F8123_SOUND_PC_SPEAKER_PC34 = 2,
    REDMCSB_F8123_SOUND_FTL_ADAPTER_PC34 = 3,
    REDMCSB_F8123_SOUND_TANDY_PC34 = 4,
    REDMCSB_F8123_SOUND_DISNEY_PC34 = 5,
    REDMCSB_F8123_SOUND_BLASTER_PC34 = 6,
    REDMCSB_F8123_SOUND_ADLIB_PC34 = 7
};

typedef int16_t (*redmcsb_f8123_tandy_timer_status_pc34_compat)(void *context);

typedef struct redmcsb_f8123_sound_state_pc34_compat {
    int16_t sound_device;
    int16_t remaining_sound_units;
    redmcsb_f8123_tandy_timer_status_pc34_compat tandy_timer_status;
    void *context;
} redmcsb_f8123_sound_state_pc34_compat;

/* IBMIO.C:2080-2083 has an intentionally empty PC implementation. */
void redmcsb_f8123_play_audio_cd_track_pc34_compat(
    redmcsb_f8123_sound_state_pc34_compat *sound_state);

/* Returns V8117006_ for supported non-Tandy devices, not a normalized bool. */
int16_t redmcsb_f8124_is_sound_play_complete_pc34_compat(
    const redmcsb_f8123_sound_state_pc34_compat *sound_state);

const char *redmcsb_f8123_sound_completion_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
