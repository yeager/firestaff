/*
 * test_csb_v22_finished_art_material_gate_pc34.c
 *
 * CSB V2.2 finished-art / material screenshot pixel gate - synthetic
 * CTest. Builds synthetic modern_asset_manifest.json fixtures under
 * /tmp/scratch/csb_v22_famg/ and exercises every classification /
 * gate branch the gate exposes:
 *
 *   1. Unset manifest path         -> NO_MANIFEST gate, all slots MISSING
 *   2. Set path, no manifest file  -> NO_MANIFEST gate
 *   3. Empty manifest              -> SYNTHETIC_PLACEHOLDER gate
 *   4. Placeholder-only manifest   -> SYNTHETIC_PLACEHOLDER gate
 *   5. Mixed placeholder + missing -> SYNTHETIC_PLACEHOLDER gate
 *      (declared slots are all PLACEHOLDER, others MISSING)
 *   6. PARTIAL (one slot REAL, others PLACEHOLDER) -> PARTIAL gate
 *   7. PARTIAL (real metadata, missing file)       -> SYNTHETIC_PLACEHOLDER gate
 *      (the manifest declares pbr_hero but the file is not on disk;
 *       PARTIAL-only slots route through the placeholder fallback,
 *       so the gate stays SYNTHETIC_PLACEHOLDER)
 *   8. PARTIAL (incomplete fields)  -> SYNTHETIC_PLACEHOLDER gate
 *   9. All-real manifest            -> FINISHED_REAL gate
 *  10. Garbage manifest             -> NO_MANIFEST (parser rejects)
 *  11. validate_manifest returns -1 / 0 / 1 as documented
 *  12. uses_placeholder() is 1 for non-REAL, 0 for REAL
 *  13. get_slot_info() populates inline fields when slot is present
 *  14. real_count() returns slot counts correctly
 *  15. installed flag mirrors gate state
 *  16. is_finished_real() / is_synthetic_or_partial() mirror gate state
 *  17. NULL / out-of-range inputs are safe
 *  18. Source-evidence citation contains the documented anchors
 *  19. CSB-specific slot coverage: CSB-only chaos_rune and
 *      door_prison slots dispatch through their CSB-only categories
 *      (chaos_runes / door_shapes) so the gate tracks the
 *      CSBWin/Chaos.cpp:60-69 dispatch surface
 *
 * Source-locked against the module under test
 * include/csb_v22_finished_art_material_gate_pc34.h.
 * Sibling gate pattern: tests/test_dm1_v22_finished_art_material_
 * gate_pc34.c and tests/test_dm2_v22_finished_art_material_gate_pc34.c.
 */

#include "csb_v22_finished_art_material_gate_pc34.h"
#include "fs_portable_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Test harness ───────────────────────────────────────────────── */

static int s_pass = 0;
static int s_fail = 0;

#define CHECK(expr, msg)                                                  \
    do {                                                                  \
        s_pass++;                                                         \
        if (!(expr)) {                                                    \
            fprintf(stderr, "FAIL %s:%d: %s -- %s\n",                      \
                    __FILE__, __LINE__, #expr, (msg));                    \
            s_fail++;                                                     \
        }                                                                 \
    } while (0)

/* Helper: write a string to a file (returns 1 on success). */
static int write_file(const char* path, const char* content) {
    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;
    size_t len = strlen(content);
    size_t written = fwrite(content, 1, len, fp);
    fclose(fp);
    return (written == len);
}

/* Slot fixture table - mirrors k_slot_table in the module under test. */
typedef struct {
    CSB_V22_FamgSlot slot;
    const char* id;
    const char* category;
    int width;
    int height;
} SlotFixture;

static const SlotFixture k_slots[] = {
    { CSB_V22_FAMG_WALL_DUNGEON,         "wall_dungeon_01",         "wall_shapes",        96, 96 },
    { CSB_V22_FAMG_FLOOR_PLAIN,          "floor_plain_01",          "floor_shapes",       96, 96 },
    { CSB_V22_FAMG_FLOOR_CRACKED,        "floor_cracked_01",        "floor_shapes",       96, 96 },
    { CSB_V22_FAMG_CREATURE_CHAOS_FIEND, "creature_chaos_fiend_01", "creature_shapes",    64, 64 },
    { CSB_V22_FAMG_PANEL_LORD_ORDER,     "panel_lord_order_01",     "ui_chrome",         128, 32 },
    { CSB_V22_FAMG_CHAMPION_WARRIOR_CSB, "champion_warrior_csb_01", "champion_portraits", 64, 64 },
    { CSB_V22_FAMG_DOOR_PRISON,          "door_prison_01",          "door_shapes",        64, 96 },
    { CSB_V22_FAMG_CHAOS_RUNE,           "chaos_rune_01",           "chaos_runes",        32, 32 }
};

/* Helper: build the manifest path that the module resolves for a given
 * data dir. Mirrors csb_v22_famg_set_manifest_path. */
static void build_expected_manifest_path(char* out, size_t outSize,
                                         const char* dataDir) {
    /* dataDir = <root>/data/csb -> manifest = <root>/assets/csb/modern/modern_asset_manifest.json
     * Walk up two parents. */
    char a[FSP_PATH_MAX];
    char b[FSP_PATH_MAX];
    const char* slash;
    slash = strrchr(dataDir, '/');
    if (!slash) {
        snprintf(out, outSize, "%s/assets/csb/modern/modern_asset_manifest.json", dataDir);
        return;
    }
    size_t la = (size_t)(slash - dataDir);
    if (la >= sizeof(a)) la = sizeof(a) - 1U;
    memcpy(a, dataDir, la); a[la] = '\0';
    slash = strrchr(a, '/');
    if (!slash) {
        snprintf(out, outSize, "%s/assets/csb/modern/modern_asset_manifest.json", dataDir);
        return;
    }
    size_t lb = (size_t)(slash - a);
    if (lb >= sizeof(b)) lb = sizeof(b) - 1U;
    memcpy(b, a, lb); b[lb] = '\0';
    snprintf(out, outSize, "%s/assets/csb/modern/modern_asset_manifest.json", b);
}

/* Helper: clean the scratch dir before each scenario. We also remove
 * the asset tree the module looks at so the slot's source_file resolve
 * matches the manifest we author below. */
static void clean_scratch(void) {
    system("rm -rf /tmp/scratch/csb-v22-famg-test");
}

static void parent_dir(char* out, size_t outSize, const char* path) {
    const char* slash = strrchr(path, '/');
    if (!slash) {
        snprintf(out, outSize, ".");
        return;
    }
    size_t n = (size_t)(slash - path);
    if (n >= outSize) n = outSize - 1U;
    memcpy(out, path, n);
    out[n] = '\0';
}

static void setup_manifest_dirs(const char* dataDir,
                                char* manifestPath, size_t manifestPathSize,
                                char* modernDir, size_t modernDirSize) {
    build_expected_manifest_path(manifestPath, manifestPathSize, dataDir);
    parent_dir(modernDir, modernDirSize, manifestPath);
    CHECK(FSP_CreateDirectoryRecursive(modernDir) == 1, "created modern dir");
    for (size_t i = 0; i < CSB_V22_FAMG_MATERIAL_COUNT; ++i) {
        char dir[FSP_PATH_MAX];
        FSP_JoinPath(dir, sizeof(dir), modernDir, k_slots[i].category);
        CHECK(FSP_CreateDirectoryRecursive(dir) == 1, "created category dir");
    }
}

static void source_filename(char* out, size_t outSize, const SlotFixture* s) {
    snprintf(out, outSize, "%s.png", s->id);
}

static void source_path(char* out, size_t outSize, const char* modernDir,
                        const SlotFixture* s) {
    char file[128];
    char dir[FSP_PATH_MAX];
    source_filename(file, sizeof(file), s);
    FSP_JoinPath(dir, sizeof(dir), modernDir, s->category);
    FSP_JoinPath(out, outSize, dir, file);
}

static void create_real_file(const char* modernDir, const SlotFixture* s) {
    char path[FSP_PATH_MAX];
    source_path(path, sizeof(path), modernDir, s);
    CHECK(write_file(path, "fake-csb-v22-finished-art-bytes"),
          "created source_file on disk");
}

/* Write a manifest. realMask bit i -> pbr_hero, else placeholder.
 * firstOnly=1 -> emit only slot 0. missingFirst=1 -> slot 0 has
 * a non-existent source_file. omitDimsFirst=1 -> slot 0 omits width
 * + height fields (partial metadata). */
static int write_manifest(const char* path,
                          int realMask,
                          int firstOnly,
                          int missingFirst,
                          int omitDimsFirst) {
    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;
    fprintf(fp, "{\"manifestVersion\":\"1.0.0\",\"packId\":\"csb-v22-famg-test\",\"slots\":[");
    int count = firstOnly ? 1 : (int)CSB_V22_FAMG_MATERIAL_COUNT;
    for (int i = 0; i < count; ++i) {
        const SlotFixture* s = &k_slots[i];
        const char* gen = (realMask & (1 << i)) ? "pbr_hero" : "placeholder";
        char file[128];
        source_filename(file, sizeof(file), s);
        if (missingFirst && i == 0) {
            snprintf(file, sizeof(file), "missing_%s.png", s->id);
        }
        if (i > 0) fprintf(fp, ",");
        fprintf(fp, "{\"id\":\"%s\",\"generator\":\"%s\",\"source_file\":\"%s\"",
                s->id, gen, file);
        if (!(omitDimsFirst && i == 0)) {
            fprintf(fp, ",\"width\":%d,\"height\":%d", s->width, s->height);
        }
        fprintf(fp, "}");
    }
    fprintf(fp, "]}");
    fclose(fp);
    return 1;
}

/* ── Test scenarios ─────────────────────────────────────────────── */

static void test_unset_manifest_path(void) {
    const char* dataDir = "/tmp/scratch/csb-v22-famg-test/data/csb";
    clean_scratch();
    FSP_CreateDirectoryRecursive(dataDir);
    csb_v22_famg_set_manifest_path(NULL);
    CHECK(csb_v22_famg_get_manifest_path()[0] == '\0',
          "unset path returns empty manifest path");
    CHECK(csb_v22_famg_gate() == CSB_V22_FAMG_GATE_NO_MANIFEST,
          "unset path -> NO_MANIFEST gate");
    CHECK(csb_v22_famg_get_installed() == 0,
          "unset path -> installed=0");
    CHECK(csb_v22_famg_is_finished_real() == 0,
          "unset path -> not finished");
    CHECK(csb_v22_famg_is_synthetic_or_partial() == 0,
          "unset path -> not synthetic-or-partial");
    for (size_t i = 0; i < CSB_V22_FAMG_MATERIAL_COUNT; ++i) {
        CHECK(csb_v22_famg_classify_slot(k_slots[i].slot) ==
              CSB_V22_FAMG_CLASS_MISSING,
              "unset path -> slot MISSING");
    }
}

static void test_missing_manifest_file(void) {
    const char* dataDir = "/tmp/scratch/csb-v22-famg-test/data/csb";
    char manifestPath[FSP_PATH_MAX];
    char modernDir[FSP_PATH_MAX];
    clean_scratch();
    setup_manifest_dirs(dataDir,
                        manifestPath, sizeof(manifestPath),
                        modernDir, sizeof(modernDir));
    /* No manifest file written. */
    csb_v22_famg_set_manifest_path(dataDir);
    CHECK(strcmp(csb_v22_famg_get_manifest_path(), manifestPath) == 0,
          "manifest path matches expected");
    CHECK(csb_v22_famg_gate() == CSB_V22_FAMG_GATE_NO_MANIFEST,
          "missing manifest -> NO_MANIFEST gate");
    CHECK(csb_v22_famg_validate_manifest(NULL) == -1,
          "missing manifest validate -> -1");
}

static void test_garbage_manifest(void) {
    const char* dataDir = "/tmp/scratch/csb-v22-famg-test/data/csb";
    char manifestPath[FSP_PATH_MAX];
    char modernDir[FSP_PATH_MAX];
    clean_scratch();
    setup_manifest_dirs(dataDir,
                        manifestPath, sizeof(manifestPath),
                        modernDir, sizeof(modernDir));
    csb_v22_famg_set_manifest_path(dataDir);
    CHECK(write_file(manifestPath, "this is not json at all"),
          "wrote garbage manifest");
    CHECK(csb_v22_famg_validate_manifest(NULL) == -1,
          "garbage manifest validate -> -1");
    CHECK(csb_v22_famg_gate() == CSB_V22_FAMG_GATE_NO_MANIFEST,
          "garbage manifest -> NO_MANIFEST gate");
}

static void test_empty_manifest(void) {
    const char* dataDir = "/tmp/scratch/csb-v22-famg-test/data/csb";
    char manifestPath[FSP_PATH_MAX];
    char modernDir[FSP_PATH_MAX];
    clean_scratch();
    setup_manifest_dirs(dataDir,
                        manifestPath, sizeof(manifestPath),
                        modernDir, sizeof(modernDir));
    csb_v22_famg_set_manifest_path(dataDir);
    CHECK(write_file(manifestPath, "{}"),
          "wrote empty manifest");
    CHECK(csb_v22_famg_validate_manifest(NULL) == 0,
          "empty manifest validate -> 0");
    CHECK(csb_v22_famg_gate() == CSB_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER,
          "empty manifest -> SYNTHETIC_PLACEHOLDER gate");
    CHECK(csb_v22_famg_classify_slot(CSB_V22_FAMG_WALL_DUNGEON) ==
          CSB_V22_FAMG_CLASS_MISSING,
          "empty manifest -> first slot MISSING");
}

static void test_placeholder_manifest(void) {
    const char* dataDir = "/tmp/scratch/csb-v22-famg-test/data/csb";
    char manifestPath[FSP_PATH_MAX];
    char modernDir[FSP_PATH_MAX];
    clean_scratch();
    setup_manifest_dirs(dataDir,
                        manifestPath, sizeof(manifestPath),
                        modernDir, sizeof(modernDir));
    csb_v22_famg_set_manifest_path(dataDir);
    CHECK(write_manifest(manifestPath, 0, 0, 0, 0),
          "wrote placeholder manifest");
    CHECK(csb_v22_famg_validate_manifest(NULL) == 1,
          "placeholder manifest validate -> 1");
    CHECK(csb_v22_famg_gate() == CSB_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER,
          "placeholder manifest -> SYNTHETIC_PLACEHOLDER gate");
    for (size_t i = 0; i < CSB_V22_FAMG_MATERIAL_COUNT; ++i) {
        CHECK(csb_v22_famg_classify_slot(k_slots[i].slot) ==
              CSB_V22_FAMG_CLASS_PLACEHOLDER,
              "placeholder manifest -> slot PLACEHOLDER");
    }
    CHECK(csb_v22_famg_is_synthetic_or_partial() == 1,
          "placeholder manifest -> synthetic helper");
    CHECK(csb_v22_famg_get_installed() == 0,
          "placeholder manifest -> installed=0");
}

static void test_partial_manifest(void) {
    const char* dataDir = "/tmp/scratch/csb-v22-famg-test/data/csb";
    char manifestPath[FSP_PATH_MAX];
    char modernDir[FSP_PATH_MAX];
    int total = 0;
    int real = 0;
    clean_scratch();
    setup_manifest_dirs(dataDir,
                        manifestPath, sizeof(manifestPath),
                        modernDir, sizeof(modernDir));
    csb_v22_famg_set_manifest_path(dataDir);
    create_real_file(modernDir, &k_slots[0]);
    CHECK(write_manifest(manifestPath, 1, 0, 0, 0),
          "wrote one-real partial manifest");
    CHECK(csb_v22_famg_gate() == CSB_V22_FAMG_GATE_PARTIAL,
          "one-real -> PARTIAL gate");
    real = csb_v22_famg_real_count(&total);
    CHECK(real == 1, "partial -> real_count=1");
    CHECK(total == (int)CSB_V22_FAMG_MATERIAL_COUNT,
          "partial -> total matches material count");
    CHECK(csb_v22_famg_uses_placeholder(CSB_V22_FAMG_WALL_DUNGEON) == 0,
          "partial -> real slot no placeholder");
    CHECK(csb_v22_famg_uses_placeholder(CSB_V22_FAMG_FLOOR_PLAIN) == 1,
          "partial -> placeholder slot uses placeholder");
    CHECK(csb_v22_famg_get_installed() == 1,
          "partial -> installed=1");
}

static void test_missing_file_partial(void) {
    const char* dataDir = "/tmp/scratch/csb-v22-famg-test/data/csb";
    char manifestPath[FSP_PATH_MAX];
    char modernDir[FSP_PATH_MAX];
    clean_scratch();
    setup_manifest_dirs(dataDir,
                        manifestPath, sizeof(manifestPath),
                        modernDir, sizeof(modernDir));
    csb_v22_famg_set_manifest_path(dataDir);
    /* Manifest declares pbr_hero for slot 0 but the source_file
     * points to a path that is not on disk -> PARTIAL classification. */
    CHECK(write_manifest(manifestPath, 1, 1, 1, 0),
          "wrote missing-file manifest");
    CHECK(csb_v22_famg_classify_slot(CSB_V22_FAMG_WALL_DUNGEON) ==
          CSB_V22_FAMG_CLASS_PARTIAL,
          "missing-file -> first slot PARTIAL");
    CHECK(csb_v22_famg_gate() == CSB_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER,
          "partial-only -> SYNTHETIC_PLACEHOLDER gate");
}

static void test_incomplete_metadata(void) {
    const char* dataDir = "/tmp/scratch/csb-v22-famg-test/data/csb";
    char manifestPath[FSP_PATH_MAX];
    char modernDir[FSP_PATH_MAX];
    clean_scratch();
    setup_manifest_dirs(dataDir,
                        manifestPath, sizeof(manifestPath),
                        modernDir, sizeof(modernDir));
    csb_v22_famg_set_manifest_path(dataDir);
    create_real_file(modernDir, &k_slots[0]);
    CHECK(write_manifest(manifestPath, 1, 1, 0, 1),
          "wrote omit-dims partial manifest");
    CHECK(csb_v22_famg_classify_slot(CSB_V22_FAMG_WALL_DUNGEON) ==
          CSB_V22_FAMG_CLASS_PARTIAL,
          "omit-dims -> slot PARTIAL");
    CHECK(csb_v22_famg_gate() == CSB_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER,
          "omit-dims -> SYNTHETIC_PLACEHOLDER gate");
}

static void test_finished_real_manifest(void) {
    const char* dataDir = "/tmp/scratch/csb-v22-famg-test/data/csb";
    char manifestPath[FSP_PATH_MAX];
    char modernDir[FSP_PATH_MAX];
    int total = 0;
    int real = 0;
    clean_scratch();
    setup_manifest_dirs(dataDir,
                        manifestPath, sizeof(manifestPath),
                        modernDir, sizeof(modernDir));
    csb_v22_famg_set_manifest_path(dataDir);
    for (size_t i = 0; i < CSB_V22_FAMG_MATERIAL_COUNT; ++i) {
        create_real_file(modernDir, &k_slots[i]);
    }
    {
        int allMask = (1 << CSB_V22_FAMG_MATERIAL_COUNT) - 1;
        CHECK(write_manifest(manifestPath, allMask, 0, 0, 0),
              "wrote all-real manifest");
    }
    CHECK(csb_v22_famg_gate() == CSB_V22_FAMG_GATE_FINISHED_REAL,
          "all-real -> FINISHED_REAL gate");
    real = csb_v22_famg_real_count(&total);
    CHECK(real == (int)CSB_V22_FAMG_MATERIAL_COUNT,
          "all-real -> real_count matches count");
    CHECK(total == (int)CSB_V22_FAMG_MATERIAL_COUNT,
          "all-real -> total matches count");
    CHECK(csb_v22_famg_is_finished_real() == 1,
          "all-real -> is_finished_real");
    CHECK(csb_v22_famg_is_synthetic_or_partial() == 0,
          "all-real -> not synthetic-or-partial");
    CHECK(csb_v22_famg_get_installed() == 1,
          "all-real -> installed=1");
    for (size_t i = 0; i < CSB_V22_FAMG_MATERIAL_COUNT; ++i) {
        CHECK(csb_v22_famg_classify_slot(k_slots[i].slot) ==
              CSB_V22_FAMG_CLASS_REAL,
              "all-real -> slot REAL");
    }
}

static void test_get_slot_info(void) {
    const char* dataDir = "/tmp/scratch/csb-v22-famg-test/data/csb";
    char manifestPath[FSP_PATH_MAX];
    char modernDir[FSP_PATH_MAX];
    CSB_V22_FamgSlotInfo info;
    clean_scratch();
    setup_manifest_dirs(dataDir,
                        manifestPath, sizeof(manifestPath),
                        modernDir, sizeof(modernDir));
    csb_v22_famg_set_manifest_path(dataDir);
    create_real_file(modernDir, &k_slots[0]);
    CHECK(write_manifest(manifestPath, 1, 1, 0, 0),
          "wrote one-real manifest for slot_info");
    memset(&info, 0, sizeof(info));
    CHECK(csb_v22_famg_get_slot_info(CSB_V22_FAMG_WALL_DUNGEON, &info) == 1,
          "get_slot_info -> 1");
    CHECK(info.slot == CSB_V22_FAMG_WALL_DUNGEON,
          "get_slot_info -> slot set");
    CHECK(strcmp(info.id, "wall_dungeon_01") == 0,
          "get_slot_info -> id populated");
    CHECK(strcmp(info.category, "wall_shapes") == 0,
          "get_slot_info -> category populated");
    CHECK(strcmp(info.generator, "pbr_hero") == 0,
          "get_slot_info -> generator populated");
    CHECK(info.width == 96 && info.height == 96,
          "get_slot_info -> width/height populated");
    CHECK(info.file_exists == 1,
          "get_slot_info -> file_exists=1 when source resolves");
    CHECK(info.classification == CSB_V22_FAMG_CLASS_REAL,
          "get_slot_info -> classification REAL");
}

static void test_slot_info_unknown_assets(void) {
    const char* dataDir = "/tmp/scratch/csb-v22-famg-test/data/csb";
    char manifestPath[FSP_PATH_MAX];
    char modernDir[FSP_PATH_MAX];
    CSB_V22_FamgSlotInfo info;
    clean_scratch();
    setup_manifest_dirs(dataDir,
                        manifestPath, sizeof(manifestPath),
                        modernDir, sizeof(modernDir));
    csb_v22_famg_set_manifest_path(dataDir);
    CHECK(write_manifest(manifestPath, 0, 0, 0, 0),
          "wrote placeholder manifest");
    CHECK(csb_v22_famg_get_slot_info(CSB_V22_FAMG_DOOR_PRISON, &info) == 1,
          "placeholder slot -> get_slot_info=1");
    CHECK(strcmp(info.generator, "placeholder") == 0,
          "placeholder slot -> generator is placeholder");
    CHECK(strcmp(info.category, "door_shapes") == 0,
          "placeholder slot -> category is door_shapes (CSB-only)");
    CHECK(info.classification == CSB_V22_FAMG_CLASS_PLACEHOLDER,
          "placeholder slot -> PLACEHOLDER");
}

static void test_null_and_oob_safety(void) {
    /* Null arguments must not crash. */
    CHECK(csb_v22_famg_get_slot_info(CSB_V22_FAMG_WALL_DUNGEON, NULL) == 0,
          "null out -> 0");
    CHECK(csb_v22_famg_classify_slot((CSB_V22_FamgSlot)9999) ==
          CSB_V22_FAMG_CLASS_UNKNOWN,
          "out-of-range slot -> UNKNOWN");
    CHECK(strcmp(csb_v22_famg_slot_name((CSB_V22_FamgSlot)9999), "UNKNOWN") == 0,
          "out-of-range slot -> name UNKNOWN");
    CHECK(csb_v22_famg_slot_category((CSB_V22_FamgSlot)9999)[0] == '\0',
          "out-of-range slot -> category empty");
    CHECK(csb_v22_famg_slot_manifest_id((CSB_V22_FamgSlot)9999)[0] == '\0',
          "out-of-range slot -> manifest_id empty");
    CHECK(csb_v22_famg_uses_placeholder((CSB_V22_FamgSlot)9999) == 1,
          "out-of-range slot -> uses_placeholder=1");
    {
        int total = -1;
        int real = csb_v22_famg_real_count(&total);
        /* With no manifest path set, total is 0 not -1 */
        csb_v22_famg_set_manifest_path(NULL);
        real = csb_v22_famg_real_count(&total);
        CHECK(real == 0, "no manifest -> real_count=0");
        CHECK(total == 0, "no manifest -> total=0");
    }
}

static void test_names_and_evidence(void) {
    CHECK(strcmp(csb_v22_famg_class_name(CSB_V22_FAMG_CLASS_UNKNOWN), "UNKNOWN") == 0,
          "class UNKNOWN name");
    CHECK(strcmp(csb_v22_famg_class_name(CSB_V22_FAMG_CLASS_MISSING), "MISSING") == 0,
          "class MISSING name");
    CHECK(strcmp(csb_v22_famg_class_name(CSB_V22_FAMG_CLASS_PLACEHOLDER), "PLACEHOLDER") == 0,
          "class PLACEHOLDER name");
    CHECK(strcmp(csb_v22_famg_class_name(CSB_V22_FAMG_CLASS_PARTIAL), "PARTIAL") == 0,
          "class PARTIAL name");
    CHECK(strcmp(csb_v22_famg_class_name(CSB_V22_FAMG_CLASS_REAL), "REAL") == 0,
          "class REAL name");
    CHECK(strcmp(csb_v22_famg_class_name((CSB_V22_FamgClass)999), "INVALID") == 0,
          "class INVALID name");

    CHECK(strcmp(csb_v22_famg_gate_name(CSB_V22_FAMG_GATE_NOT_PROBED), "NOT_PROBED") == 0,
          "gate NOT_PROBED name");
    CHECK(strcmp(csb_v22_famg_gate_name(CSB_V22_FAMG_GATE_NO_MANIFEST), "NO_MANIFEST") == 0,
          "gate NO_MANIFEST name");
    CHECK(strcmp(csb_v22_famg_gate_name(CSB_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER), "SYNTHETIC_PLACEHOLDER") == 0,
          "gate SYNTHETIC_PLACEHOLDER name");
    CHECK(strcmp(csb_v22_famg_gate_name(CSB_V22_FAMG_GATE_PARTIAL), "PARTIAL") == 0,
          "gate PARTIAL name");
    CHECK(strcmp(csb_v22_famg_gate_name(CSB_V22_FAMG_GATE_FINISHED_REAL), "FINISHED_REAL") == 0,
          "gate FINISHED_REAL name");
    CHECK(strcmp(csb_v22_famg_gate_name((CSB_V22_FamgGate)999), "INVALID") == 0,
          "gate INVALID name");

    for (size_t i = 0; i < CSB_V22_FAMG_MATERIAL_COUNT; ++i) {
        CHECK(strcmp(csb_v22_famg_slot_name(k_slots[i].slot), k_slots[i].id) == 0,
              "slot name matches manifest id");
        CHECK(strcmp(csb_v22_famg_slot_category(k_slots[i].slot), k_slots[i].category) == 0,
              "slot category matches fixture");
        CHECK(strcmp(csb_v22_famg_slot_manifest_id(k_slots[i].slot), k_slots[i].id) == 0,
              "slot manifest_id matches manifest id");
    }

    /* Source evidence must cite the documented anchors. */
    const char* ev = csb_v22_famg_source_evidence();
    CHECK(ev != NULL, "source evidence non-null");
    CHECK(strstr(ev, "DUNVIEW.C F0128") != NULL,
          "evidence cites CSB DUNVIEW.C F0128");
    CHECK(strstr(ev, "PANEL.C F0354") != NULL,
          "evidence cites CSB PANEL.C F0354");
    CHECK(strstr(ev, "COMMAND.C:108-113") != NULL,
          "evidence cites CSB COMMAND.C");
    CHECK(strstr(ev, "CSBWin/Viewport.cpp:7290") != NULL,
          "evidence cites CSBWin viewport");
    CHECK(strstr(ev, "CSBWin/Chaos.cpp:60-69") != NULL,
          "evidence cites CSBWin Chaos dispatch");
    CHECK(strstr(ev, "csb_v22_inplace_draw") != NULL,
          "evidence cites in-place module");
    CHECK(strstr(ev, "FIRESTAFF_GAP_LIST") != NULL,
          "evidence cites gap list");
    CHECK(strstr(ev, "Honest boundary") != NULL,
          "evidence states honest boundary");
}

static void test_installed_setter(void) {
    csb_v22_famg_set_installed(1);
    CHECK(csb_v22_famg_get_installed() == 1, "setter 1 -> 1");
    csb_v22_famg_set_installed(0);
    CHECK(csb_v22_famg_get_installed() == 0, "setter 0 -> 0");
    csb_v22_famg_set_installed(42);
    CHECK(csb_v22_famg_get_installed() == 1, "setter non-zero -> 1");
}

static void test_validate_at_explicit_path(void) {
    const char* dataDir = "/tmp/scratch/csb-v22-famg-test/data/csb";
    char manifestPath[FSP_PATH_MAX];
    char modernDir[FSP_PATH_MAX];
    clean_scratch();
    setup_manifest_dirs(dataDir,
                        manifestPath, sizeof(manifestPath),
                        modernDir, sizeof(modernDir));
    CHECK(write_manifest(manifestPath, 0, 0, 0, 0),
          "wrote placeholder manifest");
    CHECK(csb_v22_famg_validate_manifest(manifestPath) == 1,
          "explicit valid path validate -> 1");
    CHECK(csb_v22_famg_validate_manifest("") == -1,
          "explicit empty path validate -> -1");
    CHECK(csb_v22_famg_validate_manifest("/nonexistent/manifest.json") == -1,
          "explicit nonexistent path validate -> -1");
}

static void test_csb_specific_categories(void) {
    /* The CSB-only categories (chaos_runes, door_shapes) must be
     * tracked separately from DM1's door_shapes-only surface. The
     * door_prison slot uses category "door_shapes" which is the same
     * category DM1 uses, but chaos_rune is a CSB-only category. */
    const SlotFixture* chaos = NULL;
    const SlotFixture* door = NULL;
    for (size_t i = 0; i < CSB_V22_FAMG_MATERIAL_COUNT; ++i) {
        if (k_slots[i].slot == CSB_V22_FAMG_CHAOS_RUNE) chaos = &k_slots[i];
        if (k_slots[i].slot == CSB_V22_FAMG_DOOR_PRISON) door = &k_slots[i];
    }
    CHECK(chaos != NULL && strcmp(chaos->category, "chaos_runes") == 0,
          "chaos_rune uses CSB-only category");
    CHECK(door != NULL && strcmp(door->category, "door_shapes") == 0,
          "door_prison uses door_shapes category");
    CHECK(strcmp(csb_v22_famg_slot_manifest_id(CSB_V22_FAMG_CHAOS_RUNE),
                 "chaos_rune_01") == 0,
          "chaos_rune manifest id is chaos_rune_01");
    CHECK(strcmp(csb_v22_famg_slot_manifest_id(CSB_V22_FAMG_DOOR_PRISON),
                 "door_prison_01") == 0,
          "door_prison manifest id is door_prison_01");
}

static void test_per_cell_routing(void) {
    /* The 9-square CSB viewport (CSBWin/Viewport.cpp:7290) maps to
     * 3 depth x 3 lateral cells. Every cell in this first cut routes
     * to the WALL_DUNGEON slot because the runtime variant -> asset_id
     * mapping in csb_v22_inplace_draw_pc34.c maps all wall variants to
     * wall_dungeon_01. Per-cell refinement (mossy walls for slime
     * zones) is a follow-up that mirrors the variant enum in
     * csb_v22_shapes.h. */
    int depth, lateral;
    for (depth = 0; depth <= 2; ++depth) {
        for (lateral = -1; lateral <= 1; ++lateral) {
            CSB_V22_FamgSlot slot =
                csb_v22_famg_slot_for_cell(depth, lateral);
            CHECK(slot == CSB_V22_FAMG_WALL_DUNGEON,
                  "in-range cell routes to WALL_DUNGEON");
        }
    }
    /* Out-of-range cells return the sentinel value (MATERIAL_COUNT). */
    CHECK(csb_v22_famg_slot_for_cell(-1, 0) == CSB_V22_FAMG_MATERIAL_COUNT,
          "out-of-range depth -1 -> sentinel");
    CHECK(csb_v22_famg_slot_for_cell(3, 0) == CSB_V22_FAMG_MATERIAL_COUNT,
          "out-of-range depth 3 -> sentinel");
    CHECK(csb_v22_famg_slot_for_cell(0, -2) == CSB_V22_FAMG_MATERIAL_COUNT,
          "out-of-range lateral -2 -> sentinel");
    CHECK(csb_v22_famg_slot_for_cell(0, 2) == CSB_V22_FAMG_MATERIAL_COUNT,
          "out-of-range lateral 2 -> sentinel");
    /* classify_cell short-circuits to UNKNOWN for out-of-range. */
    {
        csb_v22_famg_set_manifest_path(NULL);
        CHECK(csb_v22_famg_classify_cell(0, 0) == CSB_V22_FAMG_CLASS_MISSING,
              "no manifest path -> classify_cell returns MISSING for in-range");
        CHECK(csb_v22_famg_classify_cell(-1, 0) == CSB_V22_FAMG_CLASS_UNKNOWN,
              "out-of-range -> classify_cell returns UNKNOWN");
    }
    /* classify_cell respects manifest state for in-range cells. */
    {
        const char* dataDir = "/tmp/scratch/csb-v22-famg-test/data/csb";
        char manifestPath[FSP_PATH_MAX];
        char modernDir[FSP_PATH_MAX];
        clean_scratch();
        setup_manifest_dirs(dataDir,
                            manifestPath, sizeof(manifestPath),
                            modernDir, sizeof(modernDir));
        csb_v22_famg_set_manifest_path(dataDir);
        CHECK(write_manifest(manifestPath, 0, 0, 0, 0),
              "wrote placeholder manifest for per-cell routing");
        for (depth = 0; depth <= 2; ++depth) {
            for (lateral = -1; lateral <= 1; ++lateral) {
                CSB_V22_FamgClass cls =
                    csb_v22_famg_classify_cell(depth, lateral);
                CHECK(cls == CSB_V22_FAMG_CLASS_PLACEHOLDER,
                      "placeholder manifest -> in-range cell PLACEHOLDER");
            }
        }
        csb_v22_famg_set_manifest_path(NULL);
    }
    /* Reset manifest path after this scenario to avoid leaking state. */
    csb_v22_famg_set_manifest_path(NULL);
}

int main(void) {
    test_unset_manifest_path();
    test_missing_manifest_file();
    test_garbage_manifest();
    test_empty_manifest();
    test_placeholder_manifest();
    test_partial_manifest();
    test_missing_file_partial();
    test_incomplete_metadata();
    test_finished_real_manifest();
    test_get_slot_info();
    test_slot_info_unknown_assets();
    test_null_and_oob_safety();
    test_names_and_evidence();
    test_installed_setter();
    test_validate_at_explicit_path();
    test_csb_specific_categories();
    test_per_cell_routing();

    clean_scratch();
    printf("csb_v22_finished_art_material_gate_pc34: %d/%d checks passed\n",
           s_pass, s_pass + s_fail);
    if (s_fail > 0) {
        printf("csb_v22_finished_art_material_gate_pc34: FAIL\n");
        return 1;
    }
    printf("csb_v22_finished_art_material_gate_pc34: PASS\n");
    return 0;
}
