#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "theron_v1_raw_loader_trace.h"
#include "theron_v1_track02_raw_media_intake.h"

int main(int argc, char **argv)
{
    Theron_V1Track02RawMediaIntakeReceipt media;
    Theron_V1RawLoaderTraceReceipt loader;

    if (argc == 1) {
        puts("SKIP: explicit CUE, Track 02 MD5, and dynamic CD_READ trace required");
        return 0;
    }
    if (argc != 5 || strcmp(argv[1], "--inspect")) {
        fprintf(stderr, "usage: %s --inspect <cue> <track02-md5> <dynamic-trace>\n", argv[0]);
        return 2;
    }
    if (!theron_v1_track02_raw_media_intake_discover(argv[2], &media) ||
        media.status == THERON_V1_TRACK02_MEDIA_INTAKE_UNAVAILABLE) {
        puts("SKIP: Track 02 media unavailable");
        return 0;
    }
    if (media.status != THERON_V1_TRACK02_MEDIA_INTAKE_READY ||
        !media.raw_trace_preparation_allowed || strcmp(media.track02_md5, argv[3])) {
        fputs("REJECTED: unauthenticated Track 02 input\n", stderr);
        return 1;
    }
    {
        struct stat trace_status;
        if (stat(argv[4], &trace_status) != 0) {
            puts("SKIP: external dynamic CD_READ trace unavailable");
            return 0;
        }
    }
    if (!theron_v1_raw_loader_trace_import_mednafen_capture_file(argv[4], argv[3], &loader)) {
        fputs("REJECTED: dynamic CD_READ trace is absent, malformed, or untrusted\n", stderr);
        return 1;
    }
    puts("SKIP: verified media and dynamic CD_READ observed; external ordered palette, bitmap, and destination-record receipts still required");
    return 0;
}
