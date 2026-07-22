/*
 * Opt-in capture preflight only.  This never starts Mednafen: it reports
 * whether an explicit raw Track 02 and System Card 3.0 satisfy the capture
 * boundary.  02End containers remain opaque and cannot permit capture.
 *
 * Optional inputs:
 *   THERON_RAW_TRACK02=/absolute/path/to/authentic-track02.bin
 *   THERON_TRACK02_END=/absolute/path/to/authentic-02End.iso
 *   THERON_SYSTEM_CARD=/absolute/path/to/syscard3.pce
 */
#include "asset_status_m12.h"
#include "theron_v1_iso_end_receipt.h"
#include "theron_v1_track02.h"
#include "theron_v1_track_media_availability.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define THERON_SYSCARD3_MD5 "ff1a674273fe3540ccef576376407d1d"
#define THERON_SYSCARD3_BYTES 0x40200u

typedef enum {
    THERON_CAPTURE_SYSTEM_CARD_MISSING = 0,
    THERON_CAPTURE_SYSTEM_CARD_HASH_MISMATCH,
    THERON_CAPTURE_SYSTEM_CARD_READY
} Theron_CaptureSystemCardStatus;

static int file_md5(const char *path, char md5[33], size_t *out_bytes)
{
    struct stat info;

    if (!path || !path[0] || !out_bytes || stat(path, &info) != 0 ||
        info.st_size <= 0 || !m12_file_md5_hex(path, md5)) {
        return 0;
    }
    *out_bytes = (size_t)info.st_size;
    return 1;
}

static int raw_track02_ready(const char *path)
{
    char md5[33];
    size_t bytes;

    return file_md5(path, md5, &bytes) &&
           bytes % THERON_TRACK02_RAW_SECTOR_BYTES == 0u &&
           (strcmp(md5, THERON_TRACK02_MD5_JP_BIN) == 0 ||
            strcmp(md5, THERON_TRACK02_MD5_US_BIN) == 0);
}

static int end_variant_receipt(const char *path, Theron_V1IsoEndReceipt *out)
{
    char md5[33];
    size_t bytes;
    Theron_V1IsoEndSpan span;

    if (!out || !file_md5(path, md5, &bytes)) {
        return 0;
    }
    span.offset = 0u;
    span.byte_count = bytes;
    return theron_v1_iso_end_receipt(md5, bytes, &span, 1u, out);
}

static Theron_CaptureSystemCardStatus system_card_status(const char *path)
{
    char md5[33];
    size_t bytes;

    if (!path || !path[0]) {
        return THERON_CAPTURE_SYSTEM_CARD_MISSING;
    }
    if (!file_md5(path, md5, &bytes) || bytes != THERON_SYSCARD3_BYTES ||
        strcmp(md5, THERON_SYSCARD3_MD5) != 0) {
        return THERON_CAPTURE_SYSTEM_CARD_HASH_MISMATCH;
    }
    return THERON_CAPTURE_SYSTEM_CARD_READY;
}

static const char *media_name(Theron_V1TrackMediaAvailability media)
{
    switch (media) {
    case THERON_V1_TRACK_MEDIA_RAW_READY:
        return "raw_track02_ready";
    case THERON_V1_TRACK_MEDIA_END_VARIANT:
        return "raw_track02_end_variant";
    case THERON_V1_TRACK_MEDIA_MISSING:
    default:
        return "raw_track02_missing";
    }
}

static const char *system_card_name(Theron_CaptureSystemCardStatus status)
{
    switch (status) {
    case THERON_CAPTURE_SYSTEM_CARD_READY:
        return "ready";
    case THERON_CAPTURE_SYSTEM_CARD_HASH_MISMATCH:
        return "hash_mismatch";
    case THERON_CAPTURE_SYSTEM_CARD_MISSING:
    default:
        return "missing";
    }
}

static int capture_permitted(Theron_V1TrackMediaAvailability media,
                             Theron_CaptureSystemCardStatus system_card)
{
    return media == THERON_V1_TRACK_MEDIA_RAW_READY &&
           system_card == THERON_CAPTURE_SYSTEM_CARD_READY;
}

static int selftest(void)
{
    Theron_V1IsoEndReceipt end = {1, 1, 0, 1, 0};
    Theron_V1_TrackMediaAvailabilityReceipt missing;
    Theron_V1_TrackMediaAvailabilityReceipt end_variant;
    Theron_V1_TrackMediaAvailabilityReceipt raw;

    missing = theron_v1_track_media_availability("raw_track02_missing", NULL);
    end_variant = theron_v1_track_media_availability("raw_track02_missing", &end);
    raw = theron_v1_track_media_availability("raw_track02_ready", &end);
    return !capture_permitted(missing.availability,
                              THERON_CAPTURE_SYSTEM_CARD_READY) &&
           !capture_permitted(end_variant.availability,
                              THERON_CAPTURE_SYSTEM_CARD_READY) &&
           !capture_permitted(raw.availability,
                              THERON_CAPTURE_SYSTEM_CARD_HASH_MISMATCH) &&
           capture_permitted(raw.availability,
                             THERON_CAPTURE_SYSTEM_CARD_READY);
}

int main(int argc, char **argv)
{
    Theron_V1IsoEndReceipt end;
    Theron_V1_TrackMediaAvailabilityReceipt media;
    Theron_CaptureSystemCardStatus system_card;
    int permitted;

    if (argc == 2 && strcmp(argv[1], "--selftest") == 0) {
        return selftest() ? 0 : 1;
    }
    if (argc != 1) {
        fprintf(stderr, "usage: %s [--selftest]\n", argv[0]);
        return 2;
    }

    memset(&end, 0, sizeof(end));
    media = theron_v1_track_media_availability(
        raw_track02_ready(getenv("THERON_RAW_TRACK02"))
            ? "raw_track02_ready"
            : "raw_track02_missing",
        end_variant_receipt(getenv("THERON_TRACK02_END"), &end) ? &end : NULL);
    system_card = system_card_status(getenv("THERON_SYSTEM_CARD"));
    permitted = capture_permitted(media.availability, system_card);
    printf("raw_track02=%s system_card=%s mednafen_launch=%s emulator=not_launched\n",
           media_name(media.availability), system_card_name(system_card),
           permitted ? "permitted_not_started" : "blocked_not_started");
    return 0;
}
