#include "nexus_v1_iso_reader.h"
#include "nexus_v1_sound.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char *root = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    const char *zip = getenv("FIRESTAFF_NEXUS_ZIP");
    char cue[768];
    char payload[768];
    int track;
    Nexus_SoundEngine sound;

    if (!root || !root[0]) {
        puts("SKIP: FIRESTAFF_NEXUS_DATA_DIR is unset");
        return 77;
    }
    if (snprintf(cue, sizeof(cue), "%s/Dungeon Master Nexus (Japan).cue",
                 root) < 0 || strlen(cue) >= sizeof(cue)) {
        puts("SKIP: Nexus CUE path is too long");
        return 77;
    }

    for (track = 2; track <= 9; ++track) {
        if (nexus_iso_cue_audio_track_path(cue, track, payload,
                                           (int)sizeof(payload)) != 0 ||
            !strstr(payload, "(Track ") || !strstr(payload, ").bin")) {
            fprintf(stderr, "FAIL: authentic CUE track %d did not bind a raw BIN\n",
                    track);
            return 1;
        }
    }
    if (nexus_iso_cue_audio_track_path(cue, 1, payload,
                                       (int)sizeof(payload)) == 0 ||
        payload[0] != '\0') {
        fputs("FAIL: MODE1 data track was promoted as CDDA\n", stderr);
        return 1;
    }
    memset(&sound, 0, sizeof(sound));
    if (nexus_sound_init(&sound) != 0) {
        fputs("FAIL: native Nexus sound consumer did not initialize\n", stderr);
        return 1;
    }
    nexus_sound_set_cue_path(&sound, cue);
    if (nexus_sound_cd_track(&sound, 2) != 0 || !sound.cd_source_bound ||
        !strstr(sound.cd_source_path, "(Track 2).bin") ||
        sound.cd_track_path[0] != '\0' || sound.cd_playing != 0) {
        fputs("FAIL: raw CUE track was not retained by the native no-playback consumer\n",
              stderr);
        nexus_sound_shutdown(&sound);
        return 1;
    }
    nexus_sound_shutdown(&sound);
    if (zip && zip[0]) {
        for (track = 2; track <= 9; ++track) {
            if (nexus_iso_zip_cue_audio_track_path(zip, track, payload,
                                                   (int)sizeof(payload)) != 0 ||
                !strstr(payload, "::Dungeon Master Nexus (Japan) (Track ") ||
                !strstr(payload, ").bin")) {
                fprintf(stderr, "FAIL: authentic ZIP CUE track %d did not bind a raw BIN\n",
                        track);
                return 1;
            }
        }
        if (nexus_iso_zip_cue_audio_track_path(zip, 1, payload,
                                               (int)sizeof(payload)) == 0 ||
            payload[0] != '\0') {
            fputs("FAIL: ZIP MODE1 data track was promoted as CDDA\n", stderr);
            return 1;
        }
        memset(&sound, 0, sizeof(sound));
        if (nexus_sound_init(&sound) != 0) return 1;
        nexus_sound_set_cue_path(&sound, zip);
        if (nexus_sound_cd_track(&sound, 2) != 0 || !sound.cd_source_bound ||
            !strstr(sound.cd_source_path, "::Dungeon Master Nexus (Japan) (Track 2).bin") ||
            sound.cd_track_path[0] != '\0' || sound.cd_playing != 0) {
            fputs("FAIL: ZIP CUE track was not retained by native no-playback consumer\n",
                  stderr);
            nexus_sound_shutdown(&sound);
            return 1;
        }
        nexus_sound_shutdown(&sound);
    }
    puts("PASS: authentic Nexus CUE and ZIP CUE bind tracks 2-9 to original raw BIN media");
    return 0;
}
