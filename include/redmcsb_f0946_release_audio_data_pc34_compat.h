#ifndef FIRESTAFF_REDMCSB_F0946_RELEASE_AUDIO_DATA_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0946_RELEASE_AUDIO_DATA_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * SOUND.C F0946_ReleaseAudioData releases Amiga audio.device IOAudio
 * requests and message ports through Exec APIs. No PC 3.4 branch or portable
 * host adapter is supplied, so this operation is not applicable on this host.
 */
bool redmcsb_f0946_release_audio_data_pc34_compat(void);

const char *redmcsb_f0946_release_audio_data_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0946_RELEASE_AUDIO_DATA_PC34_COMPAT_H */
