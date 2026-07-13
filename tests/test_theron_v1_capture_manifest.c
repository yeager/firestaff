#include "theron_v1_capture_manifest.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *name) {
    if (!condition) {
        ++failures;
        printf("[FAIL] %s\n", name);
    }
}

int main(void) {
    static const char valid[] =
        "THERON_CAPTURE_MANIFEST_V1\n"
        "track02_path=/media/track02.bin\n"
        "track02_md5=f23601102138f87c33025877767ebf76\n"
        "system_card_path=/bios/syscard3.pce\n"
        "system_card_md5=ff1a674273fe3540ccef576376407d1d\n"
        "loader_trace_path=/captures/live.trace";
    Theron_V1CaptureManifest manifest;
    Theron_V1CaptureManifest copy;
    char written[1024];

    check(theron_v1_capture_manifest_parse(valid, &manifest),
          "accepts exact, lower-case MD5-bound manifest");
    check(theron_v1_capture_manifest_matches(
              &manifest, "/media/track02.bin",
              "f23601102138f87c33025877767ebf76", "/bios/syscard3.pce",
              "ff1a674273fe3540ccef576376407d1d", "/captures/live.trace"),
          "matches every bound capture input");
    check(theron_v1_capture_manifest_write(&manifest, written, sizeof(written)) &&
              strcmp(written, valid) == 0 &&
              theron_v1_capture_manifest_parse(written, &copy),
          "round trips canonical manifest text");
    check(!theron_v1_capture_manifest_parse(
              "THERON_CAPTURE_MANIFEST_V1\n"
              "track02_path=/media/track02.bin\n"
              "track02_md5=F23601102138F87C33025877767EBF76\n"
              "system_card_path=/bios/syscard3.pce\n"
              "system_card_md5=ff1a674273fe3540ccef576376407d1d\n"
              "loader_trace_path=/captures/live.trace", &manifest),
          "rejects non-canonical MD5 text");
    check(!theron_v1_capture_manifest_parse(
              "THERON_CAPTURE_MANIFEST_V1\n"
              "track02_path=/media/track02.bin\n"
              "track02_md5=f23601102138f87c33025877767ebf76\n"
              "system_card_path=/bios/syscard3.pce\n"
              "system_card_md5=ff1a674273fe3540ccef576376407d1d\n"
              "loader_trace_path=/captures/live.trace\nextra=forbidden", &manifest),
          "rejects unbound trailing fields");
    check(!theron_v1_capture_manifest_parse(
              "THERON_CAPTURE_MANIFEST_V1\n"
              "track02_path=/media/track02.bin\n"
              "track02_md5=f23601102138f87c33025877767ebf76\n"
              "system_card_path=/bios/syscard3.pce\n"
              "system_card_md5=ff1a674273fe3540ccef576376407d1d\n"
              "loader_trace_path=/captures/live\ttrace", &manifest),
          "rejects control bytes in capture paths");
    printf("theron capture manifest: %d failure(s)\n", failures);
    return failures != 0;
}
