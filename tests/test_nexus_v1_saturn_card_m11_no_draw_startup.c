#include "nexus_v1_launcher.h"
#include "nexus_v1_saturn_save_capture.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int failures;

static uint64_t fnv1a64(const uint8_t *bytes, size_t byte_count)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t index;

    for (index = 0U; index < byte_count; ++index) {
        hash ^= bytes[index];
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

static void check(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static void build_engine(Nexus_V1_Engine *engine,
                         Nexus_V1_BpkRuntimeUploadRow *row)
{
    memset(engine, 0, sizeof(*engine));
    memset(row, 0, sizeof(*row));
    engine->menu_bpk_package_fnv1a64 = UINT64_C(0x1122334455667788);
    engine->menu_bpk_upload_receipt_valid = 1;
    engine->menu_bpk_upload_row_count = 1;
    engine->menu_bpk_upload_receipt.first_prs3_entry_index = 3U;
    engine->menu_bpk_upload_receipt.first_prs3_payload_offset = 64U;
    engine->menu_bpk_upload_receipt.first_prs3_payload_size = 20U;
    engine->menu_bpk_upload_receipt.first_prs3_payload_fnv1a64 =
        UINT64_C(0x8877665544332211);
    engine->menu_bpk_upload_receipt.first_prs3_version =
        NEXUS_V1_BPK_PRS3_VERSION;
    engine->menu_bpk_upload_receipt.first_prs3_pixel_count = 4U;
    engine->menu_bpk_upload_receipt.first_prs3_header_first_u32 = 12U;
    row->entry_index = 3U;
    row->payload_offset = 64U;
    row->payload_size = 20U;
    row->payload_fnv1a64 = UINT64_C(0x8877665544332211);
    row->stream_offset = 72U;
    row->stream_size = 12U;
    row->header_first_u32 = 12U;
    row->prs3_version = NEXUS_V1_BPK_PRS3_VERSION;
    row->prs3_pixel_count = 4U;
    row->mode = NEXUS_V1_BPK_MODE_8BPP;
    row->expected_output_bytes = 4U;
    row->prs3_header_valid = 1;
    row->compression.valid = 1;
    row->compression.entry_index = row->entry_index;
    row->compression.mode_flags = row->mode;
    row->compression.declared_pixel_count = row->prs3_pixel_count;
    row->compression.declared_output_bytes = row->expected_output_bytes;
    row->compression.compressed_offset = 76U;
    row->compression.compressed_length = 8U;
    row->compression.compressed_fnv1a64 = UINT64_C(0x1);
    engine->menu_bpk_upload_receipt.first_prs3_compression = row->compression;
    row->decode_blocked = 1;
    row->evidence_only = 1;
    row->renderer_handoff_blocked = 1;
    row->upload_blocked = 1;
    engine->menu_bpk_upload_rows[0] = *row;
}

static void build_level_aux(Nexus_V1_Engine *engine,
                            Nexus_V1_SlevSalAssetDiscoveryReceipt *assets)
{
    Nexus_V1_SlevSalDirectLevelIdentity *level = &assets->levels[4];
    const char *slev = nexus_v1_known_file_md5("SLEV04.BIN");
    const char *sal = nexus_v1_known_file_md5("SNDLEV04.SAL");
    const char *map = nexus_v1_known_file_md5("SNDLEV04.MAP");
    const char *driver = nexus_v1_known_file_md5("SDDRVS.TSK");

    memset(assets, 0, sizeof(*assets));
    assets->valid = 1;
    assets->direct_files_only = 1;
    assets->corpus_fnv1a64 = UINT64_C(0x88);
    level->valid = 1;
    level->level_index = 4U;
    level->slev.valid = level->sal.valid = level->map.valid = 1;
    snprintf(level->slev.canonical_name, sizeof(level->slev.canonical_name),
             "SLEV04.BIN");
    snprintf(level->sal.canonical_name, sizeof(level->sal.canonical_name),
             "SNDLEV04.SAL");
    snprintf(level->map.canonical_name, sizeof(level->map.canonical_name),
             "SNDLEV04.MAP");
    snprintf(level->slev.md5, sizeof(level->slev.md5), "%s", slev);
    snprintf(level->sal.md5, sizeof(level->sal.md5), "%s", sal);
    snprintf(level->map.md5, sizeof(level->map.md5), "%s", map);
    level->slev.byte_count = 256U;
    level->slev.fnv1a64 = UINT64_C(0x445566778899aabb);
    assets->sound_driver.valid = 1;
    snprintf(assets->sound_driver.canonical_name,
             sizeof(assets->sound_driver.canonical_name), "SDDRVS.TSK");
    snprintf(assets->sound_driver.md5, sizeof(assets->sound_driver.md5), "%s",
             driver);
    assets->sound_driver.byte_count = 512U;
    assets->sound_driver.fnv1a64 = UINT64_C(0x0102030405060708);
    engine->level_loaded = 1;
    engine->game.current_level = 4;
    engine->level_aux_runtime_receipt.level_index = 4;
    engine->level_aux_runtime_receipt.canonical_pair_bound = 1;
    engine->level_aux_runtime_receipt.slev.canonical_hash_verified = 1;
    engine->level_aux_runtime_receipt.slev.exact_source_entry_observed = 1;
    engine->level_aux_runtime_receipt.sal.canonical_hash_verified = 1;
    engine->level_aux_runtime_receipt.sal.exact_source_entry_observed = 1;
    engine->level_aux_runtime_receipt.map.canonical_hash_verified = 1;
    engine->level_aux_runtime_receipt.map.exact_source_entry_observed = 1;
    engine->level_aux_runtime_receipt.sound_driver.canonical_hash_verified = 1;
    engine->level_aux_runtime_receipt.sound_driver.exact_source_entry_observed = 1;
    snprintf(engine->level_aux_runtime_receipt.slev.canonical_name, 16, "SLEV04.BIN");
    snprintf(engine->level_aux_runtime_receipt.sal.canonical_name, 16, "SNDLEV04.SAL");
    snprintf(engine->level_aux_runtime_receipt.map.canonical_name, 16, "SNDLEV04.MAP");
    snprintf(engine->level_aux_runtime_receipt.sound_driver.canonical_name, 16,
             "SDDRVS.TSK");
    snprintf(engine->level_aux_runtime_receipt.slev.canonical_md5, 33, "%s", slev);
    snprintf(engine->level_aux_runtime_receipt.sal.canonical_md5, 33, "%s", sal);
    snprintf(engine->level_aux_runtime_receipt.map.canonical_md5, 33, "%s", map);
    snprintf(engine->level_aux_runtime_receipt.sound_driver.canonical_md5, 33, "%s",
             driver);
    engine->script_runtime_receipt.level_index = 4;
    engine->script_runtime_receipt.status = NEXUS_SCRIPT_RUNTIME_BLOCKED_UNSUPPORTED_FORMAT;
    engine->script_runtime_receipt.real_task_profile_supported = 1;
    engine->script_runtime_receipt.real_task_header_supported = 1;
    engine->script_runtime_receipt.blocks_real_script_dispatch = 1;
    engine->sfx_runtime_receipt.level_index = 4;
    engine->sfx_runtime_receipt.status = NEXUS_SFX_RUNTIME_BLOCKED_UNSUPPORTED_DECODE;
    engine->sfx_runtime_receipt.sal_loaded = engine->sfx_runtime_receipt.map_loaded = 1;
    engine->sfx_runtime_receipt.sal_canonical_source_verified = 1;
    engine->sfx_runtime_receipt.map_canonical_source_verified = 1;
    engine->sfx_runtime_receipt.sound_driver_canonical_source_verified = 1;
    engine->sfx_runtime_receipt.blocks_real_sfx_playback = 1;
}

int main(void)
{
    Nexus_V1_Engine engine;
    Nexus_V1_BpkRuntimeUploadRow row;
    Nexus_V1_SaturnSaveCaptureReceipt card;
    Nexus_V1_LauncherMenuBpkNoDrawPresentationReceipt presentation;
    Nexus_V1_LauncherM11MenuBpkNoDrawHostReceipt host;
    Nexus_V1_LauncherSaturnCardM11NoDrawStartupReceipt startup;
    Nexus_V1_SlevSalAssetDiscoveryReceipt assets;
    Nexus_V1_LauncherM11SlevSalNoDrawReceipt aux;
    Nexus_V1_SlevTaskBodyCapturePlan task_plan;
    Nexus_V1_LauncherM11SlevTaskBodyNoDispatchReceipt task_body;
    Nexus_V1_SalContainerProvenanceReceipt sal_container;
    Nexus_V1_SndlevMapProvenanceReceipt map_table;
    Nexus_V1_LauncherM11SlevTaskSalNoOpStartupReceipt task_sal;
    Nexus_V1_LauncherSlevTaskSalCaptureBinding task_sal_binding;
    Nexus_V1_LauncherM11SlevSalCaptureImportReceipt script_capture;
    Nexus_V1_LauncherM11SlevSalLocalArtifactReceipt script_capture_preflight;
    Nexus_V1_LauncherM12M11SlevSalCaptureRouteReceipt script_capture_route;
    Nexus_V1_LauncherM12M11SlevSalCaptureRouteReceipt script_capture_resume;
    uint8_t sal_bytes[] = { 'd','s','p','0','1','.','E','X', 1,2,3,4,5,6,7,8 };
    uint8_t map_bytes[10] = { 0 };
    uint8_t capture_bytes[NEXUS_V1_M11_SLEV_SAL_CAPTURE_HEADER_BYTES + 12U];
    static const uint8_t opaque_capture_payload[12] = {
        0x42U, 0xd1U, 0x06U, 0x9aU, 0x35U, 0xe8U,
        0x71U, 0x0cU, 0xbfU, 0x24U, 0x5dU, 0x90U
    };
    uint64_t package;

    build_engine(&engine, &row);
    package = engine.menu_bpk_package_fnv1a64;
    memset(&card, 0, sizeof(card));
    card.valid = 1;
    card.status = NEXUS_V1_SATURN_SAVE_CAPTURE_ADMITTED_OPAQUE;
    card.opaque_only = 1;
    card.title_route_bound = 1;
    card.champion_route_bound = 1;
    card.image_bytes = NEXUS_V1_SATURN_SAVE_IMAGE_BYTES;
    card.image_fnv1a64 = UINT64_C(0x1234);
    check(nexus_v1_engine_set_saturn_save_capture_receipt(&engine, 7U, &card),
          "opaque direct card is admitted for the active route");
    check(nexus_v1_launcher_menu_bpk_no_draw_presentation_receipt(
              &engine.menu_bpk_upload_receipt, &row, &presentation) &&
              nexus_v1_launcher_admit_m11_menu_bpk_no_draw_host(
                  &engine, 7U, package, &presentation, &host),
          "matching M11 no-draw host receipt is admitted");
    check(nexus_v1_launcher_bind_saturn_card_m11_no_draw_startup(
              &engine, 7U, card.image_fnv1a64, package, 1, &startup) &&
              startup.valid && startup.opaque_saturn_card_only &&
              startup.no_draw_only && startup.draw_disabled,
          "same package and epoch atomically bind card and M11 evidence");
    ++engine.menu_bpk_upload_rows[0].compression.compressed_fnv1a64;
    check(!nexus_v1_launcher_bind_saturn_card_m11_no_draw_startup(
               &engine, 7U, card.image_fnv1a64, package, 1, &startup) &&
              !startup.valid,
          "stale PRS3 body row rejects card-bound launcher reuse");
    --engine.menu_bpk_upload_rows[0].compression.compressed_fnv1a64;
    build_level_aux(&engine, &assets);
    check(nexus_v1_launcher_admit_m11_slev_sal_no_draw(
              &engine, &assets, 7U, package, card.image_fnv1a64, 1, 4U, &aux) &&
              aux.valid && aux.no_draw_only && aux.blocks_real_script_dispatch &&
              aux.blocks_real_sfx_playback && aux.level_aux.no_runtime_only,
          "active direct SLEV/SAL identities bind to M11 only as no-runtime evidence");
    memset(&task_plan, 0, sizeof(task_plan));
    task_plan.valid = 1;
    task_plan.no_dispatch_only = 1;
    task_plan.targets[4].valid = 1;
    task_plan.targets[4].level_index = 4U;
    task_plan.targets[4].source_fnv1a64 = assets.levels[4].slev.fnv1a64;
    task_plan.targets[4].entry_pc = 0x1000U;
    task_plan.targets[4].task_body_pc = 0x1010U;
    task_plan.targets[4].task_body_opcode = 0x33U;
    task_plan.targets[4].callback_or_write_pc = 0x1020U;
    task_plan.targets[4].raw_trace_fnv1a64 = UINT64_C(0xaabbccddeeff0011);
    task_plan.targets[4].raw_trace_byte_count = 128U;
    task_plan.targets[4].source_order_required = 1;
    task_plan.targets[4].original_saturn_trace_required = 1;
    task_plan.targets[4].no_dispatch_only = 1;
    check(nexus_v1_launcher_admit_m11_slev_task_body_no_dispatch(
              &engine, &assets, &task_plan, 7U, package, card.image_fnv1a64,
              1, 4U, &task_body) && task_body.valid && task_body.no_draw_only &&
              task_body.blocks_real_script_dispatch &&
              task_body.blocks_real_sfx_playback &&
              task_body.slev.fnv1a64 == task_body.task_body.source_fnv1a64,
          "M11 binds a source-ordered original SLEV task-body target only to the exact card/package/epoch startup route");
    map_bytes[0] = 0x20U;
    map_bytes[8] = 0xffU;
    map_bytes[9] = 0xffU;
    assets.levels[4].sal.byte_count = sizeof(sal_bytes);
    assets.levels[4].sal.fnv1a64 = fnv1a64(sal_bytes, sizeof(sal_bytes));
    assets.levels[4].map.byte_count = sizeof(map_bytes);
    assets.levels[4].map.fnv1a64 = fnv1a64(map_bytes, sizeof(map_bytes));
    check(nexus_v1_sal_container_provenance_parse(
              sal_bytes, sizeof(sal_bytes), assets.levels[4].sal.fnv1a64,
              &sal_container) &&
          nexus_v1_sndlev_map_provenance_parse(
              map_bytes, sizeof(map_bytes), assets.levels[4].map.fnv1a64,
              &map_table),
          "SAL and MAP fixtures expose bounded opaque source receipts");
    memset(&task_sal_binding, 0, sizeof(task_sal_binding));
    task_sal_binding.original_saturn_trace_bound = 1;
    task_sal_binding.task_trace_fnv1a64 = task_body.task_body.raw_trace_fnv1a64;
    task_sal_binding.sal_descriptor_fnv1a64 = sal_container.descriptor_fnv1a64;
    task_sal_binding.map_table_fnv1a64 = map_table.table_fnv1a64;
    task_sal_binding.sound_driver_fnv1a64 = assets.sound_driver.fnv1a64;
    check(
          nexus_v1_launcher_admit_m11_slev_task_sal_no_op_startup(
              &engine, &assets, &task_body, &sal_container, &map_table,
              &task_sal_binding, 7U,
              package, card.image_fnv1a64, 1, 4U, &task_sal) && task_sal.valid &&
          task_sal.no_op_only && task_sal.commands_opaque && task_sal.audio_opaque &&
          task_sal.task_trace_fnv1a64 == task_body.task_body.raw_trace_fnv1a64 &&
          task_sal.sal_descriptor_fnv1a64 == sal_container.descriptor_fnv1a64 &&
          task_sal.map_table_fnv1a64 == map_table.table_fnv1a64 &&
          task_sal.sound_driver_fnv1a64 == assets.sound_driver.fnv1a64,
          "M11 binds exact opaque SLEV task, SAL container, MAP table and SDDRVS identities as a no-op startup route");
    memset(capture_bytes, 0, sizeof(capture_bytes));
    memcpy(capture_bytes, NEXUS_V1_M11_SLEV_SAL_CAPTURE_MAGIC, 8U);
    write_be32(capture_bytes, 8U, NEXUS_V1_M11_SLEV_SAL_CAPTURE_VERSION);
    write_be32(capture_bytes, 12U, NEXUS_V1_M11_SLEV_SAL_CAPTURE_HEADER_BYTES);
    write_be64(capture_bytes, 16U, task_sal.route_epoch);
    write_be64(capture_bytes, 24U, task_sal.package_fnv1a64);
    write_be64(capture_bytes, 32U, task_sal.card_fnv1a64);
    write_be64(capture_bytes, 40U, task_sal.task_trace_fnv1a64);
    write_be64(capture_bytes, 48U, task_sal.sal_descriptor_fnv1a64);
    write_be64(capture_bytes, 56U, task_sal.map_table_fnv1a64);
    write_be64(capture_bytes, 64U, task_sal.sound_driver_fnv1a64);
    write_be32(capture_bytes, 72U, NEXUS_V1_M11_SLEV_SAL_CAPTURE_HEADER_BYTES);
    write_be32(capture_bytes, 76U, sizeof(opaque_capture_payload));
    memcpy(capture_bytes + NEXUS_V1_M11_SLEV_SAL_CAPTURE_HEADER_BYTES,
           opaque_capture_payload, sizeof(opaque_capture_payload));
    write_be64(capture_bytes, 80U,
               fnv1a64(opaque_capture_payload, sizeof(opaque_capture_payload)));
    write_be64(capture_bytes, 88U, task_sal.task.task_body.source_fnv1a64);
    check(nexus_v1_launcher_verify_m11_slev_sal_local_artifact(
              &task_sal, capture_bytes, sizeof(capture_bytes),
              &script_capture_preflight) && script_capture_preflight.valid &&
          script_capture_preflight.route_bound && script_capture_preflight.task_bound &&
          script_capture_preflight.sal_bound && script_capture_preflight.map_bound &&
          script_capture_preflight.sddrvs_bound &&
          script_capture_preflight.payload_bounds_bound &&
          script_capture_preflight.payload_hash_bound &&
          script_capture_preflight.payload_opaque &&
          script_capture_preflight.no_draw_only && script_capture_preflight.no_op_only,
          "local NXSLSC01 preflight accepts only exact opaque route evidence");
    check(nexus_v1_launcher_import_m11_slev_sal_capture(
              &engine, &assets, &task_sal, capture_bytes, sizeof(capture_bytes),
              &script_capture) && script_capture.valid && script_capture.route_bound &&
          script_capture.header_version_bound && script_capture.payload_bounds_bound &&
          script_capture.payload_hash_bound && script_capture.no_op_only &&
          script_capture.commands_opaque && script_capture.audio_opaque &&
          !script_capture.dispatch_permitted && !script_capture.playback_permitted &&
          !script_capture.fallback_script_permitted,
          "M11 imports only a bounded original SLEV/SAL command-capture envelope as opaque no-op evidence");
    check(nexus_v1_launcher_admit_m12_m11_slev_sal_capture_required(
              &task_sal,
              "96e106f740ab448cf89f0dd49dfbac7fe5391cb6bd6e14ad5e3061c13330266f",
              1U,
              "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
              &script_capture_route) && script_capture_route.valid &&
          script_capture_route.capture_required && !script_capture_route.capture_imported &&
          !script_capture_route.resume_ready && script_capture_route.operator_only &&
          script_capture_route.no_draw_only && script_capture_route.no_op_only &&
          script_capture_route.commands_opaque && script_capture_route.audio_opaque &&
          !script_capture_route.dispatch_permitted &&
          !script_capture_route.playback_permitted,
          "M12/M11 exposes a hash-bound operator-only SLEV/SAL capture-required route");
    check(nexus_v1_launcher_resume_m12_m11_slev_sal_capture(
              &engine, &assets, &script_capture_route, capture_bytes,
              sizeof(capture_bytes), &script_capture_resume) &&
          script_capture_resume.valid && !script_capture_resume.capture_required &&
          script_capture_resume.capture_imported && script_capture_resume.resume_ready &&
          script_capture_resume.capture.valid && script_capture_resume.no_draw_only &&
          script_capture_resume.no_op_only && script_capture_resume.commands_opaque &&
          script_capture_resume.audio_opaque &&
          !script_capture_resume.dispatch_permitted &&
          !script_capture_resume.playback_permitted,
          "M12/M11 resumes only from a matching NXSLSC01 receipt while commands and audio stay opaque");
    script_capture_route.startup.map_table_fnv1a64 ^= UINT64_C(1);
    check(!nexus_v1_launcher_resume_m12_m11_slev_sal_capture(
              &engine, &assets, &script_capture_route, capture_bytes,
              sizeof(capture_bytes), &script_capture_resume) &&
          !script_capture_resume.valid && script_capture_resume.capture_required &&
          script_capture_resume.no_op_only,
          "M12/M11 rejects stale capture-route identities before resume");
    script_capture_route.startup.map_table_fnv1a64 ^= UINT64_C(1);
    capture_bytes[56U] ^= 1U;
    check(!nexus_v1_launcher_verify_m11_slev_sal_local_artifact(
              &task_sal, capture_bytes, sizeof(capture_bytes),
              &script_capture_preflight) && !script_capture_preflight.valid &&
          script_capture_preflight.no_draw_only && script_capture_preflight.no_op_only,
          "local NXSLSC01 preflight rejects a drifted MAP identity");
    check(!nexus_v1_launcher_import_m11_slev_sal_capture(
              &engine, &assets, &task_sal, capture_bytes, sizeof(capture_bytes),
              &script_capture) && !script_capture.valid && script_capture.no_op_only,
          "M11 rejects an external SLEV/SAL capture whose MAP-table identity drifts");
    capture_bytes[56U] ^= 1U;
    capture_bytes[NEXUS_V1_M11_SLEV_SAL_CAPTURE_HEADER_BYTES] ^= 1U;
    check(!nexus_v1_launcher_import_m11_slev_sal_capture(
              &engine, &assets, &task_sal, capture_bytes, sizeof(capture_bytes),
              &script_capture) && !script_capture.valid && script_capture.no_op_only,
          "M11 rejects an external SLEV/SAL capture whose opaque payload hash drifts");
    capture_bytes[NEXUS_V1_M11_SLEV_SAL_CAPTURE_HEADER_BYTES] ^= 1U;
    map_table.table_fnv1a64 ^= UINT64_C(1);
    check(!nexus_v1_launcher_admit_m11_slev_task_sal_no_op_startup(
              &engine, &assets, &task_body, &sal_container, &map_table,
              &task_sal_binding, 7U,
              package, card.image_fnv1a64, 1, 4U, &task_sal) && !task_sal.valid &&
          task_sal.no_op_only,
          "M11 rejects MAP-table FNV drift before opaque audio/script startup");
    map_table.table_fnv1a64 ^= UINT64_C(1);
    sal_container.descriptor_fnv1a64 ^= UINT64_C(1);
    check(!nexus_v1_launcher_admit_m11_slev_task_sal_no_op_startup(
              &engine, &assets, &task_body, &sal_container, &map_table,
              &task_sal_binding, 7U,
              package, card.image_fnv1a64, 1, 4U, &task_sal) && !task_sal.valid &&
          task_sal.no_op_only,
          "M11 rejects SAL descriptor FNV drift before opaque audio/script startup");
    sal_container.descriptor_fnv1a64 ^= UINT64_C(1);
    task_plan.targets[4].raw_trace_fnv1a64 = 0U;
    check(!nexus_v1_launcher_admit_m11_slev_task_body_no_dispatch(
               &engine, &assets, &task_plan, 7U, package, card.image_fnv1a64,
               1, 4U, &task_body) && !task_body.valid && task_body.no_draw_only,
          "M11 rejects an incomplete SLEV task-body trace before script dispatch");
    task_plan.targets[4].raw_trace_fnv1a64 = UINT64_C(0xaabbccddeeff0011);
    task_plan.targets[4].source_fnv1a64 ^= UINT64_C(1);
    check(!nexus_v1_launcher_admit_m11_slev_task_body_no_dispatch(
               &engine, &assets, &task_plan, 7U, package, card.image_fnv1a64,
               1, 4U, &task_body) && !task_body.valid && task_body.no_draw_only,
          "M11 rejects a task-body target whose direct SLEV source identity drifts");
    task_plan.targets[4].source_fnv1a64 = assets.levels[4].slev.fnv1a64;
    engine.game.current_level = 5;
    check(!nexus_v1_launcher_admit_m11_slev_sal_no_draw(
              &engine, &assets, 7U, package, card.image_fnv1a64, 1, 4U, &aux) &&
              !aux.valid,
          "level transition invalidates the active SLEV/SAL M11 route");
    engine.game.current_level = 4;
    assets.levels[4].sal.md5[0] = '0';
    check(!nexus_v1_launcher_admit_m11_slev_sal_no_draw(
              &engine, &assets, 7U, package, card.image_fnv1a64, 1, 4U, &aux) &&
              !aux.valid,
          "SLEV/SAL identity drift rejects the M11 no-runtime route");
    assets.levels[4].sal.md5[0] = nexus_v1_known_file_md5("SNDLEV04.SAL")[0];
    check(!nexus_v1_launcher_bind_saturn_card_m11_no_draw_startup(
               &engine, 7U, card.image_fnv1a64, package + 1U, 1, &startup) &&
              !startup.valid,
          "cross-package title/card binding rejects");
    check(!nexus_v1_launcher_bind_saturn_card_m11_no_draw_startup(
               &engine, 8U, card.image_fnv1a64, package, 1, &startup) &&
              !startup.valid,
          "stale M11/card epoch rejects before champion startup");
    check(!nexus_v1_launcher_bind_saturn_card_m11_no_draw_startup(
               &engine, 7U, card.image_fnv1a64 + 1U, package, 1, &startup) &&
              !startup.valid,
          "card identity drift rejects before champion startup");

    if (failures) {
        fprintf(stderr, "Nexus Saturn-card M11 no-draw startup: %d failure(s)\n",
                failures);
        return 1;
    }
    puts("Nexus Saturn-card M11 no-draw startup: PASS");
    return 0;
}
