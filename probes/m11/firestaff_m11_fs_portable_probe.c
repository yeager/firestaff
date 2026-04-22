/*
 * firestaff_m11_fs_portable_probe.c — Verification probe for fs_portable_compat.
 *
 * Tests: JoinPath, ParentDir, NormalizeSeparators, PathExists, FileExists,
 *        DirExists, CreateDirectory(Recursive), GetUserDataDir,
 *        GetUserConfigDir, ResolveDataDir.
 */

#include "fs_portable_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

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
    snprintf(cmd, sizeof(cmd), "rm -rf '%s'", path);
    (void)system(cmd);
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
    snprintf(tmpBase, sizeof(tmpBase), "/tmp/fsp_probe_%d", (int)getpid());
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

    /* Cleanup. */
    rmdir_shallow(tmpBase);

    printf("\n# summary: %d/%d invariants passed\n", g_passed, g_total);
    return g_passed == g_total ? 0 : 1;
}
