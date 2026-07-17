#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include "theron_v1_track02_raw_media_intake.h"

static int has_cue_suffix(const char *name)
{
    size_t bytes = name ? strlen(name) : 0u;
    return bytes > 4u && strcmp(name + bytes - 4u, ".cue") == 0;
}

int main(int argc, char **argv)
{
    DIR *root;
    struct dirent *entry;
    unsigned int examined = 0u;
    unsigned int authenticated = 0u;
    if (argc == 1) {
        puts("SKIP: explicit capture corpus root required");
        return 0;
    }
    if (argc != 3 || strcmp(argv[1], "--discover")) {
        fprintf(stderr, "usage: %s --discover <capture-corpus-root>\n", argv[0]);
        return 2;
    }
    root = opendir(argv[2]);
    if (!root) {
        puts("SKIP: capture corpus root unavailable");
        return 0;
    }
    while ((entry = readdir(root)) != NULL && examined < 64u) {
        Theron_V1Track02RawMediaIntakeReceipt media;
        char path[THERON_V1_TRACK02_MEDIA_PATH_CAPACITY];
        if (!has_cue_suffix(entry->d_name) ||
            snprintf(path, sizeof(path), "%s/%s", argv[2], entry->d_name) >= (int)sizeof(path)) continue;
        ++examined;
        if (theron_v1_track02_raw_media_intake_discover(path, &media) &&
            media.status == THERON_V1_TRACK02_MEDIA_INTAKE_READY && media.raw_trace_preparation_allowed) ++authenticated;
    }
    closedir(root);
    if (!authenticated) {
        puts("SKIP: no authenticated Track 02 CUE candidate in capture corpus");
        return 0;
    }
    printf("SKIP: %u authenticated Track 02 CUE candidate(s) require CD-read, palette, bitmap, and destination observations\n", authenticated);
    return 0;
}
