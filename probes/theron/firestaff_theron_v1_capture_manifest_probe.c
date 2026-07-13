#include "theron_v1_capture_manifest.h"

#include <stdio.h>

int main(int argc, char **argv)
{
    Theron_V1CaptureManifest manifest;
    Theron_V1CaptureManifest copy;
    char text[1024];
    const char *valid =
        "THERON_CAPTURE_MANIFEST_V2\n"
        "track02_path=/raw\n"
        "track02_md5=12345678901234567890123456789012\n"
        "system_card_path=/card\n"
        "system_card_md5=12345678901234567890123456789012\n"
        "loader_trace_path=/trace\n"
        "loader_trace_md5=12345678901234567890123456789012";

    if (argc == 7) {
        snprintf(manifest.track02_path, sizeof(manifest.track02_path), "%s",
                 argv[1]);
        snprintf(manifest.track02_md5, sizeof(manifest.track02_md5), "%s",
                 argv[2]);
        snprintf(manifest.system_card_path, sizeof(manifest.system_card_path),
                 "%s", argv[3]);
        snprintf(manifest.system_card_md5, sizeof(manifest.system_card_md5),
                 "%s", argv[4]);
        snprintf(manifest.trace_path, sizeof(manifest.trace_path), "%s",
                 argv[5]);
        snprintf(manifest.trace_md5, sizeof(manifest.trace_md5), "%s",
                 argv[6]);
        manifest.valid = 1;
        return theron_v1_capture_manifest_write(&manifest, text,
                                                sizeof(text)) &&
                       theron_v1_capture_manifest_parse(text, &copy) &&
                       theron_v1_capture_manifest_matches_preflight_inputs(
                           &copy, argv[1], argv[2], argv[3], argv[4], argv[5],
                           argv[6])
                   ? 0 : 1;
    }

    return theron_v1_capture_manifest_parse(valid, &manifest) &&
                   theron_v1_capture_manifest_write(&manifest, text,
                                                    sizeof(text)) &&
                   theron_v1_capture_manifest_parse(text, &copy) &&
                   theron_v1_capture_manifest_matches_preflight_inputs(
                       &copy, "/raw", "12345678901234567890123456789012",
                       "/card", "12345678901234567890123456789012",
                       "/trace", "12345678901234567890123456789012")
               ? 0 : 1;
}
