/*
 * CSB V1 save runtime boundary regression.
 *
 * Proves the CSB V1 save layer can validate a save header and load a bounded
 * runtime-state prefix without requiring a full playable dungeon.
 *
 * Source-lock:
 *   ReDMCSB SAVEHEAD.C F0429/F0430 lines 13-95 handle 512-byte save headers.
 *   ReDMCSB LOADSAVE.C F0435 lines ~2665-2724 validates the header and GameID
 *   before reading GLOBAL_DATA and later runtime sections.
 */

#include "csb_v1_save_load_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { passed++; printf("  PASS: %s\n", msg); } \
    else { failed++; printf("  FAIL: %s\n", msg); } \
} while (0)

#define CHECK_EQ(got, want, label) do { \
    if ((got) == (want)) { \
        passed++; printf("  PASS: %s == %d\n", label, (int)(want)); \
    } else { \
        failed++; printf("  FAIL: %s got=%d want=%d\n", label, (int)(got), (int)(want)); \
    } \
} while (0)

static const char *test_save_path(void)
{
    static char path[512];
    const char *tmp = getenv("TMPDIR");
    if (!tmp || !tmp[0]) tmp = ".";
    snprintf(path, sizeof(path),
             "%s/firestaff_csb_v1_save_runtime_boundary_%lu.fsav",
             tmp, (unsigned long)CSB_V1_SAVE_MAGIC_CSB);
    return path;
}

static void build_state(uint8_t *state, size_t size)
{
    size_t i;
    for (i = 0; i < size; i++) {
        state[i] = (uint8_t)((i * 17u + 3u) & 0xffu);
    }
}

static void test_header_only_compatibility_and_bounded_prefix_load(void)
{
    const char *path = test_save_path();
    uint8_t saved_state[96];
    uint8_t loaded_prefix[128];
    CSB_V1_SaveHeader hdr;
    CSB_V1_SaveHeader out_hdr;
    int r;

    remove(path);
    build_state(saved_state, sizeof(saved_state));
    memset(loaded_prefix, 0xa5, sizeof(loaded_prefix));
    memset(&hdr, 0, sizeof(hdr));
    memset(&out_hdr, 0, sizeof(out_hdr));

    r = csb_v1_save_header_build(&hdr, CSB_V1_SAVE_MAGIC_CSB, 0x1234u,
                                  0x87654321u, 11, 22, 3, 1, 4,
                                  0x01020304u, 54321u);
    CHECK_EQ(r, 0, "save header build");

    r = csb_v1_save_game(path, saved_state, (int)sizeof(saved_state), &hdr);
    CHECK_EQ(r, CSB_V1_SAVE_OK, "synthetic save write");

    r = csb_v1_save_verify_compatible(path, CSB_V1_SAVE_MAGIC_CSB, 0x1234u);
    CHECK_EQ(r, CSB_V1_LOAD_OK, "header-only compatible save check");
    CHECK(csb_v1_runtime_can_load_resume_path(path),
          "runtime resume validator accepts native CSB save header");

    r = csb_v1_save_verify_compatible(path, CSB_V1_SAVE_MAGIC_CSB, 0x9999u);
    CHECK_EQ(r, CSB_V1_LOAD_ERR_DIFFERENT_GAME,
             "header-only rejects different GameID");

    r = csb_v1_load_game(path, loaded_prefix, 32, &out_hdr);
    CHECK_EQ(r, CSB_V1_LOAD_OK, "bounded prefix load");
    CHECK(memcmp(loaded_prefix, saved_state, 32) == 0,
          "bounded load copies requested state prefix");
    CHECK(loaded_prefix[32] == 0xa5 && loaded_prefix[127] == 0xa5,
          "bounded load leaves bytes past caller size untouched");
    CHECK(out_hdr.Magic == CSB_V1_SAVE_MAGIC_CSB,
          "bounded load returns CSB header magic");
    CHECK(out_hdr.GameID == 0x1234u,
          "bounded load returns GameID");

    r = csb_v1_load_game(path, NULL, 0, &out_hdr);
    CHECK_EQ(r, CSB_V1_LOAD_OK, "direct header-only load");
    CHECK(out_hdr.DungeonSeed == 0x87654321u,
          "direct header-only load returns header fields");

    remove(path);
}

static void test_truncated_load_and_backup_restore_are_transactional(void)
{
    const char *path = test_save_path();
    const char *backup;
    uint8_t saved_state[96];
    uint8_t replacement_state[96];
    uint8_t destination[96];
    uint8_t untouched_destination[96];
    CSB_V1_SaveHeader hdr;
    CSB_V1_SaveHeader before_header;
    CSB_V1_SaveHeader out_header;
    FILE *f;
    int r;

    remove(path);
    build_state(saved_state, sizeof(saved_state));
    memset(replacement_state, 0x5c, sizeof(replacement_state));
    memset(destination, 0xa5, sizeof(destination));
    memset(untouched_destination, 0xa5, sizeof(untouched_destination));
    memset(&hdr, 0, sizeof(hdr));
    memset(&before_header, 0x3c, sizeof(before_header));
    out_header = before_header;
    r = csb_v1_save_header_build(&hdr, CSB_V1_SAVE_MAGIC_CSB, 0x2345u,
                                  0x12345678u, 1, 2, 3, 0, 2,
                                  0x10203040u, 999u);
    CHECK_EQ(r, 0, "transactional save header build");
    r = csb_v1_save_game(path, saved_state, (int)sizeof(saved_state), &hdr);
    CHECK_EQ(r, CSB_V1_SAVE_OK, "initial native save write");
    r = csb_v1_save_backup(path);
    CHECK_EQ(r, 0, "explicit complete backup before truncation");

    f = fopen(path, "wb");
    CHECK(f != NULL, "open native save for truncation");
    if (f) {
        CHECK(fwrite(&hdr, 1, sizeof(hdr), f) == sizeof(hdr),
              "write valid truncated save header");
        CHECK(fwrite(saved_state, 1, 11, f) == 11,
              "write incomplete native state payload");
        fclose(f);
    }
    r = csb_v1_load_game(path, destination, (int)sizeof(destination),
                         &out_header);
    CHECK_EQ(r, CSB_V1_LOAD_ERR_UNREADABLE,
             "truncated payload fails bounded load");
    CHECK(memcmp(destination, untouched_destination, sizeof(destination)) == 0,
          "truncated payload leaves caller state unchanged");
    CHECK(memcmp(&out_header, &before_header, sizeof(out_header)) == 0,
          "truncated payload leaves caller header unchanged");

    r = csb_v1_save_restore_backup(path);
    CHECK_EQ(r, CSB_V1_LOAD_OK, "restore complete backup after truncation");
    r = csb_v1_save_game(path, replacement_state, (int)sizeof(replacement_state),
                         &hdr);
    CHECK_EQ(r, CSB_V1_SAVE_OK, "replacement save preserves backup");
    backup = csb_v1_save_get_backup_path(path);
    CHECK(backup != NULL, "backup path is available");
    r = csb_v1_save_restore_backup(path);
    CHECK_EQ(r, CSB_V1_LOAD_OK, "restore backup replaces active save");
    memset(destination, 0, sizeof(destination));
    r = csb_v1_load_game(path, destination, (int)sizeof(destination), NULL);
    CHECK_EQ(r, CSB_V1_LOAD_OK, "restored backup loads");
    CHECK(memcmp(destination, saved_state, sizeof(destination)) == 0,
          "restored backup retains original complete state");

    remove(path);
    if (backup) remove(backup);
}

int main(void)
{
    printf("=== CSB V1 Save Runtime Boundary Regression ===\n\n");

    test_header_only_compatibility_and_bounded_prefix_load();
    test_truncated_load_and_backup_restore_are_transactional();

    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
