#include "theron_v1_profile_media_audio_status.h"

Theron_V1ProfileMediaAudioStatus theron_v1_profile_media_audio_status(
    Theron_V1ProfileMediaAvailability media,
    Theron_V1CdAudioReceipt audio) {
    Theron_V1ProfileMediaAudioStatus receipt = {
        theron_v1_profile_media_availability_name(media),
        "missing",
        0,
        0
    };

    if (audio.availability == THERON_V1_CD_AUDIO_FORMAT_MISMATCH) {
        receipt.audio_status = "format_mismatch";
    } else if (audio.availability == THERON_V1_CD_AUDIO_READY) {
        receipt.audio_status = "ready";
    }
    if (audio.playback_allowed) {
        receipt.title_audio_available = 1;
        receipt.soul_room_audio_available = 1;
    }
    return receipt;
}
