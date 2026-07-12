/*
 * Opt-in raw Track 02 availability probe.  It reads only explicitly supplied
 * environment paths and reports redacted availability with the startup
 * bitmap-executor boundary.  It never builds or executes a fallback plan.
 *
 * Optional inputs:
 *   THERON_RAW_TRACK02=/absolute/path/to/authentic-track02.bin
 *   THERON_TRACK02_END=/absolute/path/to/authentic-02End.iso
 */
#include "asset_status_m12.h"
#include "theron_v1_iso_end_receipt.h"
#include "theron_v1_track02.h"
#include "theron_v1_track_media_availability.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static const char *availability_name(Theron_V1TrackMediaAvailability value)
{
    switch (value) {
    case THERON_V1_TRACK_MEDIA_RAW_READY:
        return "raw_track02_ready";
    case THERON_V1_TRACK_MEDIA_END_VARIANT:
        return "raw_track02_end_variant";
    case THERON_V1_TRACK_MEDIA_MISSING:
    default:
        return "raw_track02_missing";
    }
}

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

    if (!file_md5(path, md5, &bytes) ||
        bytes % THERON_TRACK02_RAW_SECTOR_BYTES != 0u) {
        return 0;
    }
    return strcmp(md5, THERON_TRACK02_MD5_JP_BIN) == 0 ||
           strcmp(md5, THERON_TRACK02_MD5_US_BIN) == 0;
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
    span.bytes = bytes;
    return theron_v1_iso_end_receipt(md5, bytes, &span, 1u, out);
}

static int selftest(void)
{
    Theron_V1IsoEndReceipt end = {1, 1, 0u, 1u, "opaque-end"};
    Theron_V1_TrackMediaAvailabilityReceipt missing;
    Theron_V1_TrackMediaAvailabilityReceipt end_variant;
    Theron_V1_TrackMediaAvailabilityReceipt raw;

    missing = theron_v1_track_media_availability("raw_track02_missing", NULL);
    end_variant = theron_v1_track_media_availability("raw_track02_missing", &end);
    raw = theron_v1_track_media_availability("raw_track02_ready", &end);
    return missing.availability == THERON_V1_TRACK_MEDIA_MISSING &&
           !missing.loader_usable &&
           end_variant.availability == THERON_V1_TRACK_MEDIA_END_VARIANT &&
           !end_variant.loader_usable &&
           raw.availability == THERON_V1_TRACK_MEDIA_RAW_READY &&
           raw.loader_usable;
}

int main(int argc, char **argv)
{
    const char *raw_path;
    const char *end_path;
    const char *raw_status;
    Theron_V1IsoEndReceipt end;
    Theron_V1_TrackMediaAvailabilityReceipt availability;
    int have_end;

    if (argc == 2 && strcmp(argv[1], "--selftest") == 0) {
        return selftest() ? 0 : 1;
    }
    if (argc != 1) {
        fprintf(stderr, "usage: %s [--selftest]\n", argv[0]);
        return 2;
    }

    raw_path = getenv("THERON_RAW_TRACK02");
    end_path = getenv("THERON_TRACK02_END");
    raw_status = raw_track02_ready(raw_path) ? "raw_track02_ready" :
                                               "raw_track02_missing";
    memset(&end, 0, sizeof(end));
    have_end = end_variant_receipt(end_path, &end);
    availability = theron_v1_track_media_availability(
        raw_status, have_end ? &end : NULL);

    printf("raw_track02=%s title_gate=RAW_TRACK02_BITMAP_REQUIRED "
           "stage_gate=RAW_TRACK02_BITMAP_REQUIRED "
           "soul_room_gate=RAW_TRACK02_BITMAP_REQUIRED "
           "fallback=never_run\n",
           availability_name(availability.availability));
    return 0;
}
