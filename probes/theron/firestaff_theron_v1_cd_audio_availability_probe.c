#include "theron_v1_cd_audio_availability.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(_WIN32)
#include <direct.h>
#include <io.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

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

static int file_exists(const char *path) {
    FILE *file = fopen(path, "rb");
    if (file) fclose(file);
    return file != NULL;
}

static int run_synthetic_tests(void) {
    char directory[] = "/tmp/firestaff_theron_cd_audio_XXXXXX";
    char cue_path[1024];
    char track_path[1024];
    Theron_V1CdAudioReceipt receipt;
    int failed = 0;
    size_t i;

    if (!mkdtemp(directory)) return 1;

    /* Synthetic complete canonical layout with .wav files. */
    snprintf(cue_path, sizeof(cue_path), "%s/complete.cue", directory);
    {
        FILE *cue = fopen(cue_path, "wb");
        if (!cue) { failed = 1; }
        if (!failed) {
            fprintf(cue,
                "FILE \"track01.wav\" WAVE\n"
                "  TRACK 01 AUDIO\n"
                "    INDEX 01 00:00:00\n"
                "FILE \"track02.iso\" BINARY\n"
                "  TRACK 02 MODE1/2048\n"
                "    INDEX 01 00:00:00\n");
            for (i = 3u; i <= 18u; ++i) {
                fprintf(cue,
                    "FILE \"track%02zu.wav\" WAVE\n"
                    "  TRACK %02zu AUDIO\n"
                    "    INDEX 01 00:00:00\n", i, i);
            }
            fprintf(cue,
                "FILE \"track19.iso\" BINARY\n"
                "  TRACK 19 MODE1/2048\n"
                "    INDEX 01 00:00:00\n");
            fclose(cue);
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
            !receipt.track_present[1] ||
            !receipt.track_present[2] ||
            !receipt.track_present[19]) {
            printf("theron_v1_cd_audio_availability: FAIL complete synthetic test "
                   "(availability=%d, playback=%d, tracks=%u, audio=%u, data=%u)\n",
                   (int)receipt.availability, receipt.playback_allowed,
                   receipt.track_count, receipt.audio_track_count,
                   receipt.data_track_count);
            failed = 1;
        }
    }

    /* .ogg fallback: CUE declares .wav but only .ogg files exist. */
    if (!failed) {
        snprintf(cue_path, sizeof(cue_path), "%s/ogg_fallback.cue", directory);
        {
            FILE *cue = fopen(cue_path, "wb");
            if (!cue) { failed = 1; }
            else {
                fprintf(cue,
                    "FILE \"audio01.wav\" WAVE\n"
                    "  TRACK 01 AUDIO\n"
                    "    INDEX 01 00:00:00\n"
                    "FILE \"data02.iso\" BINARY\n"
                    "  TRACK 02 MODE1/2048\n"
                    "    INDEX 01 00:00:00\n");
                for (i = 3u; i <= 18u; ++i) {
                    fprintf(cue,
                        "FILE \"audio%02zu.wav\" WAVE\n"
                        "  TRACK %02zu AUDIO\n"
                        "    INDEX 01 00:00:00\n", i, i);
                }
                fprintf(cue,
                    "FILE \"data19.iso\" BINARY\n"
                    "  TRACK 19 MODE1/2048\n"
                    "    INDEX 01 00:00:00\n");
                fclose(cue);
            }
        }
        for (i = 1u; i <= 18u && !failed; ++i) {
            if (i == 2u) continue;
            snprintf(track_path, sizeof(track_path), "%s/audio%02zu.ogg",
                     directory, i);
            if (!write_file(track_path, "ogg", 3u)) failed = 1;
        }
        if (!failed) {
            snprintf(track_path, sizeof(track_path), "%s/data02.iso", directory);
            if (!write_file(track_path, "iso", 3u)) failed = 1;
            snprintf(track_path, sizeof(track_path), "%s/data19.iso", directory);
            if (!write_file(track_path, "iso", 3u)) failed = 1;
        }
        receipt = theron_v1_cd_audio_availability(cue_path, directory);
        if (receipt.availability != THERON_V1_CD_AUDIO_READY ||
            !receipt.playback_allowed ||
            !receipt.track_present[1] ||
            strcmp(receipt.track_paths[1] + strlen(receipt.track_paths[1]) - 4u,
                   ".ogg") != 0) {
            printf("theron_v1_cd_audio_availability: FAIL ogg fallback test "
                   "(availability=%d, playback=%d, present1=%d, path1=%s)\n",
                   (int)receipt.availability, receipt.playback_allowed,
                   receipt.track_present[1], receipt.track_paths[1]);
            failed = 1;
        }
    }

    /* Missing audio track file: canonical layout declared, one file absent. */
    if (!failed) {
        snprintf(cue_path, sizeof(cue_path), "%s/missing.cue", directory);
        {
            FILE *cue = fopen(cue_path, "wb");
            if (!cue) { failed = 1; }
            else {
                fprintf(cue,
                    "FILE \"track01.wav\" WAVE\n"
                    "  TRACK 01 AUDIO\n"
                    "    INDEX 01 00:00:00\n"
                    "FILE \"track02.iso\" BINARY\n"
                    "  TRACK 02 MODE1/2048\n"
                    "    INDEX 01 00:00:00\n");
                for (i = 3u; i <= 18u; ++i) {
                    fprintf(cue,
                        "FILE \"track%02zu.wav\" WAVE\n"
                        "  TRACK %02zu AUDIO\n"
                        "    INDEX 01 00:00:00\n", i, i);
                }
                fprintf(cue,
                    "FILE \"track19.iso\" BINARY\n"
                    "  TRACK 19 MODE1/2048\n"
                    "    INDEX 01 00:00:00\n");
                fclose(cue);
            }
        }
        snprintf(track_path, sizeof(track_path), "%s/track01.wav", directory);
        if (!failed && !write_file(track_path, "wav", 3u)) failed = 1;
        snprintf(track_path, sizeof(track_path), "%s/track02.iso", directory);
        if (!failed && !write_file(track_path, "iso", 3u)) failed = 1;
        /* Ensure Track 03 from the complete test is gone so it is missing. */
        snprintf(track_path, sizeof(track_path), "%s/track03.wav", directory);
        remove(track_path);
        /* Track 03 intentionally missing; layout is still canonical. */
        receipt = theron_v1_cd_audio_availability(cue_path, directory);
        if (receipt.availability != THERON_V1_CD_AUDIO_TRACK_FILE_MISSING ||
            receipt.playback_allowed) {
            printf("theron_v1_cd_audio_availability: FAIL missing-track test "
                   "(availability=%d, playback=%d)\n",
                   (int)receipt.availability, receipt.playback_allowed);
            failed = 1;
        }
    }

    /* Layout mismatch: missing data track. */
    if (!failed) {
        snprintf(cue_path, sizeof(cue_path), "%s/layout.cue", directory);
        {
            FILE *cue = fopen(cue_path, "wb");
            if (!cue) { failed = 1; }
            else {
                fprintf(cue,
                    "FILE \"track01.wav\" WAVE\n"
                    "  TRACK 01 AUDIO\n"
                    "    INDEX 01 00:00:00\n"
                    "FILE \"track02.iso\" BINARY\n"
                    "  TRACK 02 MODE1/2048\n"
                    "    INDEX 01 00:00:00\n");
                fclose(cue);
            }
        }
        receipt = theron_v1_cd_audio_availability(cue_path, directory);
        if (receipt.availability != THERON_V1_CD_AUDIO_LAYOUT_MISMATCH ||
            receipt.playback_allowed) {
            printf("theron_v1_cd_audio_availability: FAIL layout-mismatch test "
                   "(availability=%d, playback=%d)\n",
                   (int)receipt.availability, receipt.playback_allowed);
            failed = 1;
        }
    }

    /* Cleanup. */
    for (i = 1u; i <= 18u; ++i) {
        if (i == 2u) continue;
        snprintf(track_path, sizeof(track_path), "%s/track%02zu.wav", directory, i);
        remove(track_path);
        snprintf(track_path, sizeof(track_path), "%s/track%02zu.ogg", directory, i);
        remove(track_path);
        snprintf(track_path, sizeof(track_path), "%s/audio%02zu.ogg", directory, i);
        remove(track_path);
    }
    {
        snprintf(track_path, sizeof(track_path), "%s/track02.iso", directory);
        remove(track_path);
        snprintf(track_path, sizeof(track_path), "%s/track19.iso", directory);
        remove(track_path);
        snprintf(track_path, sizeof(track_path), "%s/data02.iso", directory);
        remove(track_path);
        snprintf(track_path, sizeof(track_path), "%s/data19.iso", directory);
        remove(track_path);
        snprintf(track_path, sizeof(track_path), "%s/complete.cue", directory);
        remove(track_path);
        snprintf(track_path, sizeof(track_path), "%s/ogg_fallback.cue", directory);
        remove(track_path);
        snprintf(track_path, sizeof(track_path), "%s/missing.cue", directory);
        remove(track_path);
        snprintf(track_path, sizeof(track_path), "%s/layout.cue", directory);
        remove(track_path);
    }
    rmdir(directory);

    return failed;
}

static int run_real_data_test(void) {
    const char *home = getenv("HOME");
    char cue_path[1024];
    Theron_V1CdAudioReceipt receipt;
    if (!home) return 0; /* Skip if no HOME. */
    snprintf(cue_path, sizeof(cue_path),
             "%s/.firestaff/data/theron/TQUS.cue", home);
    if (!file_exists(cue_path)) {
        printf("theron_v1_cd_audio_availability: SKIP real-data test "
               "(%s not found)\n", cue_path);
        return 0;
    }
    receipt = theron_v1_cd_audio_availability(cue_path, NULL);
    if (receipt.availability != THERON_V1_CD_AUDIO_READY ||
        !receipt.playback_allowed) {
        printf("theron_v1_cd_audio_availability: FAIL real-data test "
               "(availability=%d, playback_allowed=%d, reason=%s)\n",
               (int)receipt.availability, receipt.playback_allowed,
               receipt.unavailable_reason);
        return 1;
    }
    printf("theron_v1_cd_audio_availability: PASS real-data test "
           "(19 tracks, %u audio, %u data)\n",
           receipt.audio_track_count, receipt.data_track_count);
    return 0;
}

int main(void) {
    int failed = 0;
    failed |= run_synthetic_tests();
    failed |= run_real_data_test();
    return failed ? 1 : 0;
}
