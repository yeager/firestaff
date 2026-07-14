#ifndef FIRESTAFF_REDMCSB_F0945_INIT_AUDIO_DATA_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0945_INIT_AUDIO_DATA_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ReDMCSB F0945 initializes Amiga audio.device request blocks, reply ports,
 * and the four hardware audio channels. Its SOUND.C implementation is only
 * selected for Amiga media, not I34E/I34M. PC 3.4 therefore has no source
 * behavior to reproduce and this boundary is explicitly unavailable.
 */
bool redmcsb_f0945_init_audio_data_pc34_compat(void);

const char *redmcsb_f0945_init_audio_data_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0945_INIT_AUDIO_DATA_PC34_COMPAT_H */
