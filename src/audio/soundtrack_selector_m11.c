/*
 * soundtrack_selector_m11 — soundtrack profile resolver.
 *
 * See include/soundtrack_selector_m11.h for the contract. Implementation
 * is intentionally tiny and synchronous: the resolver probes a small
 * fixed set of file extensions and returns the first hit. When nothing
 * matches the caller falls back to the built-in V1 soundtrack.
 *
 * No ReDMCSB equivalent — Firestaff audio extra. Default mode is 0
 * (Original), which never touches the filesystem.
 */

#include "soundtrack_selector_m11.h"
#include "fs_portable_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static const char* const g_labels[M11_SOUNDTRACK_MODE_COUNT] = {
    "Original",
    "Remastered",
    "Custom Folder"
};

static const char* const g_extensions[] = {
    ".ogg",
    ".mp3",
    ".wav",
    ".flac",
    NULL
};

const char* M11_Soundtrack_GetLabel(int mode) {
    if (mode < 0 || mode >= M11_SOUNDTRACK_MODE_COUNT) {
        return NULL;
    }
    return g_labels[mode];
}

int M11_Soundtrack_IsValid(int mode) {
    return (mode >= 0 && mode < M11_SOUNDTRACK_MODE_COUNT) ? 1 : 0;
}

static int file_exists(const char* path) {
    struct stat st;
    if (!path || path[0] == '\0') return 0;
    if (stat(path, &st) != 0) return 0;
    return S_ISREG(st.st_mode) ? 1 : 0;
}

static int probe_with_extensions(const char* dir,
                                 const char* trackName,
                                 char* outPath,
                                 int outSize) {
    int ei;
    char candidate[FSP_PATH_MAX];
    char resolved[FSP_PATH_MAX];
    if (!dir || !trackName || !outPath || outSize <= 0) {
        return 0;
    }
    for (ei = 0; g_extensions[ei] != NULL; ++ei) {
        int n = snprintf(candidate, sizeof(candidate), "%s/%s%s",
                         dir, trackName, g_extensions[ei]);
        if (n <= 0 || n >= (int)sizeof(candidate)) {
            continue;
        }
        if (file_exists(candidate) &&
            FSP_ResolvePhysicalPath(resolved, sizeof(resolved), candidate) &&
            strlen(resolved) < (size_t)outSize) {
            memcpy(outPath, resolved, strlen(resolved) + 1U);
            return 1;
        }
    }
    outPath[0] = '\0';
    return 0;
}

int M11_Soundtrack_GetDefaultCustomDir(char* outPath, int outSize) {
    const char* home;
    int n;
    if (!outPath || outSize <= 0) return 0;
    outPath[0] = '\0';
    home = getenv("HOME");
    if (!home || home[0] == '\0') {
        return 0;
    }
    n = snprintf(outPath, (size_t)outSize, "%s/.firestaff/music", home);
    if (n <= 0 || n >= outSize) {
        outPath[0] = '\0';
        return 0;
    }
    return 1;
}

int M11_Soundtrack_GetTrackPath(int mode,
                                const char* trackName,
                                const char* customMusicPath,
                                char* outPath,
                                int outSize) {
    char defaultDir[512];
    if (outPath && outSize > 0) {
        outPath[0] = '\0';
    }
    if (mode == M11_SOUNDTRACK_MODE_ORIGINAL ||
        !trackName || trackName[0] == '\0') {
        return M11_SOUNDTRACK_RESULT_ORIGINAL;
    }
    if (mode == M11_SOUNDTRACK_MODE_REMASTERED) {
        /* Resolve the existing CWD-relative search root before returning.
         * The launcher does not change CWD; CUSTOM supplies another root. */
        if (outPath && outSize > 0 &&
            probe_with_extensions("data/music/remastered", trackName,
                                  outPath, outSize)) {
            return M11_SOUNDTRACK_RESULT_RESOLVED;
        }
        if (outPath && outSize > 0) outPath[0] = '\0';
        return M11_SOUNDTRACK_RESULT_FALLBACK;
    }
    if (mode == M11_SOUNDTRACK_MODE_CUSTOM) {
        const char* dir = customMusicPath;
        if (!dir || dir[0] == '\0') {
            if (M11_Soundtrack_GetDefaultCustomDir(defaultDir, (int)sizeof(defaultDir))) {
                dir = defaultDir;
            } else {
                dir = NULL;
            }
        }
        if (dir && outPath && outSize > 0 &&
            probe_with_extensions(dir, trackName, outPath, outSize)) {
            return M11_SOUNDTRACK_RESULT_RESOLVED;
        }
        if (outPath && outSize > 0) outPath[0] = '\0';
        return M11_SOUNDTRACK_RESULT_FALLBACK;
    }
    /* Unknown mode → behave like Original. */
    return M11_SOUNDTRACK_RESULT_ORIGINAL;
}
