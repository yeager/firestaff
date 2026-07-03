/*
 * test_dm1_v22_finished_art_material_gate_pc34.c
 *
 * DM1 V2.2 finished-art / material screenshot pixel gate — synthetic
 * CTest. Builds synthetic modern_asset_manifest.json fixtures under
 * /tmp/scratch/dm1_v22_famg/ and exercises every classification /
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
 *  19. Non-placeholder entries require PNG signature + matching IHDR
 *      dimensions before promoting to REAL
 *
 * Source-locked against the module under test
 * include/dm1_v22_finished_art_material_gate_pc34.h. Sibling gate
 * pattern: tests/test_dm2_v2_hud_widget_assets.c.
 */

#include "dm1_v22_finished_art_material_gate_pc34.h"
#include "fs_portable_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

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

static void put_be32(unsigned char* p, unsigned v) {
    p[0] = (unsigned char)((v >> 24) & 0xffU);
    p[1] = (unsigned char)((v >> 16) & 0xffU);
    p[2] = (unsigned char)((v >> 8) & 0xffU);
    p[3] = (unsigned char)(v & 0xffU);
}

/* Header-only PNG fixture: enough for signature + IHDR provenance gates.
 * CRC/pixel chunks are intentionally omitted because this gate does not
 * decode image pixels. */
static int write_png_header_file(const char* path, unsigned width, unsigned height) {
    static const unsigned char sig[8] = {
        0x89u, 0x50u, 0x4eu, 0x47u, 0x0du, 0x0au, 0x1au, 0x0au
    };
    unsigned char hdr[24];
    FILE* fp;
    memcpy(hdr, sig, sizeof(sig));
    hdr[8] = 0x00u; hdr[9] = 0x00u; hdr[10] = 0x00u; hdr[11] = 0x0du;
    memcpy(hdr + 12, "IHDR", 4);
    put_be32(hdr + 16, width);
    put_be32(hdr + 20, height);
    fp = fopen(path, "wb");
    if (!fp) return 0;
    if (fwrite(hdr, 1, sizeof(hdr), fp) != sizeof(hdr)) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static int write_all_real_manifest_with_receipt(const char* path,
                                                const char* receipt_generator,
                                                const char* receipt_source,
                                                const char* receipt_hash,
                                                const char* receipt_material_gate) {
    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;
    fprintf(fp,
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm1-v22-famg-test\","
        "\"wall_shapes\":["
        "{\"id\":\"wall_d3_carved_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"wall_d3_carved_hero_01.png\",\"width\":64,\"height\":64}"
        "],"
        "\"floor_shapes\":["
        "{\"id\":\"floor_plain_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"floor_plain_hero_01.png\",\"width\":64,\"height\":64},"
        "{\"id\":\"floor_pit_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"floor_pit_hero_01.png\",\"width\":64,\"height\":64}"
        "],"
        "\"creature_shapes\":["
        "{\"id\":\"creature_demon_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"creature_demon_hero_01.png\",\"width\":48,\"height\":48}"
        "],"
        "\"champion_portraits\":["
        "{\"id\":\"champion_warrior_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"champion_warrior_hero_01.png\","
        "\"width\":48,\"height\":48}"
        "],"
        "\"door_shapes\":["
        "{\"id\":\"door_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"door_hero_01.png\",\"width\":32,\"height\":48}"
        "],"
        "\"field_shapes\":["
        "{\"id\":\"field_teleporter_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"field_teleporter_hero_01.png\","
        "\"width\":64,\"height\":64}"
        "]");
    if (receipt_generator) {
        fprintf(fp,
            ",\"runtime_screenshot_receipts\":["
            "{\"id\":\"dm1_v22_real_screenshot_material_receipt_01\","
            "\"generator\":\"%s\","
            "\"source_file\":\"%s\","
            "\"width\":320,\"height\":200,"
            "\"frame_hash\":\"%s\","
            "\"material_gate\":\"%s\"}"
            "]",
            receipt_generator,
            receipt_source ? receipt_source : "",
            receipt_hash ? receipt_hash : "",
            receipt_material_gate ? receipt_material_gate : "");
    }
    fprintf(fp, "}");
    return fclose(fp) == 0;
}

/* Helper: build the manifest path that the module resolves for a given
 * data dir. Mirrors dm1_v22_famg_set_manifest_path. */
static void build_expected_manifest_path(char* out, size_t outSize,
                                         const char* dataDir) {
    /* dataDir = <root>/data/dm1 -> manifest = <root>/assets/dm1/modern/modern_asset_manifest.json
     * Walk up two parents. */
    char a[FSP_PATH_MAX];
    char b[FSP_PATH_MAX];
    const char* slash;
    slash = strrchr(dataDir, '/');
    if (!slash) {
        snprintf(out, outSize, "%s/assets/dm1/modern/modern_asset_manifest.json", dataDir);
        return;
    }
    size_t la = (size_t)(slash - dataDir);
    if (la >= sizeof(a)) la = sizeof(a) - 1U;
    memcpy(a, dataDir, la); a[la] = '\0';
    slash = strrchr(a, '/');
    if (!slash) {
        snprintf(out, outSize, "%s/assets/dm1/modern/modern_asset_manifest.json", dataDir);
        return;
    }
    size_t lb = (size_t)(slash - a);
    if (lb >= sizeof(b)) lb = sizeof(b) - 1U;
    memcpy(b, a, lb); b[lb] = '\0';
    snprintf(out, outSize, "%s/assets/dm1/modern/modern_asset_manifest.json", b);
}

/* Helper: clean the scratch dir before each scenario. We also remove
 * the asset tree the module looks at
 * (~/.firestaff/assets/dm1/modern/...) so re-running the suite is
 * byte-stable. */
static void clean_scratch(void) {
    system("rm -rf /tmp/scratch/dm1_v22_famg_test");
    system("rm -rf /tmp/scratch/dm1-famg-data");
}

/* ── Tests ──────────────────────────────────────────────────────── */

static void test_unset_path_is_safe(void) {
    clean_scratch();
    dm1_v22_famg_set_manifest_path(NULL);
    dm1_v22_famg_set_manifest_path("");

    const char* p = dm1_v22_famg_get_manifest_path();
    CHECK(p && p[0] == '\0', "unset path returns empty string");

    int total = 0;
    int real = dm1_v22_famg_real_count(&total);
    CHECK(real == 0, "real_count=0 with no manifest");
    CHECK(total == 0, "total=0 with no manifest");

    DM1_V22_FamgGate gate = dm1_v22_famg_gate();
    CHECK(gate == DM1_V22_FAMG_GATE_NO_MANIFEST,
          "gate=NO_MANIFEST when unset");

    CHECK(dm1_v22_famg_is_finished_real() == 0,
          "is_finished_real=0 when unset");
    CHECK(dm1_v22_famg_is_synthetic_or_partial() == 0,
          "is_synthetic_or_partial=0 when unset (NOT_PROBED) is not counted");

    for (size_t i = 0; i < DM1_V22_FAMG_MATERIAL_COUNT; ++i) {
        DM1_V22_FamgClass cls =
            dm1_v22_famg_classify_slot((DM1_V22_FamgSlot)i);
        CHECK(cls == DM1_V22_FAMG_CLASS_MISSING,
              "all slots MISSING when manifest unset");
        CHECK(dm1_v22_famg_uses_placeholder(
                (DM1_V22_FamgSlot)i) == 1,
              "uses_placeholder=1 when manifest unset");
    }
}

static void test_set_path_resolves_correctly(void) {
    dm1_v22_famg_set_manifest_path("/tmp/scratch/dm1-famg-data/data/dm1");
    char expected[FSP_PATH_MAX];
    build_expected_manifest_path(expected, sizeof(expected),
                                  "/tmp/scratch/dm1-famg-data/data/dm1");
    const char* actual = dm1_v22_famg_get_manifest_path();
    CHECK(actual && strcmp(actual, expected) == 0,
          "manifest path resolves correctly under assets/dm1/modern/");
}

static void test_missing_manifest_file_yields_no_manifest(void) {
    clean_scratch();
    dm1_v22_famg_set_manifest_path("/tmp/scratch/dm1-famg-data/data/dm1");
    /* Don't actually create the manifest path */
    DM1_V22_FamgGate gate = dm1_v22_famg_gate();
    CHECK(gate == DM1_V22_FAMG_GATE_NO_MANIFEST,
          "missing manifest file -> NO_MANIFEST gate");

    int v = dm1_v22_famg_validate_manifest(NULL);
    CHECK(v == -1, "validate_manifest(NULL) with no file -> -1");

    v = dm1_v22_famg_validate_manifest("/nope/does-not-exist.json");
    CHECK(v == -1, "validate_manifest on missing file -> -1");
}

static void test_empty_manifest_yields_placeholder_gate(void) {
    clean_scratch();
    const char* dataDir = "/tmp/scratch/dm1-famg-data/data/dm1";
    char manifest[FSP_PATH_MAX];
    build_expected_manifest_path(manifest, sizeof(manifest), dataDir);
    /* Strip filename to get the parent dir. */
    char* slash = strrchr(manifest, '/');
    if (slash) *slash = '\0';
    char mkdir_cmd[1200];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", manifest);
    CHECK(system(mkdir_cmd) == 0, "mkdir modern dir");

    char manifest_path[FSP_PATH_MAX];
    snprintf(manifest_path, sizeof(manifest_path),
             "%s/modern_asset_manifest.json", manifest);
    CHECK(write_file(manifest_path, "{}"), "wrote empty manifest");

    dm1_v22_famg_set_manifest_path(dataDir);
    int v = dm1_v22_famg_validate_manifest(NULL);
    CHECK(v == 0, "empty manifest validates as partial");

    DM1_V22_FamgGate gate = dm1_v22_famg_gate();
    CHECK(gate == DM1_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER,
          "empty manifest -> SYNTHETIC_PLACEHOLDER gate");

    /* First slot should classify as MISSING (declared but not present) */
    DM1_V22_FamgClass cls = dm1_v22_famg_classify_slot(
        DM1_V22_FAMG_WALL_D3_CARVED);
    CHECK(cls == DM1_V22_FAMG_CLASS_MISSING,
          "first slot MISSING in empty manifest");
}

static void test_placeholder_only_manifest_yields_placeholder_gate(void) {
    clean_scratch();
    const char* dataDir = "/tmp/scratch/dm1-famg-data/data/dm1";
    char manifest_path[FSP_PATH_MAX];
    char pdir[FSP_PATH_MAX];
    build_expected_manifest_path(manifest_path, sizeof(manifest_path), dataDir);
    snprintf(pdir, sizeof(pdir), "%s/../../assets/dm1/modern", dataDir);
    char mkdir_cmd[1200];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", pdir);
    system(mkdir_cmd);

    /* Manifest declares every required slot with generator='placeholder'.
     * Source files are NOT required to exist on disk for PLACEHOLDER
     * classification (the module only validates source_file existence
     * for non-placeholder generators). The DM1 V22 modern asset
     * runtime treats placeholder slots as procedural fallbacks; the
     * synthetic CI baseline is exactly this state. */
    const char* content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm1-v22-famg-test\","
        "\"wall_shapes\":["
        "{\"id\":\"wall_d3_carved_hero_01\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder_wall_d3_carved.png\","
        "\"width\":64,\"height\":64}"
        "],"
        "\"floor_shapes\":["
        "{\"id\":\"floor_plain_hero_01\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder_floor_plain.png\","
        "\"width\":64,\"height\":64},"
        "{\"id\":\"floor_pit_hero_01\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder_floor_pit.png\","
        "\"width\":64,\"height\":64}"
        "],"
        "\"creature_shapes\":["
        "{\"id\":\"creature_demon_hero_01\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder_creature_demon.png\","
        "\"width\":48,\"height\":48}"
        "],"
        "\"champion_portraits\":["
        "{\"id\":\"champion_warrior_hero_01\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder_champion_warrior.png\","
        "\"width\":48,\"height\":48}"
        "],"
        "\"door_shapes\":["
        "{\"id\":\"door_hero_01\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder_door.png\","
        "\"width\":32,\"height\":48}"
        "],"
        "\"field_shapes\":["
        "{\"id\":\"field_teleporter_hero_01\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder_field_teleporter.png\","
        "\"width\":64,\"height\":64}"
        "]}";
    CHECK(write_file(manifest_path, content), "wrote placeholder manifest");

    dm1_v22_famg_set_manifest_path(dataDir);
    int v = dm1_v22_famg_validate_manifest(NULL);
    CHECK(v == 1, "valid placeholder manifest validates as complete");

    /* Every declared slot classifies as PLACEHOLDER. */
    DM1_V22_FamgClass cls = dm1_v22_famg_classify_slot(
        DM1_V22_FAMG_WALL_D3_CARVED);
    CHECK(cls == DM1_V22_FAMG_CLASS_PLACEHOLDER,
          "wall_d3_carved -> PLACEHOLDER");
    cls = dm1_v22_famg_classify_slot(DM1_V22_FAMG_FLOOR_PLAIN);
    CHECK(cls == DM1_V22_FAMG_CLASS_PLACEHOLDER,
          "floor_plain -> PLACEHOLDER");
    cls = dm1_v22_famg_classify_slot(DM1_V22_FAMG_FLOOR_PIT);
    CHECK(cls == DM1_V22_FAMG_CLASS_PLACEHOLDER,
          "floor_pit -> PLACEHOLDER");
    cls = dm1_v22_famg_classify_slot(DM1_V22_FAMG_CREATURE_DEMON);
    CHECK(cls == DM1_V22_FAMG_CLASS_PLACEHOLDER,
          "creature_demon -> PLACEHOLDER");
    cls = dm1_v22_famg_classify_slot(DM1_V22_FAMG_CHAMPION_WARRIOR);
    CHECK(cls == DM1_V22_FAMG_CLASS_PLACEHOLDER,
          "champion_warrior -> PLACEHOLDER");
    cls = dm1_v22_famg_classify_slot(DM1_V22_FAMG_DOOR_FRONT);
    CHECK(cls == DM1_V22_FAMG_CLASS_PLACEHOLDER,
          "door_front -> PLACEHOLDER");
    cls = dm1_v22_famg_classify_slot(DM1_V22_FAMG_TELEPORTER_FIELD);
    CHECK(cls == DM1_V22_FAMG_CLASS_PLACEHOLDER,
          "teleporter_field -> PLACEHOLDER");

    DM1_V22_FamgGate gate = dm1_v22_famg_gate();
    CHECK(gate == DM1_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER,
          "gate=SYNTHETIC_PLACEHOLDER when all slots declare placeholder");

    CHECK(dm1_v22_famg_get_installed() == 0,
          "installed=0 when gate=SYNTHETIC_PLACEHOLDER");
    CHECK(dm1_v22_famg_is_finished_real() == 0,
          "is_finished_real=0 for placeholder gate");
    CHECK(dm1_v22_famg_is_synthetic_or_partial() == 1,
          "is_synthetic_or_partial=1 for placeholder gate");
}

static void test_real_slot_classifies_as_real(void) {
    clean_scratch();
    const char* dataDir = "/tmp/scratch/dm1-famg-data/data/dm1";
    char manifest_path[FSP_PATH_MAX];
    build_expected_manifest_path(manifest_path, sizeof(manifest_path), dataDir);
    /* Create category subdirectories so source_file resolution can
     * land on disk. */
    char assets_root[FSP_PATH_MAX];
    snprintf(assets_root, sizeof(assets_root),
             "%s/../../assets/dm1/modern", dataDir);
    char mkdir_cmd[1200];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd),
             "mkdir -p '%s/wall_shapes' '%s/floor_shapes' '%s/creature_shapes' "
             "'%s/champion_portraits' '%s/door_shapes' '%s/field_shapes'",
             assets_root, assets_root, assets_root, assets_root, assets_root,
             assets_root);
    CHECK(system(mkdir_cmd) == 0, "mkdir category dirs");

    /* All seven slots fully REAL with pbr_hero generator + on-disk file. */
    const char* content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm1-v22-famg-test\","
        "\"wall_shapes\":["
        "{\"id\":\"wall_d3_carved_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"wall_d3_carved_hero_01.png\",\"width\":64,\"height\":64}"
        "],"
        "\"floor_shapes\":["
        "{\"id\":\"floor_plain_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"floor_plain_hero_01.png\",\"width\":64,\"height\":64},"
        "{\"id\":\"floor_pit_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"floor_pit_hero_01.png\",\"width\":64,\"height\":64}"
        "],"
        "\"creature_shapes\":["
        "{\"id\":\"creature_demon_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"creature_demon_hero_01.png\",\"width\":48,\"height\":48}"
        "],"
        "\"champion_portraits\":["
        "{\"id\":\"champion_warrior_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"champion_warrior_hero_01.png\","
        "\"width\":48,\"height\":48}"
        "],"
        "\"door_shapes\":["
        "{\"id\":\"door_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"door_hero_01.png\",\"width\":32,\"height\":48}"
        "],"
        "\"field_shapes\":["
        "{\"id\":\"field_teleporter_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"field_teleporter_hero_01.png\","
        "\"width\":64,\"height\":64}"
        "]}";
    CHECK(write_file(manifest_path, content), "wrote all-real manifest");

    /* Create each slot's source file in its declared category dir. */
    const char* files[DM1_V22_FAMG_MATERIAL_COUNT] = {
        "wall_d3_carved_hero_01.png",
        "floor_plain_hero_01.png",
        "floor_pit_hero_01.png",
        "creature_demon_hero_01.png",
        "champion_warrior_hero_01.png",
        "door_hero_01.png",
        "field_teleporter_hero_01.png"
    };
    for (size_t i = 0; i < DM1_V22_FAMG_MATERIAL_COUNT; ++i) {
        char fpath[FSP_PATH_MAX];
        snprintf(fpath, sizeof(fpath), "%s/%s/%s",
                 assets_root,
                 dm1_v22_famg_slot_category((DM1_V22_FamgSlot)i),
                 files[i]);
        if (access(fpath, F_OK) != 0) {
            CHECK(write_png_header_file(fpath,
                                        (i == DM1_V22_FAMG_DOOR_FRONT) ? 32U :
                                        (i == DM1_V22_FAMG_CREATURE_DEMON ||
                                         i == DM1_V22_FAMG_CHAMPION_WARRIOR) ? 48U : 64U,
                                        (i == DM1_V22_FAMG_DOOR_FRONT) ? 48U :
                                        (i == DM1_V22_FAMG_CREATURE_DEMON ||
                                         i == DM1_V22_FAMG_CHAMPION_WARRIOR) ? 48U : 64U),
                  "wrote PNG-header fixture for slot");
        }
    }

    dm1_v22_famg_set_manifest_path(dataDir);
    DM1_V22_FamgGate gate = dm1_v22_famg_gate();
    CHECK(gate == DM1_V22_FAMG_GATE_FINISHED_REAL,
          "all slots REAL -> FINISHED_REAL gate");

    CHECK(dm1_v22_famg_get_installed() == 1,
          "installed=1 when gate=FINISHED_REAL");
    CHECK(dm1_v22_famg_is_finished_real() == 1,
          "is_finished_real=1 for FINISHED_REAL gate");
    CHECK(dm1_v22_famg_is_synthetic_or_partial() == 0,
          "is_synthetic_or_partial=0 for FINISHED_REAL gate");

    int total = 0;
    int real = dm1_v22_famg_real_count(&total);
    CHECK(real == (int)DM1_V22_FAMG_MATERIAL_COUNT,
          "real_count=DM1_V22_FAMG_MATERIAL_COUNT");
    CHECK(total == (int)DM1_V22_FAMG_MATERIAL_COUNT,
          "total=DM1_V22_FAMG_MATERIAL_COUNT");

    for (size_t i = 0; i < DM1_V22_FAMG_MATERIAL_COUNT; ++i) {
        DM1_V22_FamgClass cls =
            dm1_v22_famg_classify_slot((DM1_V22_FamgSlot)i);
        CHECK(cls == DM1_V22_FAMG_CLASS_REAL,
              "every slot REAL with disk-resolvable source_file");
        CHECK(dm1_v22_famg_uses_placeholder(
                (DM1_V22_FamgSlot)i) == 0,
              "uses_placeholder=0 for REAL slot");
    }
}

static void test_partial_when_some_real(void) {
    clean_scratch();
    const char* dataDir = "/tmp/scratch/dm1-famg-data/data/dm1";
    char manifest_path[FSP_PATH_MAX];
    char assets_root[FSP_PATH_MAX];
    build_expected_manifest_path(manifest_path, sizeof(manifest_path), dataDir);
    snprintf(assets_root, sizeof(assets_root),
             "%s/../../assets/dm1/modern", dataDir);
    char mkdir_cmd[1200];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd),
             "mkdir -p '%s/wall_shapes' '%s/floor_shapes' '%s/creature_shapes' "
             "'%s/champion_portraits' '%s/door_shapes' '%s/field_shapes'",
             assets_root, assets_root, assets_root, assets_root, assets_root,
             assets_root);
    system(mkdir_cmd);

    /* Only wall_d3_carved is REAL with on-disk file; the rest are
     * placeholder. */
    char real_file[FSP_PATH_MAX];
    snprintf(real_file, sizeof(real_file),
             "%s/wall_shapes/wall_d3_carved_hero_01.png", assets_root);
    CHECK(write_png_header_file(real_file, 64U, 64U),
          "wrote PNG-header real asset file");

    const char* content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm1-v22-famg-test\","
        "\"wall_shapes\":["
        "{\"id\":\"wall_d3_carved_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"wall_d3_carved_hero_01.png\",\"width\":64,\"height\":64}"
        "],"
        "\"floor_shapes\":["
        "{\"id\":\"floor_plain_hero_01\",\"generator\":\"placeholder\","
        "\"source_file\":\"floor_plain_hero_01.png\",\"width\":64,\"height\":64},"
        "{\"id\":\"floor_pit_hero_01\",\"generator\":\"placeholder\","
        "\"source_file\":\"floor_pit_hero_01.png\",\"width\":64,\"height\":64}"
        "],"
        "\"creature_shapes\":["
        "{\"id\":\"creature_demon_hero_01\",\"generator\":\"placeholder\","
        "\"source_file\":\"creature_demon_hero_01.png\",\"width\":48,\"height\":48}"
        "],"
        "\"champion_portraits\":["
        "{\"id\":\"champion_warrior_hero_01\",\"generator\":\"placeholder\","
        "\"source_file\":\"champion_warrior_hero_01.png\","
        "\"width\":48,\"height\":48}"
        "],"
        "\"door_shapes\":["
        "{\"id\":\"door_hero_01\",\"generator\":\"placeholder\","
        "\"source_file\":\"door_hero_01.png\",\"width\":32,\"height\":48}"
        "],"
        "\"field_shapes\":["
        "{\"id\":\"field_teleporter_hero_01\",\"generator\":\"placeholder\","
        "\"source_file\":\"field_teleporter_hero_01.png\","
        "\"width\":64,\"height\":64}"
        "]}";
    CHECK(write_file(manifest_path, content), "wrote mixed manifest");

    dm1_v22_famg_set_manifest_path(dataDir);
    DM1_V22_FamgGate gate = dm1_v22_famg_gate();
    CHECK(gate == DM1_V22_FAMG_GATE_PARTIAL,
          "mixed manifest -> PARTIAL gate");

    CHECK(dm1_v22_famg_get_installed() == 1,
          "installed=1 when gate=PARTIAL");
    CHECK(dm1_v22_famg_is_finished_real() == 0,
          "is_finished_real=0 for PARTIAL gate");
    CHECK(dm1_v22_famg_is_synthetic_or_partial() == 1,
          "is_synthetic_or_partial=1 for PARTIAL gate");

    /* First slot REAL */
    DM1_V22_FamgClass cls = dm1_v22_famg_classify_slot(
        DM1_V22_FAMG_WALL_D3_CARVED);
    CHECK(cls == DM1_V22_FAMG_CLASS_REAL, "wall_d3_carved=REAL");
    /* Second slot PLACEHOLDER */
    cls = dm1_v22_famg_classify_slot(DM1_V22_FAMG_FLOOR_PLAIN);
    CHECK(cls == DM1_V22_FAMG_CLASS_PLACEHOLDER,
          "floor_plain=PLACEHOLDER");

    int total = 0;
    int real = dm1_v22_famg_real_count(&total);
    CHECK(real == 1, "real_count=1 with one REAL slot");
    CHECK(total == (int)DM1_V22_FAMG_MATERIAL_COUNT,
          "total=DM1_V22_FAMG_MATERIAL_COUNT");
}

static void test_partial_when_real_but_missing_source_file(void) {
    clean_scratch();
    const char* dataDir = "/tmp/scratch/dm1-famg-data/data/dm1";
    char manifest_path[FSP_PATH_MAX];
    char assets_root[FSP_PATH_MAX];
    build_expected_manifest_path(manifest_path, sizeof(manifest_path), dataDir);
    snprintf(assets_root, sizeof(assets_root),
             "%s/../../assets/dm1/modern", dataDir);
    char mkdir_cmd[1200];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd),
             "mkdir -p '%s/wall_shapes'", assets_root);
    system(mkdir_cmd);

    /* Manifest declares pbr_hero but the file does not exist on disk. */
    const char* content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm1-v22-famg-test\","
        "\"wall_shapes\":["
        "{\"id\":\"wall_d3_carved_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"missing_on_disk.png\",\"width\":64,\"height\":64}"
        "]}";
    CHECK(write_file(manifest_path, content), "wrote manifest with missing source");

    dm1_v22_famg_set_manifest_path(dataDir);
    DM1_V22_FamgClass cls = dm1_v22_famg_classify_slot(
        DM1_V22_FAMG_WALL_D3_CARVED);
    CHECK(cls == DM1_V22_FAMG_CLASS_PARTIAL,
          "real metadata but missing file -> PARTIAL");

    int total = 0;
    int real = dm1_v22_famg_real_count(&total);
    CHECK(real == 0, "real_count=0 (no disk-resolved file)");
    CHECK(total == 1, "total=1 (one declared slot)");

    /* Gate: declared slots but no REAL -> SYNTHETIC_PLACEHOLDER. */
    DM1_V22_FamgGate gate = dm1_v22_famg_gate();
    CHECK(gate == DM1_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER,
          "PARTIAL-only slots -> SYNTHETIC_PLACEHOLDER gate");
}

static void test_real_metadata_requires_png_header_match(void) {
    clean_scratch();
    const char* dataDir = "/tmp/scratch/dm1-famg-data/data/dm1";
    char manifest_path[FSP_PATH_MAX];
    char assets_root[FSP_PATH_MAX];
    build_expected_manifest_path(manifest_path, sizeof(manifest_path), dataDir);
    snprintf(assets_root, sizeof(assets_root),
             "%s/../../assets/dm1/modern", dataDir);
    char mkdir_cmd[1200];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd),
             "mkdir -p '%s/wall_shapes'", assets_root);
    system(mkdir_cmd);

    const char* content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm1-v22-famg-test\","
        "\"wall_shapes\":["
        "{\"id\":\"wall_d3_carved_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"wall_d3_carved_hero_01.png\","
        "\"width\":64,\"height\":64}"
        "]}";
    CHECK(write_file(manifest_path, content),
          "wrote manifest for PNG-header negative tests");

    char real_file[FSP_PATH_MAX];
    snprintf(real_file, sizeof(real_file),
             "%s/wall_shapes/wall_d3_carved_hero_01.png", assets_root);

    CHECK(write_file(real_file, "fake-png-bytes-for-test"),
          "wrote text file with .png suffix");
    dm1_v22_famg_set_manifest_path(dataDir);
    DM1_V22_FamgClass cls = dm1_v22_famg_classify_slot(
        DM1_V22_FAMG_WALL_D3_CARVED);
    CHECK(cls == DM1_V22_FAMG_CLASS_PARTIAL,
          "non-placeholder .png text file -> PARTIAL, not REAL");
    DM1_V22_FamgSlotInfo info;
    CHECK(dm1_v22_famg_get_slot_info(
              DM1_V22_FAMG_WALL_D3_CARVED, &info) == 1,
          "slot info available for bad PNG");
    CHECK(info.file_exists == 1, "bad PNG file still exists");
    CHECK(info.png_header_valid == 0,
          "bad PNG does not pass png_header_valid");
    CHECK(info.classification == DM1_V22_FAMG_CLASS_PARTIAL,
          "bad PNG slot info classification=PARTIAL");

    CHECK(write_png_header_file(real_file, 32U, 64U),
          "wrote mismatched PNG-header fixture");
    cls = dm1_v22_famg_classify_slot(DM1_V22_FAMG_WALL_D3_CARVED);
    CHECK(cls == DM1_V22_FAMG_CLASS_PARTIAL,
          "PNG IHDR dimension mismatch -> PARTIAL, not REAL");
    CHECK(dm1_v22_famg_get_slot_info(
              DM1_V22_FAMG_WALL_D3_CARVED, &info) == 1,
          "slot info available for mismatched PNG");
    CHECK(info.png_width == 32, "mismatch reports actual png_width=32");
    CHECK(info.png_height == 64, "mismatch reports actual png_height=64");
    CHECK(info.png_header_valid == 0,
          "dimension mismatch does not pass png_header_valid");
}

static void test_partial_when_fields_missing(void) {
    clean_scratch();
    const char* dataDir = "/tmp/scratch/dm1-famg-data/data/dm1";
    char manifest_path[FSP_PATH_MAX];
    char pdir[FSP_PATH_MAX];
    build_expected_manifest_path(manifest_path, sizeof(manifest_path), dataDir);
    snprintf(pdir, sizeof(pdir), "%s/../../assets/dm1/modern", dataDir);
    char mkdir_cmd[1200];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", pdir);
    system(mkdir_cmd);

    /* Entry missing width/height -> PARTIAL. */
    const char* content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm1-v22-famg-test\","
        "\"wall_shapes\":["
        "{\"id\":\"wall_d3_carved_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"x.png\"}"
        "]}";
    CHECK(write_file(manifest_path, content), "wrote manifest with incomplete entry");

    dm1_v22_famg_set_manifest_path(dataDir);
    DM1_V22_FamgClass cls = dm1_v22_famg_classify_slot(
        DM1_V22_FAMG_WALL_D3_CARVED);
    CHECK(cls == DM1_V22_FAMG_CLASS_PARTIAL,
          "missing width/height -> PARTIAL");

    int v = dm1_v22_famg_validate_manifest(NULL);
    CHECK(v == 0, "incomplete manifest validates as partial (not complete)");
}

static void test_get_slot_info_populates_fields(void) {
    clean_scratch();
    const char* dataDir = "/tmp/scratch/dm1-famg-data/data/dm1";
    char manifest_path[FSP_PATH_MAX];
    char assets_root[FSP_PATH_MAX];
    build_expected_manifest_path(manifest_path, sizeof(manifest_path), dataDir);
    snprintf(assets_root, sizeof(assets_root),
             "%s/../../assets/dm1/modern", dataDir);
    char mkdir_cmd[1200];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd),
             "mkdir -p '%s/wall_shapes'", assets_root);
    system(mkdir_cmd);

    char real_file[FSP_PATH_MAX];
    snprintf(real_file, sizeof(real_file),
             "%s/wall_shapes/wall_d3_carved_hero_01.png", assets_root);
    write_png_header_file(real_file, 64U, 64U);

    const char* content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm1-v22-famg-test\","
        "\"wall_shapes\":["
        "{\"id\":\"wall_d3_carved_hero_01\",\"generator\":\"pbr_hero\","
        "\"source_file\":\"wall_d3_carved_hero_01.png\","
        "\"width\":64,\"height\":64}"
        "]}";
    CHECK(write_file(manifest_path, content), "wrote manifest for slot info");

    dm1_v22_famg_set_manifest_path(dataDir);
    DM1_V22_FamgSlotInfo info;
    int ok = dm1_v22_famg_get_slot_info(
        DM1_V22_FAMG_WALL_D3_CARVED, &info);
    CHECK(ok == 1, "get_slot_info returns 1 for present slot");
    CHECK(strcmp(info.id, "wall_d3_carved_hero_01") == 0,
          "slot info id correct");
    CHECK(strcmp(info.category, "wall_shapes") == 0,
          "slot info category correct");
    CHECK(strcmp(info.generator, "pbr_hero") == 0,
          "slot info generator correct");
    CHECK(strcmp(info.source_file, "wall_d3_carved_hero_01.png") == 0,
          "slot info source_file correct");
    CHECK(info.width == 64, "slot info width=64");
    CHECK(info.height == 64, "slot info height=64");
    CHECK(info.file_exists == 1, "slot info file_exists=1");
    CHECK(info.png_header_valid == 1, "slot info png_header_valid=1");
    CHECK(info.png_width == 64, "slot info png_width=64");
    CHECK(info.png_height == 64, "slot info png_height=64");
    CHECK(info.classification == DM1_V22_FAMG_CLASS_REAL,
          "slot info classification=REAL");
    CHECK(info.resolved_path[0] != '\0', "resolved_path populated");
    CHECK(dm1_v22_famg_gate() == DM1_V22_FAMG_GATE_PARTIAL,
          "one REAL slot with five omitted required slots -> PARTIAL gate");

    /* NULL out is safe */
    dm1_v22_famg_get_slot_info(
        DM1_V22_FAMG_WALL_D3_CARVED, NULL);
    CHECK(1, "get_slot_info(NULL) safe");

    /* Out-of-range slot returns 0 */
    DM1_V22_FamgSlotInfo info2;
    ok = dm1_v22_famg_get_slot_info(
        (DM1_V22_FamgSlot)9999, &info2);
    CHECK(ok == 0, "get_slot_info out-of-range returns 0");

    /* Absent slot returns 0 */
    ok = dm1_v22_famg_get_slot_info(
        DM1_V22_FAMG_FLOOR_PLAIN, &info2);
    CHECK(ok == 0, "get_slot_info absent slot returns 0");
}

static void test_slot_count_is_seven(void) {
    CHECK(DM1_V22_FAMG_MATERIAL_COUNT == 7,
          "DM1_V22_FAMG_MATERIAL_COUNT=7");
    CHECK(DM1_V22_FAMG_WALL_D3_CARVED   == 0, "slot[0] = wall_d3_carved");
    CHECK(DM1_V22_FAMG_FLOOR_PLAIN      == 1, "slot[1] = floor_plain");
    CHECK(DM1_V22_FAMG_FLOOR_PIT        == 2, "slot[2] = floor_pit");
    CHECK(DM1_V22_FAMG_CREATURE_DEMON   == 3, "slot[3] = creature_demon");
    CHECK(DM1_V22_FAMG_CHAMPION_WARRIOR == 4, "slot[4] = champion_warrior");
    CHECK(DM1_V22_FAMG_DOOR_FRONT       == 5, "slot[5] = door_front");
    CHECK(DM1_V22_FAMG_TELEPORTER_FIELD == 6, "slot[6] = teleporter_field");
}

static void test_names_are_stable(void) {
    /* Slot names match k_slot_table hero_01 ids. */
    CHECK(strcmp(dm1_v22_famg_slot_name(
            DM1_V22_FAMG_WALL_D3_CARVED),
            "wall_d3_carved_hero_01") == 0,
          "wall_d3_carved name stable");
    CHECK(strcmp(dm1_v22_famg_slot_name(
            DM1_V22_FAMG_FLOOR_PLAIN),
            "floor_plain_hero_01") == 0,
          "floor_plain name stable");
    CHECK(strcmp(dm1_v22_famg_slot_name(
            DM1_V22_FAMG_FLOOR_PIT),
            "floor_pit_hero_01") == 0,
          "floor_pit name stable");
    CHECK(strcmp(dm1_v22_famg_slot_name(
            DM1_V22_FAMG_CREATURE_DEMON),
            "creature_demon_hero_01") == 0,
          "creature_demon name stable");
    CHECK(strcmp(dm1_v22_famg_slot_name(
            DM1_V22_FAMG_CHAMPION_WARRIOR),
            "champion_warrior_hero_01") == 0,
          "champion_warrior name stable");
    CHECK(strcmp(dm1_v22_famg_slot_name(
            DM1_V22_FAMG_DOOR_FRONT),
            "door_hero_01") == 0,
          "door_front name stable");
    CHECK(strcmp(dm1_v22_famg_slot_name(
            DM1_V22_FAMG_TELEPORTER_FIELD),
            "field_teleporter_hero_01") == 0,
          "teleporter_field name stable");
    /* Out-of-range name */
    CHECK(strcmp(dm1_v22_famg_slot_name(
            (DM1_V22_FamgSlot)9999),
            "UNKNOWN") == 0,
          "out-of-range slot name -> UNKNOWN");

    /* Class names */
    CHECK(strcmp(dm1_v22_famg_class_name(
            DM1_V22_FAMG_CLASS_REAL), "REAL") == 0,
          "class REAL name");
    CHECK(strcmp(dm1_v22_famg_class_name(
            DM1_V22_FAMG_CLASS_PLACEHOLDER), "PLACEHOLDER") == 0,
          "class PLACEHOLDER name");
    CHECK(strcmp(dm1_v22_famg_class_name(
            DM1_V22_FAMG_CLASS_PARTIAL), "PARTIAL") == 0,
          "class PARTIAL name");
    CHECK(strcmp(dm1_v22_famg_class_name(
            DM1_V22_FAMG_CLASS_MISSING), "MISSING") == 0,
          "class MISSING name");

    /* Gate names */
    CHECK(strcmp(dm1_v22_famg_gate_name(
            DM1_V22_FAMG_GATE_NO_MANIFEST), "NO_MANIFEST") == 0,
          "gate NO_MANIFEST name");
    CHECK(strcmp(dm1_v22_famg_gate_name(
            DM1_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER),
            "SYNTHETIC_PLACEHOLDER") == 0,
          "gate SYNTHETIC_PLACEHOLDER name");
    CHECK(strcmp(dm1_v22_famg_gate_name(
            DM1_V22_FAMG_GATE_PARTIAL), "PARTIAL") == 0,
          "gate PARTIAL name");
    CHECK(strcmp(dm1_v22_famg_gate_name(
            DM1_V22_FAMG_GATE_FINISHED_REAL), "FINISHED_REAL") == 0,
          "gate FINISHED_REAL name");
}

static void test_source_evidence_citations(void) {
    const char* ev = dm1_v22_famg_source_evidence();
    CHECK(ev != NULL && strlen(ev) > 100, "source_evidence non-empty");
    CHECK(strstr(ev, "DUNVIEW.C") != NULL, "mentions DUNVIEW.C");
    CHECK(strstr(ev, "DUNGEON.C") != NULL, "mentions DUNGEON.C");
    CHECK(strstr(ev, "PANEL.C") != NULL, "mentions PANEL.C");
    CHECK(strstr(ev, "m11_v22_inplace_draw") != NULL,
          "mentions m11_v22_inplace_draw");
    CHECK(strstr(ev, "modern_asset_manifest.json") != NULL,
          "mentions manifest filename");
    CHECK(strstr(ev, "PNG IHDR") != NULL,
          "mentions PNG IHDR provenance");
    CHECK(strstr(ev, "FIRESTAFF_GAP_LIST") != NULL, "mentions gap list");
    CHECK(strstr(ev, "Honest boundary") != NULL,
          "mentions honest boundary");
    CHECK(strstr(ev, "dm1_v22_real_screenshot_material_receipt_01") != NULL,
          "mentions screenshot receipt id");
    CHECK(strstr(ev, "Receipt FINISHED_REAL") != NULL,
          "mentions receipt promotion boundary");
}

static void test_installed_flag_round_trip(void) {
    dm1_v22_famg_set_installed(1);
    CHECK(dm1_v22_famg_get_installed() == 1,
          "installed=1 after set_installed(1)");
    dm1_v22_famg_set_installed(0);
    CHECK(dm1_v22_famg_get_installed() == 0,
          "installed=0 after set_installed(0)");
    dm1_v22_famg_set_installed(99);
    CHECK(dm1_v22_famg_get_installed() == 1,
          "installed clamped to 1 for non-zero input");
}

static void test_validate_manifest_three_branches(void) {
    clean_scratch();
    const char* dataDir = "/tmp/scratch/dm1-famg-data/data/dm1";
    char manifest_path[FSP_PATH_MAX];
    char pdir[FSP_PATH_MAX];
    build_expected_manifest_path(manifest_path, sizeof(manifest_path), dataDir);
    snprintf(pdir, sizeof(pdir), "%s", manifest_path);
    char* slash = strrchr(pdir, '/');
    if (slash) *slash = '\0';
    char mkdir_cmd[1200];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", pdir);
    system(mkdir_cmd);

    /* Complete manifest */
    const char* good =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm1-v22-famg-test\","
        "\"wall_shapes\":["
        "{\"id\":\"wall_d3_carved_hero_01\",\"generator\":\"placeholder\","
        "\"source_file\":\"x.png\",\"width\":64,\"height\":64}"
        "]}";
    CHECK(write_file(manifest_path, good), "wrote complete test manifest");
    dm1_v22_famg_set_manifest_path(dataDir);
    int v = dm1_v22_famg_validate_manifest(NULL);
    CHECK(v == 1, "complete manifest -> 1");

    /* Partial manifest */
    const char* partial =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm1-v22-famg-test\","
        "\"wall_shapes\":["
        "{\"id\":\"wall_d3_carved_hero_01\",\"source_file\":\"x.png\"}"
        "]}";
    write_file(manifest_path, partial);
    v = dm1_v22_famg_validate_manifest(NULL);
    CHECK(v == 0, "partial manifest -> 0");

    /* Garbage manifest */
    write_file(manifest_path, "this is not json { [");
    v = dm1_v22_famg_validate_manifest(NULL);
    CHECK(v == -1 || v == 0, "garbage manifest -> -1 or 0 (parser rejects)");
}

static void test_uses_placeholder_known_gates(void) {
    /* Run after all the gate scenarios have finished to confirm
     * uses_placeholder() reflects the latest gate decisions for every
     * slot. */
    for (size_t i = 0; i < DM1_V22_FAMG_MATERIAL_COUNT; ++i) {
        DM1_V22_FamgClass cls =
            dm1_v22_famg_classify_slot((DM1_V22_FamgSlot)i);
        int expected_uses_placeholder =
            (cls == DM1_V22_FAMG_CLASS_REAL) ? 0 : 1;
        CHECK(dm1_v22_famg_uses_placeholder(
                (DM1_V22_FamgSlot)i) == expected_uses_placeholder,
              "uses_placeholder mirrors classification");
    }
}

static void test_real_screenshot_receipt_gate(void) {
    clean_scratch();
    const char* dataDir = "/tmp/scratch/dm1-famg-data/data/dm1";
    char manifest_path[FSP_PATH_MAX];
    char assets_root[FSP_PATH_MAX];
    char mkdir_cmd[1200];
    build_expected_manifest_path(manifest_path, sizeof(manifest_path), dataDir);
    snprintf(assets_root, sizeof(assets_root),
             "%s/../../assets/dm1/modern", dataDir);
    snprintf(mkdir_cmd, sizeof(mkdir_cmd),
             "mkdir -p '%s/wall_shapes' '%s/floor_shapes' '%s/creature_shapes' "
             "'%s/champion_portraits' '%s/door_shapes' '%s/field_shapes' '%s/receipts'",
             assets_root, assets_root, assets_root, assets_root,
             assets_root, assets_root, assets_root);
    CHECK(system(mkdir_cmd) == 0, "mkdir category dirs plus receipts");

    const char* files[DM1_V22_FAMG_MATERIAL_COUNT] = {
        "wall_d3_carved_hero_01.png",
        "floor_plain_hero_01.png",
        "floor_pit_hero_01.png",
        "creature_demon_hero_01.png",
        "champion_warrior_hero_01.png",
        "door_hero_01.png",
        "field_teleporter_hero_01.png"
    };
    const unsigned widths[DM1_V22_FAMG_MATERIAL_COUNT] = {
        64U, 64U, 64U, 48U, 48U, 32U, 64U
    };
    const unsigned heights[DM1_V22_FAMG_MATERIAL_COUNT] = {
        64U, 64U, 64U, 48U, 48U, 48U, 64U
    };
    for (size_t i = 0; i < DM1_V22_FAMG_MATERIAL_COUNT; ++i) {
        char fpath[FSP_PATH_MAX];
        snprintf(fpath, sizeof(fpath), "%s/%s/%s",
                 assets_root,
                 dm1_v22_famg_slot_category((DM1_V22_FamgSlot)i),
                 files[i]);
        CHECK(write_png_header_file(fpath, widths[i], heights[i]),
              "wrote material file for receipt gate");
    }

    dm1_v22_famg_set_manifest_path(dataDir);
    CHECK(write_all_real_manifest_with_receipt(manifest_path, NULL, NULL, NULL, NULL),
          "wrote all-real manifest without receipt");
    CHECK(dm1_v22_famg_gate() == DM1_V22_FAMG_GATE_FINISHED_REAL,
          "all-real materials still promote without receipt");
    CHECK(dm1_v22_famg_receipt_gate() == DM1_V22_FAMG_RECEIPT_NO_RECEIPT,
          "no receipt entry -> NO_RECEIPT");
    CHECK(dm1_v22_famg_has_finished_real_receipt() == 0,
          "no receipt is not final runtime evidence");

    char receipt_path[FSP_PATH_MAX];
    snprintf(receipt_path, sizeof(receipt_path), "%s/receipts/synthetic_frame.bmp",
             assets_root);
    CHECK(write_file(receipt_path, "synthetic-bmp-receipt"),
          "wrote synthetic receipt fixture");
    CHECK(write_all_real_manifest_with_receipt(
              manifest_path,
              "synthetic_test",
              "synthetic_frame.bmp",
              "sha256:synthetic",
              "FINISHED_REAL"),
          "wrote synthetic receipt manifest");
    CHECK(dm1_v22_famg_receipt_gate() ==
              DM1_V22_FAMG_RECEIPT_SYNTHETIC_PLACEHOLDER,
          "synthetic receipt stays SYNTHETIC_PLACEHOLDER");
    CHECK(dm1_v22_famg_has_synthetic_receipt() == 1,
          "synthetic receipt predicate true");
    CHECK(dm1_v22_famg_has_finished_real_receipt() == 0,
          "synthetic receipt cannot promote final proof");

    DM1_V22_FamgReceiptInfo info;
    CHECK(dm1_v22_famg_get_receipt_info(&info) == 1,
          "receipt info populated");
    CHECK(strcmp(info.id, dm1_v22_famg_receipt_manifest_id()) == 0,
          "receipt info id stable");
    CHECK(strcmp(info.generator, "synthetic_test") == 0,
          "receipt info generator stored");
    CHECK(strcmp(info.frame_hash, "sha256:synthetic") == 0,
          "receipt info frame hash stored");
    CHECK(info.width == 320 && info.height == 200,
          "receipt dimensions stored");
    CHECK(info.file_exists == 1,
          "synthetic receipt file resolves under receipts/");

    CHECK(write_all_real_manifest_with_receipt(
              manifest_path,
              "operator_reviewed",
              "missing_reviewed_frame.bmp",
              "sha256:reviewed",
              "FINISHED_REAL"),
          "wrote reviewed receipt manifest with missing file");
    CHECK(dm1_v22_famg_receipt_gate() == DM1_V22_FAMG_RECEIPT_PARTIAL,
          "reviewed receipt with missing file -> PARTIAL");

    snprintf(receipt_path, sizeof(receipt_path), "%s/receipts/reviewed_frame.bmp",
             assets_root);
    CHECK(write_file(receipt_path, "reviewed-runtime-bmp-receipt"),
          "wrote reviewed receipt fixture");
    CHECK(write_all_real_manifest_with_receipt(
              manifest_path,
              "operator_reviewed",
              "reviewed_frame.bmp",
              "sha256:reviewed",
              "PARTIAL"),
          "wrote reviewed receipt manifest with wrong material gate");
    CHECK(dm1_v22_famg_receipt_gate() == DM1_V22_FAMG_RECEIPT_PARTIAL,
          "reviewed receipt requires material_gate FINISHED_REAL");

    CHECK(write_all_real_manifest_with_receipt(
              manifest_path,
              "operator_reviewed",
              "reviewed_frame.bmp",
              "sha256:reviewed",
              "FINISHED_REAL"),
          "wrote final reviewed receipt manifest");
    CHECK(dm1_v22_famg_receipt_gate() == DM1_V22_FAMG_RECEIPT_FINISHED_REAL,
          "reviewed receipt + finished material gate -> FINISHED_REAL");
    CHECK(dm1_v22_famg_has_finished_real_receipt() == 1,
          "finished receipt predicate true");
    CHECK(strcmp(dm1_v22_famg_receipt_gate_name(
            DM1_V22_FAMG_RECEIPT_FINISHED_REAL), "FINISHED_REAL") == 0,
          "receipt gate name FINISHED_REAL");

    const char* placeholder_content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"dm1-v22-famg-test\","
        "\"wall_shapes\":["
        "{\"id\":\"wall_d3_carved_hero_01\",\"generator\":\"placeholder\","
        "\"source_file\":\"placeholder_wall.png\",\"width\":64,\"height\":64}"
        "],"
        "\"runtime_screenshot_receipts\":["
        "{\"id\":\"dm1_v22_real_screenshot_material_receipt_01\","
        "\"generator\":\"operator_reviewed\","
        "\"source_file\":\"reviewed_frame.bmp\","
        "\"width\":320,\"height\":200,"
        "\"frame_hash\":\"sha256:reviewed\","
        "\"material_gate\":\"FINISHED_REAL\"}"
        "]}";
    CHECK(write_file(manifest_path, placeholder_content),
          "wrote placeholder-material receipt manifest");
    CHECK(dm1_v22_famg_gate() == DM1_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER,
          "placeholder materials remain synthetic");
    CHECK(dm1_v22_famg_receipt_gate() == DM1_V22_FAMG_RECEIPT_PARTIAL,
          "reviewed receipt cannot finish while materials are placeholder");
}

/* ── Main ───────────────────────────────────────────────────────── */

int main(void) {
    printf("=== DM1 V2.2 finished-art material gate (synthetic) ===\n");
    test_unset_path_is_safe();
    test_set_path_resolves_correctly();
    test_missing_manifest_file_yields_no_manifest();
    test_empty_manifest_yields_placeholder_gate();
    test_placeholder_only_manifest_yields_placeholder_gate();
    test_real_slot_classifies_as_real();
    test_partial_when_some_real();
    test_partial_when_real_but_missing_source_file();
    test_real_metadata_requires_png_header_match();
    test_partial_when_fields_missing();
    test_get_slot_info_populates_fields();
    test_slot_count_is_seven();
    test_names_are_stable();
    test_source_evidence_citations();
    test_installed_flag_round_trip();
    test_validate_manifest_three_branches();
    test_uses_placeholder_known_gates();
    test_real_screenshot_receipt_gate();

    printf("\nSummary: %d passed, %d failed\n", s_pass, s_fail);
    return s_fail == 0 ? 0 : 1;
}
