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
#include <dirent.h>
#define FSP_SEP '/'
#define FSP_ALT_SEP '\\'
#endif

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

/* ── Internal helpers ───────────────────────────────────────────────── */

static int fsp_ascii_name_equal(const char* a, const char* b) {
    while (*a && *b) {
        unsigned char x = (unsigned char)*a++, y = (unsigned char)*b++;
        if (x >= 'A' && x <= 'Z') x = (unsigned char)(x + ('a' - 'A'));
        if (y >= 'A' && y <= 'Z') y = (unsigned char)(y + ('a' - 'A'));
        if (x != y) return 0;
    }
    return *a == *b;
}

static int fsp_unique_file_candidate(const char* directory, const char* wanted,
                                      const char* name, char* selected,
                                      int found) {
    char candidate[FSP_PATH_MAX];
    if (!fsp_ascii_name_equal(wanted, name)) return found;
    if (!FSP_JoinPath(candidate, sizeof(candidate), directory, name)) return -1;
    if (!FSP_FileExists(candidate)) return found;
    if (found) return -2;
    strcpy(selected, candidate);
    return 1;
}

int FSP_FindUniqueFileIgnoreCase(const char* directory, const char* name,
                                 char* out, size_t outSize) {
    char selected[FSP_PATH_MAX] = {0};
    int found = 0;
    if (!out || !outSize) return -1;
    out[0] = '\0';
    if (!directory || !name || !name[0] || strchr(name, '/') ||
        strchr(name, '\\') || strchr(name, ':')) return -1;
#ifdef _WIN32
    WIN32_FIND_DATAA entry;
    HANDLE search;
    char pattern[FSP_PATH_MAX];
    if (!FSP_JoinPath(pattern, sizeof(pattern), directory, "*")) return -1;
    search = FindFirstFileA(pattern, &entry);
    if (search == INVALID_HANDLE_VALUE)
        return GetLastError() == ERROR_FILE_NOT_FOUND ? 0 : -1;
    do {
        found = fsp_unique_file_candidate(directory, name, entry.cFileName,
                                           selected, found);
        if (found < 0) break;
    } while (FindNextFileA(search, &entry));
    if (found >= 0 && GetLastError() != ERROR_NO_MORE_FILES) found = -1;
    FindClose(search);
#else
    DIR* dir = opendir(directory);
    struct dirent* entry;
    if (!dir) return -1;
    for (;;) {
        errno = 0;
        entry = readdir(dir);
        if (!entry) { if (errno) found = -1; break; }
        found = fsp_unique_file_candidate(directory, name, entry->d_name,
                                           selected, found);
        if (found < 0) break;
    }
    if (closedir(dir) != 0 && found >= 0) found = -1;
#endif
    if (found <= 0) return found;
    if (strlen(selected) >= outSize) return 0;
    strcpy(out, selected);
    return 1;
}

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

int FSP_ReplaceFile(const char* temporaryPath, const char* destinationPath) {
    if (!temporaryPath || !temporaryPath[0] ||
        !destinationPath || !destinationPath[0]) {
        return 0;
    }
#if defined(_WIN32)
    /* Unlike POSIX rename, the Windows CRT rename rejects existing targets.
     * Match the narrow-path encoding used to create the temporary file. */
    return MoveFileExA(temporaryPath, destinationPath,
                      MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
    return rename(temporaryPath, destinationPath) == 0;
#endif
}

int FSP_JoinPath(char* out, size_t outSize,
                 const char* left, const char* right) {
    char leftCopy[FSP_PATH_MAX];
    char rightCopy[FSP_PATH_MAX];
    int rc;
    size_t leftLen;
    if (!out || outSize == 0U || !left || !right) {
        return 0;
    }
    /* Several asset callers intentionally extend a path in place, e.g.
     * FSP_JoinPath(path, sizeof(path), path, "modern"). snprintf() does
     * not permit its destination to overlap a %s argument, so preserve both
     * segments before producing the joined path. */
    if (strlen(left) >= sizeof(leftCopy) || strlen(right) >= sizeof(rightCopy)) {
        return 0;
    }
    memcpy(leftCopy, left, strlen(left) + 1U);
    memcpy(rightCopy, right, strlen(right) + 1U);
    left = leftCopy;
    right = rightCopy;
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

int FSP_ResolvePhysicalPath(char* out, size_t outSize, const char* path) {
    if (!out || outSize == 0U || !path || path[0] == '\0') {
        return 0;
    }
#if defined(_WIN32)
    {
        DWORD written = GetFullPathNameA(path, (DWORD)outSize, out, NULL);
        if (written == 0U || (size_t)written >= outSize) {
            out[0] = '\0';
            return 0;
        }
        return 1;
    }
#else
    {
        char *resolved = realpath(path, NULL);
        if (!resolved) {
            out[0] = '\0';
            return 0;
        }
        if (strlen(resolved) >= outSize) {
            free(resolved);
            out[0] = '\0';
            return 0;
        }
        fsp_copy(out, outSize, resolved);
        free(resolved);
        return 1;
    }
#endif
}

static int fsp_path_within(const char* path, const char* root) {
    size_t root_len;
    size_t i;
    if (!path || !root || !path[0] || !root[0]) return 0;
    root_len = strlen(root);
    while (root_len > 1U && fsp_is_separator(root[root_len - 1U])) {
        --root_len;
    }
    if (strlen(path) < root_len) return 0;
    for (i = 0; i < root_len; ++i) {
        unsigned char a = (unsigned char)path[i];
        unsigned char b = (unsigned char)root[i];
#if defined(_WIN32)
        if (a >= 'A' && a <= 'Z') a = (unsigned char)(a + ('a' - 'A'));
        if (b >= 'A' && b <= 'Z') b = (unsigned char)(b + ('a' - 'A'));
#endif
        if (a != b) return 0;
    }
    return path[root_len] == '\0' || fsp_is_separator(path[root_len]);
}

static int fsp_game_id_is_safe_component(const char* gameId) {
    const unsigned char* p = (const unsigned char*)gameId;
    if (!p || !p[0]) return 0;
    while (*p) {
        if (!((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') ||
              (*p >= '0' && *p <= '9') || *p == '-' || *p == '_')) {
            return 0;
        }
        ++p;
    }
    return 1;
}

static int fsp_modern_dir_from_data_root(char* out, size_t outSize,
                                         const char* dataRoot,
                                         const char* gameId) {
    char assetsRoot[FSP_PATH_MAX];
    char gameRoot[FSP_PATH_MAX];
    if (!dataRoot || !dataRoot[0] ||
        !FSP_ParentDir(assetsRoot, sizeof(assetsRoot), dataRoot) ||
        !FSP_JoinPath(assetsRoot, sizeof(assetsRoot), assetsRoot, "assets") ||
        !FSP_JoinPath(gameRoot, sizeof(gameRoot), assetsRoot, gameId) ||
        !FSP_JoinPath(out, outSize, gameRoot, "modern")) {
        return 0;
    }
    return 1;
}

static int fsp_modern_dir_from_data_path(char* out, size_t outSize,
                                         const char* dataDir,
                                         const char* gameId) {
    char parent1[FSP_PATH_MAX];
    char parent2[FSP_PATH_MAX];
    char assetsRoot[FSP_PATH_MAX];
    char gameRoot[FSP_PATH_MAX];
    if (!FSP_ParentDir(parent1, sizeof(parent1), dataDir) ||
        !FSP_ParentDir(parent2, sizeof(parent2), parent1)) {
        if (!FSP_JoinPath(assetsRoot, sizeof(assetsRoot), dataDir, "assets"))
            return 0;
    } else if (!FSP_JoinPath(assetsRoot, sizeof(assetsRoot), parent2,
                             "assets")) {
        return 0;
    }
    return FSP_JoinPath(gameRoot, sizeof(gameRoot), assetsRoot, gameId) &&
           FSP_JoinPath(out, outSize, gameRoot, "modern");
}

int FSP_ResolveGameModernAssetDir(char* out, size_t outSize,
                                  const char* dataDir,
                                  const char* gameId) {
    char configuredDataRoot[FSP_PATH_MAX];
    char physicalDataRoot[FSP_PATH_MAX];
    char physicalDataDir[FSP_PATH_MAX];
    char logicalModernDir[FSP_PATH_MAX];
    char physicalModernDir[FSP_PATH_MAX];
    char manifest[FSP_PATH_MAX];
    if (!out || outSize == 0U || !dataDir || !dataDir[0] ||
        !fsp_game_id_is_safe_component(gameId)) {
        return 0;
    }
    out[0] = '\0';

    /* A catalog may hand the runtime a realpath below a symlinked default
     * ~/.firestaff/data root. Map that physical child back to the configured
     * logical profile before falling back to dataDir-relative custom roots. */
    if (FSP_GetDefaultOriginalsDir(configuredDataRoot,
                                   sizeof(configuredDataRoot)) &&
        FSP_ResolvePhysicalPath(physicalDataRoot, sizeof(physicalDataRoot),
                                configuredDataRoot) &&
        FSP_ResolvePhysicalPath(physicalDataDir, sizeof(physicalDataDir),
                                dataDir) &&
        fsp_path_within(physicalDataDir, physicalDataRoot) &&
        fsp_modern_dir_from_data_root(logicalModernDir,
                                      sizeof(logicalModernDir),
                                      configuredDataRoot, gameId)) {
        snprintf(out, outSize, "%s", logicalModernDir);
        return strlen(logicalModernDir) < outSize;
    }

    /* Keep custom/scratch root behavior, including the prior CSB preference
     * for an existing logical manifest over a physical sibling manifest. */
    if (!fsp_modern_dir_from_data_path(logicalModernDir,
                                       sizeof(logicalModernDir), dataDir,
                                       gameId)) {
        return 0;
    }
    if (FSP_JoinPath(manifest, sizeof(manifest), logicalModernDir,
                     "modern_asset_manifest.json") &&
        FSP_FileExists(manifest)) {
        snprintf(out, outSize, "%s", logicalModernDir);
        return strlen(logicalModernDir) < outSize;
    }
    if (FSP_ResolvePhysicalPath(physicalDataDir, sizeof(physicalDataDir),
                                dataDir) &&
        fsp_modern_dir_from_data_path(physicalModernDir,
                                      sizeof(physicalModernDir),
                                      physicalDataDir, gameId) &&
        FSP_JoinPath(manifest, sizeof(manifest), physicalModernDir,
                     "modern_asset_manifest.json") &&
        FSP_FileExists(manifest)) {
        snprintf(out, outSize, "%s", physicalModernDir);
        return strlen(physicalModernDir) < outSize;
    }
    snprintf(out, outSize, "%s", logicalModernDir);
    return strlen(logicalModernDir) < outSize;
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

    /* A process may select a separate originals root without changing the
     * desktop profile. Tests use this to avoid scanning installed media. */
    {
        const char* override = getenv("FIRESTAFF_ORIGINALS_DIR");
        if (override && override[0] != '\0') {
            size_t length = strlen(override);
            out[0] = '\0';
            if (length >= outSize) return 0;
            memcpy(out, override, length + 1U);
            return 1;
        }
    }

#if defined(__ANDROID__)
    /* Android: app-private external storage under Documents/Firestaff/data.
     * SDL_GetPrefPath is not available here (no SDL dependency), so use the
     * standard Android external storage path. Users transfer game data via
     * USB/file manager to this location. */
    {
        const char* extStorage = getenv("EXTERNAL_STORAGE");
        if (extStorage && extStorage[0] != '\0') {
            rc = snprintf(out, outSize, "%s/Documents/Firestaff/data", extStorage);
            return rc > 0 && (size_t)rc < outSize;
        }
    }
    fsp_copy(out, outSize, "/sdcard/Documents/Firestaff/data");
    return 1;
#elif defined(__APPLE__) && TARGET_OS_IOS
    /* iOS: Documents/Firestaff/data inside the app sandbox.
     * Users transfer game data via the Files app or iTunes File Sharing. */
    {
        const char* home = getenv("HOME");
        if (home && home[0] != '\0') {
            rc = snprintf(out, outSize, "%s/Documents/Firestaff/data", home);
            return rc > 0 && (size_t)rc < outSize;
        }
    }
    fsp_copy(out, outSize, "./data");
    return 1;
#elif defined(_WIN32)
    {
        const char* userProfile = getenv("USERPROFILE");
        if (userProfile && userProfile[0] != '\0') {
            rc = snprintf(out, outSize, "%s\\.firestaff\\data", userProfile);
            return rc > 0 && (size_t)rc < outSize;
        }
    }
    {
        const char* appData = getenv("APPDATA");
        if (appData && appData[0] != '\0') {
            rc = snprintf(out, outSize, "%s\\Firestaff\\data", appData);
            return rc > 0 && (size_t)rc < outSize;
        }
    }
    fsp_copy(out, outSize, ".\\data");
    return 1;
#else
    {
        const char* home = getenv("HOME");
        if (home && home[0] != '\0') {
            rc = snprintf(out, outSize, "%s/.firestaff/data", home);
            return rc > 0 && (size_t)rc < outSize;
        }
    }
    fsp_copy(out, outSize, "./data");
    return 1;
#endif
}

int FSP_ResolveDataDir(char* out, size_t outSize, const char* requestedDir) {
    const char* envData;

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

    /* Priority 3: platform-specific default. */
    return FSP_GetDefaultOriginalsDir(out, outSize);
}

int FSP_SetEnv(const char* name, const char* value, int overwrite) {
    if (!name || !value) return -1;
#if defined(_WIN32)
    if (!overwrite) {
        /* Match POSIX setenv overwrite=0 semantics: only set if unset. */
        size_t envSize;
        getenv_s(&envSize, NULL, 0, name);
        if (envSize > 0) return 0;
    }
    {
        /* _putenv takes "NAME=VALUE" string. _putenv_s takes (name, value)
         * separately. We use _putenv_s when available (UCRT). */
#if defined(_putenv_s)
        return (_putenv_s(name, value) == 0) ? 0 : -1;
#else
        char buf[2048];
        int n = snprintf(buf, sizeof(buf), "%s=%s", name, value);
        if (n <= 0 || (size_t)n >= sizeof(buf)) return -1;
        return (_putenv(buf) == 0) ? 0 : -1;
#endif
    }
#else
    return setenv(name, value, overwrite);
#endif
}

int FSP_UnsetEnv(const char* name) {
    if (!name) return -1;
#if defined(_WIN32)
    {
        char buf[2048];
        int n = snprintf(buf, sizeof(buf), "%s=", name);
        if (n <= 0 || (size_t)n >= sizeof(buf)) return -1;
        return (_putenv(buf) == 0) ? 0 : -1;
    }
#else
    return unsetenv(name);
#endif
}
