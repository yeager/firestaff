#include "theron_v1_cd_audio_availability.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int write_file(const char *path, const void *contents, size_t length) {
    FILE *file = fopen(path, "wb");
    if (!file) return 0;
    if (length && fwrite(contents, 1u, length, file) != length) {
        fclose(file);
        return 0;
    }
    fclose(file);
    return 1;
}

int main(void) {
#if defined(_WIN32)
    printf("test_theron_v1_cd_audio_availability: SKIP (fixture path)\n");
    return 0;
#else
    char directory[] = "/tmp/firestaff_theron_cd_audio_unit_XXXXXX";
    char cue_path[1024];
    char track_path[1024];
    Theron_V1CdAudioReceipt receipt;
    int failed = 0;
    size_t i;

    if (!mkdtemp(directory)) return 1;

    /* Canonical 19-track layout with .wav CD-DA files. */
    snprintf(cue_path, sizeof(cue_path), "%s/canonical.cue", directory);
    {
        FILE *cue = fopen(cue_path, "wb");
        if (!cue) { failed = 1; }
        else {
            fprintf(cue,
                "FILE track01.wav WAVE\n"
                "  TRACK 01 AUDIO\n"
                "    INDEX 01 00:00:00\n"
                "FILE track02.iso BINARY\n"
                "  TRACK 02 MODE1/2048\n"
                "    INDEX 01 00:00:00\n");
            for (i = 3u; i <= 18u; ++i) {
                fprintf(cue,
                    "FILE track%02zu.wav WAVE\n"
                    "  TRACK %02zu AUDIO\n"
                    "    INDEX 01 00:00:00\n", i, i);
            }
            fprintf(cue,
                "FILE track19.iso BINARY\n"
                "  TRACK 19 MODE1/2048\n"
                "    INDEX 01 00:00:00\n");
            fclose(cue);
        }
    }
    for (i = 1u; i <= 18u && !failed; ++i) {
        if (i == 2u) continue;
        snprintf(track_path, sizeof(track_path), "%s/track%02zu.wav",
                 directory, i);
        if (!write_file(track_path, "wav", 3u)) failed = 1;
    }
    if (!failed) {
        snprintf(track_path, sizeof(track_path), "%s/track02.iso", directory);
        if (!write_file(track_path, "iso", 3u)) failed = 1;
        snprintf(track_path, sizeof(track_path), "%s/track19.iso", directory);
        if (!write_file(track_path, "iso", 3u)) failed = 1;
    }

    receipt = theron_v1_cd_audio_availability(cue_path, directory);
    if (receipt.availability != THERON_V1_CD_AUDIO_READY ||
        !receipt.playback_allowed ||
        receipt.track_count != 19u ||
        receipt.audio_track_count != 17u ||
        receipt.data_track_count != 2u ||
        !receipt.track_is_audio[1] ||
        receipt.track_is_audio[2] ||
        !receipt.track_is_audio[18] ||
        receipt.track_is_audio[19] ||
        !receipt.track_present[1] ||
        !receipt.track_present[2] ||
        !receipt.track_present[18] ||
        !receipt.track_present[19]) {
        failed = 1;
    }

    /* CUE input rejection. */
    if (!failed) {
        receipt = theron_v1_cd_audio_availability(NULL, directory);
        if (receipt.availability != THERON_V1_CD_AUDIO_CUE_NOT_FOUND) {
            failed = 1;
        }
        receipt = theron_v1_cd_audio_availability("/nonexistent/path.cue",
                                                  directory);
        if (receipt.availability != THERON_V1_CD_AUDIO_CUE_NOT_FOUND) {
            failed = 1;
        }
    }

    /* Layout mismatch: only two tracks. */
    if (!failed) {
        char short_cue[1024];
        FILE *cue = fopen(short_cue, "wb");
        snprintf(short_cue, sizeof(short_cue), "%s/short.cue", directory);
        cue = fopen(short_cue, "wb");
        if (!cue) { failed = 1; }
        else {
            fprintf(cue,
                "FILE track01.wav WAVE\n"
                "  TRACK 01 AUDIO\n"
                "    INDEX 01 00:00:00\n"
                "FILE track02.iso BINARY\n"
                "  TRACK 02 MODE1/2048\n"
                "    INDEX 01 00:00:00\n");
            fclose(cue);
        }
        receipt = theron_v1_cd_audio_availability(short_cue, directory);
        if (receipt.availability != THERON_V1_CD_AUDIO_LAYOUT_MISMATCH ||
            receipt.playback_allowed) {
            failed = 1;
        }
    }

    /* Cleanup. */
    for (i = 1u; i <= 18u; ++i) {
        if (i == 2u) continue;
        snprintf(track_path, sizeof(track_path), "%s/track%02zu.wav",
                 directory, i);
        remove(track_path);
    }
    snprintf(track_path, sizeof(track_path), "%s/track02.iso", directory);
    remove(track_path);
    snprintf(track_path, sizeof(track_path), "%s/track19.iso", directory);
    remove(track_path);
    snprintf(track_path, sizeof(track_path), "%s/canonical.cue", directory);
    remove(track_path);
    snprintf(track_path, sizeof(track_path), "%s/short.cue", directory);
    remove(track_path);
    rmdir(directory);

    printf("test_theron_v1_cd_audio_availability: %s\n", failed ? "FAIL" : "PASS");
    return failed ? 1 : 0;
#endif
}
