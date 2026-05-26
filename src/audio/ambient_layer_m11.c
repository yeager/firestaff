/*
 * ambient_layer_m11 — runtime state for the optional ambient layer.
 *
 * The layer is intentionally minimal: it owns enabled/volume/context
 * state plus a resolved-file path that the SDL audio backend can read
 * when mixing.  Probing logic mirrors the soundtrack selector: it
 * looks for <dir>/<context>.<ext> with a small extension list.
 *
 * No ReDMCSB equivalent — Firestaff audio extra. Default state is
 * disabled, so V1 launches never touch this module.
 */

#include "ambient_layer_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define M11_AMBIENT_DIR_CAP   512
#define M11_AMBIENT_PATH_CAP  768
#define M11_AMBIENT_NAME_CAP   64

static const char* const g_context_names[M11_AMBIENT_CONTEXT_COUNT] = {
    "none",
    "underground",
    "combat",
    "quiet"
};

static const char* const g_context_labels[M11_AMBIENT_CONTEXT_COUNT] = {
    "None",
    "Underground",
    "Combat",
    "Quiet"
};

static const char* const g_extensions[] = {
    ".ogg",
    ".mp3",
    ".wav",
    ".flac",
    NULL
};

static int  g_enabled = 0;
static int  g_volume = 40;
static int  g_context = M11_AMBIENT_CONTEXT_NONE;
static char g_dir[M11_AMBIENT_DIR_CAP] = "";
static char g_resolvedPath[M11_AMBIENT_PATH_CAP] = "";

static int clamp_int(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static int file_exists(const char* path) {
    struct stat st;
    if (!path || path[0] == '\0') return 0;
    if (stat(path, &st) != 0) return 0;
    return S_ISREG(st.st_mode) ? 1 : 0;
}

static void resolve_default_directory(char* out, int outSize) {
    const char* home;
    int n;
    if (!out || outSize <= 0) return;
    out[0] = '\0';
    home = getenv("HOME");
    if (!home || home[0] == '\0') return;
    n = snprintf(out, (size_t)outSize, "%s/.firestaff/ambient", home);
    if (n <= 0 || n >= outSize) {
        out[0] = '\0';
    }
}

static void refresh_resolved_path(void) {
    const char* dir;
    char fallbackDir[M11_AMBIENT_DIR_CAP];
    char name[M11_AMBIENT_NAME_CAP];
    int ei;

    g_resolvedPath[0] = '\0';

    if (!g_enabled) return;
    if (g_context <= M11_AMBIENT_CONTEXT_NONE ||
        g_context >= M11_AMBIENT_CONTEXT_COUNT) {
        return;
    }
    dir = g_dir[0] ? g_dir : NULL;
    if (!dir) {
        resolve_default_directory(fallbackDir, (int)sizeof(fallbackDir));
        if (fallbackDir[0] == '\0') return;
        dir = fallbackDir;
    }
    if (!M11_Ambient_ResolveContextName(g_context, name, (int)sizeof(name))) {
        return;
    }
    /* Strip the trailing extension placeholder from name (ResolveContextName
       returns the bare filename without extension). */
    for (ei = 0; g_extensions[ei] != NULL; ++ei) {
        char candidate[M11_AMBIENT_PATH_CAP];
        int n = snprintf(candidate, sizeof(candidate), "%s/%s%s",
                         dir, name, g_extensions[ei]);
        if (n <= 0 || n >= (int)sizeof(candidate)) continue;
        if (file_exists(candidate)) {
            /* Copy into g_resolvedPath (may be smaller than candidate). */
            size_t need = (size_t)n + 1;
            if (need <= sizeof(g_resolvedPath)) {
                memcpy(g_resolvedPath, candidate, need);
            }
            return;
        }
    }
}

const char* M11_Ambient_GetContextLabel(int context) {
    if (context < 0 || context >= M11_AMBIENT_CONTEXT_COUNT) {
        return NULL;
    }
    return g_context_labels[context];
}

void M11_Ambient_SetEnabled(int enabled) {
    g_enabled = enabled ? 1 : 0;
    refresh_resolved_path();
}

int M11_Ambient_GetEnabled(void) {
    return g_enabled;
}

void M11_Ambient_SetVolume(int volumePercent) {
    g_volume = clamp_int(volumePercent, 0, 100);
}

int M11_Ambient_GetVolume(void) {
    return g_volume;
}

void M11_Ambient_SetContext(int context) {
    if (context < 0 || context >= M11_AMBIENT_CONTEXT_COUNT) {
        context = M11_AMBIENT_CONTEXT_NONE;
    }
    if (context == g_context) return;
    g_context = context;
    refresh_resolved_path();
}

int M11_Ambient_GetContext(void) {
    return g_context;
}

const char* M11_Ambient_GetActivePath(void) {
    return g_resolvedPath;
}

void M11_Ambient_SetDirectory(const char* dirPath) {
    if (!dirPath || dirPath[0] == '\0') {
        g_dir[0] = '\0';
    } else {
        size_t len = strlen(dirPath);
        if (len >= sizeof(g_dir)) len = sizeof(g_dir) - 1;
        memcpy(g_dir, dirPath, len);
        g_dir[len] = '\0';
    }
    refresh_resolved_path();
}

const char* M11_Ambient_GetDirectory(void) {
    return g_dir;
}

int M11_Ambient_ResolveContextName(int context, char* outName, int outSize) {
    if (!outName || outSize <= 0) return 0;
    outName[0] = '\0';
    if (context < 0 || context >= M11_AMBIENT_CONTEXT_COUNT) return 0;
    if (context == M11_AMBIENT_CONTEXT_NONE) return 0;
    {
        int n = snprintf(outName, (size_t)outSize, "%s", g_context_names[context]);
        if (n <= 0 || n >= outSize) {
            outName[0] = '\0';
            return 0;
        }
    }
    return 1;
}

void M11_Ambient_Tick(void) {
    /* Currently a no-op. The SDL audio backend will eventually wire
     * the resolved path through SDL_LoadWAV / decoded streaming. The
     * hook is exposed now so the main loop can call it without
     * waiting for the mixer integration. */
}
