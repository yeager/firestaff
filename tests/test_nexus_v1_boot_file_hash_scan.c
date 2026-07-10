#include "nexus_v1_engine.h"
#include "nexus_v1_viewport.h"
#include "firestaff_nexus_v1_boot_profile.h"
#include "fs_portable_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#include <process.h>
#define TEST_GETPID() _getpid()
#define TEST_SEP "\\"
#else
#include <unistd.h>
#define TEST_GETPID() getpid()
#define TEST_SEP "/"
#endif

static int failures;

static void check_int(int condition, const char* message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static int copy_file_bytes(const char* src, const char* dst) {
    unsigned char buf[8192];
    FILE* in = fopen(src, "rb");
    FILE* out;
    size_t n;
    if (!in) return 0;
    out = fopen(dst, "wb");
    if (!out) {
        fclose(in);
        return 0;
    }
    while ((n = fread(buf, 1U, sizeof(buf), in)) > 0U) {
        if (fwrite(buf, 1U, n, out) != n) {
            fclose(in);
            fclose(out);
            return 0;
        }
    }
    fclose(in);
    return fclose(out) == 0;
}

static int make_root(char* out, size_t outBytes) {
    int rc = snprintf(out,
                      outBytes,
                      "%s%sfirestaff-nexus-boot-hash-%ld",
                      getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp",
                      TEST_SEP,
                      (long)TEST_GETPID());
    return rc > 0 && (size_t)rc < outBytes && FSP_CreateDirectoryRecursive(out);
}

static int local_file_exists(const char* path) {
    FILE* fp = fopen(path, "rb");
    if (!fp) return 0;
    fclose(fp);
    return 1;
}

static int diag_details_contain(const Nexus_V1_Diagnostic* diags,
                                size_t count,
                                const char* needle) {
    size_t i;
    if (!diags || !needle) return 0;
    for (i = 0U; i < count; ++i) {
        if (strstr(diags[i].detail, needle) != NULL) {
            return 1;
        }
    }
    return 0;
}

static int palette_contains_color(const Nexus_Viewport* viewport,
                                  unsigned int rgba) {
    int i;
    if (!viewport) return 0;
    for (i = 0; i < 256; ++i) {
        if (viewport->fb.palette[i] == rgba) return 1;
    }
    return 0;
}

static void wb16(unsigned char* p, unsigned int value) {
    p[0] = (unsigned char)(value >> 8);
    p[1] = (unsigned char)value;
}

static void wb32(unsigned char* p, unsigned int value) {
    p[0] = (unsigned char)(value >> 24);
    p[1] = (unsigned char)(value >> 16);
    p[2] = (unsigned char)(value >> 8);
    p[3] = (unsigned char)value;
}

static void test_dgn_material_plan_cache(void) {
    static Nexus_V1_Engine engine;
    static Nexus_Viewport viewport;
    static unsigned char dgn[NEXUS_DGN_BLOCK_SIZE * 20];
    static unsigned char pixels[64];
    Nexus_V1_DgnMaterialPlan const *first;
    Nexus_V1_DgnMaterialPlan const *second;
    unsigned char* structure1;
    int i;

    memset(&engine, 0, sizeof(engine));
    memset(dgn, 0, sizeof(dgn));
    wb16(dgn + 0x0c, 1U);
    wb16(dgn + 0x0e, 19U);
    wb32(dgn + 0x10,
         (unsigned int)(0x40 + NEXUS_DGN_STRUCTURE1B_BYTES + 1024));
    structure1 = dgn + NEXUS_DGN_BLOCK_SIZE;
    structure1[1] = 0x50;
    structure1[2] = 0x40;
    structure1[3] = 0x40;
    wb32(structure1 + 0x10, 0x38U);
    wb32(structure1 + 0x14, 0x40U);
    wb32(structure1 + 0x18, 0x40U + NEXUS_DGN_STRUCTURE1B_BYTES);
    engine.level_loaded = 1;
    engine.game.current_level = 3;
    check_int(nexus_v1_level_load(&engine.current_level, dgn,
                                  (int)sizeof(dgn), 3) == 0,
              "Nexus DGN material cache fixture loads");
    engine.current_level.squares[4][3] = 1;
    engine.current_level.floor_material_refs[4][3] = 7;
    engine.current_level.wall_material_refs[4][3][0] = 9;
    engine.floor_materials.valid = 1;
    engine.wall_materials.valid = 1;
    for (i = 0; i < NEXUS_DMDF_MATERIAL_COUNT; ++i) {
        engine.floor_materials.surfaces[i].valid = 1;
        engine.floor_materials.surfaces[i].pixels = pixels;
        engine.floor_materials.surfaces[i].width = 8;
        engine.floor_materials.surfaces[i].height = 8;
        engine.floor_materials.surfaces[i].palette[13] = 0xff45ab67U;
        engine.wall_materials.surfaces[i].valid = 1;
        engine.wall_materials.surfaces[i].pixels = pixels;
        engine.wall_materials.surfaces[i].width = 8;
        engine.wall_materials.surfaces[i].height = 8;
        engine.wall_materials.surfaces[i].palette[13] = 0xff45ab67U;
    }

    first = nexus_v1_prepare_dgn_material_plan(&engine, 3, 4, 0);
    check_int(first != NULL && first->valid,
              "Nexus DGN material plan accepts decoded DMDF/BPK surfaces");
    second = nexus_v1_prepare_dgn_material_plan(&engine, 3, 4, 0);
    check_int(second == first && engine.dgn_material_plan.cache_hit_count == 1U,
              "Nexus DGN material plan is shared for an already loaded level");
    nexus_viewport_init(&viewport);
    engine.game.party_x = 3;
    engine.game.party_y = 4;
    engine.game.party_dir = 0;
    nexus_viewport_render(&viewport, &engine);
    check_int(viewport.material_engine == &engine &&
                  viewport.material_generation == engine.dgn_material_plan.generation &&
                  viewport.last_dgn_render_receipt.palette_synced &&
                  palette_contains_color(&viewport, 0xff45ab67U),
              "Nexus viewport caches the palette from the verified DGN material plan");
    nexus_v1_invalidate_dgn_material_plan(&engine);
    check_int(engine.dgn_material_plan.valid == 0,
              "Nexus DGN material plan invalidates before a new or resumed level");
    engine.floor_materials.surfaces[7].valid = 0;
    check_int(nexus_v1_prepare_dgn_material_plan(&engine, 3, 4, 0) == NULL,
              "Nexus DGN material plan rejects an undecoded PRS3-only surface");
}

int main(void) {
    const char* home = getenv("HOME");
    char root[FSP_PATH_MAX];
    char src[FSP_PATH_MAX];
    char dst[FSP_PATH_MAX];
    char dm_bin_src[FSP_PATH_MAX];
    char dm_bin_dst[FSP_PATH_MAX];
    char menu_bpk_src[FSP_PATH_MAX];
    char menu_bpk_dst[FSP_PATH_MAX];
    char profile_root[FSP_PATH_MAX];
    char profile_nexus_dir[FSP_PATH_MAX];
    char profile_dm_bin_dst[FSP_PATH_MAX];
    char level_src[FSP_PATH_MAX];
    char level_dst[FSP_PATH_MAX];
    char slev00_src[FSP_PATH_MAX];
    char slev00_dst[FSP_PATH_MAX];
    char sal00_src[FSP_PATH_MAX];
    char sal00_dst[FSP_PATH_MAX];
    char map00_src[FSP_PATH_MAX];
    char map00_dst[FSP_PATH_MAX];

    test_dgn_material_plan_cache();
    char profile_level_root[FSP_PATH_MAX];
    char profile_level_nexus_dir[FSP_PATH_MAX];
    char profile_level_dst[FSP_PATH_MAX];
    Nexus_V1_Engine engine;
    Nexus_V1_GameState game;
    Nexus_V1_BootProfile profile;
    Nexus_V1_Diagnostic diags[4];
    Nexus_V1_BpkRuntimeDecodeReceipt receipt;
    Nexus_V1_BpkRuntimeUploadReceipt upload_receipt;
    Nexus_V1_BpkRuntimeUploadRow upload_rows[4];
    Nexus_V1_MenuBpkRendererHandoffReceipt handoff;
    Nexus_V1_DgnRendererHandoffReceipt dgn_handoff;
    Nexus_ScriptRuntimeReceipt script_receipt;
    Nexus_SfxRuntimeReceipt sfx_receipt;
    uint8_t* data;
    int size = 0;
    int menu_bpk_copied = 0;
    int slev00_copied = 0;
    int sndlev00_copied = 0;

    if (!home || !home[0]) {
        puts("SKIP: HOME unset");
        return 0;
    }
    if (!FSP_JoinPath(src, sizeof(src), home, ".firestaff/data/nexus/TITLE.CG") ||
        !local_file_exists(src)) {
        puts("SKIP: local Nexus TITLE.CG not present");
        return 0;
    }
    check_int(make_root(root, sizeof(root)), "temp Nexus root created");
    check_int(FSP_JoinPath(dst, sizeof(dst), root, "renamed-title.payload"),
              "renamed path built");
    check_int(copy_file_bytes(src, dst), "TITLE.CG copied under arbitrary name");

    memset(&engine, 0, sizeof(engine));
    engine.source = NEXUS_SRC_EXTRACTED;
    snprintf(engine.data_dir, sizeof(engine.data_dir), "%s", root);

    data = nexus_v1_read_file(&engine, "TITLE.CG", &size);
    check_int(data != NULL, "Nexus TITLE.CG resolves by hash when renamed");
    check_int(size > 100000, "renamed Nexus TITLE.CG size is plausible");
    free(data);

    if (FSP_JoinPath(dm_bin_src, sizeof(dm_bin_src), home, ".firestaff/data/nexus/DM.BIN") &&
        local_file_exists(dm_bin_src) &&
        FSP_JoinPath(dm_bin_dst, sizeof(dm_bin_dst), root, "renamed-saturn-data.payload") &&
        copy_file_bytes(dm_bin_src, dm_bin_dst)) {
        if (FSP_JoinPath(menu_bpk_src, sizeof(menu_bpk_src), home, ".firestaff/data/nexus/MENU.BPK") &&
            local_file_exists(menu_bpk_src) &&
            FSP_JoinPath(menu_bpk_dst, sizeof(menu_bpk_dst), root, "renamed-menu-bpk.payload") &&
            copy_file_bytes(menu_bpk_src, menu_bpk_dst)) {
            menu_bpk_copied = 1;
        }

        memset(&engine, 0, sizeof(engine));
        check_int(nexus_v1_init(&engine, root) == 0,
                  "Nexus init accepts renamed DM.BIN marker by hash");
        check_int(engine.source == NEXUS_SRC_EXTRACTED,
                  "renamed DM.BIN selects extracted Nexus source");
        if (menu_bpk_copied) {
            memset(&receipt, 0, sizeof(receipt));
            check_int(nexus_v1_menu_bpk_decode_receipt_ready(&engine) == 1,
                      "Nexus init records MENU.BPK decode receipt");
            check_int(nexus_v1_menu_bpk_decode_receipt(&engine, &receipt) == 0,
                      "Nexus engine exposes MENU.BPK decode receipt");
            check_int(receipt.route == NEXUS_V1_BPK_DECODE_ROUTE_BLOCKED_PRS3,
                      "Nexus MENU.BPK runtime route blocks on PRS3 decoder");
            check_int(receipt.blocked_prs3_surfaces == 162U,
                      "Nexus MENU.BPK receipt preserves PRS3 surface blocker count");
            check_int(receipt.first_blocked_entry == 1U,
                      "Nexus MENU.BPK receipt exposes first blocked surface entry");
            memset(&upload_receipt, 0, sizeof(upload_receipt));
            memset(upload_rows, 0, sizeof(upload_rows));
            check_int(nexus_v1_menu_bpk_upload_plan_receipt(
                          &engine,
                          &upload_receipt) == 0,
                      "Nexus engine exposes MENU.BPK upload plan receipt");
            check_int(upload_receipt.route ==
                          NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_PRS3,
                      "Nexus MENU.BPK upload plan blocks on PRS3");
            check_int(upload_receipt.blocked_prs3_uploads == 162U &&
                          upload_receipt.planned_rows == 162U,
                      "Nexus MENU.BPK upload plan preserves blocker counts");
            check_int(upload_receipt.fallback_visuals_permitted == 0,
                      "Nexus MENU.BPK upload plan forbids fallback visuals");
            check_int(nexus_v1_menu_bpk_upload_plan_rows(
                          &engine,
                          upload_rows,
                          (int)(sizeof(upload_rows) /
                                sizeof(upload_rows[0]))) > 0,
                      "Nexus engine exposes bounded MENU.BPK upload rows");
            check_int(upload_rows[0].entry_index == 1U &&
                          upload_rows[0].decode_blocked == 1 &&
                          upload_rows[0].stream_offset > 0U,
                      "Nexus MENU.BPK upload row carries PRS3 stream blocker");
            memset(&handoff, 0, sizeof(handoff));
            check_int(nexus_v1_menu_bpk_renderer_handoff_receipt(
                          &engine,
                          &handoff) == 0,
                      "Nexus engine emits MENU.BPK renderer handoff receipt");
            check_int(handoff.status ==
                          NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_BLOCKED_PRS3,
                      "Nexus MENU.BPK renderer handoff blocks on PRS3");
            check_int(handoff.blocks_real_menu_surface_render == 1 &&
                          handoff.fallback_visuals_permitted == 0,
                      "Nexus MENU.BPK handoff forbids fallback when real PRS3 surfaces block");
            check_int(handoff.surface_entries == 162U &&
                          handoff.first_blocked_entry == 1U,
                      "Nexus MENU.BPK handoff exposes blocked surface counts to renderer");
            check_int(strcmp(nexus_v1_menu_bpk_renderer_handoff_status_name(
                                 handoff.status),
                             "blocked-prs3") == 0,
                      "Nexus MENU.BPK handoff status has stable route name");
        } else {
            puts("SKIP: local Nexus MENU.BPK not present for engine decode receipt");
        }
        nexus_v1_shutdown(&engine);

        check_int(FSP_JoinPath(profile_root, sizeof(profile_root), root, "profile-root") &&
                  FSP_JoinPath(profile_nexus_dir, sizeof(profile_nexus_dir), profile_root, "nexus") &&
                  FSP_CreateDirectoryRecursive(profile_nexus_dir) &&
                  FSP_JoinPath(profile_dm_bin_dst,
                               sizeof(profile_dm_bin_dst),
                               profile_nexus_dir,
                               "renamed-dm-bin.marker") &&
                  copy_file_bytes(dm_bin_src, profile_dm_bin_dst),
                  "renamed DM.BIN profile fixture written");
        memset(&profile, 0, sizeof(profile));
        memset(diags, 0, sizeof(diags));
        check_int(Nexus_V1_BootProfile_Init(&profile, profile_root, profile_root, 0U) == 0,
                  "Nexus boot profile initialized for renamed DM.BIN root");
        (void)Nexus_V1_BootProfile_ValidateAssets(&profile, diags, 4U);
        check_int(strstr(diags[0].detail, "DM.BIN") == NULL,
                  "Nexus boot profile accepts renamed DM.BIN by hash");
    } else {
        puts("SKIP: local Nexus DM.BIN not present for init hash test");
    }

    if (FSP_JoinPath(level_src, sizeof(level_src), home, ".firestaff/data/nexus/LEV00.DGN") &&
        local_file_exists(level_src) &&
        FSP_JoinPath(level_dst, sizeof(level_dst), root, "renamed-level-zero.payload") &&
        copy_file_bytes(level_src, level_dst)) {
        if (FSP_JoinPath(slev00_src, sizeof(slev00_src), home, ".firestaff/data/nexus/SLEV00.BIN") &&
            local_file_exists(slev00_src) &&
            FSP_JoinPath(slev00_dst, sizeof(slev00_dst), root, "renamed-script-level-zero.payload") &&
            copy_file_bytes(slev00_src, slev00_dst)) {
            slev00_copied = 1;
        }
        if (FSP_JoinPath(sal00_src, sizeof(sal00_src), home, ".firestaff/data/nexus/SNDLEV00.SAL") &&
            FSP_JoinPath(map00_src, sizeof(map00_src), home, ".firestaff/data/nexus/SNDLEV00.MAP") &&
            local_file_exists(sal00_src) &&
            local_file_exists(map00_src) &&
            FSP_JoinPath(sal00_dst, sizeof(sal00_dst), root, "renamed-sound-level-zero.sal-payload") &&
            FSP_JoinPath(map00_dst, sizeof(map00_dst), root, "renamed-sound-level-zero.map-payload") &&
            copy_file_bytes(sal00_src, sal00_dst) &&
            copy_file_bytes(map00_src, map00_dst)) {
            sndlev00_copied = 1;
        }

        memset(&engine, 0, sizeof(engine));
        check_int(nexus_v1_init(&engine, root) == 0,
                  "Nexus init accepts renamed LEV00.DGN marker by hash");
        check_int(engine.source == NEXUS_SRC_EXTRACTED,
                  "renamed LEV00.DGN selects extracted Nexus source");
        nexus_v1_shutdown(&engine);

        nexus_v1_game_init(&game, root);
        check_int(nexus_v1_game_load_level(&game, 0) == 0,
                  "Nexus level loader accepts renamed LEV00.DGN by hash");
        check_int(strstr(game.level_path, "renamed-level-zero.payload") != NULL,
                  "Nexus level loader stores hash-resolved renamed path");
        memset(&engine, 0, sizeof(engine));
        check_int(nexus_v1_init(&engine, root) == 0,
                  "Nexus init accepts renamed LEV00.DGN for runtime DGN handoff");
        check_int(nexus_v1_load_level(&engine, 0) == 0,
                  "Nexus runtime loads renamed LEV00.DGN by hash");
        memset(&dgn_handoff, 0, sizeof(dgn_handoff));
        check_int(nexus_v1_current_level_dgn_renderer_handoff_receipt(
                      &engine,
                      &dgn_handoff) == 0,
                  "Nexus engine emits current-level DGN renderer handoff");
        check_int(dgn_handoff.dmweb_container == 1 &&
                      dgn_handoff.width == 64 &&
                      dgn_handoff.height == 64,
                  "Nexus DGN handoff exposes real 64x64 DMWeb route");
        check_int(dgn_handoff.fallback_visuals_permitted == 0,
                  "Nexus DGN handoff forbids fallback visuals");
        memset(&script_receipt, 0, sizeof(script_receipt));
        check_int(nexus_v1_current_level_script_runtime_receipt(
                      &engine,
                      &script_receipt) == 0,
                  "Nexus engine emits current-level script runtime receipt");
        if (slev00_copied) {
            check_int(script_receipt.candidate_source_loaded == 1 &&
                          script_receipt.candidate_source_bytes > 0,
                      "Nexus script receipt sees hash-resolved renamed SLEV00 candidate bytes");
            check_int(script_receipt.status ==
                          NEXUS_SCRIPT_RUNTIME_BLOCKED_UNSUPPORTED_FORMAT,
                      "Nexus script receipt blocks unsupported real SLEV format");
            check_int(script_receipt.dispatch_enabled == 0 &&
                          script_receipt.fallback_visuals_permitted == 0,
                      "Nexus script receipt forbids fallback dispatch");
            check_int(strcmp(nexus_script_runtime_status_name(
                                 script_receipt.status),
                             "blocked-unsupported-format") == 0,
                      "Nexus script receipt has stable blocked route name");
        } else {
            puts("SKIP: local Nexus SLEV00.BIN not present for script runtime receipt");
        }
        memset(&sfx_receipt, 0, sizeof(sfx_receipt));
        check_int(nexus_v1_current_level_sfx_runtime_receipt(
                      &engine,
                      &sfx_receipt) == 0,
                  "Nexus engine emits current-level SFX runtime receipt");
        if (sndlev00_copied) {
            check_int(sfx_receipt.sal_loaded == 1 &&
                          sfx_receipt.map_loaded == 1,
                      "Nexus SFX receipt sees hash-resolved renamed SNDLEV00 SAL/MAP bytes");
            check_int(sfx_receipt.status ==
                          NEXUS_SFX_RUNTIME_BLOCKED_UNSUPPORTED_DECODE,
                      "Nexus SFX receipt blocks real SAL/MAP decode gap");
            check_int(sfx_receipt.sal_receipt.receipt_class ==
                          NEXUS_V1_AUDIO_RECEIPT_SIZE_MATCH &&
                          sfx_receipt.map_receipt.receipt_class ==
                              NEXUS_V1_AUDIO_RECEIPT_SIZE_MATCH,
                      "Nexus SFX receipt preserves SAL/MAP size receipts");
            check_int(sfx_receipt.cd_track == 2 &&
                          sfx_receipt.blocks_real_sfx_playback == 1 &&
                          sfx_receipt.fallback_visuals_permitted == 0,
                      "Nexus SFX receipt forbids fallback playback");
            check_int(strcmp(nexus_sound_sfx_runtime_status_name(
                                 sfx_receipt.status),
                             "blocked-unsupported-decode") == 0,
                      "Nexus SFX receipt has stable blocked route name");
        } else {
            puts("SKIP: local Nexus SNDLEV00.SAL/.MAP not present for SFX runtime receipt");
        }
        nexus_v1_shutdown(&engine);

        check_int(FSP_JoinPath(profile_level_root, sizeof(profile_level_root), root, "profile-level-root") &&
                  FSP_JoinPath(profile_level_nexus_dir, sizeof(profile_level_nexus_dir), profile_level_root, "nexus") &&
                  FSP_CreateDirectoryRecursive(profile_level_nexus_dir) &&
                  FSP_JoinPath(profile_level_dst,
                               sizeof(profile_level_dst),
                               profile_level_nexus_dir,
                               "renamed-level-zero.marker") &&
                  copy_file_bytes(level_src, profile_level_dst),
                  "renamed LEV00.DGN profile fixture written");
        memset(&profile, 0, sizeof(profile));
        memset(diags, 0, sizeof(diags));
        check_int(Nexus_V1_BootProfile_Init(&profile, profile_level_root, profile_level_root, 0U) == 0,
                  "Nexus boot profile initialized for renamed LEV00.DGN root");
        (void)Nexus_V1_BootProfile_ValidateAssets(&profile, diags, 4U);
        check_int(!diag_details_contain(diags, 4U, "LEV00.DGN"),
                  "Nexus boot profile accepts renamed LEV00.DGN by hash");
    } else {
        puts("SKIP: local Nexus LEV00.DGN not present for level hash test");
    }

    if (failures) return 1;
    puts("ok: Nexus boot file resolver finds renamed startup files by hash");
    return 0;
}
