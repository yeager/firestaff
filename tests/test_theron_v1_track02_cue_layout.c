#include "theron_v1_track02.h"

#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#include <process.h>
#define THERON_MKDIR(path) _mkdir(path)
#define THERON_GETPID() _getpid()
#else
#include <sys/stat.h>
#include <unistd.h>
#define THERON_MKDIR(path) mkdir((path), 0700)
#define THERON_GETPID() getpid()
#endif

static int write_file(const char *path, const char *contents)
{
    FILE *file = fopen(path, "wb");
    if (!file) return 0;
    if (contents && fputs(contents, file) == EOF) {
        fclose(file);
        return 0;
    }
    return fclose(file) == 0;
}

int main(void)
{
    char root[256];
    char cue_path[320];
    char iso_path[320];
    char bin_path[320];
    char plain_iso_path[320];
    char resolved[THERON_TRACK02_MOUNT_PATH_CAPACITY];

    snprintf(root, sizeof(root), "firestaff-theron-cue-%ld",
             (long)THERON_GETPID());
    snprintf(cue_path, sizeof(cue_path), "%s/layout.cue", root);
    snprintf(iso_path, sizeof(iso_path), "%s/Track 02.iso", root);
    snprintf(bin_path, sizeof(bin_path), "%s/Track 02.bin", root);
    snprintf(plain_iso_path, sizeof(plain_iso_path), "%s/track02.iso", root);
    if (THERON_MKDIR(root) != 0 || !write_file(iso_path, "x") ||
        !write_file(bin_path, "x") || !write_file(plain_iso_path, "x")) {
        fputs("FAIL: CUE test setup\n", stderr);
        return 1;
    }

    if (!write_file(cue_path,
                    "FILE track02.iso BINARY\n"
                    "  TRACK 02 MODE1/2048\n"
                    "    INDEX 01 00:00:00\n") ||
        theron_v1_track02_resolve_media_path(cue_path, resolved) !=
            THERON_TRACK02_SIGNAL_OK || strcmp(resolved, plain_iso_path) != 0) {
        fputs("FAIL: unquoted MODE1/2048 CUE did not resolve\n", stderr);
        return 1;
    }

    if (!write_file(cue_path,
                    "FILE \"Track 02.iso\" BINARY\n"
                    "  TRACK 02 MODE1/2048\n"
                    "    INDEX 01 00:00:00\n") ||
        theron_v1_track02_resolve_media_path(cue_path, resolved) !=
            THERON_TRACK02_SIGNAL_OK || strcmp(resolved, iso_path) != 0) {
        fputs("FAIL: MODE1/2048 CUE did not resolve its declared payload\n",
              stderr);
        return 1;
    }

    if (!write_file(cue_path,
                    "FILE \"Track 02.bin\" BINARY\n"
                    "  TRACK 02 MODE1/2352\n"
                    "    INDEX 01 00:00:00\n") ||
        theron_v1_track02_resolve_media_path(cue_path, resolved) !=
            THERON_TRACK02_SIGNAL_OK || strcmp(resolved, bin_path) != 0) {
        fputs("FAIL: MODE1/2352 CUE did not resolve its declared payload\n",
              stderr);
        return 1;
    }

    if (!write_file(cue_path,
                    "file \"Track 02.iso\" binary\n"
                    "  track 02 mode1/2048\n"
                    "    index 01 00:00:00\n") ||
        theron_v1_track02_resolve_media_path(cue_path, resolved) !=
            THERON_TRACK02_SIGNAL_OK || strcmp(resolved, iso_path) != 0) {
        fputs("FAIL: lowercase MODE1/2048 CUE did not resolve\n", stderr);
        return 1;
    }

    if (!write_file(cue_path,
                    "FILE \"Track 02.iso\" BINARY\n"
                    "  TRACK 02 MODE1/2048\n") ||
        theron_v1_track02_resolve_media_path(cue_path, resolved) !=
            THERON_TRACK02_SIGNAL_NOT_FOUND) {
        fputs("FAIL: Track 02 CUE without INDEX 01 was accepted\n", stderr);
        return 1;
    }

    if (!write_file(cue_path,
                    "FILE \"Track 02.iso\" BINARY\n"
                    "  TRACK 02 MODE1/2048\n"
                    "    INDEX 01 00:00:00\n"
                    "    INDEX 01 00:01:00\n") ||
        theron_v1_track02_resolve_media_path(cue_path, resolved) !=
            THERON_TRACK02_SIGNAL_NOT_FOUND) {
        fputs("FAIL: Track 02 CUE with duplicate INDEX 01 was accepted\n",
              stderr);
        return 1;
    }

    if (!write_file(cue_path,
                    "FILE \"Track 02.bin\" BINARY\n"
                    "  TRACK 02 MODE1/2352\n"
                    "    INDEX 01 00:00:00\n"
                    "FILE \"Track 02.iso\" BINARY\n"
                    "  TRACK 02 MODE1/2048\n"
                    "    INDEX 01 00:00:00\n") ||
        theron_v1_track02_resolve_media_path(cue_path, resolved) !=
            THERON_TRACK02_SIGNAL_NOT_FOUND) {
        fputs("FAIL: ambiguous Track 02 CUE was accepted\n", stderr);
        return 1;
    }

    remove(cue_path);
    remove(iso_path);
    remove(bin_path);
    remove(plain_iso_path);
    rmdir(root);
    puts("theron Track 02 CUE layout passed");
    return 0;
}
