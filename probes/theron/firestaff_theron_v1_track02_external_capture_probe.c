#include <stdio.h>
#include <string.h>

#include "theron_v1_track02_external_capture_launcher.h"

int main(int argc, char **argv) {
    Theron_V1Track02ExternalCaptureRequest request;
    Theron_V1Track02ExternalCaptureReceipt receipt;

    if (argc == 1) {
        printf("SKIP: explicit emulator, CUE/BIN, MD5, and manifest path required\n");
        return 0;
    }
    if (argc != 6 || strcmp(argv[1], "--prepare")) {
        fprintf(stderr,
                "usage: %s --prepare <emulator> <cue-or-bin> <expected-md5> <manifest>\n",
                argv[0]);
        return 2;
    }
    request.emulator_path = argv[2];
    request.media_path = argv[3];
    request.expected_track02_md5 = argv[4];
    request.manifest_path = argv[5];
    if (!theron_v1_track02_external_capture_write_skeleton(&request, &receipt)) {
        fprintf(stderr, "capture request rejected\n");
        return 1;
    }
    if (receipt.status == THERON_V1_TRACK02_EXTERNAL_CAPTURE_SKELETON_WRITTEN) {
        printf("READY: strict capture manifest skeleton written; emulator not launched\n");
        return 0;
    }
    if (receipt.status == THERON_V1_TRACK02_EXTERNAL_CAPTURE_UNAVAILABLE) {
        printf("SKIP: local Track 02 media unavailable\n");
        return 0;
    }
    fprintf(stderr, "REJECTED: emulator/media/MD5/manifest request mismatch\n");
    return 1;
}
