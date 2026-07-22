#include "theron_v1_track02.h"

#include <stdio.h>
#include <string.h>

#if !defined(_WIN32)
#include <stdlib.h>
#include <unistd.h>
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

int main(void) {
#if defined(_WIN32)
    printf("test_theron_v1_track01_cdda_handoff: SKIP (fixture path)\n");
    return 0;
#else
    char directory[] = "/tmp/firestaff_theron_track01_XXXXXX";
    char cue[512];
    char audio[512];
    char data[512];
    unsigned char sectors[THERON_TRACK01_CDDA_SECTOR_BYTES * 3u] = {0};
    Theron_Track01CddaHandoff handoff;
    Theron_Track01CddaStream stream = {0};
    int failed = 0;
    if (!mkdtemp(directory)) return 1;
    snprintf(cue, sizeof(cue), "%s/original.cue", directory);
    snprintf(audio, sizeof(audio), "%s/track01.bin", directory);
    snprintf(data, sizeof(data), "%s/track02.bin", directory);
    sectors[0] = 0x34u;
    sectors[THERON_TRACK01_CDDA_SECTOR_BYTES] = 0x56u;
    sectors[THERON_TRACK01_CDDA_SECTOR_BYTES * 2u] = 0x78u;
    failed |= !write_file(audio, sectors, sizeof(sectors));
    failed |= !write_file(data, "fixture verified data bytes", strlen("fixture verified data bytes"));
    failed |= !write_file(cue,
        "FILE \"track01.bin\" BINARY\n"
        "  TRACK 01 AUDIO\n"
        "    INDEX 01 00:00:01\n"
        "FILE \"track02.bin\" BINARY\n"
        "  TRACK 02 MODE1/2352\n"
        "    INDEX 01 00:00:00\n",
        strlen("FILE \"track01.bin\" BINARY\n"
               "  TRACK 01 AUDIO\n"
               "    INDEX 01 00:00:01\n"
               "FILE \"track02.bin\" BINARY\n"
               "  TRACK 02 MODE1/2352\n"
               "    INDEX 01 00:00:00\n"));
    if (!failed && theron_v1_track01_cdda_handoff_from_verified_media(
            cue, THERON_TRACK02_MD5_US_BIN, &handoff) != THERON_TRACK01_CDDA_AVAILABLE) {
        failed = 1;
    }
    if (!failed && (!handoff.playback_handoff_ready || !handoff.original_cdda ||
                    handoff.index_lba != 1u || handoff.audio_sector_count != 2u ||
                    handoff.audio_start_byte != THERON_TRACK01_CDDA_SECTOR_BYTES ||
                    strcmp(handoff.audio_path, audio) != 0 ||
                    strcmp(handoff.track02_path, data) != 0)) {
        failed = 1;
    }
    /* The two-sector fixture is shorter than the bounded queue.  Filling the
     * queue must wrap only to its CUE-derived Track 01 start, never beyond it.
     * The queue bound is THERON_TRACK01_CDDA_MAX_QUEUED_SECTORS (16 since
     * 2026-07-14): 2 initial sectors + 7 wrap loops of 2 = 16 queued. */
    if (!failed && (!theron_v1_track01_cdda_lifecycle_update(&handoff, 1, &stream) ||
                    stream.sectors_read != 2u ||
                    stream.sectors_queued != THERON_TRACK01_CDDA_MAX_QUEUED_SECTORS ||
                    stream.loop_count != (THERON_TRACK01_CDDA_MAX_QUEUED_SECTORS - 2u) / 2u)) {
        failed = 1;
    }
    if (!failed && (!stream.output_started ||
                    !theron_v1_track01_cdda_lifecycle_update(&handoff, 0, &stream) ||
                    stream.output_started || stream.audio_file || stream.sdl_stream)) {
        failed = 1;
    }
    if (!failed && !write_file(audio, sectors, THERON_TRACK01_CDDA_SECTOR_BYTES - 1u)) {
        failed = 1;
    }
    if (!failed && theron_v1_track01_cdda_handoff_from_verified_media(
            cue, THERON_TRACK02_MD5_US_BIN, &handoff) != THERON_TRACK01_CDDA_UNAVAILABLE) {
        failed = 1;
    }
    if (!failed && theron_v1_track01_cdda_handoff_from_verified_media(
            data, THERON_TRACK02_MD5_US_BIN, &handoff) != THERON_TRACK01_CDDA_UNAVAILABLE) {
        failed = 1;
    }
    if (!failed && theron_v1_track01_cdda_handoff_from_verified_media(
            cue, "00000000000000000000000000000000", &handoff) != THERON_TRACK01_CDDA_UNVERIFIED) {
        failed = 1;
    }
    if (!failed && (theron_v1_track01_cdda_lifecycle_update(&handoff, 1, &stream) ||
                    stream.output_started || stream.audio_file || stream.sdl_stream)) {
        failed = 1;
    }
    remove(cue);
    remove(audio);
    remove(data);
    rmdir(directory);
    printf("test_theron_v1_track01_cdda_handoff: %s\n", failed ? "FAIL" : "PASS");
    return failed;
#endif
}
