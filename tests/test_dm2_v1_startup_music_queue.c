#include "dm2_v1_sound.h"
#include "dm2_v1_midi_backend.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    DM2_V1_MusicQueueReceipt receipt;
    DM2_V1_MusicScheduleReceipt schedule;
    static const unsigned char smf[] = {
        'M','T','h','d', 0,0,0,6, 0,0, 0,1, 0,96,
        'M','T','r','k', 0,0,0,8,
        0, 0x80,0x3c,0x00, 0x60, 0xff,0x2f,0
    };
    FILE *file;

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
    dm2_v1_sound_bind_verified_music_assets(".", 1);
    if (dm2_v1_sound_queue_music(0, 1, &receipt) !=
            DM2_V1_MUSIC_QUEUE_ASSET_MISSING ||
        strcmp(receipt.asset_path, "./00.hmp.mid") != 0 ||
        receipt.request_queued != 0) {
        fprintf(stderr, "DM2 title music lookup did not require 00.hmp.mid\n");
        return 1;
    }
    file = fopen("/tmp/00.hmp.mid", "wb");
    if (!file) {
        fprintf(stderr, "DM2 schedule fixture could not be written\n");
        return 1;
    }
    if (fwrite(smf, 1, sizeof(smf), file) != sizeof(smf) || fclose(file) != 0) {
        fprintf(stderr, "DM2 schedule fixture could not be written\n");
        return 1;
    }
    dm2_v1_sound_bind_verified_music_assets("/tmp", 1);
    if (dm2_v1_sound_queue_music(0, 1, &receipt) !=
            (dm2_v1_midi_backend_is_compiled() && receipt.backend_proven
                ? DM2_V1_MUSIC_QUEUE_READY
                : DM2_V1_MUSIC_QUEUE_DECODER_BACKEND_UNAVAILABLE) ||
        !receipt.schedule_handoff_ready || receipt.loop_duration_us != 500000 ||
        receipt.schedule_event_count != 2 ||
        !dm2_v1_sound_schedule_music(0, &schedule) ||
        !schedule.valid || schedule.event_count_due != 1 ||
        schedule.backend_proven != receipt.backend_proven ||
        schedule.pcm_handoff_ready ||
        !dm2_v1_sound_schedule_music(500000, &schedule) ||
        schedule.loop_count != 1 || schedule.event_count_due != 2) {
        remove("/tmp/00.hmp.mid");
        fprintf(stderr, "DM2 looped MIDI scheduling receipt mismatch\n");
        return 1;
    }
    remove("/tmp/00.hmp.mid");
    dm2_v1_sound_stop_music();
    puts("PASS DM2 title HMP lookup capability-gates native MIDI delivery without PCM");
    return 0;
}
