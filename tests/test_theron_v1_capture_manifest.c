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
        "THERON_CAPTURE_MANIFEST_V2\n"
        "track02_path=/media/track02.bin\n"
        "track02_md5=f23601102138f87c33025877767ebf76\n"
        "system_card_path=/bios/syscard3.pce\n"
        "system_card_md5=ff1a674273fe3540ccef576376407d1d\n"
        "loader_trace_path=/captures/live.trace\n"
        "loader_trace_md5=35d3b8fae88a3864d12d0e4d62e4bcfa";
    Theron_V1CaptureManifest manifest;
    Theron_V1CaptureManifest copy;
    char written[1024];
    char track02_path[] = "/tmp/firestaff-theron-track02-XXXXXX";
    char system_card_path[] = "/tmp/firestaff-theron-system-card-XXXXXX";
    char trace_path[] = "/tmp/firestaff-theron-loader-trace-XXXXXX";
    int track02_fd;
    int system_card_fd;
    int trace_fd;

    check(theron_v1_capture_manifest_parse(valid, &manifest),
          "accepts exact, lower-case MD5-bound manifest");
    check(theron_v1_capture_manifest_matches(
              &manifest, "/media/track02.bin",
              "f23601102138f87c33025877767ebf76", "/bios/syscard3.pce",
              "ff1a674273fe3540ccef576376407d1d", "/captures/live.trace",
              "35d3b8fae88a3864d12d0e4d62e4bcfa"),
          "matches every bound capture input");
    check(theron_v1_capture_manifest_write(&manifest, written, sizeof(written)) &&
              strcmp(written, valid) == 0 &&
              theron_v1_capture_manifest_parse(written, &copy),
          "round trips canonical manifest text");
    check(!theron_v1_capture_manifest_parse(
              "THERON_CAPTURE_MANIFEST_V2\n"
              "track02_path=/media/track02.bin\n"
              "track02_md5=F23601102138F87C33025877767EBF76\n"
              "system_card_path=/bios/syscard3.pce\n"
              "system_card_md5=ff1a674273fe3540ccef576376407d1d\n"
              "loader_trace_path=/captures/live.trace\n"
              "loader_trace_md5=35d3b8fae88a3864d12d0e4d62e4bcfa", &manifest),
          "rejects non-canonical MD5 text");
    check(!theron_v1_capture_manifest_parse(
              "THERON_CAPTURE_MANIFEST_V2\n"
              "track02_path=/media/track02.bin\n"
              "track02_md5=f23601102138f87c33025877767ebf76\n"
              "system_card_path=/bios/syscard3.pce\n"
              "system_card_md5=ff1a674273fe3540ccef576376407d1d\n"
              "loader_trace_path=/captures/live.trace\n"
              "loader_trace_md5=35d3b8fae88a3864d12d0e4d62e4bcfa\n"
              "extra=forbidden", &manifest),
          "rejects unbound trailing fields");
    check(!theron_v1_capture_manifest_parse(
              "THERON_CAPTURE_MANIFEST_V2\n"
              "track02_path=/media/track02.bin\n"
              "track02_md5=f23601102138f87c33025877767ebf76\n"
              "system_card_path=/bios/syscard3.pce\n"
              "system_card_md5=ff1a674273fe3540ccef576376407d1d\n"
              "loader_trace_path=/captures/live\ttrace\n"
              "loader_trace_md5=35d3b8fae88a3864d12d0e4d62e4bcfa", &manifest),
          "rejects control bytes in capture paths");
    check(!theron_v1_capture_manifest_matches_preflight_inputs(
              &copy, "/media/track02.bin",
              "F23601102138F87C33025877767EBF76", "/bios/syscard3.pce",
              "ff1a674273fe3540ccef576376407d1d", "/captures/live.trace",
              "35d3b8fae88a3864d12d0e4d62e4bcfa"),
          "rejects non-canonical caller provenance hashes");
    check(!theron_v1_capture_manifest_matches_preflight_inputs(
              &copy, "/media/track02.bin",
              "f23601102138f87c33025877767ebf76", "/bios/syscard3.pce",
              "ff1a674273fe3540ccef576376407d1d", "/captures/live.trace",
              "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"),
          "rejects a trace file whose measured hash changed");
    check(!theron_v1_capture_manifest_parse(
              "THERON_CAPTURE_MANIFEST_V1\n"
              "track02_path=/media/track02.bin\n"
              "track02_md5=f23601102138f87c33025877767ebf76\n"
              "system_card_path=/bios/syscard3.pce\n"
              "system_card_md5=ff1a674273fe3540ccef576376407d1d\n"
              "loader_trace_path=/captures/live.trace", &manifest),
          "rejects legacy trace-unbound manifest format");

    track02_fd = mkstemp(track02_path);
    system_card_fd = mkstemp(system_card_path);
    trace_fd = mkstemp(trace_path);
    check(track02_fd >= 0 && system_card_fd >= 0 && trace_fd >= 0,
          "creates isolated explicit-media and trace files");
    if (track02_fd >= 0 && system_card_fd >= 0 && trace_fd >= 0) {
        check(write(track02_fd, "abc", 3u) == 3 &&
                  write(system_card_fd, "test", 4u) == 4 &&
                  write(trace_fd, "trace", 5u) == 5,
              "writes hash-known explicit-media and trace bytes");
        close(track02_fd);
        close(system_card_fd);
        close(trace_fd);
        check(theron_v1_boot_runtime_trace_files_match_declared_hashes(
                  track02_path, "900150983cd24fb0d6963f7d28e17f72",
                  system_card_path, "098f6bcd4621d373cade4e832627b4f6",
                  trace_path, "04a75036e9d520bb983c5ed03b8d0182"),
              "rehashes exact Track 02, System Card, and trace inputs");
        check(!theron_v1_boot_runtime_trace_files_match_declared_hashes(
                  track02_path, "f23601102138f87c33025877767ebf76",
                  system_card_path, "098f6bcd4621d373cade4e832627b4f6",
                  trace_path, "04a75036e9d520bb983c5ed03b8d0182"),
              "rejects a known label paired with changed Track 02 bytes");
        check(!theron_v1_boot_runtime_trace_files_match_declared_hashes(
                  track02_path, "900150983cd24fb0d6963f7d28e17f72",
                  system_card_path, "098f6bcd4621d373cade4e832627b4f6",
                  trace_path, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"),
              "rejects a changed loader trace hash");
    }
    if (track02_fd >= 0) unlink(track02_path);
    if (system_card_fd >= 0) unlink(system_card_path);
    if (trace_fd >= 0) unlink(trace_path);
    printf("theron capture manifest: %d failure(s)\n", failures);
    return failures != 0;
}
