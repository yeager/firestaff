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
    char directory[] = "firestaff_theron_cd_audio_unit_XXXXXX";
    char cue_path[1024];
    char track_path[1024];
    Theron_V1CdAudioReceipt receipt;
    int failed = 0;
    size_t i;
    const char *real_cue;

    /* Optional integration check against an authentic dump.  This branch
     * never creates or substitutes media: callers must provide the real CUE
     * and its sibling files explicitly. */
    real_cue = getenv("FIRESTAFF_THERON_REAL_CD_CUE");
    if (real_cue && real_cue[0]) {
        receipt = theron_v1_cd_audio_availability(real_cue, NULL);
        if (receipt.availability != THERON_V1_CD_AUDIO_READY ||
            !receipt.playback_allowed || receipt.track_count != 19u ||
            receipt.audio_track_count != 17u ||
            receipt.data_track_count != 2u) {
            fprintf(stderr,
                    "real Theron CD receipt failed: status=%d tracks=%u "
                    "audio=%u data=%u reason=%s\n",
                    (int)receipt.availability, receipt.track_count,
                    receipt.audio_track_count, receipt.data_track_count,
                    receipt.unavailable_reason);
            return 1;
        }
        for (i = 1u; i <= THERON_V1_CD_AUDIO_TRACK_COUNT; ++i) {
            int expected_audio = i == 1u || (i >= 3u && i <= 18u);
            if (!receipt.track_present[i] ||
                receipt.track_is_audio[i] != expected_audio) {
                fprintf(stderr,
                        "real Theron CD track %zu failed: present=%d audio=%d\n",
                        i, receipt.track_present[i],
                        receipt.track_is_audio[i]);
                return 1;
            }
        }
        printf("PASS: authentic Theron CUE resolves all 19 original tracks "
               "(17 audio, 2 data)\n");
    }

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

    /* Raw CDDA BINs may not claim availability with a partial sector. */
    if (!failed) {
        char raw_cue[1024];
        char raw_audio[1024];
        unsigned char partial_sector[THERON_TRACK01_CDDA_SECTOR_BYTES - 1u] = {0};
        FILE *cue;
        snprintf(raw_cue, sizeof(raw_cue), "%s/raw-audio.cue", directory);
        snprintf(raw_audio, sizeof(raw_audio), "%s/raw-track01.bin", directory);
        cue = fopen(raw_cue, "wb");
        if (!cue) { failed = 1; }
        else {
            fprintf(cue,
                "FILE raw-track01.bin BINARY\n"
                "  TRACK 01 AUDIO\n"
                "FILE track02.iso BINARY\n"
                "  TRACK 02 MODE1/2048\n");
            for (i = 3u; i <= 18u; ++i) {
                fprintf(cue,
                    "FILE track%02zu.wav WAVE\n"
                    "  TRACK %02zu AUDIO\n", i, i);
            }
            fprintf(cue,
                "FILE track19.iso BINARY\n"
                "  TRACK 19 MODE1/2048\n");
            fclose(cue);
        }
        if (!failed &&
            !write_file(raw_audio, partial_sector, sizeof(partial_sector))) {
            failed = 1;
        }
        if (!failed) {
            receipt = theron_v1_cd_audio_availability(raw_cue, directory);
            if (receipt.availability != THERON_V1_CD_AUDIO_TRACK_FILE_MISSING ||
                receipt.playback_allowed || receipt.track_present[1]) {
                failed = 1;
            }
        }
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
        FILE *cue;
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

    /* A duplicate track number must never authorize an otherwise readable
     * disc.  All media below is only parser input; the authentic-media
     * branch above remains the source of real Theron disc evidence. */
    if (!failed) {
        char duplicate_cue[1024];
        FILE *cue;
        snprintf(duplicate_cue, sizeof(duplicate_cue),
                 "%s/duplicate.cue", directory);
        cue = fopen(duplicate_cue, "wb");
        if (!cue) { failed = 1; }
        else {
            fprintf(cue,
                "FILE track01.wav WAVE\n"
                "  TRACK 01 AUDIO\n"
                "FILE track02.iso BINARY\n"
                "  TRACK 02 MODE1/2048\n");
            for (i = 3u; i <= 18u; ++i) {
                fprintf(cue,
                    "FILE track%02zu.wav WAVE\n"
                    "  TRACK %02zu AUDIO\n", i, i);
            }
            fprintf(cue,
                "FILE track19.iso BINARY\n"
                "  TRACK 19 MODE1/2048\n"
                "FILE duplicate-track18.wav WAVE\n"
                "  TRACK 18 AUDIO\n");
            fclose(cue);
        }
        if (!failed) {
            receipt = theron_v1_cd_audio_availability(duplicate_cue,
                                                      directory);
            if (receipt.availability != THERON_V1_CD_AUDIO_CUE_PARSE_ERROR ||
                receipt.playback_allowed) {
                failed = 1;
            }
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
    snprintf(track_path, sizeof(track_path), "%s/duplicate.cue", directory);
    remove(track_path);
    snprintf(track_path, sizeof(track_path), "%s/raw-audio.cue", directory);
    remove(track_path);
    snprintf(track_path, sizeof(track_path), "%s/raw-track01.bin", directory);
    remove(track_path);
    rmdir(directory);

    printf("test_theron_v1_cd_audio_availability: %s\n", failed ? "FAIL" : "PASS");
    return failed ? 1 : 0;
#endif
}
