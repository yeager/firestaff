#include <stdio.h>
#include <string.h>

#include "theron_v1_track02_mednafen_trace_converter.h"
#include "theron_v1_track02_raw_media_intake.h"

int main(int argc, char **argv) {
    Theron_V1Track02RawMediaIntakeReceipt media;
    Theron_V1Track02MednafenTraceConvertRequest trace_request;
    Theron_V1Track02MednafenTraceConvertReceipt trace_receipt;

    if (argc == 1) {
        printf("SKIP: explicit CUE, Track 02 MD5, Mednafen export, export MD5, and event-log path required\n");
        return 0;
    }
    if (argc != 7 || strcmp(argv[1], "--bind")) {
        fprintf(stderr,
                "usage: %s --bind <cue> <track02-md5> <export> <export-md5> <event-log>\n",
                argv[0]);
        return 2;
    }
    if (!theron_v1_track02_raw_media_intake_discover(argv[2], &media) ||
        media.status != THERON_V1_TRACK02_MEDIA_INTAKE_READY ||
        !media.raw_trace_preparation_allowed || strcmp(media.track02_md5, argv[3])) {
        fprintf(stderr, "REJECTED: Track 02 CUE/MD5 is not a raw trace source\n");
        return 1;
    }
    trace_request.source_trace_path = argv[4];
    trace_request.expected_source_trace_md5 = argv[5];
    trace_request.event_log_path = argv[6];
    if (!theron_v1_track02_mednafen_trace_convert_file(&trace_request,
                                                        &trace_receipt)) {
        fprintf(stderr, "REJECTED: Mednafen export/MD5/event-log request mismatch\n");
        return 1;
    }
    if (trace_receipt.status == THERON_V1_TRACK02_MEDNAFEN_TRACE_UNAVAILABLE) {
        printf("SKIP: external Mednafen export unavailable\n");
        return 0;
    }
    if (trace_receipt.status != THERON_V1_TRACK02_MEDNAFEN_TRACE_CONVERTED) {
        fprintf(stderr, "REJECTED: Mednafen export/MD5/event-log request mismatch\n");
        return 1;
    }
    printf("READY: raw Track 02 and strict HuC6280 evidence verified; emulator not launched\n");
    return 0;
}
