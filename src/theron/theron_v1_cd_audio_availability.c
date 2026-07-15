#include "theron_v1_cd_audio_availability.h"

#include <string.h>

Theron_V1CdAudioReceipt theron_v1_cd_audio_availability(
    const char *cue_format,
    const char *local_format) {
    Theron_V1CdAudioReceipt receipt = {
        THERON_V1_CD_AUDIO_MISSING,
        0
    };

    if (!cue_format || !local_format) {
        return receipt;
    }
    if (strcmp(cue_format, "WAVE") == 0 &&
        strcmp(local_format, "VORBIS") == 0) {
        receipt.availability = THERON_V1_CD_AUDIO_FORMAT_MISMATCH;
        return receipt;
    }
    if (strcmp(cue_format, local_format) == 0) {
        receipt.availability = THERON_V1_CD_AUDIO_READY;
        receipt.playback_allowed = 1;
    }
    return receipt;
}
