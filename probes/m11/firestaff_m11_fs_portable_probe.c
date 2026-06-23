/*
 * firestaff_m11_fs_portable_probe.c — Verification probe for fs_portable_compat.
 *
 * Tests: JoinPath, ParentDir, NormalizeSeparators, PathExists, FileExists,
 *        DirExists, CreateDirectory(Recursive), GetUserDataDir,
 *        GetUserConfigDir, ResolveDataDir.
 */

#include "fs_portable_compat.h"
#include "config_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#include <direct.h>
#include <process.h>
#else
#include <unistd.h>
#endif

static int g_passed = 0;
static int g_total = 0;

static void check(const char* tag, int cond) {
    ++g_total;
    if (cond) {
        ++g_passed;
        printf("[PASS] %s\n", tag);
    } else {
        printf("[FAIL] %s\n", tag);
    }
}

/* Remove a directory tree (test cleanup). Shallow — only handles one level. */
static void rmdir_shallow(const char* path) {
    char cmd[FSP_PATH_MAX + 16];
#if defined(_WIN32)
    snprintf(cmd, sizeof(cmd), "rmdir /s /q \"%s\" >nul 2>nul", path);
#else
    snprintf(cmd, sizeof(cmd), "rm -rf '%s'", path);
#endif
    if(system(cmd)){;}
}

static int portable_setenv(const char* name, const char* value) {
#if defined(_WIN32)
    return _putenv_s(name, value) == 0;
#else
    if (!value) {
        return unsetenv(name) == 0;
    }
    return setenv(name, value, 1) == 0;
#endif
}

static int portable_unsetenv(const char* name) {
#if defined(_WIN32)
    return _putenv_s(name, "") == 0;
#else
    return unsetenv(name) == 0;
#endif
}

int main(void) {
    char buf[FSP_PATH_MAX];
    char buf2[FSP_PATH_MAX];
    char tmpBase[FSP_PATH_MAX];
    int rc;

    printf("=== fs_portable_compat probe ===\n\n");

    /* --- INV_FS_01: JoinPath basic concatenation --- */
    rc = FSP_JoinPath(buf, sizeof(buf), "alpha", "beta");
    check("INV_FS_01 JoinPath basic",
          rc == 1 && strcmp(buf, "alpha/beta") == 0);

    /* --- INV_FS_02: JoinPath trailing separator dedup --- */
    rc = FSP_JoinPath(buf, sizeof(buf), "alpha/", "beta");
    check("INV_FS_02 JoinPath trailing sep dedup",
          rc == 1 && strcmp(buf, "alpha/beta") == 0);

    /* --- INV_FS_03: JoinPath empty left --- */
    rc = FSP_JoinPath(buf, sizeof(buf), "", "beta");
    check("INV_FS_03 JoinPath empty left",
          rc == 1 && strcmp(buf, "beta") == 0);

    /* --- INV_FS_04: JoinPath null safety --- */
    rc = FSP_JoinPath(buf, sizeof(buf), NULL, "beta");
    check("INV_FS_04 JoinPath null safety", rc == 0);

    /* --- INV_FS_05: JoinPath truncation guard --- */
    rc = FSP_JoinPath(buf, 5, "abc", "defgh");
    check("INV_FS_05 JoinPath truncation guard", rc == 0);

    /* --- INV_FS_06: ParentDir extracts parent --- */
    rc = FSP_ParentDir(buf, sizeof(buf), "/usr/local/bin");
    check("INV_FS_06 ParentDir extracts parent",
          rc == 1 && strcmp(buf, "/usr/local") == 0);

    /* --- INV_FS_07: ParentDir root case --- */
    rc = FSP_ParentDir(buf, sizeof(buf), "/file");
    check("INV_FS_07 ParentDir root case",
          rc == 1 && strcmp(buf, "/") == 0);

    /* --- INV_FS_08: ParentDir no separator --- */
    rc = FSP_ParentDir(buf, sizeof(buf), "nodir");
    check("INV_FS_08 ParentDir no separator", rc == 0);

    /* --- INV_FS_09: NormalizeSeparators on POSIX --- */
    {
        char testPath[32];
        snprintf(testPath, sizeof(testPath), "a\\b\\c");
        FSP_NormalizeSeparators(testPath);
#if defined(_WIN32)
        check("INV_FS_09 NormalizeSeparators",
              strcmp(testPath, "a\\b\\c") == 0);
#else
        check("INV_FS_09 NormalizeSeparators",
              strcmp(testPath, "a/b/c") == 0);
#endif
    }

    /* --- INV_FS_10: NormalizeSeparators null safe --- */
    check("INV_FS_10 NormalizeSeparators null safe",
          FSP_NormalizeSeparators(NULL) == NULL);

    /* --- INV_FS_11: PathExists on current dir --- */
    check("INV_FS_11 PathExists cwd", FSP_PathExists(".") == 1);

    /* --- INV_FS_12: PathExists on nonexistent --- */
    check("INV_FS_12 PathExists nonexistent",
          FSP_PathExists("/no/such/path/ever") == 0);

    /* --- INV_FS_13: FileExists / DirExists distinction --- */
    check("INV_FS_13a DirExists on cwd", FSP_DirExists(".") == 1);
    check("INV_FS_13b FileExists on cwd", FSP_FileExists(".") == 0);

    /* --- INV_FS_14: CreateDirectory + DirExists --- */
#if defined(_WIN32)
    snprintf(tmpBase, sizeof(tmpBase), ".\\fsp_probe_%d", (int)_getpid());
#else
    snprintf(tmpBase, sizeof(tmpBase), "/tmp/fsp_probe_%d", (int)getpid());
#endif
    rmdir_shallow(tmpBase);
    rc = FSP_CreateDirectory(tmpBase);
    check("INV_FS_14 CreateDirectory", rc == 1 && FSP_DirExists(tmpBase));

    /* --- INV_FS_15: CreateDirectory idempotent --- */
    rc = FSP_CreateDirectory(tmpBase);
    check("INV_FS_15 CreateDirectory idempotent", rc == 1);

    /* --- INV_FS_16: CreateDirectoryRecursive nested --- */
    rc = FSP_JoinPath(buf, sizeof(buf), tmpBase, "a/b/c");
    if (rc) {
        rc = FSP_CreateDirectoryRecursive(buf);
        check("INV_FS_16 CreateDirectoryRecursive nested",
              rc == 1 && FSP_DirExists(buf));
    } else {
        check("INV_FS_16 CreateDirectoryRecursive nested (join failed)", 0);
    }

    /* --- INV_FS_17: FileExists after writing a test file --- */
    rc = FSP_JoinPath(buf2, sizeof(buf2), tmpBase, "testfile.txt");
    if (rc) {
        FILE* f = fopen(buf2, "w");
        if (f) {
            fprintf(f, "hello\n");
            fclose(f);
        }
        check("INV_FS_17 FileExists after write",
              FSP_FileExists(buf2) == 1);
    } else {
        check("INV_FS_17 FileExists after write (join failed)", 0);
    }

    /* --- INV_FS_18: GetUserDataDir returns non-empty --- */
    rc = FSP_GetUserDataDir(buf, sizeof(buf));
    check("INV_FS_18 GetUserDataDir non-empty",
          rc == 1 && buf[0] != '\0');
    printf("  -> user data dir: %s\n", buf);

    /* --- INV_FS_19: GetUserConfigDir returns non-empty --- */
    rc = FSP_GetUserConfigDir(buf, sizeof(buf));
    check("INV_FS_19 GetUserConfigDir non-empty",
          rc == 1 && buf[0] != '\0');
    printf("  -> user config dir: %s\n", buf);

    /* --- INV_FS_20: ResolveDataDir with explicit dir --- */
    rc = FSP_ResolveDataDir(buf, sizeof(buf), "/explicit/path");
    check("INV_FS_20 ResolveDataDir explicit",
          rc == 1 && strcmp(buf, "/explicit/path") == 0);

    /* --- INV_FS_21: ResolveDataDir falls through to env or default --- */
    rc = FSP_ResolveDataDir(buf, sizeof(buf), NULL);
    check("INV_FS_21 ResolveDataDir fallback non-empty",
          rc == 1 && buf[0] != '\0');
    printf("  -> resolved data dir: %s\n", buf);

    /* --- INV_FS_22: ResolveDataDir empty string treated as unset --- */
    rc = FSP_ResolveDataDir(buf, sizeof(buf), "");
    check("INV_FS_22 ResolveDataDir empty string",
          rc == 1 && buf[0] != '\0');

    /* --- INV_FS_23: Env override keeps explicit runtime data roots --- */
    rc = portable_setenv("FIRESTAFF_DATA", "/tmp/firestaff-explicit-data");
    if (rc) {
        rc = FSP_ResolveDataDir(buf, sizeof(buf), NULL);
        check("INV_FS_23 ResolveDataDir FIRESTAFF_DATA override",
              rc == 1 && strcmp(buf, "/tmp/firestaff-explicit-data") == 0);
    } else {
        check("INV_FS_23 ResolveDataDir FIRESTAFF_DATA override (setenv failed)", 0);
    }
    (void)portable_unsetenv("FIRESTAFF_DATA");

    /* --- INV_FS_24: Public release default data root --- */
#if defined(_WIN32)
    rc = FSP_ResolveDataDir(buf, sizeof(buf), NULL);
    check("INV_FS_24 ResolveDataDir Windows install data",
          rc == 1 &&
          strstr(buf, "\\data") != NULL &&
          strstr(buf, "firestaff-test-no-assets") == NULL &&
          strstr(buf, "AppData") == NULL);
#else
    rc = portable_setenv("HOME", tmpBase);
    if (rc) {
        rc = FSP_ResolveDataDir(buf, sizeof(buf), NULL);
        rc = rc && FSP_JoinPath(buf2, sizeof(buf2), tmpBase, ".firestaff/data");
        check("INV_FS_24 ResolveDataDir POSIX home data",
              rc == 1 &&
              strcmp(buf, buf2) == 0 &&
              strstr(buf, "firestaff-test-no-assets") == NULL);
    } else {
        check("INV_FS_24 ResolveDataDir POSIX home data (setenv failed)", 0);
    }
#endif

    /* --- INV_FS_25: M12 config defaults use game data, not originals/test roots --- */
    {
        M12_Config config;
        M12_Config_SetDefaults(&config);
#if defined(_WIN32)
        check("INV_FS_25 M12 config default data dir",
              strstr(config.dataDir, "\\data") != NULL &&
              strstr(config.dataDir, "\\originals") == NULL &&
              strstr(config.dataDir, "firestaff-test-no-assets") == NULL);
#else
        check("INV_FS_25 M12 config default data dir",
              strcmp(config.dataDir, buf2) == 0 &&
              strstr(config.dataDir, "firestaff-test-no-assets") == NULL);
#endif
    }

    /* Cleanup. */
    rmdir_shallow(tmpBase);

    printf("\n# summary: %d/%d invariants passed\n", g_passed, g_total);
    return g_passed == g_total ? 0 : 1;
}
