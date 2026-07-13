#include "theron_v1_capture_manifest.h"
#include "theron_v1_boot.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

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
    char track02_path[] = "/tmp/firestaff-theron-track02-XXXXXX";
    char system_card_path[] = "/tmp/firestaff-theron-system-card-XXXXXX";
    int track02_fd;
    int system_card_fd;

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

    track02_fd = mkstemp(track02_path);
    system_card_fd = mkstemp(system_card_path);
    check(track02_fd >= 0 && system_card_fd >= 0,
          "creates isolated explicit-media files");
    if (track02_fd >= 0 && system_card_fd >= 0) {
        check(write(track02_fd, "abc", 3u) == 3 &&
                  write(system_card_fd, "test", 4u) == 4,
              "writes hash-known explicit-media bytes");
        close(track02_fd);
        close(system_card_fd);
        check(theron_v1_boot_runtime_trace_files_match_declared_hashes(
                  track02_path, "900150983cd24fb0d6963f7d28e17f72",
                  system_card_path, "098f6bcd4621d373cade4e832627b4f6"),
              "rehashes exact Track 02 and System Card inputs");
        check(!theron_v1_boot_runtime_trace_files_match_declared_hashes(
                  track02_path, "f23601102138f87c33025877767ebf76",
                  system_card_path, "098f6bcd4621d373cade4e832627b4f6"),
              "rejects a known label paired with changed Track 02 bytes");
    }
    if (track02_fd >= 0) unlink(track02_path);
    if (system_card_fd >= 0) unlink(system_card_path);
    printf("theron capture manifest: %d failure(s)\n", failures);
    return failures != 0;
}
