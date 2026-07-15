#include "theron_v1_cd_audio_availability.h"

int main(void) {
    Theron_V1CdAudioReceipt mismatch =
        theron_v1_cd_audio_availability("WAVE", "VORBIS");
    Theron_V1CdAudioReceipt ready =
        theron_v1_cd_audio_availability("WAVE", "WAVE");

    return mismatch.availability == THERON_V1_CD_AUDIO_FORMAT_MISMATCH &&
        !mismatch.playback_allowed &&
        ready.availability == THERON_V1_CD_AUDIO_READY &&
        ready.playback_allowed ? 0 : 1;
}
