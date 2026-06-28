/*
 * test_dm2_v22_finished_art_material_gate_pc34.c
 *
 * DM2 V2.2 finished-art material gate synthetic CTest.
 *
 * Builds modern_asset_manifest.json fixtures under /tmp/scratch and
 * verifies the placeholder-vs-real state machine for the same material
 * ids used by dm2_v22_viewport_swap_pc34.c:
 *
 *   wall_dm2_temple_01, wall_dm2_outdoor_01, floor_dm2_outdoor_01,
 *   floor_dm2_pit_01, floor_dm2_stairs_01, creature_dm2_brigand_01,
 *   sky_dm2_outdoor_01, ground_dm2_outdoor_01, ground_dm2_horizon_01,
 *   tree_dm2_outdoor_01, door_dm2_wood_01.
 *
 * Source-lock anchors:
 *   - SKULL.ASM T520/T560/T600 (DM2 viewport ticks)
 *   - ReDMCSB DUNVIEW.C:2962-3070 (outdoor sky/ground composition)
 *   - include/dm2_v22_viewport_swap_pc34.h (shape -> asset_id table)
 *   - src/dm2/dm2_v22_modern_assets_pc34.c (line-style JSON scanner)
 *   - sibling dm2_v2_hud_widget_assets.c placeholder-vs-real pattern
 */

#include "dm2_v22_finished_art_material_gate_pc34.h"
#include "fs_portable_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int s_pass = 0;
static int s_fail = 0;

#define CHECK(expr, msg)                                                   \
    do {                                                                   \
        s_pass++;                                                          \
        if (!(expr)) {                                                     \
            fprintf(stderr, "FAIL %s:%d: %s -- %s\n",                      \
                    __FILE__, __LINE__, #expr, (msg));                    \
            s_fail++;                                                      \
        }                                                                  \
    } while (0)

typedef struct {
    DM2_V22_FamgSlot slot;
    const char* id;
    const char* category;
    int width;
    int height;
} SlotFixture;

static const SlotFixture k_slots[] = {
    { DM2_V22_FAMG_WALL_DUNGEON,     "wall_dm2_temple_01",      "wall_shapes",     96, 96 },
    { DM2_V22_FAMG_WALL_OUTDOOR,     "wall_dm2_outdoor_01",     "wall_shapes",     96, 96 },
    { DM2_V22_FAMG_FLOOR_PLAIN,      "floor_dm2_outdoor_01",    "floor_shapes",    96, 96 },
    { DM2_V22_FAMG_FLOOR_PIT,        "floor_dm2_pit_01",        "floor_shapes",    96, 96 },
    { DM2_V22_FAMG_FLOOR_STAIRS,     "floor_dm2_stairs_01",     "floor_shapes",    96, 96 },
    { DM2_V22_FAMG_CREATURE_BRIGAND, "creature_dm2_brigand_01", "creature_shapes", 64, 64 },
    { DM2_V22_FAMG_SKY,              "sky_dm2_outdoor_01",      "wall_shapes",    192, 64 },
    { DM2_V22_FAMG_GROUND,           "ground_dm2_outdoor_01",   "floor_shapes",   192, 64 },
    { DM2_V22_FAMG_GROUND_HORIZON,   "ground_dm2_horizon_01",   "floor_shapes",   192,  8 },
    { DM2_V22_FAMG_TREE,             "tree_dm2_outdoor_01",     "creature_shapes", 64, 96 },
    { DM2_V22_FAMG_DOOR_WOOD,        "door_dm2_wood_01",        "door_shapes",     64, 96 }
};

static int write_file(const char* path, const char* content) {
    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;
    size_t len = strlen(content);
    size_t written = fwrite(content, 1, len, fp);
    fclose(fp);
    return written == len;
}

static void clean_scratch(void) {
    system("rm -rf /tmp/scratch/dm2-v22-famg-test");
}

static void build_expected_manifest_path(char* out, size_t outSize,
                                         const char* dataDir) {
    char parent1[FSP_PATH_MAX];
    char parent2[FSP_PATH_MAX];
    char assets[FSP_PATH_MAX];
    char dm2[FSP_PATH_MAX];
    char modern[FSP_PATH_MAX];
    if (!FSP_ParentDir(parent1, sizeof(parent1), dataDir) ||
        !FSP_ParentDir(parent2, sizeof(parent2), parent1)) {
        FSP_JoinPath(assets, sizeof(assets), dataDir, "assets");
    } else {
        FSP_JoinPath(assets, sizeof(assets), parent2, "assets");
    }
    FSP_JoinPath(dm2, sizeof(dm2), assets, "dm2");
    FSP_JoinPath(modern, sizeof(modern), dm2, "modern");
    FSP_JoinPath(out, outSize, modern, "modern_asset_manifest.json");
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
    for (size_t i = 0; i < DM2_V22_FAMG_MATERIAL_COUNT; ++i) {
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
    CHECK(write_file(path, "fake-dm2-v22-finished-art-bytes"),
          "created source_file on disk");
}

static int write_manifest(const char* path,
                          const char* defaultGenerator,
                          int realMask,
                          int omitDimensionsForFirst,
                          int firstOnly,
                          int missingFileNameForFirst) {
    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;
    fprintf(fp, "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm2-v22-famg-test\",\"slots\":[");
    int count = firstOnly ? 1 : (int)DM2_V22_FAMG_MATERIAL_COUNT;
    for (int i = 0; i < count; ++i) {
        const SlotFixture* s = &k_slots[i];
        char file[128];
        source_filename(file, sizeof(file), s);
        const char* gen = (realMask & (1 << i)) ? "pbr_hero" : defaultGenerator;
        if (missingFileNameForFirst && i == 0) {
            snprintf(file, sizeof(file), "missing_%s.png", s->id);
        }
        if (i > 0) fprintf(fp, ",");
        fprintf(fp, "{\"id\":\"%s\",\"generator\":\"%s\",\"source_file\":\"%s\"",
                s->id, gen, file);
        if (!(omitDimensionsForFirst && i == 0)) {
            fprintf(fp, ",\"width\":%d,\"height\":%d", s->width, s->height);
        }
        fprintf(fp, "}");
    }
    fprintf(fp, "]}");
    fclose(fp);
    return 1;
}

static void test_unset_path_is_safe(void) {
    clean_scratch();
    dm2_v22_famg_set_manifest_path(NULL);
    dm2_v22_famg_set_manifest_path("");
    CHECK(dm2_v22_famg_get_manifest_path()[0] == '\0', "unset path is empty");

    int total = -1;
    int real = dm2_v22_famg_real_count(&total);
    CHECK(real == 0, "real_count=0 with no manifest");
    CHECK(total == 0, "total=0 with no manifest");
    CHECK(dm2_v22_famg_gate() == DM2_V22_FAMG_GATE_NO_MANIFEST,
          "unset path -> NO_MANIFEST");
    CHECK(dm2_v22_famg_is_finished_real() == 0,
          "unset path is not finished real");
    CHECK(dm2_v22_famg_is_synthetic_or_partial() == 0,
          "unset path is not synthetic/partial");

    for (size_t i = 0; i < DM2_V22_FAMG_MATERIAL_COUNT; ++i) {
        CHECK(dm2_v22_famg_classify_slot(k_slots[i].slot) ==
              DM2_V22_FAMG_CLASS_MISSING,
              "unset manifest classifies slot as MISSING");
        CHECK(dm2_v22_famg_uses_placeholder(k_slots[i].slot) == 1,
              "unset manifest uses placeholder");
    }
}

static void test_path_resolution_and_missing_manifest(void) {
    const char* dataDir = "/tmp/scratch/dm2-v22-famg-test/data/dm2";
    char expected[FSP_PATH_MAX];
    clean_scratch();
    FSP_CreateDirectoryRecursive(dataDir);
    build_expected_manifest_path(expected, sizeof(expected), dataDir);
    dm2_v22_famg_set_manifest_path(dataDir);
    CHECK(strcmp(dm2_v22_famg_get_manifest_path(), expected) == 0,
          "manifest path resolves to assets/dm2/modern");
    CHECK(dm2_v22_famg_gate() == DM2_V22_FAMG_GATE_NO_MANIFEST,
          "missing manifest -> NO_MANIFEST");
    CHECK(dm2_v22_famg_validate_manifest(NULL) == -1,
          "missing stored manifest validates as -1");
    CHECK(dm2_v22_famg_validate_manifest("/no/such/manifest.json") == -1,
          "missing explicit manifest validates as -1");
}

static void test_empty_manifest(void) {
    const char* dataDir = "/tmp/scratch/dm2-v22-famg-test/data/dm2";
    char manifest[FSP_PATH_MAX];
    char modern[FSP_PATH_MAX];
    clean_scratch();
    setup_manifest_dirs(dataDir, manifest, sizeof(manifest), modern, sizeof(modern));
    CHECK(write_file(manifest, "{}"), "wrote empty manifest");
    dm2_v22_famg_set_manifest_path(dataDir);
    CHECK(dm2_v22_famg_validate_manifest(NULL) == 0,
          "empty manifest validates as readable but incomplete");
    CHECK(dm2_v22_famg_gate() == DM2_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER,
          "empty manifest -> SYNTHETIC_PLACEHOLDER");
    CHECK(dm2_v22_famg_classify_slot(DM2_V22_FAMG_WALL_DUNGEON) ==
          DM2_V22_FAMG_CLASS_MISSING, "empty manifest leaves slots missing");
}

static void test_placeholder_manifest(void) {
    const char* dataDir = "/tmp/scratch/dm2-v22-famg-test/data/dm2";
    char manifest[FSP_PATH_MAX];
    char modern[FSP_PATH_MAX];
    clean_scratch();
    setup_manifest_dirs(dataDir, manifest, sizeof(manifest), modern, sizeof(modern));
    CHECK(write_manifest(manifest, "placeholder", 0, 0, 0, 0),
          "wrote placeholder manifest");
    dm2_v22_famg_set_manifest_path(dataDir);
    CHECK(dm2_v22_famg_validate_manifest(NULL) == 1,
          "placeholder manifest validates as complete");
    CHECK(dm2_v22_famg_gate() == DM2_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER,
          "all placeholders -> SYNTHETIC_PLACEHOLDER");
    CHECK(dm2_v22_famg_get_installed() == 0,
          "placeholder gate is not installed");
    CHECK(dm2_v22_famg_is_synthetic_or_partial() == 1,
          "placeholder gate is synthetic/partial");
    for (size_t i = 0; i < DM2_V22_FAMG_MATERIAL_COUNT; ++i) {
        CHECK(dm2_v22_famg_classify_slot(k_slots[i].slot) ==
              DM2_V22_FAMG_CLASS_PLACEHOLDER,
              "placeholder manifest classifies every slot as PLACEHOLDER");
        CHECK(dm2_v22_famg_uses_placeholder(k_slots[i].slot) == 1,
              "placeholder slot uses placeholder");
    }
}

static void test_all_real_manifest(void) {
    const char* dataDir = "/tmp/scratch/dm2-v22-famg-test/data/dm2";
    char manifest[FSP_PATH_MAX];
    char modern[FSP_PATH_MAX];
    int allMask = (1 << DM2_V22_FAMG_MATERIAL_COUNT) - 1;
    clean_scratch();
    setup_manifest_dirs(dataDir, manifest, sizeof(manifest), modern, sizeof(modern));
    for (size_t i = 0; i < DM2_V22_FAMG_MATERIAL_COUNT; ++i) {
        create_real_file(modern, &k_slots[i]);
    }
    CHECK(write_manifest(manifest, "placeholder", allMask, 0, 0, 0),
          "wrote all-real manifest");
    dm2_v22_famg_set_manifest_path(dataDir);
    CHECK(dm2_v22_famg_gate() == DM2_V22_FAMG_GATE_FINISHED_REAL,
          "all eleven slots REAL -> FINISHED_REAL");
    CHECK(dm2_v22_famg_get_installed() == 1,
          "finished real gate is installed");
    CHECK(dm2_v22_famg_is_finished_real() == 1,
          "finished real helper is true");
    CHECK(dm2_v22_famg_is_synthetic_or_partial() == 0,
          "finished real is not synthetic/partial");

    int total = 0;
    int real = dm2_v22_famg_real_count(&total);
    CHECK(real == (int)DM2_V22_FAMG_MATERIAL_COUNT,
          "real_count equals material count");
    CHECK(total == (int)DM2_V22_FAMG_MATERIAL_COUNT,
          "total equals material count");
    for (size_t i = 0; i < DM2_V22_FAMG_MATERIAL_COUNT; ++i) {
        DM2_V22_FamgSlotInfo info;
        CHECK(dm2_v22_famg_classify_slot(k_slots[i].slot) ==
              DM2_V22_FAMG_CLASS_REAL,
              "all-real manifest classifies every slot as REAL");
        CHECK(dm2_v22_famg_uses_placeholder(k_slots[i].slot) == 0,
              "real slot does not use placeholder");
        CHECK(dm2_v22_famg_get_slot_info(k_slots[i].slot, &info) == 1,
              "get_slot_info succeeds for real slot");
        CHECK(strcmp(info.id, k_slots[i].id) == 0, "slot info id matches");
        CHECK(strcmp(info.category, k_slots[i].category) == 0,
              "slot info category matches");
        CHECK(info.file_exists == 1, "slot info file_exists=1");
        CHECK(info.classification == DM2_V22_FAMG_CLASS_REAL,
              "slot info classification REAL");
    }
}

static void test_partial_manifest(void) {
    const char* dataDir = "/tmp/scratch/dm2-v22-famg-test/data/dm2";
    char manifest[FSP_PATH_MAX];
    char modern[FSP_PATH_MAX];
    clean_scratch();
    setup_manifest_dirs(dataDir, manifest, sizeof(manifest), modern, sizeof(modern));
    create_real_file(modern, &k_slots[0]);
    CHECK(write_manifest(manifest, "placeholder", 1, 0, 0, 0),
          "wrote one-real manifest");
    dm2_v22_famg_set_manifest_path(dataDir);
    CHECK(dm2_v22_famg_gate() == DM2_V22_FAMG_GATE_PARTIAL,
          "one real plus placeholders -> PARTIAL");
    CHECK(dm2_v22_famg_get_installed() == 1, "partial gate is installed");
    CHECK(dm2_v22_famg_is_finished_real() == 0, "partial is not finished");
    CHECK(dm2_v22_famg_is_synthetic_or_partial() == 1,
          "partial is synthetic/partial");
    CHECK(dm2_v22_famg_classify_slot(DM2_V22_FAMG_WALL_DUNGEON) ==
          DM2_V22_FAMG_CLASS_REAL, "first slot REAL");
    CHECK(dm2_v22_famg_classify_slot(DM2_V22_FAMG_WALL_OUTDOOR) ==
          DM2_V22_FAMG_CLASS_PLACEHOLDER, "second slot PLACEHOLDER");
    int total = 0;
    int real = dm2_v22_famg_real_count(&total);
    CHECK(real == 1, "partial real_count=1");
    CHECK(total == (int)DM2_V22_FAMG_MATERIAL_COUNT,
          "partial total includes all declared slots");
}

static void test_partial_only_does_not_promote_to_finished(void) {
    const char* dataDir = "/tmp/scratch/dm2-v22-famg-test/data/dm2";
    char manifest[FSP_PATH_MAX];
    char modern[FSP_PATH_MAX];
    clean_scratch();
    setup_manifest_dirs(dataDir, manifest, sizeof(manifest), modern, sizeof(modern));
    CHECK(write_manifest(manifest, "placeholder", 1, 0, 1, 1),
          "wrote non-placeholder first slot with missing file");
    dm2_v22_famg_set_manifest_path(dataDir);
    CHECK(dm2_v22_famg_classify_slot(DM2_V22_FAMG_WALL_DUNGEON) ==
          DM2_V22_FAMG_CLASS_PARTIAL, "real metadata missing file -> PARTIAL");
    CHECK(dm2_v22_famg_gate() == DM2_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER,
          "partial-only manifest stays synthetic placeholder");
    int total = 0;
    int real = dm2_v22_famg_real_count(&total);
    CHECK(real == 0, "partial-only real_count=0");
    CHECK(total == 1, "partial-only total=1 declared slot");
}

static void test_missing_fields_and_garbage_manifest(void) {
    const char* dataDir = "/tmp/scratch/dm2-v22-famg-test/data/dm2";
    char manifest[FSP_PATH_MAX];
    char modern[FSP_PATH_MAX];
    clean_scratch();
    setup_manifest_dirs(dataDir, manifest, sizeof(manifest), modern, sizeof(modern));
    CHECK(write_manifest(manifest, "pbr_hero", 1, 1, 1, 0),
          "wrote incomplete first slot");
    dm2_v22_famg_set_manifest_path(dataDir);
    CHECK(dm2_v22_famg_validate_manifest(NULL) == 0,
          "missing dimensions -> validate 0");
    CHECK(dm2_v22_famg_classify_slot(DM2_V22_FAMG_WALL_DUNGEON) ==
          DM2_V22_FAMG_CLASS_PARTIAL, "missing dimensions -> PARTIAL");
    CHECK(write_file(manifest, "not json at all"), "wrote garbage manifest");
    CHECK(dm2_v22_famg_validate_manifest(NULL) == -1,
          "garbage manifest -> validate -1");
    CHECK(dm2_v22_famg_gate() == DM2_V22_FAMG_GATE_NO_MANIFEST,
          "garbage manifest -> NO_MANIFEST");
}

static void test_names_inputs_and_evidence(void) {
    CHECK(DM2_V22_FAMG_MATERIAL_COUNT == 11, "material count is eleven");
    for (size_t i = 0; i < DM2_V22_FAMG_MATERIAL_COUNT; ++i) {
        CHECK((int)k_slots[i].slot == (int)i, "slot ordinal is stable");
        CHECK(strcmp(dm2_v22_famg_slot_name(k_slots[i].slot), k_slots[i].id) == 0,
              "slot name matches runtime asset id");
        CHECK(strcmp(dm2_v22_famg_slot_manifest_id(k_slots[i].slot),
                     k_slots[i].id) == 0,
              "slot manifest id matches runtime asset id");
        CHECK(strcmp(dm2_v22_famg_slot_category(k_slots[i].slot),
                     k_slots[i].category) == 0,
              "slot category is stable");
    }
    CHECK(strcmp(dm2_v22_famg_slot_name((DM2_V22_FamgSlot)9999), "UNKNOWN") == 0,
          "out-of-range slot name -> UNKNOWN");
    CHECK(strcmp(dm2_v22_famg_slot_category((DM2_V22_FamgSlot)9999), "") == 0,
          "out-of-range category -> empty");
    CHECK(strcmp(dm2_v22_famg_slot_manifest_id((DM2_V22_FamgSlot)9999), "") == 0,
          "out-of-range manifest id -> empty");
    CHECK(dm2_v22_famg_classify_slot((DM2_V22_FamgSlot)9999) ==
          DM2_V22_FAMG_CLASS_UNKNOWN, "out-of-range classify -> UNKNOWN");
    CHECK(dm2_v22_famg_get_slot_info((DM2_V22_FamgSlot)9999, NULL) == 0,
          "get_slot_info invalid NULL is safe");

    CHECK(strcmp(dm2_v22_famg_class_name(DM2_V22_FAMG_CLASS_UNKNOWN), "UNKNOWN") == 0,
          "class UNKNOWN name");
    CHECK(strcmp(dm2_v22_famg_class_name(DM2_V22_FAMG_CLASS_MISSING), "MISSING") == 0,
          "class MISSING name");
    CHECK(strcmp(dm2_v22_famg_class_name(DM2_V22_FAMG_CLASS_PLACEHOLDER), "PLACEHOLDER") == 0,
          "class PLACEHOLDER name");
    CHECK(strcmp(dm2_v22_famg_class_name(DM2_V22_FAMG_CLASS_PARTIAL), "PARTIAL") == 0,
          "class PARTIAL name");
    CHECK(strcmp(dm2_v22_famg_class_name(DM2_V22_FAMG_CLASS_REAL), "REAL") == 0,
          "class REAL name");
    CHECK(strcmp(dm2_v22_famg_gate_name(DM2_V22_FAMG_GATE_NOT_PROBED), "NOT_PROBED") == 0,
          "gate NOT_PROBED name");
    CHECK(strcmp(dm2_v22_famg_gate_name(DM2_V22_FAMG_GATE_NO_MANIFEST), "NO_MANIFEST") == 0,
          "gate NO_MANIFEST name");
    CHECK(strcmp(dm2_v22_famg_gate_name(DM2_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER),
                 "SYNTHETIC_PLACEHOLDER") == 0,
          "gate SYNTHETIC_PLACEHOLDER name");
    CHECK(strcmp(dm2_v22_famg_gate_name(DM2_V22_FAMG_GATE_PARTIAL), "PARTIAL") == 0,
          "gate PARTIAL name");
    CHECK(strcmp(dm2_v22_famg_gate_name(DM2_V22_FAMG_GATE_FINISHED_REAL),
                 "FINISHED_REAL") == 0,
          "gate FINISHED_REAL name");
    dm2_v22_famg_set_installed(0);
    CHECK(dm2_v22_famg_get_installed() == 0, "installed reset to 0");
    dm2_v22_famg_set_installed(42);
    CHECK(dm2_v22_famg_get_installed() == 1, "installed clamps non-zero to 1");

    const char* ev = dm2_v22_famg_source_evidence();
    CHECK(ev && strstr(ev, "SKULL.ASM") != NULL, "evidence cites SKULL.ASM");
    CHECK(ev && strstr(ev, "T520/T560/T600") != NULL, "evidence cites ticks");
    CHECK(ev && strstr(ev, "DUNVIEW.C:2962-3070") != NULL,
          "evidence cites outdoor DUNVIEW range");
    CHECK(ev && strstr(ev, "dm2_v22_viewport_swap_pc34") != NULL,
          "evidence cites viewport swap");
    CHECK(ev && strstr(ev, "dm2_v22_modern_assets_pc34") != NULL,
          "evidence cites modern asset scanner");
    CHECK(ev && strstr(ev, "wall_dm2_temple_01") != NULL,
          "evidence names tracked slots");
    CHECK(ev && strstr(ev, "FIRESTAFF_GAP_LIST") != NULL,
          "evidence cites gap list");
    CHECK(ev && strstr(ev, "Honest boundary") != NULL,
          "evidence states honest boundary");
}

int main(void) {
    printf("=== DM2 V2.2 finished-art material gate (synthetic) ===\n");
    test_unset_path_is_safe();
    test_path_resolution_and_missing_manifest();
    test_empty_manifest();
    test_placeholder_manifest();
    test_all_real_manifest();
    test_partial_manifest();
    test_partial_only_does_not_promote_to_finished();
    test_missing_fields_and_garbage_manifest();
    test_names_inputs_and_evidence();
    clean_scratch();
    printf("\nSummary: %d passed, %d failed\n", s_pass, s_fail);
    return s_fail == 0 ? 0 : 1;
}
