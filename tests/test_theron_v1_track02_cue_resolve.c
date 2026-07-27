#include "theron_v1_track02.h"

#include <stdio.h>
#include <string.h>

#if !defined(_WIN32)
#include <stdlib.h>
#include <unistd.h>
#endif

Theron_MapLoadResult theron_v1_level_load(Theron_V1_Level *level,
                                          const uint8_t *data,
                                          int data_size,
                                          int dungeon_id,
                                          int sub_level_index) {
    (void)level;
    (void)data;
    (void)data_size;
    (void)dungeon_id;
    (void)sub_level_index;
    return THERON_MAP_ERR_NULL;
}

void theron_v1_world_runtime_media_invalidate_cache(Theron_V1_World *world) {
    (void)world;
}

static int write_file(const char *path, const void *contents, size_t length) {
    FILE *file = fopen(path, "wb");

    if (!file) return 0;
    if (length && fwrite(contents, 1u, length, file) != length) {
        fclose(file);
        return 0;
    }
    fclose(file);
    return 1;
}

int main(void) {
#if defined(_WIN32)
    printf("test_theron_v1_track02_cue_resolve: SKIP (fixture path)\n");
    return 0;
#else
    char directory[] = "/tmp/firestaff_theron_track02_cue_XXXXXX";
    char cue[512];
    char data[512];
    char split[512];
    char resolved[THERON_TRACK02_MOUNT_PATH_CAPACITY];
    int failed = 0;

    if (!mkdtemp(directory)) return 1;
    snprintf(cue, sizeof(cue), "%s/original.cue", directory);
    snprintf(data, sizeof(data), "%s/track02.bin", directory);
    snprintf(split, sizeof(split), "%s/TQUS02End.iso", directory);

    failed |= !write_file(data, "track02 raw sector fixture",
                          strlen("track02 raw sector fixture"));
    failed |= !write_file(split, "split track02 fixture",
                          strlen("split track02 fixture"));

    failed |= !write_file(cue,
        "FILE \"track02.bin\" BINARY\n"
        "  TRACK 02 MODE1/2352\n"
        "    INDEX 01 00:00:00\n",
        strlen("FILE \"track02.bin\" BINARY\n"
               "  TRACK 02 MODE1/2352\n"
               "    INDEX 01 00:00:00\n"));
    if (!failed && theron_v1_track02_resolve_media_path(cue, resolved) !=
            THERON_TRACK02_SIGNAL_OK) {
        failed = 1;
    }
    if (!failed && strcmp(resolved, data) != 0) {
        failed = 1;
    }

    /* A CUE saved by a UTF-8 editor may have a BOM before its first FILE
     * directive. It must resolve exactly like the source CUE, not report an
     * invalid Track 02 solely because of text encoding metadata. */
    failed |= !write_file(cue,
        "\xef\xbb\xbf" "FILE \"track02.bin\" BINARY\n"
        "  TRACK 02 MODE1/2352\n"
        "    INDEX 01 00:00:00\n",
        strlen("\xef\xbb\xbf" "FILE \"track02.bin\" BINARY\n"
               "  TRACK 02 MODE1/2352\n"
               "    INDEX 01 00:00:00\n"));
    if (!failed && theron_v1_track02_resolve_media_path(cue, resolved) !=
            THERON_TRACK02_SIGNAL_OK) {
        failed = 1;
    }
    if (!failed && strcmp(resolved, data) != 0) {
        failed = 1;
    }

    failed |= !write_file(cue,
        "FILE \"track02.bin\" BINARY\n"
        "  TRACK 02 MODE1/2048\n"
        "    INDEX 01 00:00:00\n",
        strlen("FILE \"track02.bin\" BINARY\n"
               "  TRACK 02 MODE1/2048\n"
               "    INDEX 01 00:00:00\n"));
    if (!failed && theron_v1_track02_resolve_media_path(cue, resolved) !=
            THERON_TRACK02_SIGNAL_OK) {
        failed = 1;
    }
    if (!failed && strcmp(resolved, data) != 0) {
        failed = 1;
    }

    failed |= !write_file(cue,
        "FILE \"TQUS02.iso\" BINARY\n"
        "  TRACK 02 MODE1/2352\n"
        "    INDEX 01 00:00:00\n",
        strlen("FILE \"TQUS02.iso\" BINARY\n"
               "  TRACK 02 MODE1/2352\n"
               "    INDEX 01 00:00:00\n"));
    if (!failed && theron_v1_track02_resolve_media_path(cue, resolved) !=
            THERON_TRACK02_SIGNAL_OK) {
        failed = 1;
    }
    if (!failed && strcmp(resolved, split) != 0) {
        failed = 1;
    }

    failed |= !write_file(cue,
        "FILE \"TQUS02.iso\" BINARY\n"
        "  TRACK 02 MODE1/2048\n"
        "    INDEX 01 00:00:00\n",
        strlen("FILE \"TQUS02.iso\" BINARY\n"
               "  TRACK 02 MODE1/2048\n"
               "    INDEX 01 00:00:00\n"));
    if (!failed && theron_v1_track02_resolve_media_path(cue, resolved) !=
            THERON_TRACK02_SIGNAL_OK) {
        failed = 1;
    }
    if (!failed && strcmp(resolved, split) != 0) {
        failed = 1;
    }

    failed |= !write_file(cue,
        "FILE \"missing.iso\" BINARY\n"
        "  TRACK 02 MODE1/2048\n"
        "    INDEX 01 00:00:00\n",
        strlen("FILE \"missing.iso\" BINARY\n"
               "  TRACK 02 MODE1/2048\n"
               "    INDEX 01 00:00:00\n"));
    if (!failed && theron_v1_track02_resolve_media_path(cue, resolved) !=
            THERON_TRACK02_SIGNAL_NOT_FOUND) {
        failed = 1;
    }

    failed |= !write_file(cue,
        "FILE \"track02.bin\" BINARY\n"
        "  TRACK 02 MODE1/2352\n"
        "    INDEX 01 00:00:00\n"
        "    INDEX 01 00:00:01\n",
        strlen("FILE \"track02.bin\" BINARY\n"
               "  TRACK 02 MODE1/2352\n"
               "    INDEX 01 00:00:00\n"
               "    INDEX 01 00:00:01\n"));
    if (!failed && theron_v1_track02_resolve_media_path(cue, resolved) !=
            THERON_TRACK02_SIGNAL_NOT_FOUND) {
        failed = 1;
    }

    remove(cue);
    remove(data);
    remove(split);
    rmdir(directory);
    printf("test_theron_v1_track02_cue_resolve: %s\n",
           failed ? "FAIL" : "PASS");
    return failed;
#endif
}
