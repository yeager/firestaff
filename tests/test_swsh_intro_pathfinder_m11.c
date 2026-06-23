/*
 * test_swsh_intro_pathfinder_m11.c
 *
 * DM1 V1 v2.7.4 release regression: the FTL/SWSH intro was silently
 * skipped when SWOOSH lived anywhere other than a tiny hard-coded
 * $HOME/.openclaw/data/firestaff-original-games anchor or
 * <dataDir>/SWOOSH. The most common layout — DM1 data in
 * $HOME/.firestaff/data/dm1/SWOOSH, or a canonical PC 3.4 install
 * where SWOOSH sits next to DATA/ — was missed. The finder is now
 * M11_SWSH_Intro_FindLogoPath() in src/engine/swsh_intro_pathfinder_m11.c
 * and this test pins every search branch the v2.7.4 release path relied
 * on.
 *
 * Synthetic, no copyrighted data required: tests build tiny MZ-shaped
 * or source-shaped raw logo payloads in /tmp.
 */

#include "swsh_intro_pathfinder_m11.h"

#include "menu_startup_m12.h"
#include "fs_portable_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#define MKDIR(path) mkdir((path), 0700)
#endif

static int failures = 0;
static int tests = 0;

static int test_setenv(const char* name, const char* value) {
#ifdef _WIN32
    return _putenv_s(name, value ? value : "");
#else
    return setenv(name, value ? value : "", 1);
#endif
}

static int test_unsetenv(const char* name) {
#ifdef _WIN32
    return _putenv_s(name, "");
#else
    return unsetenv(name);
#endif
}

#define CHECK(expr) do { \
    tests++; \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        failures++; \
    } \
} while (0)

static size_t append_run(unsigned char* buf, size_t pos, unsigned int count, unsigned int color) {
    while (count > 0u) {
        unsigned int chunk = count > 320u ? 320u : count;
        if (chunk <= 8u) {
            buf[pos++] = (unsigned char)(((chunk - 1u) << 4) | (color & 0x0fu));
        } else if (chunk <= 256u) {
            buf[pos++] = (unsigned char)(0x80u | (color & 0x0fu));
            buf[pos++] = (unsigned char)(chunk - 1u);
        } else {
            buf[pos++] = (unsigned char)(0xc0u | (color & 0x0fu));
            buf[pos++] = (unsigned char)(((chunk - 1u) >> 8) & 0xffu);
            buf[pos++] = (unsigned char)((chunk - 1u) & 0xffu);
        }
        count -= chunk;
    }
    return pos;
}

static size_t build_synthetic_source_shaped_swoosh(unsigned char* buf, size_t cap) {
    unsigned int row;
    size_t pos = 0u;
    if (cap < 1600u) return 0u;
    buf[pos++] = 0x40u;
    buf[pos++] = 0x01u;
    buf[pos++] = 0xc8u;
    buf[pos++] = 0x00u;
    for (row = 0u; row < 51u; ++row) pos = append_run(buf, pos, 320u, 0u);
    for (row = 0u; row < 119u; ++row) {
        pos = append_run(buf, pos, 24u, 0u);
        pos = append_run(buf, pos, 82u, 15u);
        pos = append_run(buf, pos, 163u, 0u);
        pos = append_run(buf, pos, 1u, 15u);
        pos = append_run(buf, pos, 50u, 0u);
    }
    for (row = 0u; row < 30u; ++row) pos = append_run(buf, pos, 320u, 0u);
    return pos;
}

/* Build a fake MZ-wrapped SWOOSH payload: an MZ header + padding + a
 * raw IMG2 320x200 logo stream embedded somewhere in the file. */
static int write_mz_wrapped_swoosh(const char* path) {
    unsigned char buf[2048];
    unsigned char logo[1600];
    FILE* f;
    size_t wrote;
    size_t logoBytes;
    memset(buf, 0, sizeof(buf));
    buf[0] = 'M';
    buf[1] = 'Z';
    logoBytes = build_synthetic_source_shaped_swoosh(logo, sizeof(logo));
    if (logoBytes == 0u || 64u + logoBytes > sizeof(buf)) return 0;
    memcpy(buf + 64u, logo, logoBytes);
    f = fopen(path, "wb");
    if (!f) return 0;
    wrote = fwrite(buf, 1, sizeof(buf), f);
    fclose(f);
    return wrote == sizeof(buf);
}

/* Build a raw PC IMG2 320x200 logo stream (no MZ wrapper). */
static int write_raw_img1_swoosh(const char* path) {
    unsigned char buf[1600];
    FILE* f;
    size_t wrote;
    size_t logoBytes;
    memset(buf, 0, sizeof(buf));
    logoBytes = build_synthetic_source_shaped_swoosh(buf, sizeof(buf));
    if (logoBytes == 0u) return 0;
    f = fopen(path, "wb");
    if (!f) return 0;
    wrote = fwrite(buf, 1, logoBytes, f);
    fclose(f);
    return wrote == logoBytes;
}

/* Junk payload: not MZ, not 320x200, just garbage. Must be rejected
 * so a stray "SWOOSH" file in a stale data root cannot break the
 * intro by being treated as a valid logo. */
static int write_junk_swoosh(const char* path) {
    unsigned char buf[64];
    FILE* f;
    size_t wrote;
    memset(buf, 0xA5u, sizeof(buf));
    f = fopen(path, "wb");
    if (!f) return 0;
    wrote = fwrite(buf, 1, sizeof(buf), f);
    fclose(f);
    return wrote == sizeof(buf);
}

static void rm_rf(const char* path) {
    char cmd[1024];
    if (!path || path[0] == '\0') return;
    snprintf(cmd, sizeof(cmd), "rm -rf '%s' 2>/dev/null", path);
    (void)system(cmd);
}

static int mkdir_p(const char* path) {
    char tmp[1024];
    size_t i;
    if (!path || path[0] == '\0') return 0;
    snprintf(tmp, sizeof(tmp), "%s", path);
    for (i = 1; i < sizeof(tmp) && tmp[i] != '\0'; ++i) {
        if (tmp[i] == '/') {
            tmp[i] = '\0';
            if (MKDIR(tmp) != 0) {
                /* ignore EEXIST */
            }
            tmp[i] = '/';
        }
    }
    if (MKDIR(tmp) != 0) {
        /* ignore EEXIST */
    }
    return 1;
}

static void reset_menu(M12_StartupMenuState* state) {
    memset(state, 0, sizeof(*state));
}

/* Seed a minimal menuState with one DM1 version whose matchedPath
 * points to a fake GRAPHICS.DAT. */
static void seed_dm1_matched_path(M12_StartupMenuState* state, const char* matchedGraphicsPath) {
    M12_AssetVersionStatus* v;
    if (!state) return;
    reset_menu(state);
    snprintf(state->assetStatus.dataDir, sizeof(state->assetStatus.dataDir),
             "%s", matchedGraphicsPath[0] ? "/tmp/firestaff-test-swsh-data" : ".");
    v = &state->assetStatus.versions[0][0];
    memset(v, 0, sizeof(*v));
    v->gameId = "dm1";
    v->versionId = "pc34-en";
    v->label = "PC 3.4 English";
    v->shortLabel = "PC 3.4 EN";
    v->matched = 1;
    snprintf(v->matchedPath, sizeof(v->matchedPath), "%s", matchedGraphicsPath);
    v->matchedMd5[0] = 'f';
    v->matchedMd5[1] = 'a';
    v->matchedMd5[2] = '\0';
}

/* ─── Tests ───────────────────────────────────────────────────────── */

/* (A) Junk payload must be rejected. The v2.7.4 path accepted any
 * existing "SWOOSH" file regardless of content. The FinderIsolatedState
 * helper forces the search to ignore real $HOME SWOOSH so the test is
 * deterministic on developer machines that already have a real anchor. */
typedef struct FinderIsolatedState {
    char oldHome[1024];
    char oldDataDir[1024];
    int hadHome;
    int hadDataDir;
} FinderIsolatedState;

static void finder_isolate(FinderIsolatedState* s) {
    const char* home = getenv("HOME");
    if (home) {
        snprintf(s->oldHome, sizeof(s->oldHome), "%s", home);
        s->hadHome = 1;
    } else {
        s->hadHome = 0;
    }
    (void)test_setenv("HOME", "/nonexistent-firestaff-test-isolation-home");
    /* Wipe FIRESTAFF_SWOOSH so the env override does not short-circuit. */
    {
        const char* e = getenv("FIRESTAFF_SWOOSH");
        if (e) {
            snprintf(s->oldDataDir, sizeof(s->oldDataDir), "%s", e);
            s->hadDataDir = 1;
        } else {
            s->hadDataDir = 0;
        }
        (void)test_unsetenv("FIRESTAFF_SWOOSH");
    }
}

static void finder_restore(const FinderIsolatedState* s) {
    if (s->hadHome) {
        (void)test_setenv("HOME", s->oldHome);
    } else {
        (void)test_unsetenv("HOME");
    }
    if (s->hadDataDir) {
        (void)test_setenv("FIRESTAFF_SWOOSH", s->oldDataDir);
    } else {
        (void)test_unsetenv("FIRESTAFF_SWOOSH");
    }
}

static void test_payload_looks_valid_rejects_junk(void) {
    const char* path = "/tmp/firestaff-test-swsh-junk.bin";
    char out[FSP_PATH_MAX];
    FinderIsolatedState iso;
    rm_rf(path);
    CHECK(write_junk_swoosh(path) == 1);
    CHECK(M11_SWSH_Intro_PayloadLooksValid(path) == 0);
    finder_isolate(&iso);
    CHECK(M11_SWSH_Intro_FindLogoPath(NULL, "/tmp", out, sizeof(out)) == 0);
    finder_restore(&iso);
    rm_rf(path);
}

/* (B) MZ-wrapped SWOOSH must be accepted by the validator. */
static void test_payload_looks_valid_accepts_mz(void) {
    const char* path = "/tmp/firestaff-test-swsh-mz.bin";
    rm_rf(path);
    CHECK(write_mz_wrapped_swoosh(path) == 1);
    CHECK(M11_SWSH_Intro_PayloadLooksValid(path) == 1);
    rm_rf(path);
}

/* (C) Raw source-shaped 320x200 SWOOSH must be accepted. */
static void test_payload_looks_valid_accepts_raw_img1(void) {
    const char* path = "/tmp/firestaff-test-swsh-raw.bin";
    rm_rf(path);
    CHECK(write_raw_img1_swoosh(path) == 1);
    CHECK(M11_SWSH_Intro_PayloadLooksValid(path) == 1);
    rm_rf(path);
}

/* (D) The asset-catalog branch must locate SWOOSH next to the
 * matched GRAPHICS.DAT (the parent-dir layout from PC 3.4 installs
 * where SWOOSH lives beside DATA/). */
static void test_finder_uses_asset_matched_path_parent(void) {
    char graphPath[1024];
    char swooshPath[1024];
    char out[FSP_PATH_MAX];
    M12_StartupMenuState state;
    rm_rf("/tmp/firestaff-test-swsh-asset-parent");
    CHECK(mkdir_p("/tmp/firestaff-test-swsh-asset-parent/dm1") == 1);
    snprintf(graphPath, sizeof(graphPath),
             "/tmp/firestaff-test-swsh-asset-parent/dm1/GRAPHICS.DAT");
    snprintf(swooshPath, sizeof(swooshPath),
             "/tmp/firestaff-test-swsh-asset-parent/dm1/SWOOSH");
    CHECK(write_raw_img1_swoosh(swooshPath) == 1);
    seed_dm1_matched_path(&state, graphPath);
    CHECK(M11_SWSH_Intro_FindLogoPath(&state, NULL, out, sizeof(out)) == 1);
    CHECK(strcmp(out, swooshPath) == 0);
    rm_rf("/tmp/firestaff-test-swsh-asset-parent");
}

/* (E) Asset-catalog branch must also locate SWOOSH at the
 * grandparent of the matched GRAPHICS.DAT (PC 3.4 install root:
 * GRAPHICS.DAT is in .../DungeonMasterPC34/DATA/, SWOOSH is in
 * .../DungeonMasterPC34/, two parent dirs up from GRAPHICS.DAT). */
static void test_finder_uses_asset_matched_path_grandparent(void) {
    char graphPath[1024];
    char swooshPath[1024];
    char out[FSP_PATH_MAX];
    M12_StartupMenuState state;
    rm_rf("/tmp/firestaff-test-swsh-asset-grand");
    CHECK(mkdir_p("/tmp/firestaff-test-swsh-asset-grand/DungeonMasterPC34/DATA") == 1);
    snprintf(graphPath, sizeof(graphPath),
             "/tmp/firestaff-test-swsh-asset-grand/DungeonMasterPC34/DATA/GRAPHICS.DAT");
    snprintf(swooshPath, sizeof(swooshPath),
             "/tmp/firestaff-test-swsh-asset-grand/DungeonMasterPC34/SWOOSH");
    CHECK(write_mz_wrapped_swoosh(swooshPath) == 1);
    seed_dm1_matched_path(&state, graphPath);
    CHECK(M11_SWSH_Intro_FindLogoPath(&state, NULL, out, sizeof(out)) == 1);
    CHECK(strcmp(out, swooshPath) == 0);
    rm_rf("/tmp/firestaff-test-swsh-asset-grand");
}

/* (F) dataDir/dm1/SWOOSH must be found. This is the v2.7.4 regression:
 * the previous finder only checked dataDir/SWOOSH, which silently
 * missed the structured $HOME/.firestaff/data/dm1/ layout. */
static void test_finder_uses_data_dir_dm1_subdir(void) {
    const char* dataDir = "/tmp/firestaff-test-swsh-datadir-dm1";
    char swooshPath[1024];
    char out[FSP_PATH_MAX];
    M12_StartupMenuState state;
    rm_rf(dataDir);
    CHECK(mkdir_p("/tmp/firestaff-test-swsh-datadir-dm1/dm1") == 1);
    snprintf(swooshPath, sizeof(swooshPath), "%s/dm1/SWOOSH", dataDir);
    CHECK(write_raw_img1_swoosh(swooshPath) == 1);
    reset_menu(&state);
    CHECK(M11_SWSH_Intro_FindLogoPath(&state, dataDir, out, sizeof(out)) == 1);
    CHECK(strcmp(out, swooshPath) == 0);
    rm_rf(dataDir);
}

/* (G) Junk at dataDir/dm1/SWOOSH must be rejected; a real SWOOSH in
 * the asset-catalog grandparent branch must be preferred. */
static void test_finder_rejects_junk_prefers_valid(void) {
    char graphPath[1024];
    char junkSwoosh[1024];
    char validSwoosh[1024];
    char out[FSP_PATH_MAX];
    M12_StartupMenuState state;
    rm_rf("/tmp/firestaff-test-swsh-junk-vs-valid");
    CHECK(mkdir_p("/tmp/firestaff-test-swsh-junk-vs-valid/dm1") == 1);
    CHECK(mkdir_p("/tmp/firestaff-test-swsh-junk-vs-valid/DungeonMasterPC34/DATA") == 1);
    snprintf(junkSwoosh, sizeof(junkSwoosh),
             "/tmp/firestaff-test-swsh-junk-vs-valid/dm1/SWOOSH");
    snprintf(graphPath, sizeof(graphPath),
             "/tmp/firestaff-test-swsh-junk-vs-valid/DungeonMasterPC34/DATA/GRAPHICS.DAT");
    snprintf(validSwoosh, sizeof(validSwoosh),
             "/tmp/firestaff-test-swsh-junk-vs-valid/DungeonMasterPC34/SWOOSH");
    CHECK(write_junk_swoosh(junkSwoosh) == 1);
    CHECK(write_mz_wrapped_swoosh(validSwoosh) == 1);
    seed_dm1_matched_path(&state, graphPath);
    CHECK(M11_SWSH_Intro_FindLogoPath(&state, NULL, out, sizeof(out)) == 1);
    CHECK(strcmp(out, validSwoosh) == 0);
    CHECK(strcmp(out, junkSwoosh) != 0);
    rm_rf("/tmp/firestaff-test-swsh-junk-vs-valid");
}

/* (H) The canonical $HOME OpenClaw original-games anchor must still
 * be discovered when no menuState or dataDir is provided. This is the
 * last-resort fallback that v2.7.4's narrow in-tree search relied
 * on, and the regression pinned by this test ensures we did not lose
 * it during the refactor. */
static void test_finder_uses_home_canonical_anchor(void) {
    const char* home = getenv("HOME");
    char out[FSP_PATH_MAX];
    char swooshPath[1024];
    char canonicalDir[1024];
    char canonicalSwoosh[1024];
    FILE* f;
    if (!home || home[0] == '\0') {
        printf("SKIP home anchor test: HOME unset\n");
        return;
    }
    snprintf(canonicalDir, sizeof(canonicalDir),
             "%s/.openclaw/data/firestaff-original-games/DM/_canonical/dm1",
             home);
    snprintf(swooshPath, sizeof(swooshPath), "%s/SWOOSH", canonicalDir);
    snprintf(canonicalSwoosh, sizeof(canonicalSwoosh),
             "%s/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/SWOOSH",
             home);
    if (access(swooshPath, F_OK) == 0) {
        M12_StartupMenuState state;
        reset_menu(&state);
        CHECK(M11_SWSH_Intro_FindLogoPath(&state, NULL, out, sizeof(out)) == 1);
        CHECK(strcmp(out, canonicalSwoosh) == 0);
        return;
    }
    /* Fall back to laying down a synthetic canonical SWOOSH so the
     * test is meaningful in CI. We restore the original afterwards
     * to avoid clobbering real user data. */
    if (mkdir_p(canonicalDir) != 1) {
        printf("SKIP home anchor test: cannot mkdir %s\n", canonicalDir);
        return;
    }
    /* Save a backup copy if a real SWOOSH is present (so we restore it
     * at the end). If the file is junk/missing we just write a fake
     * one and skip the restore. */
    {
        char backupPath[1024];
        int hadReal = 0;
        snprintf(backupPath, sizeof(backupPath), "%s.SWSH_TEST_BACKUP", swooshPath);
        f = fopen(swooshPath, "rb");
        if (f) {
            fclose(f);
            rename(swooshPath, backupPath);
            hadReal = 1;
        }
        CHECK(write_mz_wrapped_swoosh(swooshPath) == 1);
        {
            M12_StartupMenuState state;
            reset_menu(&state);
            CHECK(M11_SWSH_Intro_FindLogoPath(&state, NULL, out, sizeof(out)) == 1);
            CHECK(strcmp(out, swooshPath) == 0);
        }
        /* Restore: remove the synthetic file, put any backup back. */
        unlink(swooshPath);
        if (hadReal) {
            rename(backupPath, swooshPath);
        }
    }
}

/* (I) Null/empty inputs must be safe. Uses FinderIsolatedState to
 * ignore the developer-machine $HOME SWOOSH so the no-result case
 * is reproducible. */
static void test_finder_handles_nulls(void) {
    char out[FSP_PATH_MAX];
    FinderIsolatedState iso;
    finder_isolate(&iso);
    out[0] = '\0';
    CHECK(M11_SWSH_Intro_FindLogoPath(NULL, NULL, NULL, 0) == 0);
    CHECK(M11_SWSH_Intro_FindLogoPath(NULL, NULL, out, sizeof(out)) == 0);
    CHECK(out[0] == '\0');
    CHECK(M11_SWSH_Intro_PayloadLooksValid(NULL) == 0);
    CHECK(M11_SWSH_Intro_PayloadLooksValid("") == 0);
    CHECK(M11_SWSH_Intro_PayloadLooksValid("/nonexistent/path/SWOOSH") == 0);
    finder_restore(&iso);
}

/* (J) FIRESTAFF_SWOOSH env override must be honoured when it points
 * to a real payload. */
static void test_finder_honours_env_override(void) {
    const char* path = "/tmp/firestaff-test-swsh-env.bin";
    char out[FSP_PATH_MAX];
    char saved[1024];
    int hadSaved = 0;
    const char* prev;
    rm_rf(path);
    CHECK(write_mz_wrapped_swoosh(path) == 1);
    prev = getenv("FIRESTAFF_SWOOSH");
    if (prev) {
        snprintf(saved, sizeof(saved), "%s", prev);
        hadSaved = 1;
    }
    (void)test_setenv("FIRESTAFF_SWOOSH", path);
    CHECK(M11_SWSH_Intro_FindLogoPath(NULL, NULL, out, sizeof(out)) == 1);
    CHECK(strcmp(out, path) == 0);
    if (hadSaved) {
        (void)test_setenv("FIRESTAFF_SWOOSH", saved);
    } else {
        (void)test_unsetenv("FIRESTAFF_SWOOSH");
    }
    rm_rf(path);
}

int main(void) {
    test_payload_looks_valid_rejects_junk();
    test_payload_looks_valid_accepts_mz();
    test_payload_looks_valid_accepts_raw_img1();
    test_finder_uses_asset_matched_path_parent();
    test_finder_uses_asset_matched_path_grandparent();
    test_finder_uses_data_dir_dm1_subdir();
    test_finder_rejects_junk_prefers_valid();
    test_finder_uses_home_canonical_anchor();
    test_finder_handles_nulls();
    test_finder_honours_env_override();
    printf("swsh_intro_pathfinder_m11 tests: %d run, %d failed\n", tests, failures);
    return failures == 0 ? 0 : 1;
}
