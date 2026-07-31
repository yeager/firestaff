#include "dm2_v1_sound.h"
#include "dm2_v1_midi_backend.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    static const unsigned char smf[] = {
        'M','T','h','d', 0,0,0,6, 0,0, 0,1, 0,96,
        'M','T','r','k', 0,0,0,8,
        0, 0x90,0x3c,0x40, 0x60, 0xff,0x2f,0
    };
    DM2_V1_MusicQueueReceipt receipt;
    DM2_V1_MusicStreamReceipt stream;
    DM2_V1_MusicScheduleReceipt schedule;
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
    if (dm2_v1_sound_inspect_music_data(smf, sizeof(smf), &stream) !=
            DM2_V1_MUSIC_INSPECT_OK ||
        !dm2_v1_sound_schedule_music(0u, &schedule) ||
        schedule.event_count_due == 0u ||
        dm2_v1_sound_stop_music() != 0 ||
        dm2_v1_sound_schedule_music(0u, &schedule) != 0) {
        fprintf(stderr, "DM2 music stop retained a scheduled source stream\n");
        return 1;
    }
    puts("PASS DM2 title HMP rejects unbound sidecar audio paths");
    return 0;
}
