#include "theron_v1_capture_manifest.h"
#include "theron_v1_boot.h"
#include "theron_v1_raw_loader_trace.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <io.h>
#include <process.h>
static int win32_mkstemp(char *tmpl) {
    if (_mktemp(tmpl) == NULL) return -1;
    return _open(tmpl, _O_CREAT | _O_RDWR | _O_EXCL, 0600);
}
#define mkstemp win32_mkstemp
#define write _write
#define close _close
#define unlink _unlink
#else
#include <unistd.h>
#endif

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
    Theron_V1CaptureManifest resolved;
    Theron_V1_BootProfile profile;
    char written[1024];
    char track02_path[] = "/tmp/firestaff-theron-track02-XXXXXX";
    char system_card_path[] = "/tmp/firestaff-theron-system-card-XXXXXX";
    char trace_path[] = "/tmp/firestaff-theron-loader-trace-XXXXXX";
    char manifest_path[] = "/tmp/firestaff-theron-capture-manifest-XXXXXX";
    int track02_fd;
    int system_card_fd;
    int trace_fd;
    int manifest_fd;
    int mutate_fd;

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
    check(theron_v1_raw_loader_trace_capture_manifest_matches(
              &copy, "/media/track02.bin",
              "f23601102138f87c33025877767ebf76", "/bios/syscard3.pce",
              "ff1a674273fe3540ccef576376407d1d", "/captures/live.trace",
              "35d3b8fae88a3864d12d0e4d62e4bcfa"),
          "binds an ordered raw loader trace to its Track 02 and System Card manifest");
    check(!theron_v1_raw_loader_trace_capture_manifest_matches(
              &copy, "/media/track02.bin",
              "f23601102138f87c33025877767ebf76", "/bios/syscard3.pce",
              "098f6bcd4621d373cade4e832627b4f6", "/captures/live.trace",
              "35d3b8fae88a3864d12d0e4d62e4bcfa"),
          "rejects an ordered raw loader trace with a non-System Card 3.0 identity");
    check(!theron_v1_raw_loader_trace_capture_manifest_matches(
              &copy, "/media/track02.bin",
              "f23601102138f87c33025877767ebf76", "/bios/syscard3.pce",
              "ff1a674273fe3540ccef576376407d1d", "/captures/other.trace",
              "35d3b8fae88a3864d12d0e4d62e4bcfa"),
          "rejects an ordered raw loader trace path outside its capture manifest");
    check(!theron_v1_raw_loader_trace_capture_manifest_matches(
              &copy, "/media/track02.bin",
              "f23601102138f87c33025877767ebf76", "/bios/syscard3.pce",
              "ff1a674273fe3540ccef576376407d1d", "/captures/live.trace",
              "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"),
          "rejects an ordered raw loader trace whose hash changed");

    track02_fd = mkstemp(track02_path);
    system_card_fd = mkstemp(system_card_path);
    trace_fd = mkstemp(trace_path);
    manifest_fd = mkstemp(manifest_path);
    check(track02_fd >= 0 && system_card_fd >= 0 && trace_fd >= 0 &&
              manifest_fd >= 0,
          "creates isolated explicit-media, trace, and manifest files");
    if (track02_fd >= 0 && system_card_fd >= 0 && trace_fd >= 0 &&
        manifest_fd >= 0) {
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
        memset(&manifest, 0, sizeof(manifest));
        snprintf(manifest.track02_path, sizeof(manifest.track02_path), "%s",
                 track02_path);
        snprintf(manifest.track02_md5, sizeof(manifest.track02_md5), "%s",
                 "900150983cd24fb0d6963f7d28e17f72");
        snprintf(manifest.system_card_path, sizeof(manifest.system_card_path),
                 "%s", system_card_path);
        snprintf(manifest.system_card_md5, sizeof(manifest.system_card_md5),
                 "%s", "098f6bcd4621d373cade4e832627b4f6");
        snprintf(manifest.trace_path, sizeof(manifest.trace_path), "%s",
                 trace_path);
        snprintf(manifest.trace_md5, sizeof(manifest.trace_md5), "%s",
                 "04a75036e9d520bb983c5ed03b8d0182");
        manifest.valid = 1;
        check(theron_v1_capture_manifest_write(&manifest, written,
                                                sizeof(written)) &&
                  write(manifest_fd, written, strlen(written)) ==
                      (ssize_t)strlen(written),
              "writes a bounded hash-bound runtime capture manifest");
        close(manifest_fd);
        theron_v1_boot_profile_init(&profile);
        profile.assets_verified = 1;
        snprintf(profile.graphics_path, sizeof(profile.graphics_path), "%s",
                 track02_path);
        snprintf(profile.graphics_md5, sizeof(profile.graphics_md5), "%s",
                 "900150983cd24fb0d6963f7d28e17f72");
        check(theron_v1_boot_runtime_capture_manifest_from_file(
                  &profile, manifest_path, &resolved) && resolved.valid &&
                  strcmp(resolved.trace_md5,
                         "04a75036e9d520bb983c5ed03b8d0182") == 0,
              "binds a V2 capture manifest to the selected Track 02 profile");
        mutate_fd = open(track02_path, O_WRONLY | O_APPEND);
        check(mutate_fd >= 0 && write(mutate_fd, "d", 1u) == 1,
              "changes Track 02 bytes after boot-profile identity was set");
        if (mutate_fd >= 0) close(mutate_fd);
        check(!theron_v1_boot_runtime_capture_manifest_from_file(
                  &profile, manifest_path, &resolved) && !resolved.valid,
              "rejects a capture manifest after selected Track 02 changes");
        snprintf(profile.graphics_md5, sizeof(profile.graphics_md5), "%s",
                 "f23601102138f87c33025877767ebf76");
        check(!theron_v1_boot_runtime_capture_manifest_from_file(
                  &profile, manifest_path, &resolved) && !resolved.valid,
              "rejects a manifest for a different selected Track 02 identity");
    }
    if (track02_fd >= 0) unlink(track02_path);
    if (system_card_fd >= 0) unlink(system_card_path);
    if (trace_fd >= 0) unlink(trace_path);
    if (manifest_fd >= 0) unlink(manifest_path);
    printf("theron capture manifest: %d failure(s)\n", failures);
    return failures != 0;
}
