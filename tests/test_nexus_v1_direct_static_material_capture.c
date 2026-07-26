#include "nexus_v1_engine.h"
#include "nexus_v1_launcher.h"
#include "nexus_v1_lev_corpus_discovery.h"
#include "asset_find_by_hash.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\n", message); \
        ++failures; \
    } \
} while (0)

static uint64_t fnv1a64(const uint8_t *data, size_t size)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t index;
    for (index = 0U; index < size; ++index) {
        hash ^= data[index];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static void write_be32(uint8_t *bytes, size_t offset, uint32_t value)
{
    bytes[offset] = (uint8_t)(value >> 24U);
    bytes[offset + 1U] = (uint8_t)(value >> 16U);
    bytes[offset + 2U] = (uint8_t)(value >> 8U);
    bytes[offset + 3U] = (uint8_t)value;
}

static void write_be64(uint8_t *bytes, size_t offset, uint64_t value)
{
    bytes[offset] = (uint8_t)(value >> 56U);
    bytes[offset + 1U] = (uint8_t)(value >> 48U);
    bytes[offset + 2U] = (uint8_t)(value >> 40U);
    bytes[offset + 3U] = (uint8_t)(value >> 32U);
    bytes[offset + 4U] = (uint8_t)(value >> 24U);
    bytes[offset + 5U] = (uint8_t)(value >> 16U);
    bytes[offset + 6U] = (uint8_t)(value >> 8U);
    bytes[offset + 7U] = (uint8_t)value;
}

static int count_canonical_transform_capture_targets(const char *data_dir,
                                                     int *out_static_count)
{
    int level_index;
    int target_count = 0;
    int static_target_count = 0;

    if (!out_static_count) return -1;
    *out_static_count = 0;
    for (level_index = 0; level_index < 16; ++level_index) {
        char name[16], path[1024];
        FILE *file;
        long file_size;
        uint8_t *data = NULL;
        Nexus_V1_Engine engine;
        Nexus_V1_Level level;
        int source_entry;

        snprintf(name, sizeof(name), "LEV%02d.DGN", level_index);
        if (!nexus_v1_known_file_md5(name) ||
            snprintf(path, sizeof(path), "%s/%s", data_dir, name) >=
                (int)sizeof(path) ||
            !asset_file_matches_md5(path, nexus_v1_known_file_md5(name)) ||
            !(file = fopen(path, "rb"))) return -1;
        if (fseek(file, 0L, SEEK_END) != 0 || (file_size = ftell(file)) <= 0L ||
            fseek(file, 0L, SEEK_SET) != 0 ||
            !(data = (uint8_t *)malloc((size_t)file_size)) ||
            fread(data, 1U, (size_t)file_size, file) != (size_t)file_size) {
            free(data);
            fclose(file);
            return -1;
        }
        fclose(file);
        memset(&engine, 0, sizeof(engine));
        memset(&level, 0, sizeof(level));
        if (nexus_v1_level_load(&level, data, (int)file_size, level_index) != 0) {
            free(data);
            return -1;
        }
        engine.level_loaded = 1;
        engine.game.current_level = level_index;
        engine.current_level = level;
        engine.current_level_dgn_data = data;
        engine.current_level_dgn_size = (int)file_size;
        engine.current_level_structure2_source.level_index = level_index;
        engine.current_level_structure2_source.canonical_hash_verified = 1;
        engine.current_level_structure2_source.materialization_bound = 1;
        engine.current_level_structure2_source.structure2_payload_envelope_valid = 1;
        engine.current_level_structure2_source.loaded_bytes_bound = 1;
        engine.current_level_structure2_source.loaded_dgn_size = (int)file_size;
        engine.current_level_structure2_source.loaded_dgn_fnv1a64 =
            fnv1a64(data, (size_t)file_size);
        for (source_entry = 0;
             source_entry < engine.current_level.structure1f_entry_count;
             ++source_entry) {
            Nexus_V1_DgnStructure1FTransformCaptureTarget target;
            Nexus_V1_DgnStructure1FDirectStaticMaterialCaptureTarget static_target;
            if (nexus_v1_engine_build_structure1f_transform_capture_target(
                    &engine, source_entry, &target) == 1 && target.valid &&
                target.geometry.source_geometry_bound &&
                target.geometry.vertex_slot_count >= 3 &&
                target.geometry.vertex_slot_count <= 4 &&
                target.transform_table.valid &&
                target.transform_table.source_table_bound &&
                target.transform_table.parsed_model_rows_match &&
                target.transform_table.entry_count > 0 &&
                target.transform_table.table_byte_count ==
                    target.transform_table.entry_count *
                        NEXUS_DGN_STRUCTURE1A_ENTRY_BYTES &&
                target.transform_table.raw_table_fnv1a64 != 0U &&
                target.transform_table.selector_column_fnv1a64 != 0U &&
                target.transform_table.selectors.complete &&
                !target.transform_table.transform_semantics_proven &&
                target.transform_table_source_bound &&
                target.owner_transform_selector_source_bound &&
                target.transform_selectors.resolved_selector_count > 0 &&
                !target.transform_semantics_proven &&
                target.capture_producer_required &&
                target.original_saturn_capture_required && target.no_draw_only &&
                !target.fallback_visuals_permitted &&
                target.blocks_real_dgn_mesh_render) {
                ++target_count;
            }
            memset(&static_target, 0, sizeof(static_target));
            if (nexus_v1_engine_build_structure1f_direct_static_material_capture_target(
                    &engine, source_entry, &static_target) == 1 &&
                static_target.valid && static_target.direct_face_material_bound &&
                static_target.static_material.static_selector_descriptor_bound &&
                static_target.static_material.image_payload_anchor_bound &&
                static_target.static_material.image_payload_interval_bound &&
                static_target.no_draw_only &&
                !static_target.fallback_visuals_permitted &&
                static_target.blocks_real_dgn_mesh_render) {
                ++static_target_count;
            }
        }
        free(data);
    }
    *out_static_count = static_target_count;
    return target_count;
}

int main(void)
{
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    char path[2048];
    FILE *file;
    long file_size;
    uint8_t *data = NULL;
    Nexus_V1_Engine engine;
    Nexus_V1_Level level;
    int source_entry;
    int found = 0;
    int untextured_found = 0;
    int animated_found = 0;
    int transform_found = 0;
    int adjacency_transform_found = 0;
    int direct_static_target_count = 0;
    Nexus_V1_LevCorpusDiscoveryReceipt lev_corpus;

    if (!data_dir || !data_dir[0]) {
        puts("skip: FIRESTAFF_NEXUS_DATA_DIR is not set");
        return 0;
    }
    {
        Nexus_V1_DgnStructure1FDirectFaceCaptureHostReceipt host_receipt;

        memset(&host_receipt, 0, sizeof(host_receipt));
        CHECK(nexus_v1_launcher_dgn_direct_face_capture_intake(
                  0, "unavailable\n", sizeof("unavailable\n") - 1U,
                  &host_receipt) == 0 &&
              !host_receipt.launcher_initialized &&
              !host_receipt.active_level_bound &&
              !host_receipt.package_source_bound &&
              host_receipt.no_draw_only &&
              !host_receipt.fallback_visuals_permitted &&
              host_receipt.blocks_real_dgn_mesh_render,
              "uninitialized launcher cannot bypass direct-face package ownership");
    }
    {
        Nexus_V1_Engine correlation_engine;
        Nexus_V1_MenuBpkPaltWarningPaletteCorrelationReceipt correlation;
        Nexus_V1_DmBinVdp1StateWriteReceipt vdp1_state;

        memset(&correlation_engine, 0, sizeof(correlation_engine));
        correlation_engine.source = NEXUS_SRC_EXTRACTED;
        snprintf(correlation_engine.data_dir,
                 sizeof(correlation_engine.data_dir), "%s", data_dir);
        memset(&correlation, 0, sizeof(correlation));
        CHECK(nexus_v1_engine_menu_bpk_palt_warning_palette_correlation(
                  &correlation_engine, &correlation) == 1 && correlation.valid &&
              correlation.menu_bpk_source_hash_verified &&
              correlation.warning_source_hash_verified &&
              correlation.palt.valid && correlation.warning_dgt2_pp_bound &&
              correlation.warning_clut_fnv1a64 != 0U &&
              correlation.matching_entry_count == 224U &&
              correlation.mismatched_entry_count == 32U &&
              correlation.bgr555_low15_matching_entry_count == 255U &&
              correlation.bgr555_low15_mismatched_entry_count == 1U &&
              correlation.high_bit_only_mismatch_count == 31U &&
              correlation.colour_word_mismatch_count == 1U &&
              correlation.indexed_word_alignment_proven &&
              correlation.bgr555_word_encoding_correlation_proven &&
              correlation.bgr555_low15_correlation_proven &&
              !correlation.prs3_palette_association_proven &&
              !correlation.palette_application_proven &&
              !correlation.decoder_promoted && correlation.no_draw_only &&
              !correlation.fallback_visuals_permitted,
              "canonical MENU.BPK PALT aligns with the documented WARNING DGT2 CLUT without enabling PRS3 rendering");
        memset(&vdp1_state, 0, sizeof(vdp1_state));
        CHECK(nexus_v1_engine_dm_bin_vdp1_state_write_receipt(
                  &correlation_engine, &vdp1_state) == 1 &&
              vdp1_state.source.canonical_hash_verified &&
              vdp1_state.static_instruction_dataflow_proven &&
              vdp1_state.vdp1_register_0x04_write_proven &&
              vdp1_state.vdp1_register_0x04_value == 2U &&
              vdp1_state.vdp1_vram_base_r14_store_proven &&
              vdp1_state.vdp1_register_0x06_write_proven &&
              vdp1_state.vdp1_register_0x06_value == 0x8000U &&
              vdp1_state.vdp1_register_0x08_write_proven &&
              vdp1_state.vdp1_register_0x08_value == 0U &&
              vdp1_state.vdp1_register_0x0a_write_proven &&
              vdp1_state.vdp1_register_0x0a_value == 0xffffU &&
              !vdp1_state.vdp1_command_emission_proven &&
              !vdp1_state.palette_semantics_proven &&
              !vdp1_state.transform_semantics_proven &&
              vdp1_state.no_draw_only && !vdp1_state.fallback_visuals_permitted,
              "canonical DM.BIN contributes only raw VDP1 capture prerequisites");
    }
    CHECK(snprintf(path, sizeof(path), "%s/LEV01.DGN", data_dir) <
              (int)sizeof(path) &&
          asset_file_matches_md5(path, "751e1442bf7dccbd41bf146b5be144ab"),
          "canonical LEV01 source is hash verified");
    file = failures == 0 ? fopen(path, "rb") : NULL;
    CHECK(file != NULL, "canonical LEV01 opens");
    if (file) {
        CHECK(fseek(file, 0L, SEEK_END) == 0 &&
              (file_size = ftell(file)) > 0L &&
              fseek(file, 0L, SEEK_SET) == 0,
              "canonical LEV01 has bounded source bytes");
        if (failures == 0) {
            data = (uint8_t *)malloc((size_t)file_size);
            CHECK(data != NULL && fread(data, 1U, (size_t)file_size, file) ==
                  (size_t)file_size, "canonical LEV01 reads completely");
        }
        fclose(file);
    }
    memset(&engine, 0, sizeof(engine));
    memset(&level, 0, sizeof(level));
    if (failures == 0) {
        CHECK(nexus_v1_level_load(&level, data, (int)file_size, 1) == 0,
              "canonical LEV01 parses before direct material selection");
    }
    if (failures == 0) {
        engine.level_loaded = 1;
        engine.game.current_level = 1;
        engine.current_level = level;
        engine.current_level_dgn_data = data;
        engine.current_level_dgn_size = (int)file_size;
        engine.current_level_structure2_source.level_index = 1;
        engine.current_level_structure2_source.canonical_hash_verified = 1;
        engine.current_level_structure2_source.materialization_bound = 1;
        engine.current_level_structure2_source.structure2_payload_envelope_valid = 1;
        engine.current_level_structure2_source.loaded_bytes_bound = 1;
        engine.current_level_structure2_source.loaded_dgn_size = (int)file_size;
        /* Match the active source receipt's FNV-1a identity requirement. */
        {
            uint64_t hash = UINT64_C(1469598103934665603);
            size_t index;
            for (index = 0U; index < (size_t)file_size; ++index) {
                hash ^= data[index];
                hash *= UINT64_C(1099511628211);
            }
            engine.current_level_structure2_source.loaded_dgn_fnv1a64 = hash;
        }
    }
    if (failures == 0) {
        memset(&lev_corpus, 0, sizeof(lev_corpus));
        CHECK(nexus_v1_lev_corpus_discover_direct(data_dir, &lev_corpus) == 1 &&
              lev_corpus.valid && lev_corpus.direct_files_only &&
              !lev_corpus.payload_materialized && lev_corpus.levels[1].valid,
              "canonical LEV00-15 direct corpus discovery retains only identities");
        CHECK(count_canonical_transform_capture_targets(data_dir,
                                                         &direct_static_target_count) > 0,
              "retail LEV00-15 binds direct mesh geometry to raw transform selectors");
        CHECK(direct_static_target_count == 0,
              "retail LEV00-15 cannot claim a direct Structure1F-to-Structure2 material link");
        for (source_entry = 0;
             source_entry < engine.current_level.structure1f_entry_count;
             ++source_entry) {
            Nexus_V1_DgnStructure1FTransformCaptureTarget target;
            char capture_path[128];
            char capture_text[4096];
            FILE *capture_file;
            size_t capture_size;
            Nexus_V1_DgnStructure1FDirectFaceCaptureManifestReceipt manifest_receipt;
            Nexus_V1_DgnStructure1FDirectFaceRawCaptureReceipt raw_receipt;
            Nexus_V1_DgnStructure1FVdp1MaterialReceipt material_receipt;
            Nexus_V1_DgnStructure3RawCaptureHostReceipt absent_raw_capture;
            Nexus_V1_DgnStructure1F2FaceAdjacencyTransformReceipt adjacency;
            char *level_value;

            memset(&target, 0, sizeof(target));
            if (nexus_v1_engine_build_structure1f_transform_capture_target(
                    &engine, source_entry, &target) != 1) {
                continue;
            }
            transform_found = 1;
            memset(&adjacency, 0, sizeof(adjacency));
            if (nexus_v1_engine_build_structure1f2_face_adjacency_transform_receipt(
                    &engine, source_entry, &adjacency) == 1) {
                Nexus_V1_DgnStructure1F2FaceAdjacencyTransformReceipt stale;
                Nexus_V1_DgnM11DirectLevNoDrawReceipt m11_dungeon;
                Nexus_V1_MultiLevelCaptureCampaignLaunchPlan campaign;
                Nexus_V1_LauncherMultiLevelM11DungeonHandoffReceipt
                    campaign_handoff;
                Nexus_V1_LauncherM11Structure2FaceCaptureReplayTarget
                    replay_target;
                Nexus_V1_LauncherM11Structure2FaceCaptureReplayEvidence
                    replay_evidence;
                Nexus_V1_LauncherM11Structure2FaceCaptureReplayReceipt
                    replay_receipt;
                Nexus_V1_LauncherM11Structure2FaceVdp1CaptureImportReceipt
                    capture_import;
                Nexus_V1_LauncherM11Structure3TopologyCaptureReplayTarget
                    topology_target;
                Nexus_V1_LauncherM11Structure3TopologyCaptureImportReceipt
                    topology_import;
                Nexus_V1_LauncherM12M11Structure3TopologyCaptureRouteReceipt
                    topology_capture_route;
                Nexus_V1_LauncherM12M11Structure3TopologyCaptureRouteReceipt
                    topology_capture_resume;
                Nexus_V1_LauncherM11Structure3TopologyLocalArtifactReceipt
                    topology_local_artifact;
                Nexus_V1_LauncherM12M11Vdp1CaptureRouteReceipt capture_route;
                Nexus_V1_LauncherM12M11Vdp1CaptureRouteReceipt capture_resume;
                Nexus_V1_LauncherM11Structure2FaceVdp1LocalArtifactReceipt
                    vdp1_local_artifact;
                uint8_t capture_fixture[
                    NEXUS_V1_M11_STRUCTURE2_FACE_VDP1_CAPTURE_HEADER_BYTES + 16U];
                uint8_t topology_capture_fixture[
                    NEXUS_V1_M11_STRUCTURE3_TOPOLOGY_CAPTURE_HEADER_BYTES + 16U];
                static const uint8_t opaque_capture_fixture[16] = {
                    0x83U, 0x19U, 0xc4U, 0x5eU, 0x27U, 0xb0U, 0x6dU, 0xfaU,
                    0x40U, 0x92U, 0x1eU, 0x75U, 0xacU, 0x36U, 0xd8U, 0x0bU
                };

                adjacency_transform_found = 1;
                CHECK(adjacency.valid &&
                      adjacency.structure1f_record_source_bound &&
                      adjacency.face_mesh_adjacency_bound &&
                      adjacency.structure2_source_envelope_bound &&
                      adjacency.owner_transform_selector_bound &&
                      !adjacency.transform_semantics_proven &&
                      adjacency.no_draw_only &&
                      !adjacency.fallback_visuals_permitted &&
                      adjacency.blocks_real_dgn_mesh_render &&
                      nexus_v1_engine_consume_structure1f2_face_adjacency_transform_no_draw(
                          &engine, &adjacency) == 1,
                      "M11 consumes only exact parser-bound Structure1F/3 face adjacency, Structure2 envelope identity, and raw transform selectors");
                memset(&m11_dungeon, 0, sizeof(m11_dungeon));
                engine.external_saturn_save_capture_valid = 1;
                engine.external_saturn_save_card_fnv1a64 = UINT64_C(0x1234);
                engine.external_saturn_save_route_epoch = 1U;
                engine.menu_bpk_no_draw_host_valid = 1;
                engine.menu_bpk_no_draw_host_route_epoch = 1U;
                engine.menu_bpk_package_fnv1a64 = UINT64_C(0x5678);
                engine.menu_bpk_no_draw_host_package_fnv1a64 = UINT64_C(0x5678);
                memset(&campaign, 0, sizeof(campaign));
                campaign.valid = 1;
                campaign.operator_only = 1;
                campaign.jobs[1].valid = 1;
                campaign.jobs[1].level_index = 1U;
                campaign.jobs[1].dgn_fnv1a64 = lev_corpus.levels[1].fnv1a64;
                memset(&campaign_handoff, 0, sizeof(campaign_handoff));
                CHECK(nexus_v1_launcher_admit_multi_level_m11_dungeon_handoff(
                          &engine, &lev_corpus, &campaign, 1U, UINT64_C(0x5678),
                          UINT64_C(0x1234), 1, 1U, &adjacency,
                          &campaign_handoff) == 1 &&
                      campaign_handoff.valid && campaign_handoff.no_draw_only &&
                      campaign_handoff.draw_disabled &&
                      campaign_handoff.level_index == 1U &&
                      campaign_handoff.route_epoch == 1U &&
                      campaign_handoff.dgn_fnv1a64 ==
                          lev_corpus.levels[1].fnv1a64 &&
                      campaign_handoff.direct_lev.valid &&
                      campaign_handoff.direct_lev.package_fnv1a64 ==
                          UINT64_C(0x5678) &&
                      campaign_handoff.direct_lev.card_fnv1a64 ==
                          UINT64_C(0x1234) &&
                      nexus_v1_engine_m11_direct_lev_dungeon_no_draw_ready(
                          &engine, 1U, 1, lev_corpus.levels[1].md5,
                          lev_corpus.levels[1].byte_count,
                          lev_corpus.levels[1].fnv1a64, &m11_dungeon) == 1 &&
                      m11_dungeon.valid && m11_dungeon.no_draw_only &&
                      !m11_dungeon.fallback_visuals_permitted &&
                      m11_dungeon.blocks_real_dgn_mesh_render &&
                      m11_dungeon.header_descriptor.valid &&
                      m11_dungeon.header_descriptor.level_index == 1U &&
                      m11_dungeon.header_descriptor.package_fnv1a64 ==
                          lev_corpus.levels[1].fnv1a64 &&
                      m11_dungeon.header_descriptor.header_length == 0x14U &&
                      m11_dungeon.header_descriptor.descriptor_length > 0U &&
                      m11_dungeon.header_descriptor.no_draw_only &&
                      !m11_dungeon.header_descriptor.fallback_visuals_permitted &&
                      m11_dungeon.structure1f_descriptor.valid &&
                      m11_dungeon.structure1f_descriptor.level_index == 1U &&
                      m11_dungeon.structure1f_descriptor.route_epoch == 1U &&
                      m11_dungeon.structure1f_descriptor.package_fnv1a64 ==
                          lev_corpus.levels[1].fnv1a64 &&
                      m11_dungeon.structure1f_descriptor.structure1f_entry_index ==
                          adjacency.structure1f_source.entry_index &&
                      m11_dungeon.structure1f_descriptor.descriptor_offset ==
                          adjacency.structure1f_source.descriptor_offset &&
                      m11_dungeon.structure1f_descriptor.descriptor_length ==
                          adjacency.structure1f_source.descriptor_length &&
                      m11_dungeon.structure1f_descriptor.descriptor_fnv1a64 ==
                          adjacency.structure1f_source.descriptor_fnv1a64 &&
                      m11_dungeon.structure1f_descriptor.face_mesh_reference_bound &&
                      m11_dungeon.structure1f_descriptor.material_reference_opaque &&
                      m11_dungeon.structure1f_descriptor.no_draw_only &&
                      !m11_dungeon.structure1f_descriptor.fallback_visuals_permitted &&
                      m11_dungeon.structure2_face_descriptor.valid &&
                      m11_dungeon.structure2_face_descriptor.level_index == 1U &&
                      m11_dungeon.structure2_face_descriptor.route_epoch == 1U &&
                      m11_dungeon.structure2_face_descriptor.package_fnv1a64 ==
                          lev_corpus.levels[1].fnv1a64 &&
                      m11_dungeon.structure2_face_descriptor.structure1f_entry_index ==
                          adjacency.structure1f_source.entry_index &&
                      m11_dungeon.structure2_face_descriptor.face_length == 12U &&
                      m11_dungeon.structure2_face_descriptor.face_fnv1a64 != 0U &&
                      m11_dungeon.structure2_face_descriptor.descriptor_length ==
                          NEXUS_DGN_STRUCTURE2_DESCRIPTOR_BYTES &&
                      m11_dungeon.structure2_face_descriptor.descriptor_fnv1a64 != 0U &&
                      m11_dungeon.structure2_face_descriptor.image_candidate_length > 0U &&
                      m11_dungeon.structure2_face_descriptor.image_candidate_fnv1a64 != 0U &&
                      m11_dungeon.structure2_face_descriptor.face_descriptor_bound &&
                      m11_dungeon.structure2_face_descriptor.candidates_opaque &&
                      m11_dungeon.structure2_face_descriptor.no_draw_only &&
                      !m11_dungeon.structure2_face_descriptor.fallback_visuals_permitted &&
                      m11_dungeon.structure3_topology_descriptor.valid &&
                      m11_dungeon.structure3_topology_descriptor.level_index == 1U &&
                      m11_dungeon.structure3_topology_descriptor.route_epoch == 1U &&
                      m11_dungeon.structure3_topology_descriptor.package_fnv1a64 ==
                          lev_corpus.levels[1].fnv1a64 &&
                      m11_dungeon.structure3_topology_descriptor.structure1f_entry_index ==
                          adjacency.structure1f_source.entry_index &&
                      m11_dungeon.structure3_topology_descriptor.face_offset ==
                          m11_dungeon.structure2_face_descriptor.face_offset &&
                      m11_dungeon.structure3_topology_descriptor.face_fnv1a64 ==
                          m11_dungeon.structure2_face_descriptor.face_fnv1a64 &&
                      m11_dungeon.structure3_topology_descriptor.vertex_table_length > 0U &&
                      m11_dungeon.structure3_topology_descriptor.vertex_table_fnv1a64 != 0U &&
                      m11_dungeon.structure3_topology_descriptor.face_vertex_index_count >= 3U &&
                      m11_dungeon.structure3_topology_descriptor.face_vertex_index_count <= 4U &&
                      m11_dungeon.structure3_topology_descriptor.referenced_vertex_rows_fnv1a64 != 0U &&
                      m11_dungeon.structure3_topology_descriptor.normal_length == 12U &&
                      m11_dungeon.structure3_topology_descriptor.normal_fnv1a64 != 0U &&
                      m11_dungeon.structure3_topology_descriptor.topology_framing_bound &&
                      m11_dungeon.structure3_topology_descriptor.capture_required &&
                      m11_dungeon.structure3_topology_descriptor.no_draw_only &&
                      !m11_dungeon.structure3_topology_descriptor.fallback_visuals_permitted,
                      "M12/M11 champion handoff admits exact direct LEV Structure1F/Structure3 topology framing only as capture-required no-draw evidence");
                memset(&topology_target, 0, sizeof(topology_target));
                memset(&topology_import, 0, sizeof(topology_import));
                CHECK(nexus_v1_launcher_build_m11_structure3_topology_capture_replay_target(
                          &engine, &lev_corpus, &campaign_handoff.direct_lev,
                          &topology_target) == 1 && topology_target.valid &&
                      topology_target.direct_source_rehashed &&
                      topology_target.original_saturn_capture_required &&
                      topology_target.no_draw_only && !topology_target.payload_materialized &&
                      topology_target.topology.capture_required &&
                      topology_target.topology.topology_framing_bound &&
                      !topology_target.fallback_visuals_permitted,
                      "M11 builds a direct-source-rehashed Structure3 topology capture-required target");
                memset(topology_capture_fixture, 0, sizeof(topology_capture_fixture));
                memcpy(topology_capture_fixture,
                       NEXUS_V1_M11_STRUCTURE3_TOPOLOGY_CAPTURE_MAGIC, 8U);
                write_be32(topology_capture_fixture, 8U,
                           NEXUS_V1_M11_STRUCTURE3_TOPOLOGY_CAPTURE_VERSION);
                write_be32(topology_capture_fixture, 12U,
                           NEXUS_V1_M11_STRUCTURE3_TOPOLOGY_CAPTURE_HEADER_BYTES);
                write_be64(topology_capture_fixture, 16U, topology_target.route_epoch);
                write_be64(topology_capture_fixture, 24U, topology_target.package_fnv1a64);
                write_be64(topology_capture_fixture, 32U, topology_target.card_fnv1a64);
                write_be64(topology_capture_fixture, 40U, topology_target.dgn_fnv1a64);
                write_be64(topology_capture_fixture, 48U, topology_target.dgn_byte_count);
                write_be32(topology_capture_fixture, 56U,
                           (uint32_t)topology_target.topology.structure1f_entry_index);
                write_be32(topology_capture_fixture, 60U,
                           topology_target.topology.structure3_entry_index);
                write_be32(topology_capture_fixture, 64U,
                           topology_target.topology.face_ordinal);
                write_be32(topology_capture_fixture, 68U,
                           topology_target.topology.vertex_table_offset);
                write_be32(topology_capture_fixture, 72U,
                           topology_target.topology.vertex_table_length);
                write_be64(topology_capture_fixture, 76U,
                           topology_target.topology.vertex_table_fnv1a64);
                write_be64(topology_capture_fixture, 84U,
                           topology_target.topology.referenced_vertex_rows_fnv1a64);
                write_be32(topology_capture_fixture, 92U,
                           topology_target.topology.normal_offset);
                write_be32(topology_capture_fixture, 96U,
                           topology_target.topology.normal_length);
                write_be64(topology_capture_fixture, 100U,
                           topology_target.topology.normal_fnv1a64);
                write_be32(topology_capture_fixture, 108U,
                           NEXUS_V1_M11_STRUCTURE3_TOPOLOGY_CAPTURE_HEADER_BYTES);
                write_be32(topology_capture_fixture, 112U,
                           sizeof(opaque_capture_fixture));
                memcpy(topology_capture_fixture +
                           NEXUS_V1_M11_STRUCTURE3_TOPOLOGY_CAPTURE_HEADER_BYTES,
                       opaque_capture_fixture, sizeof(opaque_capture_fixture));
                write_be64(topology_capture_fixture, 116U,
                           fnv1a64(opaque_capture_fixture,
                                   sizeof(opaque_capture_fixture)));
                CHECK(nexus_v1_launcher_import_m11_structure3_topology_capture(
                          &engine, &lev_corpus, &campaign_handoff.direct_lev,
                          &topology_target, topology_capture_fixture,
                          sizeof(topology_capture_fixture), &topology_import) == 1 &&
                      topology_import.valid && topology_import.header_version_bound &&
                      topology_import.payload_bounds_bound &&
                      topology_import.payload_hash_bound && topology_import.target_bound &&
                      topology_import.topology_opaque && topology_import.no_draw_only &&
                      !topology_import.fallback_visuals_permitted,
                      "M11 imports only an exact bounded Structure3 topology capture envelope as opaque evidence");
                topology_capture_fixture[100U] ^= 1U;
                CHECK(nexus_v1_launcher_import_m11_structure3_topology_capture(
                          &engine, &lev_corpus, &campaign_handoff.direct_lev,
                          &topology_target, topology_capture_fixture,
                          sizeof(topology_capture_fixture), &topology_import) == 0 &&
                      !topology_import.valid && topology_import.no_draw_only,
                      "M11 rejects a Structure3 topology capture whose paired-normal identity drifts");
                topology_capture_fixture[100U] ^= 1U;
                topology_capture_fixture[32U] ^= 1U;
                CHECK(nexus_v1_launcher_import_m11_structure3_topology_capture(
                          &engine, &lev_corpus, &campaign_handoff.direct_lev,
                          &topology_target, topology_capture_fixture,
                          sizeof(topology_capture_fixture), &topology_import) == 0 &&
                      !topology_import.valid && topology_import.no_draw_only,
                      "M11 rejects a Structure3 topology capture whose bound card identity drifts");
                topology_capture_fixture[32U] ^= 1U;
                memset(&topology_capture_route, 0, sizeof(topology_capture_route));
                CHECK(nexus_v1_launcher_admit_m12_m11_structure3_topology_capture_required(
                          &engine, &lev_corpus, &campaign_handoff.direct_lev,
                          "96e106f740ab448cf89f0dd49dfbac7fe5391cb6bd6e14ad5e3061c13330266f",
                          NEXUS_V1_LAUNCHER_SATURN_BIOS_REGION_US,
                          "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
                          &topology_target, &topology_capture_route) == 1 &&
                      topology_capture_route.valid &&
                      topology_capture_route.capture_required &&
                      topology_capture_route.operator_only &&
                      topology_capture_route.no_draw_only &&
                      !topology_capture_route.capture_imported &&
                      !topology_capture_route.resume_ready &&
                      !topology_capture_route.decoder_permitted &&
                      !topology_capture_route.fallback_visuals_permitted,
                      "M12/M11 keeps NXS3TOP1 as a BIOS/disc-bound capture-required no-draw route");
                memset(&topology_capture_resume, 0, sizeof(topology_capture_resume));
                memset(&topology_local_artifact, 0, sizeof(topology_local_artifact));
                CHECK(nexus_v1_launcher_verify_m11_structure3_topology_local_artifact(
                          &topology_capture_route,
                          "96e106f740ab448cf89f0dd49dfbac7fe5391cb6bd6e14ad5e3061c13330266f",
                          "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
                          topology_capture_fixture, sizeof(topology_capture_fixture),
                          &topology_local_artifact) == 1 && topology_local_artifact.valid &&
                      topology_local_artifact.bios_bound && topology_local_artifact.disc_bound &&
                      topology_local_artifact.lev_bound && topology_local_artifact.face_bound &&
                      topology_local_artifact.vertex_bound && topology_local_artifact.normal_bound &&
                      topology_local_artifact.payload_bounds_bound &&
                      topology_local_artifact.payload_hash_bound &&
                      topology_local_artifact.topology_opaque &&
                      topology_local_artifact.no_draw_only &&
                      !topology_local_artifact.fallback_visuals_permitted,
                      "local NXS3TOP1 preflight binds BIOS, disc, LEV, face, vertex, normal, and opaque payload");
                CHECK(nexus_v1_launcher_resume_m12_m11_structure3_topology_local_artifact(
                          &engine, &lev_corpus, &campaign_handoff.direct_lev,
                          &topology_capture_route,
                          "96e106f740ab448cf89f0dd49dfbac7fe5391cb6bd6e14ad5e3061c13330266f",
                          "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
                          topology_capture_fixture,
                          sizeof(topology_capture_fixture), &topology_capture_resume) == 1 &&
                      topology_capture_resume.valid &&
                      !topology_capture_resume.capture_required &&
                      topology_capture_resume.capture_imported &&
                      topology_capture_resume.resume_ready &&
                      topology_capture_resume.capture.valid &&
                      topology_capture_resume.capture.topology_opaque &&
                      topology_capture_resume.no_draw_only &&
                      !topology_capture_resume.decoder_permitted &&
                      !topology_capture_resume.fallback_visuals_permitted,
                      "M12/M11 resumes NXS3TOP1 only after local preflight and the exact opaque topology importer");
                topology_capture_fixture[76U] ^= 1U;
                CHECK(nexus_v1_launcher_verify_m11_structure3_topology_local_artifact(
                          &topology_capture_route,
                          "96e106f740ab448cf89f0dd49dfbac7fe5391cb6bd6e14ad5e3061c13330266f",
                          "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
                          topology_capture_fixture, sizeof(topology_capture_fixture),
                          &topology_local_artifact) == 0 && !topology_local_artifact.valid &&
                      topology_local_artifact.no_draw_only,
                      "local NXS3TOP1 preflight rejects vertex metadata drift before import");
                topology_capture_fixture[76U] ^= 1U;
                CHECK(nexus_v1_launcher_resume_m12_m11_structure3_topology_local_artifact(
                          &engine, &lev_corpus, &campaign_handoff.direct_lev,
                          &topology_capture_route,
                          "06e106f740ab448cf89f0dd49dfbac7fe5391cb6bd6e14ad5e3061c13330266f",
                          "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
                          topology_capture_fixture,
                          sizeof(topology_capture_fixture), &topology_capture_resume) == 0 &&
                      !topology_capture_resume.valid && topology_capture_resume.capture_required &&
                      topology_capture_resume.no_draw_only,
                      "local NXS3TOP1 bridge rejects BIOS identity drift before route resume");
                topology_capture_route.target.card_fnv1a64 ^= UINT64_C(1);
                CHECK(nexus_v1_launcher_resume_m12_m11_structure3_topology_local_artifact(
                          &engine, &lev_corpus, &campaign_handoff.direct_lev,
                          &topology_capture_route,
                          "96e106f740ab448cf89f0dd49dfbac7fe5391cb6bd6e14ad5e3061c13330266f",
                          "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
                          topology_capture_fixture,
                          sizeof(topology_capture_fixture), &topology_capture_resume) == 0 &&
                      !topology_capture_resume.valid &&
                      topology_capture_resume.capture_required &&
                      topology_capture_resume.no_draw_only,
                      "M12/M11 rejects NXS3TOP1 resume after saved card-route drift");
                topology_capture_route.target.card_fnv1a64 ^= UINT64_C(1);
                memset(&replay_target, 0, sizeof(replay_target));
                memset(&replay_evidence, 0, sizeof(replay_evidence));
                memset(&replay_receipt, 0, sizeof(replay_receipt));
                CHECK(nexus_v1_launcher_build_m11_structure2_face_capture_replay_target(
                          &engine, &lev_corpus, &campaign_handoff.direct_lev,
                          &replay_target) == 1 && replay_target.valid &&
                      replay_target.direct_source_rehashed &&
                      replay_target.original_saturn_capture_required &&
                      replay_target.no_draw_only && !replay_target.payload_materialized &&
                      !replay_target.fallback_visuals_permitted &&
                      replay_target.face_descriptor.descriptor_fnv1a64 ==
                          m11_dungeon.structure2_face_descriptor.descriptor_fnv1a64,
                      "M11 builds a rehashed payload-free original-Saturn Structure2 face capture target");
                replay_evidence.original_saturn_capture_verified = 1;
                replay_evidence.replay_sequence = replay_target.route_epoch;
                replay_evidence.trace_fnv1a64 = UINT64_C(0x1020304050607080);
                replay_evidence.trace_byte_count = 96U;
                replay_evidence.texture_candidate_fnv1a64 =
                    replay_target.face_descriptor.image_candidate_fnv1a64;
                replay_evidence.palette_candidate_fnv1a64 =
                    replay_target.face_descriptor.palette_candidate_fnv1a64;
                replay_evidence.vdp1_command_fnv1a64 = UINT64_C(0x8877665544332211);
                replay_evidence.vdp1_command_byte_count = 32U;
                CHECK(nexus_v1_launcher_admit_m11_structure2_face_capture_replay(
                          &engine, &lev_corpus, &campaign_handoff.direct_lev,
                          &replay_target, &replay_evidence, &replay_receipt) == 1 &&
                      replay_receipt.valid && replay_receipt.vdp1_binding_observed &&
                      replay_receipt.texture_candidate_bound &&
                      !replay_receipt.decoder_permitted && replay_receipt.no_draw_only &&
                      !replay_receipt.fallback_visuals_permitted,
                      "M11 accepts only exact original-Saturn capture lane identities as opaque no-draw replay evidence");
                memset(capture_fixture, 0, sizeof(capture_fixture));
                memcpy(capture_fixture,
                       NEXUS_V1_M11_STRUCTURE2_FACE_VDP1_CAPTURE_MAGIC, 8U);
                write_be32(capture_fixture, 8U,
                           NEXUS_V1_M11_STRUCTURE2_FACE_VDP1_CAPTURE_VERSION);
                write_be32(capture_fixture, 12U,
                           NEXUS_V1_M11_STRUCTURE2_FACE_VDP1_CAPTURE_HEADER_BYTES);
                write_be64(capture_fixture, 16U, replay_target.route_epoch);
                write_be64(capture_fixture, 24U, replay_target.package_fnv1a64);
                write_be64(capture_fixture, 32U, replay_target.card_fnv1a64);
                write_be64(capture_fixture, 40U, replay_target.dgn_fnv1a64);
                write_be64(capture_fixture, 48U, replay_target.dgn_byte_count);
                write_be64(capture_fixture, 56U,
                           replay_target.face_descriptor.face_fnv1a64);
                write_be64(capture_fixture, 64U,
                           replay_target.face_descriptor.descriptor_fnv1a64);
                write_be64(capture_fixture, 72U,
                           replay_target.face_descriptor.image_candidate_fnv1a64);
                write_be64(capture_fixture, 80U,
                           replay_target.face_descriptor.palette_candidate_fnv1a64);
                write_be32(capture_fixture, 88U,
                           NEXUS_V1_M11_STRUCTURE2_FACE_VDP1_CAPTURE_HEADER_BYTES);
                write_be32(capture_fixture, 92U, sizeof(opaque_capture_fixture));
                memcpy(capture_fixture +
                           NEXUS_V1_M11_STRUCTURE2_FACE_VDP1_CAPTURE_HEADER_BYTES,
                       opaque_capture_fixture, sizeof(opaque_capture_fixture));
                write_be64(capture_fixture, 96U,
                           fnv1a64(opaque_capture_fixture,
                                   sizeof(opaque_capture_fixture)));
                write_be64(capture_fixture, 104U, replay_evidence.trace_fnv1a64);
                write_be64(capture_fixture, 112U, replay_evidence.trace_byte_count);
                write_be64(capture_fixture, 120U,
                           replay_evidence.vdp1_command_fnv1a64);
                write_be64(capture_fixture, 128U,
                           replay_evidence.vdp1_command_byte_count);
                memset(&capture_import, 0, sizeof(capture_import));
                CHECK(nexus_v1_launcher_import_m11_structure2_face_vdp1_capture(
                          &engine, &lev_corpus, &campaign_handoff.direct_lev,
                          &replay_target, capture_fixture, sizeof(capture_fixture),
                          &capture_import) == 1 && capture_import.valid &&
                      capture_import.header_version_bound &&
                      capture_import.payload_bounds_bound &&
                      capture_import.payload_hash_bound && capture_import.target_bound &&
                      capture_import.capture_byte_count == sizeof(capture_fixture) &&
                      capture_import.payload_length == sizeof(opaque_capture_fixture) &&
                      !capture_import.decoder_permitted && capture_import.no_draw_only &&
                      !capture_import.fallback_visuals_permitted,
                      "M11 imports only a bounded, hash-bound external VDP1 capture envelope as no-draw evidence");
                memset(&capture_route, 0, sizeof(capture_route));
                CHECK(nexus_v1_launcher_admit_m12_m11_vdp1_capture_required(
                          &engine, &lev_corpus, &campaign_handoff.direct_lev,
                          "96e106f740ab448cf89f0dd49dfbac7fe5391cb6bd6e14ad5e3061c13330266f",
                          NEXUS_V1_LAUNCHER_SATURN_BIOS_REGION_US,
                          "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
                          &replay_target, &capture_route) == 1 &&
                      capture_route.valid && capture_route.capture_required &&
                      capture_route.operator_only && capture_route.no_draw_only &&
                      !capture_route.capture_imported && !capture_route.resume_ready &&
                      !capture_route.decoder_permitted &&
                      !capture_route.fallback_visuals_permitted,
                      "M12/M11 publishes an exact BIOS/disc-bound capture-required route before a VDP1 artifact exists");
                memset(&capture_resume, 0, sizeof(capture_resume));
                memset(&vdp1_local_artifact, 0, sizeof(vdp1_local_artifact));
                CHECK(nexus_v1_launcher_verify_m11_structure2_face_vdp1_local_artifact(
                          &capture_route,
                          "96e106f740ab448cf89f0dd49dfbac7fe5391cb6bd6e14ad5e3061c13330266f",
                          "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
                          capture_fixture, sizeof(capture_fixture),
                          &vdp1_local_artifact) == 1 && vdp1_local_artifact.valid &&
                      vdp1_local_artifact.bios_bound && vdp1_local_artifact.disc_bound &&
                      vdp1_local_artifact.dgn_bound && vdp1_local_artifact.face_bound &&
                      vdp1_local_artifact.descriptor_bound &&
                      vdp1_local_artifact.candidates_bound &&
                      vdp1_local_artifact.payload_bounds_bound &&
                      vdp1_local_artifact.payload_hash_bound &&
                      vdp1_local_artifact.trace_command_bound &&
                      vdp1_local_artifact.payload_opaque && vdp1_local_artifact.no_draw_only &&
                      !vdp1_local_artifact.fallback_visuals_permitted,
                      "local NXSVDP1C preflight binds BIOS, disc, DGN, face, descriptor, candidates, and opaque payload");
                CHECK(nexus_v1_launcher_resume_m12_m11_vdp1_local_artifact(
                          &engine, &lev_corpus, &campaign_handoff.direct_lev,
                          &capture_route,
                          "96e106f740ab448cf89f0dd49dfbac7fe5391cb6bd6e14ad5e3061c13330266f",
                          "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
                          capture_fixture, sizeof(capture_fixture),
                          &capture_resume) == 1 && capture_resume.valid &&
                      !capture_resume.capture_required &&
                      capture_resume.capture_imported && capture_resume.resume_ready &&
                      capture_resume.capture.valid && capture_resume.no_draw_only &&
                      !capture_resume.decoder_permitted &&
                      !capture_resume.fallback_visuals_permitted,
                      "M12/M11 resumes NXSVDP1C only after local preflight and opaque rebind");
                capture_fixture[72U] ^= 1U;
                CHECK(nexus_v1_launcher_verify_m11_structure2_face_vdp1_local_artifact(
                          &capture_route,
                          "96e106f740ab448cf89f0dd49dfbac7fe5391cb6bd6e14ad5e3061c13330266f",
                          "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
                          capture_fixture, sizeof(capture_fixture),
                          &vdp1_local_artifact) == 0 && !vdp1_local_artifact.valid &&
                      vdp1_local_artifact.no_draw_only,
                      "local NXSVDP1C preflight rejects image-candidate drift before import");
                capture_fixture[72U] ^= 1U;
                CHECK(nexus_v1_launcher_resume_m12_m11_vdp1_local_artifact(
                          &engine, &lev_corpus, &campaign_handoff.direct_lev,
                          &capture_route,
                          "06e106f740ab448cf89f0dd49dfbac7fe5391cb6bd6e14ad5e3061c13330266f",
                          "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
                          capture_fixture, sizeof(capture_fixture),
                          &capture_resume) == 0 && !capture_resume.valid &&
                      capture_resume.capture_required && capture_resume.no_draw_only,
                      "local NXSVDP1C bridge rejects BIOS identity drift before route resume");
                capture_route.target.card_fnv1a64 ^= UINT64_C(1);
                CHECK(nexus_v1_launcher_resume_m12_m11_vdp1_local_artifact(
                          &engine, &lev_corpus, &campaign_handoff.direct_lev,
                          &capture_route,
                          "96e106f740ab448cf89f0dd49dfbac7fe5391cb6bd6e14ad5e3061c13330266f",
                          "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
                          capture_fixture, sizeof(capture_fixture),
                          &capture_resume) == 0 && !capture_resume.valid &&
                      capture_resume.capture_required && capture_resume.no_draw_only,
                      "M12/M11 keeps the capture-required route closed when its saved card identity drifts");
                capture_route.target.card_fnv1a64 ^= UINT64_C(1);
                capture_fixture[64U] ^= 1U;
                CHECK(nexus_v1_launcher_import_m11_structure2_face_vdp1_capture(
                          &engine, &lev_corpus, &campaign_handoff.direct_lev,
                          &replay_target, capture_fixture, sizeof(capture_fixture),
                          &capture_import) == 0 && !capture_import.valid &&
                      capture_import.no_draw_only,
                      "M11 rejects an external VDP1 capture whose Structure2 descriptor identity drifts");
                capture_fixture[64U] ^= 1U;
                capture_fixture[NEXUS_V1_M11_STRUCTURE2_FACE_VDP1_CAPTURE_HEADER_BYTES] ^= 1U;
                CHECK(nexus_v1_launcher_import_m11_structure2_face_vdp1_capture(
                          &engine, &lev_corpus, &campaign_handoff.direct_lev,
                          &replay_target, capture_fixture, sizeof(capture_fixture),
                          &capture_import) == 0 && !capture_import.valid &&
                      capture_import.no_draw_only,
                      "M11 rejects an external VDP1 capture whose opaque payload hash drifts");
                capture_fixture[NEXUS_V1_M11_STRUCTURE2_FACE_VDP1_CAPTURE_HEADER_BYTES] ^= 1U;
                write_be32(capture_fixture, 8U,
                           NEXUS_V1_M11_STRUCTURE2_FACE_VDP1_CAPTURE_VERSION + 1U);
                CHECK(nexus_v1_launcher_import_m11_structure2_face_vdp1_capture(
                          &engine, &lev_corpus, &campaign_handoff.direct_lev,
                          &replay_target, capture_fixture, sizeof(capture_fixture),
                          &capture_import) == 0 && !capture_import.valid &&
                      capture_import.no_draw_only,
                      "M11 rejects an external VDP1 capture with an unsupported envelope version");
                replay_evidence.texture_candidate_fnv1a64 ^= UINT64_C(1);
                CHECK(nexus_v1_launcher_admit_m11_structure2_face_capture_replay(
                          &engine, &lev_corpus, &campaign_handoff.direct_lev,
                          &replay_target, &replay_evidence, &replay_receipt) == 0 &&
                      !replay_receipt.valid && replay_receipt.no_draw_only,
                      "M11 rejects replay evidence when its Structure2 texture candidate FNV drifts");
                replay_evidence.texture_candidate_fnv1a64 =
                    replay_target.face_descriptor.image_candidate_fnv1a64;
                campaign_handoff.direct_lev.card_fnv1a64 ^= UINT64_C(1);
                CHECK(nexus_v1_launcher_build_m11_structure2_face_capture_replay_target(
                          &engine, &lev_corpus, &campaign_handoff.direct_lev,
                          &replay_target) == 0 && !replay_target.valid &&
                      replay_target.no_draw_only,
                      "M11 rejects a capture target when the admitted direct-card route drifts");
                campaign_handoff.direct_lev.card_fnv1a64 ^= UINT64_C(1);
                campaign.jobs[1].dgn_fnv1a64 ^= UINT64_C(1);
                CHECK(nexus_v1_launcher_admit_multi_level_m11_dungeon_handoff(
                          &engine, &lev_corpus, &campaign, 1U, UINT64_C(0x5678),
                          UINT64_C(0x1234), 1, 1U, &adjacency,
                          &campaign_handoff) == 0 &&
                      !campaign_handoff.valid && campaign_handoff.no_draw_only &&
                      campaign_handoff.draw_disabled,
                      "M11 rejects a selected capture-plan job when its DGN FNV drifts");
                campaign.jobs[1].dgn_fnv1a64 = lev_corpus.levels[1].fnv1a64;
                CHECK(nexus_v1_engine_m11_direct_lev_dungeon_no_draw_ready(
                          &engine, 1U, 0, lev_corpus.levels[1].md5,
                          lev_corpus.levels[1].byte_count,
                          lev_corpus.levels[1].fnv1a64, NULL) == 0 &&
                      nexus_v1_engine_m11_direct_lev_dungeon_no_draw_ready(
                          &engine, 2U, 1, lev_corpus.levels[1].md5,
                          lev_corpus.levels[1].byte_count,
                          lev_corpus.levels[1].fnv1a64, NULL) == 0 &&
                      nexus_v1_engine_m11_direct_lev_dungeon_no_draw_ready(
                          &engine, 1U, 1, lev_corpus.levels[1].md5,
                          lev_corpus.levels[1].byte_count,
                          lev_corpus.levels[1].fnv1a64 ^ UINT64_C(1), NULL) == 0,
                      "M11 rejects level swap, stale epoch, and package FNV drift");
                stale = adjacency;
                stale.structure1f_source.package_fnv1a64 ^= UINT64_C(1);
                CHECK(nexus_v1_engine_consume_structure1f2_face_adjacency_transform_no_draw(
                          &engine, &stale) == 0,
                      "M11 rejects face adjacency receipts from a stale DGN package");
                stale = adjacency;
                stale.structure1f_source.descriptor_offset =
                    (uint32_t)stale.structure1f_source.source_byte_count;
                stale.structure1f_source.descriptor_length = 1U;
                CHECK(nexus_v1_engine_consume_structure1f2_face_adjacency_transform_no_draw(
                          &engine, &stale) == 0,
                      "M11 rejects Structure1F adjacency receipts with an out-of-bounds parser span");
                engine.m11_direct_lev_dungeon.structure1f_descriptor
                    .descriptor_fnv1a64 ^= UINT64_C(1);
                CHECK(nexus_v1_engine_m11_direct_lev_dungeon_no_draw_ready(
                          &engine, 1U, 1, lev_corpus.levels[1].md5,
                          lev_corpus.levels[1].byte_count,
                          lev_corpus.levels[1].fnv1a64, NULL) == 0,
                      "M11 rejects stale selected Structure1F descriptor FNV");
                engine.m11_direct_lev_dungeon.structure1f_descriptor
                    .descriptor_fnv1a64 ^= UINT64_C(1);
                engine.m11_direct_lev_dungeon.structure1f_descriptor
                    .descriptor_offset =
                    engine.m11_direct_lev_dungeon.header_descriptor.descriptor_offset +
                    engine.m11_direct_lev_dungeon.header_descriptor.descriptor_length;
                CHECK(nexus_v1_engine_m11_direct_lev_dungeon_no_draw_ready(
                          &engine, 1U, 1, lev_corpus.levels[1].md5,
                          lev_corpus.levels[1].byte_count,
                          lev_corpus.levels[1].fnv1a64, NULL) == 0,
                      "M11 rejects selected Structure1F descriptors outside the parser-bounded table");
                engine.m11_direct_lev_dungeon.structure1f_descriptor
                    .descriptor_offset = adjacency.structure1f_source.descriptor_offset;
                engine.m11_direct_lev_dungeon.structure2_face_descriptor
                    .descriptor_fnv1a64 ^= UINT64_C(1);
                CHECK(nexus_v1_engine_m11_direct_lev_dungeon_no_draw_ready(
                          &engine, 1U, 1, lev_corpus.levels[1].md5,
                          lev_corpus.levels[1].byte_count,
                          lev_corpus.levels[1].fnv1a64, NULL) == 0,
                      "M11 rejects stale selected Structure2 descriptor FNV");
                engine.m11_direct_lev_dungeon.structure2_face_descriptor
                    .descriptor_fnv1a64 ^= UINT64_C(1);
                engine.m11_direct_lev_dungeon.structure2_face_descriptor
                    .face_offset = (uint32_t)engine.current_level_dgn_size;
                CHECK(nexus_v1_engine_m11_direct_lev_dungeon_no_draw_ready(
                          &engine, 1U, 1, lev_corpus.levels[1].md5,
                          lev_corpus.levels[1].byte_count,
                          lev_corpus.levels[1].fnv1a64, NULL) == 0,
                      "M11 rejects selected Structure3 face descriptors outside the loaded DGN bounds");
                engine.m11_direct_lev_dungeon.structure2_face_descriptor
                    .face_offset = m11_dungeon.structure2_face_descriptor.face_offset;
                engine.m11_direct_lev_dungeon.structure3_topology_descriptor
                    .referenced_vertex_rows_fnv1a64 ^= UINT64_C(1);
                CHECK(nexus_v1_engine_m11_direct_lev_dungeon_no_draw_ready(
                          &engine, 1U, 1, lev_corpus.levels[1].md5,
                          lev_corpus.levels[1].byte_count,
                          lev_corpus.levels[1].fnv1a64, NULL) == 0,
                      "M11 rejects a selected Structure3 topology receipt whose parser-observed vertex rows drift");
                engine.m11_direct_lev_dungeon.structure3_topology_descriptor
                    .referenced_vertex_rows_fnv1a64 ^= UINT64_C(1);
                engine.m11_direct_lev_dungeon.structure3_topology_descriptor
                    .vertex_table_offset = (uint32_t)engine.current_level_dgn_size;
                CHECK(nexus_v1_engine_m11_direct_lev_dungeon_no_draw_ready(
                          &engine, 1U, 1, lev_corpus.levels[1].md5,
                          lev_corpus.levels[1].byte_count,
                          lev_corpus.levels[1].fnv1a64, NULL) == 0,
                      "M11 rejects an out-of-bounds selected Structure3 topology vertex table");
                engine.m11_direct_lev_dungeon.structure3_topology_descriptor
                    .vertex_table_offset = m11_dungeon.structure3_topology_descriptor
                        .vertex_table_offset;
                engine.m11_direct_lev_dungeon.header_descriptor.descriptor_fnv1a64 ^=
                    UINT64_C(1);
                CHECK(nexus_v1_engine_m11_direct_lev_dungeon_no_draw_ready(
                          &engine, 1U, 1, lev_corpus.levels[1].md5,
                          lev_corpus.levels[1].byte_count,
                          lev_corpus.levels[1].fnv1a64, NULL) == 0,
                      "M11 rejects stale saved Structure1F descriptor provenance");
            }
            CHECK(target.valid && target.geometry.source_geometry_bound &&
                  target.geometry.direct_mesh.structure1f_entry_index == source_entry &&
                  target.transform_table.valid &&
                  target.transform_table.source_table_bound &&
                  target.transform_table.parsed_model_rows_match &&
                  target.transform_table.entry_count > 0 &&
                  target.transform_table.table_byte_count ==
                      target.transform_table.entry_count *
                          NEXUS_DGN_STRUCTURE1A_ENTRY_BYTES &&
                  target.transform_table.raw_table_fnv1a64 != 0U &&
                  target.transform_table.selector_column_fnv1a64 != 0U &&
                  target.transform_table.selectors.complete &&
                  !target.transform_table.transform_semantics_proven &&
                  target.transform_table_source_bound &&
                  target.owner_transform_selector_source_bound &&
                  target.transform_selectors.resolved_selector_count > 0 &&
                  !target.transform_semantics_proven &&
                  target.capture_producer_required &&
                  target.original_saturn_capture_required && target.no_draw_only &&
                  !target.fallback_visuals_permitted &&
                  target.blocks_real_dgn_mesh_render,
                  "direct owner retains raw transform selectors without assigning semantics");
            snprintf(capture_path, sizeof(capture_path),
                     "/tmp/firestaff-nexus-direct-face-%d.txt", source_entry);
            CHECK(nexus_v1_engine_write_structure1f_direct_face_capture_target(
                      &engine, source_entry, capture_path, &target) == 1 &&
                  target.valid && target.geometry.source_geometry_bound &&
                  target.no_draw_only && !target.fallback_visuals_permitted &&
                  target.blocks_real_dgn_mesh_render,
                  "direct owner writes an authoritative no-draw face capture bridge");
            capture_file = fopen(capture_path, "rb");
            capture_size = capture_file ? fread(capture_text, 1U,
                                                 sizeof(capture_text) - 1U,
                                                 capture_file) : 0U;
            if (capture_file) fclose(capture_file);
            capture_text[capture_size] = '\0';
            CHECK(capture_size > 0U &&
                  strstr(capture_text,
                         NEXUS_V1_STRUCTURE1F_DIRECT_FACE_CAPTURE_TARGET_MAGIC) &&
                  strstr(capture_text, "face_row_fnv1a32=") &&
                  strstr(capture_text, "referenced_vertex_rows_fnv1a32=") &&
                  strstr(capture_text, "normal_row_fnv1a32=") &&
                  strstr(capture_text, "original_saturn_capture_required=1") &&
                  strstr(capture_text, "no_draw_only=1"),
                  "direct face bridge retains package rows and remains capture-only");
            memset(&manifest_receipt, 0, sizeof(manifest_receipt));
            CHECK(nexus_v1_engine_consume_structure1f_direct_face_capture_manifest(
                      &engine, source_entry, capture_text, capture_size,
                      &manifest_receipt) == 1 &&
                  manifest_receipt.status ==
                      NEXUS_V1_STRUCTURE1F_DIRECT_FACE_CAPTURE_MANIFEST_ACCEPTED_NO_DRAW &&
                  manifest_receipt.package_bytes_bound &&
                  manifest_receipt.manifest_target_bound &&
                  manifest_receipt.owner_geometry_bound &&
                  manifest_receipt.transform_selectors_bound &&
                  manifest_receipt.original_saturn_capture_required &&
                  manifest_receipt.no_draw_only &&
                  !manifest_receipt.fallback_visuals_permitted &&
                  manifest_receipt.blocks_real_dgn_mesh_render,
                  "package boundary accepts only its exact no-draw direct-face manifest");
            memset(&absent_raw_capture, 0, sizeof(absent_raw_capture));
            memset(&raw_receipt, 0, sizeof(raw_receipt));
            CHECK(nexus_v1_engine_bind_structure1f_direct_face_raw_capture(
                      &engine, source_entry, capture_text, capture_size,
                      &absent_raw_capture, &raw_receipt) == 0 &&
                  raw_receipt.status ==
                      NEXUS_V1_STRUCTURE1F_DIRECT_FACE_RAW_CAPTURE_BLOCKED_CAPTURE &&
                  raw_receipt.direct_face.status ==
                      NEXUS_V1_STRUCTURE1F_DIRECT_FACE_CAPTURE_MANIFEST_ACCEPTED_NO_DRAW &&
                  !raw_receipt.raw_capture_authenticated &&
                  !raw_receipt.direct_face_candidate_bound &&
                  raw_receipt.no_draw_only &&
                  !raw_receipt.fallback_visuals_permitted &&
                  raw_receipt.blocks_real_dgn_mesh_render,
                  "direct face refuses absent Saturn capture lanes without drawing");
            memset(&material_receipt, 0, sizeof(material_receipt));
            CHECK(nexus_v1_engine_bind_structure1f_vdp1_material_capture(
                      &engine, source_entry, capture_text, capture_size,
                      &absent_raw_capture, &material_receipt) == 0 &&
                  material_receipt.status ==
                      NEXUS_V1_STRUCTURE1F_VDP1_MATERIAL_BLOCKED_DIRECT_CAPTURE &&
                  material_receipt.direct_capture.status ==
                      NEXUS_V1_STRUCTURE1F_DIRECT_FACE_RAW_CAPTURE_BLOCKED_CAPTURE &&
                  !material_receipt.runtime_lanes_match_authenticated_capture &&
                  !material_receipt.texture_command_vram_bound &&
                  !material_receipt.palette_lane_bound &&
                  !material_receipt.pixel_decode_proven &&
                  !material_receipt.palette_decode_proven &&
                  !material_receipt.decoder_permitted &&
                  material_receipt.no_draw_only &&
                  !material_receipt.fallback_visuals_permitted &&
                  material_receipt.blocks_real_dgn_mesh_render,
                  "direct face material route blocks without authentic capture lanes");
            level_value = strstr(capture_text, "level_index=");
            CHECK(level_value != NULL, "direct face manifest retains its level identity");
            if (level_value) {
                level_value += strlen("level_index=");
                *level_value = 'x';
                memset(&manifest_receipt, 0, sizeof(manifest_receipt));
                CHECK(nexus_v1_engine_consume_structure1f_direct_face_capture_manifest(
                          &engine, source_entry, capture_text, capture_size,
                          &manifest_receipt) == 0 &&
                      manifest_receipt.status ==
                          NEXUS_V1_STRUCTURE1F_DIRECT_FACE_CAPTURE_MANIFEST_BLOCKED_TARGET_MISMATCH &&
                      manifest_receipt.package_bytes_bound &&
                      !manifest_receipt.manifest_target_bound &&
                      manifest_receipt.no_draw_only &&
                      manifest_receipt.blocks_real_dgn_mesh_render,
                      "package boundary rejects a changed owner manifest without enabling draw");
            }
            remove(capture_path);
            break;
        }
        CHECK(transform_found,
              "canonical LEV01 exposes at least one source-only transform capture target");
        CHECK(adjacency_transform_found,
              "canonical LEV01 exposes an exact no-draw Structure1F/2/3 adjacency receipt");
        for (source_entry = 0;
             source_entry < engine.current_level.structure1f_entry_count;
             ++source_entry) {
            Nexus_V1_DgnStructure1FTransformCaptureTarget target;
            Nexus_V1_DgnStructure1FTransformTraceAdmissionReceipt trace;
            const uint8_t raw_trace[] = { 0x53U, 0x48U, 0x32U, 0x2dU, 0x54U };
            const uint8_t transform_state[] = { 0x10U, 0x00U, 0x20U, 0x00U };
            char manifest[2048];

            memset(&target, 0, sizeof(target));
            if (nexus_v1_engine_build_structure1f_transform_capture_target(
                    &engine, source_entry, &target) != 1) {
                continue;
            }
            snprintf(manifest, sizeof(manifest),
                     "magic=%s\nproducer=saturn-debugger\n"
                     "trace_sha256=0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\n"
                     "level_index=%x\nstructure1f_entry_index=%x\n"
                     "structure1a_index=%x\nstructure3_model_index=%x\n"
                     "face_ordinal=%x\nz_rotation=%x\n"
                     "structure1a_table_offset=%x\nstructure1a_table_bytes=%x\n"
                     "structure1a_table_fnv1a64=%016llx\n"
                     "selector_column_fnv1a64=%016llx\n"
                     "raw_trace_fnv1a64=%016llx\n"
                     "transform_state_fnv1a64=%016llx\n",
                     NEXUS_V1_STRUCTURE1F_TRANSFORM_TRACE_MAGIC,
                     target.geometry.direct_mesh.level_index,
                     target.geometry.direct_mesh.structure1f_entry_index,
                     target.geometry.direct_mesh.structure1a_index,
                     target.geometry.direct_mesh.structure3_model_index,
                     target.geometry.direct_mesh.face_ordinal,
                     target.geometry.direct_mesh.z_rotation,
                     target.transform_table.table_byte_offset,
                     target.transform_table.table_byte_count,
                     (unsigned long long)target.transform_table.raw_table_fnv1a64,
                     (unsigned long long)target.transform_table.selector_column_fnv1a64,
                     (unsigned long long)fnv1a64(raw_trace, sizeof(raw_trace)),
                     (unsigned long long)fnv1a64(transform_state,
                                                 sizeof(transform_state)));
            memset(&trace, 0, sizeof(trace));
            CHECK(nexus_v1_engine_admit_structure1f_transform_capture_trace(
                      &engine, source_entry, manifest, strlen(manifest), raw_trace,
                      sizeof(raw_trace), transform_state, sizeof(transform_state), 0,
                      &trace) == 0 &&
                  trace.status ==
                      NEXUS_V1_STRUCTURE1F_TRANSFORM_TRACE_BLOCKED_PROVENANCE &&
                  trace.capture_target_bound && trace.manifest_target_bound &&
                  trace.raw_trace_bytes_bound && trace.transform_state_bytes_bound &&
                  !trace.transform_semantics_proven && trace.no_draw_only &&
                  !trace.fallback_visuals_permitted &&
                  trace.blocks_real_dgn_mesh_render,
                  "source-bound transform trace remains blocked without Saturn provenance");
            {
                Nexus_V1_DgnStructure1FTransformTracePaths paths;
                Nexus_V1_DgnStructure1FTransformTraceFileIntakeReceipt intake;
                char capture_target_path[128];
                char manifest_path[128];
                char raw_trace_path[128];
                char transform_state_path[128];
                char attestation_path[128];
                char attestation_text[2048];
                FILE *sidecar;

                snprintf(capture_target_path, sizeof(capture_target_path),
                         "/tmp/firestaff-nexus-direct-face-target-%ld.txt",
                         (long)getpid());
                snprintf(manifest_path, sizeof(manifest_path),
                         "/tmp/firestaff-nexus-transform-manifest-%ld.txt",
                         (long)getpid());
                snprintf(raw_trace_path, sizeof(raw_trace_path),
                         "/tmp/firestaff-nexus-transform-trace-%ld.bin",
                         (long)getpid());
                snprintf(transform_state_path, sizeof(transform_state_path),
                         "/tmp/firestaff-nexus-transform-state-%ld.bin",
                         (long)getpid());
                snprintf(attestation_path, sizeof(attestation_path),
                         "/tmp/firestaff-nexus-transform-attestation-%ld.txt",
                         (long)getpid());
                snprintf(attestation_text, sizeof(attestation_text),
                         "magic=%s\nreviewer=independent-saturn-review\n"
                         "attestation_sha256=0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\n"
                         "level_index=%x\nstructure1f_entry_index=%x\n"
                         "structure1a_index=%x\nstructure3_model_index=%x\n"
                         "face_ordinal=%x\nz_rotation=%x\n"
                         "structure1a_table_fnv1a64=%016llx\n"
                         "selector_column_fnv1a64=%016llx\n"
                         "raw_trace_fnv1a64=%016llx\n"
                         "transform_state_fnv1a64=%016llx\n"
                         "original_saturn_source_attested=1\n",
                         NEXUS_V1_STRUCTURE1F_TRANSFORM_ATTESTATION_MAGIC,
                         target.geometry.direct_mesh.level_index,
                         target.geometry.direct_mesh.structure1f_entry_index,
                         target.geometry.direct_mesh.structure1a_index,
                         target.geometry.direct_mesh.structure3_model_index,
                         target.geometry.direct_mesh.face_ordinal,
                         target.geometry.direct_mesh.z_rotation,
                         (unsigned long long)target.transform_table.raw_table_fnv1a64,
                         (unsigned long long)target.transform_table.selector_column_fnv1a64,
                         (unsigned long long)fnv1a64(raw_trace, sizeof(raw_trace)),
                         (unsigned long long)fnv1a64(transform_state,
                                                     sizeof(transform_state)));
                CHECK(nexus_v1_engine_write_structure1f_direct_face_capture_target(
                          &engine, source_entry, capture_target_path, &target) == 1,
                      "direct face target sidecar writes from the loaded package");
                sidecar = fopen(manifest_path, "wb");
                CHECK(sidecar != NULL &&
                      fwrite(manifest, 1U, strlen(manifest), sidecar) ==
                          strlen(manifest),
                      "transform manifest sidecar writes");
                if (sidecar) fclose(sidecar);
                sidecar = fopen(raw_trace_path, "wb");
                CHECK(sidecar != NULL &&
                      fwrite(raw_trace, 1U, sizeof(raw_trace), sidecar) ==
                          sizeof(raw_trace),
                      "transform raw-trace sidecar writes");
                if (sidecar) fclose(sidecar);
                sidecar = fopen(transform_state_path, "wb");
                CHECK(sidecar != NULL &&
                      fwrite(transform_state, 1U, sizeof(transform_state), sidecar) ==
                          sizeof(transform_state),
                      "transform-state sidecar writes");
                if (sidecar) fclose(sidecar);
                sidecar = fopen(attestation_path, "wb");
                CHECK(sidecar != NULL &&
                      fwrite(attestation_text, 1U, strlen(attestation_text),
                             sidecar) == strlen(attestation_text),
                      "transform attestation sidecar writes");
                if (sidecar) fclose(sidecar);
                memset(&paths, 0, sizeof(paths));
                paths.capture_target_path = capture_target_path;
                paths.manifest_path = manifest_path;
                paths.raw_trace_path = raw_trace_path;
                paths.transform_state_path = transform_state_path;
                paths.attestation_path = attestation_path;
                memset(&intake, 0, sizeof(intake));
                CHECK(nexus_v1_engine_ingest_structure1f_transform_capture_trace(
                          &engine, source_entry, &paths, &intake) == 1 &&
                      intake.sidecar_paths_distinct &&
                      intake.capture_target_bytes_read && intake.manifest_bytes_read &&
                      intake.raw_trace_bytes_read &&
                      intake.transform_state_bytes_read && intake.attestation_bytes_read &&
                      intake.capture_target.status ==
                          NEXUS_V1_STRUCTURE1F_DIRECT_FACE_CAPTURE_MANIFEST_ACCEPTED_NO_DRAW &&
                      intake.capture_target.package_bytes_bound &&
                      intake.capture_target.manifest_target_bound &&
                      intake.capture_target.owner_geometry_bound &&
                      intake.capture_target.transform_selectors_bound &&
                      intake.attestation.status ==
                          NEXUS_V1_STRUCTURE1F_TRANSFORM_ATTESTATION_ADMITTED_OPAQUE &&
                      intake.attestation.capture_target_bound &&
                      intake.attestation.manifest_target_bound &&
                      intake.attestation.raw_trace_bound &&
                      intake.attestation.transform_state_bound &&
                      intake.attestation.original_saturn_source_attested &&
                      intake.admission.status ==
                          NEXUS_V1_STRUCTURE1F_TRANSFORM_TRACE_ADMITTED_OPAQUE &&
                      intake.admission.capture_target_bound &&
                      intake.admission.manifest_target_bound &&
                      intake.admission.raw_trace_bytes_bound &&
                      intake.admission.transform_state_bytes_bound &&
                      !intake.admission.transform_semantics_proven &&
                      intake.no_draw_only && !intake.fallback_visuals_permitted &&
                      intake.blocks_real_dgn_mesh_render,
                      "attested transform sidecars require the exact package target and remain no-draw");
                sidecar = fopen(capture_target_path, "wb");
                CHECK(sidecar != NULL &&
                      fwrite("changed\n", 1U, sizeof("changed\n") - 1U, sidecar) ==
                          sizeof("changed\n") - 1U,
                      "changed direct face target sidecar writes");
                if (sidecar) fclose(sidecar);
                memset(&intake, 0, sizeof(intake));
                CHECK(nexus_v1_engine_ingest_structure1f_transform_capture_trace(
                          &engine, source_entry, &paths, &intake) == 0 &&
                      intake.capture_target_bytes_read &&
                      intake.capture_target.status ==
                          NEXUS_V1_STRUCTURE1F_DIRECT_FACE_CAPTURE_MANIFEST_BLOCKED_MALFORMED &&
                      intake.attestation.original_saturn_source_attested &&
                      intake.admission.status ==
                          NEXUS_V1_STRUCTURE1F_TRANSFORM_TRACE_MISSING &&
                      intake.no_draw_only && intake.blocks_real_dgn_mesh_render,
                      "trace route fails closed when its exact package target changes");
                remove(capture_target_path);
                remove(manifest_path);
                remove(raw_trace_path);
                remove(transform_state_path);
                remove(attestation_path);
            }
            break;
        }
        for (source_entry = 0;
             source_entry < engine.current_level.structure1f_entry_count;
             ++source_entry) {
            Nexus_V1_DgnStructure1FDirectStaticMaterialCaptureTarget target;

            memset(&target, 0, sizeof(target));
            if (nexus_v1_engine_build_structure1f_direct_static_material_capture_target(
                    &engine, source_entry, &target) != 1) {
                continue;
            }
            found = 1;
            CHECK(target.valid && target.direct_face_material_bound &&
                  target.direct_mesh.structure1f_entry_index == source_entry &&
                  target.static_material.structure3_entry_index ==
                      target.direct_mesh.structure3_model_index &&
                  target.static_material.face_ordinal ==
                      target.direct_mesh.face_ordinal &&
                  target.static_material.static_selector_descriptor_bound &&
                  target.static_material.image_payload_anchor_bound &&
                  target.static_material.image_payload_interval_bound &&
                  target.capture_producer_required &&
                  target.original_saturn_capture_required &&
                  target.no_draw_only && !target.fallback_visuals_permitted &&
                  target.blocks_real_dgn_mesh_render,
                  "direct owner selects only its exact static source material lane");
            break;
        }
        CHECK(!found,
              "canonical LEV01 direct owners cannot invent a static material lane");
        for (source_entry = 0;
             source_entry < engine.current_level.structure1f_entry_count;
             ++source_entry) {
            Nexus_V1_DgnStructure1FDirectUntexturedFaceCaptureTarget target;

            memset(&target, 0, sizeof(target));
            if (nexus_v1_engine_build_structure1f_direct_untextured_face_capture_target(
                    &engine, source_entry, &target) != 1) {
                continue;
            }
            untextured_found = 1;
            CHECK(target.valid && target.direct_face_untextured_bound &&
                  target.direct_mesh.structure1f_entry_index == source_entry &&
                  target.untextured_face.structure3_entry_index ==
                      target.direct_mesh.structure3_model_index &&
                  target.untextured_face.face_ordinal ==
                      target.direct_mesh.face_ordinal &&
                  target.untextured_face.raw_fill_bound &&
                  !target.untextured_face.flat_fill_semantics_proven &&
                  !target.untextured_face.transform_semantics_proven &&
                  !target.untextured_face.pixel_palette_vdp1_semantics_proven &&
                  !target.untextured_face.decoder_permitted &&
                  target.capture_producer_required &&
                  target.original_saturn_capture_required && target.no_draw_only &&
                  !target.fallback_visuals_permitted &&
                  target.blocks_real_dgn_mesh_render,
                  "direct owner retains only its exact opaque non-textured face");
            break;
        }
        CHECK(!untextured_found,
              "canonical LEV01 direct owners cannot invent an untextured face route");
        for (source_entry = 0;
             source_entry < engine.current_level.structure1f_entry_count;
             ++source_entry) {
            Nexus_V1_DgnStructure1FDirectAnimatedMaterialCaptureTarget target;

            memset(&target, 0, sizeof(target));
            if (nexus_v1_engine_build_structure1f_direct_animated_material_capture_target(
                    &engine, source_entry, &target) != 1) {
                continue;
            }
            animated_found = 1;
            CHECK(target.valid && target.direct_face_animated_material_bound &&
                  target.direct_mesh.structure1f_entry_index == source_entry &&
                  target.animated_material.structure3_entry_index ==
                      target.direct_mesh.structure3_model_index &&
                  target.animated_material.face_ordinal ==
                      target.direct_mesh.face_ordinal &&
                  target.animated_material.animation_declaration_bound &&
                  !target.animated_material.animation_execution_permitted &&
                  !target.animated_material.pixel_palette_vdp1_semantics_proven &&
                  !target.animated_material.decoder_permitted &&
                  target.capture_producer_required &&
                  target.original_saturn_capture_required && target.no_draw_only &&
                  !target.fallback_visuals_permitted &&
                  target.blocks_real_dgn_mesh_render,
                  "direct owner retains only its exact 08xx material declaration");
            break;
        }
        CHECK(!animated_found,
              "canonical LEV01 direct owners cannot invent an 08xx material route");
    }
    {
        Nexus_V1_DgnStructure1FTransformCaptureTarget transform_target;
        Nexus_V1_DgnStructure1FDirectStaticMaterialCaptureTarget static_target;
        Nexus_V1_DgnStructure1FDirectUntexturedFaceCaptureTarget untextured_target;
        Nexus_V1_DgnStructure1FDirectAnimatedMaterialCaptureTarget animated_target;
        memset(&transform_target, 0, sizeof(transform_target));
        CHECK(nexus_v1_engine_build_structure1f_transform_capture_target(
                  &engine, engine.current_level.structure1f_entry_count,
                  &transform_target) == 0 && !transform_target.valid &&
              transform_target.no_draw_only &&
              !transform_target.fallback_visuals_permitted &&
              transform_target.blocks_real_dgn_mesh_render,
              "out-of-range owner cannot manufacture transform semantics or fallback");
        memset(&static_target, 0, sizeof(static_target));
        CHECK(nexus_v1_engine_build_structure1f_direct_static_material_capture_target(
                  &engine, engine.current_level.structure1f_entry_count,
                  &static_target) == 0 && !static_target.valid &&
              static_target.no_draw_only && !static_target.fallback_visuals_permitted &&
              static_target.blocks_real_dgn_mesh_render,
              "out-of-range owner cannot manufacture a material or fallback");
        memset(&untextured_target, 0, sizeof(untextured_target));
        CHECK(nexus_v1_engine_build_structure1f_direct_untextured_face_capture_target(
                  &engine, engine.current_level.structure1f_entry_count,
                  &untextured_target) == 0 && !untextured_target.valid &&
              untextured_target.no_draw_only &&
              !untextured_target.fallback_visuals_permitted &&
              untextured_target.blocks_real_dgn_mesh_render,
              "out-of-range owner cannot manufacture an untextured face or fallback");
        memset(&animated_target, 0, sizeof(animated_target));
        CHECK(nexus_v1_engine_build_structure1f_direct_animated_material_capture_target(
                  &engine, engine.current_level.structure1f_entry_count,
                  &animated_target) == 0 && !animated_target.valid &&
              animated_target.no_draw_only &&
              !animated_target.fallback_visuals_permitted &&
              animated_target.blocks_real_dgn_mesh_render,
              "out-of-range owner cannot manufacture animation or fallback");
    }
    free(data);
    if (failures != 0) {
        fprintf(stderr, "test_nexus_v1_direct_static_material_capture: FAIL %d\n",
                failures);
        return 1;
    }
    puts("test_nexus_v1_direct_static_material_capture: PASS");
    return 0;
}
