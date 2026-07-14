#ifndef FIRESTAFF_REDMCSB_F0709_START_SOUND_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0709_START_SOUND_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB SOUND.C F0709_StartSound. For PC 3.4 (I34E), the source
 * declaration takes an int16 sound index and an int16 scalar volume
 * (DEFS.H:9385-9389). SOUND.C materializes the SND buffer before crossing
 * this platform boundary (SOUND.C:1612-1618, 1845-1849).
 */
typedef void (*redmcsb_f0709_start_sound_pc34_backend)(
    void *context,
    int16_t sound_index,
    int16_t sound_volume);

/* Invokes the PC 3.4 audio backend with the source arguments unchanged.
 * A missing backend is a no-op, preserving the source routine's void API
 * while keeping host audio ownership outside this compatibility bridge.
 */
void redmcsb_f0709_start_sound_pc34_compat(
    int16_t sound_index,
    int16_t sound_volume,
    redmcsb_f0709_start_sound_pc34_backend backend,
    void *backend_context);

#ifdef __cplusplus
}
#endif

#endif
