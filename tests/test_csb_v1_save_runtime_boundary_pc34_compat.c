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

int main(void)
{
    printf("=== CSB V1 Save Runtime Boundary Regression ===\n\n");

    test_header_only_compatibility_and_bounded_prefix_load();

    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
