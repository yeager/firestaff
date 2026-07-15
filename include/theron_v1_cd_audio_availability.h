#ifndef THERON_V1_CD_AUDIO_AVAILABILITY_H
#define THERON_V1_CD_AUDIO_AVAILABILITY_H

typedef enum {
    THERON_V1_CD_AUDIO_MISSING = 0,
    THERON_V1_CD_AUDIO_FORMAT_MISMATCH = 1,
    THERON_V1_CD_AUDIO_READY = 2
} Theron_V1CdAudioAvailability;

typedef struct {
    Theron_V1CdAudioAvailability availability;
    int playback_allowed;
} Theron_V1CdAudioReceipt;

Theron_V1CdAudioReceipt theron_v1_cd_audio_availability(
    const char *cue_format,
    const char *local_format);

#endif
