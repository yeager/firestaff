/*
 * fs_portable_compat.c — cross-platform filesystem helpers for Firestaff.
 *
 * Centralizes path joining, directory creation, and standard-location
 * resolution that was previously duplicated across M11/M12 modules.
 */

#include "fs_portable_compat.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined(_WIN32)
#include <windows.h>
#include <direct.h>
#define FSP_SEP '\\'
#define FSP_ALT_SEP '/'
#else
#include <unistd.h>
#define FSP_SEP '/'
#define FSP_ALT_SEP '\\'
#endif

/* ── Internal helpers ───────────────────────────────────────────────── */

static int fsp_is_separator(char c) {
    return c == '/' || c == '\\';
}

static size_t fsp_copy(char* dst, size_t dstSize, const char* src) {
    size_t len;
    if (!dst || dstSize == 0U || !src) {
        return 0U;
    }
    len = strlen(src);
    if (len >= dstSize) {
        len = dstSize - 1U;
    }
    memcpy(dst, src, len);
    dst[len] = '\0';
    return len;
}

/* ── Path manipulation ──────────────────────────────────────────────── */

int FSP_JoinPath(char* out, size_t outSize,
                 const char* left, const char* right) {
    int rc;
    size_t leftLen;
    if (!out || outSize == 0U || !left || !right) {
        return 0;
    }
    leftLen = strlen(left);
    /* Skip leading separator on right when left already ends with one. */
    if (leftLen > 0U && fsp_is_separator(left[leftLen - 1U])) {
        while (*right != '\0' && fsp_is_separator(*right)) {
            ++right;
        }
        rc = snprintf(out, outSize, "%s%s", left, right);
    } else if (leftLen == 0U) {
        rc = snprintf(out, outSize, "%s", right);
    } else {
        rc = snprintf(out, outSize, "%s%c%s", left, FSP_SEP, right);
    }
    return rc > 0 && (size_t)rc < outSize;
}

int FSP_ParentDir(char* out, size_t outSize, const char* path) {
    const char* lastSep = NULL;
    const char* p;
    size_t len;
    if (!out || outSize == 0U || !path) {
        return 0;
    }
    for (p = path; *p != '\0'; ++p) {
        if (fsp_is_separator(*p)) {
            lastSep = p;
        }
    }
    if (!lastSep) {
        return 0;
    }
    len = (size_t)(lastSep - path);
    if (len == 0U) {
        /* Root directory case: "/" or "\\" */
        len = 1U;
    }
    if (len >= outSize) {
        return 0;
    }
    memcpy(out, path, len);
    out[len] = '\0';
    return 1;
}

char* FSP_NormalizeSeparators(char* path) {
    char* p;
    if (!path) {
        return NULL;
    }
    for (p = path; *p != '\0'; ++p) {
        if (*p == FSP_ALT_SEP) {
            *p = FSP_SEP;
        }
    }
    return path;
}

/* ── Filesystem queries ─────────────────────────────────────────────── */

int FSP_PathExists(const char* path) {
    struct stat st;
    if (!path || path[0] == '\0') {
        return 0;
    }
    return stat(path, &st) == 0;
}

int FSP_FileExists(const char* path) {
    struct stat st;
    if (!path || path[0] == '\0') {
        return 0;
    }
    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

int FSP_DirExists(const char* path) {
    struct stat st;
    if (!path || path[0] == '\0') {
        return 0;
    }
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

/* ── Directory creation ─────────────────────────────────────────────── */

int FSP_CreateDirectory(const char* path) {
    if (!path || path[0] == '\0') {
        return 0;
    }
#if defined(_WIN32)
    return _mkdir(path) == 0 || errno == EEXIST;
#else
    return mkdir(path, 0777) == 0 || errno == EEXIST;
#endif
}

int FSP_CreateDirectoryRecursive(const char* path) {
    char buf[FSP_PATH_MAX];
    size_t len;
    size_t i;

    if (!path || path[0] == '\0') {
        return 0;
    }
    len = strlen(path);
    if (len >= sizeof(buf)) {
        return 0;
    }
    memcpy(buf, path, len + 1U);

    /* Walk forward, creating each component. */
    for (i = 1U; i <= len; ++i) {
        if (i == len || fsp_is_separator(buf[i])) {
            char saved = buf[i];
            buf[i] = '\0';
            if (!FSP_DirExists(buf)) {
                if (!FSP_CreateDirectory(buf)) {
                    return 0;
                }
            }
            buf[i] = saved;
        }
    }
    return 1;
}

/* ── Standard locations ─────────────────────────────────────────────── */

int FSP_GetUserDataDir(char* out, size_t outSize) {
    int rc;
    if (!out || outSize == 0U) {
        return 0;
    }

#if defined(_WIN32)
    {
        const char* appData = getenv("APPDATA");
        if (appData && appData[0] != '\0') {
            rc = snprintf(out, outSize, "%s\\Firestaff", appData);
            return rc > 0 && (size_t)rc < outSize;
        }
    }
#elif defined(__APPLE__)
    {
        const char* home = getenv("HOME");
        if (home && home[0] != '\0') {
            rc = snprintf(out, outSize,
                          "%s/Library/Application Support/Firestaff", home);
            return rc > 0 && (size_t)rc < outSize;
        }
    }
#else
    /* Linux / generic POSIX: XDG_DATA_HOME or ~/.local/share */
    {
        const char* xdg = getenv("XDG_DATA_HOME");
        if (xdg && xdg[0] != '\0') {
            rc = snprintf(out, outSize, "%s/firestaff", xdg);
            return rc > 0 && (size_t)rc < outSize;
        }
    }
    {
        const char* home = getenv("HOME");
        if (home && home[0] != '\0') {
            rc = snprintf(out, outSize,
                          "%s/.local/share/firestaff", home);
            return rc > 0 && (size_t)rc < outSize;
        }
    }
#endif

    /* Fallback: current directory. */
    fsp_copy(out, outSize, ".");
    return 1;
}

int FSP_GetUserConfigDir(char* out, size_t outSize) {
    int rc;
    if (!out || outSize == 0U) {
        return 0;
    }

#if defined(_WIN32)
    {
        const char* appData = getenv("APPDATA");
        if (appData && appData[0] != '\0') {
            rc = snprintf(out, outSize, "%s\\Firestaff", appData);
            return rc > 0 && (size_t)rc < outSize;
        }
    }
#elif defined(__APPLE__)
    {
        const char* home = getenv("HOME");
        if (home && home[0] != '\0') {
            rc = snprintf(out, outSize,
                          "%s/Library/Application Support/Firestaff", home);
            return rc > 0 && (size_t)rc < outSize;
        }
    }
#else
    /* Linux / generic POSIX: XDG_CONFIG_HOME or ~/.config */
    {
        const char* xdg = getenv("XDG_CONFIG_HOME");
        if (xdg && xdg[0] != '\0') {
            rc = snprintf(out, outSize, "%s/firestaff", xdg);
            return rc > 0 && (size_t)rc < outSize;
        }
    }
    {
        const char* home = getenv("HOME");
        if (home && home[0] != '\0') {
            rc = snprintf(out, outSize,
                          "%s/.config/firestaff", home);
            return rc > 0 && (size_t)rc < outSize;
        }
    }
#endif

    fsp_copy(out, outSize, ".");
    return 1;
}

int FSP_GetDefaultOriginalsDir(char* out, size_t outSize) {
    int rc;
    if (!out || outSize == 0U) {
        return 0;
    }

#if defined(_WIN32)
    {
        char modulePath[FSP_PATH_MAX];
        DWORD len = GetModuleFileNameA(NULL, modulePath, (DWORD)sizeof(modulePath));
        if (len > 0U && len < (DWORD)sizeof(modulePath)) {
            char installDir[FSP_PATH_MAX];
            FSP_NormalizeSeparators(modulePath);
            if (FSP_ParentDir(installDir, sizeof(installDir), modulePath)) {
                return FSP_JoinPath(out, outSize, installDir, "originals");
            }
        }
    }
    fsp_copy(out, outSize, ".\\originals");
    return 1;
#else
    {
        const char* home = getenv("HOME");
        if (home && home[0] != '\0') {
            rc = snprintf(out, outSize, "%s/.firestaff/originals", home);
            return rc > 0 && (size_t)rc < outSize;
        }
    }
    fsp_copy(out, outSize, "./originals");
    return 1;
#endif
}

int FSP_ResolveDataDir(char* out, size_t outSize, const char* requestedDir) {
    int rc;
    const char* envData;
    char userDir[FSP_PATH_MAX];
#if !defined(_WIN32)
    char legacyDir[FSP_PATH_MAX];
    const char* home;
#endif

    if (!out || outSize == 0U) {
        return 0;
    }

    /* Priority 1: explicit request. */
    if (requestedDir && requestedDir[0] != '\0') {
        fsp_copy(out, outSize, requestedDir);
        return 1;
    }

    /* Priority 2: FIRESTAFF_DATA environment variable. */
    envData = getenv("FIRESTAFF_DATA");
    if (envData && envData[0] != '\0') {
        fsp_copy(out, outSize, envData);
        return 1;
    }

#if !defined(_WIN32)
    /* Priority 3: legacy ~/.firestaff/data when it already exists.
     * Earlier Firestaff builds and N2's verified PC34 asset setup used this
     * tree.  Preserve it as a read-side compatibility root so startup/title
     * assets are not skipped in favour of an empty XDG data directory. */
    home = getenv("HOME");
    if (home && home[0] != '\0') {
        rc = snprintf(legacyDir, sizeof(legacyDir), "%s/.firestaff/data", home);
        if (rc > 0 && (size_t)rc < sizeof(legacyDir) && FSP_DirExists(legacyDir)) {
            fsp_copy(out, outSize, legacyDir);
            return 1;
        }
    }
#endif

    /* Priority 4: <user-data-dir>/data */
    if (FSP_GetUserDataDir(userDir, sizeof(userDir))) {
        return FSP_JoinPath(out, outSize, userDir, "data");
    }

    /* Priority 5: current directory. */
    fsp_copy(out, outSize, ".");
    return 1;
}
