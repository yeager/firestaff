/*
 * CSB V1 save ensure_dir regression (Bug C fix).
 *
 * The default CSB V1 save directory (~/Library/Application Support/
 * Firestaff/csb/saves/ on macOS, %APPDATA%/Firestaff/csb/saves/ on
 * Windows, ~/.local/share/firestaff/csb/saves/ on Linux) must exist
 * before csb_v1_save_game can fopen() a save file inside it.  Before
 * this fix, ensure_save_dir() was a no-op stub and the launcher was
 * the only thing that ever created the directory — a power user who
 * wiped the tree would hit CSB_V1_SAVE_ERR_CANT_CREATE (= -1) on
 * save.  This test creates a temp HOME, removes the save subdir
 * (simulating a wiped tree), and asserts that csb_v1_save_game
 * succeeds anyway because ensure_save_dir is now called.
 *
 * Source-lock: ReDMCSB LOADSAVE.C F0433 STARTEND_SaveGame assumes
 * the save directory is writable; the original DM1/CSB installers
 * and CSBWin both create it on first run.  The Firestaff launcher
 * also creates it; this test makes the runtime path self-healing.
 */

#include "csb_v1_save_load_pc34_compat.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { passed++; printf("  PASS: %s\n", msg); } \
    else { failed++; printf("  FAIL: %s\n", msg); } \
} while (0)

#define CHECK_EQ(got, want, msg) do { \
    if ((got) == (want)) { passed++; printf("  PASS: %s\n", msg); } \
    else { failed++; printf("  FAIL: %s (got=%d want=%d)\n", msg, (int)(got), (int)(want)); } \
} while (0)

/* Build a minimal valid CSB V1 save header. */
static void build_header(CSB_V1_SaveHeader *h) {
    memset(h, 0, sizeof(*h));
    /* Magic + version are checked by the obfuscation layer; any
     * values will do for this regression since we only care that
     * the fopen() inside csb_v1_save_game succeeds. */
    h->Magic = 0xC5B10001u;     /* CSB V1 magic */
    h->HeaderVersion = 1;
    h->GameID = 0x0001;         /* CSB */
    h->ChampionCount = 0;
}

/* A minimal state buffer: 4 bytes of zeros is enough to prove the
 * save path runs end-to-end. */
static const uint8_t kState[4] = { 0x01, 0x02, 0x03, 0x04 };

/* Returns 1 if the path exists and is a directory, 0 if it does not
 * exist, -1 on error.  Uses stat() so it works for both POSIX and
 * the test environment. */
static int dir_exists(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return S_ISDIR(st.st_mode) ? 1 : -1;
}

/* Recursively remove a directory tree (best-effort; uses rm -rf to
 * keep the test tiny).  Returns 0 on success. */
static int rmdir_recursive(const char *path) {
    char cmd[1024];
    int n = snprintf(cmd, sizeof(cmd), "rm -rf '%s'", path);
    if (n <= 0 || (size_t)n >= sizeof(cmd)) return -1;
    return system(cmd);
}

int main(void) {
    char tmpTemplate[] = "/tmp/firestaff-csb-save-ensure-XXXXXX";
    char *tmpDir;
    char savePath[1024];

    tmpDir = mkdtemp(tmpTemplate);
    if (!tmpDir) {
        printf("FAIL: mkdtemp\n");
        return 1;
    }
    CHECK(setenv("HOME", tmpDir, 1) == 0, "setenv HOME to temp dir");
    /* On Windows the code prefers APPDATA; clear it so HOME wins. */
    CHECK(setenv("APPDATA", "", 1) == 0, "clear APPDATA so HOME is used");

    /* Build the expected save path: <HOME>/.local/share/firestaff/
     * csb/saves/runeforge.sav.  (SAVE_DIR on POSIX is
     * ".local/share/firestaff/csb/saves/".) */
    int n = snprintf(savePath, sizeof(savePath),
                     "%s/.local/share/firestaff/csb/saves/runeforge.sav",
                     tmpDir);
    CHECK(n > 0 && (size_t)n < sizeof(savePath), "build expected save path");

    /* Sanity: the default save dir under <HOME> should not exist yet. */
    char defaultDir[1024];
    snprintf(defaultDir, sizeof(defaultDir),
             "%s/.local/share/firestaff/csb/saves", tmpDir);
    int beforeExists = dir_exists(defaultDir);
    CHECK(beforeExists == 0,
          "save dir does not exist before csb_v1_save_game runs");

    /* Drive csb_v1_save_game — the fix must ensure the parent dir. */
    CSB_V1_SaveHeader header;
    build_header(&header);
    int rc = csb_v1_save_game(savePath, kState, sizeof(kState), &header);
    CHECK_EQ(rc, CSB_V1_SAVE_OK, "csb_v1_save_game returns CSB_V1_SAVE_OK");

    /* After the call, the save directory and the file must exist. */
    int afterExists = dir_exists(defaultDir);
    CHECK(afterExists == 1,
          "save dir was created by ensure_save_dir during the save");

    struct stat st;
    int fileStat = stat(savePath, &st);
    CHECK(fileStat == 0 && st.st_size > 0,
          "save file was written and is non-empty");

    /* Idempotency: wipe the directory and call save again.  This
     * proves ensure_save_dir is invoked every save (not just on
     * the very first one) so a power user who deletes the tree
     * mid-session recovers on the next save. */
    CHECK(rmdir_recursive(defaultDir) == 0, "wipe save dir to test re-create");
    CHECK(dir_exists(defaultDir) == 0, "save dir removed before second save");
    rc = csb_v1_save_game(savePath, kState, sizeof(kState), &header);
    CHECK_EQ(rc, CSB_V1_SAVE_OK,
             "second save after wipe also returns CSB_V1_SAVE_OK");
    CHECK(dir_exists(defaultDir) == 1,
          "save dir was re-created on the second save");

    /* Custom save path in a fresh directory: ensures the parent-of-
     * path branch also creates the directory. */
    char customDir[1024];
    snprintf(customDir, sizeof(customDir), "%s/custom-saves", tmpDir);
    char customPath[1024];
    snprintf(customPath, sizeof(customPath), "%s/quest.sav", customDir);
    CHECK(dir_exists(customDir) == 0, "custom parent dir absent before save");
    rc = csb_v1_save_game(customPath, kState, sizeof(kState), &header);
    CHECK_EQ(rc, CSB_V1_SAVE_OK,
             "save to fresh custom parent dir also returns CSB_V1_SAVE_OK");
    CHECK(dir_exists(customDir) == 1,
          "custom parent dir was created by the parent-of-path branch");

    /* Cleanup. */
    rmdir_recursive(tmpDir);

    if (failed) {
        printf("csb_v1_save_ensure_dir_pc34_compat: %d failure(s), %d pass(es)\n",
               failed, passed);
        return 1;
    }
    printf("csb_v1_save_ensure_dir_pc34_compat: ok (%d pass)\n", passed);
    return 0;
}
