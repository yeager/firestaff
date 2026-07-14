/*
 * ReDMCSB IO.C F0710_WaitSoundPlayComplete, PC 3.4 (I34E/I34M) route.
 *
 * The PC game owns sound completion in IODRV_16.  F0710 contains no polling,
 * timing, or mixer policy of its own: it calls that one driver slot once.
 */
#ifndef FIRESTAFF_REDMCSB_F0710_WAIT_SOUND_PLAY_COMPLETE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0710_WAIT_SOUND_PLAY_COMPLETE_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*redmcsb_f0710_wait_sound_play_complete_callback_pc34_compat)(
    void *context);

typedef struct {
    redmcsb_f0710_wait_sound_play_complete_callback_pc34_compat
        wait_sound_play_complete;
    void *context;
} redmcsb_f0710_io_driver_pc34_compat;

/*
 * Executes the sole PC 3.4 F0710 action:
 * G2161_IODriver->IODRV_16_WaitSoundPlayComplete().
 *
 * Returns false only when the supplied host driver cannot represent the
 * original vector.  It never invents a completion condition.
 */
bool redmcsb_f0710_wait_sound_play_complete_pc34_compat(
    const redmcsb_f0710_io_driver_pc34_compat *io_driver);

const char *redmcsb_f0710_wait_sound_play_complete_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
