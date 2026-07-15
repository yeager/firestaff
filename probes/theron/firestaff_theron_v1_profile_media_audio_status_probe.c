#include "theron_v1_profile_media_audio_status.h"

#include <string.h>

int main(void) {
    Theron_V1CdAudioReceipt audio = {
        THERON_V1_CD_AUDIO_FORMAT_MISMATCH,
        0
    };
    Theron_V1ProfileMediaAudioStatus receipt =
        theron_v1_profile_media_audio_status(
            THERON_V1_PROFILE_MEDIA_END_VARIANT, audio);

    return strcmp(receipt.raw_track_status,
                  "raw_track_required_end_variant") == 0 &&
        strcmp(receipt.audio_status, "format_mismatch") == 0 &&
        !receipt.title_audio_available &&
        !receipt.soul_room_audio_available ? 0 : 1;
}
