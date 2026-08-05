/*
 * nexus_v1_launcher.c
 * ===================
 * Nexus V1 launcher — singleton engine lifecycle manager.
 *
 * Owns the Nexus_V1_Engine singleton for the duration of the session.
 * All other Nexus V1 modules are accessed through nexus_v1_engine.c
 * which uses the launcher's engine field.
 *
 * Source: DM Nexus (Saturn) boot flow, NEXUS.C / NEXUS2.C engine
 * lifecycle, ReDMCSB boot/disk loading references.
 */

#include "nexus_v1_launcher.h"
#include <string.h>

int nexus_v1_launcher_select_saturn_card_startup(
    Nexus_V1_Engine *engine, uint64_t route_epoch, uint64_t card_fnv1a64,
    Nexus_V1_LauncherSaturnCardStartupReceipt *out_receipt)
{
    Nexus_V1_LauncherSaturnCardStartupReceipt receipt;
    memset(&receipt, 0, sizeof(receipt));
    receipt.opaque_saturn_card_only = 1;
    if (!out_receipt) return 0;
    if (!nexus_v1_engine_saturn_save_capture_ready(engine, route_epoch,
                                                    card_fnv1a64)) {
        *out_receipt = receipt; return 0;
    }
    receipt.valid = 1;
    receipt.card_fnv1a64 = card_fnv1a64;
    receipt.route_epoch = route_epoch;
    *out_receipt = receipt;
    return 1;
}
int nexus_v1_launcher_bind_saturn_card_boot_route(Nexus_V1_Engine *e,uint64_t epoch,uint64_t card,uint64_t package,int direct,Nexus_V1_LauncherSaturnCardBootBinding*out){Nexus_V1_LauncherSaturnCardBootBinding r;memset(&r,0,sizeof(r));r.opaque_only=1;if(!out)return 0;if(!direct||!package||!nexus_v1_engine_saturn_save_capture_ready(e,epoch,card)){*out=r;return 0;}r.valid=1;r.card_fnv1a64=card;r.package_fnv1a64=package;r.route_epoch=epoch;*out=r;return 1;}
#include "nexus_v1_mechanics.h"
#include "nexus_v1_save.h"
#include "nexus_v1_viewport.h"
#include "nexus_v1_world.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static uint32_t nexus_v1_launcher_capture_be32(const uint8_t *bytes);
static uint64_t nexus_v1_launcher_capture_be64(const uint8_t *bytes);
static uint64_t nexus_v1_launcher_capture_fnv1a64(const uint8_t *bytes,
                                                   size_t byte_count);
static int nexus_v1_launcher_sha256_text_valid(const char *text);

/* ── Singleton ──────────────────────────────────────────────────────── */
static Nexus_V1_Engine s_engine;
static int s_initialized = 0;

static uint32_t nexus_v1_launcher_dgn_command_buffer_hash(
    const Nexus_V1_DgnRenderCommand *commands, int count)
{
    const uint8_t *bytes = (const uint8_t *)commands;
    uint32_t hash = 2166136261u;
    size_t byte_count;
    if (!commands || count <= 0 ||
        count > NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS) return 0u;
    byte_count = (size_t)count * sizeof(*commands);
    for (size_t i = 0u; i < byte_count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static uint64_t nexus_v1_launcher_dgn_bytes_fnv1a64(
    const uint8_t *data, int size)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    int index;

    if (!data || size <= 0) return 0U;
    for (index = 0; index < size; ++index) {
        hash ^= (uint64_t)data[index];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static int nexus_v1_launcher_has_real_dgn_admission(
    const Nexus_V1_Engine *engine)
{
    const Nexus_V1_DgnStructure3RuntimeSource *source;

    if (!engine) return 0;
    source = &engine->structure3_runtime_source;
    if (source->valid &&
        source->level_index == engine->game.current_level &&
        source->capture_bundle_hash_verified &&
        source->capture_trace_order_verified &&
        source->original_saturn_capture_verified) {
        return 1;
    }
    return 0;
}

/* ── Public API ─────────────────────────────────────────────────────── */

int nexus_v1_launcher_init(const char *data_dir) {
    if (!data_dir) {
        printf("Nexus launcher: NULL data_dir\n");
        return -1;
    }

    /* Already initialized — return success without re-init */
    if (s_initialized) {
        printf("Nexus launcher: already initialized\n");
        return 0;
    }

    /* Init the engine singleton.
     * nexus_v1_init() auto-detects ISO vs extracted files and
     * populates the full engine: ISO reader, game state, mechanics,
     * champions, creatures, sound, and font.
     * Source: nexus_v1_engine.c nexus_v1_init() */
    int rc = nexus_v1_init(&s_engine, data_dir);
    if (rc != 0) {
        printf("Nexus launcher: nexus_v1_init failed for '%s'\n", data_dir);
        return -1;
    }

    s_initialized = 1;
    printf("Nexus launcher: initialized (data_dir='%s')\n", data_dir);
    return 0;
}

int nexus_v1_launcher_load_level(int level) {
    if (!s_initialized) {
        printf("Nexus launcher: not initialized — call nexus_v1_launcher_init first\n");
        return -1;
    }
    if (level < 0 || level > 15) {
        printf("Nexus launcher: invalid level %d (must be 0-15)\n", level);
        return -1;
    }
    int rc = nexus_v1_load_level(&s_engine, level);
    if (rc != 0) {
        printf("Nexus launcher: failed to load level %d\n", level);
        return -1;
    }
    printf("Nexus launcher: loaded level %d\n", level);
    return 0;
}

Nexus_V1_Engine *nexus_v1_launcher_get_engine(void) {
    if (!s_initialized) {
        return NULL;
    }
    return &s_engine;
}

int nexus_v1_launcher_startup_structure3_capture_intake(
    const char *manifest_text, size_t manifest_size,
    const Nexus_V1_DgnStructure3CaptureImport *capture,
    Nexus_V1_DgnStructure3CaptureHostReceipt *out_receipt)
{
    Nexus_V1_DgnStructure2SourceReceipt source;
    int source_verified;
    int result;

    if (!out_receipt) return -1;
    nexus_v1_dgn_structure3_capture_host_receipt_clear(out_receipt);
    if (!s_initialized || !s_engine.level_loaded || !capture) return 0;
    memset(&source, 0, sizeof(source));
    (void)nexus_v1_current_level_structure2_source_receipt(&s_engine, &source);
    source_verified = source.canonical_hash_verified &&
        source.materialization_bound &&
        source.level_index == s_engine.game.current_level &&
        source.loaded_bytes_bound &&
        source.loaded_dgn_size == s_engine.current_level_dgn_size &&
        source.loaded_dgn_fnv1a64 != 0U &&
        source.loaded_dgn_fnv1a64 == nexus_v1_launcher_dgn_bytes_fnv1a64(
            s_engine.current_level_dgn_data, s_engine.current_level_dgn_size);
    if (!source_verified) return 0;
    if (!s_engine.current_level_dgn_data ||
        s_engine.current_level_dgn_size <= 0) return 0;
    result = nexus_v1_dgn_structure3_capture_host_intake(
        &s_engine.current_level, s_engine.current_level_dgn_data,
        s_engine.current_level_dgn_size, source_verified,
        manifest_text, manifest_size, capture, out_receipt);
    if (result > 0) {
        result = nexus_v1_engine_consume_structure3_capture(
            &s_engine, &out_receipt->manifest.candidate,
            &out_receipt->import_receipt.binding, capture);
    }
    return result;
}

int nexus_v1_launcher_startup_structure3_raw_capture_intake(
    const char *manifest_text, size_t manifest_size,
    const Nexus_V1_DgnStructure3RawCapturePaths *paths,
    const Nexus_V1_DgnStructure3RawCaptureAttestation *attestation,
    Nexus_V1_DgnStructure3RawCaptureHostReceipt *out_receipt)
{
    Nexus_V1_DgnStructure2SourceReceipt source;
    int source_verified;
    int result;

    if (!out_receipt) return -1;
    nexus_v1_dgn_structure3_raw_capture_host_receipt_clear(out_receipt);
    if (!s_initialized || !s_engine.level_loaded || !paths || !attestation)
        return 0;
    memset(&source, 0, sizeof(source));
    (void)nexus_v1_current_level_structure2_source_receipt(&s_engine, &source);
    source_verified = source.canonical_hash_verified &&
        source.materialization_bound &&
        source.level_index == s_engine.game.current_level &&
        source.loaded_bytes_bound &&
        source.loaded_dgn_size == s_engine.current_level_dgn_size &&
        source.loaded_dgn_fnv1a64 != 0U &&
        source.loaded_dgn_fnv1a64 == nexus_v1_launcher_dgn_bytes_fnv1a64(
            s_engine.current_level_dgn_data, s_engine.current_level_dgn_size);
    if (!source_verified || !s_engine.current_level_dgn_data ||
        s_engine.current_level_dgn_size <= 0) return 0;
    result = nexus_v1_dgn_structure3_raw_capture_host_intake(
        &s_engine.current_level, s_engine.current_level_dgn_data,
        s_engine.current_level_dgn_size, source_verified, manifest_text,
        manifest_size, paths, attestation, out_receipt);
    if (result > 0) {
        result = nexus_v1_engine_consume_structure3_capture(
            &s_engine, &out_receipt->host.manifest.candidate,
            &out_receipt->host.import_receipt.binding,
            &out_receipt->raw_reader.import_packet);
    }
    return result;
}

int nexus_v1_launcher_dgn_direct_face_capture_intake(
    int structure1f_entry_index, const char *manifest_text, size_t manifest_size,
    Nexus_V1_DgnStructure1FDirectFaceCaptureHostReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FDirectFaceCaptureHostReceipt receipt;
    Nexus_V1_DgnStructure2SourceReceipt source;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    if (!s_initialized || !s_engine.level_loaded || !manifest_text ||
        manifest_size == 0U) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.launcher_initialized = 1;
    receipt.active_level_bound = 1;
    memset(&source, 0, sizeof(source));
    (void)nexus_v1_current_level_structure2_source_receipt(&s_engine, &source);
    if (!source.canonical_hash_verified || !source.materialization_bound ||
        !source.loaded_bytes_bound ||
        source.level_index != s_engine.game.current_level ||
        source.loaded_dgn_size != s_engine.current_level_dgn_size ||
        source.loaded_dgn_fnv1a64 == 0U ||
        source.loaded_dgn_fnv1a64 != nexus_v1_launcher_dgn_bytes_fnv1a64(
            s_engine.current_level_dgn_data, s_engine.current_level_dgn_size)) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.package_source_bound = 1;
    (void)nexus_v1_engine_consume_structure1f_direct_face_capture_manifest(
        &s_engine, structure1f_entry_index, manifest_text, manifest_size,
        &receipt.capture_target);
    if (receipt.capture_target.status !=
            NEXUS_V1_STRUCTURE1F_DIRECT_FACE_CAPTURE_MANIFEST_ACCEPTED_NO_DRAW ||
        nexus_v1_engine_dm_bin_vdp1_state_write_receipt(
            &s_engine, &receipt.vdp1_state) != 1 ||
        !receipt.vdp1_state.source.canonical_hash_verified ||
        !receipt.vdp1_state.static_instruction_dataflow_proven ||
        !receipt.vdp1_state.vdp1_register_0x04_write_proven ||
        !receipt.vdp1_state.vdp1_vram_base_r14_store_proven ||
        !receipt.vdp1_state.vdp1_register_0x06_write_proven ||
        !receipt.vdp1_state.vdp1_register_0x08_write_proven ||
        !receipt.vdp1_state.vdp1_register_0x0a_write_proven ||
        receipt.vdp1_state.vdp1_command_emission_proven ||
        receipt.vdp1_state.palette_semantics_proven ||
        receipt.vdp1_state.transform_semantics_proven ||
        !receipt.vdp1_state.no_draw_only ||
        receipt.vdp1_state.fallback_visuals_permitted) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.vdp1_state_source_bound = 1;
    receipt.vdp1_capture_prerequisite_bound = 1;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_launcher_dgn_direct_face_raw_capture_intake(
    int structure1f_entry_index, const char *direct_manifest_text,
    size_t direct_manifest_size, const char *capture_manifest_text,
    size_t capture_manifest_size,
    const Nexus_V1_DgnStructure3RawCapturePaths *paths,
    const Nexus_V1_DgnStructure3RawCaptureAttestation *attestation,
    Nexus_V1_DgnStructure1FDirectFaceRawCaptureHostReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FDirectFaceRawCaptureHostReceipt receipt;
    Nexus_V1_DgnStructure2SourceReceipt source;
    int source_verified;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    nexus_v1_dgn_structure3_raw_capture_host_receipt_clear(
        &receipt.raw_capture);
    if (!s_initialized || !s_engine.level_loaded || !direct_manifest_text ||
        direct_manifest_size == 0U || !capture_manifest_text ||
        capture_manifest_size == 0U || !paths || !attestation) {
        *out_receipt = receipt;
        return 0;
    }
    (void)nexus_v1_launcher_dgn_direct_face_capture_intake(
        structure1f_entry_index, direct_manifest_text, direct_manifest_size,
        &receipt.direct_face);
    if (!receipt.direct_face.vdp1_capture_prerequisite_bound) {
        *out_receipt = receipt;
        return 0;
    }
    memset(&source, 0, sizeof(source));
    (void)nexus_v1_current_level_structure2_source_receipt(&s_engine, &source);
    source_verified = source.canonical_hash_verified &&
        source.materialization_bound && source.loaded_bytes_bound &&
        source.level_index == s_engine.game.current_level &&
        source.loaded_dgn_size == s_engine.current_level_dgn_size &&
        source.loaded_dgn_fnv1a64 != 0U &&
        source.loaded_dgn_fnv1a64 == nexus_v1_launcher_dgn_bytes_fnv1a64(
            s_engine.current_level_dgn_data, s_engine.current_level_dgn_size);
    if (!source_verified || !s_engine.current_level_dgn_data ||
        s_engine.current_level_dgn_size <= 0) {
        *out_receipt = receipt;
        return 0;
    }
    (void)nexus_v1_dgn_structure3_raw_capture_host_intake(
        &s_engine.current_level, s_engine.current_level_dgn_data,
        s_engine.current_level_dgn_size, source_verified, capture_manifest_text,
        capture_manifest_size, paths, attestation, &receipt.raw_capture);
    (void)nexus_v1_engine_bind_structure1f_direct_face_raw_capture(
        &s_engine, structure1f_entry_index, direct_manifest_text,
        direct_manifest_size, &receipt.raw_capture, &receipt.joined_capture);
    if (receipt.joined_capture.status !=
        NEXUS_V1_STRUCTURE1F_DIRECT_FACE_RAW_CAPTURE_ACCEPTED_OPAQUE) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.engine_capture_consumed = nexus_v1_engine_consume_structure3_capture(
        &s_engine, &receipt.raw_capture.host.manifest.candidate,
        &receipt.raw_capture.host.import_receipt.binding,
        &receipt.raw_capture.raw_reader.import_packet);
    if (!receipt.engine_capture_consumed) {
        *out_receipt = receipt;
        return 0;
    }
    (void)nexus_v1_engine_bind_structure1f_vdp1_material_capture(
        &s_engine, structure1f_entry_index, direct_manifest_text,
        direct_manifest_size, &receipt.raw_capture, &receipt.material_capture);
    if (receipt.material_capture.status !=
        NEXUS_V1_STRUCTURE1F_VDP1_MATERIAL_ACCEPTED_OPAQUE) {
        *out_receipt = receipt;
        return 0;
    }
    *out_receipt = receipt;
    return 1;
}

void nexus_v1_launcher_boot_receipt_clear(
    Nexus_V1_LauncherBootReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    nexus_v1_startup_launch_receipt_clear(&receipt->startup_receipt);
}

void nexus_v1_launcher_runtime_receipt_clear(
    Nexus_V1_LauncherRuntimeReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    nexus_v1_startup_launch_receipt_clear(&receipt->startup_receipt);
    nexus_v1_startup_host_receipt_clear(&receipt->boot_status_receipt);
}

void nexus_v1_launcher_startup_launch_gate_receipt_clear(
    Nexus_V1_StartupLaunchGateReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->route = NEXUS_V1_STARTUP_LAUNCH_GATE_INVALID;
    nexus_v1_startup_host_receipt_clear(&receipt->host_receipt);
}

void nexus_v1_launcher_startup_asset_handoff_receipt_clear(
    Nexus_V1_StartupAssetHandoffReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->route = NEXUS_V1_STARTUP_ASSET_HANDOFF_INVALID;
}

const char *nexus_v1_launcher_startup_launch_gate_route_name(
    Nexus_V1_StartupLaunchGateRoute route)
{
    switch (route) {
    case NEXUS_V1_STARTUP_LAUNCH_GATE_INVALID: return "invalid";
    case NEXUS_V1_STARTUP_LAUNCH_GATE_DATA_ERROR: return "data-error";
    case NEXUS_V1_STARTUP_LAUNCH_GATE_TITLE_READY: return "title-ready";
    case NEXUS_V1_STARTUP_LAUNCH_GATE_MENU_ASSET_BLOCKED:
        return "menu-asset-blocked";
    case NEXUS_V1_STARTUP_LAUNCH_GATE_MENU_READY: return "menu-ready";
    default: return "unknown";
    }
}

const char *nexus_v1_launcher_startup_asset_handoff_route_name(
    Nexus_V1_StartupAssetHandoffRoute route)
{
    switch (route) {
    case NEXUS_V1_STARTUP_ASSET_HANDOFF_INVALID: return "invalid";
    case NEXUS_V1_STARTUP_ASSET_HANDOFF_DATA_ERROR: return "data-error";
    case NEXUS_V1_STARTUP_ASSET_HANDOFF_TITLE_READY: return "title-ready";
    case NEXUS_V1_STARTUP_ASSET_HANDOFF_MENU_BLOCKED:
        return "menu-blocked";
    case NEXUS_V1_STARTUP_ASSET_HANDOFF_MAIN_MENU_READY:
        return "main-menu-ready";
    default: return "unknown";
    }
}

int nexus_v1_launcher_startup_launch_gate_from_runtime_receipt(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    Nexus_V1_StartupLaunchGateReceipt *out_receipt)
{
    const Nexus_V1_LauncherStartupAssetsReceipt *assets;

    nexus_v1_launcher_startup_launch_gate_receipt_clear(out_receipt);
    if (!runtime || !out_receipt) {
        return 0;
    }
    assets = &runtime->startup_assets;
    out_receipt->assets = *assets;
    out_receipt->engine_ready = runtime->engine ? 1 : 0;
    out_receipt->level_loaded = runtime->level_loaded ? 1 : 0;
    out_receipt->title_ready = runtime->title_loaded ? 1 : 0;
    out_receipt->title_draw_ready = assets->title_route_ready ? 1 : 0;
    out_receipt->real_menu_ready =
        assets->real_menu_surface_route_ready ? 1 : 0;
    out_receipt->save_menu_ready = assets->save_menu_route_ready ? 1 : 0;
    out_receipt->champion_menu_ready =
        assets->champion_menu_route_ready ? 1 : 0;
    out_receipt->fallback_visuals_permitted =
        assets->menu_bpk_fallback_visuals_permitted ? 1 : 0;
    out_receipt->asset_route = assets->startup_menu_asset_route;
    out_receipt->asset_blocker = assets->real_menu_surface_blocker;
    out_receipt->boot_log_line = runtime->boot_log_line;

    if (!runtime->engine || !runtime->level_loaded) {
        out_receipt->route = NEXUS_V1_STARTUP_LAUNCH_GATE_DATA_ERROR;
        out_receipt->status_scope =
            runtime->startup_receipt.host_receipt.status_scope
                ? runtime->startup_receipt.host_receipt.status_scope
                : "BOOT";
        out_receipt->status =
            runtime->startup_receipt.host_receipt.status
                ? runtime->startup_receipt.host_receipt.status
                : "NEXUS DATA ERROR";
        out_receipt->host_receipt = runtime->startup_receipt.host_receipt;
        if (!out_receipt->host_receipt.status) {
            out_receipt->host_receipt.status_scope = out_receipt->status_scope;
            out_receipt->host_receipt.status = out_receipt->status;
        }
        return 1;
    }

    if (!assets->title_route_ready) {
        out_receipt->route = NEXUS_V1_STARTUP_LAUNCH_GATE_DATA_ERROR;
        out_receipt->status_scope = "ASSETS";
        out_receipt->status = assets->startup_menu_asset_route
            ? assets->startup_menu_asset_route
            : "blocked-title-assets";
        out_receipt->host_receipt.input_result =
            NEXUS_V1_STARTUP_HOST_INPUT_REDRAW;
        out_receipt->host_receipt.status_scope = out_receipt->status_scope;
        out_receipt->host_receipt.status = out_receipt->status;
        return 1;
    }

    if (!assets->real_menu_surface_route_ready) {
        out_receipt->route =
            NEXUS_V1_STARTUP_LAUNCH_GATE_MENU_ASSET_BLOCKED;
        out_receipt->status_scope = "ASSETS";
        out_receipt->status = assets->startup_menu_asset_route
            ? assets->startup_menu_asset_route
            : "blocked-startup-assets";
        out_receipt->host_receipt.input_result =
            NEXUS_V1_STARTUP_HOST_INPUT_REDRAW;
        out_receipt->host_receipt.status_scope = out_receipt->status_scope;
        out_receipt->host_receipt.status = out_receipt->status;
        return 1;
    }

    out_receipt->route = assets->save_menu_route_ready &&
                         assets->champion_menu_route_ready
        ? NEXUS_V1_STARTUP_LAUNCH_GATE_MENU_READY
        : NEXUS_V1_STARTUP_LAUNCH_GATE_TITLE_READY;
    out_receipt->status_scope = "STARTUP";
    out_receipt->status = out_receipt->route ==
        NEXUS_V1_STARTUP_LAUNCH_GATE_MENU_READY
            ? "NEXUS MENU READY"
            : "NEXUS TITLE";
    out_receipt->host_receipt.input_result =
        NEXUS_V1_STARTUP_HOST_INPUT_REDRAW;
    out_receipt->host_receipt.status_scope = out_receipt->status_scope;
    out_receipt->host_receipt.status = out_receipt->status;
    return 1;
}

static int nexus_v1_launcher_startup_asset_handoff_from_parts(
    Nexus_V1_Engine *engine,
    int level_loaded,
    int title_loaded,
    const Nexus_V1_LauncherStartupAssetsReceipt *assets,
    const char *boot_status,
    Nexus_V1_StartupAssetHandoffReceipt *out_receipt)
{
    Nexus_V1_MenuBpkRendererHandoffReceipt renderer_handoff;
    Nexus_V1_LauncherPrs3StartupStateReceipt prs3_startup;

    nexus_v1_launcher_startup_asset_handoff_receipt_clear(out_receipt);
    if (!out_receipt || !assets) {
        return 0;
    }

    out_receipt->assets = *assets;
    memset(&renderer_handoff, 0, sizeof(renderer_handoff));
    memset(&prs3_startup, 0, sizeof(prs3_startup));
    if (engine &&
        nexus_v1_menu_bpk_renderer_handoff_receipt(engine,
                                                   &renderer_handoff) == 0) {
        out_receipt->menu_bpk_renderer_handoff = renderer_handoff;
        out_receipt->menu_bpk_renderer_handoff_valid =
            renderer_handoff.receipt_valid ? 1 : 0;
        out_receipt->menu_bpk_prs3_blocks_real_menu_route =
            renderer_handoff.status ==
            NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_BLOCKED_PRS3;
        (void)nexus_v1_launcher_prs3_startup_state(
            &renderer_handoff, NULL, &prs3_startup);
    }
    out_receipt->title_asset_handoff_ready =
        engine && title_loaded && assets->title_route_ready;
    out_receipt->real_menu_asset_handoff_ready =
        assets->real_menu_surface_route_ready &&
        out_receipt->menu_bpk_renderer_handoff_valid &&
        renderer_handoff.can_render_stored_surfaces ? 1 : 0;
    out_receipt->audio_asset_handoff_ready =
        assets->startup_audio_handoff_ready ? 1 : 0;
    out_receipt->main_menu_route_ready =
        engine &&
        level_loaded &&
        out_receipt->title_asset_handoff_ready &&
        out_receipt->audio_asset_handoff_ready &&
        out_receipt->real_menu_asset_handoff_ready;
    out_receipt->fallback_visuals_permitted = 0;
    out_receipt->saturn_asset_handoff_ready =
        out_receipt->title_asset_handoff_ready &&
        out_receipt->audio_asset_handoff_ready &&
        out_receipt->real_menu_asset_handoff_ready;
    out_receipt->real_asset_route_ready =
        out_receipt->main_menu_route_ready &&
        !out_receipt->fallback_visuals_permitted;
    out_receipt->blocks_main_menu_route =
        out_receipt->main_menu_route_ready ? 0 : 1;
    out_receipt->title_asset_route =
        out_receipt->title_asset_handoff_ready
            ? "ready-title-assets"
            : "blocked-title-assets";
    out_receipt->menu_asset_route = assets->startup_menu_asset_route;
    out_receipt->audio_asset_route =
        out_receipt->audio_asset_handoff_ready
            ? "ready-track02-sfx"
            : "blocked-track02-sfx";

    if (!engine || !level_loaded) {
        out_receipt->route = NEXUS_V1_STARTUP_ASSET_HANDOFF_DATA_ERROR;
        out_receipt->status_scope = "BOOT";
        out_receipt->status = boot_status
            ? boot_status
            : "NEXUS DATA ERROR";
    } else if (!out_receipt->title_asset_handoff_ready) {
        out_receipt->route = NEXUS_V1_STARTUP_ASSET_HANDOFF_TITLE_READY;
        out_receipt->status_scope = "ASSETS";
        out_receipt->status = "blocked-title-assets";
    } else if (out_receipt->menu_bpk_prs3_blocks_real_menu_route) {
        out_receipt->route = NEXUS_V1_STARTUP_ASSET_HANDOFF_MENU_BLOCKED;
        out_receipt->status_scope = "ASSETS";
        out_receipt->status = prs3_startup.status
            ? prs3_startup.status
            : "menu-bpk-prs3-capture-required";
    } else if (renderer_handoff.status ==
               NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_BLOCKED_SOURCE) {
        out_receipt->route = NEXUS_V1_STARTUP_ASSET_HANDOFF_MENU_BLOCKED;
        out_receipt->status_scope = "ASSETS";
        out_receipt->status = "blocked-menu-bpk-source";
    } else if (!out_receipt->real_menu_asset_handoff_ready) {
        out_receipt->route = NEXUS_V1_STARTUP_ASSET_HANDOFF_MENU_BLOCKED;
        out_receipt->status_scope = "ASSETS";
        out_receipt->status = assets->startup_menu_asset_route
            ? assets->startup_menu_asset_route
            : "blocked-menu-assets";
    } else if (out_receipt->main_menu_route_ready) {
        out_receipt->route = NEXUS_V1_STARTUP_ASSET_HANDOFF_MAIN_MENU_READY;
        out_receipt->status_scope = "STARTUP";
        out_receipt->status = "main-menu-ready";
    } else {
        out_receipt->route = NEXUS_V1_STARTUP_ASSET_HANDOFF_TITLE_READY;
        out_receipt->status_scope = "ASSETS";
        out_receipt->status = out_receipt->audio_asset_handoff_ready
            ? "title-ready"
            : "blocked-track02-sfx";
    }
    return 1;
}

int nexus_v1_launcher_startup_asset_handoff_from_runtime_receipt(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    Nexus_V1_StartupAssetHandoffReceipt *out_receipt)
{
    if (!runtime) {
        nexus_v1_launcher_startup_asset_handoff_receipt_clear(out_receipt);
        return 0;
    }
    return nexus_v1_launcher_startup_asset_handoff_from_parts(
        runtime->engine,
        runtime->level_loaded,
        runtime->title_loaded,
        &runtime->startup_assets,
        runtime->startup_receipt.host_receipt.status,
        out_receipt);
}

static void nexus_v1_launcher_resume_receipt_clear(
    Nexus_V1_LauncherResumeReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    nexus_v1_startup_host_receipt_clear(&receipt->host_receipt);
}

static int nexus_v1_surface_loaded(const Nexus_V1_Engine *engine,
                                   Nexus_UISurfaceType surface)
{
    if (!engine || surface < 0 || surface >= NEXUS_SURFACE_COUNT) {
        return 0;
    }
    return engine->ui.surfaces[surface].data &&
           engine->ui.surfaces[surface].w > 0 &&
           engine->ui.surfaces[surface].h > 0;
}

int nexus_v1_launcher_menu_bpk_no_draw_presentation_receipt(
    const Nexus_V1_BpkRuntimeUploadReceipt *upload,
    const Nexus_V1_BpkRuntimeUploadRow *row,
    Nexus_V1_LauncherMenuBpkNoDrawPresentationReceipt *out_receipt)
{
    if (!out_receipt) {
        return 0;
    }
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!upload || !row || upload->unknown_prs3_mode_entries != 0U ||
        upload->first_prs3_entry_index == UINT32_MAX ||
        (row->entry_index == upload->first_prs3_entry_index &&
         (row->payload_offset != upload->first_prs3_payload_offset ||
          row->payload_size != upload->first_prs3_payload_size ||
          row->payload_fnv1a64 != upload->first_prs3_payload_fnv1a64 ||
          row->prs3_version != upload->first_prs3_version ||
          row->prs3_pixel_count != upload->first_prs3_pixel_count ||
          row->header_first_u32 != upload->first_prs3_header_first_u32 ||
          row->header_minus_payload != upload->first_prs3_header_minus_payload ||
          memcmp(&row->compression, &upload->first_prs3_compression,
                 sizeof(row->compression)) != 0)) ||
        !row->compression.valid || !upload->first_prs3_compression.valid ||
        row->compression.entry_index != row->entry_index ||
        row->compression.mode_flags != row->mode ||
        row->compression.declared_pixel_count != row->prs3_pixel_count ||
        row->compression.declared_output_bytes != row->expected_output_bytes ||
        row->compression.compressed_offset < row->stream_offset ||
        row->compression.compressed_length == 0U ||
        row->compression.compressed_offset > UINT32_MAX -
            row->compression.compressed_length ||
        row->stream_offset > UINT32_MAX - row->stream_size ||
        row->compression.compressed_offset + row->compression.compressed_length >
            row->stream_offset + row->stream_size ||
        row->compression.compressed_fnv1a64 == 0U ||
        row->compression.decoder_promoted || row->compression.pixels_exposed ||
        row->compression.fallback_visuals_permitted ||
        !row->prs3_header_valid ||
        row->prs3_version != NEXUS_V1_BPK_PRS3_VERSION ||
        row->payload_size < NEXUS_V1_BPK_PRS3_HEADER_BYTES ||
        row->payload_offset > UINT32_MAX - 8U ||
        row->stream_offset != row->payload_offset + 8U ||
        row->stream_size != row->payload_size - 8U ||
        row->stream_size < 4U ||
        !row->decode_blocked || !row->evidence_only ||
        !row->renderer_handoff_blocked || !row->upload_blocked ||
        row->upload_ready || row->decoded_pixels_emitted != 0U ||
        row->fallback_visuals_permitted) {
        return 0;
    }
    out_receipt->valid = 1;
    out_receipt->no_draw_only = 1;
    out_receipt->entry_index = row->entry_index;
    out_receipt->payload_offset = row->payload_offset;
    out_receipt->payload_length = row->payload_size;
    out_receipt->payload_fnv1a64 = row->payload_fnv1a64;
    out_receipt->prs3_version = row->prs3_version;
    out_receipt->prs3_pixel_count = row->prs3_pixel_count;
    out_receipt->header_first_u32 = row->header_first_u32;
    out_receipt->header_minus_payload = row->header_minus_payload;
    out_receipt->compression = row->compression;
    return 1;
}

int nexus_v1_launcher_admit_m11_menu_bpk_no_draw_host(
    Nexus_V1_Engine *engine, uint64_t route_epoch,
    uint64_t package_fnv1a64,
    const Nexus_V1_LauncherMenuBpkNoDrawPresentationReceipt *presentation,
    Nexus_V1_LauncherM11MenuBpkNoDrawHostReceipt *out_receipt)
{
    Nexus_V1_BpkRuntimeUploadReceipt upload;
    Nexus_V1_BpkRuntimeUploadRow rows[NEXUS_V1_BPK_UPLOAD_PLAN_MAX_ROWS];
    int count;
    int i;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!engine || !presentation || !presentation->valid ||
        !presentation->no_draw_only || !route_epoch || !package_fnv1a64 ||
        package_fnv1a64 != engine->menu_bpk_package_fnv1a64 ||
        nexus_v1_menu_bpk_upload_plan_receipt(engine, &upload) != 0) {
        if (engine) {
            (void)nexus_v1_engine_set_menu_bpk_no_draw_host_receipt(
                engine, 0U, 0U, NULL, NULL);
        }
        return 0;
    }
    count = nexus_v1_menu_bpk_upload_plan_rows(
        engine, rows, (int)NEXUS_V1_BPK_UPLOAD_PLAN_MAX_ROWS);
    for (i = 0; count > 0 && i < count; ++i) {
        if (rows[i].entry_index == presentation->entry_index &&
            nexus_v1_launcher_menu_bpk_no_draw_presentation_receipt(
                &upload, &rows[i], &out_receipt->presentation) &&
            out_receipt->presentation.payload_offset == presentation->payload_offset &&
            out_receipt->presentation.payload_length == presentation->payload_length &&
            out_receipt->presentation.payload_fnv1a64 == presentation->payload_fnv1a64 &&
            out_receipt->presentation.prs3_version == presentation->prs3_version &&
            out_receipt->presentation.prs3_pixel_count == presentation->prs3_pixel_count &&
            out_receipt->presentation.header_first_u32 == presentation->header_first_u32 &&
            out_receipt->presentation.header_minus_payload == presentation->header_minus_payload &&
            memcmp(&out_receipt->presentation.compression,
                   &presentation->compression,
                   sizeof(presentation->compression)) == 0 &&
            nexus_v1_engine_set_menu_bpk_no_draw_host_receipt(
                engine, route_epoch, package_fnv1a64, &upload, &rows[i])) {
            out_receipt->valid = 1;
            out_receipt->no_draw_only = 1;
            out_receipt->draw_disabled = 1;
            out_receipt->route_epoch = route_epoch;
            out_receipt->package_fnv1a64 = package_fnv1a64;
            return 1;
        }
    }
    (void)nexus_v1_engine_set_menu_bpk_no_draw_host_receipt(
        engine, 0U, 0U, NULL, NULL);
    memset(out_receipt, 0, sizeof(*out_receipt));
    return 0;
}

int nexus_v1_launcher_consume_m11_menu_bpk_no_draw_host(
    const Nexus_V1_Engine *engine, uint64_t route_epoch,
    uint64_t package_fnv1a64,
    Nexus_V1_LauncherM11MenuBpkNoDrawHostReceipt *out_receipt)
{
    Nexus_V1_BpkRuntimeUploadReceipt upload;
    Nexus_V1_BpkRuntimeUploadRow rows[NEXUS_V1_BPK_UPLOAD_PLAN_MAX_ROWS];
    int count;
    int index;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!nexus_v1_engine_menu_bpk_no_draw_host_ready(
            engine, route_epoch, package_fnv1a64)) {
        return 0;
    }
    if (nexus_v1_menu_bpk_upload_plan_receipt(engine, &upload) != 0 ||
        (count = nexus_v1_menu_bpk_upload_plan_rows(
             engine, rows, (int)NEXUS_V1_BPK_UPLOAD_PLAN_MAX_ROWS)) <= 0) {
        return 0;
    }
    for (index = 0; index < count; ++index) {
        const Nexus_V1_BpkRuntimeUploadRow *row = &rows[index];
        if (row->entry_index == engine->menu_bpk_no_draw_host_entry_index &&
            row->payload_offset == engine->menu_bpk_no_draw_host_payload_offset &&
            row->payload_size == engine->menu_bpk_no_draw_host_payload_size &&
            row->payload_fnv1a64 == engine->menu_bpk_no_draw_host_payload_fnv1a64 &&
            memcmp(&row->compression,
                   &engine->menu_bpk_no_draw_host_compression,
                   sizeof(row->compression)) == 0 &&
            nexus_v1_launcher_menu_bpk_no_draw_presentation_receipt(
                &upload, row, &out_receipt->presentation)) break;
    }
    if (index == count) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    out_receipt->valid = 1;
    out_receipt->no_draw_only = 1;
    out_receipt->draw_disabled = 1;
    out_receipt->route_epoch = route_epoch;
    out_receipt->package_fnv1a64 = package_fnv1a64;
    return 1;
}

int nexus_v1_launcher_bind_saturn_card_m11_no_draw_startup(
    Nexus_V1_Engine *engine, uint64_t route_epoch,
    uint64_t card_fnv1a64, uint64_t package_fnv1a64,
    int direct_card_selected,
    Nexus_V1_LauncherSaturnCardM11NoDrawStartupReceipt *out_receipt)
{
    Nexus_V1_LauncherSaturnCardM11NoDrawStartupReceipt receipt;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.opaque_saturn_card_only = 1;
    receipt.no_draw_only = 1;
    receipt.draw_disabled = 1;
    if (!engine || !direct_card_selected || !route_epoch ||
        !card_fnv1a64 || !package_fnv1a64 ||
        !nexus_v1_launcher_bind_saturn_card_boot_route(
            engine, route_epoch, card_fnv1a64, package_fnv1a64, 1,
            &receipt.card) ||
        !nexus_v1_launcher_consume_m11_menu_bpk_no_draw_host(
            engine, route_epoch, package_fnv1a64, &receipt.m11) ||
        !receipt.card.opaque_only || !receipt.m11.no_draw_only ||
        !receipt.m11.draw_disabled ||
        receipt.card.package_fnv1a64 != receipt.m11.package_fnv1a64 ||
        receipt.card.route_epoch != receipt.m11.route_epoch) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.card_fnv1a64 = card_fnv1a64;
    receipt.package_fnv1a64 = package_fnv1a64;
    receipt.route_epoch = route_epoch;
    *out_receipt = receipt;
    return 1;
}

static int nexus_v1_launcher_aux_identity_matches(
    const Nexus_V1_LevelAuxSourceReceipt *active,
    const Nexus_V1_SlevSalDirectIdentity *discovered)
{
    return active && discovered && discovered->valid &&
        active->canonical_hash_verified && active->exact_source_entry_observed &&
        strcmp(active->canonical_name, discovered->canonical_name) == 0 &&
        strcmp(active->canonical_md5, discovered->md5) == 0;
}

int nexus_v1_launcher_admit_m11_slev_sal_no_draw(
    Nexus_V1_Engine *engine,
    const Nexus_V1_SlevSalAssetDiscoveryReceipt *assets,
    uint64_t route_epoch, uint64_t package_fnv1a64, uint64_t card_fnv1a64,
    int direct_card_selected, uint32_t level_index,
    Nexus_V1_LauncherM11SlevSalNoDrawReceipt *out_receipt)
{
    Nexus_V1_LauncherM11SlevSalNoDrawReceipt receipt;
    const Nexus_V1_SlevSalDirectLevelIdentity *level_assets;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    receipt.blocks_real_script_dispatch = 1;
    receipt.blocks_real_sfx_playback = 1;
    if (!engine || !assets || !assets->valid || assets->payload_materialized ||
        !assets->direct_files_only || !route_epoch || !package_fnv1a64 ||
        level_index >= NEXUS_V1_SLEV_SAL_ASSET_LEVEL_COUNT ||
        engine->game.current_level != (int)level_index ||
        !nexus_v1_launcher_bind_saturn_card_m11_no_draw_startup(
            engine, route_epoch, card_fnv1a64, package_fnv1a64,
            direct_card_selected, &receipt.startup) ||
        nexus_v1_current_level_aux_admission_receipt(engine,
                                                     &receipt.level_aux) != 1) {
        *out_receipt = receipt;
        return 0;
    }
    level_assets = &assets->levels[level_index];
    if (!level_assets->valid || receipt.level_aux.status !=
            NEXUS_V1_LEVEL_AUX_ADMISSION_READY_NO_RUNTIME ||
        receipt.level_aux.level_index != (int)level_index ||
        !receipt.level_aux.no_runtime_only ||
        !receipt.level_aux.blocks_real_script_dispatch ||
        !receipt.level_aux.blocks_real_sfx_playback ||
        !nexus_v1_launcher_aux_identity_matches(
            &engine->level_aux_runtime_receipt.slev, &level_assets->slev) ||
        !nexus_v1_launcher_aux_identity_matches(
            &engine->level_aux_runtime_receipt.sal, &level_assets->sal) ||
        !nexus_v1_launcher_aux_identity_matches(
            &engine->level_aux_runtime_receipt.map, &level_assets->map) ||
        !nexus_v1_launcher_aux_identity_matches(
            &engine->level_aux_runtime_receipt.sound_driver,
            &assets->sound_driver)) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.level_index = level_index;
    receipt.route_epoch = route_epoch;
    receipt.package_fnv1a64 = package_fnv1a64;
    receipt.asset_corpus_fnv1a64 = assets->corpus_fnv1a64;
    receipt.assets = *level_assets;
    receipt.sound_driver = assets->sound_driver;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_launcher_admit_m11_slev_task_body_no_dispatch(
    Nexus_V1_Engine *engine,
    const Nexus_V1_SlevSalAssetDiscoveryReceipt *assets,
    const Nexus_V1_SlevTaskBodyCapturePlan *plan,
    uint64_t route_epoch, uint64_t package_fnv1a64, uint64_t card_fnv1a64,
    int direct_card_selected, uint32_t level_index,
    Nexus_V1_LauncherM11SlevTaskBodyNoDispatchReceipt *out_receipt)
{
    Nexus_V1_LauncherM11SlevTaskBodyNoDispatchReceipt receipt;
    const Nexus_V1_SlevTaskBodyCaptureTarget *target;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    receipt.blocks_real_script_dispatch = 1;
    receipt.blocks_real_sfx_playback = 1;
    if (!assets || !plan || !plan->valid || !plan->no_dispatch_only ||
        plan->fallback_script_permitted ||
        level_index >= NEXUS_V1_SLEV_TASK_BODY_CAPTURE_LEVEL_COUNT ||
        !(target = &plan->targets[level_index]) || !target->valid ||
        target->level_index != level_index || !target->source_fnv1a64 ||
        !target->entry_pc || !target->task_body_pc ||
        !target->callback_or_write_pc || !target->raw_trace_fnv1a64 ||
        !target->raw_trace_byte_count || !target->source_order_required ||
        !target->original_saturn_trace_required || !target->no_dispatch_only ||
        target->fallback_script_permitted ||
        !nexus_v1_launcher_admit_m11_slev_sal_no_draw(
            engine, assets, route_epoch, package_fnv1a64, card_fnv1a64,
            direct_card_selected, level_index, &receipt.level_aux) ||
        !receipt.level_aux.assets.slev.valid ||
        receipt.level_aux.assets.slev.fnv1a64 != target->source_fnv1a64 ||
        !receipt.level_aux.assets.slev.byte_count ||
        !receipt.level_aux.assets.slev.md5[0]) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.level_index = level_index;
    receipt.route_epoch = route_epoch;
    receipt.package_fnv1a64 = package_fnv1a64;
    receipt.card_fnv1a64 = card_fnv1a64;
    receipt.slev = receipt.level_aux.assets.slev;
    receipt.task_body = *target;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_launcher_admit_m11_slev_task_sal_no_op_startup(
    Nexus_V1_Engine *engine,
    const Nexus_V1_SlevSalAssetDiscoveryReceipt *assets,
    const Nexus_V1_LauncherM11SlevTaskBodyNoDispatchReceipt *task,
    const Nexus_V1_SalContainerProvenanceReceipt *sal_container,
    const Nexus_V1_SndlevMapProvenanceReceipt *map_table,
    const Nexus_V1_LauncherSlevTaskSalCaptureBinding *binding,
    uint64_t route_epoch, uint64_t package_fnv1a64, uint64_t card_fnv1a64,
    int direct_card_selected, uint32_t level_index,
    Nexus_V1_LauncherM11SlevTaskSalNoOpStartupReceipt *out_receipt)
{
    Nexus_V1_LauncherM11SlevTaskSalNoOpStartupReceipt receipt;
    Nexus_V1_LauncherM11SlevSalNoDrawReceipt current;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    receipt.no_op_only = 1;
    receipt.commands_opaque = 1;
    receipt.audio_opaque = 1;
    receipt.blocks_real_script_dispatch = 1;
    receipt.blocks_real_sfx_playback = 1;
    memset(&current, 0, sizeof(current));
    if (!assets || !task || !task->valid || !task->no_draw_only ||
        !task->blocks_real_script_dispatch || !task->blocks_real_sfx_playback ||
        !sal_container || !sal_container->valid || sal_container->codec_proven ||
        sal_container->playback_permitted || !sal_container->source_fnv1a64 ||
        !sal_container->source_byte_count || !sal_container->descriptor_length ||
        !sal_container->descriptor_fnv1a64 ||
        sal_container->descriptor_offset != NEXUS_V1_SAL_CONTAINER_HEADER_BYTES ||
        !map_table || !map_table->valid || map_table->playback_permitted ||
        !map_table->source_fnv1a64 || !map_table->source_byte_count ||
        !map_table->table_length || !map_table->table_fnv1a64 ||
        !map_table->record_count || !binding ||
        !binding->original_saturn_trace_bound || !binding->task_trace_fnv1a64 ||
        !binding->sal_descriptor_fnv1a64 || !binding->map_table_fnv1a64 ||
        !binding->sound_driver_fnv1a64 ||
        task->level_index != level_index || task->route_epoch != route_epoch ||
        task->package_fnv1a64 != package_fnv1a64 ||
        task->card_fnv1a64 != card_fnv1a64 ||
        !task->task_body.raw_trace_fnv1a64 ||
        !task->task_body.raw_trace_byte_count ||
        !nexus_v1_launcher_admit_m11_slev_sal_no_draw(
            engine, assets, route_epoch, package_fnv1a64, card_fnv1a64,
            direct_card_selected, level_index, &current) ||
        task->slev.fnv1a64 != current.assets.slev.fnv1a64 ||
        task->task_body.source_fnv1a64 != current.assets.slev.fnv1a64 ||
        task->level_aux.sound_driver.fnv1a64 != current.sound_driver.fnv1a64 ||
        !current.sound_driver.fnv1a64 ||
        binding->task_trace_fnv1a64 != task->task_body.raw_trace_fnv1a64 ||
        binding->sal_descriptor_fnv1a64 != sal_container->descriptor_fnv1a64 ||
        binding->map_table_fnv1a64 != map_table->table_fnv1a64 ||
        binding->sound_driver_fnv1a64 != current.sound_driver.fnv1a64 ||
        sal_container->source_fnv1a64 != current.assets.sal.fnv1a64 ||
        sal_container->source_byte_count != current.assets.sal.byte_count ||
        map_table->source_fnv1a64 != current.assets.map.fnv1a64 ||
        map_table->source_byte_count != current.assets.map.byte_count) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.level_index = level_index;
    receipt.route_epoch = route_epoch;
    receipt.package_fnv1a64 = package_fnv1a64;
    receipt.card_fnv1a64 = card_fnv1a64;
    receipt.task_trace_fnv1a64 = task->task_body.raw_trace_fnv1a64;
    receipt.sal_descriptor_fnv1a64 = sal_container->descriptor_fnv1a64;
    receipt.map_table_fnv1a64 = map_table->table_fnv1a64;
    receipt.sound_driver_fnv1a64 = current.sound_driver.fnv1a64;
    receipt.task = *task;
    receipt.sal_container = *sal_container;
    receipt.map_table = *map_table;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_launcher_import_m11_slev_sal_capture(
    Nexus_V1_Engine *engine,
    const Nexus_V1_SlevSalAssetDiscoveryReceipt *assets,
    const Nexus_V1_LauncherM11SlevTaskSalNoOpStartupReceipt *route,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    Nexus_V1_LauncherM11SlevSalCaptureImportReceipt *out_receipt)
{
    Nexus_V1_LauncherM11SlevSalCaptureImportReceipt receipt;
    Nexus_V1_LauncherM11SlevSalNoDrawReceipt current;
    uint32_t header_bytes;
    uint32_t payload_offset;
    uint32_t payload_length;
    uint64_t payload_fnv1a64;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    receipt.no_op_only = 1;
    receipt.commands_opaque = 1;
    receipt.audio_opaque = 1;
    memset(&current, 0, sizeof(current));
    if (!route || !route->valid || !route->no_draw_only ||
        !route->no_op_only || !route->commands_opaque || !route->audio_opaque ||
        !route->blocks_real_script_dispatch || !route->blocks_real_sfx_playback ||
        !route->task.valid || !route->task.task_body.raw_trace_fnv1a64 ||
        !route->sal_descriptor_fnv1a64 || !route->map_table_fnv1a64 ||
        !route->sound_driver_fnv1a64 || !capture_bytes ||
        capture_byte_count < NEXUS_V1_M11_SLEV_SAL_CAPTURE_HEADER_BYTES ||
        memcmp(capture_bytes, NEXUS_V1_M11_SLEV_SAL_CAPTURE_MAGIC, 8U) != 0 ||
        nexus_v1_launcher_capture_be32(capture_bytes + 8U) !=
            NEXUS_V1_M11_SLEV_SAL_CAPTURE_VERSION ||
        (header_bytes = nexus_v1_launcher_capture_be32(capture_bytes + 12U)) !=
            NEXUS_V1_M11_SLEV_SAL_CAPTURE_HEADER_BYTES ||
        header_bytes > capture_byte_count ||
        !nexus_v1_launcher_admit_m11_slev_sal_no_draw(
            engine, assets, route->route_epoch, route->package_fnv1a64,
            route->card_fnv1a64, 1, route->level_index, &current) ||
        current.assets.slev.fnv1a64 != route->task.task_body.source_fnv1a64 ||
        current.assets.sal.fnv1a64 != route->sal_container.source_fnv1a64 ||
        current.assets.map.fnv1a64 != route->map_table.source_fnv1a64 ||
        current.sound_driver.fnv1a64 != route->sound_driver_fnv1a64 ||
        nexus_v1_launcher_capture_be64(capture_bytes + 16U) != route->route_epoch ||
        nexus_v1_launcher_capture_be64(capture_bytes + 24U) != route->package_fnv1a64 ||
        nexus_v1_launcher_capture_be64(capture_bytes + 32U) != route->card_fnv1a64 ||
        nexus_v1_launcher_capture_be64(capture_bytes + 40U) != route->task_trace_fnv1a64 ||
        nexus_v1_launcher_capture_be64(capture_bytes + 48U) != route->sal_descriptor_fnv1a64 ||
        nexus_v1_launcher_capture_be64(capture_bytes + 56U) != route->map_table_fnv1a64 ||
        nexus_v1_launcher_capture_be64(capture_bytes + 64U) != route->sound_driver_fnv1a64 ||
        nexus_v1_launcher_capture_be64(capture_bytes + 88U) !=
            route->task.task_body.source_fnv1a64) {
        *out_receipt = receipt;
        return 0;
    }
    payload_offset = nexus_v1_launcher_capture_be32(capture_bytes + 72U);
    payload_length = nexus_v1_launcher_capture_be32(capture_bytes + 76U);
    payload_fnv1a64 = nexus_v1_launcher_capture_be64(capture_bytes + 80U);
    if (payload_offset != header_bytes || !payload_length || !payload_fnv1a64 ||
        payload_offset > capture_byte_count ||
        payload_length > capture_byte_count - payload_offset ||
        payload_offset + (size_t)payload_length != capture_byte_count ||
        nexus_v1_launcher_capture_fnv1a64(capture_bytes + payload_offset,
                                          payload_length) != payload_fnv1a64) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.route = *route;
    receipt.capture_fnv1a64 = nexus_v1_launcher_capture_fnv1a64(
        capture_bytes, capture_byte_count);
    receipt.capture_byte_count = capture_byte_count;
    receipt.payload_offset = payload_offset;
    receipt.payload_length = payload_length;
    receipt.payload_fnv1a64 = payload_fnv1a64;
    receipt.header_version_bound = 1;
    receipt.payload_bounds_bound = 1;
    receipt.payload_hash_bound = 1;
    receipt.route_bound = 1;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_launcher_verify_m11_slev_sal_local_artifact(
    const Nexus_V1_LauncherM11SlevTaskSalNoOpStartupReceipt *route,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    Nexus_V1_LauncherM11SlevSalLocalArtifactReceipt *out_receipt)
{
    Nexus_V1_LauncherM11SlevSalLocalArtifactReceipt receipt;
    uint32_t payload_offset;
    uint32_t payload_length;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    receipt.no_op_only = 1;
    if (!route || !route->valid || !route->no_draw_only || !route->no_op_only ||
        !route->commands_opaque || !route->audio_opaque || !capture_bytes ||
        capture_byte_count < NEXUS_V1_M11_SLEV_SAL_CAPTURE_HEADER_BYTES ||
        memcmp(capture_bytes, NEXUS_V1_M11_SLEV_SAL_CAPTURE_MAGIC, 8U) != 0 ||
        nexus_v1_launcher_capture_be32(capture_bytes + 8U) !=
            NEXUS_V1_M11_SLEV_SAL_CAPTURE_VERSION ||
        nexus_v1_launcher_capture_be32(capture_bytes + 12U) !=
            NEXUS_V1_M11_SLEV_SAL_CAPTURE_HEADER_BYTES) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.route_bound =
        nexus_v1_launcher_capture_be64(capture_bytes + 16U) == route->route_epoch &&
        nexus_v1_launcher_capture_be64(capture_bytes + 24U) == route->package_fnv1a64 &&
        nexus_v1_launcher_capture_be64(capture_bytes + 32U) == route->card_fnv1a64;
    receipt.task_bound =
        nexus_v1_launcher_capture_be64(capture_bytes + 40U) == route->task_trace_fnv1a64 &&
        nexus_v1_launcher_capture_be64(capture_bytes + 88U) ==
            route->task.task_body.source_fnv1a64;
    receipt.sal_bound = nexus_v1_launcher_capture_be64(capture_bytes + 48U) ==
        route->sal_descriptor_fnv1a64;
    receipt.map_bound = nexus_v1_launcher_capture_be64(capture_bytes + 56U) ==
        route->map_table_fnv1a64;
    receipt.sddrvs_bound = nexus_v1_launcher_capture_be64(capture_bytes + 64U) ==
        route->sound_driver_fnv1a64;
    payload_offset = nexus_v1_launcher_capture_be32(capture_bytes + 72U);
    payload_length = nexus_v1_launcher_capture_be32(capture_bytes + 76U);
    receipt.payload_bounds_bound =
        payload_offset == NEXUS_V1_M11_SLEV_SAL_CAPTURE_HEADER_BYTES &&
        payload_length > 0U && payload_offset <= capture_byte_count &&
        payload_length <= capture_byte_count - payload_offset &&
        payload_offset + (size_t)payload_length == capture_byte_count;
    receipt.payload_hash_bound = receipt.payload_bounds_bound &&
        nexus_v1_launcher_capture_be64(capture_bytes + 80U) ==
            nexus_v1_launcher_capture_fnv1a64(
                capture_bytes + payload_offset, payload_length);
    if (!receipt.route_bound || !receipt.task_bound || !receipt.sal_bound ||
        !receipt.map_bound || !receipt.sddrvs_bound ||
        !receipt.payload_hash_bound) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.payload_opaque = 1;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_launcher_admit_m12_m11_slev_sal_capture_required(
    const Nexus_V1_LauncherM11SlevTaskSalNoOpStartupReceipt *startup,
    const char *bios_sha256, uint32_t bios_region, const char *disc_sha256,
    Nexus_V1_LauncherM12M11SlevSalCaptureRouteReceipt *out_receipt)
{
    Nexus_V1_LauncherM12M11SlevSalCaptureRouteReceipt receipt;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.capture_required = 1;
    receipt.operator_only = 1;
    receipt.no_draw_only = 1;
    receipt.no_op_only = 1;
    receipt.commands_opaque = 1;
    receipt.audio_opaque = 1;
    if (!startup || !startup->valid || !startup->no_draw_only ||
        !startup->no_op_only || !startup->commands_opaque ||
        !startup->audio_opaque || !startup->blocks_real_script_dispatch ||
        !startup->blocks_real_sfx_playback || !startup->task_trace_fnv1a64 ||
        !startup->sal_descriptor_fnv1a64 || !startup->map_table_fnv1a64 ||
        !startup->sound_driver_fnv1a64 || !startup->route_epoch ||
        !startup->package_fnv1a64 || !startup->card_fnv1a64 ||
        !nexus_v1_launcher_sha256_text_valid(bios_sha256) ||
        !nexus_v1_launcher_sha256_text_valid(disc_sha256) ||
        bios_region < 1U || bios_region > 3U) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.bios_region = bios_region;
    memcpy(receipt.bios_sha256, bios_sha256, 64U);
    receipt.bios_sha256[64] = '\0';
    memcpy(receipt.disc_sha256, disc_sha256, 64U);
    receipt.disc_sha256[64] = '\0';
    receipt.startup = *startup;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_launcher_resume_m12_m11_slev_sal_capture(
    Nexus_V1_Engine *engine,
    const Nexus_V1_SlevSalAssetDiscoveryReceipt *assets,
    const Nexus_V1_LauncherM12M11SlevSalCaptureRouteReceipt *route,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    Nexus_V1_LauncherM12M11SlevSalCaptureRouteReceipt *out_receipt)
{
    Nexus_V1_LauncherM12M11SlevSalCaptureRouteReceipt receipt;
    Nexus_V1_LauncherM11SlevSalLocalArtifactReceipt local_artifact;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.capture_required = 1;
    receipt.operator_only = 1;
    receipt.no_draw_only = 1;
    receipt.no_op_only = 1;
    receipt.commands_opaque = 1;
    receipt.audio_opaque = 1;
    if (!route || !route->valid || !route->capture_required ||
        route->capture_imported || !route->operator_only ||
        !route->no_draw_only || !route->no_op_only ||
        !route->commands_opaque || !route->audio_opaque ||
        route->dispatch_permitted || route->playback_permitted ||
        !nexus_v1_launcher_admit_m12_m11_slev_sal_capture_required(
            &route->startup, route->bios_sha256, route->bios_region,
            route->disc_sha256, &receipt)) {
        *out_receipt = receipt;
        return 0;
    }
    if (!nexus_v1_launcher_verify_m11_slev_sal_local_artifact(
            &receipt.startup, capture_bytes, capture_byte_count,
            &local_artifact) ||
        !nexus_v1_launcher_import_m11_slev_sal_capture(
            engine, assets, &receipt.startup, capture_bytes, capture_byte_count,
            &receipt.capture)) {
        receipt.valid = 0;
        *out_receipt = receipt;
        return 0;
    }
    receipt.capture_required = 0;
    receipt.capture_imported = 1;
    receipt.resume_ready = 1;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_launcher_admit_direct_lev_m11_dungeon_handoff(
    Nexus_V1_Engine *engine, const Nexus_V1_LevCorpusDiscoveryReceipt *corpus,
    uint64_t route_epoch, uint64_t package_fnv1a64, uint64_t card_fnv1a64,
    int direct_card_selected, uint32_t level_index,
    const Nexus_V1_DgnStructure1F2FaceAdjacencyTransformReceipt *geometry,
    Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt *out_receipt)
{
    Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt receipt;
    const Nexus_V1_LevCorpusDirectLevelIdentity *level;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    receipt.draw_disabled = 1;
    if (!engine || !corpus || !corpus->valid || corpus->payload_materialized ||
        !corpus->direct_files_only || !route_epoch || !package_fnv1a64 ||
        !card_fnv1a64 || !direct_card_selected ||
        level_index >= NEXUS_V1_LEV_CORPUS_LEVEL_COUNT ||
        !nexus_v1_launcher_bind_saturn_card_m11_no_draw_startup(
            engine, route_epoch, card_fnv1a64, package_fnv1a64, 1,
            &receipt.champion_startup)) {
        *out_receipt = receipt;
        return 0;
    }
    level = &corpus->levels[level_index];
    if (!level->valid || !level->md5[0] || !level->byte_count ||
        !level->fnv1a64 ||
        !nexus_v1_lev_corpus_admit_m11_dungeon_no_draw(
            engine, corpus, route_epoch, level_index, geometry) ||
        !nexus_v1_engine_m11_direct_lev_dungeon_no_draw_ready(
            engine, route_epoch, (int)level_index, level->md5,
            level->byte_count, level->fnv1a64, &receipt.dungeon)) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.route_epoch = route_epoch;
    receipt.package_fnv1a64 = package_fnv1a64;
    receipt.card_fnv1a64 = card_fnv1a64;
    receipt.level_index = level_index;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_launcher_build_m11_structure2_face_capture_replay_target(
    Nexus_V1_Engine *engine, const Nexus_V1_LevCorpusDiscoveryReceipt *corpus,
    const Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt *handoff,
    Nexus_V1_LauncherM11Structure2FaceCaptureReplayTarget *out_target)
{
    Nexus_V1_LauncherM11Structure2FaceCaptureReplayTarget target;
    const Nexus_V1_LevCorpusDirectLevelIdentity *level;
    Nexus_V1_DgnM11DirectLevNoDrawReceipt dungeon;

    if (!out_target) return 0;
    memset(&target, 0, sizeof(target));
    target.no_draw_only = 1;
    target.blocks_real_dgn_mesh_render = 1;
    if (!engine || !corpus || !corpus->valid || corpus->payload_materialized ||
        !corpus->direct_files_only || !handoff || !handoff->valid ||
        !handoff->no_draw_only || !handoff->draw_disabled ||
        !handoff->route_epoch || !handoff->package_fnv1a64 ||
        !handoff->card_fnv1a64 ||
        handoff->level_index >= NEXUS_V1_LEV_CORPUS_LEVEL_COUNT ||
        !handoff->champion_startup.valid ||
        !handoff->champion_startup.opaque_saturn_card_only ||
        !handoff->champion_startup.no_draw_only ||
        handoff->champion_startup.draw_disabled == 0 ||
        handoff->champion_startup.route_epoch != handoff->route_epoch ||
        handoff->champion_startup.package_fnv1a64 != handoff->package_fnv1a64 ||
        handoff->champion_startup.card_fnv1a64 != handoff->card_fnv1a64) {
        *out_target = target;
        return 0;
    }
    level = &corpus->levels[handoff->level_index];
    memset(&dungeon, 0, sizeof(dungeon));
    if (!level->valid || !nexus_v1_lev_corpus_direct_identity_still_matches(level) ||
        !nexus_v1_engine_m11_direct_lev_dungeon_no_draw_ready(
            engine, handoff->route_epoch, (int)handoff->level_index, level->md5,
            level->byte_count, level->fnv1a64, &dungeon) ||
        !dungeon.valid || !dungeon.no_draw_only ||
        dungeon.fallback_visuals_permitted ||
        !dungeon.blocks_real_dgn_mesh_render ||
        dungeon.route_epoch != handoff->route_epoch ||
        dungeon.level_index != (int)handoff->level_index ||
        dungeon.dgn_byte_count != level->byte_count ||
        dungeon.dgn_fnv1a64 != level->fnv1a64 ||
        !dungeon.structure2_face_descriptor.valid ||
        dungeon.structure2_face_descriptor.route_epoch != handoff->route_epoch ||
        dungeon.structure2_face_descriptor.level_index != handoff->level_index ||
        dungeon.structure2_face_descriptor.package_fnv1a64 != level->fnv1a64 ||
        !dungeon.structure2_face_descriptor.face_descriptor_bound ||
        !dungeon.structure2_face_descriptor.candidates_opaque ||
        !dungeon.structure2_face_descriptor.no_draw_only ||
        dungeon.structure2_face_descriptor.fallback_visuals_permitted) {
        *out_target = target;
        return 0;
    }
    target.valid = 1;
    target.route_epoch = handoff->route_epoch;
    target.package_fnv1a64 = handoff->package_fnv1a64;
    target.card_fnv1a64 = handoff->card_fnv1a64;
    target.level_index = handoff->level_index;
    target.dgn_fnv1a64 = level->fnv1a64;
    target.dgn_byte_count = level->byte_count;
    target.face_descriptor = dungeon.structure2_face_descriptor;
    target.direct_source_rehashed = 1;
    target.original_saturn_capture_required = 1;
    *out_target = target;
    return 1;
}

static int nexus_v1_launcher_m11_structure2_face_capture_target_matches(
    const Nexus_V1_LauncherM11Structure2FaceCaptureReplayTarget *left,
    const Nexus_V1_LauncherM11Structure2FaceCaptureReplayTarget *right)
{
    const Nexus_V1_DgnM11Structure2FaceDescriptorIntakeReceipt *a;
    const Nexus_V1_DgnM11Structure2FaceDescriptorIntakeReceipt *b;

    if (!left || !right || !left->valid || !right->valid ||
        !left->direct_source_rehashed || !right->direct_source_rehashed ||
        !left->original_saturn_capture_required ||
        !right->original_saturn_capture_required || !left->no_draw_only ||
        !right->no_draw_only || left->payload_materialized ||
        right->payload_materialized || left->fallback_visuals_permitted ||
        right->fallback_visuals_permitted || !left->blocks_real_dgn_mesh_render ||
        !right->blocks_real_dgn_mesh_render ||
        left->route_epoch != right->route_epoch ||
        left->package_fnv1a64 != right->package_fnv1a64 ||
        left->card_fnv1a64 != right->card_fnv1a64 ||
        left->level_index != right->level_index ||
        left->dgn_fnv1a64 != right->dgn_fnv1a64 ||
        left->dgn_byte_count != right->dgn_byte_count) return 0;
    a = &left->face_descriptor;
    b = &right->face_descriptor;
    return a->valid && b->valid && a->route_epoch == b->route_epoch &&
        a->package_fnv1a64 == b->package_fnv1a64 &&
        a->structure1f_entry_index == b->structure1f_entry_index &&
        a->structure3_entry_index == b->structure3_entry_index &&
        a->face_ordinal == b->face_ordinal && a->face_offset == b->face_offset &&
        a->face_length == b->face_length && a->face_fnv1a64 == b->face_fnv1a64 &&
        a->structure2_descriptor_index == b->structure2_descriptor_index &&
        a->descriptor_offset == b->descriptor_offset &&
        a->descriptor_length == b->descriptor_length &&
        a->descriptor_fnv1a64 == b->descriptor_fnv1a64 &&
        a->image_candidate_offset == b->image_candidate_offset &&
        a->image_candidate_length == b->image_candidate_length &&
        a->image_candidate_fnv1a64 == b->image_candidate_fnv1a64 &&
        a->palette_candidate_offset == b->palette_candidate_offset &&
        a->palette_candidate_length == b->palette_candidate_length &&
        a->palette_candidate_fnv1a64 == b->palette_candidate_fnv1a64 &&
        a->face_descriptor_bound && b->face_descriptor_bound &&
        a->candidates_opaque && b->candidates_opaque && a->no_draw_only &&
        b->no_draw_only && !a->fallback_visuals_permitted &&
        !b->fallback_visuals_permitted;
}

static uint32_t nexus_v1_launcher_capture_be32(const uint8_t *bytes)
{
    return ((uint32_t)bytes[0] << 24) | ((uint32_t)bytes[1] << 16) |
        ((uint32_t)bytes[2] << 8) | (uint32_t)bytes[3];
}

static uint64_t nexus_v1_launcher_capture_be64(const uint8_t *bytes)
{
    uint64_t value = 0U;
    int index;

    for (index = 0; index < 8; ++index)
        value = (value << 8) | bytes[index];
    return value;
}

static uint64_t nexus_v1_launcher_capture_fnv1a64(const uint8_t *bytes,
                                                   size_t byte_count)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t index;

    if (!bytes || byte_count == 0U) return 0U;
    for (index = 0U; index < byte_count; ++index) {
        hash ^= bytes[index];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static int nexus_v1_launcher_sha256_text_valid(const char *text)
{
    size_t index;

    if (!text || strlen(text) != 64U) return 0;
    for (index = 0U; index < 64U; ++index) {
        if (!((text[index] >= '0' && text[index] <= '9') ||
              (text[index] >= 'a' && text[index] <= 'f') ||
              (text[index] >= 'A' && text[index] <= 'F'))) return 0;
    }
    return 1;
}

int nexus_v1_launcher_verify_m11_prs3_material_local_artifact(
    const Nexus_V1_LauncherSaturnCardM11NoDrawStartupReceipt *route,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    Nexus_V1_LauncherM11Prs3MaterialLocalArtifactReceipt *out_receipt)
{
    Nexus_V1_LauncherM11Prs3MaterialLocalArtifactReceipt receipt;
    const Nexus_V1_LauncherMenuBpkNoDrawPresentationReceipt *presentation;
    uint32_t payload_offset, payload_length;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    if (!route || !route->valid || !route->opaque_saturn_card_only ||
        !route->card.valid || !route->card.opaque_only || !route->no_draw_only ||
        !route->draw_disabled || !route->route_epoch || !route->package_fnv1a64 ||
        !route->card_fnv1a64 || !route->m11.valid || !route->m11.no_draw_only ||
        !route->m11.draw_disabled || route->m11.route_epoch != route->route_epoch ||
        route->m11.package_fnv1a64 != route->package_fnv1a64 ||
        route->card.card_fnv1a64 != route->card_fnv1a64 ||
        route->card.package_fnv1a64 != route->package_fnv1a64 ||
        route->card.route_epoch != route->route_epoch ||
        !route->m11.presentation.valid || !route->m11.presentation.no_draw_only ||
        !route->m11.presentation.compression.valid ||
        route->m11.presentation.compression.decoder_promoted ||
        route->m11.presentation.compression.pixels_exposed ||
        route->m11.presentation.compression.fallback_visuals_permitted ||
        !capture_bytes ||
        capture_byte_count < NEXUS_V1_M11_PRS3_MATERIAL_CAPTURE_HEADER_BYTES ||
        memcmp(capture_bytes, NEXUS_V1_M11_PRS3_MATERIAL_CAPTURE_MAGIC, 8U) != 0 ||
        nexus_v1_launcher_capture_be32(capture_bytes + 8U) !=
            NEXUS_V1_M11_PRS3_MATERIAL_CAPTURE_VERSION ||
        nexus_v1_launcher_capture_be32(capture_bytes + 12U) !=
            NEXUS_V1_M11_PRS3_MATERIAL_CAPTURE_HEADER_BYTES) {
        *out_receipt = receipt;
        return 0;
    }
    presentation = &route->m11.presentation;
    payload_offset = nexus_v1_launcher_capture_be32(capture_bytes + 64U);
    payload_length = nexus_v1_launcher_capture_be32(capture_bytes + 68U);
    receipt.route_bound =
        nexus_v1_launcher_capture_be64(capture_bytes + 16U) == route->route_epoch &&
        nexus_v1_launcher_capture_be64(capture_bytes + 24U) == route->package_fnv1a64 &&
        nexus_v1_launcher_capture_be64(capture_bytes + 32U) == route->card_fnv1a64;
    receipt.entry_bound = nexus_v1_launcher_capture_be32(capture_bytes + 40U) ==
        presentation->entry_index;
    receipt.body_bound =
        nexus_v1_launcher_capture_be32(capture_bytes + 44U) ==
            presentation->compression.compressed_offset &&
        nexus_v1_launcher_capture_be32(capture_bytes + 48U) ==
            presentation->compression.compressed_length &&
        nexus_v1_launcher_capture_be64(capture_bytes + 56U) ==
            presentation->compression.compressed_fnv1a64;
    receipt.declared_output_bound = nexus_v1_launcher_capture_be32(capture_bytes + 52U) ==
        presentation->compression.declared_output_bytes;
    receipt.payload_bounds_bound = payload_offset ==
            NEXUS_V1_M11_PRS3_MATERIAL_CAPTURE_HEADER_BYTES &&
        payload_length > 0U && payload_offset <= capture_byte_count &&
        payload_length <= capture_byte_count - payload_offset &&
        payload_offset + (size_t)payload_length == capture_byte_count;
    receipt.payload_hash_bound = receipt.payload_bounds_bound &&
        nexus_v1_launcher_capture_be64(capture_bytes + 72U) ==
            nexus_v1_launcher_capture_fnv1a64(capture_bytes + payload_offset,
                                              payload_length);
    if (!receipt.route_bound || !receipt.entry_bound || !receipt.body_bound ||
        !receipt.declared_output_bound || !receipt.payload_bounds_bound ||
        !receipt.payload_hash_bound || !nexus_v1_launcher_capture_be64(
            capture_bytes + 80U) || !nexus_v1_launcher_capture_be64(
            capture_bytes + 88U)) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.capture_fnv1a64 = nexus_v1_launcher_capture_fnv1a64(capture_bytes,
                                                                  capture_byte_count);
    receipt.capture_byte_count = capture_byte_count;
    receipt.payload_opaque = 1;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_launcher_admit_m12_m11_prs3_material_capture_required(
    const Nexus_V1_LauncherSaturnCardM11NoDrawStartupReceipt *route,
    const char *bios_sha256, const char *disc_sha256,
    Nexus_V1_LauncherM12M11Prs3MaterialCaptureRouteReceipt *out_receipt)
{
    Nexus_V1_LauncherM12M11Prs3MaterialCaptureRouteReceipt receipt;
    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt)); receipt.capture_required = receipt.operator_only = receipt.no_draw_only = 1;
    if (!route || !route->valid || !route->opaque_saturn_card_only || !route->no_draw_only ||
        !route->draw_disabled || !route->m11.valid || !route->m11.presentation.valid ||
        !route->m11.presentation.compression.valid || !nexus_v1_launcher_sha256_text_valid(bios_sha256) ||
        !nexus_v1_launcher_sha256_text_valid(disc_sha256)) { *out_receipt = receipt; return 0; }
    receipt.valid = 1; receipt.route = *route;
    memcpy(receipt.bios_sha256, bios_sha256, 64U); receipt.bios_sha256[64] = '\0';
    memcpy(receipt.disc_sha256, disc_sha256, 64U); receipt.disc_sha256[64] = '\0';
    *out_receipt = receipt; return 1;
}

int nexus_v1_launcher_resume_m12_m11_prs3_material_capture(
    const Nexus_V1_LauncherM12M11Prs3MaterialCaptureRouteReceipt *route,
    const char *bios_sha256, const char *disc_sha256,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    Nexus_V1_LauncherM12M11Prs3MaterialCaptureRouteReceipt *out_receipt)
{
    Nexus_V1_LauncherM12M11Prs3MaterialCaptureRouteReceipt receipt;
    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt)); receipt.capture_required = receipt.operator_only = receipt.no_draw_only = 1;
    if (!route || !route->valid || !route->capture_required || route->capture_imported ||
        !route->operator_only || !route->no_draw_only || route->decoder_permitted ||
        route->fallback_visuals_permitted || !bios_sha256 || !disc_sha256 ||
        strcmp(route->bios_sha256, bios_sha256) || strcmp(route->disc_sha256, disc_sha256) ||
        !nexus_v1_launcher_verify_m11_prs3_material_local_artifact(&route->route,
            capture_bytes, capture_byte_count, &receipt.capture)) { *out_receipt = receipt; return 0; }
    receipt = *route; receipt.capture_required = 0; receipt.capture_imported = receipt.resume_ready = 1;
    *out_receipt = receipt; return 1;
}

int nexus_v1_launcher_prs3_startup_state(
    const Nexus_V1_MenuBpkRendererHandoffReceipt *handoff,
    const Nexus_V1_LauncherM12M11Prs3MaterialCaptureRouteReceipt *capture,
    Nexus_V1_LauncherPrs3StartupStateReceipt *out_receipt)
{
    Nexus_V1_LauncherPrs3StartupStateReceipt receipt;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.state = NEXUS_V1_LAUNCHER_PRS3_STARTUP_INVALID;
    receipt.status = "menu-bpk-prs3-invalid";
    if (!handoff) {
        *out_receipt = receipt;
        return 0;
    }
    if (handoff->status != NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_BLOCKED_PRS3) {
        receipt.valid = receipt.scan_complete = receipt.startup_can_settle = 1;
        receipt.state = NEXUS_V1_LAUNCHER_PRS3_STARTUP_NOT_REQUIRED;
        receipt.status = "menu-bpk-prs3-not-required";
        *out_receipt = receipt;
        return 1;
    }

    receipt.valid = receipt.scan_complete = receipt.startup_can_settle = 1;
    receipt.no_draw_only = 1;
    if (capture && capture->valid && !capture->capture_required &&
        capture->capture_imported && capture->resume_ready &&
        capture->operator_only && capture->no_draw_only &&
        !capture->decoder_permitted && !capture->fallback_visuals_permitted &&
        capture->capture.valid && capture->capture.payload_opaque &&
        capture->capture.no_draw_only && !capture->capture.decoder_permitted &&
        !capture->capture.fallback_visuals_permitted) {
        receipt.state = NEXUS_V1_LAUNCHER_PRS3_STARTUP_CAPTURE_CONSUMED;
        receipt.capture_consumed = 1;
        receipt.status = "menu-bpk-prs3-capture-consumed";
    } else {
        receipt.state = NEXUS_V1_LAUNCHER_PRS3_STARTUP_CAPTURE_REQUIRED;
        receipt.capture_required = 1;
        receipt.status = "menu-bpk-prs3-capture-required";
    }
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_launcher_m12_prs3_startup_transition(
    const Nexus_V1_MenuBpkRendererHandoffReceipt *handoff,
    const Nexus_V1_LauncherM12M11Prs3MaterialCaptureRouteReceipt *capture,
    Nexus_V1_LauncherPrs3StartupAction action,
    Nexus_V1_LauncherM12Prs3StartupTransitionReceipt *out_receipt)
{
    Nexus_V1_LauncherM12Prs3StartupTransitionReceipt receipt;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (action != NEXUS_V1_LAUNCHER_PRS3_STARTUP_ACTION_PRESENT &&
        action != NEXUS_V1_LAUNCHER_PRS3_STARTUP_ACTION_RETURN_TO_IDLE &&
        action != NEXUS_V1_LAUNCHER_PRS3_STARTUP_ACTION_RESCAN) {
        *out_receipt = receipt;
        return 0;
    }
    if (!nexus_v1_launcher_prs3_startup_state(handoff, capture,
                                               &receipt.prs3)) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = receipt.m12_handled = 1;
    receipt.terminal = receipt.prs3.capture_required ||
        receipt.prs3.capture_consumed;
    receipt.return_to_idle = receipt.terminal &&
        action == NEXUS_V1_LAUNCHER_PRS3_STARTUP_ACTION_RETURN_TO_IDLE;
    receipt.rescan_complete = receipt.terminal &&
        action == NEXUS_V1_LAUNCHER_PRS3_STARTUP_ACTION_RESCAN;
    receipt.no_draw_only = receipt.prs3.no_draw_only;
    receipt.status = receipt.prs3.status;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_launcher_admit_m11_structure2_face_capture_replay(
    Nexus_V1_Engine *engine, const Nexus_V1_LevCorpusDiscoveryReceipt *corpus,
    const Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt *handoff,
    const Nexus_V1_LauncherM11Structure2FaceCaptureReplayTarget *target,
    const Nexus_V1_LauncherM11Structure2FaceCaptureReplayEvidence *evidence,
    Nexus_V1_LauncherM11Structure2FaceCaptureReplayReceipt *out_receipt)
{
    Nexus_V1_LauncherM11Structure2FaceCaptureReplayReceipt receipt;
    Nexus_V1_LauncherM11Structure2FaceCaptureReplayTarget current;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    memset(&current, 0, sizeof(current));
    if (!target || !evidence || !evidence->original_saturn_capture_verified ||
        !evidence->replay_sequence || !evidence->trace_fnv1a64 ||
        !evidence->trace_byte_count || !evidence->vdp1_command_fnv1a64 ||
        !evidence->vdp1_command_byte_count ||
        !nexus_v1_launcher_build_m11_structure2_face_capture_replay_target(
            engine, corpus, handoff, &current) ||
        !nexus_v1_launcher_m11_structure2_face_capture_target_matches(
            target, &current) ||
        evidence->replay_sequence != target->route_epoch ||
        evidence->texture_candidate_fnv1a64 !=
            target->face_descriptor.image_candidate_fnv1a64 ||
        evidence->palette_candidate_fnv1a64 !=
            target->face_descriptor.palette_candidate_fnv1a64) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.target = current;
    receipt.trace_fnv1a64 = evidence->trace_fnv1a64;
    receipt.trace_byte_count = evidence->trace_byte_count;
    receipt.vdp1_command_fnv1a64 = evidence->vdp1_command_fnv1a64;
    receipt.vdp1_command_byte_count = evidence->vdp1_command_byte_count;
    receipt.texture_candidate_bound = 1;
    receipt.palette_candidate_bound =
        current.face_descriptor.palette_candidate_fnv1a64 != 0U;
    receipt.vdp1_binding_observed = 1;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_launcher_import_m11_structure2_face_vdp1_capture(
    Nexus_V1_Engine *engine, const Nexus_V1_LevCorpusDiscoveryReceipt *corpus,
    const Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt *handoff,
    const Nexus_V1_LauncherM11Structure2FaceCaptureReplayTarget *target,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    Nexus_V1_LauncherM11Structure2FaceVdp1CaptureImportReceipt *out_receipt)
{
    Nexus_V1_LauncherM11Structure2FaceVdp1CaptureImportReceipt receipt;
    Nexus_V1_LauncherM11Structure2FaceCaptureReplayTarget current;
    uint32_t header_bytes;
    uint32_t payload_offset;
    uint32_t payload_length;
    uint64_t payload_fnv1a64;
    uint64_t trace_fnv1a64;
    uint64_t trace_byte_count;
    uint64_t command_fnv1a64;
    uint64_t command_byte_count;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    memset(&current, 0, sizeof(current));
    if (!target || !capture_bytes ||
        capture_byte_count < NEXUS_V1_M11_STRUCTURE2_FACE_VDP1_CAPTURE_HEADER_BYTES ||
        memcmp(capture_bytes, NEXUS_V1_M11_STRUCTURE2_FACE_VDP1_CAPTURE_MAGIC,
               8U) != 0 ||
        nexus_v1_launcher_capture_be32(capture_bytes + 8U) !=
            NEXUS_V1_M11_STRUCTURE2_FACE_VDP1_CAPTURE_VERSION ||
        (header_bytes = nexus_v1_launcher_capture_be32(capture_bytes + 12U)) !=
            NEXUS_V1_M11_STRUCTURE2_FACE_VDP1_CAPTURE_HEADER_BYTES ||
        header_bytes > capture_byte_count ||
        !nexus_v1_launcher_build_m11_structure2_face_capture_replay_target(
            engine, corpus, handoff, &current) ||
        !nexus_v1_launcher_m11_structure2_face_capture_target_matches(
            target, &current) ||
        nexus_v1_launcher_capture_be64(capture_bytes + 16U) !=
            current.route_epoch ||
        nexus_v1_launcher_capture_be64(capture_bytes + 24U) !=
            current.package_fnv1a64 ||
        nexus_v1_launcher_capture_be64(capture_bytes + 32U) !=
            current.card_fnv1a64 ||
        nexus_v1_launcher_capture_be64(capture_bytes + 40U) !=
            current.dgn_fnv1a64 ||
        nexus_v1_launcher_capture_be64(capture_bytes + 48U) !=
            current.dgn_byte_count ||
        nexus_v1_launcher_capture_be64(capture_bytes + 56U) !=
            current.face_descriptor.face_fnv1a64 ||
        nexus_v1_launcher_capture_be64(capture_bytes + 64U) !=
            current.face_descriptor.descriptor_fnv1a64 ||
        nexus_v1_launcher_capture_be64(capture_bytes + 72U) !=
            current.face_descriptor.image_candidate_fnv1a64 ||
        nexus_v1_launcher_capture_be64(capture_bytes + 80U) !=
            current.face_descriptor.palette_candidate_fnv1a64) {
        *out_receipt = receipt;
        return 0;
    }
    payload_offset = nexus_v1_launcher_capture_be32(capture_bytes + 88U);
    payload_length = nexus_v1_launcher_capture_be32(capture_bytes + 92U);
    payload_fnv1a64 = nexus_v1_launcher_capture_be64(capture_bytes + 96U);
    trace_fnv1a64 = nexus_v1_launcher_capture_be64(capture_bytes + 104U);
    trace_byte_count = nexus_v1_launcher_capture_be64(capture_bytes + 112U);
    command_fnv1a64 = nexus_v1_launcher_capture_be64(capture_bytes + 120U);
    command_byte_count = nexus_v1_launcher_capture_be64(capture_bytes + 128U);
    if (payload_offset != header_bytes || !payload_length || !payload_fnv1a64 ||
        payload_offset > capture_byte_count ||
        payload_length > capture_byte_count - payload_offset ||
        payload_offset + (size_t)payload_length != capture_byte_count ||
        nexus_v1_launcher_capture_fnv1a64(capture_bytes + payload_offset,
                                          payload_length) != payload_fnv1a64 ||
        !trace_fnv1a64 || !trace_byte_count || !command_fnv1a64 ||
        !command_byte_count) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.target = current;
    receipt.capture_fnv1a64 = nexus_v1_launcher_capture_fnv1a64(
        capture_bytes, capture_byte_count);
    receipt.capture_byte_count = capture_byte_count;
    receipt.payload_offset = payload_offset;
    receipt.payload_length = payload_length;
    receipt.payload_fnv1a64 = payload_fnv1a64;
    receipt.trace_fnv1a64 = trace_fnv1a64;
    receipt.trace_byte_count = trace_byte_count;
    receipt.vdp1_command_fnv1a64 = command_fnv1a64;
    receipt.vdp1_command_byte_count = command_byte_count;
    receipt.header_version_bound = 1;
    receipt.payload_bounds_bound = 1;
    receipt.payload_hash_bound = 1;
    receipt.target_bound = 1;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_launcher_build_m11_structure3_topology_capture_replay_target(
    Nexus_V1_Engine *engine, const Nexus_V1_LevCorpusDiscoveryReceipt *corpus,
    const Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt *handoff,
    Nexus_V1_LauncherM11Structure3TopologyCaptureReplayTarget *out_target)
{
    Nexus_V1_LauncherM11Structure3TopologyCaptureReplayTarget target;
    const Nexus_V1_LevCorpusDirectLevelIdentity *level;
    Nexus_V1_DgnM11DirectLevNoDrawReceipt dungeon;

    if (!out_target) return 0;
    memset(&target, 0, sizeof(target));
    target.no_draw_only = 1;
    target.blocks_real_dgn_mesh_render = 1;
    if (!engine || !corpus || !corpus->valid || corpus->payload_materialized ||
        !corpus->direct_files_only || !handoff || !handoff->valid ||
        !handoff->no_draw_only || !handoff->draw_disabled ||
        !handoff->route_epoch || !handoff->package_fnv1a64 ||
        !handoff->card_fnv1a64 ||
        handoff->level_index >= NEXUS_V1_LEV_CORPUS_LEVEL_COUNT ||
        !handoff->champion_startup.valid ||
        !handoff->champion_startup.opaque_saturn_card_only ||
        !handoff->champion_startup.no_draw_only ||
        handoff->champion_startup.draw_disabled == 0 ||
        handoff->champion_startup.route_epoch != handoff->route_epoch ||
        handoff->champion_startup.package_fnv1a64 != handoff->package_fnv1a64 ||
        handoff->champion_startup.card_fnv1a64 != handoff->card_fnv1a64) {
        *out_target = target;
        return 0;
    }
    level = &corpus->levels[handoff->level_index];
    memset(&dungeon, 0, sizeof(dungeon));
    if (!level->valid || !nexus_v1_lev_corpus_direct_identity_still_matches(level) ||
        !nexus_v1_engine_m11_direct_lev_dungeon_no_draw_ready(
            engine, handoff->route_epoch, (int)handoff->level_index, level->md5,
            level->byte_count, level->fnv1a64, &dungeon) || !dungeon.valid ||
        !dungeon.no_draw_only || dungeon.fallback_visuals_permitted ||
        !dungeon.blocks_real_dgn_mesh_render ||
        dungeon.route_epoch != handoff->route_epoch ||
        dungeon.level_index != (int)handoff->level_index ||
        dungeon.dgn_byte_count != level->byte_count ||
        dungeon.dgn_fnv1a64 != level->fnv1a64 ||
        !dungeon.structure3_topology_descriptor.valid ||
        dungeon.structure3_topology_descriptor.route_epoch != handoff->route_epoch ||
        dungeon.structure3_topology_descriptor.level_index != handoff->level_index ||
        dungeon.structure3_topology_descriptor.package_fnv1a64 != level->fnv1a64 ||
        !dungeon.structure3_topology_descriptor.topology_framing_bound ||
        !dungeon.structure3_topology_descriptor.capture_required ||
        !dungeon.structure3_topology_descriptor.no_draw_only ||
        dungeon.structure3_topology_descriptor.fallback_visuals_permitted ||
        !dungeon.structure3_topology_descriptor.blocks_real_dgn_mesh_render ||
        !dungeon.structure3_topology_descriptor.vertex_table_length ||
        !dungeon.structure3_topology_descriptor.vertex_table_fnv1a64 ||
        !dungeon.structure3_topology_descriptor.referenced_vertex_rows_fnv1a64 ||
        dungeon.structure3_topology_descriptor.normal_length != 12U ||
        !dungeon.structure3_topology_descriptor.normal_fnv1a64) {
        *out_target = target;
        return 0;
    }
    target.valid = 1;
    target.route_epoch = handoff->route_epoch;
    target.package_fnv1a64 = handoff->package_fnv1a64;
    target.card_fnv1a64 = handoff->card_fnv1a64;
    target.level_index = handoff->level_index;
    target.dgn_fnv1a64 = level->fnv1a64;
    target.dgn_byte_count = level->byte_count;
    target.topology = dungeon.structure3_topology_descriptor;
    target.direct_source_rehashed = 1;
    target.original_saturn_capture_required = 1;
    *out_target = target;
    return 1;
}

static int nexus_v1_launcher_m11_structure3_topology_capture_target_matches(
    const Nexus_V1_LauncherM11Structure3TopologyCaptureReplayTarget *left,
    const Nexus_V1_LauncherM11Structure3TopologyCaptureReplayTarget *right)
{
    const Nexus_V1_DgnM11Structure3TopologyDescriptorIntakeReceipt *a;
    const Nexus_V1_DgnM11Structure3TopologyDescriptorIntakeReceipt *b;

    if (!left || !right || !left->valid || !right->valid ||
        !left->direct_source_rehashed || !right->direct_source_rehashed ||
        !left->original_saturn_capture_required ||
        !right->original_saturn_capture_required || !left->no_draw_only ||
        !right->no_draw_only || left->payload_materialized ||
        right->payload_materialized || left->fallback_visuals_permitted ||
        right->fallback_visuals_permitted || !left->blocks_real_dgn_mesh_render ||
        !right->blocks_real_dgn_mesh_render ||
        left->route_epoch != right->route_epoch ||
        left->package_fnv1a64 != right->package_fnv1a64 ||
        left->card_fnv1a64 != right->card_fnv1a64 ||
        left->level_index != right->level_index ||
        left->dgn_fnv1a64 != right->dgn_fnv1a64 ||
        left->dgn_byte_count != right->dgn_byte_count) return 0;
    a = &left->topology;
    b = &right->topology;
    return a->valid && b->valid && a->route_epoch == b->route_epoch &&
        a->package_fnv1a64 == b->package_fnv1a64 &&
        a->structure1f_entry_index == b->structure1f_entry_index &&
        a->structure3_entry_index == b->structure3_entry_index &&
        a->face_ordinal == b->face_ordinal && a->face_offset == b->face_offset &&
        a->face_length == b->face_length && a->face_fnv1a64 == b->face_fnv1a64 &&
        a->vertex_table_offset == b->vertex_table_offset &&
        a->vertex_table_length == b->vertex_table_length &&
        a->vertex_table_fnv1a64 == b->vertex_table_fnv1a64 &&
        a->vertex_count == b->vertex_count &&
        a->face_vertex_index_count == b->face_vertex_index_count &&
        a->referenced_vertex_rows_fnv1a64 == b->referenced_vertex_rows_fnv1a64 &&
        a->normal_offset == b->normal_offset && a->normal_length == b->normal_length &&
        a->normal_fnv1a64 == b->normal_fnv1a64 &&
        a->topology_framing_bound && b->topology_framing_bound &&
        a->capture_required && b->capture_required && a->no_draw_only &&
        b->no_draw_only && !a->fallback_visuals_permitted &&
        !b->fallback_visuals_permitted;
}

int nexus_v1_launcher_import_m11_structure3_topology_capture(
    Nexus_V1_Engine *engine, const Nexus_V1_LevCorpusDiscoveryReceipt *corpus,
    const Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt *handoff,
    const Nexus_V1_LauncherM11Structure3TopologyCaptureReplayTarget *target,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    Nexus_V1_LauncherM11Structure3TopologyCaptureImportReceipt *out_receipt)
{
    Nexus_V1_LauncherM11Structure3TopologyCaptureImportReceipt receipt;
    Nexus_V1_LauncherM11Structure3TopologyCaptureReplayTarget current;
    uint32_t header_bytes;
    uint32_t payload_offset;
    uint32_t payload_length;
    uint64_t payload_fnv1a64;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    memset(&current, 0, sizeof(current));
    if (!target || !capture_bytes ||
        capture_byte_count < NEXUS_V1_M11_STRUCTURE3_TOPOLOGY_CAPTURE_HEADER_BYTES ||
        memcmp(capture_bytes, NEXUS_V1_M11_STRUCTURE3_TOPOLOGY_CAPTURE_MAGIC,
               8U) != 0 ||
        nexus_v1_launcher_capture_be32(capture_bytes + 8U) !=
            NEXUS_V1_M11_STRUCTURE3_TOPOLOGY_CAPTURE_VERSION ||
        (header_bytes = nexus_v1_launcher_capture_be32(capture_bytes + 12U)) !=
            NEXUS_V1_M11_STRUCTURE3_TOPOLOGY_CAPTURE_HEADER_BYTES ||
        header_bytes > capture_byte_count ||
        !nexus_v1_launcher_build_m11_structure3_topology_capture_replay_target(
            engine, corpus, handoff, &current) ||
        !nexus_v1_launcher_m11_structure3_topology_capture_target_matches(
            target, &current) ||
        nexus_v1_launcher_capture_be64(capture_bytes + 16U) != current.route_epoch ||
        nexus_v1_launcher_capture_be64(capture_bytes + 24U) != current.package_fnv1a64 ||
        nexus_v1_launcher_capture_be64(capture_bytes + 32U) != current.card_fnv1a64 ||
        nexus_v1_launcher_capture_be64(capture_bytes + 40U) != current.dgn_fnv1a64 ||
        nexus_v1_launcher_capture_be64(capture_bytes + 48U) != current.dgn_byte_count ||
        nexus_v1_launcher_capture_be32(capture_bytes + 56U) !=
            (uint32_t)current.topology.structure1f_entry_index ||
        nexus_v1_launcher_capture_be32(capture_bytes + 60U) !=
            current.topology.structure3_entry_index ||
        nexus_v1_launcher_capture_be32(capture_bytes + 64U) !=
            current.topology.face_ordinal ||
        nexus_v1_launcher_capture_be32(capture_bytes + 68U) !=
            current.topology.vertex_table_offset ||
        nexus_v1_launcher_capture_be32(capture_bytes + 72U) !=
            current.topology.vertex_table_length ||
        nexus_v1_launcher_capture_be64(capture_bytes + 76U) !=
            current.topology.vertex_table_fnv1a64 ||
        nexus_v1_launcher_capture_be64(capture_bytes + 84U) !=
            current.topology.referenced_vertex_rows_fnv1a64 ||
        nexus_v1_launcher_capture_be32(capture_bytes + 92U) !=
            current.topology.normal_offset ||
        nexus_v1_launcher_capture_be32(capture_bytes + 96U) !=
            current.topology.normal_length ||
        nexus_v1_launcher_capture_be64(capture_bytes + 100U) !=
            current.topology.normal_fnv1a64) {
        *out_receipt = receipt;
        return 0;
    }
    payload_offset = nexus_v1_launcher_capture_be32(capture_bytes + 108U);
    payload_length = nexus_v1_launcher_capture_be32(capture_bytes + 112U);
    payload_fnv1a64 = nexus_v1_launcher_capture_be64(capture_bytes + 116U);
    if (payload_offset != header_bytes || !payload_length || !payload_fnv1a64 ||
        payload_offset > capture_byte_count ||
        payload_length > capture_byte_count - payload_offset ||
        payload_offset + (size_t)payload_length != capture_byte_count ||
        nexus_v1_launcher_capture_fnv1a64(capture_bytes + payload_offset,
                                           payload_length) != payload_fnv1a64) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.target = current;
    receipt.capture_fnv1a64 = nexus_v1_launcher_capture_fnv1a64(
        capture_bytes, capture_byte_count);
    receipt.capture_byte_count = capture_byte_count;
    receipt.payload_offset = payload_offset;
    receipt.payload_length = payload_length;
    receipt.payload_fnv1a64 = payload_fnv1a64;
    receipt.header_version_bound = 1;
    receipt.payload_bounds_bound = 1;
    receipt.payload_hash_bound = 1;
    receipt.target_bound = 1;
    receipt.topology_opaque = 1;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_launcher_admit_m12_m11_vdp1_capture_required(
    Nexus_V1_Engine *engine, const Nexus_V1_LevCorpusDiscoveryReceipt *corpus,
    const Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt *handoff,
    const char *bios_sha256, Nexus_V1_LauncherSaturnBiosRegion bios_region,
    const char *disc_sha256,
    const Nexus_V1_LauncherM11Structure2FaceCaptureReplayTarget *target,
    Nexus_V1_LauncherM12M11Vdp1CaptureRouteReceipt *out_receipt)
{
    Nexus_V1_LauncherM12M11Vdp1CaptureRouteReceipt receipt;
    Nexus_V1_LauncherM11Structure2FaceCaptureReplayTarget current;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.capture_required = 1;
    receipt.operator_only = 1;
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    memset(&current, 0, sizeof(current));
    if (!nexus_v1_launcher_sha256_text_valid(bios_sha256) ||
        !nexus_v1_launcher_sha256_text_valid(disc_sha256) ||
        bios_region == NEXUS_V1_LAUNCHER_SATURN_BIOS_REGION_INVALID ||
        bios_region > NEXUS_V1_LAUNCHER_SATURN_BIOS_REGION_EU ||
        !target ||
        !nexus_v1_launcher_build_m11_structure2_face_capture_replay_target(
            engine, corpus, handoff, &current) ||
        !nexus_v1_launcher_m11_structure2_face_capture_target_matches(
            target, &current)) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.bios_region = bios_region;
    memcpy(receipt.bios_sha256, bios_sha256, 64U);
    receipt.bios_sha256[64] = '\0';
    memcpy(receipt.disc_sha256, disc_sha256, 64U);
    receipt.disc_sha256[64] = '\0';
    receipt.target = current;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_launcher_resume_m12_m11_vdp1_capture(
    Nexus_V1_Engine *engine, const Nexus_V1_LevCorpusDiscoveryReceipt *corpus,
    const Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt *handoff,
    const Nexus_V1_LauncherM12M11Vdp1CaptureRouteReceipt *route,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    Nexus_V1_LauncherM12M11Vdp1CaptureRouteReceipt *out_receipt)
{
    Nexus_V1_LauncherM12M11Vdp1CaptureRouteReceipt receipt;
    Nexus_V1_LauncherM12M11Vdp1CaptureRouteReceipt current;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.capture_required = 1;
    receipt.operator_only = 1;
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    memset(&current, 0, sizeof(current));
    if (!route || !route->valid || !route->capture_required ||
        route->capture_imported || !route->operator_only ||
        route->decoder_permitted || route->fallback_visuals_permitted ||
        !route->no_draw_only || !route->blocks_real_dgn_mesh_render ||
        !nexus_v1_launcher_admit_m12_m11_vdp1_capture_required(
            engine, corpus, handoff, route->bios_sha256, route->bios_region,
            route->disc_sha256, &route->target, &current)) {
        *out_receipt = receipt;
        return 0;
    }
    receipt = current;
    receipt.capture = (Nexus_V1_LauncherM11Structure2FaceVdp1CaptureImportReceipt){0};
    if (!nexus_v1_launcher_import_m11_structure2_face_vdp1_capture(
            engine, corpus, handoff, &receipt.target, capture_bytes,
            capture_byte_count, &receipt.capture)) {
        receipt.capture_required = 1;
        receipt.valid = 0;
        *out_receipt = receipt;
        return 0;
    }
    receipt.capture_required = 0;
    receipt.capture_imported = 1;
    receipt.resume_ready = 1;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_launcher_verify_m11_structure2_face_vdp1_local_artifact(
    const Nexus_V1_LauncherM12M11Vdp1CaptureRouteReceipt *route,
    const char *bios_sha256, const char *disc_sha256,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    Nexus_V1_LauncherM11Structure2FaceVdp1LocalArtifactReceipt *out_receipt)
{
    Nexus_V1_LauncherM11Structure2FaceVdp1LocalArtifactReceipt receipt;
    const Nexus_V1_LauncherM11Structure2FaceCaptureReplayTarget *target;
    uint32_t payload_offset, payload_length;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    if (!route || !route->valid || !route->capture_required ||
        route->capture_imported || !route->operator_only ||
        route->decoder_permitted || route->fallback_visuals_permitted ||
        !route->no_draw_only || !route->blocks_real_dgn_mesh_render ||
        !nexus_v1_launcher_sha256_text_valid(bios_sha256) ||
        !nexus_v1_launcher_sha256_text_valid(disc_sha256) ||
        strcmp(route->bios_sha256, bios_sha256) != 0 ||
        strcmp(route->disc_sha256, disc_sha256) != 0 || !capture_bytes ||
        capture_byte_count < NEXUS_V1_M11_STRUCTURE2_FACE_VDP1_CAPTURE_HEADER_BYTES ||
        memcmp(capture_bytes, NEXUS_V1_M11_STRUCTURE2_FACE_VDP1_CAPTURE_MAGIC, 8U) != 0 ||
        nexus_v1_launcher_capture_be32(capture_bytes + 8U) !=
            NEXUS_V1_M11_STRUCTURE2_FACE_VDP1_CAPTURE_VERSION ||
        nexus_v1_launcher_capture_be32(capture_bytes + 12U) !=
            NEXUS_V1_M11_STRUCTURE2_FACE_VDP1_CAPTURE_HEADER_BYTES) {
        *out_receipt = receipt;
        return 0;
    }
    target = &route->target;
    payload_offset = nexus_v1_launcher_capture_be32(capture_bytes + 88U);
    payload_length = nexus_v1_launcher_capture_be32(capture_bytes + 92U);
    receipt.bios_bound = 1;
    receipt.disc_bound = 1;
    receipt.dgn_bound =
        nexus_v1_launcher_capture_be64(capture_bytes + 16U) == target->route_epoch &&
        nexus_v1_launcher_capture_be64(capture_bytes + 24U) == target->package_fnv1a64 &&
        nexus_v1_launcher_capture_be64(capture_bytes + 32U) == target->card_fnv1a64 &&
        nexus_v1_launcher_capture_be64(capture_bytes + 40U) == target->dgn_fnv1a64 &&
        nexus_v1_launcher_capture_be64(capture_bytes + 48U) == target->dgn_byte_count;
    receipt.face_bound = nexus_v1_launcher_capture_be64(capture_bytes + 56U) ==
        target->face_descriptor.face_fnv1a64;
    receipt.descriptor_bound = nexus_v1_launcher_capture_be64(capture_bytes + 64U) ==
        target->face_descriptor.descriptor_fnv1a64;
    receipt.candidates_bound =
        nexus_v1_launcher_capture_be64(capture_bytes + 72U) ==
            target->face_descriptor.image_candidate_fnv1a64 &&
        nexus_v1_launcher_capture_be64(capture_bytes + 80U) ==
            target->face_descriptor.palette_candidate_fnv1a64;
    receipt.payload_bounds_bound = payload_offset ==
            NEXUS_V1_M11_STRUCTURE2_FACE_VDP1_CAPTURE_HEADER_BYTES &&
        payload_length > 0U && payload_offset <= capture_byte_count &&
        payload_length <= capture_byte_count - payload_offset &&
        payload_offset + (size_t)payload_length == capture_byte_count;
    receipt.payload_hash_bound = receipt.payload_bounds_bound &&
        nexus_v1_launcher_capture_be64(capture_bytes + 96U) ==
            nexus_v1_launcher_capture_fnv1a64(capture_bytes + payload_offset,
                                              payload_length);
    receipt.trace_command_bound = nexus_v1_launcher_capture_be64(capture_bytes + 104U) != 0U &&
        nexus_v1_launcher_capture_be64(capture_bytes + 112U) != 0U &&
        nexus_v1_launcher_capture_be64(capture_bytes + 120U) != 0U &&
        nexus_v1_launcher_capture_be64(capture_bytes + 128U) != 0U;
    if (!receipt.dgn_bound || !receipt.face_bound || !receipt.descriptor_bound ||
        !receipt.candidates_bound || !receipt.payload_bounds_bound ||
        !receipt.payload_hash_bound || !receipt.trace_command_bound) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.capture_fnv1a64 = nexus_v1_launcher_capture_fnv1a64(capture_bytes,
                                                                  capture_byte_count);
    receipt.capture_byte_count = capture_byte_count;
    receipt.payload_opaque = 1;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_launcher_resume_m12_m11_vdp1_local_artifact(
    Nexus_V1_Engine *engine, const Nexus_V1_LevCorpusDiscoveryReceipt *corpus,
    const Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt *handoff,
    const Nexus_V1_LauncherM12M11Vdp1CaptureRouteReceipt *route,
    const char *bios_sha256, const char *disc_sha256,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    Nexus_V1_LauncherM12M11Vdp1CaptureRouteReceipt *out_receipt)
{
    Nexus_V1_LauncherM11Structure2FaceVdp1LocalArtifactReceipt preflight;

    if (!out_receipt) return 0;
    memset(&preflight, 0, sizeof(preflight));
    if (!nexus_v1_launcher_verify_m11_structure2_face_vdp1_local_artifact(
            route, bios_sha256, disc_sha256, capture_bytes, capture_byte_count,
            &preflight)) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        out_receipt->capture_required = 1;
        out_receipt->operator_only = 1;
        out_receipt->no_draw_only = 1;
        out_receipt->blocks_real_dgn_mesh_render = 1;
        return 0;
    }
    return nexus_v1_launcher_resume_m12_m11_vdp1_capture(
        engine, corpus, handoff, route, capture_bytes, capture_byte_count,
        out_receipt);
}

static int nexus_v1_launcher_owner_material_target_matches(
    const Nexus_V1_DgnStructure1AStructure3MaterialCaptureTarget *left,
    const Nexus_V1_DgnStructure1AStructure3MaterialCaptureTarget *right)
{
    const Nexus_V1_DgnStructure1AStructure3CaptureTargetReceipt *left_owner;
    const Nexus_V1_DgnStructure1AStructure3CaptureTargetReceipt *right_owner;
    const Nexus_V1_DgnStructure2DescriptorCaptureTarget *left_descriptor;
    const Nexus_V1_DgnStructure2DescriptorCaptureTarget *right_descriptor;

    if (!left || !right || !left->valid || !right->valid) return 0;
    left_owner = &left->owner_face_target;
    right_owner = &right->owner_face_target;
    left_descriptor = &left->material_target.descriptor_target;
    right_descriptor = &right->material_target.descriptor_target;
    return left->level_index == right->level_index &&
        left->material_target.source_byte_count == right->material_target.source_byte_count &&
        left->material_target.source_bytes_fnv1a64 == right->material_target.source_bytes_fnv1a64 &&
        left_owner->owner_x == right_owner->owner_x &&
        left_owner->owner_y == right_owner->owner_y &&
        left_owner->structure1f_entry_index == right_owner->structure1f_entry_index &&
        left_owner->structure1a_index == right_owner->structure1a_index &&
        left_owner->face_target.candidate.entry_index ==
            right_owner->face_target.candidate.entry_index &&
        left_owner->face_target.candidate.face_ordinal ==
            right_owner->face_target.candidate.face_ordinal &&
        left_owner->face_target.candidate.face_row_fnv1a32 ==
            right_owner->face_target.candidate.face_row_fnv1a32 &&
        left_descriptor->descriptor_index == right_descriptor->descriptor_index &&
        left_descriptor->descriptor_bytes_fnv1a64 == right_descriptor->descriptor_bytes_fnv1a64 &&
        left_descriptor->image_payload_candidate_fnv1a64 ==
            right_descriptor->image_payload_candidate_fnv1a64 &&
        left_descriptor->palette_payload_candidate_fnv1a64 ==
            right_descriptor->palette_payload_candidate_fnv1a64;
}

int nexus_v1_launcher_admit_m12_m11_owner_material_capture_required(
    Nexus_V1_Engine *engine, int topology_candidate_index,
    uint32_t structure3_entry_index, uint32_t structure3_face_ordinal,
    const char *bios_sha256, Nexus_V1_LauncherSaturnBiosRegion bios_region,
    const char *disc_sha256,
    const Nexus_V1_DgnStructure1AStructure3MaterialCaptureTarget *target,
    Nexus_V1_LauncherM12M11OwnerMaterialCaptureRouteReceipt *out_receipt)
{
    Nexus_V1_LauncherM12M11OwnerMaterialCaptureRouteReceipt receipt;
    Nexus_V1_DgnStructure1AStructure3MaterialCaptureTarget current;
    Nexus_V1_DgnStructure1AStructure3CaptureTargetRouteReceipt target_route;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.capture_required = 1;
    receipt.operator_only = 1;
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    memset(&current, 0, sizeof(current));
    memset(&target_route, 0, sizeof(target_route));
    if (!nexus_v1_launcher_sha256_text_valid(bios_sha256) ||
        !nexus_v1_launcher_sha256_text_valid(disc_sha256) ||
        bios_region == NEXUS_V1_LAUNCHER_SATURN_BIOS_REGION_INVALID ||
        bios_region > NEXUS_V1_LAUNCHER_SATURN_BIOS_REGION_EU || !target ||
        nexus_v1_engine_build_structure1a_structure3_material_capture_target(
            engine, topology_candidate_index, structure3_entry_index,
            structure3_face_ordinal, &current, &target_route) != 1 ||
        !target_route.active_canonical_lev_bound || !target_route.target_built ||
        !nexus_v1_launcher_owner_material_target_matches(target, &current)) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.bios_region = bios_region;
    memcpy(receipt.bios_sha256, bios_sha256, 64U);
    receipt.bios_sha256[64] = '\0';
    memcpy(receipt.disc_sha256, disc_sha256, 64U);
    receipt.disc_sha256[64] = '\0';
    receipt.topology_candidate_index = topology_candidate_index;
    receipt.structure3_entry_index = structure3_entry_index;
    receipt.structure3_face_ordinal = structure3_face_ordinal;
    receipt.target = current;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_launcher_resume_m12_m11_owner_material_capture(
    Nexus_V1_Engine *engine,
    const Nexus_V1_LauncherM12M11OwnerMaterialCaptureRouteReceipt *route,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    Nexus_V1_LauncherM12M11OwnerMaterialCaptureRouteReceipt *out_receipt)
{
    Nexus_V1_LauncherM12M11OwnerMaterialCaptureRouteReceipt current;
    Nexus_V1_OwnerMaterialCaptureAdmissionReceipt capture;

    if (!out_receipt) return 0;
    memset(&current, 0, sizeof(current));
    current.capture_required = 1;
    current.operator_only = 1;
    current.no_draw_only = 1;
    current.blocks_real_dgn_mesh_render = 1;
    if (!route || !route->valid || !route->capture_required ||
        route->capture_admitted || !route->operator_only ||
        !route->no_draw_only || route->owner_mapping_proven ||
        route->mesh_semantics_permitted || route->texture_semantics_permitted ||
        route->decoder_permitted || route->fallback_visuals_permitted ||
        !route->blocks_real_dgn_mesh_render ||
        !nexus_v1_launcher_admit_m12_m11_owner_material_capture_required(
            engine, route->topology_candidate_index,
            route->structure3_entry_index, route->structure3_face_ordinal,
            route->bios_sha256, route->bios_region, route->disc_sha256,
            &route->target, &current) ||
        !nexus_v1_owner_material_capture_admit(&current.target, capture_bytes,
                                               capture_byte_count, &capture)) {
        *out_receipt = current;
        return 0;
    }
    current.capture = capture;
    current.capture_required = 0;
    current.capture_admitted = 1;
    current.resume_ready = 1;
    *out_receipt = current;
    return 1;
}

int nexus_v1_launcher_import_m12_m11_owner_material_capture(
    Nexus_V1_Engine *engine,
    const Nexus_V1_LauncherM12M11OwnerMaterialCaptureRouteReceipt *route,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    const char *manifest_text, size_t manifest_size,
    const uint8_t *raw_trace, size_t raw_trace_size,
    int original_saturn_capture_verified,
    Nexus_V1_DgnOwnerMaterialTraceAdmissionReceipt *out_trace_receipt,
    Nexus_V1_LauncherM12M11OwnerMaterialCaptureRouteReceipt *out_receipt)
{
    Nexus_V1_LauncherM12M11OwnerMaterialCaptureRouteReceipt current;
    Nexus_V1_OwnerMaterialCaptureAdmissionReceipt capture;
    Nexus_V1_DgnOwnerMaterialTraceAdmissionReceipt trace;

    if (!out_receipt || !out_trace_receipt) return 0;
    memset(&current, 0, sizeof(current));
    current.capture_required = 1;
    current.operator_only = 1;
    current.no_draw_only = 1;
    current.blocks_real_dgn_mesh_render = 1;
    memset(&trace, 0, sizeof(trace));
    trace.status = NEXUS_V1_OWNER_MATERIAL_TRACE_MISSING;
    trace.level_index = -1;
    trace.descriptor_index = -1;
    trace.no_draw_only = 1;
    trace.blocks_real_dgn_mesh_render = 1;
    if (!route || !route->valid || !route->capture_required ||
        route->capture_admitted || !route->operator_only ||
        !route->no_draw_only || route->owner_mapping_proven ||
        route->mesh_semantics_permitted || route->texture_semantics_permitted ||
        route->decoder_permitted || route->fallback_visuals_permitted ||
        !route->blocks_real_dgn_mesh_render || !manifest_text ||
        manifest_size == 0U || !raw_trace || raw_trace_size == 0U ||
        !original_saturn_capture_verified ||
        !nexus_v1_launcher_admit_m12_m11_owner_material_capture_required(
            engine, route->topology_candidate_index,
            route->structure3_entry_index, route->structure3_face_ordinal,
            route->bios_sha256, route->bios_region, route->disc_sha256,
            &route->target, &current) ||
        !nexus_v1_owner_material_capture_admit(&current.target, capture_bytes,
                                               capture_byte_count, &capture) ||
        raw_trace_size != capture.raw_trace_byte_count ||
        nexus_v1_launcher_capture_fnv1a64(raw_trace, raw_trace_size) !=
            capture.raw_trace_fnv1a64 ||
        nexus_v1_engine_admit_structure1a_structure3_material_capture_trace(
            engine, current.topology_candidate_index,
            current.structure3_entry_index, current.structure3_face_ordinal,
            manifest_text, manifest_size, raw_trace, raw_trace_size,
            original_saturn_capture_verified, &trace) != 1) {
        *out_trace_receipt = trace;
        *out_receipt = current;
        return 0;
    }
    capture.original_saturn_capture_verified = 1;
    current.capture = capture;
    current.capture_required = 0;
    current.capture_admitted = 1;
    current.resume_ready = 1;
    *out_trace_receipt = trace;
    *out_receipt = current;
    return 1;
}

int nexus_v1_launcher_export_m12_owner_material_capture_campaign_required(
    uint32_t expected_witness_count, const char *bios_sha256,
    Nexus_V1_LauncherSaturnBiosRegion bios_region, const char *disc_sha256,
    Nexus_V1_LauncherM12OwnerMaterialCaptureCampaignRouteReceipt *out_receipt)
{
    Nexus_V1_LauncherM12OwnerMaterialCaptureCampaignRouteReceipt receipt;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.capture_required = 1;
    receipt.operator_only = 1;
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    if (expected_witness_count <
            NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_MIN_WITNESSES ||
        expected_witness_count >
            NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_MAX_WITNESSES ||
        !nexus_v1_launcher_sha256_text_valid(bios_sha256) ||
        !nexus_v1_launcher_sha256_text_valid(disc_sha256) ||
        bios_region == NEXUS_V1_LAUNCHER_SATURN_BIOS_REGION_INVALID ||
        bios_region > NEXUS_V1_LAUNCHER_SATURN_BIOS_REGION_EU) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.bios_region = bios_region;
    memcpy(receipt.bios_sha256, bios_sha256, 64U);
    receipt.bios_sha256[64] = '\0';
    memcpy(receipt.disc_sha256, disc_sha256, 64U);
    receipt.disc_sha256[64] = '\0';
    receipt.expected_witness_count = expected_witness_count;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_launcher_import_m12_owner_material_capture_campaign(
    const Nexus_V1_LauncherM12OwnerMaterialCaptureCampaignRouteReceipt *route,
    const Nexus_V1_OwnerMaterialCaptureCampaignInput *campaign_input,
    Nexus_V1_LauncherM12OwnerMaterialCaptureCampaignRouteReceipt *out_receipt)
{
    Nexus_V1_LauncherM12OwnerMaterialCaptureCampaignRouteReceipt current;
    Nexus_V1_OwnerMaterialCaptureCampaignReceipt campaign;

    if (!out_receipt) return 0;
    memset(&current, 0, sizeof(current));
    current.capture_required = 1;
    current.operator_only = 1;
    current.no_draw_only = 1;
    current.blocks_real_dgn_mesh_render = 1;
    memset(&campaign, 0, sizeof(campaign));
    campaign.no_draw_only = 1;
    campaign.blocks_real_dgn_mesh_render = 1;
    if (!route || !route->valid || !route->capture_required ||
        route->captures_imported || !route->operator_only ||
        !route->no_draw_only || route->owner_mapping_proven ||
        route->mesh_semantics_permitted || route->texture_semantics_permitted ||
        route->decoder_permitted || route->fallback_visuals_permitted ||
        !route->blocks_real_dgn_mesh_render || !campaign_input ||
        campaign_input->witness_count != route->expected_witness_count ||
        !nexus_v1_launcher_export_m12_owner_material_capture_campaign_required(
            route->expected_witness_count, route->bios_sha256, route->bios_region,
            route->disc_sha256, &current) ||
        !nexus_v1_owner_material_capture_campaign_admit(campaign_input,
                                                         &campaign)) {
        *out_receipt = current;
        return 0;
    }
    current.campaign = campaign;
    current.capture_required = 0;
    current.captures_imported = 1;
    current.resume_ready = 1;
    *out_receipt = current;
    return 1;
}

int nexus_v1_launcher_import_m12_owner_material_capture_campaign_artifact(
    const Nexus_V1_LauncherM12OwnerMaterialCaptureCampaignRouteReceipt *route,
    const uint8_t *artifact_bytes, size_t artifact_byte_count,
    const Nexus_V1_OwnerMaterialCaptureCampaignInput *campaign_input,
    Nexus_V1_OwnerMaterialCaptureCampaignArtifactReceipt *out_artifact_receipt,
    Nexus_V1_LauncherM12OwnerMaterialCaptureCampaignRouteReceipt *out_receipt)
{
    Nexus_V1_LauncherM12OwnerMaterialCaptureCampaignRouteReceipt imported;
    Nexus_V1_LauncherM12OwnerMaterialCaptureCampaignRouteReceipt required;
    Nexus_V1_OwnerMaterialCaptureCampaignArtifactReceipt artifact;

    if (!out_artifact_receipt || !out_receipt) return 0;
    memset(&artifact, 0, sizeof(artifact));
    artifact.no_draw_only = 1;
    artifact.blocks_real_dgn_mesh_render = 1;
    memset(&required, 0, sizeof(required));
    required.capture_required = 1;
    required.operator_only = 1;
    required.no_draw_only = 1;
    required.blocks_real_dgn_mesh_render = 1;
    if (!nexus_v1_launcher_import_m12_owner_material_capture_campaign(
            route, campaign_input, &imported) ||
        !nexus_v1_owner_material_capture_campaign_artifact_admit(
            &imported.campaign, artifact_bytes, artifact_byte_count, &artifact)) {
        if (route) (void)nexus_v1_launcher_export_m12_owner_material_capture_campaign_required(
            route->expected_witness_count, route->bios_sha256, route->bios_region,
            route->disc_sha256, &required);
        *out_artifact_receipt = artifact;
        *out_receipt = required;
        return 0;
    }
    *out_artifact_receipt = artifact;
    *out_receipt = imported;
    return 1;
}

int nexus_v1_launcher_admit_m12_owner_material_capture_witness_required(
    const Nexus_V1_LauncherM12OwnerMaterialCaptureCampaignRouteReceipt *campaign_route,
    uint32_t witness_index,
    Nexus_V1_LauncherM12OwnerMaterialCaptureWitnessRouteReceipt *out_receipt)
{
    Nexus_V1_LauncherM12OwnerMaterialCaptureWitnessRouteReceipt receipt;
    Nexus_V1_LauncherM12OwnerMaterialCaptureCampaignRouteReceipt current;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.capture_required = 1;
    receipt.operator_only = 1;
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    memset(&current, 0, sizeof(current));
    if (!campaign_route || !campaign_route->valid ||
        !campaign_route->capture_required || campaign_route->captures_imported ||
        !campaign_route->operator_only || !campaign_route->no_draw_only ||
        campaign_route->decoder_permitted || campaign_route->fallback_visuals_permitted ||
        !campaign_route->blocks_real_dgn_mesh_render ||
        witness_index >= campaign_route->expected_witness_count ||
        !nexus_v1_launcher_export_m12_owner_material_capture_campaign_required(
            campaign_route->expected_witness_count, campaign_route->bios_sha256,
            campaign_route->bios_region, campaign_route->disc_sha256, &current)) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.witness_index = witness_index;
    receipt.campaign_route = current;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_launcher_import_m12_owner_material_capture_witness(
    const Nexus_V1_LauncherM12OwnerMaterialCaptureWitnessRouteReceipt *route,
    const uint8_t *campaign_artifact_bytes, size_t campaign_artifact_byte_count,
    const Nexus_V1_OwnerMaterialCaptureCampaignInput *campaign_input,
    const Nexus_V1_OwnerMaterialCaptureArtifactPreflightInput *witness_input,
    Nexus_V1_LauncherM12OwnerMaterialCaptureWitnessRouteReceipt *out_receipt)
{
    Nexus_V1_LauncherM12OwnerMaterialCaptureWitnessRouteReceipt required;
    Nexus_V1_LauncherM12OwnerMaterialCaptureCampaignRouteReceipt campaign;
    Nexus_V1_OwnerMaterialCaptureCampaignArtifactReceipt artifact;
    Nexus_V1_OwnerMaterialCaptureArtifactPreflightInput preflight;
    Nexus_V1_OwnerMaterialCaptureArtifactPreflightReceipt witness;

    if (!out_receipt) return 0;
    memset(&required, 0, sizeof(required));
    required.capture_required = 1;
    required.operator_only = 1;
    required.no_draw_only = 1;
    required.blocks_real_dgn_mesh_render = 1;
    memset(&campaign, 0, sizeof(campaign));
    memset(&artifact, 0, sizeof(artifact));
    memset(&witness, 0, sizeof(witness));
    witness.no_draw_only = 1;
    witness.blocks_real_dgn_mesh_render = 1;
    if (!route || !route->valid || !route->capture_required ||
        route->capture_admitted || !route->operator_only || !route->no_draw_only ||
        route->decoder_permitted || route->fallback_visuals_permitted ||
        !route->blocks_real_dgn_mesh_render || !witness_input ||
        witness_input->witness_index != route->witness_index ||
        !nexus_v1_launcher_admit_m12_owner_material_capture_witness_required(
            &route->campaign_route, route->witness_index, &required) ||
        !nexus_v1_launcher_import_m12_owner_material_capture_campaign_artifact(
            &route->campaign_route, campaign_artifact_bytes,
            campaign_artifact_byte_count, campaign_input, &artifact, &campaign)) {
        *out_receipt = required;
        return 0;
    }
    preflight = *witness_input;
    preflight.campaign = &campaign.campaign;
    if (!nexus_v1_owner_material_capture_artifact_preflight(&preflight, &witness)) {
        *out_receipt = required;
        return 0;
    }
    required.capture_required = 0;
    required.capture_admitted = 1;
    required.resume_ready = 1;
    required.campaign_route = campaign;
    required.campaign_artifact = artifact;
    required.witness = witness;
    *out_receipt = required;
    return 1;
}

int nexus_v1_launcher_admit_m12_owner_material_capture_multi_witness_required(
    const Nexus_V1_LauncherM12OwnerMaterialCaptureCampaignRouteReceipt *campaign_route,
    const uint32_t *witness_indices, uint32_t witness_count,
    Nexus_V1_LauncherM12OwnerMaterialCaptureMultiWitnessRouteReceipt *out_receipt)
{
    Nexus_V1_LauncherM12OwnerMaterialCaptureMultiWitnessRouteReceipt receipt;
    Nexus_V1_LauncherM12OwnerMaterialCaptureCampaignRouteReceipt current;
    uint32_t index;
    uint32_t prior;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.capture_required = 1;
    receipt.operator_only = 1;
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    memset(&current, 0, sizeof(current));
    if (!campaign_route || !witness_indices ||
        witness_count < NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_MIN_WITNESSES ||
        witness_count > campaign_route->expected_witness_count ||
        !nexus_v1_launcher_export_m12_owner_material_capture_campaign_required(
            campaign_route->expected_witness_count, campaign_route->bios_sha256,
            campaign_route->bios_region, campaign_route->disc_sha256, &current)) {
        *out_receipt = receipt;
        return 0;
    }
    for (index = 0U; index < witness_count; ++index) {
        if (witness_indices[index] >= current.expected_witness_count) {
            *out_receipt = receipt;
            return 0;
        }
        for (prior = 0U; prior < index; ++prior) {
            if (witness_indices[prior] == witness_indices[index]) {
                *out_receipt = receipt;
                return 0;
            }
        }
        receipt.witness_indices[index] = witness_indices[index];
    }
    receipt.valid = 1;
    receipt.witness_count = witness_count;
    receipt.campaign_route = current;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_launcher_admit_m11_multi_witness_dungeon_capture_start(
    const Nexus_V1_Engine *engine,
    const Nexus_V1_LauncherM12OwnerMaterialCaptureMultiWitnessRouteReceipt *route,
    const Nexus_V1_OwnerMaterialCaptureCampaignInput *campaign_input,
    Nexus_V1_LauncherM11MultiWitnessDungeonCaptureStartReceipt *out_receipt)
{
    Nexus_V1_LauncherM11MultiWitnessDungeonCaptureStartReceipt receipt;
    Nexus_V1_OwnerMaterialCaptureCampaignReceipt campaign;
    const Nexus_V1_OwnerMaterialCaptureCampaignWitness *witness;
    const Nexus_V1_DgnStructure1AStructure3MaterialCaptureTarget *target;
    const Nexus_V1_DgnStructure2DescriptorCaptureTarget *descriptor;
    uint32_t selected;
    uint32_t index;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    memset(&campaign, 0, sizeof(campaign));
    if (!engine || !route || !route->valid || !route->capture_required ||
        route->captures_admitted || !route->operator_only ||
        !route->no_draw_only || route->decoder_permitted ||
        route->fallback_visuals_permitted || !route->blocks_real_dgn_mesh_render ||
        !campaign_input || !nexus_v1_owner_material_capture_campaign_admit(
            campaign_input, &campaign) ||
        campaign.witness_count != route->campaign_route.expected_witness_count) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.level_index = (uint32_t)engine->game.current_level;
    for (selected = 0U; selected < route->witness_count; ++selected) {
        index = route->witness_indices[selected];
        if (index >= campaign.witness_count) {
            *out_receipt = receipt;
            return 0;
        }
        if (campaign.witnesses[index].level_index == receipt.level_index) {
            witness = &campaign_input->witnesses[index];
            target = witness->target;
            if (!target || !target->valid || target->level_index !=
                    engine->game.current_level || !target->owner_face_source_bound ||
                !target->static_material_source_bound ||
                !target->capture_producer_required ||
                !target->original_saturn_capture_required || !target->no_draw_only ||
                target->fallback_visuals_permitted ||
                !target->blocks_real_dgn_mesh_render) {
                *out_receipt = receipt;
                return 0;
            }
            descriptor = &target->material_target.descriptor_target;
            if (target->material_target.source_bytes_fnv1a64 !=
                    campaign.witnesses[index].source_fnv1a64 ||
                descriptor->descriptor_bytes_fnv1a64 !=
                    campaign.witnesses[index].descriptor_fnv1a64 ||
                (uint64_t)target->owner_face_target.face_target.candidate
                    .face_row_fnv1a32 != campaign.witnesses[index].face_row_fnv1a64) {
                *out_receipt = receipt;
                return 0;
            }
            receipt.valid = 1;
            receipt.selected_witness = 1;
            receipt.capture_required = 1;
            receipt.operator_only = 1;
            receipt.witness_index = index;
            receipt.source_fnv1a64 = campaign.witnesses[index].source_fnv1a64;
            receipt.descriptor_fnv1a64 = campaign.witnesses[index].descriptor_fnv1a64;
            receipt.face_row_fnv1a64 = campaign.witnesses[index].face_row_fnv1a64;
            receipt.capture_target = *target;
            receipt.capture_target_bound = 1;
            *out_receipt = receipt;
            return 1;
        }
    }
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_launcher_admit_m12_m11_structure3_topology_capture_required(
    Nexus_V1_Engine *engine, const Nexus_V1_LevCorpusDiscoveryReceipt *corpus,
    const Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt *handoff,
    const char *bios_sha256, Nexus_V1_LauncherSaturnBiosRegion bios_region,
    const char *disc_sha256,
    const Nexus_V1_LauncherM11Structure3TopologyCaptureReplayTarget *target,
    Nexus_V1_LauncherM12M11Structure3TopologyCaptureRouteReceipt *out_receipt)
{
    Nexus_V1_LauncherM12M11Structure3TopologyCaptureRouteReceipt receipt;
    Nexus_V1_LauncherM11Structure3TopologyCaptureReplayTarget current;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.capture_required = 1;
    receipt.operator_only = 1;
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    memset(&current, 0, sizeof(current));
    if (!nexus_v1_launcher_sha256_text_valid(bios_sha256) ||
        !nexus_v1_launcher_sha256_text_valid(disc_sha256) ||
        bios_region == NEXUS_V1_LAUNCHER_SATURN_BIOS_REGION_INVALID ||
        bios_region > NEXUS_V1_LAUNCHER_SATURN_BIOS_REGION_EU || !target ||
        !nexus_v1_launcher_build_m11_structure3_topology_capture_replay_target(
            engine, corpus, handoff, &current) ||
        !nexus_v1_launcher_m11_structure3_topology_capture_target_matches(
            target, &current)) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.bios_region = bios_region;
    memcpy(receipt.bios_sha256, bios_sha256, 64U);
    receipt.bios_sha256[64] = '\0';
    memcpy(receipt.disc_sha256, disc_sha256, 64U);
    receipt.disc_sha256[64] = '\0';
    receipt.target = current;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_launcher_resume_m12_m11_structure3_topology_capture(
    Nexus_V1_Engine *engine, const Nexus_V1_LevCorpusDiscoveryReceipt *corpus,
    const Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt *handoff,
    const Nexus_V1_LauncherM12M11Structure3TopologyCaptureRouteReceipt *route,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    Nexus_V1_LauncherM12M11Structure3TopologyCaptureRouteReceipt *out_receipt)
{
    Nexus_V1_LauncherM12M11Structure3TopologyCaptureRouteReceipt receipt;
    Nexus_V1_LauncherM12M11Structure3TopologyCaptureRouteReceipt current;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.capture_required = 1;
    receipt.operator_only = 1;
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    memset(&current, 0, sizeof(current));
    if (!route || !route->valid || !route->capture_required ||
        route->capture_imported || !route->operator_only ||
        route->decoder_permitted || route->fallback_visuals_permitted ||
        !route->no_draw_only || !route->blocks_real_dgn_mesh_render ||
        !nexus_v1_launcher_admit_m12_m11_structure3_topology_capture_required(
            engine, corpus, handoff, route->bios_sha256, route->bios_region,
            route->disc_sha256, &route->target, &current)) {
        *out_receipt = receipt;
        return 0;
    }
    receipt = current;
    receipt.capture = (Nexus_V1_LauncherM11Structure3TopologyCaptureImportReceipt){0};
    if (!nexus_v1_launcher_import_m11_structure3_topology_capture(
            engine, corpus, handoff, &receipt.target, capture_bytes,
            capture_byte_count, &receipt.capture)) {
        receipt.capture_required = 1;
        receipt.valid = 0;
        *out_receipt = receipt;
        return 0;
    }
    receipt.capture_required = 0;
    receipt.capture_imported = 1;
    receipt.resume_ready = 1;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_launcher_verify_m11_structure3_topology_local_artifact(
    const Nexus_V1_LauncherM12M11Structure3TopologyCaptureRouteReceipt *route,
    const char *bios_sha256, const char *disc_sha256,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    Nexus_V1_LauncherM11Structure3TopologyLocalArtifactReceipt *out_receipt)
{
    Nexus_V1_LauncherM11Structure3TopologyLocalArtifactReceipt receipt;
    const Nexus_V1_LauncherM11Structure3TopologyCaptureReplayTarget *target;
    uint32_t payload_offset, payload_length;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    if (!route || !route->valid || !route->capture_required ||
        route->capture_imported || !route->operator_only ||
        !route->no_draw_only || route->decoder_permitted ||
        route->fallback_visuals_permitted ||
        !route->blocks_real_dgn_mesh_render ||
        !nexus_v1_launcher_sha256_text_valid(bios_sha256) ||
        !nexus_v1_launcher_sha256_text_valid(disc_sha256) ||
        strcmp(route->bios_sha256, bios_sha256) != 0 ||
        strcmp(route->disc_sha256, disc_sha256) != 0 || !capture_bytes ||
        capture_byte_count < NEXUS_V1_M11_STRUCTURE3_TOPOLOGY_CAPTURE_HEADER_BYTES ||
        memcmp(capture_bytes, NEXUS_V1_M11_STRUCTURE3_TOPOLOGY_CAPTURE_MAGIC, 8U) != 0 ||
        nexus_v1_launcher_capture_be32(capture_bytes + 8U) !=
            NEXUS_V1_M11_STRUCTURE3_TOPOLOGY_CAPTURE_VERSION ||
        nexus_v1_launcher_capture_be32(capture_bytes + 12U) !=
            NEXUS_V1_M11_STRUCTURE3_TOPOLOGY_CAPTURE_HEADER_BYTES) {
        *out_receipt = receipt;
        return 0;
    }
    target = &route->target;
    payload_offset = nexus_v1_launcher_capture_be32(capture_bytes + 108U);
    payload_length = nexus_v1_launcher_capture_be32(capture_bytes + 112U);
    receipt.bios_bound = 1;
    receipt.disc_bound = 1;
    receipt.lev_bound =
        nexus_v1_launcher_capture_be64(capture_bytes + 16U) == target->route_epoch &&
        nexus_v1_launcher_capture_be64(capture_bytes + 24U) == target->package_fnv1a64 &&
        nexus_v1_launcher_capture_be64(capture_bytes + 32U) == target->card_fnv1a64 &&
        nexus_v1_launcher_capture_be64(capture_bytes + 40U) == target->dgn_fnv1a64 &&
        nexus_v1_launcher_capture_be64(capture_bytes + 48U) == target->dgn_byte_count;
    receipt.face_bound =
        nexus_v1_launcher_capture_be32(capture_bytes + 56U) ==
            (uint32_t)target->topology.structure1f_entry_index &&
        nexus_v1_launcher_capture_be32(capture_bytes + 60U) ==
            target->topology.structure3_entry_index &&
        nexus_v1_launcher_capture_be32(capture_bytes + 64U) ==
            target->topology.face_ordinal;
    receipt.vertex_bound =
        nexus_v1_launcher_capture_be32(capture_bytes + 68U) == target->topology.vertex_table_offset &&
        nexus_v1_launcher_capture_be32(capture_bytes + 72U) == target->topology.vertex_table_length &&
        nexus_v1_launcher_capture_be64(capture_bytes + 76U) == target->topology.vertex_table_fnv1a64 &&
        nexus_v1_launcher_capture_be64(capture_bytes + 84U) ==
            target->topology.referenced_vertex_rows_fnv1a64;
    receipt.normal_bound =
        nexus_v1_launcher_capture_be32(capture_bytes + 92U) == target->topology.normal_offset &&
        nexus_v1_launcher_capture_be32(capture_bytes + 96U) == target->topology.normal_length &&
        nexus_v1_launcher_capture_be64(capture_bytes + 100U) == target->topology.normal_fnv1a64;
    receipt.payload_bounds_bound = payload_offset ==
            NEXUS_V1_M11_STRUCTURE3_TOPOLOGY_CAPTURE_HEADER_BYTES &&
        payload_length > 0U && payload_offset <= capture_byte_count &&
        payload_length <= capture_byte_count - payload_offset;
    receipt.payload_hash_bound = receipt.payload_bounds_bound &&
        nexus_v1_launcher_capture_be64(capture_bytes + 116U) ==
            nexus_v1_launcher_capture_fnv1a64(capture_bytes + payload_offset,
                                              payload_length);
    if (!receipt.lev_bound || !receipt.face_bound || !receipt.vertex_bound ||
        !receipt.normal_bound || !receipt.payload_bounds_bound ||
        !receipt.payload_hash_bound) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.capture_fnv1a64 = nexus_v1_launcher_capture_fnv1a64(capture_bytes,
                                                                  capture_byte_count);
    receipt.capture_byte_count = capture_byte_count;
    receipt.topology_opaque = 1;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_launcher_resume_m12_m11_structure3_topology_local_artifact(
    Nexus_V1_Engine *engine, const Nexus_V1_LevCorpusDiscoveryReceipt *corpus,
    const Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt *handoff,
    const Nexus_V1_LauncherM12M11Structure3TopologyCaptureRouteReceipt *route,
    const char *bios_sha256, const char *disc_sha256,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    Nexus_V1_LauncherM12M11Structure3TopologyCaptureRouteReceipt *out_receipt)
{
    Nexus_V1_LauncherM11Structure3TopologyLocalArtifactReceipt preflight;

    if (!out_receipt) return 0;
    memset(&preflight, 0, sizeof(preflight));
    if (!nexus_v1_launcher_verify_m11_structure3_topology_local_artifact(
            route, bios_sha256, disc_sha256, capture_bytes, capture_byte_count,
            &preflight)) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        out_receipt->capture_required = 1;
        out_receipt->operator_only = 1;
        out_receipt->no_draw_only = 1;
        out_receipt->blocks_real_dgn_mesh_render = 1;
        return 0;
    }
    return nexus_v1_launcher_resume_m12_m11_structure3_topology_capture(
        engine, corpus, handoff, route, capture_bytes, capture_byte_count,
        out_receipt);
}

int nexus_v1_launcher_admit_m11_sddrvs_dungeon_no_draw(
    Nexus_V1_Engine *engine,
    const Nexus_V1_SlevSalAssetDiscoveryReceipt *assets,
    const Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt *dungeon,
    uint64_t route_epoch, uint64_t package_fnv1a64, uint64_t card_fnv1a64,
    int direct_card_selected, uint32_t level_index,
    Nexus_V1_LauncherM11SddrvsDungeonNoDrawReceipt *out_receipt)
{
    Nexus_V1_LauncherM11SddrvsDungeonNoDrawReceipt receipt;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    receipt.blocks_real_script_dispatch = 1;
    receipt.blocks_real_sfx_playback = 1;
    if (!engine || !assets || !assets->valid || !dungeon || !dungeon->valid ||
        !route_epoch || !package_fnv1a64 || !card_fnv1a64 ||
        level_index >= NEXUS_V1_SLEV_SAL_ASSET_LEVEL_COUNT ||
        dungeon->route_epoch != route_epoch ||
        dungeon->package_fnv1a64 != package_fnv1a64 ||
        dungeon->card_fnv1a64 != card_fnv1a64 ||
        dungeon->level_index != level_index || !dungeon->no_draw_only ||
        !dungeon->draw_disabled ||
        !nexus_v1_slev_sal_direct_identity_still_matches(
            &assets->sound_driver) ||
        !nexus_v1_engine_m11_direct_lev_dungeon_no_draw_ready(
            engine, route_epoch, (int)level_index,
            dungeon->dungeon.dgn_md5, dungeon->dungeon.dgn_byte_count,
            dungeon->dungeon.dgn_fnv1a64, NULL) ||
        !nexus_v1_launcher_admit_m11_slev_sal_no_draw(
            engine, assets, route_epoch, package_fnv1a64, card_fnv1a64,
            direct_card_selected, level_index, &receipt.level_aux)) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.level_index = level_index;
    receipt.route_epoch = route_epoch;
    receipt.package_fnv1a64 = package_fnv1a64;
    receipt.card_fnv1a64 = card_fnv1a64;
    receipt.sound_driver = assets->sound_driver;
    receipt.dungeon = *dungeon;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_launcher_admit_m11_slev_sal_dungeon_no_draw(
    Nexus_V1_Engine *engine,
    const Nexus_V1_SlevSalAssetDiscoveryReceipt *assets,
    const Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt *dungeon,
    uint64_t route_epoch, uint64_t package_fnv1a64, uint64_t card_fnv1a64,
    int direct_card_selected, uint32_t level_index,
    Nexus_V1_LauncherM11SlevSalDungeonNoDrawReceipt *out_receipt)
{
    Nexus_V1_LauncherM11SlevSalDungeonNoDrawReceipt receipt;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    receipt.blocks_real_script_dispatch = 1;
    receipt.blocks_real_sfx_playback = 1;
    if (!engine || !assets || !assets->valid || !dungeon || !dungeon->valid ||
        !route_epoch || !package_fnv1a64 || !card_fnv1a64 ||
        level_index >= NEXUS_V1_SLEV_SAL_ASSET_LEVEL_COUNT ||
        dungeon->route_epoch != route_epoch ||
        dungeon->package_fnv1a64 != package_fnv1a64 ||
        dungeon->card_fnv1a64 != card_fnv1a64 ||
        dungeon->level_index != level_index || !dungeon->no_draw_only ||
        !dungeon->draw_disabled ||
        !nexus_v1_slev_sal_level_identities_still_match(assets, level_index) ||
        !nexus_v1_sal_container_provenance_from_direct_identity(
            &assets->levels[level_index].sal, &receipt.sal_container) ||
        !nexus_v1_sndlev_map_provenance_from_direct_identity(
            &assets->levels[level_index].map, &receipt.map_table) ||
        !receipt.sal_container.valid || receipt.sal_container.codec_proven ||
        receipt.sal_container.playback_permitted ||
        receipt.sal_container.source_fnv1a64 !=
            assets->levels[level_index].sal.fnv1a64 ||
        receipt.sal_container.source_byte_count !=
            assets->levels[level_index].sal.byte_count ||
        receipt.sal_container.descriptor_offset !=
            NEXUS_V1_SAL_CONTAINER_HEADER_BYTES ||
        !receipt.sal_container.descriptor_length ||
        !receipt.map_table.valid || receipt.map_table.playback_permitted ||
        receipt.map_table.source_fnv1a64 != assets->levels[level_index].map.fnv1a64 ||
        receipt.map_table.source_byte_count != assets->levels[level_index].map.byte_count ||
        receipt.map_table.header_length != NEXUS_V1_SNDLEV_MAP_HEADER_BYTES ||
        !receipt.map_table.record_count || !receipt.map_table.table_length ||
        !nexus_v1_engine_m11_direct_lev_dungeon_no_draw_ready(
            engine, route_epoch, (int)level_index,
            dungeon->dungeon.dgn_md5, dungeon->dungeon.dgn_byte_count,
            dungeon->dungeon.dgn_fnv1a64, NULL) ||
        !nexus_v1_launcher_admit_m11_slev_sal_no_draw(
            engine, assets, route_epoch, package_fnv1a64, card_fnv1a64,
            direct_card_selected, level_index, &receipt.level_aux)) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.level_index = level_index;
    receipt.route_epoch = route_epoch;
    receipt.package_fnv1a64 = package_fnv1a64;
    receipt.card_fnv1a64 = card_fnv1a64;
    receipt.assets = assets->levels[level_index];
    receipt.dungeon = *dungeon;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_launcher_admit_m11_sndlev_map_row_no_draw(
    Nexus_V1_Engine *engine,
    const Nexus_V1_SlevSalAssetDiscoveryReceipt *assets,
    const Nexus_V1_LauncherM11SlevSalDungeonNoDrawReceipt *dungeon,
    uint64_t route_epoch, uint64_t package_fnv1a64, uint64_t card_fnv1a64,
    uint32_t level_index, uint32_t row_index,
    Nexus_V1_LauncherM11SndlevMapRowNoDrawReceipt *out_receipt)
{
    Nexus_V1_LauncherM11SndlevMapRowNoDrawReceipt receipt;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    receipt.blocks_real_script_dispatch = 1;
    receipt.blocks_real_sfx_playback = 1;
    if (!engine || !assets || !assets->valid || !dungeon || !dungeon->valid ||
        !route_epoch || !package_fnv1a64 || !card_fnv1a64 ||
        level_index >= NEXUS_V1_SLEV_SAL_ASSET_LEVEL_COUNT ||
        dungeon->route_epoch != route_epoch ||
        dungeon->package_fnv1a64 != package_fnv1a64 ||
        dungeon->card_fnv1a64 != card_fnv1a64 ||
        dungeon->level_index != level_index || !dungeon->no_draw_only ||
        !dungeon->blocks_real_script_dispatch ||
        !dungeon->blocks_real_sfx_playback || !dungeon->map_table.valid ||
        dungeon->map_table.playback_permitted ||
        !nexus_v1_sndlev_map_row_provenance_from_direct_identity(
            &assets->levels[level_index].map, row_index, &receipt.row) ||
        !receipt.row.valid || receipt.row.playback_permitted ||
        receipt.row.table_fnv1a64 != dungeon->map_table.table_fnv1a64 ||
        receipt.row.row_length != NEXUS_V1_SNDLEV_MAP_RECORD_BYTES ||
        receipt.row.row_offset < dungeon->map_table.table_offset ||
        (uint64_t)receipt.row.row_offset + receipt.row.row_length >
            dungeon->map_table.table_offset + dungeon->map_table.table_length ||
        !nexus_v1_slev_sal_level_identities_still_match(assets, level_index) ||
        !nexus_v1_engine_m11_direct_lev_dungeon_no_draw_ready(
            engine, route_epoch, (int)level_index,
            dungeon->dungeon.dungeon.dgn_md5,
            dungeon->dungeon.dungeon.dgn_byte_count,
            dungeon->dungeon.dungeon.dgn_fnv1a64, NULL)) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.level_index = level_index;
    receipt.row_index = row_index;
    receipt.route_epoch = route_epoch;
    receipt.package_fnv1a64 = package_fnv1a64;
    receipt.card_fnv1a64 = card_fnv1a64;
    receipt.table_fnv1a64 = receipt.row.table_fnv1a64;
    receipt.dungeon = *dungeon;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_launcher_admit_multi_level_m11_dungeon_handoff(
    Nexus_V1_Engine *engine, const Nexus_V1_LevCorpusDiscoveryReceipt *corpus,
    const Nexus_V1_MultiLevelCaptureCampaignLaunchPlan *plan,
    uint64_t route_epoch, uint64_t package_fnv1a64, uint64_t card_fnv1a64,
    int direct_card_selected, uint32_t level_index,
    const Nexus_V1_DgnStructure1F2FaceAdjacencyTransformReceipt *geometry,
    Nexus_V1_LauncherMultiLevelM11DungeonHandoffReceipt *out_receipt)
{
    Nexus_V1_LauncherMultiLevelM11DungeonHandoffReceipt receipt;
    const Nexus_V1_MultiLevelCaptureJob *job;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    receipt.draw_disabled = 1;
    if (!plan || !plan->valid || !plan->operator_only ||
        plan->evidence_generated || plan->graphics_permitted || level_index >=
            NEXUS_V1_MULTI_LEVEL_CAPTURE_JOB_COUNT) {
        *out_receipt = receipt;
        return 0;
    }
    job = &plan->jobs[level_index];
    if (!job->valid || job->level_index != level_index || !job->dgn_fnv1a64 ||
        !corpus || !corpus->valid || !corpus->levels[level_index].valid ||
        corpus->levels[level_index].fnv1a64 != job->dgn_fnv1a64 ||
        !nexus_v1_launcher_admit_direct_lev_m11_dungeon_handoff(
            engine, corpus, route_epoch, package_fnv1a64, card_fnv1a64,
            direct_card_selected, level_index, geometry, &receipt.direct_lev)) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.level_index = level_index;
    receipt.route_epoch = route_epoch;
    receipt.dgn_fnv1a64 = job->dgn_fnv1a64;
    *out_receipt = receipt;
    return 1;
}

static void nexus_v1_launcher_fill_startup_assets_receipt(
    const Nexus_V1_Engine *engine,
    int title_loaded,
    Nexus_V1_LauncherStartupAssetsReceipt *receipt)
{
    Nexus_V1_BpkRuntimeUploadReceipt bpk;
    Nexus_V1_BpkRuntimeUploadRow rows[NEXUS_V1_BPK_UPLOAD_PLAN_MAX_ROWS];
    Nexus_SfxRuntimeReceipt sfx;
    int title_capture_source_ready;

    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->menu_bpk_upload_route = NEXUS_V1_BPK_UPLOAD_ROUTE_INVALID;
    receipt->startup_sfx_status = NEXUS_SFX_RUNTIME_MISSING;
    receipt->startup_sfx_level_index = -1;
    receipt->startup_cd_track = -1;
    if (!engine) {
        return;
    }

    receipt->title_surface_loaded =
        nexus_v1_surface_loaded(engine, NEXUS_SURFACE_TITLE);
    receipt->warning_surface_loaded =
        nexus_v1_surface_loaded(engine, NEXUS_SURFACE_WARNING);
    receipt->gameover_surface_loaded =
        nexus_v1_surface_loaded(engine, NEXUS_SURFACE_GAMEOVER);
    receipt->status_bg_surface_loaded =
        nexus_v1_surface_loaded(engine, NEXUS_SURFACE_STABG);
    receipt->title_screen_loaded = title_loaded ? 1 : 0;
    receipt->startup_surfaces_loaded =
        nexus_v1_startup_surfaces_loaded_count(engine);
    receipt->startup_surfaces_expected =
        nexus_v1_startup_surfaces_expected_count(engine);
    receipt->startup_surfaces_fallback =
        nexus_v1_startup_surfaces_fallback_count(engine);
    receipt->faces_loaded = nexus_v1_startup_faces_loaded_count(engine);
    receipt->faces_expected = nexus_v1_startup_faces_expected_count(engine);
    receipt->faces_fallback = nexus_v1_startup_faces_fallback_count(engine);
    /* The verified TITLE.CG loader produces an indexed character-generator
     * atlas. It is useful source evidence, but it is not the Saturn title
     * framebuffer. A title capture handoff must replace that source before
     * the startup asset receipt can advertise a drawable title route. */
    title_capture_source_ready =
        !engine->ui.surfaces[NEXUS_SURFACE_TITLE].source ||
        strcmp(engine->ui.surfaces[NEXUS_SURFACE_TITLE].source,
               "TITLE.CG/4bpp-atlas") != 0;

    memset(&bpk, 0, sizeof(bpk));
    memset(rows, 0, sizeof(rows));
    if (nexus_v1_menu_bpk_upload_plan_receipt(engine, &bpk) == 0) {
        receipt->menu_bpk_upload_receipt_valid = 1;
        receipt->menu_bpk_provenance_archive_entries = bpk.archive_entries;
        receipt->menu_bpk_provenance_table_entries = bpk.planned_rows;
        receipt->menu_bpk_provenance_expected_bytes = bpk.expected_upload_bytes;
        receipt->menu_bpk_provenance_bounded = bpk.archive_entries > 0U &&
            bpk.planned_rows > 0U && !bpk.truncated &&
            !bpk.fallback_visuals_permitted;
        receipt->menu_bpk_upload_route = bpk.route;
        receipt->menu_bpk_archive_entries = (int)bpk.archive_entries;
        receipt->menu_bpk_surface_entries = (int)bpk.surface_entries;
        receipt->menu_bpk_directory_trailer_found =
            bpk.directory_trailer_found;
        receipt->menu_bpk_directory_trailer_at_entry_zero =
            bpk.directory_trailer_at_entry_zero;
        receipt->menu_bpk_directory_trailer_valid =
            bpk.directory_trailer_valid;
        receipt->menu_bpk_planned_rows = (int)bpk.planned_rows;
        receipt->menu_bpk_blocked_prs3_uploads =
            (int)bpk.blocked_prs3_uploads;
        receipt->menu_bpk_blocks_real_menu_surface_render =
            bpk.blocks_real_menu_surface_render;
        receipt->menu_bpk_fallback_visuals_permitted =
            bpk.fallback_visuals_permitted;
        {
            int row_count = nexus_v1_menu_bpk_upload_plan_rows(
                engine, rows, (int)NEXUS_V1_BPK_UPLOAD_PLAN_MAX_ROWS);
            int i;
            for (i = 0; row_count > 0 && i < row_count; ++i) {
                if (rows[i].entry_index == bpk.first_prs3_entry_index) {
                    (void)nexus_v1_launcher_menu_bpk_no_draw_presentation_receipt(
                        &bpk, &rows[i],
                        &receipt->menu_bpk_no_draw_presentation);
                    break;
                }
            }
        }
    }

    memset(&sfx, 0, sizeof(sfx));
    if (nexus_v1_current_level_sfx_runtime_receipt(engine, &sfx) == 0) {
        receipt->startup_sfx_status = sfx.status;
        receipt->startup_sfx_level_index = sfx.level_index;
        receipt->startup_cd_track = sfx.cd_track;
        receipt->startup_sfx_blocks_real_playback =
            sfx.blocks_real_sfx_playback;
        receipt->startup_audio_handoff_ready =
            sfx.level_index == 0 && sfx.cd_track == 2 &&
            sfx.status == NEXUS_SFX_RUNTIME_READY_DECODED &&
            !sfx.blocks_real_sfx_playback;
    }

    receipt->startup_assets_ready =
        receipt->title_screen_loaded &&
        title_capture_source_ready &&
        nexus_v1_startup_surfaces_ready(engine) &&
        nexus_v1_startup_faces_ready(engine);
    receipt->title_route_ready =
        receipt->title_screen_loaded &&
        title_capture_source_ready &&
        nexus_v1_startup_surfaces_ready(engine);
    receipt->real_menu_surface_route_ready =
        receipt->menu_bpk_upload_receipt_valid &&
        receipt->menu_bpk_provenance_bounded &&
        (receipt->menu_bpk_upload_route == NEXUS_V1_BPK_UPLOAD_ROUTE_READY_STORED ||
         receipt->menu_bpk_upload_route == NEXUS_V1_BPK_UPLOAD_ROUTE_READY_DECODED) &&
        !receipt->menu_bpk_blocks_real_menu_surface_render;
    receipt->real_menu_surface_route_blocked =
        receipt->real_menu_surface_route_ready ? 0 : 1;
    if (!title_capture_source_ready) {
        receipt->real_menu_surface_blocker = "title-vdp-capture-required";
        receipt->startup_menu_asset_route = "blocked-title-vdp-capture";
    } else if (!nexus_v1_startup_surfaces_ready(engine)) {
        receipt->real_menu_surface_blocker = "startup-surfaces";
        receipt->startup_menu_asset_route = "blocked-startup-surfaces";
    } else if (!nexus_v1_startup_faces_ready(engine)) {
        receipt->real_menu_surface_blocker = "faces";
        receipt->startup_menu_asset_route = "blocked-faces";
    } else if (!receipt->menu_bpk_upload_receipt_valid) {
        receipt->real_menu_surface_blocker = "menu-bpk";
        receipt->startup_menu_asset_route = "blocked-menu-bpk";
    } else if (receipt->menu_bpk_upload_route ==
               NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_PRS3) {
        receipt->real_menu_surface_blocker = "menu-bpk-prs3-capture-required";
        receipt->startup_menu_asset_route =
            "menu-bpk-prs3-capture-required";
    } else if (receipt->menu_bpk_upload_route ==
               NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_TRUNCATED) {
        receipt->real_menu_surface_blocker = "menu-bpk-truncated";
        receipt->startup_menu_asset_route = "blocked-menu-bpk-truncated";
    } else if (receipt->menu_bpk_upload_route ==
               NEXUS_V1_BPK_UPLOAD_ROUTE_NO_SURFACES) {
        receipt->real_menu_surface_blocker = "menu-bpk-no-surfaces";
        receipt->startup_menu_asset_route = "blocked-menu-bpk-no-surfaces";
    } else if (receipt->menu_bpk_upload_route ==
               NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_CAPACITY) {
        receipt->real_menu_surface_blocker = "menu-bpk-capacity";
        receipt->startup_menu_asset_route = "blocked-menu-bpk-capacity";
    } else if (receipt->real_menu_surface_route_ready) {
        receipt->real_menu_surface_blocker = "none";
        receipt->startup_menu_asset_route = "ready-real-menu-surfaces";
    } else {
        receipt->real_menu_surface_blocker = "menu-bpk-invalid";
        receipt->startup_menu_asset_route = "blocked-menu-bpk-invalid";
    }
    receipt->save_menu_route_ready =
        receipt->startup_audio_handoff_ready &&
        receipt->real_menu_surface_route_ready;
    receipt->champion_menu_route_ready =
        receipt->save_menu_route_ready &&
        nexus_v1_startup_faces_ready(engine);
    receipt->main_menu_route_ready =
        engine->level_loaded &&
        receipt->startup_assets_ready &&
        receipt->startup_audio_handoff_ready &&
        receipt->real_menu_surface_route_ready;
}

static int nexus_v1_launcher_startup_assets_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_LauncherStartupAssetsReceipt *out_assets)
{
    if (!state || !state->engine || !out_assets) {
        return 0;
    }
    nexus_v1_launcher_fill_startup_assets_receipt(
        state->engine,
        1,
        out_assets);
    return 1;
}

static void nexus_v1_launcher_fill_save_asset_blocked_route(
    const Nexus_V1_StartupHostFacts *facts,
    const Nexus_V1_LauncherStartupAssetsReceipt *assets,
    Nexus_V1_StartupSaveRouteReceipt *out_receipt)
{
    Nexus_V1_StartupDrawCommand commands[80];

    nexus_v1_startup_save_route_receipt_clear(out_receipt);
    if (!facts || !assets || !out_receipt) {
        return;
    }
    out_receipt->route = NEXUS_V1_STARTUP_SAVE_ROUTE_ASSET_BLOCKED;
    out_receipt->host_input_result = NEXUS_V1_STARTUP_HOST_INPUT_REDRAW;
    out_receipt->status_scope = "ASSETS";
    out_receipt->status = assets->startup_menu_asset_route
        ? assets->startup_menu_asset_route
        : "blocked-startup-assets";
    if (nexus_v1_startup_menu_state_receipt_from_facts(
            &out_receipt->save_state_receipt,
            facts->save_dir,
            facts->slot_mask,
            facts->save_selected_row)) {
        out_receipt->save_state_receipt_valid = 1;
        out_receipt->row_count = out_receipt->save_state_receipt.row_count;
        out_receipt->selected_row =
            out_receipt->save_state_receipt.selected_row;
        out_receipt->slot_mask =
            (int)out_receipt->save_state_receipt.slot_mask;
        out_receipt->draw_command_count =
            nexus_v1_startup_presentation_build_save_from_facts(
                out_receipt->save_state_receipt.save_dir,
                out_receipt->save_state_receipt.slot_mask,
                out_receipt->save_state_receipt.selected_row,
                commands,
                (int)(sizeof(commands) / sizeof(commands[0])));
        {
            int read_index, write_index;
            for (read_index = 0, write_index = 0;
                 read_index < out_receipt->draw_command_count;
                 ++read_index) {
                if (commands[read_index].kind != NEXUS_V1_STARTUP_DRAW_TEXT)
                    commands[write_index++] = commands[read_index];
            }
            out_receipt->draw_command_count = write_index;
        }
    }
}

static void nexus_v1_launcher_fill_save_asset_blocked_action(
    const Nexus_V1_StartupHostFacts *facts,
    const Nexus_V1_LauncherStartupAssetsReceipt *assets,
    Nexus_V1_StartupSaveExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    if (out_execution) {
        memset(out_execution, 0, sizeof(*out_execution));
        out_execution->kind = NEXUS_V1_STARTUP_SAVE_EXEC_IGNORE;
        out_execution->status_scope = "ASSETS";
        out_execution->status = assets && assets->startup_menu_asset_route
            ? assets->startup_menu_asset_route
            : "blocked-startup-assets";
    }
    if (!out_receipt) {
        return;
    }
    nexus_v1_startup_host_action_receipt_clear(out_receipt);
    out_receipt->host_receipt.input_result =
        NEXUS_V1_STARTUP_HOST_INPUT_REDRAW;
    out_receipt->host_receipt.status_scope = "ASSETS";
    out_receipt->host_receipt.status =
        assets && assets->startup_menu_asset_route
            ? assets->startup_menu_asset_route
            : "blocked-startup-assets";
    if (facts && nexus_v1_startup_menu_state_receipt_from_facts(
            &out_receipt->save_state_receipt,
            facts->save_dir,
            facts->slot_mask,
            facts->save_selected_row)) {
        out_receipt->save_state_receipt_valid = 1;
    }
}

static int nexus_v1_launcher_startup_save_assets_blocked(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_LauncherStartupAssetsReceipt *out_assets)
{
    if (!nexus_v1_launcher_startup_assets_from_runtime_state(state,
                                                             out_assets)) {
        return 0;
    }
    return out_assets->save_menu_route_ready ? 0 : 1;
}

static int nexus_v1_launcher_startup_champion_assets_blocked(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_LauncherStartupAssetsReceipt *out_assets)
{
    if (!nexus_v1_launcher_startup_assets_from_runtime_state(state,
                                                             out_assets)) {
        return 0;
    }
    return out_assets->champion_menu_route_ready ? 0 : 1;
}

static void nexus_v1_launcher_fill_champion_asset_blocked_receipt(
    const Nexus_V1_StartupHostFacts *facts,
    const Nexus_V1_LauncherStartupAssetsReceipt *assets,
    Nexus_V1_StartupChampionExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    if (out_execution) {
        memset(out_execution, 0, sizeof(*out_execution));
        out_execution->kind = NEXUS_V1_STARTUP_CHAMPION_EXEC_IGNORE;
        out_execution->cursor = -1;
        out_execution->status_scope = "ASSETS";
        out_execution->status = assets && assets->startup_menu_asset_route
            ? assets->startup_menu_asset_route
            : "blocked-startup-assets";
    }
    if (!out_receipt) {
        return;
    }
    nexus_v1_startup_host_action_receipt_clear(out_receipt);
    out_receipt->host_receipt.input_result =
        NEXUS_V1_STARTUP_HOST_INPUT_REDRAW;
    out_receipt->host_receipt.status_scope = "ASSETS";
    out_receipt->host_receipt.status =
        assets && assets->startup_menu_asset_route
            ? assets->startup_menu_asset_route
            : "blocked-startup-assets";
    if (facts && nexus_v1_startup_champion_state_receipt_from_facts(
            facts->champion_pool,
            &out_receipt->champion_state_receipt,
            facts->slot_mask,
            facts->champion_cursor,
            facts->champion_frame)) {
        out_receipt->champion_state_receipt_valid = 1;
    }
}

void nexus_v1_launcher_startup_runtime_state_clear(
    Nexus_V1_StartupRuntimeState *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
}

void nexus_v1_launcher_runtime_startup_snapshot_clear(
    Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot)
{
    if (!snapshot) {
        return;
    }
    memset(snapshot, 0, sizeof(*snapshot));
}

void nexus_v1_launcher_startup_menu_presentation_receipt_clear(
    Nexus_V1_StartupMenuPresentationReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->kind = NEXUS_V1_STARTUP_MENU_PRESENTATION_INVALID;
    nexus_v1_startup_host_receipt_clear(&receipt->host_receipt);
}

void nexus_v1_launcher_startup_title_handoff_receipt_clear(
    Nexus_V1_StartupTitleHandoffReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    nexus_v1_startup_title_route_receipt_clear(&receipt->title_route);
    nexus_v1_startup_host_receipt_clear(&receipt->host_receipt);
}

void nexus_v1_launcher_startup_runtime_handoff_receipt_clear(
    Nexus_V1_StartupRuntimeHandoffReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->route = NEXUS_V1_STARTUP_RUNTIME_HANDOFF_INVALID;
    receipt->dgn_handoff.status = NEXUS_V1_DGN_RENDERER_HANDOFF_MISSING;
    receipt->render_plan.status = NEXUS_V1_DGN_RENDERER_HANDOFF_MISSING;
    receipt->dgn_viewport_host_route_status =
        NEXUS_V1_DGN_HOST_ROUTE_MISSING;
    receipt->script_receipt.status = NEXUS_SCRIPT_RUNTIME_MISSING;
    receipt->script_receipt.level_index = -1;
    receipt->party_x = -1;
    receipt->party_y = -1;
    receipt->party_dir = -1;
    nexus_v1_startup_host_action_receipt_clear(
        &receipt->host_action_receipt);
}

void nexus_v1_launcher_startup_runtime_route_receipt_clear(
    Nexus_V1_StartupRuntimeRouteReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->route = NEXUS_V1_STARTUP_RUNTIME_HANDOFF_INVALID;
    receipt->startup_sfx_status = NEXUS_SFX_RUNTIME_MISSING;
    receipt->startup_sfx_level_index = -1;
    receipt->startup_cd_track = -1;
    receipt->dgn_handoff_status = NEXUS_V1_DGN_RENDERER_HANDOFF_MISSING;
    receipt->dgn_render_plan_status = NEXUS_V1_DGN_RENDERER_HANDOFF_MISSING;
    receipt->dgn_viewport_host_route_status =
        NEXUS_V1_DGN_HOST_ROUTE_MISSING;
    receipt->script_runtime_status = NEXUS_SCRIPT_RUNTIME_MISSING;
    nexus_v1_startup_host_action_receipt_clear(&receipt->host_action_receipt);
    nexus_v1_launcher_startup_runtime_handoff_receipt_clear(
        &receipt->runtime_handoff);
}

void nexus_v1_launcher_startup_route_proof_receipt_clear(
    Nexus_V1_StartupRouteProofReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->route = NEXUS_V1_STARTUP_ROUTE_PROOF_INVALID;
    nexus_v1_launcher_startup_launch_gate_receipt_clear(
        &receipt->launch_gate);
    nexus_v1_launcher_startup_asset_handoff_receipt_clear(
        &receipt->asset_handoff);
    nexus_v1_launcher_startup_title_handoff_receipt_clear(
        &receipt->title_handoff);
    nexus_v1_launcher_startup_menu_presentation_receipt_clear(
        &receipt->menu_presentation);
    nexus_v1_launcher_startup_runtime_route_receipt_clear(
        &receipt->runtime_route_receipt);
    nexus_v1_launcher_startup_runtime_handoff_receipt_clear(
        &receipt->runtime_handoff);
}

void nexus_v1_launcher_startup_full_start_receipt_clear(
    Nexus_V1_StartupFullStartReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->route = NEXUS_V1_STARTUP_FULL_START_INVALID;
    receipt->cd_track = -1;
    receipt->sfx_status = NEXUS_SFX_RUNTIME_MISSING;
    nexus_v1_launcher_startup_launch_gate_receipt_clear(
        &receipt->launch_gate);
    nexus_v1_launcher_startup_asset_handoff_receipt_clear(
        &receipt->asset_handoff);
    nexus_v1_startup_host_receipt_clear(&receipt->host_receipt);
}

void nexus_v1_launcher_startup_full_start_consumer_receipt_clear(
    Nexus_V1_StartupFullStartConsumerReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    nexus_v1_launcher_startup_full_start_receipt_clear(
        &receipt->full_start);
    nexus_v1_launcher_startup_title_handoff_receipt_clear(
        &receipt->title_handoff);
    nexus_v1_launcher_startup_menu_presentation_receipt_clear(
        &receipt->presentation);
    nexus_v1_startup_save_route_receipt_clear(&receipt->save_route);
}

void nexus_v1_launcher_startup_full_start_package_receipt_clear(
    Nexus_V1_StartupFullStartPackageReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    nexus_v1_launcher_startup_full_start_consumer_receipt_clear(
        &receipt->consumer);
    receipt->title_frame_max = 0;
    receipt->save_capture_frame = -1;
    receipt->champion_capture_frame = -1;
    receipt->dungeon_capture_frame = -1;
    receipt->saturn_save_capture_frame = -1;
    receipt->saturn_champion_capture_frame = -1;
    receipt->saturn_dungeon_capture_frame = -1;
    receipt->capture_route_expected_consumer_route = "blocked-startup";
    receipt->consumer_route = "blocked-startup";
}

void nexus_v1_launcher_m12_startup_package_receipt_clear(
    Nexus_V1_M12StartupPackageReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->capture_route = NEXUS_V1_STARTUP_CAPTURE_INVALID;
    receipt->first_capture_draw_kind = NEXUS_V1_STARTUP_DRAW_NONE;
    receipt->warning_capture_frame = 0;
    receipt->title_capture_frame = nexus_v1_boot_warning_frames();
    receipt->save_capture_frame = -1;
    receipt->champion_capture_frame = -1;
    receipt->dungeon_capture_frame = -1;
    receipt->gameover_capture_frame = 0;
    receipt->saturn_save_capture_frame = -1;
    receipt->saturn_champion_capture_frame = -1;
    receipt->saturn_dungeon_capture_frame = -1;
    receipt->game_id = "nexus";
    receipt->card_title_label = "DM Nexus";
    receipt->card_subtitle_label = "Saturn boot, title, save, champions";
    receipt->timing_summary_label = "warning 48f / title ready 102f";
    receipt->ready_status_label = "TITLE MENU READY";
    receipt->ready_detail_label = "TITLE, WARNING, SAVE, CHAMPIONS";
    receipt->path_label = "NEXUS TITLE MENU";
    receipt->contract_label =
        "NEXUS HOST-CALLER/FULL-START PACKAGE RECEIPTS";
    receipt->capture_label = "NEXUS TIMING CAPTURE PROOF";
    receipt->capture_route_label = "invalid";
    receipt->first_capture_draw_label = "none";
    receipt->next_step_label = "REQUIRED GAME DATA";
    receipt->active_proof_label = "REQUIRED GAME DATA";
    receipt->status_label = "DATA MISSING";
    receipt->detail_label = "KNOWN SLOT, HASH COVERAGE STILL GROWING";
    receipt->launch_status_label = "DATA MISSING";
    receipt->launch_detail_label = "KNOWN SLOT, HASH COVERAGE STILL GROWING";
    receipt->blocked_status_label = "DATA MISSING";
    receipt->blocked_detail_label = "KNOWN SLOT, HASH COVERAGE STILL GROWING";
}

void nexus_v1_launcher_startup_receipt_bundle_clear(
    Nexus_V1_StartupReceiptBundle *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    nexus_v1_launcher_startup_full_start_package_receipt_clear(
        &receipt->package);
    nexus_v1_launcher_m12_startup_package_receipt_clear(
        &receipt->m12_package);
    receipt->capture_route = NEXUS_V1_STARTUP_CAPTURE_INVALID;
    receipt->first_draw_kind = NEXUS_V1_STARTUP_DRAW_NONE;
    receipt->route_label = "invalid";
    receipt->first_draw_label = "none";
    receipt->active_capture_frame = -1;
    receipt->saturn_active_capture_frame = -1;
    receipt->status_scope = "STARTUP";
    receipt->status = "blocked-startup";
}

void nexus_v1_launcher_startup_real_asset_ownership_receipt_clear(
    Nexus_V1_StartupRealAssetOwnershipReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->route = NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_INVALID;
    nexus_v1_launcher_startup_receipt_bundle_clear(
        &receipt->startup_bundle);
    nexus_v1_launcher_startup_asset_handoff_receipt_clear(
        &receipt->asset_handoff);
    nexus_v1_launcher_startup_runtime_route_receipt_clear(
        &receipt->runtime_route);
    receipt->menu_bpk_handoff.status =
        NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_MISSING;
    receipt->dgn_handoff.status = NEXUS_V1_DGN_RENDERER_HANDOFF_MISSING;
    receipt->dgn_render_plan.status =
        NEXUS_V1_DGN_RENDERER_HANDOFF_MISSING;
    receipt->receipt_owner = "nexus-v1-launcher";
    receipt->asset_route = "blocked-startup";
    receipt->asset_blocker = "startup";
    receipt->active_capture_frame = -1;
    receipt->saturn_active_capture_frame = -1;
    receipt->saturn_save_capture_frame = -1;
    receipt->saturn_champion_capture_frame = -1;
    receipt->saturn_dungeon_capture_frame = -1;
    receipt->status_scope = "STARTUP";
    receipt->status = "blocked-startup";
}

#define NEXUS_V1_HOST_ROUTE_STARTUP_BIT  1u
#define NEXUS_V1_HOST_ROUTE_TITLE_BIT    2u
#define NEXUS_V1_HOST_ROUTE_SAVE_BIT     4u
#define NEXUS_V1_HOST_ROUTE_CHAMPION_BIT 8u
#define NEXUS_V1_HOST_ROUTE_DUNGEON_BIT  16u

void nexus_v1_launcher_startup_host_caller_receipt_clear(
    Nexus_V1_StartupHostCallerReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    nexus_v1_launcher_startup_real_asset_ownership_receipt_clear(
        &receipt->ownership);
    receipt->capture_route = NEXUS_V1_STARTUP_CAPTURE_INVALID;
    receipt->ownership_route = NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_INVALID;
    receipt->saturn_save_capture_frame = -1;
    receipt->saturn_champion_capture_frame = -1;
    receipt->saturn_dungeon_capture_frame = -1;
    receipt->host_active_capture_frame = -1;
    receipt->host_saturn_active_capture_frame = -1;
    receipt->host_route = "blocked-startup";
    receipt->startup_package_route = "blocked-startup";
    receipt->status_scope = "STARTUP";
    receipt->status = "blocked-startup";
}

void nexus_v1_launcher_complete_support_receipt_clear(
    Nexus_V1_CompleteSupportReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    nexus_v1_launcher_startup_host_caller_receipt_clear(
        &receipt->title_host);
    nexus_v1_launcher_startup_host_caller_receipt_clear(
        &receipt->save_host);
    nexus_v1_launcher_startup_host_caller_receipt_clear(
        &receipt->champion_host);
    receipt->expected_route_mask =
        NEXUS_V1_HOST_ROUTE_STARTUP_BIT |
        NEXUS_V1_HOST_ROUTE_TITLE_BIT |
        NEXUS_V1_HOST_ROUTE_SAVE_BIT |
        NEXUS_V1_HOST_ROUTE_CHAMPION_BIT |
        NEXUS_V1_HOST_ROUTE_DUNGEON_BIT;
    receipt->status_scope = "NEXUS";
    receipt->status = "invalid";
}

int nexus_v1_launcher_complete_support_receipt_from_host_routes(
    const Nexus_V1_StartupHostCallerReceipt *title_host,
    const Nexus_V1_StartupHostCallerReceipt *save_host,
    const Nexus_V1_StartupHostCallerReceipt *champion_host,
    Nexus_V1_CompleteSupportReceipt *out_receipt)
{
    unsigned int complete_mask;

    nexus_v1_launcher_complete_support_receipt_clear(out_receipt);
    if (!out_receipt || !title_host || !save_host || !champion_host) {
        return 0;
    }

    out_receipt->title_host = *title_host;
    out_receipt->save_host = *save_host;
    out_receipt->champion_host = *champion_host;
    out_receipt->title_route_complete =
        title_host->title_host_package_route_complete &&
        title_host->title_route_saturn_capture_exact &&
        title_host->host_all_route_timing_matrix_complete &&
        title_host->single_saturn_startup_owner_ready;
    out_receipt->save_route_complete =
        save_host->save_host_package_route_complete &&
        save_host->save_route_saturn_capture_exact &&
        save_host->host_all_route_timing_matrix_complete &&
        save_host->single_saturn_startup_owner_ready;
    out_receipt->champion_route_complete =
        champion_host->champion_host_package_route_complete &&
        champion_host->champion_route_saturn_capture_exact &&
        champion_host->host_all_route_timing_matrix_complete &&
        champion_host->single_saturn_startup_owner_ready;
    out_receipt->dungeon_capture_route_consumed =
        champion_host->dungeon_capture_route_consumed &&
        champion_host->dungeon_capture_route &&
        strcmp(champion_host->dungeon_capture_route,
               "runtime-dgn-handoff") == 0;
    out_receipt->dgn_material_surface_coverage_complete =
        champion_host->dgn_material_surface_coverage_complete &&
        champion_host->dgn_viewport_material_surface_count ==
            champion_host->dgn_command_count &&
        champion_host->dgn_viewport_floor_material_surface_count ==
            champion_host->ownership.dgn_render_plan.floor_count &&
        champion_host->dgn_viewport_wall_material_surface_count ==
            champion_host->ownership.dgn_render_plan.wall_count &&
        champion_host->dgn_viewport_ceiling_material_surface_count ==
            champion_host->ownership.dgn_render_plan.ceiling_count;
    out_receipt->dgn_material_semantics_complete =
        out_receipt->dgn_material_surface_coverage_complete &&
        champion_host->dgn_material_semantics_complete &&
        champion_host->ownership.dgn_render_plan.material_semantics_complete;
    out_receipt->dgn_material_path_consumed =
        out_receipt->dgn_material_surface_coverage_complete &&
        out_receipt->dgn_material_semantics_complete &&
        champion_host->host_route_consumes_dgn_material_path &&
        champion_host->host_runtime_dgn_material_path_consumed &&
        champion_host->host_execute_dgn_draws;
    out_receipt->bpk_material_surface_count =
        champion_host->bpk_material_surface_count;
    out_receipt->bpk_truecolor_material_surface_count =
        champion_host->bpk_truecolor_material_surface_count;
    out_receipt->bpk_prs3_material_surface_count =
        champion_host->bpk_prs3_material_surface_count;
    out_receipt->dgn_viewport_host_route_status =
        champion_host->dgn_viewport_host_route_status;
    out_receipt->dgn_viewport_host_route_ready =
        champion_host->dgn_viewport_host_route_ready ? 1 : 0;
    out_receipt->dgn_viewport_host_route_consumed =
        champion_host->dgn_viewport_host_route_consumed ? 1 : 0;
    out_receipt->dgn_viewport_host_route_package_consumed =
        champion_host->dgn_viewport_host_route_package_consumed ? 1 : 0;
    out_receipt->dgn_viewport_host_route_blocks_runtime =
        champion_host->dgn_viewport_host_route_blocks_runtime ? 1 : 0;
    out_receipt->dgn_viewport_capture_ready =
        champion_host->dgn_viewport_capture_ready ? 1 : 0;
    out_receipt->dgn_viewport_frame_hash =
        champion_host->dgn_viewport_frame_hash;
    out_receipt->dungeon_route_complete =
        champion_host->dungeon_host_package_route_complete &&
        out_receipt->dungeon_capture_route_consumed &&
        out_receipt->dgn_material_surface_coverage_complete &&
        out_receipt->dgn_material_path_consumed &&
        champion_host->dgn_viewport_host_route_ready &&
        champion_host->dgn_viewport_host_route_consumed &&
        champion_host->dgn_viewport_host_route_package_consumed &&
        !champion_host->dgn_viewport_host_route_blocks_runtime &&
        champion_host->dgn_viewport_capture_ready &&
        champion_host->dgn_viewport_frame_hash != 0u &&
        champion_host->dungeon_route_saturn_capture_exact &&
        champion_host->dungeon_startup_route_consumption_complete &&
        champion_host->host_execute_dgn_draws;
    out_receipt->dgn_mesh_runtime_complete =
        champion_host->host_runtime_dgn_ready &&
        out_receipt->dgn_material_surface_coverage_complete &&
        out_receipt->dgn_material_semantics_complete &&
        champion_host->dgn_handoff_consumed &&
        champion_host->host_runtime_dgn_material_path_consumed &&
        champion_host->dgn_command_count > 0 &&
        champion_host->dgn_viewport_host_route_ready &&
        champion_host->dgn_viewport_host_route_consumed &&
        champion_host->dgn_viewport_host_route_package_consumed &&
        !champion_host->dgn_viewport_host_route_blocks_runtime &&
        champion_host->dgn_viewport_capture_ready &&
        champion_host->dgn_viewport_frame_hash != 0u &&
        champion_host->copied_dgn_command_count ==
            champion_host->dgn_command_count;
    out_receipt->dgn_viewport_runtime_complete =
        champion_host->host_runtime_dgn_viewport_render_ready &&
        champion_host->dgn_viewport_host_route_ready &&
        champion_host->dgn_viewport_host_route_consumed &&
        champion_host->dgn_viewport_host_route_package_consumed &&
        !champion_host->dgn_viewport_host_route_blocks_runtime &&
        champion_host->dgn_viewport_capture_ready &&
        champion_host->dgn_viewport_frame_hash != 0u &&
        champion_host->dgn_viewport_rasterized_command_count ==
            champion_host->dgn_command_count &&
        champion_host->dgn_viewport_written_pixels > 0;
    out_receipt->startup_package_consumed_by_all_routes =
        title_host->startup_host_package_route_complete &&
        save_host->startup_host_package_route_complete &&
        champion_host->startup_host_package_route_complete &&
        title_host->full_start_package_consumed &&
        save_host->full_start_package_consumed &&
        champion_host->full_start_package_consumed;
    out_receipt->host_route_matrix_complete =
        title_host->host_all_route_matrix_complete &&
        save_host->host_all_route_matrix_complete &&
        champion_host->host_all_route_matrix_complete;
    out_receipt->saturn_timing_matrix_complete =
        title_host->host_all_route_timing_matrix_complete &&
        save_host->host_all_route_timing_matrix_complete &&
        champion_host->host_all_route_timing_matrix_complete;
    out_receipt->saturn_title_capture_frame =
        title_host->saturn_title_capture_frame;
    out_receipt->saturn_save_capture_frame =
        save_host->saturn_save_capture_frame;
    out_receipt->saturn_champion_capture_frame =
        champion_host->saturn_champion_capture_frame;
    out_receipt->saturn_dungeon_capture_frame =
        champion_host->saturn_dungeon_capture_frame;
    out_receipt->saturn_non_title_capture_mask =
        (save_host->host_saturn_non_title_capture_mask & 1u) |
        (champion_host->host_saturn_non_title_capture_mask & 2u) |
        (champion_host->host_saturn_non_title_capture_mask & 4u);
    out_receipt->saturn_expected_capture_mask =
        save_host->host_saturn_expected_capture_mask |
        champion_host->host_saturn_expected_capture_mask;
    out_receipt->saturn_non_title_capture_count =
        (out_receipt->saturn_non_title_capture_mask & 1u ? 1 : 0) +
        (out_receipt->saturn_non_title_capture_mask & 2u ? 1 : 0) +
        (out_receipt->saturn_non_title_capture_mask & 4u ? 1 : 0);
    out_receipt->saturn_non_title_capture_complete =
        out_receipt->saturn_non_title_capture_mask == 7u &&
        (out_receipt->saturn_expected_capture_mask & 7u) == 7u &&
        out_receipt->saturn_non_title_capture_count == 3 &&
        out_receipt->saturn_save_capture_frame ==
            save_host->ownership.startup_bundle.package.boot_start_ready_frames &&
        out_receipt->saturn_champion_capture_frame ==
            champion_host->ownership.startup_bundle.package.boot_start_ready_frames &&
        out_receipt->saturn_dungeon_capture_frame ==
            champion_host->ownership.startup_bundle.package.boot_start_ready_frames;
    out_receipt->no_fallback_visuals_enforced =
        title_host->no_fallback_visuals_enforced &&
        save_host->no_fallback_visuals_enforced &&
        champion_host->no_fallback_visuals_enforced &&
        title_host->suppress_legacy_placeholder_visuals &&
        save_host->suppress_legacy_placeholder_visuals &&
        champion_host->suppress_legacy_placeholder_visuals;
    out_receipt->fallback_visuals_permitted =
        title_host->ownership.fallback_visuals_permitted ||
        save_host->ownership.fallback_visuals_permitted ||
        champion_host->ownership.fallback_visuals_permitted;

    complete_mask = NEXUS_V1_HOST_ROUTE_STARTUP_BIT;
    if (out_receipt->title_route_complete) {
        complete_mask |= NEXUS_V1_HOST_ROUTE_TITLE_BIT;
    }
    if (out_receipt->save_route_complete) {
        complete_mask |= NEXUS_V1_HOST_ROUTE_SAVE_BIT;
    }
    if (out_receipt->champion_route_complete) {
        complete_mask |= NEXUS_V1_HOST_ROUTE_CHAMPION_BIT;
    }
    if (out_receipt->dungeon_route_complete &&
        out_receipt->dgn_mesh_runtime_complete &&
        out_receipt->dgn_viewport_runtime_complete) {
        complete_mask |= NEXUS_V1_HOST_ROUTE_DUNGEON_BIT;
    }
    out_receipt->complete_route_mask = complete_mask;
    out_receipt->all_nexus_startup_routes_complete =
        out_receipt->startup_package_consumed_by_all_routes &&
        out_receipt->title_route_complete &&
        out_receipt->save_route_complete &&
        out_receipt->champion_route_complete &&
        out_receipt->host_route_matrix_complete &&
        out_receipt->saturn_timing_matrix_complete &&
        out_receipt->saturn_non_title_capture_complete;
    out_receipt->all_nexus_runtime_routes_complete =
        out_receipt->all_nexus_startup_routes_complete &&
        out_receipt->dungeon_route_complete &&
        out_receipt->dgn_mesh_runtime_complete &&
        out_receipt->dgn_viewport_runtime_complete;
    out_receipt->complete_support_ready =
        out_receipt->all_nexus_runtime_routes_complete &&
        out_receipt->complete_route_mask == out_receipt->expected_route_mask &&
        out_receipt->no_fallback_visuals_enforced &&
        !out_receipt->fallback_visuals_permitted;
    out_receipt->status_scope = "NEXUS";
    out_receipt->status = out_receipt->complete_support_ready
        ? "complete-support-ready"
        : out_receipt->dgn_viewport_runtime_complete
        ? "incomplete-startup-route-matrix"
        : "incomplete-dgn-runtime";
    return 1;
}

static int nexus_v1_launcher_saturn_timing_exact(
    int boot_warning_frames,
    int boot_start_ready_frames,
    int title_frame_max)
{
    return boot_warning_frames == nexus_v1_boot_warning_frames() &&
           boot_start_ready_frames == nexus_v1_boot_start_ready_frames() &&
           title_frame_max == nexus_v1_boot_start_ready_frames();
}

static int nexus_v1_launcher_saturn_capture_frames_exact(
    int warning_capture_frame,
    int title_capture_frame,
    int gameover_capture_frame)
{
    return warning_capture_frame == 0 &&
           title_capture_frame == nexus_v1_boot_warning_frames() &&
           gameover_capture_frame == 0;
}

static int nexus_v1_launcher_saturn_full_start_capture_frames_exact(
    int warning_capture_frame,
    int title_capture_frame,
    int save_capture_frame,
    int champion_capture_frame,
    int dungeon_capture_frame,
    int gameover_capture_frame,
    int boot_start_ready_frames)
{
    if (!nexus_v1_launcher_saturn_capture_frames_exact(
            warning_capture_frame,
            title_capture_frame,
            gameover_capture_frame)) {
        return 0;
    }
    if (save_capture_frame != -1 &&
        save_capture_frame != boot_start_ready_frames) {
        return 0;
    }
    if (champion_capture_frame != -1 &&
        champion_capture_frame != boot_start_ready_frames) {
        return 0;
    }
    if (dungeon_capture_frame != -1 &&
        dungeon_capture_frame != boot_start_ready_frames) {
        return 0;
    }
    return 1;
}

static int nexus_v1_launcher_active_capture_frame(
    const Nexus_V1_StartupFullStartPackageReceipt *package)
{
    if (!package) {
        return -1;
    }
    switch (package->capture_route) {
    case NEXUS_V1_STARTUP_CAPTURE_TITLE:
        return package->title_capture_frame;
    case NEXUS_V1_STARTUP_CAPTURE_SAVE:
        return package->save_capture_frame;
    case NEXUS_V1_STARTUP_CAPTURE_CHAMPION:
        return package->champion_capture_frame;
    case NEXUS_V1_STARTUP_CAPTURE_BLOCKED:
    case NEXUS_V1_STARTUP_CAPTURE_MENU_IDLE:
    case NEXUS_V1_STARTUP_CAPTURE_INVALID:
    default:
        return -1;
    }
}

static int nexus_v1_launcher_saturn_active_capture_frame(
    const Nexus_V1_StartupFullStartPackageReceipt *package)
{
    if (!package) {
        return -1;
    }
    switch (package->capture_route) {
    case NEXUS_V1_STARTUP_CAPTURE_TITLE:
        return package->saturn_title_capture_frame;
    case NEXUS_V1_STARTUP_CAPTURE_SAVE:
        return package->saturn_save_capture_frame;
    case NEXUS_V1_STARTUP_CAPTURE_CHAMPION:
        return package->saturn_champion_capture_frame;
    case NEXUS_V1_STARTUP_CAPTURE_BLOCKED:
    case NEXUS_V1_STARTUP_CAPTURE_MENU_IDLE:
    case NEXUS_V1_STARTUP_CAPTURE_INVALID:
    default:
        return -1;
    }
}

static unsigned int nexus_v1_launcher_saturn_non_title_capture_mask(
    int save_capture_frame,
    int champion_capture_frame,
    int dungeon_capture_frame)
{
    unsigned int mask = 0;
    if (save_capture_frame >= 0) {
        mask |= 1u;
    }
    if (champion_capture_frame >= 0) {
        mask |= 2u;
    }
    if (dungeon_capture_frame >= 0) {
        mask |= 4u;
    }
    return mask;
}

static unsigned int nexus_v1_launcher_expected_non_title_capture_mask(
    Nexus_V1_StartupCaptureRoute route,
    int runtime_dgn_handoff_ready)
{
    switch (route) {
    case NEXUS_V1_STARTUP_CAPTURE_SAVE:
        return 1u;
    case NEXUS_V1_STARTUP_CAPTURE_CHAMPION:
        return runtime_dgn_handoff_ready ? 6u : 2u;
    case NEXUS_V1_STARTUP_CAPTURE_TITLE:
    case NEXUS_V1_STARTUP_CAPTURE_MENU_IDLE:
    case NEXUS_V1_STARTUP_CAPTURE_BLOCKED:
    case NEXUS_V1_STARTUP_CAPTURE_INVALID:
    default:
        return 0u;
    }
}

static int nexus_v1_launcher_capture_mask_count(unsigned int mask)
{
    int count = 0;
    while (mask) {
        count += (int)(mask & 1u);
        mask >>= 1;
    }
    return count;
}

static void nexus_v1_launcher_fill_bpk_material_counts(
    const Nexus_V1_Engine *engine,
    int *out_total,
    int *out_truecolor,
    int *out_prs3)
{
    int total = 0;
    int truecolor = 0;
    int prs3 = 0;
    if (engine) {
        total =
            engine->floor_materials.bpk_imported_surface_count +
            engine->wall_materials.bpk_imported_surface_count;
        truecolor =
            engine->floor_materials.bpk_truecolor_surface_count +
            engine->wall_materials.bpk_truecolor_surface_count;
        prs3 =
            engine->floor_materials.bpk_prs3_surface_count +
            engine->wall_materials.bpk_prs3_surface_count;
    }
    if (out_total) *out_total = total;
    if (out_truecolor) *out_truecolor = truecolor;
    if (out_prs3) *out_prs3 = prs3;
}

static int nexus_v1_launcher_startup_base_saturn_capture_exact(
    const Nexus_V1_StartupFullStartPackageReceipt *package)
{
    if (!package) {
        return 0;
    }
    return package->saturn_timing_exact &&
           package->saturn_capture_frames_exact &&
           package->saturn_warning_frame == 0 &&
           package->saturn_title_capture_frame ==
               nexus_v1_boot_warning_frames() &&
           package->saturn_title_ready_frame ==
               nexus_v1_boot_start_ready_frames() &&
           package->saturn_gameover_capture_frame == 0;
}

static unsigned int nexus_v1_launcher_expected_all_route_mask(
    Nexus_V1_StartupCaptureRoute route,
    int runtime_dgn_handoff_ready)
{
    unsigned int mask = NEXUS_V1_HOST_ROUTE_STARTUP_BIT;
    switch (route) {
    case NEXUS_V1_STARTUP_CAPTURE_TITLE:
        return mask | NEXUS_V1_HOST_ROUTE_TITLE_BIT;
    case NEXUS_V1_STARTUP_CAPTURE_SAVE:
        return mask | NEXUS_V1_HOST_ROUTE_SAVE_BIT;
    case NEXUS_V1_STARTUP_CAPTURE_CHAMPION:
        mask |= NEXUS_V1_HOST_ROUTE_CHAMPION_BIT;
        if (runtime_dgn_handoff_ready) {
            mask |= NEXUS_V1_HOST_ROUTE_DUNGEON_BIT;
        }
        return mask;
    case NEXUS_V1_STARTUP_CAPTURE_MENU_IDLE:
    case NEXUS_V1_STARTUP_CAPTURE_BLOCKED:
    case NEXUS_V1_STARTUP_CAPTURE_INVALID:
    default:
        return 0u;
    }
}

static const char *nexus_v1_launcher_expected_consumer_route_for_capture(
    Nexus_V1_StartupCaptureRoute route)
{
    switch (route) {
    case NEXUS_V1_STARTUP_CAPTURE_TITLE:
        return "title-warning";
    case NEXUS_V1_STARTUP_CAPTURE_SAVE:
        return "save-menu";
    case NEXUS_V1_STARTUP_CAPTURE_CHAMPION:
        return "champion-menu";
    case NEXUS_V1_STARTUP_CAPTURE_MENU_IDLE:
        return "startup-menu";
    case NEXUS_V1_STARTUP_CAPTURE_BLOCKED:
    case NEXUS_V1_STARTUP_CAPTURE_INVALID:
    default:
        return "blocked-startup";
    }
}

static int nexus_v1_launcher_consumer_route_matches_capture(
    Nexus_V1_StartupCaptureRoute route,
    const char *consumer_route,
    const char *expected_route)
{
    if (!consumer_route || !expected_route) {
        return 0;
    }
    if (strcmp(consumer_route, expected_route) == 0) {
        return 1;
    }
    return route == NEXUS_V1_STARTUP_CAPTURE_TITLE &&
           strcmp(consumer_route, "startup-menu") == 0;
}

const char *nexus_v1_launcher_startup_real_asset_ownership_route_name(
    Nexus_V1_StartupRealAssetOwnershipRoute route)
{
    switch (route) {
    case NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_INVALID:
        return "invalid";
    case NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_BLOCKED_ASSETS:
        return "blocked-assets";
    case NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_TITLE_CAPTURE:
        return "title-capture";
    case NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_MENU_CAPTURE:
        return "menu-capture";
    case NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_RUNTIME_HANDOFF:
        return "runtime-handoff";
    default:
        return "unknown";
    }
}

static const char *nexus_v1_launcher_m12_capture_route_label(
    Nexus_V1_StartupCaptureRoute route)
{
    switch (route) {
    case NEXUS_V1_STARTUP_CAPTURE_BLOCKED: return "blocked-startup";
    case NEXUS_V1_STARTUP_CAPTURE_TITLE: return "title-warning";
    case NEXUS_V1_STARTUP_CAPTURE_SAVE: return "save-menu";
    case NEXUS_V1_STARTUP_CAPTURE_CHAMPION: return "champion-menu";
    case NEXUS_V1_STARTUP_CAPTURE_MENU_IDLE: return "menu-idle";
    case NEXUS_V1_STARTUP_CAPTURE_INVALID:
    default:
        return "invalid";
    }
}

static const char *nexus_v1_launcher_m12_draw_kind_label(
    Nexus_V1_StartupDrawKind kind)
{
    switch (kind) {
    case NEXUS_V1_STARTUP_DRAW_TITLE_BACKGROUND:
        return "title-background";
    case NEXUS_V1_STARTUP_DRAW_FILL_RECT:
        return "fill-rect";
    case NEXUS_V1_STARTUP_DRAW_OUTLINE_RECT:
        return "outline-rect";
    case NEXUS_V1_STARTUP_DRAW_TEXT:
        return "text";
    case NEXUS_V1_STARTUP_DRAW_PORTRAIT:
        return "portrait";
    case NEXUS_V1_STARTUP_DRAW_BOOT_TITLE_FRAME:
        return "boot-title-frame";
    case NEXUS_V1_STARTUP_DRAW_WARNING_BACKGROUND:
        return "warning-background";
    case NEXUS_V1_STARTUP_DRAW_NONE:
    default:
        return "none";
    }
}

static void nexus_v1_launcher_m12_finalize_startup_display(
    Nexus_V1_M12StartupPackageReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    receipt->capture_route_label =
        nexus_v1_launcher_m12_capture_route_label(receipt->capture_route);
    receipt->first_capture_draw_label =
        nexus_v1_launcher_m12_draw_kind_label(
            receipt->first_capture_draw_kind);
    receipt->active_proof_label =
        receipt->packaged_capture_ready
            ? receipt->capture_label
            : receipt->next_step_label;
    if (receipt->packaged_capture_ready) {
        receipt->launch_status_label = "READY TO LAUNCH";
        receipt->launch_detail_label = receipt->path_label;
        receipt->blocked_status_label = "READY TO LAUNCH";
        receipt->blocked_detail_label = receipt->path_label;
    } else {
        receipt->launch_status_label = receipt->status_label;
        receipt->launch_detail_label = receipt->detail_label;
        receipt->blocked_status_label = receipt->status_label;
        receipt->blocked_detail_label = receipt->active_proof_label;
    }
}

int nexus_v1_launcher_m12_startup_package_from_data_gate(
    int supported,
    int data_ready,
    int version_ready,
    Nexus_V1_M12StartupPackageReceipt *out_receipt)
{
    Nexus_V1_TitleFrame title;

    nexus_v1_launcher_m12_startup_package_receipt_clear(out_receipt);
    if (!out_receipt) {
        return 0;
    }
    out_receipt->handled = 1;
    out_receipt->supported = supported ? 1 : 0;
    out_receipt->data_ready = data_ready ? 1 : 0;
    out_receipt->version_ready = version_ready ? 1 : 0;
    out_receipt->packaged_capture_expected = 0;
    out_receipt->startup_step_count = 7;
    out_receipt->boot_warning_frames = nexus_v1_boot_warning_frames();
    out_receipt->boot_start_ready_frames = nexus_v1_boot_start_ready_frames();
    /* Scanner availability proves only that launch may be attempted. Surface
     * ownership and every capture frame are runtime receipts, never flags. */
    out_receipt->saturn_warning_frame = out_receipt->warning_capture_frame;
    out_receipt->saturn_title_capture_frame =
        out_receipt->title_capture_frame;
    out_receipt->saturn_title_ready_frame =
        out_receipt->boot_start_ready_frames;
    out_receipt->saturn_gameover_capture_frame =
        out_receipt->gameover_capture_frame;
    if (nexus_v1_title_frame(out_receipt->boot_start_ready_frames,
                             NEXUS_FB_H,
                             &title)) {
        out_receipt->title_frame_max = out_receipt->boot_start_ready_frames;
        out_receipt->title_frames_until_ready = title.frames_until_ready;
        out_receipt->title_hold_frame = title.hold_frame;
        out_receipt->title_prompt_visible = title.prompt_visible;
        out_receipt->title_reveal_y0 = title.reveal_y0;
        out_receipt->title_reveal_y1 = title.reveal_y1;
        out_receipt->title_reveal_h = title.reveal_h;
    }

    if (!out_receipt->supported) {
        out_receipt->status_label = "UNSUPPORTED";
        out_receipt->detail_label = "RUNTIME NOT WIRED";
        out_receipt->next_step_label = "SUPPORTED RUNTIME";
        nexus_v1_launcher_m12_finalize_startup_display(out_receipt);
        return 1;
    }
    if (out_receipt->data_ready) out_receipt->startup_step_ready_count++;
    if (out_receipt->version_ready) out_receipt->startup_step_ready_count++;
    out_receipt->startup_menu_ready = 0;
    out_receipt->full_start_graphics_ready = 0;
    out_receipt->startup_contract_ready = 0;
    out_receipt->saturn_timing_exact =
        nexus_v1_launcher_saturn_timing_exact(
            out_receipt->boot_warning_frames,
            out_receipt->boot_start_ready_frames,
            out_receipt->title_frame_max);
    out_receipt->saturn_capture_frames_exact =
        nexus_v1_launcher_saturn_capture_frames_exact(
            out_receipt->warning_capture_frame,
            out_receipt->title_capture_frame,
            out_receipt->gameover_capture_frame);
    out_receipt->packaged_capture_ready = 0;
    out_receipt->full_start_package_receipt_ready = 0;
    out_receipt->host_display_caller_expected = 0;
    if (!out_receipt->data_ready) {
        out_receipt->capture_route = NEXUS_V1_STARTUP_CAPTURE_BLOCKED;
    } else if (!out_receipt->version_ready) {
        out_receipt->status_label = "VERSION MISSING";
        out_receipt->detail_label = "SELECTED VERSION IS NOT VERIFIED";
        out_receipt->next_step_label = "SELECTED VERSION";
        out_receipt->capture_route = NEXUS_V1_STARTUP_CAPTURE_BLOCKED;
    } else {
        out_receipt->status_label = "RUNTIME RECEIPT REQUIRED";
        out_receipt->detail_label =
            "CANONICAL NEXUS DATA MUST REACH THE RUNTIME";
        out_receipt->next_step_label = "CANONICAL RUNTIME RECEIPT";
        out_receipt->capture_route = NEXUS_V1_STARTUP_CAPTURE_BLOCKED;
    }
    nexus_v1_launcher_m12_finalize_startup_display(out_receipt);
    return 1;
}

int nexus_v1_launcher_m12_startup_package_from_full_start_package(
    const Nexus_V1_StartupFullStartPackageReceipt *package,
    Nexus_V1_M12StartupPackageReceipt *out_receipt)
{
    nexus_v1_launcher_m12_startup_package_receipt_clear(out_receipt);
    if (!package || !out_receipt) {
        return 0;
    }

    out_receipt->handled = 1;
    out_receipt->supported = 1;
    out_receipt->data_ready =
        package->warning_surface_loaded &&
        package->title_surface_loaded &&
        package->gameover_surface_loaded;
    out_receipt->version_ready = out_receipt->data_ready;
    out_receipt->startup_menu_ready = package->m12_ready;
    out_receipt->full_start_graphics_ready = package->graphics_ready;
    out_receipt->startup_contract_ready = package->m11_ready;
    out_receipt->packaged_capture_expected = package->capture_valid;
    out_receipt->packaged_capture_ready =
        package->capture_valid &&
        package->capture_route_ready &&
        package->full_start_package_receipt_ready &&
        package->host_display_caller_expected &&
        package->saturn_timing_exact &&
        package->saturn_capture_frames_exact &&
        !package->fallback_visuals_permitted;
    out_receipt->startup_step_count = 7;
    out_receipt->startup_step_ready_count =
        out_receipt->packaged_capture_ready ? 7 : 3;
    out_receipt->boot_warning_frames = package->boot_warning_frames;
    out_receipt->boot_start_ready_frames =
        package->boot_start_ready_frames;
    out_receipt->title_frame_max = package->title_frame_max;
    out_receipt->title_frames_until_ready =
        package->title_frames_until_ready;
    out_receipt->title_hold_frame = package->title_hold_frame;
    out_receipt->title_prompt_visible = package->title_prompt_visible;
    out_receipt->title_reveal_y0 = package->title_reveal_y0;
    out_receipt->title_reveal_y1 = package->title_reveal_y1;
    out_receipt->title_reveal_h = package->title_reveal_h;
    out_receipt->warning_surface_loaded = package->warning_surface_loaded;
    out_receipt->title_surface_loaded = package->title_surface_loaded;
    out_receipt->gameover_surface_loaded = package->gameover_surface_loaded;
    out_receipt->warning_capture_surface_ready =
        package->warning_capture_surface_ready;
    out_receipt->title_capture_surface_ready =
        package->title_capture_surface_ready;
    out_receipt->gameover_capture_surface_ready =
        package->gameover_capture_surface_ready;
    out_receipt->warning_capture_frame = package->warning_capture_frame;
    out_receipt->title_capture_frame = package->title_capture_frame;
    out_receipt->save_capture_frame = package->save_capture_frame;
    out_receipt->champion_capture_frame = package->champion_capture_frame;
    out_receipt->dungeon_capture_frame = package->dungeon_capture_frame;
    out_receipt->gameover_capture_frame = package->gameover_capture_frame;
    out_receipt->saturn_warning_frame = package->saturn_warning_frame;
    out_receipt->saturn_title_capture_frame =
        package->saturn_title_capture_frame;
    out_receipt->saturn_save_capture_frame =
        package->saturn_save_capture_frame;
    out_receipt->saturn_champion_capture_frame =
        package->saturn_champion_capture_frame;
    out_receipt->saturn_dungeon_capture_frame =
        package->saturn_dungeon_capture_frame;
    out_receipt->saturn_title_ready_frame =
        package->saturn_title_ready_frame;
    out_receipt->saturn_gameover_capture_frame =
        package->saturn_gameover_capture_frame;
    out_receipt->saturn_timing_exact = package->saturn_timing_exact;
    out_receipt->saturn_capture_frames_exact =
        package->saturn_capture_frames_exact;
    out_receipt->full_start_package_receipt_ready =
        package->full_start_package_receipt_ready;
    out_receipt->host_display_caller_expected =
        package->host_display_caller_expected;
    out_receipt->capture_command_count = package->capture_command_count;
    out_receipt->capture_route = package->capture_route;
    out_receipt->first_capture_draw_kind =
        package->first_capture_draw_kind;
    out_receipt->status_label = package->status
        ? package->status
        : out_receipt->status_label;
    out_receipt->detail_label = package->startup_ui_blocker
        ? package->startup_ui_blocker
        : out_receipt->detail_label;
    out_receipt->next_step_label =
        out_receipt->packaged_capture_ready ? "READY" : "NEXUS STARTUP";

    nexus_v1_launcher_m12_finalize_startup_display(out_receipt);
    return 1;
}

const char *nexus_v1_launcher_startup_runtime_handoff_route_name(
    Nexus_V1_StartupRuntimeHandoffRoute route)
{
    switch (route) {
    case NEXUS_V1_STARTUP_RUNTIME_HANDOFF_INVALID: return "invalid";
    case NEXUS_V1_STARTUP_RUNTIME_HANDOFF_ASSET_BLOCKED:
        return "asset-blocked";
    case NEXUS_V1_STARTUP_RUNTIME_HANDOFF_NOT_START: return "not-start";
    case NEXUS_V1_STARTUP_RUNTIME_HANDOFF_DGN_BLOCKED:
        return "dgn-blocked";
    case NEXUS_V1_STARTUP_RUNTIME_HANDOFF_READY_RENDER_STATE:
        return "ready-render-state";
    default: return "unknown";
    }
}

const char *nexus_v1_launcher_dgn_visual_blocker_from_render_plan(
    const Nexus_V1_DgnRenderPlanReceipt *render_plan)
{
    if (!render_plan) {
        return "missing-dgn-runtime";
    }
    if (render_plan->status ==
        NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE2_SOURCE) {
        return render_plan->structure2_source_materialization_bound &&
                   !render_plan->structure2_vdp1_palette_binding_proven
            ? "blocked-structure2-vdp1-palette"
            : "blocked-structure2-source";
    }
    if (render_plan->status ==
            NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_SEMANTICS &&
        (render_plan->structure1f_plan_item_entry_count > 0 ||
         render_plan->structure1f_plan_item_floor_command_count > 0 ||
         render_plan->structure1f_plan_item_floor_command_entry_count > 0) &&
        !render_plan->item_ibs_vdp1_command_proven) {
        return "blocked-item-ibs-vdp1-provenance";
    }
    if (render_plan->status ==
        NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1B_SELECTOR) {
        return "blocked-structure1b-selector";
    }
    if (render_plan->status ==
        NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE3_FACE_SEMANTICS) {
        return "blocked-structure3-face-semantics";
    }
    if (render_plan->status ==
        NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_SEMANTICS) {
        return "blocked-structure1f-semantics";
    }
    if (render_plan->status ==
        NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE2_ENVELOPE) {
        return "blocked-structure2-envelope";
    }
    if (render_plan->status ==
        NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_CANONICAL_SOURCE) {
        return "blocked-canonical-dgn-source";
    }
    if (render_plan->status != NEXUS_V1_DGN_RENDERER_HANDOFF_READY_MESH ||
        render_plan->blocks_real_dgn_mesh_render ||
        !render_plan->plan_ready) {
        return "blocked-dgn-material";
    }
    return "ready-dgn-visual-route";
}

const char *nexus_v1_launcher_startup_route_proof_route_name(
    Nexus_V1_StartupRouteProofRoute route)
{
    switch (route) {
    case NEXUS_V1_STARTUP_ROUTE_PROOF_INVALID: return "invalid";
    case NEXUS_V1_STARTUP_ROUTE_PROOF_ASSET_BLOCKED:
        return "asset-blocked";
    case NEXUS_V1_STARTUP_ROUTE_PROOF_TITLE_READY: return "title-ready";
    case NEXUS_V1_STARTUP_ROUTE_PROOF_MENU_READY: return "menu-ready";
    case NEXUS_V1_STARTUP_ROUTE_PROOF_RUNTIME_READY:
        return "runtime-ready";
    default: return "unknown";
    }
}

const char *nexus_v1_launcher_startup_full_start_route_name(
    Nexus_V1_StartupFullStartRoute route)
{
    switch (route) {
    case NEXUS_V1_STARTUP_FULL_START_INVALID: return "invalid";
    case NEXUS_V1_STARTUP_FULL_START_BLOCKED_ASSETS:
        return "blocked-assets";
    case NEXUS_V1_STARTUP_FULL_START_WARNING_TITLE_READY:
        return "warning-title-ready";
    case NEXUS_V1_STARTUP_FULL_START_MENU_READY: return "menu-ready";
    default: return "unknown";
    }
}

static void nexus_v1_launcher_fill_full_start_host_route(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupFullStartReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    receipt->host_receipt.input_result = NEXUS_V1_STARTUP_HOST_INPUT_REDRAW;
    receipt->host_receipt.status_scope = receipt->status_scope;
    receipt->host_receipt.status = receipt->status;
    receipt->m11_host_route = "blocked-startup";
    receipt->m11_host_route_ready = 0;
    if (state) {
        (void)nexus_v1_launcher_consume_m11_menu_bpk_no_draw_host(
            state->engine, state->launcher_route_epoch,
            state->menu_bpk_package_fnv1a64,
            &receipt->m11_menu_bpk_no_draw_host);
        if (state->saturn_card_direct_selected) {
            (void)nexus_v1_launcher_bind_saturn_card_m11_no_draw_startup(
                state->engine, state->launcher_route_epoch,
                state->saturn_card_fnv1a64,
                state->menu_bpk_package_fnv1a64, 1,
                &receipt->saturn_card_m11_no_draw_startup);
        }
    }

    if (!state || !receipt->full_start_menu_ready) {
        return;
    }
    receipt->host_receipt.status_scope = "STARTUP";
    if (state->save_select_active && receipt->save_status_ready) {
        receipt->m11_host_route = "save-menu";
        receipt->host_receipt.status = "NEXUS SAVE SELECT";
        receipt->m11_host_route_ready = 1;
    } else if (state->champion_select_active &&
               receipt->champion_status_ready) {
        if (state->saturn_card_direct_selected &&
            !receipt->saturn_card_m11_no_draw_startup.valid) {
            receipt->m11_host_route = "blocked-saturn-card-m11-binding";
            receipt->host_receipt.status = "NEXUS DATA ERROR";
            return;
        }
        receipt->m11_host_route = "champion-menu";
        receipt->host_receipt.status = "NEXUS CHAMPIONS";
        receipt->m11_host_route_ready = 1;
    } else if (state->title_active && receipt->title_status_ready) {
        receipt->m11_host_route = "title-warning";
        receipt->host_receipt.status = "NEXUS TITLE";
        receipt->m11_host_route_ready = 1;
    } else {
        receipt->m11_host_route = "startup-menu";
        receipt->host_receipt.status = "full-start-menu-ready";
        receipt->m11_host_route_ready = 1;
    }
}

int nexus_v1_launcher_startup_host_facts_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupHostFacts *out_facts)
{
    if (!out_facts) {
        return 0;
    }
    memset(out_facts, 0, sizeof(*out_facts));
    if (!state) {
        return 0;
    }
    out_facts->title_active = state->title_active;
    out_facts->title_frame = state->title_frame;
    out_facts->save_select_active = state->save_select_active;
    out_facts->champion_select_active = state->champion_select_active;
    out_facts->save_dir = state->save_dir;
    out_facts->slot_mask = state->slot_mask;
    out_facts->save_selected_row = state->save_selected_row;
    out_facts->save_row_count = state->save_row_count;
    out_facts->champion_pool = state->engine ? &state->engine->champions : NULL;
    out_facts->champion_cursor = state->champion_cursor;
    out_facts->champion_frame = state->champion_frame;
    return 1;
}

int nexus_v1_launcher_startup_host_facts_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupHostFacts *out_facts)
{
    if (!snapshot) {
        return 0;
    }
    return nexus_v1_launcher_startup_host_facts_from_runtime_state(
        &snapshot->runtime,
        out_facts);
}

int nexus_v1_launcher_startup_assets_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_LauncherStartupAssetsReceipt *out_receipt)
{
    if (!out_receipt) {
        return 0;
    }
    memset(out_receipt, 0, sizeof(*out_receipt));
    return nexus_v1_launcher_startup_assets_from_runtime_state(state,
                                                               out_receipt);
}

int nexus_v1_launcher_startup_assets_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_LauncherStartupAssetsReceipt *out_receipt)
{
    if (!snapshot) {
        if (out_receipt) {
            memset(out_receipt, 0, sizeof(*out_receipt));
        }
        return 0;
    }
    return nexus_v1_launcher_startup_assets_receipt_from_runtime_state(
        &snapshot->runtime,
        out_receipt);
}

int nexus_v1_launcher_startup_advance_idle_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupIdleReceipt *out_receipt)
{
    Nexus_V1_StartupHostFacts facts;
    if (!nexus_v1_launcher_startup_host_facts_from_runtime_state(
            state,
            &facts)) {
        return 0;
    }
    return nexus_v1_startup_advance_idle_from_host_facts_with_receipt(
        &facts,
        out_receipt);
}

int nexus_v1_launcher_startup_advance_idle_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupIdleReceipt *out_receipt)
{
    if (!snapshot) {
        return 0;
    }
    return nexus_v1_launcher_startup_advance_idle_from_runtime_state(
        &snapshot->runtime,
        out_receipt);
}

int nexus_v1_launcher_startup_execute_save_firestaff_input_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    Nexus_V1_StartupHostFacts facts;
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    if (!nexus_v1_launcher_startup_host_facts_from_runtime_state(
            state,
            &facts)) {
        return 0;
    }
    if (nexus_v1_launcher_startup_save_assets_blocked(state, &assets)) {
        nexus_v1_launcher_fill_save_asset_blocked_action(&facts,
                                                         &assets,
                                                         out_execution,
                                                         out_receipt);
        return out_receipt ? 1 : 0;
    }
    return nexus_v1_startup_execute_save_firestaff_input_from_host_facts_with_receipt(
        &facts,
        menu_input,
        load_save,
        load_userdata,
        out_execution,
        out_receipt);
}

int nexus_v1_launcher_startup_execute_save_firestaff_input_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    if (!snapshot) {
        return 0;
    }
    return nexus_v1_launcher_startup_execute_save_firestaff_input_from_runtime_state(
        &snapshot->runtime,
        menu_input,
        load_save,
        load_userdata,
        out_execution,
        out_receipt);
}

int nexus_v1_launcher_startup_execute_save_pointer_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int x,
    int y,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    Nexus_V1_StartupHostFacts facts;
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    if (!nexus_v1_launcher_startup_host_facts_from_runtime_state(
            state,
            &facts)) {
        return 0;
    }
    if (nexus_v1_launcher_startup_save_assets_blocked(state, &assets)) {
        nexus_v1_launcher_fill_save_asset_blocked_action(&facts,
                                                         &assets,
                                                         out_execution,
                                                         out_receipt);
        return out_receipt ? 1 : 0;
    }
    return nexus_v1_startup_execute_save_pointer_from_host_facts_with_receipt(
        &facts,
        x,
        y,
        load_save,
        load_userdata,
        out_execution,
        out_receipt);
}

int nexus_v1_launcher_startup_execute_save_pointer_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int x,
    int y,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    if (!snapshot) {
        return 0;
    }
    return nexus_v1_launcher_startup_execute_save_pointer_from_runtime_state(
        &snapshot->runtime,
        x,
        y,
        load_save,
        load_userdata,
        out_execution,
        out_receipt);
}

int nexus_v1_launcher_startup_save_route_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveRouteReceipt *out_receipt)
{
    Nexus_V1_StartupHostFacts facts;
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    if (!nexus_v1_launcher_startup_host_facts_from_runtime_state(
            state,
            &facts)) {
        return 0;
    }
    if (nexus_v1_launcher_startup_save_assets_blocked(state, &assets)) {
        nexus_v1_launcher_fill_save_asset_blocked_route(&facts,
                                                        &assets,
                                                        out_receipt);
        return out_receipt ? 1 : 0;
    }
    return nexus_v1_startup_save_route_receipt_from_host_facts_input(
        &facts,
        menu_input,
        load_save,
        load_userdata,
        out_receipt);
}

int nexus_v1_launcher_startup_save_route_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveRouteReceipt *out_receipt)
{
    if (!snapshot) {
        return 0;
    }
    return nexus_v1_launcher_startup_save_route_receipt_from_runtime_state(
        &snapshot->runtime,
        menu_input,
        load_save,
        load_userdata,
        out_receipt);
}

int nexus_v1_launcher_startup_save_pointer_route_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int x,
    int y,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveRouteReceipt *out_receipt)
{
    Nexus_V1_StartupHostFacts facts;
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    if (!nexus_v1_launcher_startup_host_facts_from_runtime_state(
            state,
            &facts)) {
        return 0;
    }
    if (nexus_v1_launcher_startup_save_assets_blocked(state, &assets)) {
        nexus_v1_launcher_fill_save_asset_blocked_route(&facts,
                                                        &assets,
                                                        out_receipt);
        return out_receipt ? 1 : 0;
    }
    return nexus_v1_startup_save_route_receipt_from_host_facts_pointer(
        &facts,
        x,
        y,
        load_save,
        load_userdata,
        out_receipt);
}

int nexus_v1_launcher_startup_save_pointer_route_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int x,
    int y,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveRouteReceipt *out_receipt)
{
    if (!snapshot) {
        return 0;
    }
    return nexus_v1_launcher_startup_save_pointer_route_receipt_from_runtime_state(
        &snapshot->runtime,
        x,
        y,
        load_save,
        load_userdata,
        out_receipt);
}

static void nexus_v1_launcher_fill_title_asset_blocked_action(
    const Nexus_V1_StartupTitleHandoffReceipt *handoff,
    Nexus_V1_StartupTitleExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    if (out_execution) {
        memset(out_execution, 0, sizeof(*out_execution));
        out_execution->kind = NEXUS_V1_STARTUP_TITLE_EXEC_IGNORE;
        out_execution->status_scope = "ASSETS";
        out_execution->status = handoff && handoff->status
            ? handoff->status
            : "blocked-startup-assets";
    }
    if (!out_receipt) {
        return;
    }
    nexus_v1_startup_host_action_receipt_clear(out_receipt);
    if (handoff) {
        out_receipt->host_receipt = handoff->host_receipt;
    }
}

int nexus_v1_launcher_startup_execute_title_firestaff_input_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupTitleExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    Nexus_V1_StartupHostFacts facts;
    Nexus_V1_StartupTitleHandoffReceipt handoff;
    if (nexus_v1_launcher_startup_title_handoff_receipt_from_runtime_state(
            state,
            menu_input,
            &handoff) &&
        handoff.route_blocked) {
        nexus_v1_launcher_fill_title_asset_blocked_action(&handoff,
                                                          out_execution,
                                                          out_receipt);
        return out_receipt ? 1 : 0;
    }
    if (!nexus_v1_launcher_startup_host_facts_from_runtime_state(
            state,
            &facts)) {
        return 0;
    }
    return nexus_v1_startup_execute_title_firestaff_input_from_host_facts_with_receipt(
        &facts,
        menu_input,
        out_execution,
        out_receipt);
}

int nexus_v1_launcher_startup_execute_title_firestaff_input_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupTitleExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    if (!snapshot) {
        return 0;
    }
    return nexus_v1_launcher_startup_execute_title_firestaff_input_from_runtime_state(
        &snapshot->runtime,
        menu_input,
        out_execution,
        out_receipt);
}

int nexus_v1_launcher_startup_execute_title_pointer_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupTitleExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    Nexus_V1_StartupHostFacts facts;
    Nexus_V1_StartupTitleHandoffReceipt handoff;
    if (nexus_v1_launcher_startup_title_pointer_handoff_receipt_from_runtime_state(
            state,
            &handoff) &&
        handoff.route_blocked) {
        nexus_v1_launcher_fill_title_asset_blocked_action(&handoff,
                                                          out_execution,
                                                          out_receipt);
        return out_receipt ? 1 : 0;
    }
    if (!nexus_v1_launcher_startup_host_facts_from_runtime_state(
            state,
            &facts)) {
        return 0;
    }
    return nexus_v1_startup_execute_title_pointer_from_host_facts_with_receipt(
        &facts,
        out_execution,
        out_receipt);
}

int nexus_v1_launcher_startup_execute_title_pointer_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupTitleExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    if (!snapshot) {
        return 0;
    }
    return nexus_v1_launcher_startup_execute_title_pointer_from_runtime_state(
        &snapshot->runtime,
        out_execution,
        out_receipt);
}

static int nexus_v1_launcher_validate_dgn_viewport_render(
    Nexus_V1_Engine *engine,
    Nexus_V1_DgnViewportRenderReceipt *out_receipt)
{
    Nexus_Viewport viewport;
    Nexus_V1_DgnViewportRenderReceipt receipt;

    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
    }
    if (!engine || !engine->level_loaded) {
        return 0;
    }

    nexus_viewport_init(&viewport);
    nexus_viewport_render(&viewport, engine);
    if (nexus_viewport_last_dgn_render_receipt(&viewport, &receipt) != 0) {
        return 0;
    }
    if (out_receipt) {
        *out_receipt = receipt;
    }
    return receipt.attempted &&
           receipt.ready &&
           receipt.used_real_dgn_route &&
           !receipt.blocked &&
           !receipt.fallback_visuals_permitted &&
           receipt.command_count > 0 &&
           receipt.command_count == receipt.material_surface_count &&
           receipt.command_count == receipt.rasterized_command_count &&
           receipt.written_pixels > 0 &&
           receipt.captured_frame_ready &&
           receipt.frame_hash != 0u;
}

static int nexus_v1_launcher_dgn_viewport_host_route_receipt(
    Nexus_V1_Engine *engine,
    Nexus_V1_DgnViewportHostRouteReceipt *out_receipt)
{
    Nexus_Viewport viewport;
    Nexus_V1_DgnViewportHostRouteReceipt receipt;

    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
    }
    if (!engine || !engine->level_loaded) {
        return 0;
    }

    nexus_viewport_init(&viewport);
    nexus_viewport_render(&viewport, engine);
    if (nexus_viewport_dgn_host_route_receipt(
            &viewport,
            engine,
            &receipt) != 0) {
        return 0;
    }
    if (out_receipt) {
        *out_receipt = receipt;
    }
    return receipt.package_consumed &&
           receipt.host_route_consumed &&
           receipt.can_present_runtime_dgn &&
           !receipt.blocks_runtime_dgn &&
           receipt.status == NEXUS_V1_DGN_HOST_ROUTE_READY_RENDERED_MESH;
}

static int nexus_v1_launcher_title_route_asset_ready(
    Nexus_V1_StartupTitleRoute route,
    const Nexus_V1_LauncherStartupAssetsReceipt *assets);

static void nexus_v1_launcher_fill_title_asset_blocked_route(
    const Nexus_V1_StartupTitleRouteReceipt *source,
    const Nexus_V1_LauncherStartupAssetsReceipt *assets,
    Nexus_V1_StartupTitleRouteReceipt *out_receipt);

/* M11 receives the dungeon only through the launcher handoff. Keep the
 * source-validated Structure1F/1G receipt intact at that boundary; this is a
 * data gate, not a Structure2 pixel decoder or animation executor. */
int nexus_v1_launcher_startup_title_route_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupTitleRouteReceipt *out_receipt)
{
    Nexus_V1_StartupHostFacts facts;
    Nexus_V1_StartupTitleRouteReceipt route;
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    if (!nexus_v1_launcher_startup_host_facts_from_runtime_state(
            state,
            &facts)) {
        return 0;
    }
    if (!nexus_v1_startup_title_route_receipt_from_host_facts_input(
        &facts,
        menu_input,
        &route)) {
        nexus_v1_startup_title_route_receipt_clear(out_receipt);
        return 0;
    }
    if (!nexus_v1_launcher_startup_assets_from_runtime_state(state, &assets)) {
        nexus_v1_startup_title_route_receipt_clear(out_receipt);
        return 0;
    }
    if (!nexus_v1_launcher_title_route_asset_ready(route.route, &assets)) {
        nexus_v1_launcher_fill_title_asset_blocked_route(&route,
                                                         &assets,
                                                         out_receipt);
        return 1;
    }
    if (out_receipt) {
        *out_receipt = route;
    }
    return 1;
}

int nexus_v1_launcher_startup_title_route_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupTitleRouteReceipt *out_receipt)
{
    if (!snapshot) {
        return 0;
    }
    return nexus_v1_launcher_startup_title_route_receipt_from_runtime_state(
        &snapshot->runtime,
        menu_input,
        out_receipt);
}

int nexus_v1_launcher_startup_title_pointer_route_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupTitleRouteReceipt *out_receipt)
{
    Nexus_V1_StartupHostFacts facts;
    Nexus_V1_StartupTitleRouteReceipt route;
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    if (!nexus_v1_launcher_startup_host_facts_from_runtime_state(
            state,
            &facts)) {
        return 0;
    }
    if (!nexus_v1_startup_title_route_receipt_from_host_facts_pointer(
        &facts,
        &route)) {
        nexus_v1_startup_title_route_receipt_clear(out_receipt);
        return 0;
    }
    if (!nexus_v1_launcher_startup_assets_from_runtime_state(state, &assets)) {
        nexus_v1_startup_title_route_receipt_clear(out_receipt);
        return 0;
    }
    if (!nexus_v1_launcher_title_route_asset_ready(route.route, &assets)) {
        nexus_v1_launcher_fill_title_asset_blocked_route(&route,
                                                         &assets,
                                                         out_receipt);
        return 1;
    }
    if (out_receipt) {
        *out_receipt = route;
    }
    return 1;
}

int nexus_v1_launcher_startup_title_pointer_route_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupTitleRouteReceipt *out_receipt)
{
    if (!snapshot) {
        return 0;
    }
    return nexus_v1_launcher_startup_title_pointer_route_receipt_from_runtime_state(
        &snapshot->runtime,
        out_receipt);
}

static int nexus_v1_launcher_title_route_asset_ready(
    Nexus_V1_StartupTitleRoute route,
    const Nexus_V1_LauncherStartupAssetsReceipt *assets)
{
    if (!assets) {
        return 0;
    }
    switch (route) {
    case NEXUS_V1_STARTUP_TITLE_ROUTE_SAVE_SELECT:
        return assets->save_menu_route_ready ? 1 : 0;
    case NEXUS_V1_STARTUP_TITLE_ROUTE_CHAMPION_SELECT:
        return assets->champion_menu_route_ready ? 1 : 0;
    case NEXUS_V1_STARTUP_TITLE_ROUTE_HOLD:
    case NEXUS_V1_STARTUP_TITLE_ROUTE_RETURN_TO_LAUNCHER:
        return 1;
    case NEXUS_V1_STARTUP_TITLE_ROUTE_ASSET_BLOCKED:
    case NEXUS_V1_STARTUP_TITLE_ROUTE_INVALID:
    default:
        return 0;
    }
}

static void nexus_v1_launcher_fill_title_asset_blocked_route(
    const Nexus_V1_StartupTitleRouteReceipt *source,
    const Nexus_V1_LauncherStartupAssetsReceipt *assets,
    Nexus_V1_StartupTitleRouteReceipt *out_receipt)
{
    Nexus_V1_StartupTitleRouteReceipt receipt;

    nexus_v1_startup_title_route_receipt_clear(&receipt);
    if (source) {
        receipt = *source;
    }
    receipt.route = NEXUS_V1_STARTUP_TITLE_ROUTE_ASSET_BLOCKED;
    receipt.handled = 1;
    receipt.execution_kind = NEXUS_V1_STARTUP_TITLE_EXEC_IGNORE;
    receipt.host_input_result = NEXUS_V1_STARTUP_HOST_INPUT_REDRAW;
    receipt.set_title_active = 0;
    receipt.set_title_frame = 0;
    receipt.set_save_select_active = 0;
    receipt.set_save_selected_row = 0;
    receipt.set_champion_select_active = 0;
    receipt.set_champion_cursor = 0;
    receipt.status_scope = "ASSETS";
    receipt.status = assets && assets->startup_menu_asset_route
        ? assets->startup_menu_asset_route
        : "blocked-startup-assets";
    if (out_receipt) {
        *out_receipt = receipt;
    }
}

static void nexus_v1_launcher_fill_title_handoff_receipt(
    Nexus_V1_StartupTitleHandoffReceipt *receipt,
    const Nexus_V1_StartupTitleRouteReceipt *title_route,
    const Nexus_V1_LauncherStartupAssetsReceipt *assets)
{
    int route_ready;

    if (!receipt || !title_route || !assets) {
        return;
    }
    route_ready = nexus_v1_launcher_title_route_asset_ready(
        title_route->route,
        assets);
    nexus_v1_launcher_startup_title_handoff_receipt_clear(receipt);
    receipt->title_route = *title_route;
    receipt->assets = *assets;
    receipt->title_draw_ready = assets->title_route_ready ? 1 : 0;
    receipt->save_menu_ready = assets->save_menu_route_ready ? 1 : 0;
    receipt->champion_menu_ready = assets->champion_menu_route_ready ? 1 : 0;
    receipt->route_ready = route_ready;
    receipt->route_blocked = route_ready ? 0 : 1;
    receipt->asset_route = assets->startup_menu_asset_route;
    receipt->asset_blocker = assets->real_menu_surface_blocker;
    receipt->status_scope = route_ready ? title_route->status_scope : "ASSETS";
    receipt->status = route_ready
        ? title_route->status
        : (assets->startup_menu_asset_route
               ? assets->startup_menu_asset_route
               : "blocked-startup-assets");
    receipt->host_receipt.input_result =
        route_ready ? title_route->host_input_result
                    : NEXUS_V1_STARTUP_HOST_INPUT_REDRAW;
    receipt->host_receipt.status_scope = receipt->status_scope;
    receipt->host_receipt.status = receipt->status;
    if (route_ready) {
        receipt->host_receipt.mode_update.set_title_active =
            title_route->set_title_active;
        receipt->host_receipt.mode_update.title_active =
            title_route->title_active;
        receipt->host_receipt.mode_update.set_title_frame =
            title_route->set_title_frame;
        receipt->host_receipt.mode_update.title_frame =
            title_route->next_title_frame;
        receipt->host_receipt.mode_update.set_save_select_active =
            title_route->set_save_select_active;
        receipt->host_receipt.mode_update.save_select_active =
            title_route->save_select_active;
        receipt->host_receipt.mode_update.set_save_selected_row =
            title_route->set_save_selected_row;
        receipt->host_receipt.mode_update.save_selected_row =
            title_route->save_selected_row;
        receipt->host_receipt.mode_update.set_champion_select_active =
            title_route->set_champion_select_active;
        receipt->host_receipt.mode_update.champion_select_active =
            title_route->champion_select_active;
        receipt->host_receipt.mode_update.set_champion_cursor =
            title_route->set_champion_cursor;
        receipt->host_receipt.mode_update.champion_cursor =
            title_route->champion_cursor;
    }
}

int nexus_v1_launcher_startup_title_handoff_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupTitleHandoffReceipt *out_receipt)
{
    Nexus_V1_StartupTitleRouteReceipt title_route;
    Nexus_V1_LauncherStartupAssetsReceipt assets;

    nexus_v1_launcher_startup_title_handoff_receipt_clear(out_receipt);
    if (!out_receipt ||
        !nexus_v1_launcher_startup_title_route_receipt_from_runtime_state(
            state,
            menu_input,
            &title_route) ||
        !nexus_v1_launcher_startup_assets_from_runtime_state(state,
                                                             &assets)) {
        return 0;
    }
    nexus_v1_launcher_fill_title_handoff_receipt(out_receipt,
                                                 &title_route,
                                                 &assets);
    return 1;
}

int nexus_v1_launcher_startup_title_handoff_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupTitleHandoffReceipt *out_receipt)
{
    if (!snapshot) {
        nexus_v1_launcher_startup_title_handoff_receipt_clear(out_receipt);
        return 0;
    }
    return nexus_v1_launcher_startup_title_handoff_receipt_from_runtime_state(
        &snapshot->runtime,
        menu_input,
        out_receipt);
}

int nexus_v1_launcher_startup_title_pointer_handoff_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupTitleHandoffReceipt *out_receipt)
{
    Nexus_V1_StartupTitleRouteReceipt title_route;
    Nexus_V1_LauncherStartupAssetsReceipt assets;

    nexus_v1_launcher_startup_title_handoff_receipt_clear(out_receipt);
    if (!out_receipt ||
        !nexus_v1_launcher_startup_title_pointer_route_receipt_from_runtime_state(
            state,
            &title_route) ||
        !nexus_v1_launcher_startup_assets_from_runtime_state(state,
                                                             &assets)) {
        return 0;
    }
    nexus_v1_launcher_fill_title_handoff_receipt(out_receipt,
                                                 &title_route,
                                                 &assets);
    return 1;
}

int nexus_v1_launcher_startup_title_pointer_handoff_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupTitleHandoffReceipt *out_receipt)
{
    if (!snapshot) {
        nexus_v1_launcher_startup_title_handoff_receipt_clear(out_receipt);
        return 0;
    }
    return nexus_v1_launcher_startup_title_pointer_handoff_receipt_from_runtime_state(
        &snapshot->runtime,
        out_receipt);
}

int nexus_v1_launcher_startup_execute_champion_firestaff_input_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupChampionExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    Nexus_V1_StartupHostFacts facts;
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    if (!nexus_v1_launcher_startup_host_facts_from_runtime_state(
            state,
            &facts)) {
        return 0;
    }
    if (nexus_v1_launcher_startup_champion_assets_blocked(state, &assets)) {
        nexus_v1_launcher_fill_champion_asset_blocked_receipt(
            &facts,
            &assets,
            out_execution,
            out_receipt);
        return out_receipt ? 1 : 0;
    }
    return nexus_v1_startup_execute_champion_firestaff_input_from_host_facts_with_receipt(
        &facts,
        menu_input,
        out_execution,
        out_receipt);
}

int nexus_v1_launcher_startup_execute_champion_firestaff_input_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupChampionExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    if (!snapshot) {
        return 0;
    }
    return nexus_v1_launcher_startup_execute_champion_firestaff_input_from_runtime_state(
        &snapshot->runtime,
        menu_input,
        out_execution,
        out_receipt);
}

int nexus_v1_launcher_startup_execute_champion_pointer_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int x,
    int y,
    Nexus_V1_StartupChampionExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    Nexus_V1_StartupHostFacts facts;
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    if (!nexus_v1_launcher_startup_host_facts_from_runtime_state(
            state,
            &facts)) {
        return 0;
    }
    if (nexus_v1_launcher_startup_champion_assets_blocked(state, &assets)) {
        nexus_v1_launcher_fill_champion_asset_blocked_receipt(
            &facts,
            &assets,
            out_execution,
            out_receipt);
        return out_receipt ? 1 : 0;
    }
    return nexus_v1_startup_execute_champion_pointer_from_host_facts_with_receipt(
        &facts,
        x,
        y,
        out_execution,
        out_receipt);
}

int nexus_v1_launcher_startup_execute_champion_pointer_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int x,
    int y,
    Nexus_V1_StartupChampionExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    if (!snapshot) {
        return 0;
    }
    return nexus_v1_launcher_startup_execute_champion_pointer_from_runtime_state(
        &snapshot->runtime,
        x,
        y,
        out_execution,
        out_receipt);
}

int nexus_v1_launcher_startup_runtime_handoff_from_champion_execution(
    const Nexus_V1_StartupRuntimeState *state,
    const Nexus_V1_StartupChampionExecution *execution,
    const Nexus_V1_StartupHostActionReceipt *host_action,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeHandoffReceipt *out_receipt)
{
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    Nexus_V1_StartupAssetHandoffReceipt asset_handoff;
    Nexus_V1_DgnRenderPlanReceipt render_plan;
    Nexus_V1_DgnRendererHandoffReceipt dgn_handoff;
    Nexus_V1_DgnViewportRenderReceipt viewport_receipt;
    Nexus_V1_DgnViewportHostRouteReceipt viewport_host_route;
    const Nexus_V1_DgnMaterialPlan *material_plan;
    Nexus_ScriptRuntimeReceipt script_receipt;
    int viewport_render_valid;

    nexus_v1_launcher_startup_runtime_handoff_receipt_clear(out_receipt);
    /* A caller may reuse its command array across startup frames. Clear it
     * before any gate so a rejected Structure2 source can never leave an old
     * plan drawable by the host. */
    if (out_commands && max_commands > 0) {
        memset(out_commands, 0,
               (size_t)max_commands * sizeof(out_commands[0]));
    }
    if (!state || !execution || !out_receipt ||
        !nexus_v1_launcher_startup_assets_from_runtime_state(state,
                                                             &assets) ||
        !nexus_v1_launcher_startup_asset_handoff_from_parts(
            state->engine,
            state->engine ? state->engine->level_loaded : 0,
            assets.title_route_ready,
            &assets,
            NULL,
            &asset_handoff)) {
        return 0;
    }

    out_receipt->champion_execution = *execution;
    if (host_action) {
        out_receipt->host_action_receipt = *host_action;
    }
    out_receipt->assets = assets;
    out_receipt->asset_handoff = asset_handoff;
    out_receipt->asset_route = assets.startup_menu_asset_route;
    out_receipt->fallback_visuals_permitted =
        asset_handoff.fallback_visuals_permitted;

    if (asset_handoff.blocks_main_menu_route ||
        !asset_handoff.real_asset_route_ready ||
        !assets.champion_menu_route_ready) {
        out_receipt->route =
            NEXUS_V1_STARTUP_RUNTIME_HANDOFF_ASSET_BLOCKED;
        out_receipt->dgn_render_blocked = 1;
        out_receipt->status_scope = "ASSETS";
        out_receipt->status = asset_handoff.status
            ? asset_handoff.status
            : assets.startup_menu_asset_route
            ? assets.startup_menu_asset_route
            : "blocked-startup-assets";
        return 1;
    }

    if (execution->kind != NEXUS_V1_STARTUP_CHAMPION_EXEC_START_DUNGEON) {
        out_receipt->route = NEXUS_V1_STARTUP_RUNTIME_HANDOFF_NOT_START;
        out_receipt->status_scope = execution->status_scope;
        out_receipt->status = execution->status;
        return 1;
    }

    out_receipt->level_loaded = state->engine && state->engine->level_loaded;
    if (!state->engine || !state->engine->level_loaded ||
        nexus_v1_current_level_dgn_renderer_handoff_receipt(
            state->engine,
            &dgn_handoff) != 0) {
        out_receipt->route = NEXUS_V1_STARTUP_RUNTIME_HANDOFF_DGN_BLOCKED;
        out_receipt->dgn_render_blocked = 1;
        out_receipt->status_scope = "DGN";
        out_receipt->status = "missing-dgn-runtime";
        return 1;
    }

    out_receipt->dgn_handoff = dgn_handoff;
    (void)nexus_v1_level_dgn_structure1_host_provenance_receipt(
        &state->engine->current_level, &out_receipt->structure1_host_provenance);
    out_receipt->structure1_host_provenance_consumed = 1;
    if (!out_receipt->structure1_host_provenance.can_prepare_runtime_dgn) {
        out_receipt->route = NEXUS_V1_STARTUP_RUNTIME_HANDOFF_DGN_BLOCKED;
        out_receipt->dgn_render_blocked = 1;
        out_receipt->status_scope = "DGN";
        out_receipt->status = nexus_v1_dgn_structure1_host_provenance_status_name(
            out_receipt->structure1_host_provenance.status);
        return 1;
    }
    (void)nexus_v1_current_level_structure2_source_receipt(
        state->engine, &out_receipt->structure2_source);
    (void)nexus_v1_dgn_static_material_source_receipt(
        state->engine, &out_receipt->static_material_sources);
    out_receipt->dgn_static_material_source_consumed =
        out_receipt->static_material_sources.canonical_pair_bound &&
        out_receipt->static_material_sources
            .structure1b_selector_binding_proven &&
        state->engine->floor_mns_material_route_valid &&
        state->engine->wall_mns_material_route_valid;
    out_receipt->dgn_route =
        nexus_v1_dgn_renderer_handoff_status_name(dgn_handoff.status);
    /* The runtime handoff must share the viewport's material-validated DGN
     * plan. Geometry alone is not a renderable Saturn dungeon frame. */
    material_plan = nexus_v1_prepare_dgn_material_plan(
        state->engine,
        state->engine->game.party_x,
        state->engine->game.party_y,
        state->engine->game.party_dir);
    render_plan = state->engine->dgn_material_plan.receipt;
    out_receipt->structure2_source_materialization_bound =
        render_plan.structure2_source_materialization_bound;
    out_receipt->structure2_vdp1_palette_binding_proven =
        render_plan.structure2_vdp1_palette_binding_proven;
    out_receipt->item_ibs_vdp1_command_proven =
        render_plan.item_ibs_vdp1_command_proven;
    (void)nexus_v1_launcher_dgn_viewport_host_route_receipt(
        state->engine,
        &viewport_host_route);
    if (viewport_host_route.status !=
        NEXUS_V1_DGN_HOST_ROUTE_MISSING) {
        out_receipt->dgn_viewport_host_route_status =
            (int)viewport_host_route.status;
        out_receipt->dgn_viewport_host_route_ready =
            viewport_host_route.can_present_runtime_dgn ? 1 : 0;
        out_receipt->dgn_viewport_host_route_consumed =
            viewport_host_route.host_route_consumed ? 1 : 0;
        out_receipt->dgn_viewport_host_route_package_consumed =
            viewport_host_route.package_consumed ? 1 : 0;
        out_receipt->dgn_viewport_host_route_blocks_runtime =
            viewport_host_route.blocks_runtime_dgn ? 1 : 0;
        out_receipt->dgn_viewport_capture_ready =
            viewport_host_route.captured_frame_ready ? 1 : 0;
        out_receipt->dgn_viewport_frame_hash =
            viewport_host_route.frame_hash;
    }
    if (!material_plan || !render_plan.plan_ready ||
        render_plan.blocks_real_dgn_mesh_render) {
        out_receipt->route = NEXUS_V1_STARTUP_RUNTIME_HANDOFF_DGN_BLOCKED;
        out_receipt->render_plan = render_plan;
        out_receipt->dgn_render_blocked = 1;
        out_receipt->fallback_visuals_permitted =
            render_plan.fallback_visuals_permitted;
        out_receipt->status_scope = "DGN";
        out_receipt->status =
            out_receipt->dgn_viewport_host_route_blocks_runtime
                ? nexus_viewport_dgn_host_route_status_name(
                      (Nexus_V1_DgnViewportHostRouteStatus)
                          out_receipt->dgn_viewport_host_route_status)
            : !material_plan
            ? nexus_v1_launcher_dgn_visual_blocker_from_render_plan(
                  &render_plan)
            : out_receipt->dgn_route
            ? out_receipt->dgn_route
            : "blocked-dgn-render";
        return 1;
    }
    if (!nexus_v1_launcher_has_real_dgn_admission(state->engine)) {
        out_receipt->route = NEXUS_V1_STARTUP_RUNTIME_HANDOFF_DGN_BLOCKED;
        out_receipt->render_plan = render_plan;
        out_receipt->dgn_render_blocked = 1;
        out_receipt->dgn_viewport_host_route_ready = 0;
        out_receipt->dgn_viewport_host_route_blocks_runtime = 1;
        out_receipt->dgn_viewport_capture_ready = 0;
        out_receipt->dgn_viewport_frame_hash = 0u;
        out_receipt->fallback_visuals_permitted = 0;
        out_receipt->status_scope = "DGN";
        out_receipt->status = "blocked-dgn-capture-required";
        return 1;
    }
    if (!out_commands || max_commands < render_plan.command_count) {
        out_receipt->route = NEXUS_V1_STARTUP_RUNTIME_HANDOFF_DGN_BLOCKED;
        out_receipt->render_plan = render_plan;
        out_receipt->dgn_render_blocked = 1;
        out_receipt->fallback_visuals_permitted =
            render_plan.fallback_visuals_permitted;
        out_receipt->status_scope = "DGN";
        out_receipt->status = out_receipt->dgn_route
            ? out_receipt->dgn_route
            : "blocked-dgn-render";
        return 1;
    }
    viewport_render_valid =
        nexus_v1_launcher_validate_dgn_viewport_render(
            state->engine,
            &viewport_receipt);
    if (!out_receipt->dgn_viewport_host_route_ready ||
        out_receipt->dgn_viewport_host_route_blocks_runtime ||
        !viewport_render_valid) {
        out_receipt->route = NEXUS_V1_STARTUP_RUNTIME_HANDOFF_DGN_BLOCKED;
        out_receipt->render_plan = render_plan;
        out_receipt->dgn_render_blocked = 1;
        out_receipt->dgn_viewport_render_ready = 0;
        out_receipt->viewport_rasterized_command_count =
            viewport_receipt.rasterized_command_count;
        out_receipt->viewport_material_surface_count =
            viewport_receipt.material_surface_count;
        out_receipt->viewport_floor_material_surface_count =
            viewport_receipt.floor_material_surface_count;
        out_receipt->viewport_ceiling_material_surface_count =
            viewport_receipt.ceiling_material_surface_count;
        out_receipt->viewport_wall_material_surface_count =
            viewport_receipt.wall_material_surface_count;
        nexus_v1_launcher_fill_bpk_material_counts(
            state->engine,
            &out_receipt->bpk_material_surface_count,
            &out_receipt->bpk_truecolor_material_surface_count,
            &out_receipt->bpk_prs3_material_surface_count);
        out_receipt->viewport_written_pixels = viewport_receipt.written_pixels;
        out_receipt->dgn_viewport_capture_ready =
            viewport_receipt.captured_frame_ready ? 1 : 0;
        out_receipt->dgn_viewport_frame_hash =
            viewport_receipt.frame_hash;
        out_receipt->fallback_visuals_permitted =
            viewport_receipt.fallback_visuals_permitted;
        out_receipt->status_scope = "DGN";
        out_receipt->status = out_receipt->dgn_viewport_host_route_blocks_runtime
            ? nexus_viewport_dgn_host_route_status_name(
                  (Nexus_V1_DgnViewportHostRouteStatus)
                      out_receipt->dgn_viewport_host_route_status)
            : "blocked-dgn-viewport";
        return 1;
    }
    memcpy(out_commands, material_plan->commands,
           (size_t)render_plan.command_count * sizeof(*out_commands));

    /* The startup host owns a copied plan, not a second geometry evaluation.
     * Keep the exact source/host command bytes bound before the route can
     * claim a real DGN handoff. */
    out_receipt->dgn_command_buffer_hash =
        nexus_v1_launcher_dgn_command_buffer_hash(
            material_plan->commands, render_plan.command_count);
    out_receipt->dgn_command_buffer_exact =
        out_receipt->dgn_command_buffer_hash != 0u &&
        out_receipt->dgn_command_buffer_hash ==
            nexus_v1_launcher_dgn_command_buffer_hash(
                out_commands, render_plan.command_count) &&
        memcmp(out_commands, material_plan->commands,
               (size_t)render_plan.command_count * sizeof(*out_commands)) == 0;
    if (!out_receipt->dgn_command_buffer_exact) {
        memset(out_commands, 0,
               (size_t)render_plan.command_count * sizeof(*out_commands));
        out_receipt->route = NEXUS_V1_STARTUP_RUNTIME_HANDOFF_DGN_BLOCKED;
        out_receipt->render_plan = render_plan;
        out_receipt->dgn_render_blocked = 1;
        out_receipt->status_scope = "DGN";
        out_receipt->status = "blocked-dgn-command-copy";
        return 1;
    }

    out_receipt->route =
        NEXUS_V1_STARTUP_RUNTIME_HANDOFF_READY_RENDER_STATE;
    out_receipt->render_plan = render_plan;
    memset(&script_receipt, 0, sizeof(script_receipt));
    if (nexus_v1_current_level_script_runtime_receipt(
            state->engine,
            &script_receipt) == 0) {
        out_receipt->script_receipt = script_receipt;
        out_receipt->script_runtime_blocked =
            script_receipt.blocks_real_script_dispatch ? 1 : 0;
        out_receipt->script_runtime_ready =
            !out_receipt->script_runtime_blocked &&
            script_receipt.status == NEXUS_SCRIPT_RUNTIME_READY_PARSED &&
            script_receipt.dispatch_enabled;
    }
    out_receipt->runtime_ready = 1;
    out_receipt->dgn_render_ready = 1;
    out_receipt->dgn_viewport_render_ready = 1;
    out_receipt->hud_ready = out_receipt->level_loaded ? 1 : 0;
    out_receipt->dgn_render_blocked = 0;
    out_receipt->party_x = state->engine->game.party_x;
    out_receipt->party_y = state->engine->game.party_y;
    out_receipt->party_dir = state->engine->game.party_dir;
    out_receipt->command_count = render_plan.command_count;
    out_receipt->viewport_rasterized_command_count =
        viewport_receipt.rasterized_command_count;
    out_receipt->viewport_material_surface_count =
        viewport_receipt.material_surface_count;
    out_receipt->viewport_floor_material_surface_count =
        viewport_receipt.floor_material_surface_count;
    out_receipt->viewport_ceiling_material_surface_count =
        viewport_receipt.ceiling_material_surface_count;
    out_receipt->viewport_wall_material_surface_count =
        viewport_receipt.wall_material_surface_count;
    out_receipt->dgn_render_floor_count = render_plan.floor_count;
    out_receipt->dgn_render_ceiling_count = render_plan.ceiling_count;
    out_receipt->dgn_render_wall_count = render_plan.wall_count;
    out_receipt->dgn_floor_material_command_count =
        render_plan.floor_material_command_count;
    out_receipt->dgn_ceiling_material_command_count =
        render_plan.ceiling_material_command_count;
    out_receipt->dgn_wall_material_command_count =
        render_plan.wall_material_command_count;
    out_receipt->dgn_material_semantics_complete =
        render_plan.material_semantics_complete;
    nexus_v1_launcher_fill_bpk_material_counts(
        state->engine,
        &out_receipt->bpk_material_surface_count,
        &out_receipt->bpk_truecolor_material_surface_count,
        &out_receipt->bpk_prs3_material_surface_count);
    out_receipt->viewport_written_pixels = viewport_receipt.written_pixels;
    out_receipt->dgn_viewport_capture_ready =
        viewport_receipt.captured_frame_ready ? 1 : 0;
    out_receipt->dgn_viewport_frame_hash =
        viewport_receipt.frame_hash;
    out_receipt->dgn_material_plan_consumed = 1;
    out_receipt->dgn_commands_copied_from_material_plan =
        render_plan.command_count > 0 && out_receipt->dgn_command_buffer_exact;
    out_receipt->dgn_material_viewport_consumed =
        viewport_receipt.rasterized_command_count ==
            render_plan.command_count &&
        viewport_receipt.material_surface_count == render_plan.command_count &&
        viewport_receipt.floor_material_surface_count ==
            render_plan.floor_count &&
        viewport_receipt.wall_material_surface_count ==
            render_plan.wall_count &&
        viewport_receipt.ceiling_material_surface_count ==
            render_plan.ceiling_count &&
        viewport_receipt.written_pixels > 0;
    out_receipt->bpk_material_path_consumed =
        out_receipt->bpk_material_surface_count > 0;
    out_receipt->fallback_visuals_permitted =
        render_plan.fallback_visuals_permitted;
    out_receipt->status_scope = "DGN";
    out_receipt->status = "ready-render-state";
    return 1;
}

int nexus_v1_launcher_startup_runtime_handoff_from_champion_execution_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    const Nexus_V1_StartupChampionExecution *execution,
    const Nexus_V1_StartupHostActionReceipt *host_action,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeHandoffReceipt *out_receipt)
{
    if (!snapshot) {
        nexus_v1_launcher_startup_runtime_handoff_receipt_clear(out_receipt);
        return 0;
    }
    return nexus_v1_launcher_startup_runtime_handoff_from_champion_execution(
        &snapshot->runtime,
        execution,
        host_action,
        out_commands,
        max_commands,
        out_receipt);
}

int nexus_v1_launcher_startup_runtime_handoff_from_champion_firestaff_input(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeHandoffReceipt *out_receipt)
{
    Nexus_V1_StartupChampionExecution execution;
    Nexus_V1_StartupHostActionReceipt host_action;

    memset(&execution, 0, sizeof(execution));
    nexus_v1_startup_host_action_receipt_clear(&host_action);
    nexus_v1_launcher_startup_runtime_handoff_receipt_clear(out_receipt);
    if (!nexus_v1_launcher_startup_execute_champion_firestaff_input_from_runtime_state(
            state,
            menu_input,
            &execution,
            &host_action)) {
        return 0;
    }
    return nexus_v1_launcher_startup_runtime_handoff_from_champion_execution(
        state,
        &execution,
        &host_action,
        out_commands,
        max_commands,
        out_receipt);
}

int nexus_v1_launcher_startup_runtime_handoff_from_champion_pointer(
    const Nexus_V1_StartupRuntimeState *state,
    int x,
    int y,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeHandoffReceipt *out_receipt)
{
    Nexus_V1_StartupChampionExecution execution;
    Nexus_V1_StartupHostActionReceipt host_action;

    memset(&execution, 0, sizeof(execution));
    nexus_v1_startup_host_action_receipt_clear(&host_action);
    nexus_v1_launcher_startup_runtime_handoff_receipt_clear(out_receipt);
    if (!nexus_v1_launcher_startup_execute_champion_pointer_from_runtime_state(
            state,
            x,
            y,
            &execution,
            &host_action)) {
        return 0;
    }
    return nexus_v1_launcher_startup_runtime_handoff_from_champion_execution(
        state,
        &execution,
        &host_action,
        out_commands,
        max_commands,
        out_receipt);
}

int nexus_v1_launcher_startup_runtime_handoff_from_champion_firestaff_input_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeHandoffReceipt *out_receipt)
{
    if (!snapshot) {
        nexus_v1_launcher_startup_runtime_handoff_receipt_clear(out_receipt);
        return 0;
    }
    return nexus_v1_launcher_startup_runtime_handoff_from_champion_firestaff_input(
        &snapshot->runtime,
        menu_input,
        out_commands,
        max_commands,
        out_receipt);
}

int nexus_v1_launcher_startup_runtime_handoff_from_champion_pointer_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int x,
    int y,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeHandoffReceipt *out_receipt)
{
    if (!snapshot) {
        nexus_v1_launcher_startup_runtime_handoff_receipt_clear(out_receipt);
        return 0;
    }
    return nexus_v1_launcher_startup_runtime_handoff_from_champion_pointer(
        &snapshot->runtime,
        x,
        y,
        out_commands,
        max_commands,
        out_receipt);
}

static void nexus_v1_launcher_fill_runtime_route_receipt(
    const Nexus_V1_StartupChampionExecution *execution,
    const Nexus_V1_StartupHostActionReceipt *host_action,
    const Nexus_V1_StartupRuntimeHandoffReceipt *handoff,
    const Nexus_V1_DgnRenderCommand *commands,
    Nexus_V1_StartupRuntimeRouteReceipt *out_receipt)
{
    if (!out_receipt || !handoff) {
        return;
    }
    nexus_v1_launcher_startup_runtime_route_receipt_clear(out_receipt);
    if (execution) {
        out_receipt->champion_execution = *execution;
    }
    if (host_action) {
        out_receipt->host_action_receipt = *host_action;
        out_receipt->host_action_valid = 1;
    }
    out_receipt->runtime_handoff = *handoff;
    out_receipt->route = handoff->route;
    out_receipt->runtime_route_ready =
        handoff->runtime_ready && handoff->script_runtime_ready;
    out_receipt->runtime_route_blocked =
        handoff->route == NEXUS_V1_STARTUP_RUNTIME_HANDOFF_ASSET_BLOCKED ||
        handoff->route == NEXUS_V1_STARTUP_RUNTIME_HANDOFF_DGN_BLOCKED ||
        !handoff->script_runtime_ready;
    out_receipt->startup_sfx_status = handoff->assets.startup_sfx_status;
    out_receipt->startup_sfx_level_index =
        handoff->assets.startup_sfx_level_index;
    out_receipt->startup_cd_track = handoff->assets.startup_cd_track;
    out_receipt->startup_audio_handoff_ready =
        handoff->assets.startup_audio_handoff_ready ? 1 : 0;
    out_receipt->startup_sfx_blocks_real_playback =
        handoff->assets.startup_sfx_blocks_real_playback ? 1 : 0;
    out_receipt->dgn_handoff_status = handoff->dgn_handoff.status;
    out_receipt->dgn_render_plan_status = handoff->render_plan.status;
    out_receipt->dgn_render_plan_ready =
        handoff->render_plan.plan_ready ? 1 : 0;
    out_receipt->dgn_render_command_count =
        handoff->render_plan.command_count;
    out_receipt->dgn_render_floor_count = handoff->render_plan.floor_count;
    out_receipt->dgn_render_ceiling_count =
        handoff->render_plan.ceiling_count;
    out_receipt->dgn_render_wall_count = handoff->render_plan.wall_count;
    out_receipt->dgn_floor_material_command_count =
        handoff->render_plan.floor_material_command_count;
    out_receipt->dgn_ceiling_material_command_count =
        handoff->render_plan.ceiling_material_command_count;
    out_receipt->dgn_wall_material_command_count =
        handoff->render_plan.wall_material_command_count;
    out_receipt->dgn_material_semantics_complete =
        handoff->render_plan.material_semantics_complete;
    out_receipt->structure2_source_materialization_bound =
        handoff->structure2_source_materialization_bound ? 1 : 0;
    out_receipt->structure2_vdp1_palette_binding_proven =
        handoff->structure2_vdp1_palette_binding_proven ? 1 : 0;
    out_receipt->item_ibs_vdp1_command_proven =
        handoff->item_ibs_vdp1_command_proven ? 1 : 0;
    out_receipt->dgn_static_material_source_consumed =
        handoff->dgn_static_material_source_consumed ? 1 : 0;
    out_receipt->dgn_viewport_render_ready =
        handoff->dgn_viewport_render_ready ? 1 : 0;
    out_receipt->dgn_viewport_rasterized_command_count =
        handoff->viewport_rasterized_command_count;
    out_receipt->dgn_viewport_material_surface_count =
        handoff->viewport_material_surface_count;
    out_receipt->dgn_viewport_floor_material_surface_count =
        handoff->viewport_floor_material_surface_count;
    out_receipt->dgn_viewport_ceiling_material_surface_count =
        handoff->viewport_ceiling_material_surface_count;
    out_receipt->dgn_viewport_wall_material_surface_count =
        handoff->viewport_wall_material_surface_count;
    out_receipt->dgn_viewport_host_route_status =
        handoff->dgn_viewport_host_route_status;
    out_receipt->dgn_viewport_host_route_ready =
        handoff->dgn_viewport_host_route_ready ? 1 : 0;
    out_receipt->dgn_viewport_host_route_consumed =
        handoff->dgn_viewport_host_route_consumed ? 1 : 0;
    out_receipt->dgn_viewport_host_route_package_consumed =
        handoff->dgn_viewport_host_route_package_consumed ? 1 : 0;
    out_receipt->dgn_viewport_host_route_blocks_runtime =
        handoff->dgn_viewport_host_route_blocks_runtime ? 1 : 0;
    out_receipt->dgn_viewport_capture_ready =
        handoff->dgn_viewport_capture_ready ? 1 : 0;
    out_receipt->dgn_viewport_frame_hash =
        handoff->dgn_viewport_frame_hash;
    out_receipt->bpk_material_surface_count =
        handoff->bpk_material_surface_count;
    out_receipt->bpk_truecolor_material_surface_count =
        handoff->bpk_truecolor_material_surface_count;
    out_receipt->bpk_prs3_material_surface_count =
        handoff->bpk_prs3_material_surface_count;
    out_receipt->dgn_material_plan_consumed =
        handoff->dgn_material_plan_consumed ? 1 : 0;
    out_receipt->dgn_commands_copied_from_material_plan =
        handoff->dgn_commands_copied_from_material_plan ? 1 : 0;
    out_receipt->dgn_command_buffer_exact =
        handoff->dgn_command_buffer_exact ? 1 : 0;
    out_receipt->dgn_command_buffer_hash = handoff->dgn_command_buffer_hash;
    out_receipt->dgn_material_viewport_consumed =
        handoff->dgn_material_viewport_consumed ? 1 : 0;
    out_receipt->bpk_material_path_consumed =
        handoff->bpk_material_path_consumed ? 1 : 0;
    out_receipt->dgn_viewport_written_pixels =
        handoff->viewport_written_pixels;
    out_receipt->dgn_blocks_real_mesh_render =
        handoff->render_plan.blocks_real_dgn_mesh_render ||
        !handoff->dgn_viewport_render_ready ||
        !handoff->dgn_viewport_host_route_ready ||
        handoff->dgn_viewport_host_route_blocks_runtime ||
        !handoff->dgn_viewport_capture_ready ||
        handoff->dgn_viewport_frame_hash == 0u ||
        !handoff->dgn_command_buffer_exact ||
        handoff->dgn_command_buffer_hash == 0u;
    if (commands && handoff->render_plan.command_count > 0) {
        out_receipt->first_dgn_render_command_kind = commands[0].kind;
    }
    out_receipt->script_runtime_status = handoff->script_receipt.status;
    out_receipt->script_runtime_ready =
        handoff->script_runtime_ready ? 1 : 0;
    out_receipt->script_runtime_blocked =
        handoff->script_runtime_blocked ? 1 : 0;
    out_receipt->script_candidate_source_bytes =
        handoff->script_receipt.candidate_source_bytes;
    out_receipt->script_rules_loaded = handoff->script_receipt.rules_loaded;
    out_receipt->consumed_by_nexus =
        handoff->route != NEXUS_V1_STARTUP_RUNTIME_HANDOFF_INVALID;
    out_receipt->fallback_visuals_permitted =
        handoff->fallback_visuals_permitted ? 1 : 0;
    out_receipt->status_scope = handoff->status_scope;
    out_receipt->status = handoff->status;
}

int nexus_v1_launcher_startup_runtime_route_from_champion_execution(
    const Nexus_V1_StartupRuntimeState *state,
    const Nexus_V1_StartupChampionExecution *execution,
    const Nexus_V1_StartupHostActionReceipt *host_action,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeRouteReceipt *out_receipt)
{
    Nexus_V1_StartupRuntimeHandoffReceipt handoff;

    nexus_v1_launcher_startup_runtime_route_receipt_clear(out_receipt);
    if (!nexus_v1_launcher_startup_runtime_handoff_from_champion_execution(
            state,
            execution,
            host_action,
            out_commands,
            max_commands,
            &handoff)) {
        return 0;
    }
    nexus_v1_launcher_fill_runtime_route_receipt(
        execution,
        host_action,
        &handoff,
        out_commands,
        out_receipt);
    return 1;
}

int nexus_v1_launcher_startup_runtime_route_from_champion_firestaff_input(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeRouteReceipt *out_receipt)
{
    Nexus_V1_StartupChampionExecution execution;
    Nexus_V1_StartupHostActionReceipt host_action;

    memset(&execution, 0, sizeof(execution));
    nexus_v1_startup_host_action_receipt_clear(&host_action);
    nexus_v1_launcher_startup_runtime_route_receipt_clear(out_receipt);
    if (!nexus_v1_launcher_startup_execute_champion_firestaff_input_from_runtime_state(
            state,
            menu_input,
            &execution,
            &host_action) ||
        !nexus_v1_launcher_startup_runtime_route_from_champion_execution(
            state,
            &execution,
            &host_action,
            out_commands,
            max_commands,
            out_receipt)) {
        return 0;
    }
    return 1;
}

int nexus_v1_launcher_startup_runtime_route_from_champion_pointer(
    const Nexus_V1_StartupRuntimeState *state,
    int x,
    int y,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeRouteReceipt *out_receipt)
{
    Nexus_V1_StartupChampionExecution execution;
    Nexus_V1_StartupHostActionReceipt host_action;

    memset(&execution, 0, sizeof(execution));
    nexus_v1_startup_host_action_receipt_clear(&host_action);
    nexus_v1_launcher_startup_runtime_route_receipt_clear(out_receipt);
    if (!nexus_v1_launcher_startup_execute_champion_pointer_from_runtime_state(
            state,
            x,
            y,
            &execution,
            &host_action) ||
        !nexus_v1_launcher_startup_runtime_route_from_champion_execution(
            state,
            &execution,
            &host_action,
            out_commands,
            max_commands,
            out_receipt)) {
        return 0;
    }
    return 1;
}

int nexus_v1_launcher_startup_runtime_route_from_champion_firestaff_input_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeRouteReceipt *out_receipt)
{
    if (!snapshot) {
        nexus_v1_launcher_startup_runtime_route_receipt_clear(out_receipt);
        return 0;
    }
    return nexus_v1_launcher_startup_runtime_route_from_champion_firestaff_input(
        &snapshot->runtime,
        menu_input,
        out_commands,
        max_commands,
        out_receipt);
}

int nexus_v1_launcher_startup_runtime_route_from_champion_pointer_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int x,
    int y,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeRouteReceipt *out_receipt)
{
    if (!snapshot) {
        nexus_v1_launcher_startup_runtime_route_receipt_clear(out_receipt);
        return 0;
    }
    return nexus_v1_launcher_startup_runtime_route_from_champion_pointer(
        &snapshot->runtime,
        x,
        y,
        out_commands,
        max_commands,
        out_receipt);
}

int nexus_v1_launcher_startup_route_proof_from_runtime_state(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_StartupRuntimeState *state,
    const Nexus_V1_StartupChampionExecution *execution,
    const Nexus_V1_StartupHostActionReceipt *host_action,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRouteProofReceipt *out_receipt)
{
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    Nexus_V1_StartupDrawCommand draw_commands[80];

    nexus_v1_launcher_startup_route_proof_receipt_clear(out_receipt);
    if (!out_receipt || !runtime || !state ||
        !nexus_v1_launcher_startup_launch_gate_from_runtime_receipt(
            runtime,
            &out_receipt->launch_gate) ||
        !nexus_v1_launcher_startup_assets_from_runtime_state(state,
                                                             &assets)) {
        return 0;
    }
    if (!nexus_v1_launcher_startup_asset_handoff_from_parts(
            state->engine,
            state->engine ? state->engine->level_loaded : runtime->level_loaded,
            assets.title_route_ready,
            &assets,
            runtime->startup_receipt.host_receipt.status,
            &out_receipt->asset_handoff)) {
        return 0;
    }

    out_receipt->assets = assets;
    out_receipt->saturn_asset_boot_ready =
        out_receipt->launch_gate.engine_ready &&
        out_receipt->launch_gate.level_loaded &&
        assets.title_route_ready &&
        assets.startup_audio_handoff_ready;
    out_receipt->title_route_ready = assets.title_route_ready ? 1 : 0;
    out_receipt->menu_route_ready =
        assets.save_menu_route_ready && assets.champion_menu_route_ready;
    out_receipt->title_art_loaded = assets.title_surface_loaded ? 1 : 0;
    out_receipt->warning_art_loaded = assets.warning_surface_loaded ? 1 : 0;
    out_receipt->startup_surfaces_real_ready =
        assets.startup_surfaces_expected > 0 &&
        assets.startup_surfaces_loaded == assets.startup_surfaces_expected &&
        assets.startup_surfaces_fallback == 0;
    out_receipt->faces_real_ready =
        assets.faces_expected > 0 &&
        assets.faces_loaded > 0 &&
        assets.faces_loaded + assets.faces_fallback ==
            assets.faces_expected;
    out_receipt->full_start_graphics_ready =
        out_receipt->startup_surfaces_real_ready &&
        out_receipt->faces_real_ready &&
        assets.real_menu_surface_route_ready;
    out_receipt->save_load_menu_route_ready =
        assets.save_menu_route_ready &&
        out_receipt->full_start_graphics_ready;
    out_receipt->startup_ui_route_ready =
        out_receipt->title_route_ready &&
        out_receipt->full_start_graphics_ready &&
        assets.startup_audio_handoff_ready;
    if (!out_receipt->title_route_ready) {
        out_receipt->startup_ui_blocker = "title";
    } else if (!out_receipt->startup_surfaces_real_ready) {
        out_receipt->startup_ui_blocker = "startup-surfaces";
    } else if (!out_receipt->faces_real_ready) {
        out_receipt->startup_ui_blocker = "faces";
    } else if (!assets.real_menu_surface_route_ready) {
        out_receipt->startup_ui_blocker =
            assets.real_menu_surface_blocker
                ? assets.real_menu_surface_blocker
                : "menu-bpk";
    } else if (!assets.startup_audio_handoff_ready) {
        out_receipt->startup_ui_blocker = "track02-sfx";
    } else {
        out_receipt->startup_ui_blocker = "none";
    }
    out_receipt->audio_ready = assets.startup_audio_handoff_ready ? 1 : 0;
    out_receipt->startup_sfx_status = assets.startup_sfx_status;
    out_receipt->startup_sfx_level_index = assets.startup_sfx_level_index;
    out_receipt->startup_cd_track = assets.startup_cd_track;
    out_receipt->startup_sfx_blocks_real_playback =
        assets.startup_sfx_blocks_real_playback ? 1 : 0;
    out_receipt->title_menu_route_ready =
        out_receipt->title_route_ready && out_receipt->menu_route_ready;
    out_receipt->fallback_visuals_permitted =
        assets.menu_bpk_fallback_visuals_permitted ||
        out_receipt->launch_gate.fallback_visuals_permitted ||
        out_receipt->asset_handoff.fallback_visuals_permitted;
    out_receipt->asset_route = assets.startup_menu_asset_route;
    out_receipt->status_scope = out_receipt->launch_gate.status_scope;
    out_receipt->status = out_receipt->launch_gate.status;

    if (state->title_active) {
        (void)nexus_v1_launcher_startup_title_handoff_receipt_from_runtime_state(
            state,
            9,
            &out_receipt->title_handoff);
        out_receipt->title_route =
            nexus_v1_startup_title_route_name(
                out_receipt->title_handoff.title_route.route);
    }

    if (state->save_select_active) {
        if (nexus_v1_launcher_startup_save_presentation_receipt_from_runtime_state(
                state,
                draw_commands,
                (int)(sizeof(draw_commands) / sizeof(draw_commands[0])),
                &out_receipt->menu_presentation)) {
            out_receipt->menu_route = out_receipt->menu_presentation.status;
        }
    } else if (state->champion_select_active) {
        if (nexus_v1_launcher_startup_champion_presentation_receipt_from_runtime_state(
                state,
                draw_commands,
                (int)(sizeof(draw_commands) / sizeof(draw_commands[0])),
                &out_receipt->menu_presentation)) {
            out_receipt->menu_route = out_receipt->menu_presentation.status;
        }
    }

    if (execution) {
        (void)nexus_v1_launcher_startup_runtime_route_from_champion_execution(
            state,
            execution,
            host_action,
            out_commands,
            max_commands,
            &out_receipt->runtime_route_receipt);
        out_receipt->runtime_handoff =
            out_receipt->runtime_route_receipt.runtime_handoff;
        out_receipt->runtime_route =
            nexus_v1_launcher_startup_runtime_handoff_route_name(
                out_receipt->runtime_route_receipt.route);
        out_receipt->runtime_route_ready =
            out_receipt->runtime_route_receipt.runtime_route_ready;
        out_receipt->menu_runtime_route_ready =
            out_receipt->menu_route_ready &&
            out_receipt->runtime_route_ready;
        out_receipt->dgn_viewport_host_route_status =
            out_receipt->runtime_route_receipt
                .dgn_viewport_host_route_status;
        out_receipt->dgn_viewport_host_route_ready =
            out_receipt->runtime_route_receipt
                .dgn_viewport_host_route_ready ? 1 : 0;
        out_receipt->dgn_viewport_host_route_consumed =
            out_receipt->runtime_route_receipt
                .dgn_viewport_host_route_consumed ? 1 : 0;
        out_receipt->dgn_viewport_host_route_package_consumed =
            out_receipt->runtime_route_receipt
                .dgn_viewport_host_route_package_consumed ? 1 : 0;
        out_receipt->dgn_viewport_host_route_blocks_runtime =
            out_receipt->runtime_route_receipt
                .dgn_viewport_host_route_blocks_runtime ? 1 : 0;
        out_receipt->dgn_viewport_capture_ready =
            out_receipt->runtime_route_receipt
                .dgn_viewport_capture_ready ? 1 : 0;
        out_receipt->dgn_viewport_frame_hash =
            out_receipt->runtime_route_receipt
                .dgn_viewport_frame_hash;
        out_receipt->first_runtime_route_ready =
            out_receipt->runtime_handoff.render_plan.plan_ready &&
            out_receipt->runtime_handoff.command_count > 0 &&
            out_receipt->dgn_viewport_host_route_ready &&
            out_receipt->dgn_viewport_host_route_consumed &&
            out_receipt->dgn_viewport_host_route_package_consumed &&
            !out_receipt->dgn_viewport_host_route_blocks_runtime &&
            out_receipt->dgn_viewport_capture_ready &&
            out_receipt->dgn_viewport_frame_hash != 0u &&
            !out_receipt->runtime_route_receipt.dgn_blocks_real_mesh_render &&
            !out_receipt->runtime_handoff.render_plan
                 .blocks_real_dgn_mesh_render;
        out_receipt->first_runtime_route =
            out_receipt->first_runtime_route_ready
                ? "first-dgn-render-state"
                : out_receipt->runtime_route;
        out_receipt->script_runtime_status =
            out_receipt->runtime_route_receipt.script_runtime_status;
        out_receipt->script_candidate_source_bytes =
            out_receipt->runtime_route_receipt.script_candidate_source_bytes;
        out_receipt->script_runtime_route_blocked =
            out_receipt->runtime_route_receipt.script_runtime_blocked;
        out_receipt->script_runtime_route_ready =
            out_receipt->runtime_route_receipt.script_runtime_ready;
    }

    out_receipt->graphics_ready =
        assets.title_route_ready &&
        out_receipt->full_start_graphics_ready &&
        (!execution || out_receipt->first_runtime_route_ready);
    out_receipt->audio_runtime_route_ready =
        out_receipt->audio_ready && out_receipt->first_runtime_route_ready;
    out_receipt->audio_runtime_route_blocked =
        out_receipt->audio_runtime_route_ready ? 0 : 1;
    out_receipt->full_startup_route_ready =
        out_receipt->saturn_asset_boot_ready &&
        out_receipt->title_menu_route_ready &&
        out_receipt->menu_runtime_route_ready &&
        out_receipt->first_runtime_route_ready &&
        out_receipt->audio_runtime_route_ready &&
        !out_receipt->script_runtime_route_blocked &&
        !out_receipt->fallback_visuals_permitted;

    if (out_receipt->runtime_route_ready) {
        out_receipt->route = NEXUS_V1_STARTUP_ROUTE_PROOF_RUNTIME_READY;
        out_receipt->status_scope = "RUNTIME";
        out_receipt->status = "runtime-ready";
    } else if (out_receipt->launch_gate.route ==
                   NEXUS_V1_STARTUP_LAUNCH_GATE_DATA_ERROR ||
               out_receipt->launch_gate.route ==
                   NEXUS_V1_STARTUP_LAUNCH_GATE_MENU_ASSET_BLOCKED ||
               !out_receipt->asset_handoff.title_asset_handoff_ready ||
               out_receipt->asset_handoff.blocks_main_menu_route ||
               !assets.title_route_ready ||
               assets.real_menu_surface_route_blocked) {
        out_receipt->route = NEXUS_V1_STARTUP_ROUTE_PROOF_ASSET_BLOCKED;
        out_receipt->status_scope = out_receipt->asset_handoff.status_scope
            ? out_receipt->asset_handoff.status_scope
            : out_receipt->launch_gate.status_scope
            ? out_receipt->launch_gate.status_scope
            : "ASSETS";
        out_receipt->status = out_receipt->asset_handoff.status
            ? out_receipt->asset_handoff.status
            : out_receipt->launch_gate.status
            ? out_receipt->launch_gate.status
            : (assets.startup_menu_asset_route
                   ? assets.startup_menu_asset_route
                   : "blocked-startup-assets");
    } else if (out_receipt->menu_route_ready) {
        out_receipt->route = NEXUS_V1_STARTUP_ROUTE_PROOF_MENU_READY;
        out_receipt->status_scope = "STARTUP";
        out_receipt->status = "menu-ready";
    } else if (out_receipt->title_route_ready) {
        out_receipt->route = NEXUS_V1_STARTUP_ROUTE_PROOF_TITLE_READY;
        out_receipt->status_scope = "STARTUP";
        out_receipt->status = "title-ready";
    }
    return 1;
}

int nexus_v1_launcher_startup_full_start_receipt_from_runtime_state(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupFullStartReceipt *out_receipt)
{
    Nexus_V1_LauncherStartupAssetsReceipt assets;

    nexus_v1_launcher_startup_full_start_receipt_clear(out_receipt);
    if (!out_receipt || !runtime || !state ||
        !nexus_v1_launcher_startup_launch_gate_from_runtime_receipt(
            runtime,
            &out_receipt->launch_gate) ||
        !nexus_v1_launcher_startup_assets_from_runtime_state(state,
                                                             &assets)) {
        return 0;
    }
    if (!nexus_v1_launcher_startup_asset_handoff_from_parts(
            state->engine,
            state->engine ? state->engine->level_loaded : runtime->level_loaded,
            assets.title_route_ready,
            &assets,
            runtime->startup_receipt.host_receipt.status,
            &out_receipt->asset_handoff)) {
        return 0;
    }

    out_receipt->assets = assets;
    out_receipt->warning_art_loaded = assets.warning_surface_loaded ? 1 : 0;
    out_receipt->title_art_loaded = assets.title_surface_loaded ? 1 : 0;
    out_receipt->gameover_art_loaded =
        assets.gameover_surface_loaded ? 1 : 0;
    out_receipt->warning_capture_surface_ready =
        out_receipt->warning_art_loaded;
    out_receipt->title_capture_surface_ready =
        out_receipt->title_art_loaded && assets.title_route_ready;
    out_receipt->gameover_capture_surface_ready =
        out_receipt->gameover_art_loaded;
    out_receipt->warning_status_ready = out_receipt->warning_art_loaded;
    out_receipt->title_status_ready =
        out_receipt->title_art_loaded && assets.title_route_ready;
    out_receipt->boot_warning_title_ready =
        out_receipt->warning_art_loaded &&
        out_receipt->title_art_loaded &&
        assets.title_route_ready;
    out_receipt->startup_surfaces_real_ready =
        assets.startup_surfaces_expected > 0 &&
        assets.startup_surfaces_loaded == assets.startup_surfaces_expected &&
        assets.startup_surfaces_fallback == 0;
    out_receipt->faces_real_ready =
        assets.faces_expected > 0 &&
        assets.faces_loaded > 0 &&
        assets.faces_loaded + assets.faces_fallback ==
            assets.faces_expected;
    out_receipt->menu_bpk_route_ready =
        assets.real_menu_surface_route_ready ? 1 : 0;
    out_receipt->save_menu_route_ready =
        assets.save_menu_route_ready ? 1 : 0;
    out_receipt->champion_menu_route_ready =
        assets.champion_menu_route_ready ? 1 : 0;
    out_receipt->audio_track02_ready =
        assets.startup_audio_handoff_ready ? 1 : 0;
    out_receipt->cd_track = assets.startup_cd_track;
    out_receipt->sfx_status = assets.startup_sfx_status;
    out_receipt->sfx_blocks_real_playback =
        assets.startup_sfx_blocks_real_playback ? 1 : 0;
    out_receipt->full_start_graphics_ready =
        out_receipt->boot_warning_title_ready &&
        out_receipt->startup_surfaces_real_ready &&
        out_receipt->faces_real_ready &&
        out_receipt->menu_bpk_route_ready;
    out_receipt->save_status_ready =
        out_receipt->full_start_graphics_ready &&
        out_receipt->save_menu_route_ready &&
        out_receipt->audio_track02_ready;
    out_receipt->champion_status_ready =
        out_receipt->full_start_graphics_ready &&
        out_receipt->champion_menu_route_ready &&
        out_receipt->audio_track02_ready;
    out_receipt->full_start_menu_ready =
        out_receipt->full_start_graphics_ready &&
        out_receipt->save_menu_route_ready &&
        out_receipt->champion_menu_route_ready &&
        out_receipt->audio_track02_ready &&
        !out_receipt->asset_handoff.fallback_visuals_permitted;
    out_receipt->fallback_visuals_permitted =
        out_receipt->asset_handoff.fallback_visuals_permitted;
    out_receipt->asset_route = assets.startup_menu_asset_route;

    if (!out_receipt->boot_warning_title_ready) {
        out_receipt->startup_ui_blocker = "title-warning";
    } else if (!out_receipt->startup_surfaces_real_ready) {
        out_receipt->startup_ui_blocker = "startup-surfaces";
    } else if (!out_receipt->faces_real_ready) {
        out_receipt->startup_ui_blocker = "faces";
    } else if (!out_receipt->menu_bpk_route_ready) {
        out_receipt->startup_ui_blocker =
            assets.real_menu_surface_blocker
                ? assets.real_menu_surface_blocker
                : "menu-bpk";
    } else if (!out_receipt->audio_track02_ready) {
        out_receipt->startup_ui_blocker = "track02-sfx";
    } else {
        out_receipt->startup_ui_blocker = "none";
    }

    if (out_receipt->full_start_menu_ready) {
        out_receipt->route = NEXUS_V1_STARTUP_FULL_START_MENU_READY;
        out_receipt->status_scope = "STARTUP";
        out_receipt->status = "full-start-menu-ready";
    } else if (out_receipt->boot_warning_title_ready &&
               !out_receipt->asset_handoff.blocks_main_menu_route) {
        out_receipt->route =
            NEXUS_V1_STARTUP_FULL_START_WARNING_TITLE_READY;
        out_receipt->status_scope = "STARTUP";
        out_receipt->status = "warning-title-ready";
    } else {
        out_receipt->route = NEXUS_V1_STARTUP_FULL_START_BLOCKED_ASSETS;
        out_receipt->status_scope = "ASSETS";
        out_receipt->status = out_receipt->asset_handoff.status
            ? out_receipt->asset_handoff.status
            : out_receipt->startup_ui_blocker;
    }
    nexus_v1_launcher_fill_full_start_host_route(state, out_receipt);
    return 1;
}

int nexus_v1_launcher_startup_full_start_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupFullStartReceipt *out_receipt)
{
    if (!snapshot) {
        nexus_v1_launcher_startup_full_start_receipt_clear(out_receipt);
        return 0;
    }
    return nexus_v1_launcher_startup_full_start_receipt_from_runtime_state(
        runtime,
        &snapshot->runtime,
        out_receipt);
}

static int nexus_v1_launcher_build_runtime_receipt_from_startup_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_LauncherRuntimeReceipt *out_receipt)
{
    if (!state || !out_receipt) {
        return 0;
    }
    nexus_v1_launcher_runtime_receipt_clear(out_receipt);
    out_receipt->engine = state->engine;
    out_receipt->level_loaded =
        state->engine ? state->engine->level_loaded : 0;
    out_receipt->title_loaded =
        state->title_active ||
        nexus_v1_surface_loaded(state->engine, NEXUS_SURFACE_TITLE);
    out_receipt->startup_receipt.host_receipt.status_scope = "STARTUP";
    out_receipt->startup_receipt.host_receipt.status = "NEXUS STARTUP";
    if (!nexus_v1_launcher_startup_assets_from_runtime_state(
            state,
            &out_receipt->startup_assets)) {
        return 0;
    }
    out_receipt->boot_log_line =
        out_receipt->startup_assets.title_route_ready
            ? "T0: NEXUS TITLE LOADED"
            : "T0: NEXUS TITLE BLOCKED";
    return 1;
}

static void nexus_v1_launcher_fill_full_start_consumer_status(
    Nexus_V1_StartupFullStartConsumerReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    receipt->m11_ready = receipt->full_start.m11_host_route_ready ? 1 : 0;
    receipt->m12_ready = receipt->full_start.full_start_menu_ready ? 1 : 0;
    receipt->redraw =
        receipt->full_start.host_receipt.input_result ==
        NEXUS_V1_STARTUP_HOST_INPUT_REDRAW;
    receipt->consumer_route = receipt->full_start.m11_host_route
        ? receipt->full_start.m11_host_route
        : "blocked-startup";
    receipt->status_scope = receipt->full_start.host_receipt.status_scope
        ? receipt->full_start.host_receipt.status_scope
        : receipt->full_start.status_scope;
    receipt->status = receipt->full_start.host_receipt.status
        ? receipt->full_start.host_receipt.status
        : receipt->full_start.status;
}

int nexus_v1_launcher_startup_full_start_consumer_from_runtime_state(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupFullStartConsumerReceipt *out_receipt)
{
    Nexus_V1_LauncherRuntimeReceipt derived_runtime;
    const Nexus_V1_LauncherRuntimeReceipt *runtime_source = runtime;
    Nexus_V1_StartupDrawCommand draw_commands[80];

    nexus_v1_launcher_startup_full_start_consumer_receipt_clear(out_receipt);
    if (!runtime_source &&
        nexus_v1_launcher_build_runtime_receipt_from_startup_state(
            state,
            &derived_runtime)) {
        runtime_source = &derived_runtime;
    }
    if (!out_receipt || !runtime_source || !state ||
        !nexus_v1_launcher_startup_full_start_receipt_from_runtime_state(
            runtime_source,
            state,
            &out_receipt->full_start)) {
        return 0;
    }
    nexus_v1_launcher_fill_full_start_consumer_status(out_receipt);
    if (!out_receipt->full_start.full_start_menu_ready) {
        return 1;
    }

    if (state->title_active) {
        out_receipt->title_handoff_valid =
            nexus_v1_launcher_startup_title_handoff_receipt_from_runtime_state(
                state,
                menu_input,
                &out_receipt->title_handoff);
        if (out_receipt->title_handoff_valid) {
            out_receipt->draw_command_count =
                out_receipt->title_handoff.title_draw_ready ? 1 : 0;
        }
    } else if (state->save_select_active) {
        memset(draw_commands, 0, sizeof(draw_commands));
        out_receipt->presentation_valid =
            nexus_v1_launcher_startup_save_presentation_receipt_from_runtime_state(
                state,
                draw_commands,
                (int)(sizeof(draw_commands) / sizeof(draw_commands[0])),
                &out_receipt->presentation);
        if (out_receipt->presentation_valid) {
            out_receipt->draw_command_count =
                out_receipt->presentation.draw_command_count;
        }
        out_receipt->save_route_valid =
            nexus_v1_launcher_startup_save_route_receipt_from_runtime_state(
                state,
                menu_input,
                load_save,
                load_userdata,
                &out_receipt->save_route);
        if (out_receipt->save_route_valid) {
            out_receipt->save_row_count = out_receipt->save_route.row_count;
            out_receipt->selected_row = out_receipt->save_route.selected_row;
        }
    } else if (state->champion_select_active) {
        memset(draw_commands, 0, sizeof(draw_commands));
        out_receipt->presentation_valid =
            nexus_v1_launcher_startup_champion_presentation_receipt_from_runtime_state(
                state,
                draw_commands,
                (int)(sizeof(draw_commands) / sizeof(draw_commands[0])),
                &out_receipt->presentation);
        if (out_receipt->presentation_valid) {
            out_receipt->draw_command_count =
                out_receipt->presentation.draw_command_count;
        }
    }
    return 1;
}

int nexus_v1_launcher_startup_full_start_consumer_from_snapshot(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupFullStartConsumerReceipt *out_receipt)
{
    if (!snapshot) {
        nexus_v1_launcher_startup_full_start_consumer_receipt_clear(
            out_receipt);
        return 0;
    }
    return nexus_v1_launcher_startup_full_start_consumer_from_runtime_state(
        runtime,
        &snapshot->runtime,
        menu_input,
        load_save,
        load_userdata,
        out_receipt);
}

static void nexus_v1_launcher_copy_startup_text(char *dst,
                                                int dst_size,
                                                const char *src)
{
    if (!dst || dst_size <= 0) {
        return;
    }
    if (!src) {
        dst[0] = '\0';
        return;
    }
    snprintf(dst, (size_t)dst_size, "%s", src);
}

static void nexus_v1_launcher_fill_full_start_package_capture(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupFullStartPackageReceipt *receipt)
{
    Nexus_V1_StartupDrawCommand commands[80];
    Nexus_V1_BootFrame boot_frame;
    int command_count = 0;

    if (!state || !receipt) {
        return;
    }
    memset(commands, 0, sizeof(commands));
    memset(&boot_frame, 0, sizeof(boot_frame));
    receipt->boot_warning_frames = nexus_v1_boot_warning_frames();
    receipt->boot_start_ready_frames = nexus_v1_boot_start_ready_frames();
    if (state->title_active &&
        nexus_v1_boot_frame(state->title_frame, NEXUS_FB_H, &boot_frame)) {
        receipt->boot_frame_in_phase = boot_frame.frame_in_phase;
        receipt->warning_visible = boot_frame.warning_visible;
        receipt->title_frames_until_ready =
            boot_frame.warning_visible
                ? receipt->boot_start_ready_frames - state->title_frame
                : boot_frame.title.frames_until_ready;
        receipt->title_hold_frame = boot_frame.title.hold_frame;
        receipt->title_prompt_visible = boot_frame.title.prompt_visible;
        receipt->title_reveal_y0 = boot_frame.title.reveal_y0;
        receipt->title_reveal_y1 = boot_frame.title.reveal_y1;
        receipt->title_reveal_h = boot_frame.title.reveal_h;
    }
    receipt->capture_valid = 1;
    receipt->capture_route = NEXUS_V1_STARTUP_CAPTURE_BLOCKED;
    receipt->first_capture_draw_kind = NEXUS_V1_STARTUP_DRAW_NONE;
    if (state->title_active &&
        receipt->consumer.full_start.title_status_ready) {
        command_count = nexus_v1_startup_presentation_build_title(
            state->title_frame,
            commands,
            (int)(sizeof(commands) / sizeof(commands[0])));
        receipt->title_capture_ready = command_count > 0;
        receipt->title_route_active = 1;
        receipt->capture_route = NEXUS_V1_STARTUP_CAPTURE_TITLE;
    } else if (!receipt->consumer.m11_ready || !receipt->consumer.m12_ready ||
               receipt->fallback_visuals_permitted) {
        receipt->blocked_draw_suppressed = 1;
        return;
    } else if (!receipt->consumer.full_start.full_start_menu_ready) {
        receipt->blocked_draw_suppressed = 1;
        return;
    } else if (state->save_select_active &&
               receipt->consumer.presentation_valid &&
               receipt->consumer.presentation.kind ==
                   NEXUS_V1_STARTUP_MENU_PRESENTATION_SAVE) {
        command_count =
            nexus_v1_launcher_startup_presentation_build_save_from_runtime_state(
                state,
                commands,
                (int)(sizeof(commands) / sizeof(commands[0])));
        receipt->save_capture_ready = command_count > 0;
        receipt->save_route_active = 1;
        receipt->save_capture_frame = receipt->boot_start_ready_frames;
        receipt->capture_route = NEXUS_V1_STARTUP_CAPTURE_SAVE;
    } else if (state->champion_select_active &&
               receipt->consumer.presentation_valid &&
               receipt->consumer.presentation.kind ==
                   NEXUS_V1_STARTUP_MENU_PRESENTATION_CHAMPION) {
        command_count =
            nexus_v1_launcher_startup_presentation_build_champion_from_runtime_state(
                state,
                commands,
                (int)(sizeof(commands) / sizeof(commands[0])));
        receipt->champion_capture_ready = command_count > 0;
        receipt->champion_route_active = 1;
        receipt->champion_capture_frame = receipt->boot_start_ready_frames;
        receipt->capture_route = NEXUS_V1_STARTUP_CAPTURE_CHAMPION;
    } else {
        receipt->menu_idle_active = 1;
        receipt->capture_route = NEXUS_V1_STARTUP_CAPTURE_MENU_IDLE;
    }
    receipt->capture_command_count = command_count;
    receipt->capture_route_ready = command_count > 0;
    if (command_count > 0) {
        receipt->first_capture_draw_kind = commands[0].kind;
        receipt->warning_visible =
            receipt->warning_visible ||
            commands[0].kind == NEXUS_V1_STARTUP_DRAW_WARNING_BACKGROUND;
    }
}

static void nexus_v1_launcher_finalize_full_start_package_saturn_receipt(
    Nexus_V1_StartupFullStartPackageReceipt *receipt)
{
    int title_capture_ready;

    if (!receipt) {
        return;
    }
    receipt->saturn_warning_frame = receipt->warning_capture_frame;
    receipt->saturn_title_capture_frame = receipt->title_capture_frame;
    receipt->saturn_save_capture_frame = receipt->save_capture_frame;
    receipt->saturn_champion_capture_frame = receipt->champion_capture_frame;
    receipt->saturn_dungeon_capture_frame = receipt->dungeon_capture_frame;
    receipt->saturn_title_ready_frame = receipt->boot_start_ready_frames;
    receipt->saturn_gameover_capture_frame = receipt->gameover_capture_frame;
    receipt->saturn_timing_exact =
        nexus_v1_launcher_saturn_timing_exact(
            receipt->boot_warning_frames,
            receipt->boot_start_ready_frames,
            receipt->title_frame_max);
    receipt->saturn_capture_frames_exact =
        nexus_v1_launcher_saturn_full_start_capture_frames_exact(
            receipt->warning_capture_frame,
            receipt->title_capture_frame,
            receipt->save_capture_frame,
            receipt->champion_capture_frame,
            receipt->dungeon_capture_frame,
            receipt->gameover_capture_frame,
            receipt->boot_start_ready_frames);
    receipt->capture_route_expected_consumer_route =
        nexus_v1_launcher_expected_consumer_route_for_capture(
            receipt->capture_route);
    receipt->package_route_matches_capture_route =
        nexus_v1_launcher_consumer_route_matches_capture(
            receipt->capture_route,
            receipt->consumer_route,
            receipt->capture_route_expected_consumer_route);
    title_capture_ready =
        receipt->capture_route == NEXUS_V1_STARTUP_CAPTURE_TITLE &&
        receipt->capture_route_ready &&
        receipt->warning_capture_surface_ready &&
        receipt->title_capture_surface_ready &&
        !receipt->fallback_visuals_permitted &&
        receipt->saturn_timing_exact &&
        receipt->saturn_capture_frames_exact;
    receipt->full_start_package_receipt_ready =
        title_capture_ready ||
        (receipt->m11_ready &&
         receipt->m12_ready &&
         receipt->graphics_ready &&
         receipt->saturn_timing_exact &&
         receipt->saturn_capture_frames_exact &&
         receipt->package_route_matches_capture_route);
    receipt->host_display_caller_expected =
        receipt->full_start_package_receipt_ready &&
        !receipt->fallback_visuals_permitted;
}

int nexus_v1_launcher_startup_full_start_package_from_runtime_state(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupFullStartPackageReceipt *out_receipt)
{
    char phase[32];
    char animation[32];

    nexus_v1_launcher_startup_full_start_package_receipt_clear(out_receipt);
    if (!out_receipt || !state ||
        !nexus_v1_launcher_startup_full_start_consumer_from_runtime_state(
            runtime,
            state,
            menu_input,
            load_save,
            load_userdata,
            &out_receipt->consumer)) {
        return 0;
    }
    memset(phase, 0, sizeof(phase));
    memset(animation, 0, sizeof(animation));
    (void)nexus_v1_launcher_startup_presentation_receipt_from_runtime_state(
        state,
        phase,
        (int)sizeof(phase),
        &out_receipt->startup_active,
        &out_receipt->startup_frame,
        animation,
        (int)sizeof(animation),
        &out_receipt->animation_active,
        &out_receipt->title_frame,
        &out_receipt->title_frame_max,
        &out_receipt->title_ready);
    nexus_v1_launcher_copy_startup_text(out_receipt->phase,
                                        (int)sizeof(out_receipt->phase),
                                        phase);
    nexus_v1_launcher_copy_startup_text(out_receipt->animation,
                                        (int)sizeof(out_receipt->animation),
                                        animation);
    out_receipt->m11_ready = out_receipt->consumer.m11_ready;
    out_receipt->m12_ready = out_receipt->consumer.m12_ready;
    out_receipt->route_ready =
        out_receipt->consumer.full_start.m11_host_route_ready;
    out_receipt->graphics_ready =
        out_receipt->consumer.full_start.full_start_graphics_ready;
    out_receipt->audio_ready =
        out_receipt->consumer.full_start.audio_track02_ready;
    out_receipt->save_menu_ready =
        out_receipt->consumer.full_start.save_status_ready;
    out_receipt->champion_menu_ready =
        out_receipt->consumer.full_start.champion_status_ready;
    out_receipt->fallback_visuals_permitted =
        out_receipt->consumer.full_start.fallback_visuals_permitted;
    out_receipt->warning_surface_loaded =
        out_receipt->consumer.full_start.warning_art_loaded;
    out_receipt->title_surface_loaded =
        out_receipt->consumer.full_start.title_art_loaded;
    out_receipt->gameover_surface_loaded =
        out_receipt->consumer.full_start.gameover_art_loaded;
    out_receipt->warning_capture_surface_ready =
        out_receipt->consumer.full_start.warning_capture_surface_ready;
    out_receipt->title_capture_surface_ready =
        out_receipt->consumer.full_start.title_capture_surface_ready;
    out_receipt->gameover_capture_surface_ready =
        out_receipt->consumer.full_start.gameover_capture_surface_ready;
    out_receipt->warning_capture_frame = 0;
    out_receipt->title_capture_frame = nexus_v1_boot_warning_frames();
    out_receipt->gameover_capture_frame = 0;
    out_receipt->saturn_warning_frame = out_receipt->warning_capture_frame;
    out_receipt->saturn_title_capture_frame =
        out_receipt->title_capture_frame;
    out_receipt->saturn_title_ready_frame =
        out_receipt->boot_start_ready_frames;
    out_receipt->saturn_gameover_capture_frame =
        out_receipt->gameover_capture_frame;
    out_receipt->saturn_timing_exact =
        nexus_v1_launcher_saturn_timing_exact(
            out_receipt->boot_warning_frames,
            out_receipt->boot_start_ready_frames,
            out_receipt->title_frame_max);
    out_receipt->saturn_capture_frames_exact =
        nexus_v1_launcher_saturn_capture_frames_exact(
            out_receipt->warning_capture_frame,
            out_receipt->title_capture_frame,
            out_receipt->gameover_capture_frame);
    out_receipt->full_start_package_receipt_ready =
        out_receipt->m11_ready &&
        out_receipt->m12_ready &&
        out_receipt->graphics_ready &&
        out_receipt->saturn_timing_exact &&
        out_receipt->saturn_capture_frames_exact;
    out_receipt->host_display_caller_expected =
        out_receipt->full_start_package_receipt_ready &&
        !out_receipt->fallback_visuals_permitted;
    out_receipt->consumer_route = out_receipt->consumer.consumer_route;
    out_receipt->asset_route = out_receipt->consumer.full_start.asset_route;
    out_receipt->startup_ui_blocker =
        out_receipt->consumer.full_start.startup_ui_blocker;
    out_receipt->status_scope = out_receipt->consumer.status_scope;
    out_receipt->status = out_receipt->consumer.status;
    nexus_v1_launcher_fill_full_start_package_capture(state, out_receipt);
    nexus_v1_launcher_finalize_full_start_package_saturn_receipt(out_receipt);
    return 1;
}

int nexus_v1_launcher_startup_full_start_package_from_snapshot(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupFullStartPackageReceipt *out_receipt)
{
    if (!snapshot) {
        nexus_v1_launcher_startup_full_start_package_receipt_clear(
            out_receipt);
        return 0;
    }
    return nexus_v1_launcher_startup_full_start_package_from_runtime_state(
        runtime,
        &snapshot->runtime,
        menu_input,
        load_save,
        load_userdata,
        out_receipt);
}

int nexus_v1_launcher_startup_full_start_package_export_presentation(
    const Nexus_V1_StartupFullStartPackageReceipt *package,
    char *out_phase,
    int out_phase_size,
    int *out_startup_active,
    int *out_startup_frame,
    char *out_animation,
    int out_animation_size,
    int *out_animation_active,
    int *out_title_frame,
    int *out_title_frame_max,
    int *out_title_ready)
{
    if (!package) {
        return 0;
    }
    nexus_v1_launcher_copy_startup_text(out_phase,
                                        out_phase_size,
                                        package->phase);
    nexus_v1_launcher_copy_startup_text(out_animation,
                                        out_animation_size,
                                        package->animation);
    if (out_startup_active) {
        *out_startup_active = package->startup_active;
    }
    if (out_startup_frame) {
        *out_startup_frame = package->startup_frame;
    }
    if (out_animation_active) {
        *out_animation_active = package->animation_active;
    }
    if (out_title_frame) {
        *out_title_frame = package->title_frame;
    }
    if (out_title_frame_max) {
        *out_title_frame_max = package->title_frame_max;
    }
    if (out_title_ready) {
        *out_title_ready = package->title_ready;
    }
    return 1;
}

static int nexus_v1_launcher_build_full_start_package_commands(
    const Nexus_V1_StartupRuntimeState *state,
    const Nexus_V1_StartupFullStartPackageReceipt *package,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands)
{
    if (out_commands && max_commands > 0) {
        memset(out_commands,
               0,
               (size_t)max_commands * sizeof(out_commands[0]));
    }
    if (!state || !package || !out_commands || max_commands <= 0 ||
        !package->capture_route_ready ||
        !package->full_start_package_receipt_ready ||
        !package->host_display_caller_expected ||
        package->fallback_visuals_permitted) {
        return 0;
    }
    switch (package->capture_route) {
    case NEXUS_V1_STARTUP_CAPTURE_TITLE:
        return nexus_v1_startup_presentation_build_title(state->title_frame,
                                                         out_commands,
                                                         max_commands);
    case NEXUS_V1_STARTUP_CAPTURE_SAVE:
        return nexus_v1_launcher_startup_presentation_build_save_from_runtime_state(
            state,
            out_commands,
            max_commands);
    case NEXUS_V1_STARTUP_CAPTURE_CHAMPION:
        return nexus_v1_launcher_startup_presentation_build_champion_from_runtime_state(
            state,
            out_commands,
            max_commands);
    case NEXUS_V1_STARTUP_CAPTURE_BLOCKED:
    case NEXUS_V1_STARTUP_CAPTURE_MENU_IDLE:
    case NEXUS_V1_STARTUP_CAPTURE_INVALID:
    default:
        return 0;
    }
}

int nexus_v1_launcher_startup_full_start_package_build_commands_from_runtime_state(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupFullStartPackageReceipt *out_receipt)
{
    int command_count;

    if (!nexus_v1_launcher_startup_full_start_package_from_runtime_state(
            runtime,
            state,
            menu_input,
            load_save,
            load_userdata,
            out_receipt)) {
        if (out_commands && max_commands > 0) {
            memset(out_commands,
                   0,
                   (size_t)max_commands * sizeof(out_commands[0]));
        }
        return 0;
    }
    command_count = nexus_v1_launcher_build_full_start_package_commands(
        state,
        out_receipt,
        out_commands,
        max_commands);
    if (out_receipt) {
        out_receipt->capture_command_count = command_count;
        out_receipt->capture_route_ready = command_count > 0;
        out_receipt->first_capture_draw_kind =
            command_count > 0 && out_commands
                ? out_commands[0].kind
                : NEXUS_V1_STARTUP_DRAW_NONE;
    }
    return 1;
}

int nexus_v1_launcher_startup_full_start_package_build_commands_from_snapshot(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupFullStartPackageReceipt *out_receipt)
{
    if (!snapshot) {
        nexus_v1_launcher_startup_full_start_package_receipt_clear(
            out_receipt);
        if (out_commands && max_commands > 0) {
            memset(out_commands,
                   0,
                   (size_t)max_commands * sizeof(out_commands[0]));
        }
        return 0;
    }
    return nexus_v1_launcher_startup_full_start_package_build_commands_from_runtime_state(
        runtime,
        &snapshot->runtime,
        menu_input,
        load_save,
        load_userdata,
        out_commands,
        max_commands,
        out_receipt);
}

static void nexus_v1_launcher_fill_startup_bundle(
    Nexus_V1_StartupReceiptBundle *receipt,
    const Nexus_V1_StartupFullStartPackageReceipt *package,
    int command_count,
    int max_commands)
{
    if (!receipt || !package) {
        return;
    }
    receipt->package = *package;
    (void)nexus_v1_launcher_m12_startup_package_from_full_start_package(
        package,
        &receipt->m12_package);
    receipt->capture_route = package->capture_route;
    receipt->first_draw_kind = package->first_capture_draw_kind;
    receipt->command_count = command_count;
    receipt->max_commands = max_commands;
    receipt->copied_command_count =
        command_count > 0 && max_commands > 0
            ? (command_count < max_commands ? command_count : max_commands)
            : 0;
    receipt->timing_frame = package->title_frame;
    receipt->timing_frame_max = package->title_frame_max;
    receipt->timing_ready = package->title_ready;
    receipt->saturn_timing_exact = package->saturn_timing_exact;
    receipt->saturn_capture_frames_exact =
        package->saturn_capture_frames_exact;
    receipt->active_capture_frame =
        nexus_v1_launcher_active_capture_frame(package);
    receipt->saturn_active_capture_frame =
        nexus_v1_launcher_saturn_active_capture_frame(package);
    receipt->warning_visible = package->warning_visible;
    receipt->prompt_visible = package->title_prompt_visible;
    receipt->m11_ready = package->m11_ready;
    receipt->m12_ready = package->m12_ready;
    receipt->capture_ready = package->capture_route_ready;
    receipt->display_ready =
        receipt->m12_package.packaged_capture_ready ? 1 : 0;
    receipt->blocked =
        receipt->display_ready && !package->blocked_draw_suppressed ? 0 : 1;
    receipt->fallback_visuals_permitted =
        package->fallback_visuals_permitted;
    receipt->route_label = receipt->m12_package.capture_route_label;
    receipt->first_draw_label = receipt->m12_package.first_capture_draw_label;
    receipt->status_scope = package->status_scope;
    receipt->status = package->status;
}

int nexus_v1_launcher_startup_receipt_bundle_from_runtime_state(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupReceiptBundle *out_receipt)
{
    Nexus_V1_StartupFullStartPackageReceipt package;
    int copied_count;

    nexus_v1_launcher_startup_receipt_bundle_clear(out_receipt);
    memset(&package, 0, sizeof(package));
    if (!out_receipt ||
        !nexus_v1_launcher_startup_full_start_package_from_runtime_state(
            runtime,
            state,
            menu_input,
            load_save,
            load_userdata,
            &package)) {
        if (out_commands && max_commands > 0) {
            memset(out_commands,
                   0,
                   (size_t)max_commands * sizeof(out_commands[0]));
        }
        return 0;
    }
    copied_count = nexus_v1_launcher_build_full_start_package_commands(
        state,
        &package,
        out_commands,
        max_commands);
    nexus_v1_launcher_fill_startup_bundle(out_receipt,
                                          &package,
                                          package.capture_command_count,
                                          max_commands);
    out_receipt->copied_command_count = copied_count;
    return 1;
}

int nexus_v1_launcher_startup_receipt_bundle_from_snapshot(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupReceiptBundle *out_receipt)
{
    if (!snapshot) {
        nexus_v1_launcher_startup_receipt_bundle_clear(out_receipt);
        if (out_commands && max_commands > 0) {
            memset(out_commands,
                   0,
                   (size_t)max_commands * sizeof(out_commands[0]));
        }
        return 0;
    }
    return nexus_v1_launcher_startup_receipt_bundle_from_runtime_state(
        runtime,
        &snapshot->runtime,
        menu_input,
        load_save,
        load_userdata,
        out_commands,
        max_commands,
        out_receipt);
}

static Nexus_V1_StartupRealAssetOwnershipRoute
nexus_v1_launcher_expected_ownership_route_for_capture(
    Nexus_V1_StartupCaptureRoute route,
    int runtime_dgn_handoff_ready)
{
    switch (route) {
    case NEXUS_V1_STARTUP_CAPTURE_TITLE:
        return NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_TITLE_CAPTURE;
    case NEXUS_V1_STARTUP_CAPTURE_SAVE:
        return NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_MENU_CAPTURE;
    case NEXUS_V1_STARTUP_CAPTURE_CHAMPION:
        return runtime_dgn_handoff_ready
            ? NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_RUNTIME_HANDOFF
            : NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_MENU_CAPTURE;
    case NEXUS_V1_STARTUP_CAPTURE_BLOCKED:
        return NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_BLOCKED_ASSETS;
    case NEXUS_V1_STARTUP_CAPTURE_MENU_IDLE:
    case NEXUS_V1_STARTUP_CAPTURE_INVALID:
    default:
        return NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_INVALID;
    }
}

static void nexus_v1_launcher_fill_host_ownership_route_contract(
    Nexus_V1_StartupRealAssetOwnershipReceipt *receipt)
{
    Nexus_V1_StartupRealAssetOwnershipRoute expected_route;
    int non_title_capture_route_complete;
    int dungeon_route_complete;
    if (!receipt) {
        return;
    }
    expected_route = nexus_v1_launcher_expected_ownership_route_for_capture(
        receipt->capture_route,
        receipt->runtime_dgn_handoff_ready);
    receipt->host_ownership_route_matches_capture_route =
        expected_route != NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_INVALID &&
        receipt->route == expected_route;
    if (!receipt->host_ownership_route_matches_capture_route &&
        receipt->runtime_dgn_handoff_ready &&
        receipt->route ==
            NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_RUNTIME_HANDOFF) {
        receipt->host_ownership_route_matches_capture_route = 1;
    }
    receipt->package_route_consumes_host_ownership =
        receipt->host_route_consumes_package_route &&
        receipt->host_ownership_route_matches_capture_route;
    receipt->dgn_route_consumes_host_ownership =
        receipt->dgn_route_consumes_startup_package &&
        receipt->host_ownership_route_matches_capture_route &&
        receipt->route ==
            NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_RUNTIME_HANDOFF;
    receipt->startup_route_saturn_capture_exact =
        nexus_v1_launcher_startup_base_saturn_capture_exact(
            &receipt->startup_bundle.package);
    receipt->startup_route_consumes_package_capture =
        receipt->full_start_package_consumed &&
        receipt->package_capture_consumed_by_host &&
        receipt->host_route_consumes_package_route &&
        receipt->startup_route_saturn_capture_exact;
    receipt->title_route_consumes_package_capture =
        receipt->capture_route == NEXUS_V1_STARTUP_CAPTURE_TITLE &&
        receipt->package_route_consumes_host_ownership &&
        receipt->host_route_consumes_active_capture_frame &&
        receipt->title_capture_uses_real_assets &&
        receipt->saturn_active_capture_frame ==
            receipt->startup_bundle.package.saturn_title_capture_frame &&
        receipt->startup_bundle.package.saturn_title_capture_frame ==
            nexus_v1_boot_warning_frames();
    receipt->save_route_consumes_package_capture =
        receipt->capture_route == NEXUS_V1_STARTUP_CAPTURE_SAVE &&
        receipt->package_route_consumes_host_ownership &&
        receipt->host_route_consumes_active_capture_frame &&
        receipt->saturn_save_capture_frame ==
            receipt->startup_bundle.package.boot_start_ready_frames &&
        (receipt->host_saturn_non_title_capture_mask & 1u);
    receipt->champion_route_consumes_package_capture =
        receipt->capture_route == NEXUS_V1_STARTUP_CAPTURE_CHAMPION &&
        receipt->package_route_consumes_host_ownership &&
        receipt->host_route_consumes_active_capture_frame &&
        receipt->saturn_champion_capture_frame ==
            receipt->startup_bundle.package.boot_start_ready_frames &&
        (receipt->host_saturn_non_title_capture_mask & 2u);
    receipt->dungeon_route_consumes_package_capture =
        (receipt->capture_route == NEXUS_V1_STARTUP_CAPTURE_CHAMPION ||
         receipt->route ==
             NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_RUNTIME_HANDOFF) &&
        receipt->runtime_dgn_handoff_ready &&
        receipt->dgn_route_consumes_host_ownership &&
        receipt->dgn_route_saturn_capture_exact &&
        receipt->host_route_consumes_dungeon_capture_frame &&
        receipt->saturn_dungeon_capture_frame ==
            receipt->startup_bundle.package.boot_start_ready_frames &&
        (receipt->host_saturn_non_title_capture_mask & 4u);
    receipt->title_route_saturn_capture_exact =
        receipt->title_route_consumes_package_capture &&
        receipt->startup_bundle.package.saturn_title_capture_frame ==
            nexus_v1_boot_warning_frames();
    receipt->save_route_saturn_capture_exact =
        receipt->save_route_consumes_package_capture &&
        receipt->saturn_save_capture_frame ==
            receipt->startup_bundle.package.boot_start_ready_frames;
    receipt->champion_route_saturn_capture_exact =
        receipt->champion_route_consumes_package_capture &&
        receipt->saturn_champion_capture_frame ==
            receipt->startup_bundle.package.boot_start_ready_frames;
    receipt->dungeon_route_saturn_capture_exact =
        receipt->dungeon_route_consumes_package_capture &&
        receipt->saturn_dungeon_capture_frame ==
            receipt->startup_bundle.package.boot_start_ready_frames;
    receipt->startup_host_package_route_complete =
        receipt->startup_route_consumes_package_capture &&
        receipt->startup_route_saturn_capture_exact &&
        receipt->startup_bundle.package.full_start_package_receipt_ready &&
        receipt->startup_bundle.package.host_display_caller_expected;
    receipt->title_host_package_route_complete =
        receipt->title_route_consumes_package_capture &&
        receipt->title_route_saturn_capture_exact &&
        strcmp(receipt->startup_bundle.package.consumer_route,
               "title-warning") == 0 &&
        strcmp(receipt->startup_bundle.package
                   .capture_route_expected_consumer_route,
               "title-warning") == 0 &&
        receipt->route ==
            NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_TITLE_CAPTURE;
    receipt->save_host_package_route_complete =
        receipt->save_route_consumes_package_capture &&
        receipt->save_route_saturn_capture_exact &&
        strcmp(receipt->startup_bundle.package.consumer_route,
               "save-menu") == 0 &&
        strcmp(receipt->startup_bundle.package
                   .capture_route_expected_consumer_route,
               "save-menu") == 0 &&
        receipt->route == NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_MENU_CAPTURE;
    receipt->champion_host_package_route_complete =
        receipt->champion_route_consumes_package_capture &&
        receipt->champion_route_saturn_capture_exact &&
        strcmp(receipt->startup_bundle.package.consumer_route,
               "champion-menu") == 0 &&
        strcmp(receipt->startup_bundle.package
                   .capture_route_expected_consumer_route,
               "champion-menu") == 0 &&
        (receipt->route ==
             NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_MENU_CAPTURE ||
         receipt->route ==
             NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_RUNTIME_HANDOFF);
    receipt->dungeon_host_package_route_complete =
        receipt->dungeon_route_consumes_package_capture &&
        receipt->dungeon_route_saturn_capture_exact &&
        receipt->dgn_route_consumes_startup_package &&
        receipt->dgn_route_consumes_host_ownership &&
        receipt->runtime_dgn_route_joined &&
        receipt->route ==
            NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_RUNTIME_HANDOFF;
    receipt->dungeon_capture_route_consumed =
        receipt->dungeon_host_package_route_complete &&
        receipt->runtime_dgn_handoff_ready &&
        receipt->host_route_consumes_dungeon_capture_frame &&
        receipt->dgn_draw_command_count > 0;
    receipt->host_package_route_complete_mask = 0u;
    if (receipt->save_host_package_route_complete) {
        receipt->host_package_route_complete_mask |= 1u;
    }
    if (receipt->champion_host_package_route_complete) {
        receipt->host_package_route_complete_mask |= 2u;
    }
    if (receipt->dungeon_host_package_route_complete) {
        receipt->host_package_route_complete_mask |= 4u;
    }
    receipt->host_package_route_expected_mask =
        nexus_v1_launcher_expected_non_title_capture_mask(
            receipt->capture_route,
            receipt->runtime_dgn_handoff_ready);
    receipt->host_package_route_matrix_complete =
        receipt->host_package_route_expected_mask != 0u &&
        receipt->host_package_route_complete_mask ==
            receipt->host_package_route_expected_mask;
    receipt->host_saturn_exact_capture_mask = 0u;
    if (receipt->save_route_saturn_capture_exact) {
        receipt->host_saturn_exact_capture_mask |= 1u;
    }
    if (receipt->champion_route_saturn_capture_exact) {
        receipt->host_saturn_exact_capture_mask |= 2u;
    }
    if (receipt->dungeon_route_saturn_capture_exact) {
        receipt->host_saturn_exact_capture_mask |= 4u;
    }
    receipt->host_saturn_route_timing_matrix_complete =
        receipt->host_package_route_expected_mask != 0u &&
        receipt->host_saturn_exact_capture_mask ==
            receipt->host_package_route_expected_mask;
    receipt->host_package_route_timing_matrix_complete =
        receipt->host_package_route_matrix_complete &&
        receipt->host_saturn_route_timing_matrix_complete &&
        receipt->host_route_capture_matrix_ready &&
        receipt->host_route_capture_matrix_exact;
    receipt->host_all_route_complete_mask = 0u;
    if (receipt->startup_host_package_route_complete) {
        receipt->host_all_route_complete_mask |=
            NEXUS_V1_HOST_ROUTE_STARTUP_BIT;
    }
    if (receipt->title_host_package_route_complete) {
        receipt->host_all_route_complete_mask |=
            NEXUS_V1_HOST_ROUTE_TITLE_BIT;
    }
    if (receipt->save_host_package_route_complete) {
        receipt->host_all_route_complete_mask |=
            NEXUS_V1_HOST_ROUTE_SAVE_BIT;
    }
    if (receipt->champion_host_package_route_complete) {
        receipt->host_all_route_complete_mask |=
            NEXUS_V1_HOST_ROUTE_CHAMPION_BIT;
    }
    if (receipt->dungeon_host_package_route_complete) {
        receipt->host_all_route_complete_mask |=
            NEXUS_V1_HOST_ROUTE_DUNGEON_BIT;
    }
    receipt->host_all_route_expected_mask =
        nexus_v1_launcher_expected_all_route_mask(
            receipt->capture_route,
            receipt->runtime_dgn_handoff_ready);
    receipt->host_all_route_matrix_complete =
        receipt->host_all_route_expected_mask != 0u &&
        receipt->host_all_route_complete_mask ==
            receipt->host_all_route_expected_mask;
    receipt->host_saturn_all_exact_capture_mask = 0u;
    if (receipt->startup_route_saturn_capture_exact) {
        receipt->host_saturn_all_exact_capture_mask |=
            NEXUS_V1_HOST_ROUTE_STARTUP_BIT;
    }
    if (receipt->title_route_saturn_capture_exact) {
        receipt->host_saturn_all_exact_capture_mask |=
            NEXUS_V1_HOST_ROUTE_TITLE_BIT;
    }
    if (receipt->save_route_saturn_capture_exact) {
        receipt->host_saturn_all_exact_capture_mask |=
            NEXUS_V1_HOST_ROUTE_SAVE_BIT;
    }
    if (receipt->champion_route_saturn_capture_exact) {
        receipt->host_saturn_all_exact_capture_mask |=
            NEXUS_V1_HOST_ROUTE_CHAMPION_BIT;
    }
    if (receipt->dungeon_route_saturn_capture_exact) {
        receipt->host_saturn_all_exact_capture_mask |=
            NEXUS_V1_HOST_ROUTE_DUNGEON_BIT;
    }
    receipt->host_saturn_all_expected_capture_mask =
        receipt->host_all_route_expected_mask;
    receipt->host_saturn_all_route_timing_matrix_complete =
        receipt->host_saturn_all_expected_capture_mask != 0u &&
        receipt->host_saturn_all_exact_capture_mask ==
            receipt->host_saturn_all_expected_capture_mask;
    receipt->host_all_route_timing_matrix_complete =
        receipt->host_all_route_matrix_complete &&
        receipt->host_saturn_all_route_timing_matrix_complete &&
        receipt->startup_route_saturn_capture_exact &&
        receipt->host_route_capture_matrix_ready &&
        receipt->host_route_capture_matrix_exact;
    non_title_capture_route_complete =
        receipt->no_fallback_visuals_enforced &&
        !receipt->fallback_visuals_permitted &&
        !receipt->blocked_draw_suppressed &&
        receipt->package_route_matches_capture_route &&
        receipt->host_route_consumes_package_route &&
        receipt->host_ownership_route_matches_capture_route &&
        receipt->package_route_consumes_host_ownership &&
        receipt->host_route_consumes_capture_matrix &&
        receipt->host_route_capture_matrix_ready &&
        receipt->host_route_capture_matrix_exact &&
        receipt->saturn_timing_exact &&
        receipt->saturn_capture_frames_exact &&
        receipt->host_saturn_expected_capture_mask != 0u &&
        receipt->host_saturn_non_title_capture_mask ==
            receipt->host_saturn_expected_capture_mask &&
        receipt->host_package_route_timing_matrix_complete &&
        ((receipt->capture_route == NEXUS_V1_STARTUP_CAPTURE_SAVE &&
          receipt->save_host_package_route_complete) ||
         (receipt->capture_route == NEXUS_V1_STARTUP_CAPTURE_CHAMPION &&
          receipt->champion_host_package_route_complete &&
          (!receipt->runtime_dgn_handoff_ready ||
           receipt->dungeon_host_package_route_complete)));
    dungeon_route_complete =
        non_title_capture_route_complete &&
        receipt->capture_route == NEXUS_V1_STARTUP_CAPTURE_CHAMPION &&
        receipt->runtime_dgn_handoff_ready &&
        receipt->runtime_dgn_route_joined &&
        receipt->dgn_route_consumes_startup_package &&
        receipt->dgn_route_consumes_host_ownership &&
        receipt->champion_host_package_route_complete &&
        receipt->dungeon_host_package_route_complete &&
        receipt->dgn_route_saturn_capture_exact &&
        receipt->host_route_consumes_dungeon_capture_frame &&
        receipt->dgn_draw_command_count > 0;
    receipt->non_title_saturn_capture_route_complete =
        non_title_capture_route_complete ? 1 : 0;
    receipt->dungeon_startup_route_consumption_complete =
        dungeon_route_complete ? 1 : 0;
    receipt->startup_route_consumption_complete =
        non_title_capture_route_complete &&
        (receipt->capture_route == NEXUS_V1_STARTUP_CAPTURE_SAVE ||
         !receipt->runtime_dgn_handoff_ready ||
         dungeon_route_complete);
}

static void nexus_v1_launcher_fill_real_asset_ownership(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupRealAssetOwnershipReceipt *receipt)
{
    const Nexus_V1_StartupFullStartPackageReceipt *package;
    const Nexus_V1_StartupAssetHandoffReceipt *asset_handoff;
    const Nexus_V1_StartupRuntimeRouteReceipt *runtime_route;
    Nexus_V1_DgnViewportHostRouteReceipt viewport_host_route;

    if (!receipt) {
        return;
    }
    package = &receipt->startup_bundle.package;
    asset_handoff = &package->consumer.full_start.asset_handoff;
    runtime_route = &receipt->runtime_route;

    receipt->asset_handoff = *asset_handoff;
    receipt->menu_bpk_handoff = asset_handoff->menu_bpk_renderer_handoff;
    receipt->receipt_owner = "nexus-v1-launcher";
    receipt->receipt_owner_is_nexus = 1;
    receipt->title_menu_receipt_owned = 1;
    receipt->capture_receipt_owned = package->capture_valid ? 1 : 0;
    receipt->real_asset_receipt_owned = 1;
    receipt->consumes_bpk_menu_handoff =
        asset_handoff->menu_bpk_renderer_handoff_valid ? 1 : 0;
    receipt->consumes_prs3_blocker =
        asset_handoff->menu_bpk_prs3_blocks_real_menu_route ? 1 : 0;
    receipt->capture_ready = receipt->startup_bundle.capture_ready ? 1 : 0;
    receipt->display_ready = receipt->startup_bundle.display_ready ? 1 : 0;
    receipt->startup_draw_command_count =
        receipt->startup_bundle.command_count;
    receipt->capture_route = receipt->startup_bundle.capture_route;
    receipt->first_startup_draw_kind = receipt->startup_bundle.first_draw_kind;
    receipt->fallback_visuals_permitted =
        package->fallback_visuals_permitted ||
        asset_handoff->fallback_visuals_permitted ||
        runtime_route->fallback_visuals_permitted;
    receipt->blocked_draw_suppressed = package->blocked_draw_suppressed;
    receipt->asset_route = package->asset_route
        ? package->asset_route
        : asset_handoff->menu_asset_route;
    receipt->asset_blocker = package->startup_ui_blocker
        ? package->startup_ui_blocker
        : package->consumer.full_start.startup_ui_blocker;
    receipt->status_scope = package->status_scope;
    receipt->status = package->status;

    if (runtime_route->runtime_handoff.dgn_handoff.status !=
        NEXUS_V1_DGN_RENDERER_HANDOFF_MISSING) {
        receipt->dgn_handoff = runtime_route->runtime_handoff.dgn_handoff;
        receipt->dgn_render_plan =
            runtime_route->runtime_handoff.render_plan;
        receipt->static_material_sources =
            runtime_route->runtime_handoff.static_material_sources;
        receipt->consumes_dgn_handoff = 1;
        receipt->runtime_dgn_handoff_ready =
            runtime_route->runtime_route_ready &&
            runtime_route->dgn_render_plan_ready &&
            runtime_route->dgn_viewport_render_ready &&
            !runtime_route->dgn_blocks_real_mesh_render;
        receipt->runtime_dgn_viewport_render_ready =
            runtime_route->dgn_viewport_render_ready ? 1 : 0;
        receipt->dgn_draw_command_count =
            runtime_route->dgn_render_command_count;
        receipt->dgn_viewport_rasterized_command_count =
            runtime_route->dgn_viewport_rasterized_command_count;
        receipt->dgn_viewport_material_surface_count =
            runtime_route->dgn_viewport_material_surface_count;
        receipt->dgn_viewport_floor_material_surface_count =
            runtime_route->dgn_viewport_floor_material_surface_count;
        receipt->dgn_viewport_ceiling_material_surface_count =
            runtime_route->dgn_viewport_ceiling_material_surface_count;
        receipt->dgn_viewport_wall_material_surface_count =
            runtime_route->dgn_viewport_wall_material_surface_count;
        receipt->dgn_viewport_host_route_status =
            runtime_route->dgn_viewport_host_route_status;
        receipt->dgn_viewport_host_route_ready =
            runtime_route->dgn_viewport_host_route_ready ? 1 : 0;
        receipt->dgn_viewport_host_route_consumed =
            runtime_route->dgn_viewport_host_route_consumed ? 1 : 0;
        receipt->dgn_viewport_host_route_package_consumed =
            runtime_route->dgn_viewport_host_route_package_consumed ? 1 : 0;
        receipt->dgn_viewport_host_route_blocks_runtime =
            runtime_route->dgn_viewport_host_route_blocks_runtime ? 1 : 0;
        receipt->dgn_viewport_capture_ready =
            runtime_route->dgn_viewport_capture_ready ? 1 : 0;
        receipt->dgn_viewport_frame_hash =
            runtime_route->dgn_viewport_frame_hash;
        receipt->bpk_material_surface_count =
            runtime_route->bpk_material_surface_count;
        receipt->bpk_truecolor_material_surface_count =
            runtime_route->bpk_truecolor_material_surface_count;
        receipt->bpk_prs3_material_surface_count =
            runtime_route->bpk_prs3_material_surface_count;
        receipt->runtime_dgn_material_path_consumed =
            runtime_route->dgn_material_plan_consumed &&
            runtime_route->dgn_commands_copied_from_material_plan &&
            runtime_route->dgn_material_viewport_consumed &&
            runtime_route->dgn_viewport_capture_ready &&
            runtime_route->dgn_viewport_frame_hash != 0u &&
            !runtime_route->fallback_visuals_permitted;
        receipt->dgn_static_material_source_consumed =
            runtime_route->dgn_static_material_source_consumed ? 1 : 0;
        receipt->dgn_viewport_written_pixels =
            runtime_route->dgn_viewport_written_pixels;
        receipt->first_dgn_draw_kind =
            runtime_route->first_dgn_render_command_kind;
        if (receipt->runtime_dgn_handoff_ready) {
            receipt->startup_bundle.package.dungeon_capture_frame =
                receipt->startup_bundle.package.boot_start_ready_frames;
            receipt->startup_bundle.package.saturn_dungeon_capture_frame =
                receipt->startup_bundle.package.dungeon_capture_frame;
            receipt->saturn_dungeon_capture_frame =
                receipt->startup_bundle.package.saturn_dungeon_capture_frame;
        }
    }
    (void)nexus_v1_launcher_dgn_viewport_host_route_receipt(
        state ? state->engine : NULL,
        &viewport_host_route);
    if (viewport_host_route.status != NEXUS_V1_DGN_HOST_ROUTE_MISSING &&
        nexus_v1_launcher_has_real_dgn_admission(state ? state->engine : NULL)) {
        receipt->dgn_viewport_host_route_status =
            (int)viewport_host_route.status;
        receipt->dgn_viewport_host_route_ready =
            viewport_host_route.can_present_runtime_dgn ? 1 : 0;
        receipt->dgn_viewport_host_route_consumed =
            viewport_host_route.host_route_consumed ? 1 : 0;
        receipt->dgn_viewport_host_route_package_consumed =
            viewport_host_route.package_consumed ? 1 : 0;
        receipt->dgn_viewport_host_route_blocks_runtime =
            viewport_host_route.blocks_runtime_dgn ? 1 : 0;
        receipt->dgn_viewport_capture_ready =
            viewport_host_route.captured_frame_ready ? 1 : 0;
        receipt->dgn_viewport_frame_hash =
            viewport_host_route.frame_hash;
    }
    if (!nexus_v1_launcher_has_real_dgn_admission(state ? state->engine : NULL)) {
        receipt->runtime_dgn_handoff_ready = 0;
        receipt->runtime_dgn_viewport_render_ready = 0;
        receipt->dgn_draw_command_count = 0;
        receipt->dgn_viewport_rasterized_command_count = 0;
        receipt->dgn_viewport_material_surface_count = 0;
        receipt->dgn_viewport_floor_material_surface_count = 0;
        receipt->dgn_viewport_ceiling_material_surface_count = 0;
        receipt->dgn_viewport_wall_material_surface_count = 0;
        receipt->dgn_viewport_host_route_ready = 0;
        receipt->dgn_viewport_host_route_consumed = 0;
        receipt->dgn_viewport_host_route_package_consumed = 0;
        receipt->dgn_viewport_host_route_blocks_runtime = 1;
        receipt->dgn_viewport_capture_ready = 0;
        receipt->dgn_viewport_frame_hash = 0u;
        receipt->runtime_dgn_material_path_consumed = 0;
        receipt->dgn_viewport_written_pixels = 0;
        receipt->first_dgn_draw_kind = 0;
        receipt->copied_dgn_command_count = 0;
    }

    receipt->title_capture_uses_real_assets =
        package->capture_valid &&
        package->title_capture_surface_ready &&
        package->warning_capture_surface_ready &&
        !package->fallback_visuals_permitted;
    receipt->menu_capture_uses_real_assets =
        package->capture_valid &&
        (package->save_capture_ready || package->champion_capture_ready) &&
        package->consumer.full_start.menu_bpk_route_ready &&
        asset_handoff->real_menu_asset_handoff_ready &&
        !package->fallback_visuals_permitted;
    receipt->full_start_package_consumed =
        package->full_start_package_receipt_ready;
    receipt->package_capture_consumed_by_host =
        package->full_start_package_receipt_ready &&
        package->host_display_caller_expected &&
        receipt->capture_receipt_owned;
    receipt->package_route_matches_capture_route =
        package->package_route_matches_capture_route;
    receipt->host_route_consumes_package_route =
        receipt->package_capture_consumed_by_host &&
        receipt->package_route_matches_capture_route &&
        nexus_v1_launcher_consumer_route_matches_capture(
            package->capture_route,
            package->consumer_route,
            package->capture_route_expected_consumer_route);
    receipt->title_menu_capture_route_joined =
        receipt->package_capture_consumed_by_host &&
        receipt->title_capture_uses_real_assets &&
        (receipt->menu_capture_uses_real_assets ||
         package->capture_route == NEXUS_V1_STARTUP_CAPTURE_TITLE);
    receipt->bpk_menu_route_joined =
        receipt->consumes_bpk_menu_handoff &&
        asset_handoff->real_menu_asset_handoff_ready &&
        !asset_handoff->menu_bpk_prs3_blocks_real_menu_route;
    receipt->runtime_dgn_route_joined =
        (receipt->title_menu_capture_route_joined ||
         receipt->full_start_package_consumed) &&
        receipt->bpk_menu_route_joined &&
        receipt->runtime_dgn_handoff_ready &&
        receipt->consumes_dgn_handoff;
    receipt->first_host_draw_uses_package =
        receipt->startup_bundle.copied_command_count > 0 &&
        receipt->startup_bundle.first_draw_kind !=
            NEXUS_V1_STARTUP_DRAW_NONE &&
        receipt->package_capture_consumed_by_host;
    receipt->saturn_timing_exact = package->saturn_timing_exact;
    receipt->saturn_capture_frames_exact =
        package->saturn_capture_frames_exact;
    receipt->active_capture_frame =
        receipt->startup_bundle.active_capture_frame;
    receipt->saturn_active_capture_frame =
        receipt->startup_bundle.saturn_active_capture_frame;
    receipt->host_route_consumes_active_capture_frame =
        receipt->package_capture_consumed_by_host &&
        receipt->first_host_draw_uses_package &&
        receipt->capture_route != NEXUS_V1_STARTUP_CAPTURE_BLOCKED &&
        receipt->capture_route != NEXUS_V1_STARTUP_CAPTURE_MENU_IDLE &&
        receipt->active_capture_frame >= 0 &&
        receipt->saturn_active_capture_frame == receipt->active_capture_frame;
    receipt->saturn_save_capture_frame = package->saturn_save_capture_frame;
    receipt->saturn_champion_capture_frame =
        package->saturn_champion_capture_frame;
    if (!receipt->runtime_dgn_handoff_ready) {
        receipt->saturn_dungeon_capture_frame =
            package->saturn_dungeon_capture_frame;
    }
    receipt->no_fallback_visuals_enforced =
        receipt->receipt_owner_is_nexus &&
        !receipt->fallback_visuals_permitted;
    receipt->host_route_consumes_dungeon_capture_frame =
        receipt->runtime_dgn_handoff_ready &&
        receipt->runtime_dgn_route_joined &&
        receipt->saturn_dungeon_capture_frame ==
            package->boot_start_ready_frames;
    receipt->host_saturn_non_title_capture_mask =
        nexus_v1_launcher_saturn_non_title_capture_mask(
            receipt->saturn_save_capture_frame,
            receipt->saturn_champion_capture_frame,
            receipt->saturn_dungeon_capture_frame);
    receipt->host_saturn_expected_capture_mask =
        nexus_v1_launcher_expected_non_title_capture_mask(
            receipt->capture_route,
            receipt->runtime_dgn_handoff_ready);
    receipt->host_saturn_non_title_capture_count =
        nexus_v1_launcher_capture_mask_count(
            receipt->host_saturn_non_title_capture_mask);
    receipt->host_route_capture_matrix_exact =
        receipt->host_saturn_non_title_capture_mask ==
        receipt->host_saturn_expected_capture_mask;
    receipt->host_route_capture_matrix_ready =
        receipt->host_route_consumes_active_capture_frame &&
        receipt->host_route_capture_matrix_exact &&
        ((receipt->capture_route == NEXUS_V1_STARTUP_CAPTURE_TITLE &&
          receipt->title_capture_uses_real_assets &&
          nexus_v1_launcher_startup_base_saturn_capture_exact(package)) ||
         (receipt->capture_route == NEXUS_V1_STARTUP_CAPTURE_SAVE &&
          (receipt->host_saturn_non_title_capture_mask & 1u)) ||
         (receipt->capture_route == NEXUS_V1_STARTUP_CAPTURE_CHAMPION &&
          (receipt->host_saturn_non_title_capture_mask & 2u) &&
          (!receipt->runtime_dgn_handoff_ready ||
           (receipt->host_route_consumes_dungeon_capture_frame &&
            (receipt->host_saturn_non_title_capture_mask & 4u)))));
    receipt->host_route_consumes_capture_matrix =
        receipt->host_route_consumes_package_route &&
        receipt->saturn_timing_exact &&
        receipt->saturn_capture_frames_exact &&
        receipt->host_route_capture_matrix_exact &&
        receipt->host_route_capture_matrix_ready;
    receipt->dgn_route_saturn_capture_exact =
        receipt->runtime_dgn_handoff_ready &&
        receipt->saturn_dungeon_capture_frame ==
            package->boot_start_ready_frames;
    receipt->dgn_route_consumes_startup_package =
        (receipt->host_route_consumes_package_route ||
         receipt->full_start_package_consumed) &&
        receipt->runtime_dgn_route_joined &&
        receipt->host_route_consumes_dungeon_capture_frame &&
        receipt->dgn_route_saturn_capture_exact &&
        receipt->dgn_draw_command_count > 0;
    receipt->dgn_route_consumes_startup_package =
        receipt->dgn_route_consumes_startup_package &&
        receipt->runtime_dgn_viewport_render_ready &&
        receipt->dgn_viewport_rasterized_command_count ==
            receipt->dgn_draw_command_count &&
        receipt->dgn_viewport_material_surface_count ==
            receipt->dgn_draw_command_count &&
        receipt->dgn_viewport_floor_material_surface_count ==
            receipt->dgn_render_plan.floor_count &&
        receipt->dgn_viewport_wall_material_surface_count ==
            receipt->dgn_render_plan.wall_count &&
        receipt->dgn_viewport_ceiling_material_surface_count ==
            receipt->dgn_render_plan.ceiling_count &&
        receipt->dgn_viewport_written_pixels > 0;
    receipt->dgn_material_surface_coverage_complete =
        receipt->dgn_route_consumes_startup_package &&
        receipt->dgn_viewport_material_surface_count ==
            receipt->dgn_draw_command_count;
    receipt->dgn_material_semantics_complete =
        receipt->dgn_material_surface_coverage_complete &&
        receipt->dgn_render_plan.material_semantics_complete &&
        receipt->dgn_render_plan.floor_material_command_count ==
            receipt->dgn_render_plan.floor_count &&
        receipt->dgn_render_plan.ceiling_material_command_count ==
            receipt->dgn_render_plan.ceiling_count &&
        receipt->dgn_render_plan.wall_material_command_count ==
            receipt->dgn_render_plan.wall_count;

    if (!receipt->no_fallback_visuals_enforced ||
        package->blocked_draw_suppressed ||
        asset_handoff->blocks_main_menu_route ||
        package->capture_route == NEXUS_V1_STARTUP_CAPTURE_BLOCKED) {
        receipt->route =
            NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_BLOCKED_ASSETS;
        receipt->status_scope = asset_handoff->status_scope
            ? asset_handoff->status_scope
            : package->status_scope;
        receipt->status = asset_handoff->status
            ? asset_handoff->status
            : package->status;
        receipt->blocked_route_suppresses_startup_draws =
            package->blocked_draw_suppressed &&
            receipt->startup_bundle.copied_command_count == 0;
        receipt->blocked_route_suppresses_dgn_draws =
            receipt->dgn_draw_command_count == 0;
        nexus_v1_launcher_fill_host_ownership_route_contract(receipt);
        receipt->dungeon_capture_route =
            receipt->dungeon_capture_route_consumed
                ? "runtime-dgn-handoff"
                : "none";
        return;
    }

    if (state && receipt->runtime_dgn_handoff_ready) {
        receipt->route =
            NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_RUNTIME_HANDOFF;
        receipt->status_scope = "RUNTIME";
        receipt->status = "runtime-handoff-owned";
    } else if (receipt->menu_capture_uses_real_assets) {
        receipt->route =
            NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_MENU_CAPTURE;
        receipt->status_scope = "STARTUP";
        receipt->status = "menu-capture-owned";
    } else if (receipt->title_capture_uses_real_assets) {
        receipt->route =
            NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_TITLE_CAPTURE;
        receipt->status_scope = "STARTUP";
        receipt->status = "title-capture-owned";
    }
    nexus_v1_launcher_fill_host_ownership_route_contract(receipt);
    receipt->host_route_consumes_dgn_material_path =
        receipt->dgn_material_surface_coverage_complete &&
        receipt->runtime_dgn_material_path_consumed &&
        receipt->dgn_route_consumes_startup_package &&
        receipt->dgn_route_consumes_host_ownership &&
        receipt->no_fallback_visuals_enforced &&
        !receipt->fallback_visuals_permitted;
    receipt->dungeon_capture_route =
        receipt->dungeon_capture_route_consumed
            ? "runtime-dgn-handoff"
            : "none";
}

static int nexus_v1_launcher_startup_runtime_route_from_host_state(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeRouteReceipt *out_receipt)
{
    Nexus_V1_StartupChampionExecution execution;
    Nexus_V1_StartupHostActionReceipt host_action;

    nexus_v1_launcher_startup_runtime_route_receipt_clear(out_receipt);
    if (!state) {
        return 0;
    }
    if (state->champion_select_active) {
        return nexus_v1_launcher_startup_runtime_route_from_champion_firestaff_input(
            state,
            menu_input,
            out_commands,
            max_commands,
            out_receipt);
    }
    if (state->title_active || state->save_select_active ||
        !state->engine || !state->engine->level_loaded) {
        return 0;
    }

    /* A successful save load has already restored the party and level, so it
     * reaches M11 without a champion-menu start command. Reuse the same
     * runtime handoff contract with an explicit start execution; the host
     * still owns BPK/DMDF/DGN gating and emits the first DGN render plan.
     * ReDMCSB LOADSAVE.C F0433 restores runtime state before returning
     * control to the game view. */
    memset(&execution, 0, sizeof(execution));
    execution.kind = NEXUS_V1_STARTUP_CHAMPION_EXEC_START_DUNGEON;
    execution.status_scope = "SAVE";
    execution.status = "resumed-dungeon";
    nexus_v1_startup_host_action_receipt_clear(&host_action);
    host_action.host_receipt.input_result =
        NEXUS_V1_STARTUP_HOST_INPUT_REDRAW;
    host_action.host_receipt.status_scope = execution.status_scope;
    host_action.host_receipt.status = execution.status;
    return nexus_v1_launcher_startup_runtime_route_from_champion_execution(
        state,
        &execution,
        &host_action,
        out_commands,
        max_commands,
        out_receipt);
}

int nexus_v1_launcher_startup_real_asset_ownership_from_runtime_state(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupRealAssetOwnershipReceipt *out_receipt)
{
    Nexus_V1_StartupDrawCommand startup_commands[80];

    nexus_v1_launcher_startup_real_asset_ownership_receipt_clear(
        out_receipt);
    if (!out_receipt || !state ||
        !nexus_v1_launcher_startup_receipt_bundle_from_runtime_state(
            runtime,
            state,
            menu_input,
            load_save,
            load_userdata,
            startup_commands,
            (int)(sizeof(startup_commands) / sizeof(startup_commands[0])),
            &out_receipt->startup_bundle)) {
        return 0;
    }

    memset(out_receipt->dgn_commands, 0, sizeof(out_receipt->dgn_commands));
    (void)nexus_v1_launcher_startup_runtime_route_from_host_state(
        state,
        menu_input,
        out_receipt->dgn_commands,
        (int)(sizeof(out_receipt->dgn_commands) /
              sizeof(out_receipt->dgn_commands[0])),
        &out_receipt->runtime_route);
    out_receipt->copied_dgn_command_count =
        out_receipt->runtime_route.dgn_render_command_count;
    if (out_receipt->copied_dgn_command_count < 0) {
        out_receipt->copied_dgn_command_count = 0;
    }
    if (out_receipt->copied_dgn_command_count >
        NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS) {
        out_receipt->copied_dgn_command_count =
            NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS;
    }
    nexus_v1_launcher_fill_real_asset_ownership(state, out_receipt);
    return 1;
}

int nexus_v1_launcher_startup_real_asset_ownership_from_snapshot(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupRealAssetOwnershipReceipt *out_receipt)
{
    if (!snapshot) {
        nexus_v1_launcher_startup_real_asset_ownership_receipt_clear(
            out_receipt);
        return 0;
    }
    return nexus_v1_launcher_startup_real_asset_ownership_from_runtime_state(
        runtime,
        &snapshot->runtime,
        menu_input,
        load_save,
        load_userdata,
        out_receipt);
}

static void nexus_v1_launcher_zero_startup_commands(
    Nexus_V1_StartupDrawCommand *commands,
    int max_commands)
{
    if (commands && max_commands > 0) {
        memset(commands, 0, (size_t)max_commands * sizeof(commands[0]));
    }
}

static void nexus_v1_launcher_zero_dgn_commands(
    Nexus_V1_DgnRenderCommand *commands,
    int max_commands)
{
    if (commands && max_commands > 0) {
        memset(commands, 0, (size_t)max_commands * sizeof(commands[0]));
    }
}

static const char *nexus_v1_launcher_host_route_from_ownership(
    Nexus_V1_StartupRealAssetOwnershipRoute route)
{
    switch (route) {
    case NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_TITLE_CAPTURE:
        return "title-capture";
    case NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_MENU_CAPTURE:
        return "menu-capture";
    case NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_RUNTIME_HANDOFF:
        return "runtime-dgn-handoff";
    case NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_BLOCKED_ASSETS:
        return "blocked-startup";
    case NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_INVALID:
    default:
        return "invalid";
    }
}

int nexus_v1_launcher_startup_host_caller_receipt_from_runtime_state(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupDrawCommand *out_startup_commands,
    int max_startup_commands,
    Nexus_V1_DgnRenderCommand *out_dgn_commands,
    int max_dgn_commands,
    Nexus_V1_StartupHostCallerReceipt *out_receipt)
{
    Nexus_V1_StartupReceiptBundle bundle;
    int startup_copied;
    int dgn_copied = 0;

    nexus_v1_launcher_startup_host_caller_receipt_clear(out_receipt);
    nexus_v1_launcher_zero_startup_commands(out_startup_commands,
                                            max_startup_commands);
    nexus_v1_launcher_zero_dgn_commands(out_dgn_commands, max_dgn_commands);
    if (!out_receipt || !state) {
        return 0;
    }

    if (!nexus_v1_launcher_startup_real_asset_ownership_from_runtime_state(
            runtime,
            state,
            menu_input,
            load_save,
            load_userdata,
            &out_receipt->ownership)) {
        return 0;
    }

    /* Ownership already built the canonical full-start package above.  The
     * host consumes that exact package instead of rebuilding it, so a save
     * or champion action cannot be evaluated twice while assembling a frame.
     * The package remains the sole source for title/save/champion capture
     * commands and its no-draw blocker state. */
    bundle = out_receipt->ownership.startup_bundle;
    startup_copied = nexus_v1_launcher_build_full_start_package_commands(
        state,
        &bundle.package,
        out_startup_commands,
        max_startup_commands);
    bundle.copied_command_count = startup_copied;

    /* The ownership receipt already evaluated the action and material plan.
     * Copy its immutable DGN command package; do not execute the route again
     * here, since ACTION start and save confirmation are stateful. */
    dgn_copied = out_receipt->ownership.copied_dgn_command_count;
    if (dgn_copied > max_dgn_commands) {
        dgn_copied = max_dgn_commands > 0 ? max_dgn_commands : 0;
    }
    if (dgn_copied > 0 && out_dgn_commands) {
        memcpy(out_dgn_commands,
               out_receipt->ownership.dgn_commands,
               (size_t)dgn_copied * sizeof(out_dgn_commands[0]));
    }

    out_receipt->receipt_owner_is_nexus =
        out_receipt->ownership.receipt_owner_is_nexus;
    out_receipt->host_startup_capture_ready =
        out_receipt->ownership.capture_ready &&
        (out_receipt->ownership.display_ready ||
         (state->title_active &&
          out_receipt->ownership.title_capture_uses_real_assets &&
          out_receipt->ownership.saturn_timing_exact &&
          out_receipt->ownership.saturn_capture_frames_exact));
    out_receipt->host_runtime_dgn_ready =
        out_receipt->ownership.runtime_dgn_handoff_ready;
    out_receipt->host_runtime_dgn_viewport_render_ready =
        out_receipt->ownership.runtime_dgn_viewport_render_ready;
    out_receipt->bpk_handoff_consumed =
        out_receipt->ownership.consumes_bpk_menu_handoff;
    out_receipt->prs3_blocker_consumed =
        out_receipt->ownership.consumes_prs3_blocker ||
        (state->engine &&
         state->engine->menu_bpk_upload_receipt.route ==
             NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_PRS3);
    out_receipt->dgn_handoff_consumed =
        out_receipt->ownership.consumes_dgn_handoff;
    out_receipt->no_fallback_visuals_enforced =
        out_receipt->ownership.no_fallback_visuals_enforced;
    out_receipt->suppress_fallback_visuals =
        out_receipt->ownership.no_fallback_visuals_enforced &&
        !out_receipt->ownership.fallback_visuals_permitted;
    out_receipt->suppress_legacy_placeholder_visuals =
        out_receipt->suppress_fallback_visuals;
    out_receipt->startup_command_count =
        out_receipt->ownership.startup_draw_command_count;
    out_receipt->copied_startup_command_count = startup_copied;
    out_receipt->dgn_command_count =
        out_receipt->ownership.dgn_draw_command_count;
    out_receipt->dgn_viewport_rasterized_command_count =
        out_receipt->ownership.dgn_viewport_rasterized_command_count;
    out_receipt->dgn_viewport_material_surface_count =
        out_receipt->ownership.dgn_viewport_material_surface_count;
    out_receipt->dgn_viewport_floor_material_surface_count =
        out_receipt->ownership.dgn_viewport_floor_material_surface_count;
    out_receipt->dgn_viewport_ceiling_material_surface_count =
        out_receipt->ownership.dgn_viewport_ceiling_material_surface_count;
    out_receipt->dgn_viewport_wall_material_surface_count =
        out_receipt->ownership.dgn_viewport_wall_material_surface_count;
    out_receipt->dgn_viewport_host_route_status =
        out_receipt->ownership.dgn_viewport_host_route_status;
    out_receipt->dgn_viewport_host_route_ready =
        out_receipt->ownership.dgn_viewport_host_route_ready;
    out_receipt->dgn_viewport_host_route_consumed =
        out_receipt->ownership.dgn_viewport_host_route_consumed;
    out_receipt->dgn_viewport_host_route_package_consumed =
        out_receipt->ownership.dgn_viewport_host_route_package_consumed;
    out_receipt->dgn_viewport_host_route_blocks_runtime =
        out_receipt->ownership.dgn_viewport_host_route_blocks_runtime;
    out_receipt->dgn_viewport_capture_ready =
        out_receipt->ownership.dgn_viewport_capture_ready;
    out_receipt->dgn_viewport_frame_hash =
        out_receipt->ownership.dgn_viewport_frame_hash;
    out_receipt->dgn_material_surface_coverage_complete =
        out_receipt->ownership.dgn_material_surface_coverage_complete;
    out_receipt->dgn_material_semantics_complete =
        out_receipt->ownership.dgn_material_semantics_complete;
    out_receipt->host_runtime_dgn_material_path_consumed =
        out_receipt->ownership.runtime_dgn_material_path_consumed;
    out_receipt->host_route_consumes_dgn_material_path =
        out_receipt->ownership.host_route_consumes_dgn_material_path;
    out_receipt->bpk_material_surface_count =
        out_receipt->ownership.bpk_material_surface_count;
    out_receipt->bpk_truecolor_material_surface_count =
        out_receipt->ownership.bpk_truecolor_material_surface_count;
    out_receipt->bpk_prs3_material_surface_count =
        out_receipt->ownership.bpk_prs3_material_surface_count;
    out_receipt->dgn_viewport_written_pixels =
        out_receipt->ownership.dgn_viewport_written_pixels;
    out_receipt->copied_dgn_command_count = dgn_copied;
    out_receipt->copied_dgn_material_plan_complete =
        dgn_copied > 0 &&
        dgn_copied == out_receipt->dgn_command_count;
    out_receipt->title_timing_frame =
        out_receipt->ownership.startup_bundle.timing_frame;
    out_receipt->title_timing_frame_max =
        out_receipt->ownership.startup_bundle.timing_frame_max;
    out_receipt->title_timing_ready =
        out_receipt->ownership.startup_bundle.timing_ready;
    out_receipt->full_start_package_consumed =
        out_receipt->ownership.full_start_package_consumed;
    out_receipt->package_capture_consumed_by_host =
        out_receipt->ownership.package_capture_consumed_by_host;
    out_receipt->package_route_matches_capture_route =
        out_receipt->ownership.package_route_matches_capture_route;
    out_receipt->host_route_consumes_package_route =
        out_receipt->ownership.host_route_consumes_package_route;
    out_receipt->host_route_consumes_capture_matrix =
        out_receipt->ownership.host_route_consumes_capture_matrix;
    out_receipt->dgn_route_consumes_startup_package =
        out_receipt->ownership.dgn_route_consumes_startup_package;
    out_receipt->dgn_route_saturn_capture_exact =
        out_receipt->ownership.dgn_route_saturn_capture_exact;
    out_receipt->dungeon_capture_route_consumed =
        out_receipt->ownership.dungeon_capture_route_consumed;
    out_receipt->host_ownership_route_matches_capture_route =
        out_receipt->ownership.host_ownership_route_matches_capture_route;
    out_receipt->package_route_consumes_host_ownership =
        out_receipt->ownership.package_route_consumes_host_ownership;
    out_receipt->dgn_route_consumes_host_ownership =
        out_receipt->ownership.dgn_route_consumes_host_ownership;
    out_receipt->startup_route_consumption_complete =
        out_receipt->ownership.startup_route_consumption_complete;
    out_receipt->non_title_saturn_capture_route_complete =
        out_receipt->ownership.non_title_saturn_capture_route_complete;
    out_receipt->dungeon_startup_route_consumption_complete =
        out_receipt->ownership.dungeon_startup_route_consumption_complete;
    out_receipt->startup_route_consumes_package_capture =
        out_receipt->ownership.startup_route_consumes_package_capture;
    out_receipt->title_route_consumes_package_capture =
        out_receipt->ownership.title_route_consumes_package_capture;
    out_receipt->save_route_consumes_package_capture =
        out_receipt->ownership.save_route_consumes_package_capture;
    out_receipt->champion_route_consumes_package_capture =
        out_receipt->ownership.champion_route_consumes_package_capture;
    out_receipt->dungeon_route_consumes_package_capture =
        out_receipt->ownership.dungeon_route_consumes_package_capture;
    out_receipt->startup_route_saturn_capture_exact =
        out_receipt->ownership.startup_route_saturn_capture_exact;
    out_receipt->title_route_saturn_capture_exact =
        out_receipt->ownership.title_route_saturn_capture_exact;
    out_receipt->save_route_saturn_capture_exact =
        out_receipt->ownership.save_route_saturn_capture_exact;
    out_receipt->champion_route_saturn_capture_exact =
        out_receipt->ownership.champion_route_saturn_capture_exact;
    out_receipt->dungeon_route_saturn_capture_exact =
        out_receipt->ownership.dungeon_route_saturn_capture_exact;
    out_receipt->startup_host_package_route_complete =
        out_receipt->ownership.startup_host_package_route_complete;
    out_receipt->title_host_package_route_complete =
        out_receipt->ownership.title_host_package_route_complete;
    out_receipt->save_host_package_route_complete =
        out_receipt->ownership.save_host_package_route_complete;
    out_receipt->champion_host_package_route_complete =
        out_receipt->ownership.champion_host_package_route_complete;
    out_receipt->dungeon_host_package_route_complete =
        out_receipt->ownership.dungeon_host_package_route_complete;
    out_receipt->host_package_route_complete_mask =
        out_receipt->ownership.host_package_route_complete_mask;
    out_receipt->host_package_route_expected_mask =
        out_receipt->ownership.host_package_route_expected_mask;
    out_receipt->host_package_route_matrix_complete =
        out_receipt->ownership.host_package_route_matrix_complete;
    out_receipt->host_saturn_exact_capture_mask =
        out_receipt->ownership.host_saturn_exact_capture_mask;
    out_receipt->host_saturn_route_timing_matrix_complete =
        out_receipt->ownership.host_saturn_route_timing_matrix_complete;
    out_receipt->host_package_route_timing_matrix_complete =
        out_receipt->ownership.host_package_route_timing_matrix_complete;
    out_receipt->host_all_route_complete_mask =
        out_receipt->ownership.host_all_route_complete_mask;
    out_receipt->host_all_route_expected_mask =
        out_receipt->ownership.host_all_route_expected_mask;
    out_receipt->host_all_route_matrix_complete =
        out_receipt->ownership.host_all_route_matrix_complete;
    out_receipt->host_saturn_all_exact_capture_mask =
        out_receipt->ownership.host_saturn_all_exact_capture_mask;
    out_receipt->host_saturn_all_expected_capture_mask =
        out_receipt->ownership.host_saturn_all_expected_capture_mask;
    out_receipt->host_saturn_all_route_timing_matrix_complete =
        out_receipt->ownership.host_saturn_all_route_timing_matrix_complete;
    out_receipt->host_all_route_timing_matrix_complete =
        out_receipt->ownership.host_all_route_timing_matrix_complete;
    out_receipt->startup_bundle_consumed =
        out_receipt->ownership.startup_bundle.package
            .full_start_package_receipt_ready;
    out_receipt->display_callers_use_package_receipt =
        out_receipt->full_start_package_consumed &&
        out_receipt->startup_bundle_consumed;
    out_receipt->title_menu_capture_route_joined =
        out_receipt->ownership.title_menu_capture_route_joined;
    out_receipt->runtime_dgn_route_joined =
        out_receipt->ownership.runtime_dgn_route_joined;
    out_receipt->saturn_warning_frame =
        out_receipt->ownership.startup_bundle.package.saturn_warning_frame;
    out_receipt->saturn_title_capture_frame =
        out_receipt->ownership.startup_bundle.package
            .saturn_title_capture_frame;
    out_receipt->saturn_save_capture_frame =
        out_receipt->ownership.saturn_save_capture_frame;
    out_receipt->saturn_champion_capture_frame =
        out_receipt->ownership.saturn_champion_capture_frame;
    out_receipt->saturn_dungeon_capture_frame =
        out_receipt->ownership.saturn_dungeon_capture_frame;
    out_receipt->saturn_title_ready_frame =
        out_receipt->ownership.startup_bundle.package
            .saturn_title_ready_frame;
    out_receipt->saturn_gameover_capture_frame =
        out_receipt->ownership.startup_bundle.package
            .saturn_gameover_capture_frame;
    out_receipt->saturn_timing_exact =
        out_receipt->ownership.saturn_timing_exact;
    out_receipt->saturn_capture_frames_exact =
        out_receipt->ownership.saturn_capture_frames_exact;
    out_receipt->host_active_capture_frame =
        out_receipt->ownership.active_capture_frame;
    out_receipt->host_saturn_active_capture_frame =
        out_receipt->ownership.saturn_active_capture_frame;
    out_receipt->host_route_consumes_active_capture_frame =
        out_receipt->ownership.host_route_consumes_active_capture_frame;
    out_receipt->host_route_consumes_dungeon_capture_frame =
        out_receipt->ownership.host_route_consumes_dungeon_capture_frame;
    out_receipt->host_route_capture_matrix_ready =
        out_receipt->ownership.host_route_capture_matrix_ready;
    out_receipt->host_route_capture_matrix_exact =
        out_receipt->ownership.host_route_capture_matrix_exact;
    out_receipt->host_saturn_non_title_capture_count =
        out_receipt->ownership.host_saturn_non_title_capture_count;
    out_receipt->host_saturn_non_title_capture_mask =
        out_receipt->ownership.host_saturn_non_title_capture_mask;
    out_receipt->host_saturn_expected_capture_mask =
        out_receipt->ownership.host_saturn_expected_capture_mask;
    out_receipt->capture_route = out_receipt->ownership.capture_route;
    out_receipt->ownership_route = out_receipt->ownership.route;
    out_receipt->host_route =
        nexus_v1_launcher_host_route_from_ownership(out_receipt->ownership.route);
    out_receipt->dungeon_capture_route =
        out_receipt->ownership.dungeon_capture_route;
    out_receipt->startup_package_route =
        out_receipt->ownership.startup_bundle.package.consumer_route;
    out_receipt->status_scope = out_receipt->ownership.status_scope;
    out_receipt->status = out_receipt->ownership.status;

    out_receipt->host_execute_startup_draws =
        out_receipt->suppress_fallback_visuals &&
        out_receipt->host_startup_capture_ready &&
        (out_receipt->host_route_consumes_package_route ||
         (out_receipt->capture_route == NEXUS_V1_STARTUP_CAPTURE_TITLE &&
          out_receipt->ownership.title_capture_uses_real_assets)) &&
        (out_receipt->capture_route == NEXUS_V1_STARTUP_CAPTURE_TITLE ||
         out_receipt->host_route_consumes_active_capture_frame) &&
        (out_receipt->capture_route == NEXUS_V1_STARTUP_CAPTURE_TITLE ||
         out_receipt->startup_route_consumption_complete) &&
        out_receipt->saturn_timing_exact &&
        out_receipt->saturn_capture_frames_exact &&
        out_receipt->copied_startup_command_count > 0;
    out_receipt->host_execute_dgn_draws =
        out_receipt->suppress_fallback_visuals &&
        out_receipt->host_runtime_dgn_ready &&
        out_receipt->host_runtime_dgn_viewport_render_ready &&
        out_receipt->dgn_viewport_host_route_ready &&
        out_receipt->dgn_viewport_host_route_consumed &&
        out_receipt->dgn_viewport_host_route_package_consumed &&
        !out_receipt->dgn_viewport_host_route_blocks_runtime &&
        out_receipt->dungeon_capture_route_consumed &&
        (out_receipt->host_route_consumes_package_route ||
         out_receipt->dgn_route_consumes_startup_package) &&
        (out_receipt->dungeon_startup_route_consumption_complete ||
         out_receipt->dungeon_host_package_route_complete) &&
        out_receipt->dgn_material_surface_coverage_complete &&
        out_receipt->host_route_consumes_dgn_material_path &&
        out_receipt->copied_dgn_material_plan_complete &&
        out_receipt->dgn_viewport_material_surface_count ==
            out_receipt->dgn_command_count &&
        out_receipt->dgn_viewport_rasterized_command_count ==
            out_receipt->dgn_command_count &&
        out_receipt->dgn_viewport_written_pixels > 0 &&
        out_receipt->dgn_viewport_capture_ready &&
        out_receipt->dgn_viewport_frame_hash != 0u;
    out_receipt->single_saturn_startup_owner_ready =
        out_receipt->receipt_owner_is_nexus &&
        out_receipt->package_capture_consumed_by_host &&
        out_receipt->display_callers_use_package_receipt &&
        out_receipt->title_menu_capture_route_joined &&
        !out_receipt->ownership.fallback_visuals_permitted;
    if (out_receipt->host_runtime_dgn_ready) {
        out_receipt->single_saturn_startup_owner_ready =
            out_receipt->single_saturn_startup_owner_ready &&
            out_receipt->runtime_dgn_route_joined;
    }
    out_receipt->blocked_route_suppresses_all_draws =
        out_receipt->ownership.route ==
            NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_BLOCKED_ASSETS &&
        out_receipt->ownership.blocked_route_suppresses_startup_draws &&
        out_receipt->ownership.blocked_route_suppresses_dgn_draws &&
        out_receipt->copied_startup_command_count == 0 &&
        out_receipt->copied_dgn_command_count == 0;
    out_receipt->host_caller_ready =
        out_receipt->receipt_owner_is_nexus &&
        out_receipt->suppress_fallback_visuals &&
        (out_receipt->host_execute_startup_draws ||
         out_receipt->host_execute_dgn_draws ||
         out_receipt->ownership.route ==
             NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_BLOCKED_ASSETS);
    return 1;
}

int nexus_v1_launcher_startup_host_caller_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupDrawCommand *out_startup_commands,
    int max_startup_commands,
    Nexus_V1_DgnRenderCommand *out_dgn_commands,
    int max_dgn_commands,
    Nexus_V1_StartupHostCallerReceipt *out_receipt)
{
    if (!snapshot) {
        nexus_v1_launcher_startup_host_caller_receipt_clear(out_receipt);
        nexus_v1_launcher_zero_startup_commands(out_startup_commands,
                                                max_startup_commands);
        nexus_v1_launcher_zero_dgn_commands(out_dgn_commands,
                                            max_dgn_commands);
        return 0;
    }
    return nexus_v1_launcher_startup_host_caller_receipt_from_runtime_state(
        runtime,
        &snapshot->runtime,
        menu_input,
        load_save,
        load_userdata,
        out_startup_commands,
        max_startup_commands,
        out_dgn_commands,
        max_dgn_commands,
        out_receipt);
}

void nexus_v1_launcher_startup_title_transition_capture_receipt_clear(
    Nexus_V1_StartupTitleTransitionCaptureReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->active_frame = -1;
    receipt->expected_title_frame = -1;
    receipt->expected_draw_kind = NEXUS_V1_STARTUP_DRAW_NONE;
    receipt->status = "invalid";
}

int nexus_v1_launcher_startup_title_transition_capture_receipt_from_host(
    const Nexus_V1_StartupHostCallerReceipt *host,
    int active_frame,
    const Nexus_V1_StartupDrawCommand *commands,
    int command_count,
    Nexus_V1_StartupTitleTransitionCaptureReceipt *out_receipt)
{
    const Nexus_V1_StartupFullStartPackageReceipt *package;
    Nexus_V1_BootFrame boot_frame;
    const Nexus_V1_StartupDrawCommand *command;

    nexus_v1_launcher_startup_title_transition_capture_receipt_clear(
        out_receipt);
    if (!host || !out_receipt || !commands || command_count <= 0 ||
        active_frame < 0) {
        return 0;
    }
    package = &host->ownership.startup_bundle.package;
    memset(&boot_frame, 0, sizeof(boot_frame));
    if (!nexus_v1_boot_frame(active_frame, NEXUS_FB_H, &boot_frame)) {
        return 0;
    }

    command = &commands[0];
    out_receipt->active_frame = active_frame;
    out_receipt->warning_boundary =
        active_frame == nexus_v1_boot_warning_frames() - 1;
    out_receipt->title_boundary =
        active_frame == nexus_v1_boot_warning_frames();
    out_receipt->start_ready_boundary =
        active_frame == nexus_v1_boot_start_ready_frames();
    out_receipt->warning_surface_verified =
        package->consumer.full_start.assets.warning_surface_loaded &&
        package->warning_capture_surface_ready;
    out_receipt->title_surface_verified =
        package->consumer.full_start.assets.title_surface_loaded &&
        package->title_capture_surface_ready;
    out_receipt->timing_verified =
        nexus_v1_launcher_startup_base_saturn_capture_exact(package);
    out_receipt->menu_bpk_prs3_blocked =
        package->consumer.full_start.assets.menu_bpk_upload_receipt_valid &&
        package->consumer.full_start.assets.menu_bpk_upload_route ==
            NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_PRS3 &&
        package->consumer.full_start.assets
            .menu_bpk_blocks_real_menu_surface_render &&
        !package->consumer.full_start.assets.real_menu_surface_route_ready;

    if (boot_frame.warning_visible) {
        out_receipt->expected_draw_kind =
            NEXUS_V1_STARTUP_DRAW_WARNING_BACKGROUND;
        out_receipt->command_verified =
            command->kind == out_receipt->expected_draw_kind;
        out_receipt->consumer_ready =
            host->host_execute_startup_draws &&
            out_receipt->warning_surface_verified &&
            out_receipt->timing_verified &&
            out_receipt->command_verified;
        out_receipt->status = out_receipt->consumer_ready
            ? "warning-capture"
            : "blocked-warning-capture";
    } else {
        out_receipt->expected_draw_kind =
            NEXUS_V1_STARTUP_DRAW_BOOT_TITLE_FRAME;
        out_receipt->expected_title_frame = boot_frame.title_frame;
        out_receipt->command_verified =
            command->kind == out_receipt->expected_draw_kind &&
            command->title_frame == out_receipt->expected_title_frame;
        out_receipt->consumer_ready =
            host->host_execute_startup_draws &&
            out_receipt->title_surface_verified &&
            out_receipt->timing_verified &&
            out_receipt->command_verified;
        out_receipt->status = out_receipt->consumer_ready
            ? (boot_frame.start_ready ? "title-start-ready" : "title-capture")
            : "blocked-title-capture";
    }
    return out_receipt->consumer_ready;
}

int nexus_v1_launcher_startup_route_proof_from_snapshot(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    const Nexus_V1_StartupChampionExecution *execution,
    const Nexus_V1_StartupHostActionReceipt *host_action,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRouteProofReceipt *out_receipt)
{
    if (!snapshot) {
        nexus_v1_launcher_startup_route_proof_receipt_clear(out_receipt);
        return 0;
    }
    return nexus_v1_launcher_startup_route_proof_from_runtime_state(
        runtime,
        &snapshot->runtime,
        execution,
        host_action,
        out_commands,
        max_commands,
        out_receipt);
}

int nexus_v1_launcher_startup_presentation_build_save_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands)
{
    int count;
    if (!state) {
        return 0;
    }
    count = nexus_v1_startup_presentation_build_save_from_facts(
        state->save_dir,
        state->slot_mask,
        state->save_selected_row,
        out_commands,
        max_commands);
    /* FONT012 2bpp glyph decode proven (pass 216): three RLOWFIX.BIN FONT
     * resources (291+250+710 glyphs, 6x12/12x12, palette FFFF/DEF7/B9CE/8000).
     * TEXT4 15-entry menu-options resource with TABL character encoding.
     * VDP2 CHCTLA at 0x25F00006 and CRAM palette upload at 0x25F80000
     * authenticated from SH-2 disassembly.  TEXT draw commands admitted. */
    return count;
}

int nexus_v1_launcher_startup_presentation_build_save_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands)
{
    if (!snapshot) {
        return 0;
    }
    return nexus_v1_launcher_startup_presentation_build_save_from_runtime_state(
        &snapshot->runtime,
        out_commands,
        max_commands);
}

static void nexus_v1_launcher_fill_menu_presentation_receipt(
    Nexus_V1_StartupMenuPresentationReceipt *receipt,
    Nexus_V1_StartupMenuPresentationKind kind,
    const Nexus_V1_LauncherStartupAssetsReceipt *assets,
    const Nexus_V1_StartupFullStartReceipt *full_start,
    int route_ready,
    int command_count)
{
    if (!receipt || !assets) {
        return;
    }
    nexus_v1_launcher_startup_menu_presentation_receipt_clear(receipt);
    receipt->kind = kind;
    receipt->assets = *assets;
    receipt->route_ready = route_ready ? 1 : 0;
    receipt->route_blocked = route_ready ? 0 : 1;
    receipt->draw_command_count = route_ready ? command_count : 0;
    receipt->asset_route = assets->startup_menu_asset_route;
    receipt->asset_blocker = assets->real_menu_surface_blocker;
    if (full_start) {
        receipt->host_caller_valid = 1;
        receipt->package_capture_consumed_by_host =
            route_ready &&
                    full_start->m11_host_route_ready &&
                    full_start->full_start_graphics_ready
                ? 1
                : 0;
        receipt->display_callers_use_package_receipt =
            receipt->package_capture_consumed_by_host;
        receipt->suppress_fallback_visuals =
            full_start->fallback_visuals_permitted ? 0 : 1;
        receipt->blocked_route_suppresses_all_draws =
            !route_ready && receipt->suppress_fallback_visuals;
        receipt->status_scope = full_start->host_receipt.status_scope
            ? full_start->host_receipt.status_scope
            : full_start->status_scope;
        receipt->status = full_start->host_receipt.status
            ? full_start->host_receipt.status
            : full_start->status;
    } else {
        receipt->status_scope = route_ready ? "STARTUP" : "ASSETS";
        receipt->status = route_ready
            ? (kind == NEXUS_V1_STARTUP_MENU_PRESENTATION_SAVE
                   ? "NEXUS SAVE SELECT"
                   : "NEXUS CHAMPIONS")
            : (assets->startup_menu_asset_route
                   ? assets->startup_menu_asset_route
                   : "blocked-startup-assets");
    }
    receipt->host_receipt.input_result = NEXUS_V1_STARTUP_HOST_INPUT_REDRAW;
    receipt->host_receipt.status_scope = receipt->status_scope;
    receipt->host_receipt.status = receipt->status;
}

int nexus_v1_launcher_startup_save_presentation_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupMenuPresentationReceipt *out_receipt)
{
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    Nexus_V1_LauncherRuntimeReceipt derived_runtime;
    Nexus_V1_StartupFullStartReceipt full_start;
    int command_count = 0;
    int full_start_valid = 0;

    nexus_v1_launcher_startup_menu_presentation_receipt_clear(out_receipt);
    if (!out_receipt ||
        !nexus_v1_launcher_startup_assets_from_runtime_state(state,
                                                             &assets)) {
        return 0;
    }
    nexus_v1_launcher_runtime_receipt_clear(&derived_runtime);
    full_start_valid =
        nexus_v1_launcher_build_runtime_receipt_from_startup_state(
            state,
            &derived_runtime) &&
        nexus_v1_launcher_startup_full_start_receipt_from_runtime_state(
            &derived_runtime,
            state,
            &full_start);
    if (assets.save_menu_route_ready) {
        command_count =
            nexus_v1_launcher_startup_presentation_build_save_from_runtime_state(
                state,
                out_commands,
                max_commands);
    }
    nexus_v1_launcher_fill_menu_presentation_receipt(
        out_receipt,
        NEXUS_V1_STARTUP_MENU_PRESENTATION_SAVE,
        &assets,
        full_start_valid ? &full_start : NULL,
        assets.save_menu_route_ready,
        command_count);
    return 1;
}

int nexus_v1_launcher_startup_save_presentation_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupMenuPresentationReceipt *out_receipt)
{
    if (!snapshot) {
        nexus_v1_launcher_startup_menu_presentation_receipt_clear(out_receipt);
        return 0;
    }
    return nexus_v1_launcher_startup_save_presentation_receipt_from_runtime_state(
        &snapshot->runtime,
        out_commands,
        max_commands,
        out_receipt);
}

int nexus_v1_launcher_startup_presentation_build_champion_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands)
{
    int count;
    if (!state) {
        return 0;
    }
    count = nexus_v1_startup_presentation_build_champion_from_facts(
        state->engine ? &state->engine->champions : NULL,
        state->slot_mask,
        state->champion_cursor,
        state->champion_frame,
        out_commands,
        max_commands);
    if (out_commands && count > 0) {
        int read_index, write_index;
        for (read_index = 0, write_index = 0; read_index < count;
             ++read_index) {
            if (out_commands[read_index].kind != NEXUS_V1_STARTUP_DRAW_TEXT)
                out_commands[write_index++] = out_commands[read_index];
        }
        count = write_index;
    }
    return count;
}

int nexus_v1_launcher_startup_presentation_build_champion_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands)
{
    if (!snapshot) {
        return 0;
    }
    return nexus_v1_launcher_startup_presentation_build_champion_from_runtime_state(
        &snapshot->runtime,
        out_commands,
        max_commands);
}

int nexus_v1_launcher_startup_champion_presentation_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupMenuPresentationReceipt *out_receipt)
{
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    Nexus_V1_LauncherRuntimeReceipt derived_runtime;
    Nexus_V1_StartupFullStartReceipt full_start;
    int command_count = 0;
    int full_start_valid = 0;

    nexus_v1_launcher_startup_menu_presentation_receipt_clear(out_receipt);
    if (!out_receipt ||
        !nexus_v1_launcher_startup_assets_from_runtime_state(state,
                                                             &assets)) {
        return 0;
    }
    nexus_v1_launcher_runtime_receipt_clear(&derived_runtime);
    full_start_valid =
        nexus_v1_launcher_build_runtime_receipt_from_startup_state(
            state,
            &derived_runtime) &&
        nexus_v1_launcher_startup_full_start_receipt_from_runtime_state(
            &derived_runtime,
            state,
            &full_start);
    if (assets.champion_menu_route_ready) {
        command_count =
            nexus_v1_launcher_startup_presentation_build_champion_from_runtime_state(
                state,
                out_commands,
                max_commands);
    }
    nexus_v1_launcher_fill_menu_presentation_receipt(
        out_receipt,
        NEXUS_V1_STARTUP_MENU_PRESENTATION_CHAMPION,
        &assets,
        full_start_valid ? &full_start : NULL,
        assets.champion_menu_route_ready,
        command_count);
    return 1;
}

int nexus_v1_launcher_startup_champion_presentation_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupMenuPresentationReceipt *out_receipt)
{
    if (!snapshot) {
        nexus_v1_launcher_startup_menu_presentation_receipt_clear(out_receipt);
        return 0;
    }
    return nexus_v1_launcher_startup_champion_presentation_receipt_from_runtime_state(
        &snapshot->runtime,
        out_commands,
        max_commands,
        out_receipt);
}

int nexus_v1_launcher_startup_presentation_execute(
    const Nexus_V1_StartupDrawCommand *commands,
    int command_count,
    const Nexus_V1_StartupDrawExecutor *executor)
{
    return nexus_v1_startup_presentation_execute(
        commands,
        command_count,
        executor);
}

int nexus_v1_launcher_startup_presentation_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    char *out_phase,
    int out_phase_size,
    int *out_startup_active,
    int *out_startup_frame,
    char *out_animation,
    int out_animation_size,
    int *out_animation_active,
    int *out_title_frame,
    int *out_title_frame_max,
    int *out_title_ready)
{
    if (!state) {
        return 0;
    }
    return nexus_v1_startup_presentation_receipt(
        state->title_active,
        state->save_select_active,
        state->champion_select_active,
        state->title_frame,
        out_phase,
        out_phase_size,
        out_startup_active,
        out_startup_frame,
        out_animation,
        out_animation_size,
        out_animation_active,
        out_title_frame,
        out_title_frame_max,
        out_title_ready);
}

int nexus_v1_launcher_startup_presentation_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    char *out_phase,
    int out_phase_size,
    int *out_startup_active,
    int *out_startup_frame,
    char *out_animation,
    int out_animation_size,
    int *out_animation_active,
    int *out_title_frame,
    int *out_title_frame_max,
    int *out_title_ready)
{
    if (!snapshot) {
        return 0;
    }
    return nexus_v1_launcher_startup_presentation_receipt_from_runtime_state(
        &snapshot->runtime,
        out_phase,
        out_phase_size,
        out_startup_active,
        out_startup_frame,
        out_animation,
        out_animation_size,
        out_animation_active,
        out_title_frame,
        out_title_frame_max,
        out_title_ready);
}

int nexus_v1_launcher_startup_resume_status_host_receipt(
    Nexus_V1_StartupResumeStatus status,
    Nexus_V1_StartupHostReceipt *out_receipt)
{
    return nexus_v1_startup_resume_status_host_receipt(status, out_receipt);
}

int nexus_v1_launcher_startup_boot_status_host_receipt(
    Nexus_V1_StartupBootStatus status,
    Nexus_V1_StartupHostReceipt *out_receipt)
{
    return nexus_v1_startup_boot_status_host_receipt(status, out_receipt);
}

static void nexus_v1_launcher_fill_boot_receipt(
    const char *data_dir,
    Nexus_V1_Engine *engine,
    int title_loaded,
    Nexus_V1_LauncherBootReceipt *receipt)
{
    if (!receipt || !engine) {
        return;
    }
    receipt->engine = engine;
    receipt->level_loaded = engine->level_loaded;
    receipt->party_x = engine->game.party_x;
    receipt->party_y = engine->game.party_y;
    receipt->party_dir = engine->game.party_dir;
    receipt->tick_count = engine->game.tick_count;
    receipt->title_loaded = title_loaded ? 1 : 0;
    snprintf(receipt->dungeon_path,
             sizeof(receipt->dungeon_path),
             "%s/LEV00.DGN",
             data_dir ? data_dir : "");
    nexus_v1_launcher_fill_startup_assets_receipt(
        engine,
        title_loaded,
        &receipt->startup_assets);
}

int nexus_v1_launcher_boot_level0_startup(
    const char *data_dir,
    Nexus_TitleScreen *title,
    Nexus_V1_LauncherBootReceipt *out_receipt)
{
    Nexus_V1_StartupHostFacts facts;
    Nexus_V1_Engine *engine;
    int title_loaded = 0;

    if (out_receipt) {
        nexus_v1_launcher_boot_receipt_clear(out_receipt);
    }
    if (!data_dir || !out_receipt) {
        return 0;
    }
    if (nexus_v1_launcher_init(data_dir) != 0) {
        (void)nexus_v1_startup_boot_status_host_receipt(
            NEXUS_V1_STARTUP_BOOT_STATUS_DATA_ERROR,
            &out_receipt->startup_receipt.host_receipt);
        return 0;
    }
    if (nexus_v1_launcher_load_level(0) != 0) {
        (void)nexus_v1_startup_boot_status_host_receipt(
            NEXUS_V1_STARTUP_BOOT_STATUS_LEVEL_ERROR,
            &out_receipt->startup_receipt.host_receipt);
        nexus_v1_launcher_shutdown();
        return 0;
    }
    engine = nexus_v1_launcher_get_engine();
    if (!engine) {
        (void)nexus_v1_startup_boot_status_host_receipt(
            NEXUS_V1_STARTUP_BOOT_STATUS_DATA_ERROR,
            &out_receipt->startup_receipt.host_receipt);
        return 0;
    }

    /* New selected-entry boots always start from Nexus defaults; save
     * resume applies persisted party/tick state in the M11 resume path. */
    nexus_v1_game_init(&engine->game, engine->data_dir);
    nexus_v1_sync_dgn_runtime_pose(engine, 0, engine->game.party_x,
                                   engine->game.party_y, engine->game.party_dir);
    {
        int rlowfix_size = 0;
        uint8_t *rlowfix = nexus_v1_read_file(engine, "RLOWFIX.BIN",
                                              &rlowfix_size);
        if (!rlowfix || !nexus_v1_champions_init_from_rlowfix(
                &engine->champions, rlowfix, (size_t)rlowfix_size)) {
            memset(&engine->champions, 0, sizeof(engine->champions));
            engine->champions.leader_index = -1;
        }
        free(rlowfix);
    }
    if (engine->mechanics) {
        nexus_mechanics_init(engine->mechanics,
                             engine->game.party_x,
                             engine->game.party_y,
                             engine->game.party_dir);
        engine->mechanics->map_index = 0;
    }

    if (title) {
        title_loaded = nexus_title_load(title, engine) == 0 && title->loaded;
    }

    memset(&facts, 0, sizeof(facts));
    facts.champion_pool = &engine->champions;
    if (!nexus_v1_startup_launch_from_host_facts_with_receipt(
            &facts,
            &out_receipt->startup_receipt)) {
        nexus_v1_launcher_shutdown();
        return 0;
    }
    nexus_v1_launcher_fill_boot_receipt(data_dir,
                                        engine,
                                        title_loaded,
                                        out_receipt);
    if (!out_receipt->startup_assets.title_screen_loaded ||
        !out_receipt->startup_assets.warning_surface_loaded) {
        (void)nexus_v1_startup_boot_status_host_receipt(
            NEXUS_V1_STARTUP_BOOT_STATUS_ASSET_ERROR,
            &out_receipt->startup_receipt.host_receipt);
        nexus_v1_launcher_shutdown();
        return 0;
    }
    return 1;
}

int nexus_v1_launcher_boot_level0_runtime_startup(
    const char *data_dir,
    Nexus_TitleScreen *title,
    Nexus_V1_LauncherRuntimeReceipt *out_receipt)
{
    Nexus_V1_LauncherBootReceipt boot_receipt;
    Nexus_V1_StartupBootStatus boot_status;

    nexus_v1_launcher_runtime_receipt_clear(out_receipt);
    nexus_v1_launcher_boot_receipt_clear(&boot_receipt);
    if (!out_receipt) {
        return 0;
    }
    snprintf(out_receipt->title,
             sizeof(out_receipt->title),
             "%s",
             NEXUS_V1_GAME_LABEL);
    snprintf(out_receipt->source_id,
             sizeof(out_receipt->source_id),
             "%s",
             NEXUS_V1_GAME_ID);
    if (!nexus_v1_launcher_boot_level0_startup(data_dir,
                                               title,
                                               &boot_receipt)) {
        out_receipt->startup_receipt = boot_receipt.startup_receipt;
        return 0;
    }

    out_receipt->engine = boot_receipt.engine;
    out_receipt->title_screen = title;
    out_receipt->title_screen_keep = boot_receipt.title_loaded ? 1 : 0;
    out_receipt->level_loaded = boot_receipt.level_loaded;
    out_receipt->party_x = boot_receipt.party_x;
    out_receipt->party_y = boot_receipt.party_y;
    out_receipt->party_dir = boot_receipt.party_dir;
    out_receipt->tick_count = boot_receipt.tick_count;
    out_receipt->title_loaded = boot_receipt.title_loaded;
    snprintf(out_receipt->dungeon_path,
             sizeof(out_receipt->dungeon_path),
             "%s",
             boot_receipt.dungeon_path);
    out_receipt->startup_receipt = boot_receipt.startup_receipt;
    out_receipt->startup_assets = boot_receipt.startup_assets;
    boot_status = boot_receipt.title_loaded
        ? NEXUS_V1_STARTUP_BOOT_STATUS_TITLE
        : NEXUS_V1_STARTUP_BOOT_STATUS_ASSET_ERROR;
    (void)nexus_v1_launcher_startup_boot_status_host_receipt(
        boot_status,
        &out_receipt->boot_status_receipt);
    out_receipt->boot_log_line = boot_receipt.title_loaded
        ? "T0: NEXUS TITLE LOADED"
        : "T0: NEXUS STARTUP ASSETS MISSING";
    return 1;
}

int nexus_v1_launcher_resume_from_save_path(
    const char *save_path,
    Nexus_V1_LightRuntime *light_runtime,
    Nexus_V1_LauncherResumeReceipt *out_receipt)
{
    Nexus_V1_SaveHeader header;
    Nexus_V1_ChampionPool champions;
    Nexus_V1_World world;
    Nexus_V1_Engine *engine;
    Nexus_SaveResult result;
    char nglt_diagnostic[256];
    int level;

    nexus_v1_launcher_resume_receipt_clear(out_receipt);
    if (!save_path || !save_path[0] || !out_receipt) {
        return 0;
    }

    memset(&header, 0, sizeof(header));
    memset(&champions, 0, sizeof(champions));
    memset(&world, 0, sizeof(world));
    memset(nglt_diagnostic, 0, sizeof(nglt_diagnostic));

    result = nexus_v1_load_full_from_path_with_runtime(
        save_path,
        &header,
        &champions,
        &world,
        light_runtime,
        &out_receipt->nglt_decoded,
        nglt_diagnostic,
        sizeof(nglt_diagnostic),
        out_receipt->diagnostic,
        sizeof(out_receipt->diagnostic));
    if (result != NEXUS_SAVE_OK) {
        (void)nexus_v1_startup_resume_status_host_receipt(
            NEXUS_V1_STARTUP_RESUME_STATUS_FAILED,
            &out_receipt->host_receipt);
        if (!out_receipt->diagnostic[0]) {
            snprintf(out_receipt->diagnostic,
                     sizeof(out_receipt->diagnostic),
                     "%s",
                     nexus_v1_save_strerror(result));
        }
        return 0;
    }

    level = world.party_level;
    if (level < 0 || level > 15) {
        level = header.current_level;
    }
    if (level < 0 || level > 15) {
        (void)nexus_v1_startup_resume_status_host_receipt(
            NEXUS_V1_STARTUP_RESUME_STATUS_LEVEL_INVALID,
            &out_receipt->host_receipt);
        snprintf(out_receipt->diagnostic,
                 sizeof(out_receipt->diagnostic),
                 "bad level %d",
                 level);
        return 0;
    }
    if (world.party_dir < 0 || world.party_dir > 3) {
        (void)nexus_v1_startup_resume_status_host_receipt(
            NEXUS_V1_STARTUP_RESUME_STATUS_DIR_INVALID,
            &out_receipt->host_receipt);
        snprintf(out_receipt->diagnostic,
                 sizeof(out_receipt->diagnostic),
                 "bad dir %d",
                 world.party_dir);
        return 0;
    }

    if (nexus_v1_launcher_load_level(level) != 0) {
        (void)nexus_v1_startup_resume_status_host_receipt(
            NEXUS_V1_STARTUP_RESUME_STATUS_LEVEL_ERROR,
            &out_receipt->host_receipt);
        snprintf(out_receipt->diagnostic,
                 sizeof(out_receipt->diagnostic),
                 "level %d load failed",
                 level);
        return 0;
    }

    engine = nexus_v1_launcher_get_engine();
    if (!engine) {
        (void)nexus_v1_startup_resume_status_host_receipt(
            NEXUS_V1_STARTUP_RESUME_STATUS_ENGINE_LOST,
            &out_receipt->host_receipt);
        snprintf(out_receipt->diagnostic,
                 sizeof(out_receipt->diagnostic),
                 "engine lost");
        return 0;
    }

    engine->champions = champions;
    nexus_v1_sync_dgn_runtime_pose(engine, level, world.party_x,
                                   world.party_y, world.party_dir);
    engine->game.tick_count = (int)header.game_time;
    if (engine->mechanics) {
        engine->mechanics->map_index = level;
        engine->mechanics->party_x = world.party_x;
        engine->mechanics->party_y = world.party_y;
        engine->mechanics->party_dir = world.party_dir;
        engine->mechanics->total_ticks = header.game_time;
        engine->mechanics->pending_level_change = -1;
        engine->mechanics->pending_teleport = 0;
        engine->mechanics->input_head = 0;
        engine->mechanics->input_tail = 0;
        engine->mechanics->input_count = 0;
    }

    out_receipt->engine = engine;
    out_receipt->resumed = 1;
    out_receipt->level_loaded = engine->level_loaded;
    out_receipt->party_x = engine->game.party_x;
    out_receipt->party_y = engine->game.party_y;
    out_receipt->party_dir = engine->game.party_dir;
    out_receipt->tick_count = engine->game.tick_count;
    snprintf(out_receipt->dungeon_path,
             sizeof(out_receipt->dungeon_path),
             "%s/LEV%02d.DGN",
             engine->data_dir,
             level);
    (void)nexus_v1_startup_resume_status_host_receipt(
        NEXUS_V1_STARTUP_RESUME_STATUS_RESUMED,
        &out_receipt->host_receipt);
    out_receipt->log_line = out_receipt->nglt_decoded
        ? "T0: NEXUS RESUMED + LIGHT RUNTIME"
        : "T0: NEXUS RESUMED";
    return 1;
}

void nexus_v1_launcher_shutdown(void) {
    if (!s_initialized) {
        return;
    }
    nexus_v1_shutdown(&s_engine);
    s_initialized = 0;
    printf("Nexus launcher: shut down\n");
}
