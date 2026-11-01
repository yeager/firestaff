#include "dm2_v1_mac_media.h"
#include "dm2_v1_sound.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char *zip = getenv("FIRESTAFF_DM2_MAC_EN_ZIP");
    DM2_V1_MacMedia media;
    DM2_V1_MusicQueueReceipt receipt;
    DM2_V1_MusicScheduleReceipt first_schedule;
    DM2_V1_MusicScheduleReceipt repeated_schedule;
    int result;
    int song_id;

    if (dm2_v1_sound_scale_midi_channel_volume(100u, 128) != 100u ||
        dm2_v1_sound_scale_midi_channel_volume(100u, 64) != 50u ||
        dm2_v1_sound_scale_midi_channel_volume(100u, 0) != 0u ||
        dm2_v1_sound_scale_midi_channel_volume(100u, 200) != 100u) {
        fprintf(stderr, "DM2 Mac MIDI controller volume scaling failed\n");
        return 1;
    }

    {
        FILE *archive = zip && zip[0] ? fopen(zip, "rb") : NULL;
        if (!archive) {
            puts("SKIP: authentic DM2 Mac retail ZIP is not staged");
            return 77;
        }
        fclose(archive);
    }
    if (dm2_v1_mac_media_read_zip(zip, &media) != 0 ||
        !media.application_resource || media.application_resource_size == 0u) {
        fprintf(stderr, "authentic Mac application resource fork unavailable\n");
        return 1;
    }
    for (song_id = 1000; song_id < 1028; ++song_id) {
        const uint8_t *song_data = NULL;
        size_t song_size = 0u;
        int midi_id;
        char expected_path[256];
        if (dm2_v1_mac_resource_find(
                media.application_resource, media.application_resource_size,
                "SONG", (int16_t)song_id, &song_data, &song_size, NULL) != 0 ||
            !song_data || song_size < 2u) {
            fprintf(stderr, "authentic Mac SONG(%d) mapping is unavailable\n",
                    song_id);
            dm2_v1_mac_media_free(&media);
            return 1;
        }
        midi_id = ((int)song_data[0] << 8) | (int)song_data[1];
        snprintf(expected_path, sizeof(expected_path),
                 "Dungeon Master II::SONG(%d)->Midi(%d)", song_id, midi_id);
        result = dm2_v1_sound_queue_mac_midi(
            media.application_resource, media.application_resource_size,
            song_id, 1, &receipt);
        if ((result != DM2_V1_MUSIC_QUEUE_READY &&
             result != DM2_V1_MUSIC_QUEUE_DECODER_BACKEND_UNAVAILABLE) ||
            !receipt.asset_resolved || !receipt.decoder_proven ||
            !receipt.schedule_handoff_ready || receipt.schedule_event_count == 0u ||
            strcmp(receipt.asset_path, expected_path) != 0) {
            fprintf(stderr,
                    "authentic Mac SONG(%d) route failed: result=%d resolved=%d decoder=%d schedule=%d events=%u path=%s\n",
                    song_id, result, receipt.asset_resolved,
                    receipt.decoder_proven, receipt.schedule_handoff_ready,
                    receipt.schedule_event_count, receipt.asset_path);
            dm2_v1_mac_media_free(&media);
            return 1;
        }
    }
    if (!dm2_v1_sound_schedule_music(0u, &first_schedule) ||
        !dm2_v1_sound_schedule_music(0u, &repeated_schedule) ||
        first_schedule.event_count_due == 0u ||
        first_schedule.delivery_failed ||
        first_schedule.backend_proven != receipt.backend_proven ||
        repeated_schedule.event_count_due != 0u) {
        fprintf(stderr,
                "authentic Mac Midi scheduler delivery/prefix check failed: first_due=%u first_sent=%u failed=%d first_backend=%d expected_backend=%d repeated=%u\n",
                first_schedule.event_count_due,
                first_schedule.event_count_sent,
                first_schedule.delivery_failed,
                first_schedule.backend_proven,
                receipt.backend_proven,
                repeated_schedule.event_count_due);
        dm2_v1_mac_media_free(&media);
        return 1;
    }
    dm2_v1_mac_media_free(&media);
    printf("PASS: authentic Mac SONG(1000..1027) mappings reached SMF scheduling; events=%u sent=%u backend=%s\n",
           receipt.schedule_event_count,
           first_schedule.event_count_sent,
           receipt.backend_proven ? "ready" : "unavailable");
    if (!receipt.backend_proven) {
        puts("SKIP: source parsing passed, but no native MIDI playback backend is available");
        return 77;
    }
    return 0;
}
