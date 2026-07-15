#ifndef THERON_V1_PROFILE_MEDIA_AUDIO_STATUS_H
#define THERON_V1_PROFILE_MEDIA_AUDIO_STATUS_H

#include "theron_v1_cd_audio_availability.h"
#include "theron_v1_profile_media_availability.h"

typedef struct {
    const char *raw_track_status;
    const char *audio_status;
    int title_audio_available;
    int soul_room_audio_available;
} Theron_V1ProfileMediaAudioStatus;

Theron_V1ProfileMediaAudioStatus theron_v1_profile_media_audio_status(
    Theron_V1ProfileMediaAvailability media,
    Theron_V1CdAudioReceipt audio);

#endif
