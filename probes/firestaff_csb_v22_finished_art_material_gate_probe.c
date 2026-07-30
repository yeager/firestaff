/*
 * firestaff_csb_v22_finished_art_material_gate_probe.c
 *
 * Headless CSB V2.2 finished-art material gate probe. No game data or
 * SDL required. Creates synthetic modern_asset_manifest.json fixtures
 * under /tmp/scratch and verifies the CI-runnable distinction between:
 *
 *   SYNTHETIC_PLACEHOLDER - procedural placeholder manifest entries
 *   PARTIAL               - at least one real file, not all required slots
 *   FINISHED_REAL         - all tracked slots have non-placeholder
 *                           generator metadata and source_file on disk
 *
 * The CSB-only slots (chaos_rune, door_prison) dispatch through
 * CSB-specific categories (chaos_runes, door_shapes) and are covered
 * here in addition to the DM1/DM2-equivalent wall/floor/creature set.
 *
 * Source:
 *   - ReDMCSB DUNVIEW.C F0128 (CSB viewport routing)
 *   - ReDMCSB PANEL.C F0354    (CSB champion panel refresh)
 *   - CSBWin/Viewport.cpp:7290 (9-square viewport layout)
 *   - CSBWin/Chaos.cpp:60-69   (DSA / chaos rune dispatch)
 *   - include/csb_v22_inplace_draw_pc34.h (cell -> variant -> asset_id)
 *   - sibling dm1_v22 / dm2_v22 FAMG probes
 */

#include "csb_v22_finished_art_material_gate_pc34.h"
#include "fs_portable_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int s_pass = 0;
static int s_fail = 0;

typedef struct {
    CSB_V22_FamgSlot slot;
    const char* id;
    const char* category;
    int width;
    int height;
} SlotFixture;

static const SlotFixture k_slots[] = {
#define SLOT(symbol, id, category, width, height) { symbol, id, category, width, height }
    SLOT(CSB_V22_FAMG_WALL_DUNGEON_D0, "wall_dungeon_d0_01", "wall_shapes", 96, 96),
    SLOT(CSB_V22_FAMG_WALL_DUNGEON_D1, "wall_dungeon_d1_01", "wall_shapes", 96, 96),
    SLOT(CSB_V22_FAMG_WALL_DUNGEON_D2, "wall_dungeon_d2_01", "wall_shapes", 96, 96),
    SLOT(CSB_V22_FAMG_DOOR_D0, "door_d0_01", "door_shapes", 64, 96),
    SLOT(CSB_V22_FAMG_DOOR_D1, "door_d1_01", "door_shapes", 64, 96),
    SLOT(CSB_V22_FAMG_DOOR_D2, "door_d2_01", "door_shapes", 64, 96),
    SLOT(CSB_V22_FAMG_FLOOR_PLAIN_D0, "floor_plain_d0_01", "floor_shapes", 96, 96),
    SLOT(CSB_V22_FAMG_FLOOR_PLAIN_D1, "floor_plain_d1_01", "floor_shapes", 96, 96),
    SLOT(CSB_V22_FAMG_FLOOR_PLAIN_D2, "floor_plain_d2_01", "floor_shapes", 96, 96),
    SLOT(CSB_V22_FAMG_FLOOR_CRACKED_D0, "floor_cracked_d0_01", "floor_shapes", 96, 96),
    SLOT(CSB_V22_FAMG_FLOOR_CRACKED_D1, "floor_cracked_d1_01", "floor_shapes", 96, 96),
    SLOT(CSB_V22_FAMG_FLOOR_CRACKED_D2, "floor_cracked_d2_01", "floor_shapes", 96, 96),
    SLOT(CSB_V22_FAMG_FLOOR_MOSSY_D0, "floor_mossy_d0_01", "floor_shapes", 96, 96),
    SLOT(CSB_V22_FAMG_FLOOR_MOSSY_D1, "floor_mossy_d1_01", "floor_shapes", 96, 96),
    SLOT(CSB_V22_FAMG_FLOOR_MOSSY_D2, "floor_mossy_d2_01", "floor_shapes", 96, 96),
    SLOT(CSB_V22_FAMG_FLOOR_PIT, "floor_pit_01", "floor_shapes", 96, 96),
    SLOT(CSB_V22_FAMG_FLOOR_STAIRS_UP, "floor_stairs_up_01", "floor_shapes", 96, 96),
    SLOT(CSB_V22_FAMG_FLOOR_STAIRS_DOWN, "floor_stairs_down_01", "floor_shapes", 96, 96),
    SLOT(CSB_V22_FAMG_CEILING, "ceiling_01", "wall_shapes", 96, 96),
    SLOT(CSB_V22_FAMG_CREATURE_DEMON_D0, "creature_demon_d0_01", "creature_shapes", 64, 64),
    SLOT(CSB_V22_FAMG_CREATURE_DEMON_D1, "creature_demon_d1_01", "creature_shapes", 64, 64),
    SLOT(CSB_V22_FAMG_CREATURE_DEMON_D2, "creature_demon_d2_01", "creature_shapes", 64, 64),
    SLOT(CSB_V22_FAMG_PRISON_DOOR, "prison_door_01", "wall_shapes", 64, 96),
    SLOT(CSB_V22_FAMG_LORD_ORDER, "lord_order_01", "wall_shapes", 96, 96),
    SLOT(CSB_V22_FAMG_CHAOS_RUNE_0, "chaos_rune_0_01", "chaos_runes", 32, 32),
    SLOT(CSB_V22_FAMG_CHAOS_RUNE_1, "chaos_rune_1_01", "chaos_runes", 32, 32),
    SLOT(CSB_V22_FAMG_CHAOS_RUNE_2, "chaos_rune_2_01", "chaos_runes", 32, 32),
    SLOT(CSB_V22_FAMG_CHAOS_RUNE_3, "chaos_rune_3_01", "chaos_runes", 32, 32),
    SLOT(CSB_V22_FAMG_DSA_SCROLL, "dsa_scroll_01", "dsa_scrolls", 32, 32)
#undef SLOT
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
    char csb[FSP_PATH_MAX], modern[FSP_PATH_MAX];
    char physical_data_dir[FSP_PATH_MAX];
    const char* resolved_data_dir = dataDir;

    /* The production gate canonicalizes an existing data directory first;
     * on macOS /tmp is a symlink, so derive the assertion from the same
     * physical path rather than comparing two equivalent spellings. */
    if (FSP_ResolvePhysicalPath(physical_data_dir, sizeof(physical_data_dir),
                                dataDir)) {
        resolved_data_dir = physical_data_dir;
    }
    if (!FSP_ParentDir(p1, sizeof(p1), resolved_data_dir) ||
        !FSP_ParentDir(p2, sizeof(p2), p1)) {
        FSP_JoinPath(assets, sizeof(assets), resolved_data_dir, "assets");
    } else {
        FSP_JoinPath(assets, sizeof(assets), p2, "assets");
    }
    FSP_JoinPath(csb, sizeof(csb), assets, "csb");
    FSP_JoinPath(modern, sizeof(modern), csb, "modern");
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
    for (size_t i = 0; i < CSB_V22_FAMG_MATERIAL_COUNT; ++i) {
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
    write_file(path, "fake-csb-v22-material");
}

static int write_manifest(const char* path, int realMask, int firstOnly,
                          int missingFirst, int omitDimsFirst) {
    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;
    int count = firstOnly ? 1 : (int)CSB_V22_FAMG_MATERIAL_COUNT;
    fprintf(fp, "{\"manifestVersion\":\"1.0.0\",\"packId\":\"csb-v22-famg-probe\",\"slots\":[");
    for (int i = 0; i < count; ++i) {
        const SlotFixture* s = &k_slots[i];
        const char* gen = (realMask & (1 << i))
            ? "original_csb_pc34_graphics_dat" : "placeholder";
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
    /* A real V2.2 slot needs an explicit source-command admission. The test
     * fixture is intentionally synthetic on disk, but its manifest must use
     * the same schema as a reviewed pack so this probe tests the gate rather
     * than an obsolete pre-provenance manifest format. */
    fprintf(fp, "],\"routeProvenance\":[");
    for (int i = 0; i < count; ++i) {
        const SlotFixture* s = &k_slots[i];
        if (i > 0) fprintf(fp, ",");
        fprintf(fp, "{\"id\":\"%s\",\"category\":\"%s\","
                    "\"f0128ProjectionStatus\":\"admitted_probe\"}",
                s->id, s->category);
    }
    fprintf(fp, "]}");
    fclose(fp);
    return 1;
}

int main(void) {
    const char* dataDir = "/tmp/scratch/csb-v22-famg-probe/data/csb";
    char manifest[FSP_PATH_MAX];
    char modern[FSP_PATH_MAX];
    int total = 0;
    int real = 0;

    printf("=== CSB V2.2 finished-art material gate probe ===\n");
    system("rm -rf /tmp/scratch/csb-v22-famg-probe");

    printf("\n[ Scenario 1: unset path ]\n");
    csb_v22_famg_set_manifest_path(NULL);
    check("unset path -> NO_MANIFEST",
          csb_v22_famg_gate() == CSB_V22_FAMG_GATE_NO_MANIFEST);
    check("unset path -> installed=0", csb_v22_famg_get_installed() == 0);
    check("unset path -> not finished", csb_v22_famg_is_finished_real() == 0);
    for (size_t i = 0; i < CSB_V22_FAMG_MATERIAL_COUNT; ++i) {
        check("unset path -> slot MISSING",
              csb_v22_famg_classify_slot(k_slots[i].slot) ==
              CSB_V22_FAMG_CLASS_MISSING);
    }

    printf("\n[ Scenario 2: missing manifest ]\n");
    FSP_CreateDirectoryRecursive(dataDir);
    setup_paths(dataDir, manifest, sizeof(manifest), modern, sizeof(modern));
    system("rm -f /tmp/scratch/csb-v22-famg-probe/assets/csb/modern/modern_asset_manifest.json");
    csb_v22_famg_set_manifest_path(dataDir);
    check("manifest path matches expected",
          strcmp(csb_v22_famg_get_manifest_path(), manifest) == 0);
    check("missing manifest -> NO_MANIFEST",
          csb_v22_famg_gate() == CSB_V22_FAMG_GATE_NO_MANIFEST);
    check("missing manifest validate -> -1",
          csb_v22_famg_validate_manifest(NULL) == -1);

    printf("\n[ Scenario 3: empty manifest ]\n");
    check("wrote empty manifest", write_file(manifest, "{}"));
    check("empty manifest -> SYNTHETIC_PLACEHOLDER",
          csb_v22_famg_gate() == CSB_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER);
    check("empty manifest -> first slot MISSING",
          csb_v22_famg_classify_slot(CSB_V22_FAMG_WALL_DUNGEON) ==
          CSB_V22_FAMG_CLASS_MISSING);

    printf("\n[ Scenario 4: placeholder manifest ]\n");
    check("wrote placeholder manifest",
          write_manifest(manifest, 0, 0, 0, 0));
    check("placeholder -> validate 1", csb_v22_famg_validate_manifest(NULL) == 1);
    check("placeholder -> SYNTHETIC_PLACEHOLDER",
          csb_v22_famg_gate() == CSB_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER);
    for (size_t i = 0; i < CSB_V22_FAMG_MATERIAL_COUNT; ++i) {
        check("placeholder -> slot PLACEHOLDER",
              csb_v22_famg_classify_slot(k_slots[i].slot) ==
              CSB_V22_FAMG_CLASS_PLACEHOLDER);
    }
    check("placeholder -> synthetic helper",
          csb_v22_famg_is_synthetic_or_partial() == 1);

    printf("\n[ Scenario 5: partial manifest ]\n");
    create_source(modern, &k_slots[0]);
    check("wrote one-real manifest",
          write_manifest(manifest, 1, 0, 0, 0));
    check("one real -> PARTIAL",
          csb_v22_famg_gate() == CSB_V22_FAMG_GATE_PARTIAL);
    real = csb_v22_famg_real_count(&total);
    check("partial -> real_count=1", real == 1);
    check("partial -> total matches material count",
          total == (int)CSB_V22_FAMG_MATERIAL_COUNT);
    check("partial -> real slot no placeholder",
          csb_v22_famg_uses_placeholder(CSB_V22_FAMG_WALL_DUNGEON) == 0);
    check("partial -> placeholder slot uses placeholder",
          csb_v22_famg_uses_placeholder(CSB_V22_FAMG_FLOOR_PLAIN) == 1);

    printf("\n[ Scenario 6: finished-real manifest ]\n");
    for (size_t i = 0; i < CSB_V22_FAMG_MATERIAL_COUNT; ++i) {
        create_source(modern, &k_slots[i]);
    }
    check("wrote all-real manifest",
          write_manifest(manifest, (1 << CSB_V22_FAMG_MATERIAL_COUNT) - 1, 0, 0, 0));
    check("all real -> FINISHED_REAL",
          csb_v22_famg_gate() == CSB_V22_FAMG_GATE_FINISHED_REAL);
    real = csb_v22_famg_real_count(&total);
    check("finished-real -> real_count matches",
          real == (int)CSB_V22_FAMG_MATERIAL_COUNT);
    check("finished-real -> total matches",
          total == (int)CSB_V22_FAMG_MATERIAL_COUNT);
    for (size_t i = 0; i < CSB_V22_FAMG_MATERIAL_COUNT; ++i) {
        check("finished-real -> slot REAL",
              csb_v22_famg_classify_slot(k_slots[i].slot) ==
              CSB_V22_FAMG_CLASS_REAL);
    }

    printf("\n[ Scenario 7: non-placeholder metadata missing file ]\n");
    check("wrote missing-file manifest",
          write_manifest(manifest, 1, 1, 1, 0));
    check("missing file -> first slot PARTIAL",
          csb_v22_famg_classify_slot(CSB_V22_FAMG_WALL_DUNGEON) ==
          CSB_V22_FAMG_CLASS_PARTIAL);
    check("partial-only -> SYNTHETIC_PLACEHOLDER",
          csb_v22_famg_gate() == CSB_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER);

    printf("\n[ Scenario 8: invariants and source evidence ]\n");
    check("material count is 29", CSB_V22_FAMG_MATERIAL_COUNT == 29);
    for (size_t i = 0; i < CSB_V22_FAMG_MATERIAL_COUNT; ++i) {
        check("slot name matches asset id",
              strcmp(csb_v22_famg_slot_name(k_slots[i].slot), k_slots[i].id) == 0);
        check("slot category matches fixture",
              strcmp(csb_v22_famg_slot_category(k_slots[i].slot), k_slots[i].category) == 0);
    }
    check("out-of-range slot -> UNKNOWN",
          strcmp(csb_v22_famg_slot_name((CSB_V22_FamgSlot)999), "UNKNOWN") == 0);
    check("class REAL name",
          strcmp(csb_v22_famg_class_name(CSB_V22_FAMG_CLASS_REAL), "REAL") == 0);
    check("gate FINISHED_REAL name",
          strcmp(csb_v22_famg_gate_name(CSB_V22_FAMG_GATE_FINISHED_REAL),
                 "FINISHED_REAL") == 0);
    const char* ev = csb_v22_famg_source_evidence();
    check("evidence cites ReDMCSB DUNVIEW.C F0128",
          ev && strstr(ev, "DUNVIEW.C F0128") != NULL);
    check("evidence cites PANEL.C F0354",
          ev && strstr(ev, "PANEL.C F0354") != NULL);
    check("evidence cites CSBWin/Viewport.cpp:7290",
          ev && strstr(ev, "CSBWin/Viewport.cpp:7290") != NULL);
    check("evidence cites CSBWin/Chaos.cpp:60-69",
          ev && strstr(ev, "CSBWin/Chaos.cpp:60-69") != NULL);
    check("evidence cites in-place module",
          ev && strstr(ev, "csb_v22_inplace_draw") != NULL);
    check("evidence cites gap list",
          ev && strstr(ev, "FIRESTAFF_GAP_LIST") != NULL);
    check("evidence states honest boundary",
          ev && strstr(ev, "Honest boundary") != NULL);

    printf("\n[ Scenario 9: CSB-only category coverage ]\n");
    check("chaos_rune category is CSB-only 'chaos_runes'",
          strcmp(csb_v22_famg_slot_category(CSB_V22_FAMG_CHAOS_RUNE),
                 "chaos_runes") == 0);
    check("prison_door category is 'wall_shapes'",
          strcmp(csb_v22_famg_slot_category(CSB_V22_FAMG_DOOR_PRISON),
                 "wall_shapes") == 0);
    check("lord_order category is 'wall_shapes'",
          strcmp(csb_v22_famg_slot_category(CSB_V22_FAMG_LORD_ORDER),
                 "wall_shapes") == 0);

    printf("\n[ Scenario 10: per-cell 9-square routing ]\n");
    {
        int depth, lateral, count = 0;
        for (depth = 0; depth <= 2; ++depth) {
            for (lateral = -1; lateral <= 1; ++lateral) {
                check("in-range cell -> depth-specific wall slot",
                      csb_v22_famg_slot_for_cell(depth, lateral) ==
                      (CSB_V22_FamgSlot)(CSB_V22_FAMG_WALL_DUNGEON_D0 + depth));
                count++;
            }
        }
        check("covered 9 cells", count == 9);
    }
    check("out-of-range depth -1 -> sentinel",
          csb_v22_famg_slot_for_cell(-1, 0) == CSB_V22_FAMG_MATERIAL_COUNT);
    check("out-of-range depth 3 -> sentinel",
          csb_v22_famg_slot_for_cell(3, 0) == CSB_V22_FAMG_MATERIAL_COUNT);
    check("out-of-range lateral -2 -> sentinel",
          csb_v22_famg_slot_for_cell(0, -2) == CSB_V22_FAMG_MATERIAL_COUNT);
    check("out-of-range lateral 2 -> sentinel",
          csb_v22_famg_slot_for_cell(0, 2) == CSB_V22_FAMG_MATERIAL_COUNT);
    {
        csb_v22_famg_set_manifest_path(NULL);
        check("no manifest path -> classify_cell returns MISSING",
              csb_v22_famg_classify_cell(0, 0) ==
              CSB_V22_FAMG_CLASS_MISSING);
        check("out-of-range -> classify_cell returns UNKNOWN",
              csb_v22_famg_classify_cell(-1, 0) ==
              CSB_V22_FAMG_CLASS_UNKNOWN);
    }
    {
        /* Re-establish the all-real manifest + source files (Scenario 7
         * deliberately broke slot 0's source_file path so the gate can
         * observe PARTIAL). Verify all 9 cells classify as REAL once
         * the FINISHED_REAL state is restored. The unset-path check
         * above cleared the manifest path, so re-set it first. */
        csb_v22_famg_set_manifest_path(dataDir);
        for (size_t i = 0; i < CSB_V22_FAMG_MATERIAL_COUNT; ++i) {
            create_source(modern, &k_slots[i]);
        }
        check("wrote restored all-real manifest",
              write_manifest(manifest,
                             (1 << CSB_V22_FAMG_MATERIAL_COUNT) - 1,
                             0, 0, 0));
        int depth, lateral, real_cells = 0;
        for (depth = 0; depth <= 2; ++depth) {
            for (lateral = -1; lateral <= 1; ++lateral) {
                if (csb_v22_famg_classify_cell(depth, lateral) ==
                    CSB_V22_FAMG_CLASS_REAL) {
                    real_cells++;
                }
            }
        }
        check("all 9 cells REAL after FINISHED_REAL", real_cells == 9);
    }

    system("rm -rf /tmp/scratch/csb-v22-famg-probe");
    printf("\n=== Results: %d passed, %d failed ===\n", s_pass, s_fail);
    return s_fail > 0 ? 1 : 0;
}
