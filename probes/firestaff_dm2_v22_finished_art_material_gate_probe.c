/*
 * firestaff_dm2_v22_finished_art_material_gate_probe.c
 *
 * Headless DM2 V2.2 finished-art material gate probe. No game data or
 * SDL required. Creates synthetic modern_asset_manifest.json fixtures
 * under /tmp/scratch and verifies the CI-runnable distinction between:
 *
 *   SYNTHETIC_PLACEHOLDER - procedural placeholder manifest entries
 *   PARTIAL               - at least one real file, not all required slots
 *   FINISHED_REAL         - all tracked slots have non-placeholder
 *                           generator metadata and source_file on disk
 *
 * Source:
 *   - SKULL.ASM T520/T560/T600 (DM2 viewport ticks)
 *   - ReDMCSB DUNVIEW.C:2962-3070 (outdoor sky/ground composition)
 *   - include/dm2_v22_viewport_swap_pc34.h (shape -> asset_id table)
 *   - src/dm2/dm2_v22_modern_assets_pc34.c (manifest scanner pattern)
 */

#include "dm2_v22_finished_art_material_gate_pc34.h"
#include "fs_portable_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int s_pass = 0;
static int s_fail = 0;

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

static void check(const char* name, int cond) {
    if (cond) {
        printf("  PASS: %s\n", name);
        ++s_pass;
    } else {
        printf("  FAIL: %s\n", name);
        ++s_fail;
    }
}

static int write_file(const char* path, const char* content) {
    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;
    size_t len = strlen(content);
    size_t written = fwrite(content, 1, len, fp);
    fclose(fp);
    return written == len;
}

static void expected_manifest_path(char* out, size_t outSize,
                                   const char* dataDir) {
    char p1[FSP_PATH_MAX], p2[FSP_PATH_MAX], assets[FSP_PATH_MAX];
    char dm2[FSP_PATH_MAX], modern[FSP_PATH_MAX];
    if (!FSP_ParentDir(p1, sizeof(p1), dataDir) ||
        !FSP_ParentDir(p2, sizeof(p2), p1)) {
        FSP_JoinPath(assets, sizeof(assets), dataDir, "assets");
    } else {
        FSP_JoinPath(assets, sizeof(assets), p2, "assets");
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

static void setup_paths(const char* dataDir, char* manifest, size_t manifestSize,
                        char* modernDir, size_t modernDirSize) {
    expected_manifest_path(manifest, manifestSize, dataDir);
    parent_dir(modernDir, modernDirSize, manifest);
    FSP_CreateDirectoryRecursive(modernDir);
    for (size_t i = 0; i < DM2_V22_FAMG_MATERIAL_COUNT; ++i) {
        char dir[FSP_PATH_MAX];
        FSP_JoinPath(dir, sizeof(dir), modernDir, k_slots[i].category);
        FSP_CreateDirectoryRecursive(dir);
    }
}

static void source_file(char* out, size_t outSize, const SlotFixture* s) {
    snprintf(out, outSize, "%s.png", s->id);
}

static void source_path(char* out, size_t outSize, const char* modernDir,
                        const SlotFixture* s) {
    char file[128];
    char dir[FSP_PATH_MAX];
    source_file(file, sizeof(file), s);
    FSP_JoinPath(dir, sizeof(dir), modernDir, s->category);
    FSP_JoinPath(out, outSize, dir, file);
}

static void create_source(const char* modernDir, const SlotFixture* s) {
    char path[FSP_PATH_MAX];
    source_path(path, sizeof(path), modernDir, s);
    write_file(path, "fake-dm2-v22-material");
}

static int write_manifest(const char* path, int realMask, int firstOnly,
                          int missingFirst, int omitDimsFirst) {
    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;
    int count = firstOnly ? 1 : (int)DM2_V22_FAMG_MATERIAL_COUNT;
    fprintf(fp, "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm2-v22-famg-probe\",\"slots\":[");
    for (int i = 0; i < count; ++i) {
        const SlotFixture* s = &k_slots[i];
        const char* gen = (realMask & (1 << i)) ? "pbr_hero" : "placeholder";
        char file[128];
        source_file(file, sizeof(file), s);
        if (missingFirst && i == 0) snprintf(file, sizeof(file), "missing_%s.png", s->id);
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

int main(void) {
    const char* dataDir = "/tmp/scratch/dm2-v22-famg-probe/data/dm2";
    char manifest[FSP_PATH_MAX];
    char modern[FSP_PATH_MAX];
    int total = 0;
    int real = 0;

    printf("=== DM2 V2.2 finished-art material gate probe ===\n");
    system("rm -rf /tmp/scratch/dm2-v22-famg-probe");

    printf("\n[ Scenario 1: unset path ]\n");
    dm2_v22_famg_set_manifest_path(NULL);
    check("unset path -> NO_MANIFEST",
          dm2_v22_famg_gate() == DM2_V22_FAMG_GATE_NO_MANIFEST);
    check("unset path -> installed=0", dm2_v22_famg_get_installed() == 0);
    check("unset path -> not finished", dm2_v22_famg_is_finished_real() == 0);
    for (size_t i = 0; i < DM2_V22_FAMG_MATERIAL_COUNT; ++i) {
        check("unset path -> slot MISSING",
              dm2_v22_famg_classify_slot(k_slots[i].slot) ==
              DM2_V22_FAMG_CLASS_MISSING);
    }

    printf("\n[ Scenario 2: missing manifest ]\n");
    FSP_CreateDirectoryRecursive(dataDir);
    setup_paths(dataDir, manifest, sizeof(manifest), modern, sizeof(modern));
    system("rm -f /tmp/scratch/dm2-v22-famg-probe/assets/dm2/modern/modern_asset_manifest.json");
    dm2_v22_famg_set_manifest_path(dataDir);
    check("manifest path matches expected",
          strcmp(dm2_v22_famg_get_manifest_path(), manifest) == 0);
    check("missing manifest -> NO_MANIFEST",
          dm2_v22_famg_gate() == DM2_V22_FAMG_GATE_NO_MANIFEST);
    check("missing manifest validate -> -1",
          dm2_v22_famg_validate_manifest(NULL) == -1);

    printf("\n[ Scenario 3: empty manifest ]\n");
    check("wrote empty manifest", write_file(manifest, "{}"));
    check("empty manifest -> SYNTHETIC_PLACEHOLDER",
          dm2_v22_famg_gate() == DM2_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER);
    check("empty manifest -> first slot MISSING",
          dm2_v22_famg_classify_slot(DM2_V22_FAMG_WALL_DUNGEON) ==
          DM2_V22_FAMG_CLASS_MISSING);

    printf("\n[ Scenario 4: placeholder manifest ]\n");
    check("wrote placeholder manifest",
          write_manifest(manifest, 0, 0, 0, 0));
    check("placeholder -> validate 1", dm2_v22_famg_validate_manifest(NULL) == 1);
    check("placeholder -> SYNTHETIC_PLACEHOLDER",
          dm2_v22_famg_gate() == DM2_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER);
    for (size_t i = 0; i < DM2_V22_FAMG_MATERIAL_COUNT; ++i) {
        check("placeholder -> slot PLACEHOLDER",
              dm2_v22_famg_classify_slot(k_slots[i].slot) ==
              DM2_V22_FAMG_CLASS_PLACEHOLDER);
    }
    check("placeholder -> synthetic helper",
          dm2_v22_famg_is_synthetic_or_partial() == 1);

    printf("\n[ Scenario 5: partial manifest ]\n");
    create_source(modern, &k_slots[0]);
    check("wrote one-real manifest",
          write_manifest(manifest, 1, 0, 0, 0));
    check("one real -> PARTIAL",
          dm2_v22_famg_gate() == DM2_V22_FAMG_GATE_PARTIAL);
    real = dm2_v22_famg_real_count(&total);
    check("partial -> real_count=1", real == 1);
    check("partial -> total=11", total == (int)DM2_V22_FAMG_MATERIAL_COUNT);
    check("partial -> real slot no placeholder",
          dm2_v22_famg_uses_placeholder(DM2_V22_FAMG_WALL_DUNGEON) == 0);
    check("partial -> placeholder slot uses placeholder",
          dm2_v22_famg_uses_placeholder(DM2_V22_FAMG_WALL_OUTDOOR) == 1);

    printf("\n[ Scenario 6: finished-real manifest ]\n");
    for (size_t i = 0; i < DM2_V22_FAMG_MATERIAL_COUNT; ++i) {
        create_source(modern, &k_slots[i]);
    }
    check("wrote all-real manifest",
          write_manifest(manifest, (1 << DM2_V22_FAMG_MATERIAL_COUNT) - 1, 0, 0, 0));
    check("all real -> FINISHED_REAL",
          dm2_v22_famg_gate() == DM2_V22_FAMG_GATE_FINISHED_REAL);
    real = dm2_v22_famg_real_count(&total);
    check("finished-real -> real_count=11",
          real == (int)DM2_V22_FAMG_MATERIAL_COUNT);
    check("finished-real -> total=11",
          total == (int)DM2_V22_FAMG_MATERIAL_COUNT);
    for (size_t i = 0; i < DM2_V22_FAMG_MATERIAL_COUNT; ++i) {
        check("finished-real -> slot REAL",
              dm2_v22_famg_classify_slot(k_slots[i].slot) ==
              DM2_V22_FAMG_CLASS_REAL);
    }

    printf("\n[ Scenario 7: non-placeholder metadata missing file ]\n");
    check("wrote missing-file manifest",
          write_manifest(manifest, 1, 1, 1, 0));
    check("missing file -> first slot PARTIAL",
          dm2_v22_famg_classify_slot(DM2_V22_FAMG_WALL_DUNGEON) ==
          DM2_V22_FAMG_CLASS_PARTIAL);
    check("partial-only -> SYNTHETIC_PLACEHOLDER",
          dm2_v22_famg_gate() == DM2_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER);

    printf("\n[ Scenario 8: invariants and source evidence ]\n");
    check("material count is 11", DM2_V22_FAMG_MATERIAL_COUNT == 11);
    for (size_t i = 0; i < DM2_V22_FAMG_MATERIAL_COUNT; ++i) {
        check("slot name matches asset id",
              strcmp(dm2_v22_famg_slot_name(k_slots[i].slot), k_slots[i].id) == 0);
        check("slot category matches fixture",
              strcmp(dm2_v22_famg_slot_category(k_slots[i].slot), k_slots[i].category) == 0);
    }
    check("out-of-range slot -> UNKNOWN",
          strcmp(dm2_v22_famg_slot_name((DM2_V22_FamgSlot)999), "UNKNOWN") == 0);
    check("class REAL name",
          strcmp(dm2_v22_famg_class_name(DM2_V22_FAMG_CLASS_REAL), "REAL") == 0);
    check("gate FINISHED_REAL name",
          strcmp(dm2_v22_famg_gate_name(DM2_V22_FAMG_GATE_FINISHED_REAL),
                 "FINISHED_REAL") == 0);
    const char* ev = dm2_v22_famg_source_evidence();
    check("evidence cites SKULL.ASM", ev && strstr(ev, "SKULL.ASM") != NULL);
    check("evidence cites T520/T560/T600", ev && strstr(ev, "T520/T560/T600") != NULL);
    check("evidence cites DUNVIEW.C:2962-3070",
          ev && strstr(ev, "DUNVIEW.C:2962-3070") != NULL);
    check("evidence cites viewport swap",
          ev && strstr(ev, "dm2_v22_viewport_swap_pc34") != NULL);
    check("evidence cites modern assets",
          ev && strstr(ev, "dm2_v22_modern_assets_pc34") != NULL);
    check("evidence cites gap list",
          ev && strstr(ev, "FIRESTAFF_GAP_LIST") != NULL);
    check("evidence states honest boundary",
          ev && strstr(ev, "Honest boundary") != NULL);

    system("rm -rf /tmp/scratch/dm2-v22-famg-probe");
    printf("\n=== Results: %d passed, %d failed ===\n", s_pass, s_fail);
    return s_fail > 0 ? 1 : 0;
}
