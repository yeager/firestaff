#include "dm2_v1_sound.h"
#include "dm2_v1_midi_backend.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    DM2_V1_MusicQueueReceipt receipt;
    dm2_v1_sound_bind_verified_music_assets(NULL, 0);
    if (dm2_v1_sound_queue_music(0, 1, &receipt) !=
            DM2_V1_MUSIC_QUEUE_ASSET_ROOT_UNVERIFIED ||
        receipt.asset_resolved != 0 || receipt.request_queued != 0 ||
        receipt.decoder_proven != 0 || receipt.backend_proven != 0) {
        fprintf(stderr, "DM2 unverified asset root must not queue title music\n");
        return 1;
    }
    if (dm2_v1_sound_queue_music(DM2_MUSIC_TRACK_COUNT, 1, &receipt) !=
            DM2_V1_MUSIC_QUEUE_TRACK_OUT_OF_RANGE ||
        receipt.request_queued != 0) {
        fprintf(stderr, "DM2 out-of-range music track was accepted\n");
        return 1;
    }
    /* A directory that merely contains a same-named HMP is not game-data
     * provenance.  Runtime music must be read through a bound original GDAT
     * entry, so this old sidecar binding cannot make a title cue playable. */
    dm2_v1_sound_bind_verified_music_assets(".", 1);
    if (dm2_v1_sound_queue_music(0, 1, &receipt) !=
            DM2_V1_MUSIC_QUEUE_ASSET_ROOT_UNVERIFIED ||
        receipt.asset_path[0] != '\0' ||
        receipt.request_queued != 0) {
        fprintf(stderr, "DM2 title music accepted an unbound sidecar path\n");
        return 1;
    }
    dm2_v1_sound_stop_music();
    puts("PASS DM2 title HMP rejects unbound sidecar audio paths");
    return 0;
}
