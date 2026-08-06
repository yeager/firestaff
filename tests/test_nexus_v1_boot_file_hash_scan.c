#include "nexus_v1_engine.h"
#include "nexus_v1_viewport.h"
#include "firestaff_nexus_v1_boot_profile.h"
#include "fs_portable_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

static int write_file_bytes(const char* dst,
                            const unsigned char* data,
                            size_t size) {
    FILE* out = fopen(dst, "wb");
    if (!out) return 0;
    if (size > 0U && fwrite(data, 1U, size, out) != size) {
        fclose(out);
        return 0;
    }
    return fclose(out) == 0;
}

static int make_root(char* out, size_t outBytes) {
    int rc = snprintf(out,
                      outBytes,
                      "%s%sfirestaff-nexus-boot-hash-%ld-%lu",
                      getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp",
                      TEST_SEP,
                      (long)TEST_GETPID(),
                      (unsigned long)clock());
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

static int write_iso_payload_fixture(const char* dst,
                                     const unsigned char* mns,
                                     size_t mns_size,
                                     const unsigned char* fallback_dmdf,
                                     size_t fallback_dmdf_size,
                                     const unsigned char* exact_dmdf,
                                     size_t exact_dmdf_size) {
    unsigned char sector[NEXUS_ISO_DATA_SIZE];
    FILE* out = fopen(dst, "wb");
    if (!out) return 0;
    memset(sector, 0, sizeof(sector));
    if (mns_size > sizeof(sector)) {
        fclose(out);
        return 0;
    }
    memcpy(sector, mns, mns_size);
    if (fwrite(sector, 1U, sizeof(sector), out) != sizeof(sector)) {
        fclose(out);
        return 0;
    }
    memset(sector, 0, sizeof(sector));
    if (fallback_dmdf_size > sizeof(sector)) {
        fclose(out);
        return 0;
    }
    memcpy(sector, fallback_dmdf, fallback_dmdf_size);
    if (fwrite(sector, 1U, sizeof(sector), out) != sizeof(sector)) {
        fclose(out);
        return 0;
    }
    memset(sector, 0, sizeof(sector));
    if (exact_dmdf_size > sizeof(sector)) {
        fclose(out);
        return 0;
    }
    memcpy(sector, exact_dmdf, exact_dmdf_size);
    if (fwrite(sector, 1U, sizeof(sector), out) != sizeof(sector)) {
        fclose(out);
        return 0;
    }
    return fclose(out) == 0;
}

int main(void) {
    const char* data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    const char* home = getenv("HOME");
    char data_root[FSP_PATH_MAX];
    char root[FSP_PATH_MAX];
    char src[FSP_PATH_MAX];
    char dst[FSP_PATH_MAX];
    char dm_bin_src[FSP_PATH_MAX];
    char dm_bin_dst[FSP_PATH_MAX];
    char wrong_dm_bin_dst[FSP_PATH_MAX];
    char menu_bpk_src[FSP_PATH_MAX];
    char menu_bpk_dst[FSP_PATH_MAX];
    char profile_root[FSP_PATH_MAX];
    char profile_nexus_dir[FSP_PATH_MAX];
    char profile_dm_bin_dst[FSP_PATH_MAX];
    char level_src[FSP_PATH_MAX];
    char level_dst[FSP_PATH_MAX];
    char wrong_level_canonical_dst[FSP_PATH_MAX];
    char negative_root[FSP_PATH_MAX];
    char wrong_level_dst[FSP_PATH_MAX];
    char slev00_src[FSP_PATH_MAX];
    char slev00_dst[FSP_PATH_MAX];
    char sal00_src[FSP_PATH_MAX];
    char sal00_dst[FSP_PATH_MAX];
    char map00_src[FSP_PATH_MAX];
    char map00_dst[FSP_PATH_MAX];

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
    Nexus_V1_MenuBpkPrs3ExecutionEvidenceReceipt prs3_execution;
    Nexus_V1_DgnRendererHandoffReceipt dgn_handoff;
    Nexus_V1_DgnStructure2SourceReceipt structure2_source;
    Nexus_ScriptRuntimeReceipt script_receipt;
    Nexus_SfxRuntimeReceipt sfx_receipt;
    uint8_t* data;
    int size = 0;
    int menu_bpk_copied = 0;
    int slev00_copied = 0;
    int sndlev00_copied = 0;

    if (data_dir && data_dir[0]) {
        if (snprintf(data_root, sizeof(data_root), "%s", data_dir) >=
            (int)sizeof(data_root)) {
            puts("SKIP: configured Nexus data root is too long");
            return 0;
        }
    } else if (home && home[0] &&
               FSP_JoinPath(data_root, sizeof(data_root), home,
                            ".firestaff/data/nexus")) {
        /* Preserve the historical default when no explicit data root is set. */
    } else {
        puts("SKIP: Nexus data root is unset");
        return 0;
    }
    if (!FSP_JoinPath(src, sizeof(src), data_root, "TITLE.CG") ||
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

    {
        unsigned char dmdf_model[32];
        unsigned char dmdf_material[36];
        char mns_dir[FSP_PATH_MAX];
        char mns_dst[FSP_PATH_MAX];
        char dmdf_dst[FSP_PATH_MAX];
        char exact_dmdf_dst[FSP_PATH_MAX];
        memset(dmdf_model, 0, sizeof(dmdf_model));
        memset(dmdf_material, 0, sizeof(dmdf_material));
        dmdf_model[0] = 'D';
        dmdf_model[1] = 'M';
        dmdf_model[2] = 'D';
        dmdf_model[3] = 'F';
        dmdf_material[0] = 'D';
        dmdf_material[1] = 'M';
        dmdf_material[2] = 'D';
        dmdf_material[3] = 'F';
        check_int(FSP_JoinPath(mns_dir, sizeof(mns_dir), root, "models") &&
                      FSP_CreateDirectoryRecursive(mns_dir) &&
                      FSP_JoinPath(mns_dst,
                                   sizeof(mns_dst),
                                   mns_dir,
                                   "renamed-creature-family.payload.mns") &&
                      write_file_bytes(mns_dst,
                                       dmdf_model,
                                       sizeof(dmdf_model)),
                  "synthetic renamed MNS DMDF fixture written");
        memset(&engine, 0, sizeof(engine));
        engine.source = NEXUS_SRC_EXTRACTED;
        snprintf(engine.data_dir, sizeof(engine.data_dir), "%s", root);
        check_int(nexus_v1_load_model(&engine, "SCORPION.MNS") == -1 &&
                      engine.model_count == 0,
                  "Nexus model loader rejects renamed/synthetic MNS DMDF bytes");

        check_int(FSP_JoinPath(dmdf_dst,
                               sizeof(dmdf_dst),
                               root,
                               "renamed-material-family.payload.dmdf") &&
                      write_file_bytes(dmdf_dst,
                                       dmdf_model,
                                       sizeof(dmdf_model)),
                  "synthetic renamed DMDF material fixture written");
        data = nexus_v1_read_file(&engine, "FLOORS.DMDF", &size);
        check_int(data != NULL && size == (int)sizeof(dmdf_model),
                  "Nexus material loader resolves renamed DMDF by signature fallback");
        free(data);

        check_int(FSP_JoinPath(exact_dmdf_dst,
                               sizeof(exact_dmdf_dst),
                               root,
                               "WALLS.DMDF") &&
                      write_file_bytes(exact_dmdf_dst,
                                       dmdf_material,
                                       sizeof(dmdf_material)),
                  "synthetic exact DMDF material fixture written");
        data = nexus_v1_read_file(&engine, "WALLS.DMDF", &size);
        check_int(data != NULL && size == (int)sizeof(dmdf_material),
                  "Nexus material loader keeps exact DMDF filename precedence");
        free(data);

        {
            char iso_payload[FSP_PATH_MAX];
            check_int(FSP_JoinPath(iso_payload,
                                   sizeof(iso_payload),
                                   root,
                                   "synthetic-dmdf-family.iso-payload") &&
                          write_iso_payload_fixture(iso_payload,
                                                    dmdf_model,
                                                    sizeof(dmdf_model),
                                                    dmdf_model,
                                                    sizeof(dmdf_model),
                                                    dmdf_material,
                                                    sizeof(dmdf_material)),
                      "synthetic ISO DMDF-family payload fixture written");
            memset(&engine, 0, sizeof(engine));
            engine.source = NEXUS_SRC_ISO;
            engine.iso.fp = fopen(iso_payload, "rb");
            check_int(engine.iso.fp != NULL,
                      "synthetic ISO DMDF-family payload opened");
            engine.iso.valid = 1;
            engine.iso.sector_size = NEXUS_ISO_DATA_SIZE;
            engine.iso.data_offset = 0;
            engine.iso.file_count = 3;
            snprintf(engine.iso.files[0].name,
                     sizeof(engine.iso.files[0].name),
                     "%s",
                     "RENAMED_CREATURE.MNS");
            engine.iso.files[0].lba = 0U;
            engine.iso.files[0].size = (uint32_t)sizeof(dmdf_model);
            snprintf(engine.iso.files[1].name,
                     sizeof(engine.iso.files[1].name),
                     "%s",
                     "RENAMED_FLOORS.DMDF");
            engine.iso.files[1].lba = 1U;
            engine.iso.files[1].size = (uint32_t)sizeof(dmdf_model);
            snprintf(engine.iso.files[2].name,
                     sizeof(engine.iso.files[2].name),
                     "%s",
                     "WALLS.DMDF");
            engine.iso.files[2].lba = 2U;
            engine.iso.files[2].size = (uint32_t)sizeof(dmdf_material);

            check_int(nexus_v1_load_model(&engine, "SCORPION.MNS") == -1 &&
                          engine.model_count == 0,
                      "Nexus ISO model loader rejects renamed/synthetic MNS DMDF bytes");
            data = nexus_v1_read_file(&engine, "FLOORS.DMDF", &size);
            check_int(data != NULL && size == (int)sizeof(dmdf_model),
                      "Nexus ISO material loader resolves renamed DMDF by signature fallback");
            free(data);
            data = nexus_v1_read_file(&engine, "WALLS.DMDF", &size);
            check_int(data != NULL && size == (int)sizeof(dmdf_material),
                      "Nexus ISO material loader keeps exact DMDF filename precedence");
            free(data);
            nexus_iso_close(&engine.iso);
        }
    }

    if (FSP_JoinPath(dm_bin_src, sizeof(dm_bin_src), data_root, "DM.BIN") &&
        local_file_exists(dm_bin_src) &&
        FSP_JoinPath(dm_bin_dst, sizeof(dm_bin_dst), root, "renamed-saturn-data.payload") &&
        copy_file_bytes(dm_bin_src, dm_bin_dst) &&
        FSP_JoinPath(level_src, sizeof(level_src), data_root, "LEV00.DGN") &&
        local_file_exists(level_src) &&
        FSP_JoinPath(level_dst, sizeof(level_dst), root,
                     "renamed-level-zero.payload") &&
        copy_file_bytes(level_src, level_dst) &&
        FSP_JoinPath(wrong_level_canonical_dst,
                     sizeof(wrong_level_canonical_dst),
                     root,
                     "LEV00.DGN") &&
        copy_file_bytes(src, wrong_level_canonical_dst) &&
        FSP_JoinPath(wrong_dm_bin_dst, sizeof(wrong_dm_bin_dst), root,
                     "DM.BIN") &&
        copy_file_bytes(src, wrong_dm_bin_dst) &&
        FSP_JoinPath(negative_root, sizeof(negative_root), root,
                     "negative-level-name-only") &&
        FSP_CreateDirectoryRecursive(negative_root) &&
        FSP_JoinPath(wrong_level_dst, sizeof(wrong_level_dst), negative_root, "LEV00.DGN") &&
        copy_file_bytes(dm_bin_src, wrong_level_dst)) {
        if (FSP_JoinPath(menu_bpk_src, sizeof(menu_bpk_src), data_root, "MENU.BPK") &&
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
        check_int(nexus_v1_champion_panel_geometry_ready(&engine) == 1,
                  "hash-verified renamed DM.BIN outranks wrong canonical filename");
        check_int(engine.floor_bpk_container.exact_name_observed == 0 &&
                      engine.wall_bpk_container.exact_name_observed == 0 &&
                      engine.floor_bpk_container.source_present == 0 &&
                      engine.wall_bpk_container.source_present == 0 &&
                      engine.floor_bpk_container.identity_verified == 0 &&
                      engine.wall_bpk_container.identity_verified == 0 &&
                      engine.floor_bpk_container.host_route_permitted == 0 &&
                      engine.wall_bpk_container.host_route_permitted == 0,
                  "renamed original MENU.BPK is never observed as DGN material");
        if (menu_bpk_copied) {
            memset(&receipt, 0, sizeof(receipt));
            check_int(nexus_v1_menu_bpk_decode_receipt_ready(&engine) == 1,
                      "Nexus init records MENU.BPK decode receipt");
            memset(&prs3_execution, 0, sizeof(prs3_execution));
            check_int(nexus_v1_menu_bpk_prs3_execution_evidence_receipt(
                          &engine, &prs3_execution) == 0 &&
                          prs3_execution.valid &&
                          prs3_execution.menu_bpk_source_hash_verified &&
                          prs3_execution.dm_bin_source.canonical_hash_verified &&
                          prs3_execution.cross_asset_framing_verified &&
                          prs3_execution.sh2_loader_route_verified &&
                          prs3_execution.cross_asset.outer_v1_framing_matches &&
                          prs3_execution.sh2_loader.sh2_control_path_verified &&
                          prs3_execution.sh2_loader.sh2_stream_read_verified &&
                          prs3_execution.sh2_loader.sh2_output_store_verified &&
                          !prs3_execution.decoder_promoted &&
                          !prs3_execution.runtime_decode_permitted &&
                          !prs3_execution.fallback_visuals_permitted,
                      "Nexus boot binds real MENU.BPK framing to the verified DM.BIN loader without decoding");
            check_int(nexus_v1_menu_bpk_decode_receipt(&engine, &receipt) == 0,
                      "Nexus engine exposes MENU.BPK decode receipt");
            check_int(receipt.route == NEXUS_V1_BPK_DECODE_ROUTE_READY_DECODED,
                      "Nexus MENU.BPK runtime route accepts verified PRS3 surfaces");
            check_int(receipt.blocked_prs3_surfaces == 162U,
                      "Nexus MENU.BPK receipt preserves PRS3 source surface count");
            check_int(receipt.prs3_decode_successes == 162U &&
                          receipt.prs3_decode_failures == 0U &&
                          receipt.decode_blocked == 0 &&
                          receipt.prs3_decoder_promoted == 1 &&
                          receipt.prs3_decoded_pixels_emitted > 0U,
                      "Nexus MENU.BPK receipt records bounded PRS3 decode success");
            memset(&upload_receipt, 0, sizeof(upload_receipt));
            memset(upload_rows, 0, sizeof(upload_rows));
            check_int(nexus_v1_menu_bpk_upload_plan_receipt(
                          &engine,
                          &upload_receipt) == 0,
                      "Nexus engine exposes MENU.BPK upload plan receipt");
            check_int(upload_receipt.route ==
                          NEXUS_V1_BPK_UPLOAD_ROUTE_READY_DECODED,
                      "Nexus MENU.BPK upload plan accepts decoded PRS3 surfaces");
            check_int(upload_receipt.ready_uploads == 162U &&
                          upload_receipt.blocked_prs3_uploads == 0U &&
                          upload_receipt.planned_rows == 162U,
                      "Nexus MENU.BPK upload plan records every decoded surface");
            check_int(upload_receipt.fallback_visuals_permitted == 0,
                      "Nexus MENU.BPK upload plan forbids unsupported rendering");
            check_int(nexus_v1_menu_bpk_upload_plan_rows(
                          &engine,
                          upload_rows,
                          (int)(sizeof(upload_rows) /
                                sizeof(upload_rows[0]))) > 0,
                      "Nexus engine exposes bounded MENU.BPK upload rows");
            check_int(upload_rows[0].entry_index == 1U &&
                          upload_rows[0].upload_ready == 1 &&
                          upload_rows[0].decode_blocked == 0 &&
                          upload_rows[0].stream_offset > 0U,
                      "Nexus MENU.BPK upload row carries decoded PRS3 stream data");
            memset(&handoff, 0, sizeof(handoff));
            check_int(nexus_v1_menu_bpk_renderer_handoff_receipt(
                          &engine,
                          &handoff) == 0,
                      "Nexus engine emits MENU.BPK renderer handoff receipt");
            check_int(handoff.status ==
                          NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_READY_DECODED,
                      "Nexus MENU.BPK renderer handoff accepts decoded PRS3 surfaces");
            check_int(handoff.can_render_stored_surfaces == 1 &&
                          handoff.blocks_real_menu_surface_render == 0 &&
                          handoff.fallback_visuals_permitted == 0,
                      "Nexus MENU.BPK handoff exposes no unsupported surfaces");
            check_int(handoff.surface_entries == 162U &&
                          handoff.blocked_prs3_surfaces == 162U,
                      "Nexus MENU.BPK handoff retains PRS3 source provenance");
            check_int(strcmp(nexus_v1_menu_bpk_renderer_handoff_status_name(
                                 handoff.status),
                             "ready-decoded") == 0,
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
        puts("SKIP: local Nexus DM.BIN/LEV00.DGN not present for init hash test");
    }

    if (FSP_JoinPath(level_src, sizeof(level_src), data_root, "LEV00.DGN") &&
        local_file_exists(level_src) &&
        FSP_JoinPath(level_dst, sizeof(level_dst), root, "renamed-level-zero.payload") &&
        copy_file_bytes(level_src, level_dst)) {
        if (FSP_JoinPath(slev00_src, sizeof(slev00_src), data_root, "SLEV00.BIN") &&
            local_file_exists(slev00_src) &&
            FSP_JoinPath(slev00_dst, sizeof(slev00_dst), root, "renamed-script-level-zero.payload") &&
            copy_file_bytes(slev00_src, slev00_dst)) {
            slev00_copied = 1;
        }
        if (FSP_JoinPath(sal00_src, sizeof(sal00_src), data_root, "SNDLEV00.SAL") &&
            FSP_JoinPath(map00_src, sizeof(map00_src), data_root, "SNDLEV00.MAP") &&
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

        nexus_v1_game_init(&game, negative_root);
        check_int(nexus_v1_game_load_level(&game, 0) == -1,
                  "Nexus level loader rejects wrong bytes under canonical LEV00.DGN name");
        check_int(game.level_path[0] == '\0',
                  "rejected canonical LEV00.DGN does not enter game state");
        check_int(FSP_JoinPath(level_dst, sizeof(level_dst), root,
                               "renamed-level-zero.payload") &&
                      copy_file_bytes(level_src, level_dst),
                  "renamed authentic LEV00.DGN fixture written for hash test");
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
        memset(&structure2_source, 0, sizeof(structure2_source));
        check_int(nexus_v1_current_level_structure2_source_receipt(
                      &engine, &structure2_source) == 0,
                  "Nexus engine exposes Structure2 source identity receipt");
        check_int(strcmp(structure2_source.canonical_name, "LEV00.DGN") == 0 &&
                      strcmp(structure2_source.canonical_md5,
                             "603ec9c531a92539babdda84ab09e78e") == 0 &&
                      structure2_source.hash_discovery_attempted == 1 &&
                      structure2_source.canonical_hash_verified == 1 &&
                      structure2_source.materialization_bound ==
                          structure2_source.structure2_payload_envelope_valid &&
                      structure2_source.loaded_bytes_bound == 1 &&
                      structure2_source.loaded_dgn_size ==
                          engine.current_level_dgn_size &&
                      structure2_source.loaded_dgn_fnv1a64 != 0U &&
                      structure2_source.payload_decoder_permitted == 0 &&
                      structure2_source.fallback_visuals_permitted == 0,
                  "hash-resolved LEV00 remains the only authenticated Structure2 source without decoder promotion");
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
        {
            Nexus_Viewport viewport;
            Nexus_V1_DgnViewportHostRouteReceipt host_route;
            nexus_viewport_init(&viewport);
            nexus_viewport_render(&viewport, &engine);
            memset(&host_route, 0, sizeof(host_route));
            check_int(nexus_viewport_dgn_host_route_receipt(
                          &viewport,
                          &engine,
                          &host_route) == 0,
                      "Nexus real LEV00 viewport emits host-route receipt");
            check_int(host_route.package_consumed == 1 &&
                          host_route.handoff_status == dgn_handoff.status &&
                          host_route.blocks_runtime_dgn == 1 &&
                          host_route.can_present_runtime_dgn == 0 &&
                          host_route.rasterized_command_count == 0 &&
                          host_route.written_pixels == 0 &&
                          host_route.fallback_visuals_permitted == 0,
                      "Nexus real LEV00 host-route keeps an incomplete material route blocked without fallback pixels");
            check_int(host_route.status ==
                          NEXUS_V1_DGN_HOST_ROUTE_BLOCKED_HANDOFF &&
                          strcmp(nexus_viewport_dgn_host_route_status_name(
                                     host_route.status),
                                 "blocked-handoff") == 0,
                      "Nexus real LEV00 host-route blocks before DGN command copy without synthetic route");
        }
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
