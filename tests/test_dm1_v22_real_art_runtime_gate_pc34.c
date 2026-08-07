#include "dm1_v22_real_art_runtime_gate_pc34.h"
#include "dm1_v22_finished_art_material_gate_pc34.h"
#include "dm1_v22_finished_pack_receipt_pc34.h"
#include "dm1_v2_asset_pipeline_pc34.h"
#include "dm1_v2_boot_pc34.h"
#include "fs_portable_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
static int checks;

#define CHECK(expr, msg) do { \
    ++checks; \
    if (!(expr)) { ++failures; fprintf(stderr, "FAIL: %s\n", msg); } \
} while (0)

static int write_file(const char *path, const void *data, size_t size) {
    FILE *fp = fopen(path, "wb");
    if (!fp) return 0;
    if (fwrite(data, 1, size, fp) != size) { fclose(fp); return 0; }
    return fclose(fp) == 0;
}

static int write_png_header(const char *path, unsigned width, unsigned height) {
    unsigned char png[24] = {
        0x89, 'P', 'N', 'G', 0x0d, 0x0a, 0x1a, 0x0a,
        0, 0, 0, 13, 'I', 'H', 'D', 'R'
    };
    png[16] = (unsigned char)(width >> 24); png[17] = (unsigned char)(width >> 16);
    png[18] = (unsigned char)(width >> 8);  png[19] = (unsigned char)width;
    png[20] = (unsigned char)(height >> 24); png[21] = (unsigned char)(height >> 16);
    png[22] = (unsigned char)(height >> 8);  png[23] = (unsigned char)height;
    return write_file(path, png, sizeof(png));
}

static int write_real_pack(const char *modern_dir, int placeholder) {
    static const char *const ids[] = {
        "wall_d3_carved_hero_01", "floor_plain_hero_01", "floor_pit_hero_01",
        "creature_demon_hero_01", "champion_warrior_hero_01", "door_hero_01",
        "field_teleporter_hero_01"
    };
    static const char *const categories[] = {
        "wall_shapes", "floor_shapes", "floor_shapes", "creature_shapes",
        "champion_portraits", "door_shapes", "field_shapes"
    };
    char manifest[FSP_PATH_MAX];
    char png[FSP_PATH_MAX];
    char json[8192];
    size_t used;
    static const char json_format[] =
        "{\"wall_shapes\":[{\"id\":\"wall_d3_carved_hero_01\",\"generator\":\"%s\",\"source_file\":\"wall_d3_carved_hero_01.png\",\"width\":64,\"height\":64}],"
        "\"floor_shapes\":[{\"id\":\"floor_plain_hero_01\",\"generator\":\"original_graphics_dat_10x_palette_expansion\",\"source_file\":\"floor_plain_hero_01.png\",\"width\":64,\"height\":64},{\"id\":\"floor_pit_hero_01\",\"generator\":\"original_graphics_dat_10x_palette_expansion\",\"source_file\":\"floor_pit_hero_01.png\",\"width\":64,\"height\":64}],"
        "\"creature_shapes\":[{\"id\":\"creature_demon_hero_01\",\"generator\":\"original_graphics_dat_10x_palette_expansion\",\"source_file\":\"creature_demon_hero_01.png\",\"width\":48,\"height\":48}],"
        "\"champion_portraits\":[{\"id\":\"champion_warrior_hero_01\",\"generator\":\"original_graphics_dat_10x_palette_expansion\",\"source_file\":\"champion_warrior_hero_01.png\",\"width\":48,\"height\":48}],"
        "\"door_shapes\":[{\"id\":\"door_hero_01\",\"generator\":\"original_graphics_dat_10x_palette_expansion\",\"source_file\":\"door_hero_01.png\",\"width\":32,\"height\":48}],"
        "\"field_shapes\":[{\"id\":\"field_teleporter_hero_01\",\"generator\":\"original_graphics_dat_10x_palette_expansion\",\"source_file\":\"field_teleporter_hero_01.png\",\"width\":64,\"height\":64}]}";
    int i;

    for (i = 0; i < 7; ++i) {
        int width = i == 5 ? 32 : ((i == 3 || i == 4) ? 48 : 64);
        int height = i == 5 ? 48 : ((i == 3 || i == 4) ? 48 : 64);
        snprintf(png, sizeof(png), "%s/%s", modern_dir, categories[i]);
        if (!FSP_CreateDirectoryRecursive(png)) return 0;
        snprintf(png, sizeof(png), "%s/%s/%s.png", modern_dir,
                 categories[i], ids[i]);
        if (!write_png_header(png, (unsigned)width, (unsigned)height)) return 0;
    }
    {
        int written = snprintf(json, sizeof(json), json_format,
                               placeholder ? "placeholder" : "original_graphics_dat_10x_palette_expansion");
        if (written < 0 || (size_t)written >= sizeof(json)) return 0;
        used = (size_t)written;
    }
    snprintf(manifest, sizeof(manifest), "%s/modern_asset_manifest.json", modern_dir);
    return used < sizeof(json) && write_file(manifest, json, used);
}

static int write_receipt(const char *modern_dir) {
    static const char receipt_prefix[] =
        "{\"receiptVersion\":\"1.0.0\",\"manifestPath\":\"modern_asset_manifest.json\","
        "\"manifestHashFnv1a\":\"%08x\",\"reviewer\":\"test\","
        "\"reviewedAtUtc\":\"2026-07-14T00:00:00Z\",\"gateTarget\":\"FINISHED_REAL\","
        "\"reviewedSlots\":[\"wall_d3_carved_hero_01\",\"floor_plain_hero_01\","
        "\"floor_pit_hero_01\",\"creature_demon_hero_01\",\"champion_warrior_hero_01\","
        "\"door_hero_01\",\"field_teleporter_hero_01\"]}";
    char manifest[FSP_PATH_MAX];
    char receipt[FSP_PATH_MAX];
    char json[2048];
    uint32_t hash;
    snprintf(manifest, sizeof(manifest), "%s/modern_asset_manifest.json", modern_dir);
    snprintf(receipt, sizeof(receipt), "%s/finish_receipt.json", modern_dir);
    hash = dm1_v22_fpr_fnv1a_file(manifest);
    snprintf(json, sizeof(json), receipt_prefix, hash);
    return write_file(receipt, json, strlen(json));
}

int main(void) {
    const char *data_dir = "/tmp/firestaff-dm1-v22-runtime-gate/data/dm1";
    const char *modern_dir = "/tmp/firestaff-dm1-v22-runtime-gate/assets/dm1/modern";
    DM1_V22_RealArtRuntimeGate_PC34 gate;
    DM1_V2_BootStartupReceipt_PC34 boot_receipt;
    char path[FSP_PATH_MAX];

    CHECK(FSP_CreateDirectoryRecursive(data_dir), "created data root");
    CHECK(FSP_CreateDirectoryRecursive(modern_dir), "created modern root");
    snprintf(path, sizeof(path), "%s/modern_asset_manifest.json", modern_dir);
    (void)remove(path);
    snprintf(path, sizeof(path), "%s/finish_receipt.json", modern_dir);
    (void)remove(path);
    CHECK(!dm1_v22_real_art_runtime_gate_refresh_pc34(data_dir, &gate),
          "missing pack is not admitted");
    CHECK(gate.asset_root_configured && gate.asset_root_matches_receipt,
          "runtime and receipt resolve the same root");
    CHECK(!gate.material_finished_real && !gate.receipt_promoted,
          "missing pack has no real-art evidence");

    CHECK(write_real_pack(modern_dir, 0), "wrote real PNG-backed material pack");
    CHECK(!dm1_v22_real_art_runtime_gate_refresh_pc34(data_dir, &gate),
          "unreviewed real pack is not admitted");
    CHECK(dm1_v22_famg_gate() == DM1_V22_FAMG_GATE_FINISHED_REAL,
          "every PNG-backed material slot is real");
    {
        int slot;
        for (slot = 0; slot < DM1_V22_FAMG_MATERIAL_COUNT; ++slot) {
            CHECK(dm1_v22_famg_classify_slot((DM1_V22_FamgSlot)slot) ==
                      DM1_V22_FAMG_CLASS_REAL,
                  dm1_v22_famg_slot_name((DM1_V22_FamgSlot)slot));
        }
    }
    CHECK(gate.material_finished_real && !gate.receipt_promoted,
          "real art still requires receipt promotion");
    m11_v22_set_manifest_path(data_dir);
    m11_v22_set_installed(1);
    m11_v22_set_epx_cache_warm(1);
    CHECK(m11_v22_modern_assets_available() == 0,
          "critical categories alone do not admit DM1 V2.2");
    CHECK(m11_v22_best_available_shape_source(3) ==
              DM1_V22_SHAPE_SOURCE_V2_UPSCALED,
          "installed flag without receipt still falls back to V2.1");
    CHECK(dm1_v2_boot_startup_prepare_pc34("dm1", data_dir, 3, &boot_receipt),
          "boot accepts a DM1 launch request");
    CHECK(boot_receipt.resolved_mode == DM1_V2_PM_V21_UPSCALED &&
          !boot_receipt.v22_real_art_runtime_gate.admitted,
          "boot falls back to V2.1 before the review receipt arrives");
    CHECK(write_receipt(modern_dir), "wrote matching review receipt");
    CHECK(dm1_v22_real_art_runtime_gate_refresh_pc34(data_dir, &gate),
          "reviewed real pack is admitted");
    CHECK(gate.admitted && gate.asset_root_matches_receipt,
          "admission records root-bound receipt");
    {
        char shape_path[FSP_PATH_MAX];
        CHECK(m11_v22_modern_assets_available() == 1,
              "reviewed receipt admits DM1 V2.2");
        CHECK(m11_v22_best_available_shape_source(3) ==
                  DM1_V22_SHAPE_SOURCE_V2_MODERN,
              "reviewed pack resolves to V2.2 modern");
        CHECK(m11_v22_get_shape_path("wall_shapes",
                                     "wall_d3_carved_hero_01",
                                     shape_path,
                                     sizeof(shape_path)) == 1,
              "compact manifest resolves real wall asset");
        CHECK(strstr(shape_path, "wall_d3_carved_hero_01.png") != NULL,
              "resolved path points at the requested real PNG");
    }
    CHECK(dm1_v2_boot_startup_prepare_pc34("dm1", data_dir, 3, &boot_receipt),
          "boot refreshes the reviewed V2.2 receipt");
    CHECK(boot_receipt.resolved_mode == DM1_V2_PM_V22_MODERN &&
          boot_receipt.v22_real_art_runtime_gate.admitted,
          "boot admits V2.2 only with the root-bound reviewed pack");

    CHECK(write_real_pack(modern_dir, 1), "rewrote one slot as placeholder");
    CHECK(!dm1_v22_real_art_runtime_gate_refresh_pc34(data_dir, &gate),
          "placeholder regression is rejected");
    CHECK(!gate.material_finished_real && !gate.admitted,
          "no-placeholder contract remains fail-closed");
    CHECK(strstr(dm1_v22_real_art_runtime_gate_source_evidence_pc34(),
                 "placeholder art is never admitted") != NULL,
          "source evidence states the runtime boundary");

    printf("%d / %d checks passed\n", checks - failures, checks);
    return failures ? 1 : 0;
}
