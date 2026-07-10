#include "csb_v1_boot.h"

#include "memory_frontend_pc34_compat.h"
#include "memory_graphics_dat_pc34_compat.h"
#include "memory_graphics_dat_select_pc34_compat.h"
#include "memory_graphics_dat_state_pc34_compat.h"

#include <stdlib.h>
#include <string.h>

/* ReDMCSB TITLE.C F0437 lines 424-463 loads C001 once and uses C424-C426
 * zones. ENTRANCE.C F0806 lines 775-826 builds door opening frames from
 * C002/C003. CSBWin Graphics.cpp ReadGraphic is the matching PC archive
 * boundary. */

#define CSB_V1_STARTUP_SURFACE_MAX_PIXELS_PC34 (1024u * 1024u)

static int csb_v1_startup_surface_load_graphic_pc34(
    const char *path, unsigned int graphic_index,
    unsigned char **out_pixels, int *out_width, int *out_height)
{
    struct MemoryGraphicsDatState_Compat file_state;
    struct MemoryGraphicsDatRuntimeState_Compat runtime_state;
    struct MemoryGraphicsDatHeader_Compat header;
    struct MemoryGraphicsDatSelection_Compat selection;
    unsigned char *compressed = NULL;
    unsigned char *packed_storage = NULL;
    unsigned char *pixels = NULL;
    size_t packed_stride;
    size_t packed_size;
    size_t pixel_count;
    int ok = 0;
    int x;
    int y;

    if (out_pixels) *out_pixels = NULL;
    if (out_width) *out_width = 0;
    if (out_height) *out_height = 0;
    if (!path || !path[0] || !out_pixels || !out_width || !out_height) return 0;
    memset(&file_state, 0, sizeof(file_state));
    memset(&runtime_state, 0, sizeof(runtime_state));
    memset(&header, 0, sizeof(header));
    memset(&selection, 0, sizeof(selection));
    if (!F0479_MEMORY_InitializeGraphicsDatState_Compat(
            path, &file_state, &runtime_state) ||
        !F0477_MEMORY_OpenGraphicsDat_CPSDF_Compat(path, &file_state)) goto done;
    header.format = runtime_state.format;
    header.graphicCount = runtime_state.graphicCount;
    header.compressedByteCounts = runtime_state.compressedByteCounts;
    header.decompressedByteCounts = runtime_state.decompressedByteCounts;
    header.widthHeight = runtime_state.widthHeight;
    header.fileSize = runtime_state.fileSize;
    if (!F0490_MEMORY_SelectGraphicFromHeader_Compat(
            &header, graphic_index, &selection) ||
        selection.widthHeight.Width == 0 || selection.widthHeight.Height == 0) goto done;
    pixel_count = (size_t)selection.widthHeight.Width * selection.widthHeight.Height;
    packed_stride = (((size_t)selection.widthHeight.Width + 1u) & ~1u) / 2u;
    packed_size = packed_stride * selection.widthHeight.Height;
    if (pixel_count == 0u || pixel_count > CSB_V1_STARTUP_SURFACE_MAX_PIXELS_PC34 ||
        packed_size > CSB_V1_STARTUP_SURFACE_MAX_PIXELS_PC34 ||
        selection.compressedByteCount == 0u) goto done;
    compressed = (unsigned char *)calloc((size_t)selection.compressedByteCount + 16u, 1u);
    packed_storage = (unsigned char *)calloc(packed_size + 4u + 4096u, 1u);
    pixels = (unsigned char *)malloc(pixel_count);
    if (!compressed || !packed_storage || !pixels ||
        !F0474_MEMORY_LoadGraphic_CPSDF_Compat(selection.offset,
            selection.compressedByteCount, &file_state, compressed)) goto done;
    F0488_MEMORY_ExpandGraphicToBitmap_Compat(compressed, packed_storage + 4u,
                                               &selection.widthHeight);
    for (y = 0; y < (int)selection.widthHeight.Height; ++y) {
        for (x = 0; x < (int)selection.widthHeight.Width; ++x) {
            unsigned char packed = packed_storage[4u + (size_t)y * packed_stride +
                                                   (size_t)x / 2u];
            pixels[(size_t)y * selection.widthHeight.Width + (size_t)x] =
                (x & 1) ? (packed & 0x0fu) : ((packed >> 4) & 0x0fu);
        }
    }
    *out_pixels = pixels;
    *out_width = selection.widthHeight.Width;
    *out_height = selection.widthHeight.Height;
    pixels = NULL;
    ok = 1;
done:
    free(pixels);
    free(packed_storage);
    free(compressed);
    F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(&file_state);
    F0479_MEMORY_FreeGraphicsDatState_Compat(&runtime_state);
    return ok;
}

static int csb_v1_startup_surface_crop_pc34(
    CSB_V1_StartupRuntimeSurface_PC34 *out, const unsigned char *source,
    int source_width, int source_height, int asset_id, int source_x,
    int source_y, int width, int height, int transparent_color)
{
    unsigned char *pixels;
    int y;
    if (!out || !source || source_x < 0 || source_y < 0 || width <= 0 || height <= 0 ||
        source_x + width > source_width || source_y + height > source_height ||
        (size_t)width * height > CSB_V1_STARTUP_SURFACE_MAX_PIXELS_PC34) return 0;
    pixels = (unsigned char *)malloc((size_t)width * height);
    if (!pixels) return 0;
    for (y = 0; y < height; ++y)
        memcpy(pixels + (size_t)y * width,
               source + (size_t)(source_y + y) * source_width + source_x, (size_t)width);
    out->pixels = pixels;
    out->width = width;
    out->height = height;
    out->source_asset_id = asset_id;
    out->source_x = source_x;
    out->source_y = source_y;
    out->transparent_color = transparent_color;
    out->valid = 1;
    return 1;
}

void csb_v1_boot_startup_runtime_surface_set_release_pc34(
    CSB_V1_StartupRuntimeSurfaceSet_PC34 *surfaces)
{
    int i;
    if (!surfaces) return;
    for (i = 0; i < CSB_V1_STARTUP_RUNTIME_SURFACE_COUNT_PC34; ++i)
        free(surfaces->surfaces[i].pixels);
    memset(surfaces, 0, sizeof(*surfaces));
}

void csb_v1_boot_startup_runtime_asset_session_init_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session)
{
    if (session) memset(session, 0, sizeof(*session));
}

static int csb_v1_startup_session_load_surface_pc34(
    const CSB_V1_StartupAssetBinding_PC34 *binding,
    CSB_V1_StartupRuntimeSurface_PC34 *surface)
{
    unsigned char *pixels = NULL;
    int width = 0;
    int height = 0;

    if (!binding || !surface || !binding->verified ||
        binding->source != CSB_V1_STARTUP_ASSET_SOURCE_GRAPHICS_DAT_PC34 ||
        binding->graphic_index == 0u || !binding->path[0] ||
        !csb_v1_startup_surface_load_graphic_pc34(
            binding->path, binding->graphic_index, &pixels, &width, &height)) {
        return 0;
    }
    surface->pixels = pixels;
    surface->width = width;
    surface->height = height;
    surface->source_asset_id = (int)binding->graphic_index;
    surface->transparent_color = -1;
    surface->valid = 1;
    return 1;
}

int csb_v1_boot_startup_runtime_asset_session_open_pc34(
    const CSB_V1_BootProfile *profile,
    CSB_V1_StartupRuntimeAssetSession_PC34 *out_session)
{
    const CSB_V1_StartupAssetBinding_PC34 *title;
    const CSB_V1_StartupAssetBinding_PC34 *left;
    const CSB_V1_StartupAssetBinding_PC34 *right;
    const CSB_V1_StartupAssetBinding_PC34 *screen;
    const CSB_V1_StartupAssetBinding_PC34 *credits;
    const CSB_V1_StartupAssetBinding_PC34 *inventory;
    const CSB_V1_StartupAssetBinding_PC34 *resurrect;
    CSB_V1_StartupRuntimeSurfaceSet_PC34 *surfaces;

    if (!out_session) return 0;
    csb_v1_boot_startup_runtime_asset_session_init_pc34(out_session);
    if (!profile || !profile->assets_verified || !profile->graphics_verified ||
        !profile->startup_assets.real_graphics_available) return 0;
    title = csb_v1_boot_startup_asset_binding_pc34(
        profile, CSB_V1_STARTUP_ASSET_ROLE_TITLE_PRESENTS_PC34);
    left = csb_v1_boot_startup_asset_binding_pc34(
        profile, CSB_V1_STARTUP_ASSET_ROLE_ENTRANCE_LEFT_DOOR_PC34);
    right = csb_v1_boot_startup_asset_binding_pc34(
        profile, CSB_V1_STARTUP_ASSET_ROLE_ENTRANCE_RIGHT_DOOR_PC34);
    screen = csb_v1_boot_startup_asset_binding_pc34(
        profile, CSB_V1_STARTUP_ASSET_ROLE_ENTRANCE_SCREEN_PC34);
    credits = csb_v1_boot_startup_asset_binding_pc34(
        profile, CSB_V1_STARTUP_ASSET_ROLE_ENTRANCE_CREDITS_PC34);
    inventory = csb_v1_boot_startup_asset_binding_pc34(
        profile, CSB_V1_STARTUP_ASSET_ROLE_HUD_INVENTORY_PC34);
    resurrect = csb_v1_boot_startup_asset_binding_pc34(
        profile, CSB_V1_STARTUP_ASSET_ROLE_HUD_RESURRECT_PC34);
    surfaces = &out_session->surfaces;

    /* ReDMCSB TITLE.C F0437:424-463 loads C001 once for all title zones.
     * ENTRANCE.C F0806:721-778 keeps C002-C005 for the entrance loop.
     * CSBWin Graphics.cpp ReadGraphic:1717-1755 is the archive-read
     * boundary, so consumers only receive stable decoded pixels here. */
    if (!csb_v1_startup_session_load_surface_pc34(
            title, &surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34]) ||
        !csb_v1_startup_surface_crop_pc34(
            &surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_PRESENTS_PC34],
            surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34].pixels,
            surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34].width,
            surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34].height,
            1, 0, 137, 320, 16, -1) ||
        !csb_v1_startup_surface_crop_pc34(
            &surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_CHAOS_PC34],
            surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34].pixels,
            surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34].width,
            surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34].height,
            1, 0, 0, 320, 80, -1) ||
        !csb_v1_startup_surface_crop_pc34(
            &surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_STRIKES_BACK_PC34],
            surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34].pixels,
            surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34].width,
            surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34].height,
            1, 0, 80, 320, 57, 0) ||
        !csb_v1_startup_session_load_surface_pc34(
            left, &surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34]) ||
        !csb_v1_startup_session_load_surface_pc34(
            right, &surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34]) ||
        !csb_v1_startup_session_load_surface_pc34(
            screen, &surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34]) ||
        !csb_v1_startup_session_load_surface_pc34(
            credits, &surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_CREDITS_PC34])) {
        csb_v1_boot_startup_runtime_asset_session_release_pc34(out_session);
        return 0;
    }
    surfaces->title_regions_ready = 1;
    surfaces->opening_frame_ready = 1;
    surfaces->entrance_screen_ready = 1;
    surfaces->real_asset_matched = 1;
    surfaces->valid = 1;
    out_session->hud_inventory_binding = inventory ? *inventory : (CSB_V1_StartupAssetBinding_PC34){0};
    out_session->hud_resurrect_binding = resurrect ? *resurrect : (CSB_V1_StartupAssetBinding_PC34){0};
    out_session->title_assets_ready = 1;
    out_session->title_presents_ready = surfaces->surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_PRESENTS_PC34].valid;
    out_session->title_chaos_ready = surfaces->surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_CHAOS_PC34].valid;
    out_session->title_strikes_back_ready = surfaces->surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_STRIKES_BACK_PC34].valid;
    out_session->entrance_assets_ready = 1;
    out_session->door_assets_ready = surfaces->surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34].valid &&
        surfaces->surfaces[
            CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34].valid;
    out_session->hud_assets_bound = inventory && resurrect && inventory->verified &&
        resurrect->verified && inventory->source != CSB_V1_STARTUP_ASSET_SOURCE_FALLBACK_PC34 &&
        resurrect->source != CSB_V1_STARTUP_ASSET_SOURCE_FALLBACK_PC34;
    out_session->full_startup_ready =
        out_session->title_presents_ready && out_session->title_chaos_ready &&
        out_session->title_strikes_back_ready && out_session->entrance_assets_ready &&
        out_session->door_assets_ready && out_session->hud_assets_bound;
    out_session->rejects_legacy_wrappers = out_session->full_startup_ready;
    out_session->real_asset_matched = 1;
    out_session->generation = 1u;
    out_session->valid = out_session->full_startup_ready;
    if (!out_session->valid) {
        csb_v1_boot_startup_runtime_asset_session_release_pc34(out_session);
    }
    return out_session->valid;
}

void csb_v1_boot_startup_runtime_asset_session_release_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session)
{
    if (!session) return;
    csb_v1_boot_startup_runtime_surface_set_release_pc34(&session->surfaces);
    memset(session, 0, sizeof(*session));
}

int csb_v1_boot_startup_runtime_asset_session_frame_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    uint32_t source_tick,
    CSB_V1_StartupRuntimeAssetFrame_PC34 *out_frame)
{
    if (out_frame) memset(out_frame, 0, sizeof(*out_frame));
    if (!session || !plan || !out_frame || !session->valid ||
        !session->surfaces.valid || !session->full_startup_ready) return 0;
    session->source_tick = source_tick;
    out_frame->source_tick = source_tick;
    out_frame->session_generation = session->generation;
    out_frame->stage = (CSB_V1_StartupStage_PC34)plan->title_stage;
    out_frame->opening_step = plan->opening_door_step;
    out_frame->uses_verified_hud_bindings = session->hud_assets_bound;
    out_frame->left_door_surface = &session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34];
    out_frame->right_door_surface = &session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34];
    if (plan->surface == CSB_V1_STARTUP_RENDER_TITLE_PC34) {
        out_frame->title_phase_tick = plan->title_source_step;
        out_frame->title_phase_tick_count = csb_v1_startup_title_total_ticks_pc34();
        if (plan->title_stage == CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34)
            out_frame->title_surface = &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_PRESENTS_PC34];
        else if (plan->title_stage == CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34)
            out_frame->title_surface = &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_CHAOS_PC34];
        else
            out_frame->title_surface = &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_STRIKES_BACK_PC34];
    } else if (plan->surface == CSB_V1_STARTUP_RENDER_ENTRANCE_CREDITS_PC34) {
        out_frame->entrance_surface = &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_CREDITS_PC34];
    } else if (plan->surface != CSB_V1_STARTUP_RENDER_NONE_PC34 &&
               plan->surface != CSB_V1_STARTUP_RENDER_ENTRANCE_BLACK_PC34) {
        out_frame->entrance_surface = &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34];
    }
    out_frame->valid = (out_frame->title_surface || out_frame->entrance_surface) &&
        out_frame->left_door_surface->valid && out_frame->right_door_surface->valid;
    return out_frame->valid;
}

int csb_v1_boot_startup_runtime_surfaces_materialize_pc34(
    const CSB_V1_BootProfile *profile, const CSB_V1_StartupRenderPlan_PC34 *plan,
    CSB_V1_StartupRuntimeSurfaceSet_PC34 *out)
{
    CSB_V1_StartupRuntimeAssetSession_PC34 session;
    CSB_V1_StartupRuntimeAssetFrame_PC34 frame;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!profile || !plan || !profile->assets_verified || !profile->graphics_verified ||
        !profile->startup_assets.real_graphics_available ||
        !csb_v1_boot_startup_render_plan_uses_real_assets_pc34(profile, plan)) return 0;
    csb_v1_boot_startup_runtime_asset_session_init_pc34(&session);
    if (!csb_v1_boot_startup_runtime_asset_session_open_pc34(profile, &session) ||
        !csb_v1_boot_startup_runtime_asset_session_frame_pc34(
            &session, plan, 0u, &frame)) {
        csb_v1_boot_startup_runtime_asset_session_release_pc34(&session);
        return 0;
    }
    *out = session.surfaces;
    memset(&session.surfaces, 0, sizeof(session.surfaces));
    csb_v1_boot_startup_runtime_asset_session_release_pc34(&session);
    return out->valid;
}

int csb_v1_boot_startup_full_runtime_receipt_from_session_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    CSB_V1_StartupFullRuntimeReceipt_PC34 *out_receipt)
{
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!session || !session->valid || !session->surfaces.valid) return 0;
    out_receipt->real_asset_matched = session->real_asset_matched;
    out_receipt->title_presents_ready = session->title_presents_ready;
    out_receipt->title_chaos_ready = session->title_chaos_ready;
    out_receipt->title_strikes_back_ready = session->title_strikes_back_ready;
    out_receipt->title_sequence_ready =
        out_receipt->title_presents_ready && out_receipt->title_chaos_ready &&
        out_receipt->title_strikes_back_ready;
    out_receipt->entrance_ready = session->entrance_assets_ready;
    out_receipt->hud_ready = session->hud_assets_bound;
    out_receipt->door_ready = session->door_assets_ready;
    out_receipt->no_legacy_wrappers = session->rejects_legacy_wrappers;
    out_receipt->session_generation = session->generation;
    out_receipt->source_evidence =
        "ReDMCSB TITLE.C F0437; ENTRANCE.C F0806; CSBWin Graphics.cpp ReadGraphic";
    out_receipt->valid =
        out_receipt->real_asset_matched && out_receipt->title_sequence_ready &&
        out_receipt->entrance_ready && out_receipt->hud_ready &&
        out_receipt->door_ready && out_receipt->no_legacy_wrappers;
    return out_receipt->valid;
}

void csb_v1_boot_startup_complete_support_receipt_init_pc34(
    CSB_V1_StartupCompleteSupportReceipt_PC34 *receipt)
{
    if (!receipt) return;
    memset(receipt, 0, sizeof(*receipt));
    receipt->source_evidence =
        "ReDMCSB TITLE.C F0437; ENTRANCE.C F0806/F0807; "
        "CSBWin Graphics.cpp ReadGraphic and startup host loop";
}

void csb_v1_boot_startup_release_app_capture_receipt_init_pc34(
    CSB_V1_StartupReleaseAppCaptureReceipt_PC34 *receipt)
{
    if (!receipt) return;
    memset(receipt, 0, sizeof(*receipt));
    csb_v1_boot_startup_complete_support_receipt_init_pc34(
        &receipt->complete_support);
    receipt->source_evidence =
        "ReDMCSB TITLE.C F0437 lines 424-463; "
        "ENTRANCE.C F0441/F0806 lines 620-700; "
        "ENTRANCE.C F0580/F0581 lines 1123-1165";
}

int csb_v1_boot_startup_release_app_capture_receipt_from_complete_support_pc34(
    const CSB_V1_StartupCompleteSupportReceipt_PC34 *complete_support,
    CSB_V1_StartupReleaseAppCaptureReceipt_PC34 *out_receipt)
{
    const CSB_V1_BootStartupRuntimeHostCaptureGateReceipt_PC34 *host_gate;
    uint32_t hash = 2166136261u;

    if (!out_receipt) return 0;
    csb_v1_boot_startup_release_app_capture_receipt_init_pc34(out_receipt);
    if (!complete_support) return 0;

    host_gate = &complete_support->host_capture_gate;
    out_receipt->complete_support = *complete_support;
    out_receipt->complete_support_valid = complete_support->valid ? 1 : 0;
    out_receipt->host_capture_gate_valid =
        complete_support->host_capture_gate_valid && host_gate->valid ? 1 : 0;
    out_receipt->title_phase_route_complete =
        complete_support->title_phase_route_complete &&
                host_gate->title_runtime_phase_route_complete
            ? 1
            : 0;
    out_receipt->runtime_host_routes_ready =
        complete_support->runtime_host_routes_ready &&
                host_gate->all_runtime_routes_consumed
            ? 1
            : 0;
    out_receipt->draw_consumes_receipt_only =
        complete_support->draw_consumes_receipt_only &&
                host_gate->draw_consumes_receipt_only
            ? 1
            : 0;
    out_receipt->input_consumes_receipt_only =
        complete_support->input_consumes_receipt_only &&
                host_gate->input_consumes_receipt_only
            ? 1
            : 0;
    out_receipt->no_fallback_callbacks =
        complete_support->no_fallback_callbacks &&
                host_gate->no_fallback_callbacks
            ? 1
            : 0;
    out_receipt->no_wrapper_fallback_routes =
        complete_support->no_wrapper_fallback_routes &&
                host_gate->no_wrapper_fallback_routes
            ? 1
            : 0;
    out_receipt->host_route_wrappers_retired =
        complete_support->host_route_wrappers_retired &&
                host_gate->host_route_wrappers_retired
            ? 1
            : 0;
    out_receipt->no_loose_render_plan_exports =
        complete_support->no_loose_render_plan_exports &&
                host_gate->no_loose_render_plan_exports
            ? 1
            : 0;
    out_receipt->real_startup_assets_bound =
        complete_support->real_startup_assets_bound &&
                host_gate->real_startup_assets_bound
            ? 1
            : 0;
    out_receipt->title_packaged_capture_hash =
        host_gate->title_packaged_capture_hash;
    out_receipt->closed_door_packaged_capture_hash =
        host_gate->closed_door_packaged_capture_hash;
    out_receipt->utility_packaged_capture_hash =
        host_gate->utility_packaged_capture_hash;
    out_receipt->door_opening_packaged_capture_hash =
        host_gate->door_opening_packaged_capture_hash;
    out_receipt->title_release_app_capture_ready =
        host_gate->title_runtime_captured &&
                host_gate->title_packaged_capture_hash != 0u
            ? 1
            : 0;
    out_receipt->closed_door_release_app_capture_ready =
        host_gate->closed_door_hud_runtime_captured &&
                host_gate->closed_door_packaged_capture_hash != 0u
            ? 1
            : 0;
    out_receipt->utility_release_app_capture_ready =
        host_gate->utility_hud_runtime_captured &&
                host_gate->utility_packaged_capture_hash != 0u
            ? 1
            : 0;
    out_receipt->door_opening_release_app_capture_ready =
        host_gate->door_opening_runtime_captured &&
                host_gate->door_opening_packaged_capture_hash != 0u
            ? 1
            : 0;
    out_receipt->title_host_consumer_ready =
        host_gate->title_host_ownership_valid &&
                host_gate->title_host_draw_consumes_receipt_only &&
                host_gate->title_host_input_consumes_receipt_only &&
                host_gate->title_packaged_capture_hash != 0u
            ? 1
            : 0;
    out_receipt->closed_door_host_consumer_ready =
        host_gate->closed_door_host_ownership_valid &&
                host_gate->closed_door_host_draw_consumes_receipt_only &&
                host_gate->closed_door_host_input_consumes_receipt_only &&
                host_gate->closed_door_packaged_capture_hash != 0u
            ? 1
            : 0;
    out_receipt->utility_host_consumer_ready =
        host_gate->utility_host_ownership_valid &&
                host_gate->utility_host_draw_consumes_receipt_only &&
                host_gate->utility_host_input_consumes_receipt_only &&
                host_gate->utility_packaged_capture_hash != 0u
            ? 1
            : 0;
    out_receipt->door_opening_host_consumer_ready =
        host_gate->door_opening_host_ownership_valid &&
                host_gate->door_opening_host_draw_consumes_receipt_only &&
                host_gate->door_opening_host_input_consumes_receipt_only &&
                host_gate->door_opening_packaged_capture_hash != 0u
            ? 1
            : 0;
    out_receipt->route_specific_host_consumers_ready =
        out_receipt->title_host_consumer_ready &&
                out_receipt->closed_door_host_consumer_ready &&
                out_receipt->utility_host_consumer_ready &&
                out_receipt->door_opening_host_consumer_ready
            ? 1
            : 0;
    out_receipt->runtime_host_gate_hash =
        complete_support->runtime_host_gate_hash;
    out_receipt->complete_support_hash =
        complete_support->complete_support_hash;

    hash ^= out_receipt->runtime_host_gate_hash;
    hash *= 16777619u;
    hash ^= out_receipt->complete_support_hash;
    hash *= 16777619u;
    hash ^= out_receipt->title_packaged_capture_hash;
    hash *= 16777619u;
    hash ^= out_receipt->closed_door_packaged_capture_hash;
    hash *= 16777619u;
    hash ^= out_receipt->utility_packaged_capture_hash;
    hash *= 16777619u;
    hash ^= out_receipt->door_opening_packaged_capture_hash;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->title_phase_route_complete;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->runtime_host_routes_ready;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->host_route_wrappers_retired;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->no_loose_render_plan_exports;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->route_specific_host_consumers_ready;
    out_receipt->release_app_capture_hash = hash ? hash : 1u;
    out_receipt->release_app_capture_ready =
        out_receipt->complete_support_valid &&
                out_receipt->host_capture_gate_valid &&
                out_receipt->title_release_app_capture_ready &&
                out_receipt->closed_door_release_app_capture_ready &&
                out_receipt->utility_release_app_capture_ready &&
                out_receipt->door_opening_release_app_capture_ready &&
                out_receipt->title_phase_route_complete &&
                out_receipt->runtime_host_routes_ready &&
                out_receipt->draw_consumes_receipt_only &&
                out_receipt->input_consumes_receipt_only &&
                out_receipt->route_specific_host_consumers_ready &&
                out_receipt->no_fallback_callbacks &&
                out_receipt->no_wrapper_fallback_routes &&
                out_receipt->host_route_wrappers_retired &&
                out_receipt->no_loose_render_plan_exports &&
                out_receipt->real_startup_assets_bound &&
                out_receipt->release_app_capture_hash != 0u
            ? 1
            : 0;
    out_receipt->valid = out_receipt->release_app_capture_ready;
    /* ReDMCSB keeps title, entrance HUD and opening-door presentation under
     * TITLE.C F0437 lines 424-463, ENTRANCE.C F0441 lines 620-950 and
     * F0580/F0581 lines 1123-1165.  DUNVIEW.C wall/door tables around
     * lines 150-240 keep door/HUD ownership data-driven, so release capture
     * now requires each host route to consume its own receipt-owned package. */
    return out_receipt->valid;
}

int csb_v1_boot_startup_complete_support_receipt_from_runtime_and_host_pc34(
    const CSB_V1_StartupFullRuntimeReceipt_PC34 *full_runtime,
    const CSB_V1_BootStartupRuntimeHostCaptureGateReceipt_PC34 *host_capture_gate,
    CSB_V1_StartupCompleteSupportReceipt_PC34 *out_receipt)
{
    uint32_t hash = 2166136261u;

    if (!out_receipt) return 0;
    csb_v1_boot_startup_complete_support_receipt_init_pc34(out_receipt);
    if (!full_runtime || !host_capture_gate) return 0;

    out_receipt->full_runtime = *full_runtime;
    out_receipt->host_capture_gate = *host_capture_gate;
    out_receipt->full_runtime_valid = full_runtime->valid ? 1 : 0;
    out_receipt->host_capture_gate_valid = host_capture_gate->valid ? 1 : 0;
    out_receipt->real_asset_matched =
        full_runtime->real_asset_matched &&
                host_capture_gate->runtime_visual.real_asset_matched
            ? 1
            : 0;
    out_receipt->title_sequence_ready =
        full_runtime->title_sequence_ready &&
                host_capture_gate->title_runtime_phase_route_complete
            ? 1
            : 0;
    out_receipt->title_phase_route_complete =
        host_capture_gate->title_runtime_phase_route_complete ? 1 : 0;
    out_receipt->title_presents_ready =
        full_runtime->title_presents_ready &&
                host_capture_gate->title_presents_runtime_captured
            ? 1
            : 0;
    out_receipt->title_chaos_ready =
        full_runtime->title_chaos_ready &&
                host_capture_gate->title_chaos_zoom_runtime_captured &&
                host_capture_gate->title_chaos_hold_runtime_captured
            ? 1
            : 0;
    out_receipt->title_strikes_back_ready =
        full_runtime->title_strikes_back_ready &&
                host_capture_gate->title_strikes_back_runtime_captured
            ? 1
            : 0;
    out_receipt->entrance_ready =
        full_runtime->entrance_ready &&
                host_capture_gate->credits_runtime_captured
            ? 1
            : 0;
    out_receipt->hud_ready =
        full_runtime->hud_ready &&
                host_capture_gate->closed_door_hud_runtime_captured &&
                host_capture_gate->utility_hud_runtime_captured
            ? 1
            : 0;
    out_receipt->door_ready =
        full_runtime->door_ready &&
                host_capture_gate->door_opening_runtime_captured
            ? 1
            : 0;
    out_receipt->runtime_host_routes_ready =
        host_capture_gate->route_hardening_valid &&
                host_capture_gate->all_runtime_routes_consumed &&
                host_capture_gate->title_host_ownership_valid &&
                host_capture_gate->closed_door_host_ownership_valid &&
                host_capture_gate->utility_host_ownership_valid &&
                host_capture_gate->door_opening_host_ownership_valid
            ? 1
            : 0;
    out_receipt->draw_consumes_receipt_only =
        host_capture_gate->draw_consumes_receipt_only ? 1 : 0;
    out_receipt->input_consumes_receipt_only =
        host_capture_gate->input_consumes_receipt_only ? 1 : 0;
    out_receipt->no_legacy_wrappers =
        full_runtime->no_legacy_wrappers &&
                host_capture_gate->no_wrapper_fallback_routes
            ? 1
            : 0;
    out_receipt->no_fallback_callbacks =
        host_capture_gate->no_fallback_callbacks ? 1 : 0;
    out_receipt->no_wrapper_fallback_routes =
        host_capture_gate->no_wrapper_fallback_routes ? 1 : 0;
    out_receipt->host_route_wrappers_retired =
        host_capture_gate->host_route_wrappers_retired ? 1 : 0;
    out_receipt->no_loose_render_plan_exports =
        host_capture_gate->no_loose_render_plan_exports ? 1 : 0;
    out_receipt->real_startup_assets_bound =
        host_capture_gate->real_startup_assets_bound ? 1 : 0;
    out_receipt->real_startup_asset_binding_hash =
        host_capture_gate->real_startup_asset_binding_hash;
    out_receipt->session_generation = full_runtime->session_generation;
    out_receipt->runtime_host_gate_hash =
        host_capture_gate->runtime_host_gate_hash;
    hash ^= full_runtime->session_generation;
    hash *= 16777619u;
    hash ^= host_capture_gate->runtime_host_gate_hash;
    hash *= 16777619u;
    hash ^= host_capture_gate->title_runtime_phase_hash;
    hash *= 16777619u;
    hash ^= host_capture_gate->runtime_capture_hash;
    hash *= 16777619u;
    hash ^= (uint32_t)host_capture_gate->host_route_wrappers_retired;
    hash *= 16777619u;
    hash ^= (uint32_t)host_capture_gate->no_loose_render_plan_exports;
    hash *= 16777619u;
    hash ^= (uint32_t)host_capture_gate->real_startup_assets_bound;
    hash *= 16777619u;
    hash ^= host_capture_gate->real_startup_asset_binding_hash;
    out_receipt->complete_support_hash = hash ? hash : 1u;
    out_receipt->valid =
        out_receipt->full_runtime_valid &&
                out_receipt->host_capture_gate_valid &&
                out_receipt->real_asset_matched &&
                out_receipt->title_sequence_ready &&
                out_receipt->title_presents_ready &&
                out_receipt->title_chaos_ready &&
                out_receipt->title_strikes_back_ready &&
                out_receipt->entrance_ready &&
                out_receipt->hud_ready &&
                out_receipt->door_ready &&
                out_receipt->runtime_host_routes_ready &&
                out_receipt->draw_consumes_receipt_only &&
                out_receipt->input_consumes_receipt_only &&
                out_receipt->no_legacy_wrappers &&
                out_receipt->no_fallback_callbacks &&
                out_receipt->no_wrapper_fallback_routes &&
                out_receipt->host_route_wrappers_retired &&
                out_receipt->no_loose_render_plan_exports &&
                out_receipt->real_startup_assets_bound &&
                out_receipt->real_startup_asset_binding_hash != 0u &&
                out_receipt->complete_support_hash != 0u
            ? 1
            : 0;
    return out_receipt->valid;
}
