#include "csb_v1_boot.h"
#include "csb_v1_f0347_inventory_draw_panel_pc34_compat.h"

#include "memory_frontend_pc34_compat.h"
#include "memory_graphics_dat_pc34_compat.h"
#include "memory_graphics_dat_select_pc34_compat.h"
#include "memory_graphics_dat_state_pc34_compat.h"
#include "csb_v1_graphics_lzw_pc34_compat.h"
#include "csb_v1_startup_img3_decode_pc34_compat.h"
#include "vga_palette_pc34_compat.h"

#include <stdlib.h>
#include <string.h>

/* ReDMCSB TITLE.C F0437 lines 424-463 loads C001 once and uses C424-C426
 * zones. ENTRANCE.C F0806 lines 775-826 builds door opening frames from
 * C002/C003. PC3.4's archive/LZW/IMAGE3 path is F0490 then F0488. */

#define CSB_V1_STARTUP_SURFACE_MAX_PIXELS_PC34 (1024u * 1024u)

enum {
    CSB_V1_STARTUP_ENTRANCE_LEFT_DOOR_WIDTH_PC34 = 105,
    CSB_V1_STARTUP_ENTRANCE_RIGHT_DOOR_WIDTH_PC34 = 128,
    CSB_V1_STARTUP_ENTRANCE_DOOR_HEIGHT_PC34 = 161,
    CSB_V1_STARTUP_CSBGRAPHICS_HUD_INVENTORY_PC34 = 17,
    CSB_V1_STARTUP_CSBGRAPHICS_HUD_RESURRECT_PC34 = 40
};

static uint32_t csb_v1_startup_frame_hash_step_pc34(uint32_t hash,
                                                    uint32_t value);

static int csb_v1_startup_hud_capture_surface_matches_pc34(
    const CSB_V1_StartupRuntimeSurface_PC34 *surface,
    int source_asset_id,
    int width,
    int height,
    int transparent_color)
{
    return surface && surface->valid && surface->pixels &&
        surface->source_asset_id == source_asset_id &&
        surface->width == width && surface->height == height &&
        surface->transparent_color == transparent_color;
}

static int csb_v1_startup_graphic_decode_capture_admitted_pc34(
    const CSB_V1_StartupRuntimeSurface_PC34 *surface, int source_asset_id,
    int width, int height)
{
    const CSB_V1_StartupGraphicDecodeReceipt_PC34 *receipt;

    if (!surface || !surface->valid || !surface->pixels ||
        surface->source_asset_id != source_asset_id || surface->width != width ||
        surface->height != height) return 0;
    receipt = &surface->decode_receipt;
    return receipt->valid && receipt->width == (uint16_t)width &&
        receipt->height == (uint16_t)height && receipt->stream_byte_count >= 5U &&
        receipt->stream_bytes_consumed >= 4U &&
        receipt->stream_bytes_consumed <= receipt->stream_byte_count &&
        receipt->emitted_planar_pixels > 0U &&
        receipt->physical_planar_pixels >= (size_t)width * (size_t)height &&
        receipt->stream_fnv1a != 0U && receipt->indexed_pixel_fnv1a != 0U &&
        receipt->ended_at_record_boundary && receipt->indexed_colors_are_4bit;
}

/* ENTRANCE.C F0806 keeps C002, C003 and C004 resident for the whole
 * entrance loop.  A surface can therefore only enter that loop after the
 * CSBWin ExpandGraphic-compatible decoder has accounted for its own record.
 * This deliberately accepts a source-defined blank planar tail: CSBWin's
 * destination page starts cleared and some verified PC records end before
 * their padded four-plane rectangle is full. */
static int csb_v1_startup_entrance_decode_capture_matches_pc34(
    const CSB_V1_StartupRuntimeSurface_PC34 *surface, int source_asset_id,
    int width, int height)
{
    return csb_v1_startup_graphic_decode_capture_admitted_pc34(
        surface, source_asset_id, width, height) &&
        surface->decode_receipt.stream_bytes_consumed ==
            surface->decode_receipt.stream_byte_count;
}

static uint32_t csb_v1_startup_hash_text_pc34(uint32_t hash, const char *text)
{
    const unsigned char *cursor = (const unsigned char *)text;

    if (!cursor) return hash;
    while (*cursor) {
        hash = (hash ^ *cursor++) * 16777619u;
    }
    return hash ? hash : 2166136261u;
}

static int csb_v1_startup_package_geometry_matches_pc34(
    const CSB_V1_StartupRuntimeSurfaceSet_PC34 *surfaces)
{
    if (!surfaces) return 0;

    /* ReDMCSB ENTRANCE.C F0806 places C002/C003 over C004 through the
     * C430/C431 door zones. The source C003 bitmap is 128 px wide and is
     * clipped by its 105 px destination zone; retain that real source
     * geometry instead of accepting an arbitrary replacement strip. */
    return
        csb_v1_startup_entrance_decode_capture_matches_pc34(
            &surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34],
            2, CSB_V1_STARTUP_ENTRANCE_LEFT_DOOR_WIDTH_PC34,
            CSB_V1_STARTUP_ENTRANCE_DOOR_HEIGHT_PC34) &&
        csb_v1_startup_entrance_decode_capture_matches_pc34(
            &surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34],
            3, CSB_V1_STARTUP_ENTRANCE_RIGHT_DOOR_WIDTH_PC34,
            CSB_V1_STARTUP_ENTRANCE_DOOR_HEIGHT_PC34) &&
        csb_v1_startup_entrance_decode_capture_matches_pc34(
            &surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34],
            4, CSB_V1_STARTUP_RUNTIME_RASTER_WIDTH_PC34,
            CSB_V1_STARTUP_RUNTIME_RASTER_HEIGHT_PC34);
}

static int csb_v1_startup_surface_load_graphic_pc34(
    const char *path, unsigned int graphic_index,
    unsigned char **out_pixels, int *out_width, int *out_height,
    CSB_V1_StartupGraphicDecodeReceipt_PC34 *out_decode_receipt)
{
    struct MemoryGraphicsDatState_Compat file_state;
    struct MemoryGraphicsDatRuntimeState_Compat runtime_state;
    struct MemoryGraphicsDatHeader_Compat header;
    struct MemoryGraphicsDatSelection_Compat selection;
    unsigned char *compressed = NULL;
    unsigned char *decompressed = NULL;
    unsigned char *pixels = NULL;
    size_t pixel_count;
    size_t decompressed_size = 0u;
    int ok = 0;

    if (out_pixels) *out_pixels = NULL;
    if (out_width) *out_width = 0;
    if (out_height) *out_height = 0;
    if (out_decode_receipt) {
        memset(out_decode_receipt, 0, sizeof(*out_decode_receipt));
    }
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
    if (pixel_count == 0u || pixel_count > CSB_V1_STARTUP_SURFACE_MAX_PIXELS_PC34 ||
        selection.compressedByteCount == 0u ||
        selection.decompressedByteCount == 0u) goto done;
    compressed = (unsigned char *)calloc((size_t)selection.compressedByteCount + 16u, 1u);
    decompressed = (unsigned char *)calloc((size_t)selection.decompressedByteCount, 1u);
    pixels = (unsigned char *)malloc(pixel_count);
    if (!compressed || !decompressed || !pixels ||
        !F0474_MEMORY_LoadGraphic_CPSDF_Compat(selection.offset,
            selection.compressedByteCount, &file_state, compressed)) goto done;

    /* F0490 first owns the archive/LZW boundary.  C001-C005 then use
     * CSBWin Graphics.cpp::ExpandGraphic's big-endian four-plane stream,
     * not the generic ReDMCSB F0488 packed-nibble route.  The latter made
     * the decoded title and entrance depend on a stale format assumption. */
    if (selection.compressedByteCount == selection.decompressedByteCount) {
        memcpy(decompressed, compressed, (size_t)selection.decompressedByteCount);
    } else if (csb_v1_graphics_lzw_decode_pc34_compat(
                   compressed, (size_t)selection.compressedByteCount,
                   decompressed, (size_t)selection.decompressedByteCount,
                   &decompressed_size) != 0 ||
               decompressed_size != (size_t)selection.decompressedByteCount) {
        goto done;
    }
    if (!csb_v1_startup_img3_decode_to_indexed_with_receipt_pc34_compat(
            decompressed, (size_t)selection.decompressedByteCount,
            selection.widthHeight.Width, selection.widthHeight.Height,
            pixels, pixel_count, out_decode_receipt)) {
        goto done;
    }
    *out_pixels = pixels;
    *out_width = selection.widthHeight.Width;
    *out_height = selection.widthHeight.Height;
    pixels = NULL;
    ok = 1;
done:
    free(pixels);
    free(decompressed);
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

static int csb_v1_startup_session_load_csbgraphics_surface_pc34(
    const CSB_V1_BootProfile *profile,
    const CSB_V1_StartupAssetBinding_PC34 *binding,
    CSB_V1_StartupRuntimeSurface_PC34 *surface)
{
    const CSB_V1_CSBGraphicsRuntimePlanEntry *entry;
    size_t written = 0u;
    size_t pixel_count;
    unsigned char *pixels;

    if (!profile || !binding || !surface || !binding->verified ||
        binding->source != CSB_V1_STARTUP_ASSET_SOURCE_CSBGRAPHICS_DAT_PC34 ||
        !binding->path[0] || !profile->csbgraphics_cache.loaded ||
        !profile->csbgraphics_cache.file_buffer ||
        profile->csbgraphics_cache.file_size == 0u ||
        strlen(profile->csbgraphics_cache.matched_md5) != 32u ||
        strcmp(binding->path, profile->csbgraphics_cache.resolved_path) != 0 ||
        !csb_v1_boot_csbgraphics_palette_receipt_ready(profile) ||
        !profile->csbgraphics_runtime_plan.ready) return 0;

    entry = csb_v1_csbgraphics_runtime_plan_find_entry(
        &profile->csbgraphics_runtime_plan, binding->graphic_index);
    if (!entry || !entry->needs_hud_redraw || entry->expected_width == 0u ||
        entry->expected_height == 0u || entry->decompressed_size == 0u ||
        (binding->graphic_index == CSB_V1_STARTUP_CSBGRAPHICS_HUD_INVENTORY_PC34 &&
         (entry->route != CSB_V1_CSBGRAPHICS_RUNTIME_ROUTE_HUD_INVENTORY ||
          entry->expected_width != CSB_V1_STARTUP_HUD_INVENTORY_WIDTH_PC34 ||
          entry->expected_height != CSB_V1_STARTUP_HUD_INVENTORY_HEIGHT_PC34)) ||
        (binding->graphic_index == CSB_V1_STARTUP_CSBGRAPHICS_HUD_RESURRECT_PC34 &&
         (entry->route != CSB_V1_CSBGRAPHICS_RUNTIME_ROUTE_HUD_RESURRECT_PANEL ||
          entry->expected_width != CSB_V1_STARTUP_HUD_RESURRECT_WIDTH_PC34 ||
          entry->expected_height != CSB_V1_STARTUP_HUD_RESURRECT_HEIGHT_PC34))) {
        return 0;
    }
    pixel_count = (size_t)entry->expected_width * entry->expected_height;
    if (pixel_count != entry->decompressed_size) return 0;
    pixels = (unsigned char *)malloc(pixel_count);
    if (!pixels || csb_v1_csbgraphics_dat_decode_entry(
            profile->csbgraphics_cache.file_buffer,
            profile->csbgraphics_cache.file_size, binding->graphic_index,
            pixels, pixel_count, &written) != CSB_V1_CSBGRAPHICS_CLASSIFY_OK ||
        written != pixel_count) {
        free(pixels);
        return 0;
    }
    surface->pixels = pixels;
    surface->width = entry->expected_width;
    surface->height = entry->expected_height;
    surface->source_asset_id = (int)binding->graphic_index;
    surface->transparent_color = -1;
    surface->valid = 1;
    return 1;
}

static int csb_v1_startup_session_load_surface_pc34(
    const CSB_V1_BootProfile *profile,
    const CSB_V1_StartupAssetBinding_PC34 *binding,
    CSB_V1_StartupRuntimeSurface_PC34 *surface)
{
    unsigned char *pixels = NULL;
    int width = 0;
    int height = 0;

    if (!binding || !surface || !binding->verified ||
        binding->graphic_index == 0u || !binding->path[0]) return 0;
    if (binding->source == CSB_V1_STARTUP_ASSET_SOURCE_CSBGRAPHICS_DAT_PC34) {
        return csb_v1_startup_session_load_csbgraphics_surface_pc34(
            profile, binding, surface);
    }
    if (binding->source != CSB_V1_STARTUP_ASSET_SOURCE_GRAPHICS_DAT_PC34 ||
        !csb_v1_startup_surface_load_graphic_pc34(
            binding->path, binding->graphic_index, &pixels, &width, &height,
            &surface->decode_receipt)) {
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
     * CSBWin Graphics.cpp:1643/1717 resolves a hash-admitted CSBgraphics
     * HUD entry through its parsed index; it cannot become a generic surface
     * or fall back to GRAPHICS.DAT after this binding has been selected. */
    if (!csb_v1_startup_session_load_surface_pc34(
            profile, title, &surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34]) ||
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
            profile, left, &surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34]) ||
        !csb_v1_startup_session_load_surface_pc34(
            profile, right, &surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34]) ||
        !csb_v1_startup_session_load_surface_pc34(
            profile, screen, &surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34]) ||
        !csb_v1_startup_session_load_surface_pc34(
            profile, credits, &surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_CREDITS_PC34])) {
        csb_v1_boot_startup_runtime_asset_session_release_pc34(out_session);
        return 0;
    }
    if (!csb_v1_startup_graphic_decode_capture_admitted_pc34(
            &surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34],
            1, 320, 153) ||
        !csb_v1_startup_graphic_decode_capture_admitted_pc34(
            &surfaces->surfaces[
                CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34],
            4, 320, 200)) {
        csb_v1_boot_startup_runtime_asset_session_release_pc34(out_session);
        return 0;
    }
    (void)csb_v1_startup_session_load_surface_pc34(
        profile, inventory,
        &surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34]);
    (void)csb_v1_startup_session_load_surface_pc34(
        profile, resurrect,
        &surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34]);
    /* ReDMCSB PANEL.C F0346/F0347 blits C040 with C06 transparency before
     * returning the C017 panel.  This is distinct from C017's opaque base. */
    surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34]
        .transparent_color = CSB_V1_STARTUP_HUD_RESURRECT_TRANSPARENT_COLOR_PC34;
    if (!csb_v1_startup_package_geometry_matches_pc34(surfaces)) {
        csb_v1_boot_startup_runtime_asset_session_release_pc34(out_session);
        return 0;
    }
    surfaces->title_regions_ready = 1;
    surfaces->opening_frame_ready = 1;
    surfaces->entrance_screen_ready = 1;
    surfaces->hud_surfaces_ready =
        surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34]
                .valid &&
        surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34]
                .valid;
    surfaces->real_asset_matched = 1;
    surfaces->valid = 1;
    out_session->hud_inventory_binding = inventory ? *inventory : (CSB_V1_StartupAssetBinding_PC34){0};
    out_session->hud_resurrect_binding = resurrect ? *resurrect : (CSB_V1_StartupAssetBinding_PC34){0};
    if (inventory && resurrect &&
        inventory->source == CSB_V1_STARTUP_ASSET_SOURCE_CSBGRAPHICS_DAT_PC34 &&
        resurrect->source == CSB_V1_STARTUP_ASSET_SOURCE_CSBGRAPHICS_DAT_PC34) {
        uint32_t hash = 2166136261u;

        hash = csb_v1_startup_hash_text_pc34(
            hash, profile->csbgraphics_cache.matched_md5);
        hash = csb_v1_startup_hash_text_pc34(
            hash, profile->csbgraphics_cache.resolved_path);
        hash = csb_v1_startup_frame_hash_step_pc34(hash,
                                                     inventory->graphic_index);
        out_session->hud_source_receipt_hash =
            csb_v1_startup_frame_hash_step_pc34(hash, resurrect->graphic_index);
        if (csb_v1_boot_csbgraphics_palette_receipt_ready(profile)) {
            const CSB_V1_CSBGraphicsDatPaletteSourceReceipt *palette =
                &profile->csbgraphics_palette_receipt;
            uint32_t palette_hash = 2166136261u;

            palette_hash = csb_v1_startup_hash_text_pc34(
                palette_hash, palette->source_md5);
            palette_hash = csb_v1_startup_hash_text_pc34(
                palette_hash, palette->source_path);
            palette_hash = csb_v1_startup_frame_hash_step_pc34(
                palette_hash, palette->entry_span.entry_index);
            out_session->csbgraphics_palette_receipt_hash =
                csb_v1_startup_frame_hash_step_pc34(
                    palette_hash, palette->decoded_fnv1a);
            out_session->csbgraphics_palette_receipt_ready = 1;
            out_session->hud_source_receipt_hash =
                csb_v1_startup_frame_hash_step_pc34(
                    out_session->hud_source_receipt_hash,
                    out_session->csbgraphics_palette_receipt_hash);
        }
    }
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
        resurrect->verified &&
        ((inventory->source == CSB_V1_STARTUP_ASSET_SOURCE_GRAPHICS_DAT_PC34 &&
          resurrect->source == CSB_V1_STARTUP_ASSET_SOURCE_GRAPHICS_DAT_PC34) ||
         (inventory->source == CSB_V1_STARTUP_ASSET_SOURCE_CSBGRAPHICS_DAT_PC34 &&
          resurrect->source == CSB_V1_STARTUP_ASSET_SOURCE_CSBGRAPHICS_DAT_PC34)) &&
        surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34].valid &&
        surfaces->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34].valid;
    out_session->full_startup_ready =
        out_session->title_presents_ready && out_session->title_chaos_ready &&
        out_session->title_strikes_back_ready && out_session->entrance_assets_ready &&
        out_session->door_assets_ready;
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

static uint32_t csb_v1_startup_frame_hash_step_pc34(uint32_t hash,
                                                    uint32_t value)
{
    hash ^= value;
    hash *= 16777619u;
    return hash ? hash : 2166136261u;
}

static int csb_v1_startup_raster_blit_pc34(
    unsigned char *destination, int destination_width, int destination_height,
    const CSB_V1_StartupRuntimeSurface_PC34 *source, int source_x,
    int source_y, int source_width, int source_height, int destination_x,
    int destination_y, int destination_blit_width, int destination_blit_height,
    int transparent_color)
{
    int x;
    int y;
    int copied = 0;

    if (!destination || !source || !source->valid || !source->pixels ||
        destination_width <= 0 || destination_height <= 0 || source_width <= 0 ||
        source_height <= 0 || destination_blit_width <= 0 ||
        destination_blit_height <= 0 || source_x < 0 || source_y < 0 ||
        source_x + source_width > source->width ||
        source_y + source_height > source->height) return 0;
    for (y = 0; y < destination_blit_height; ++y) {
        const int dy = destination_y + y;
        const int sy = source_y + (y * source_height) / destination_blit_height;
        if (dy < 0 || dy >= destination_height) continue;
        for (x = 0; x < destination_blit_width; ++x) {
            const int dx = destination_x + x;
            const int sx = source_x + (x * source_width) / destination_blit_width;
            const unsigned char pixel = source->pixels[(size_t)sy * source->width + sx];
            if (dx < 0 || dx >= destination_width || pixel == transparent_color) continue;
            destination[(size_t)dy * destination_width + dx] = pixel;
            copied = 1;
        }
    }
    return copied;
}

static uint32_t csb_v1_startup_raster_hash_pc34(const unsigned char *pixels,
                                                size_t pixel_count)
{
    uint32_t hash = 2166136261u;
    size_t i;
    if (!pixels || pixel_count == 0u) return 0u;
    for (i = 0; i < pixel_count; ++i)
        hash = csb_v1_startup_frame_hash_step_pc34(hash, pixels[i]);
    return hash;
}

static int csb_v1_startup_raster_has_visible_pixels_pc34(
    const unsigned char *pixels, size_t pixel_count)
{
    size_t i;

    if (!pixels || pixel_count == 0u) return 0;
    for (i = 0; i < pixel_count; ++i) {
        if (pixels[i] != 0u) return 1;
    }
    return 0;
}

void csb_v1_boot_startup_runtime_raster_release_pc34(
    CSB_V1_StartupRuntimeRaster_PC34 *raster)
{
    if (!raster) return;
    free(raster->pixels);
    memset(raster, 0, sizeof(*raster));
}

void csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *receipt)
{
    if (!receipt) return;
    csb_v1_boot_startup_runtime_raster_release_pc34(&receipt->raster);
    memset(receipt, 0, sizeof(*receipt));
}

int csb_v1_boot_startup_runtime_frame_rasterize_pc34(
    const CSB_V1_StartupRuntimeAssetFrame_PC34 *frame,
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    CSB_V1_StartupRuntimeRaster_PC34 *out_raster)
{
    const CSB_V1_StartupRuntimeSurface_PC34 *surface;
    unsigned char *pixels;
    int copied = 0;
    int left_door_copied = 0;
    int right_door_copied = 0;

    if (!out_raster) return 0;
    memset(out_raster, 0, sizeof(*out_raster));
    if (!frame || !plan || !frame->valid || !frame->real_asset_matched ||
        !frame->no_legacy_wrappers) return 0;
    pixels = (unsigned char *)calloc(
        CSB_V1_STARTUP_RUNTIME_RASTER_WIDTH_PC34 *
            CSB_V1_STARTUP_RUNTIME_RASTER_HEIGHT_PC34,
        1u);
    if (!pixels) return 0;

    if (plan->surface == CSB_V1_STARTUP_RENDER_TITLE_PC34) {
        surface = frame->title_surface;
        if (!surface || !surface->valid || !surface->pixels ||
            plan->title_source_w <= 0 || plan->title_source_h <= 0 ||
            plan->title_dest_w <= 0 || plan->title_dest_h <= 0) goto done;
        /* TITLE.C F0437 selects distinct C001 source rectangles for
         * PRESENTS, CHAOS, and STRIKES BACK.  Scaling the complete 320x200
         * asset here discards that contract and can present a black phase. */
        copied = csb_v1_startup_raster_blit_pc34(
            pixels, CSB_V1_STARTUP_RUNTIME_RASTER_WIDTH_PC34,
            CSB_V1_STARTUP_RUNTIME_RASTER_HEIGHT_PC34, surface,
            plan->title_source_x, plan->title_source_y,
            plan->title_source_w, plan->title_source_h, plan->title_dest_x,
            plan->title_dest_y, plan->title_dest_w, plan->title_dest_h,
            plan->title_transparent_color);
        out_raster->title_composited = copied ? 1 : 0;
        out_raster->source_surface_count = copied ? 1 : 0;
    } else {
        surface = frame->entrance_surface;
        if (!surface || !surface->valid || !surface->pixels) goto done;
        copied = csb_v1_startup_raster_blit_pc34(
            pixels, CSB_V1_STARTUP_RUNTIME_RASTER_WIDTH_PC34,
            CSB_V1_STARTUP_RUNTIME_RASTER_HEIGHT_PC34, surface, 0, 0,
            surface->width, surface->height, plan->surface_dest_x,
            plan->surface_dest_y, surface->width, surface->height,
            plan->surface_transparent_color);
        out_raster->entrance_composited = copied ? 1 : 0;
        out_raster->source_surface_count = copied ? 1 : 0;
        if (plan->surface == CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34) {
            if (!frame->left_door_surface || !frame->right_door_surface) goto done;
            left_door_copied = csb_v1_startup_raster_blit_pc34(
                pixels, CSB_V1_STARTUP_RUNTIME_RASTER_WIDTH_PC34,
                CSB_V1_STARTUP_RUNTIME_RASTER_HEIGHT_PC34,
                frame->left_door_surface, plan->closed_left_source_x,
                plan->closed_left_source_y, plan->closed_left_w,
                plan->closed_left_h, plan->closed_left_dest_x,
                plan->closed_left_dest_y, plan->closed_left_w,
                plan->closed_left_h, -1);
            right_door_copied = csb_v1_startup_raster_blit_pc34(
                pixels, CSB_V1_STARTUP_RUNTIME_RASTER_WIDTH_PC34,
                CSB_V1_STARTUP_RUNTIME_RASTER_HEIGHT_PC34,
                frame->right_door_surface, plan->closed_right_source_x,
                plan->closed_right_source_y, plan->closed_right_w,
                plan->closed_right_h, plan->closed_right_dest_x,
                plan->closed_right_dest_y, plan->closed_right_w,
                plan->closed_right_h, -1);
            if (!left_door_copied || !right_door_copied) goto done;
            out_raster->door_composited = 1;
            out_raster->source_surface_count += 2;
        } else if (plan->surface == CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34) {
            if (!plan->opening_composite_valid || !frame->left_door_surface ||
                !frame->right_door_surface) goto done;
            if (plan->opening_left_w > 0) {
                left_door_copied = csb_v1_startup_raster_blit_pc34(
                    pixels, CSB_V1_STARTUP_RUNTIME_RASTER_WIDTH_PC34,
                    CSB_V1_STARTUP_RUNTIME_RASTER_HEIGHT_PC34,
                    frame->left_door_surface, plan->opening_left_source_x,
                    plan->opening_left_source_y, plan->opening_left_w,
                    plan->opening_left_h, plan->opening_left_dest_x,
                    plan->opening_left_dest_y, plan->opening_left_w,
                    plan->opening_left_h, -1);
                if (!left_door_copied) goto done;
                out_raster->source_surface_count++;
            }
            if (plan->opening_right_w > 0) {
                right_door_copied = csb_v1_startup_raster_blit_pc34(
                    pixels, CSB_V1_STARTUP_RUNTIME_RASTER_WIDTH_PC34,
                    CSB_V1_STARTUP_RUNTIME_RASTER_HEIGHT_PC34,
                    frame->right_door_surface, plan->opening_right_source_x,
                    plan->opening_right_source_y, plan->opening_right_w,
                    plan->opening_right_h, plan->opening_right_dest_x,
                    plan->opening_right_dest_y, plan->opening_right_w,
                    plan->opening_right_h, -1);
                if (!right_door_copied) goto done;
                out_raster->source_surface_count++;
            }
            out_raster->door_composited =
                (plan->opening_left_w <= 0 || left_door_copied) &&
                (plan->opening_right_w <= 0 || right_door_copied);
        }
    }
    if (!copied) goto done;
    out_raster->pixels = pixels;
    out_raster->width = CSB_V1_STARTUP_RUNTIME_RASTER_WIDTH_PC34;
    out_raster->height = CSB_V1_STARTUP_RUNTIME_RASTER_HEIGHT_PC34;
    out_raster->real_asset_matched = 1;
    out_raster->pixel_hash = csb_v1_startup_raster_hash_pc34(
        pixels, (size_t)out_raster->width * out_raster->height);
    out_raster->route_hash = csb_v1_startup_frame_hash_step_pc34(
        frame->frame_route_hash, out_raster->pixel_hash);
    out_raster->route_hash = csb_v1_startup_frame_hash_step_pc34(
        out_raster->route_hash, (uint32_t)out_raster->source_surface_count);
    out_raster->valid = out_raster->pixel_hash != 0u &&
        out_raster->route_hash != 0u &&
        csb_v1_startup_raster_has_visible_pixels_pc34(
            pixels, (size_t)out_raster->width * out_raster->height) &&
        (out_raster->title_composited || out_raster->entrance_composited) &&
        ((plan->surface != CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34 &&
          plan->surface != CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34) ||
         out_raster->door_composited);
    if (out_raster->valid) return 1;
done:
    csb_v1_boot_startup_runtime_raster_release_pc34(out_raster);
    return 0;
}

int csb_v1_boot_startup_runtime_hud_frame_rasterize_pc34(
    const CSB_V1_StartupRuntimeAssetFrame_PC34 *frame,
    int draw_resurrect_panel,
    CSB_V1_StartupRuntimeRaster_PC34 *out_raster)
{
    unsigned char *pixels;
    int source_count = 0;

    if (!out_raster) return 0;
    memset(out_raster, 0, sizeof(*out_raster));
    if (!frame || !frame->valid || !frame->real_asset_matched ||
        !frame->no_legacy_wrappers ||
        !csb_v1_startup_hud_capture_surface_matches_pc34(
            frame->hud_inventory_surface, 17,
            CSB_V1_STARTUP_HUD_INVENTORY_WIDTH_PC34,
            CSB_V1_STARTUP_HUD_INVENTORY_HEIGHT_PC34, -1) ||
        (draw_resurrect_panel &&
         !csb_v1_startup_hud_capture_surface_matches_pc34(
             frame->hud_resurrect_surface, 40,
             CSB_V1_STARTUP_HUD_RESURRECT_WIDTH_PC34,
             CSB_V1_STARTUP_HUD_RESURRECT_HEIGHT_PC34,
             CSB_V1_STARTUP_HUD_RESURRECT_TRANSPARENT_COLOR_PC34))) {
        return 0;
    }

    pixels = (unsigned char *)calloc(
        CSB_V1_STARTUP_RUNTIME_RASTER_WIDTH_PC34 *
            CSB_V1_STARTUP_RUNTIME_RASTER_HEIGHT_PC34,
        1u);
    if (!pixels) return 0;

    /* ReDMCSB PANEL.C F0347 expands C017 into the 224x136 viewport at
     * (0,33). F0346 then blits C040 at panel-relative (80,52), hence
     * screen (80,85), with dark-green key 6. CSBWin Character.cpp
     * TAG0189a8 lines 3836-3844 independently uses basic graphic 40 with
     * the same transparency key. */
    if (!csb_v1_startup_raster_blit_pc34(
            pixels, CSB_V1_STARTUP_RUNTIME_RASTER_WIDTH_PC34,
            CSB_V1_STARTUP_RUNTIME_RASTER_HEIGHT_PC34,
            frame->hud_inventory_surface, 0, 0,
            CSB_V1_STARTUP_HUD_INVENTORY_WIDTH_PC34,
            CSB_V1_STARTUP_HUD_INVENTORY_HEIGHT_PC34, 0, 33,
            CSB_V1_STARTUP_HUD_INVENTORY_WIDTH_PC34,
            CSB_V1_STARTUP_HUD_INVENTORY_HEIGHT_PC34, -1)) {
        free(pixels);
        return 0;
    }
    source_count = 1;
    if (draw_resurrect_panel && !csb_v1_startup_raster_blit_pc34(
            pixels, CSB_V1_STARTUP_RUNTIME_RASTER_WIDTH_PC34,
            CSB_V1_STARTUP_RUNTIME_RASTER_HEIGHT_PC34,
            frame->hud_resurrect_surface, 0, 0,
            CSB_V1_STARTUP_HUD_RESURRECT_WIDTH_PC34,
            CSB_V1_STARTUP_HUD_RESURRECT_HEIGHT_PC34, 80, 85,
            CSB_V1_STARTUP_HUD_RESURRECT_WIDTH_PC34,
            CSB_V1_STARTUP_HUD_RESURRECT_HEIGHT_PC34,
            frame->hud_resurrect_surface->transparent_color)) {
        free(pixels);
        return 0;
    }
    if (draw_resurrect_panel) source_count = 2;
    out_raster->pixels = pixels;
    out_raster->width = CSB_V1_STARTUP_RUNTIME_RASTER_WIDTH_PC34;
    out_raster->height = CSB_V1_STARTUP_RUNTIME_RASTER_HEIGHT_PC34;
    out_raster->real_asset_matched = 1;
    out_raster->entrance_composited = 1;
    out_raster->source_surface_count = source_count;
    out_raster->pixel_hash = csb_v1_startup_raster_hash_pc34(
        pixels, (size_t)out_raster->width * (size_t)out_raster->height);
    out_raster->route_hash = csb_v1_startup_frame_hash_step_pc34(
        frame->hud_binding_hash, (uint32_t)draw_resurrect_panel);
    out_raster->valid = out_raster->pixel_hash != 0u &&
        out_raster->route_hash != 0u;
    if (out_raster->valid) return 1;
    csb_v1_boot_startup_runtime_raster_release_pc34(out_raster);
    return 0;
}

int csb_v1_boot_startup_runtime_hud_panel_blit_from_session_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    int draw_resurrect_panel,
    unsigned char *destination,
    int destination_width,
    int destination_height,
    CSB_V1_StartupRuntimeHudPanelReceipt_PC34 *out_receipt)
{
    enum {
        CSB_V1_STARTUP_HUD_SCREEN_X_PC34 = 0,
        CSB_V1_STARTUP_HUD_SCREEN_Y_PC34 = 33
    };
    CSB_V1_StartupRenderPlan_PC34 plan;
    CSB_V1_StartupRuntimeAssetFrame_PC34 frame;
    CSB_V1_StartupRuntimeRaster_PC34 raster;
    CSB_V1_StartupRuntimeHudPanelReceipt_PC34 receipt;
    CSB_V1_StartupCompleteTimelineReceipt_PC34 timeline;
    CSB_V1_F0347_InventoryDrawPanelReceipt_PC34 f0347_receipt;
    int row;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&plan, 0, sizeof(plan));
    memset(&frame, 0, sizeof(frame));
    memset(&raster, 0, sizeof(raster));
    memset(&receipt, 0, sizeof(receipt));
    memset(&timeline, 0, sizeof(timeline));
    memset(&f0347_receipt, 0, sizeof(f0347_receipt));
    /* After ENTRANCE.C F0807, PANEL.C F0347 receives the live C017/C040
     * session. A closed entrance surface plus DUNGEON_RUNTIME selects that
     * exact terminal frame without re-entering any entrance draw wrapper. */
    plan.surface = CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34;
    plan.title_stage = CSB_V1_STARTUP_STAGE_DUNGEON_RUNTIME_PC34;
    plan.title_special_palette = -1;
    plan.special_palette = -1;
    if (!session || !destination || !out_receipt ||
        destination_width < CSB_V1_STARTUP_HUD_INVENTORY_WIDTH_PC34 ||
        destination_height < CSB_V1_STARTUP_HUD_SCREEN_Y_PC34 +
            CSB_V1_STARTUP_HUD_INVENTORY_HEIGHT_PC34 ||
        !csb_v1_boot_startup_complete_timeline_receipt_from_session_pc34(
            session, &timeline) ||
        !timeline.valid || !timeline.terminal_f0807_complete ||
        !timeline.hud_session_ready ||
        !csb_v1_boot_startup_runtime_asset_session_frame_pc34(
            session, &plan, session->source_tick, &frame) ||
        !csb_v1_boot_startup_runtime_hud_frame_rasterize_pc34(
            &frame, draw_resurrect_panel ? 1 : 0, &raster) ||
        !raster.valid || !raster.real_asset_matched ||
        raster.width != CSB_V1_STARTUP_RUNTIME_RASTER_WIDTH_PC34 ||
        raster.height != CSB_V1_STARTUP_RUNTIME_RASTER_HEIGHT_PC34 ||
        raster.source_surface_count != (draw_resurrect_panel ? 2 : 1)) {
        csb_v1_boot_startup_runtime_raster_release_pc34(&raster);
        return 0;
    }

    receipt.real_asset_matched = frame.real_asset_matched;
    receipt.c017_presented = 1;
    receipt.c040_presented = draw_resurrect_panel ? 1 : 0;
    receipt.no_legacy_wrappers = frame.no_legacy_wrappers;
    receipt.no_synthetic_surface = 1;
    receipt.source_tick = frame.source_tick;
    receipt.session_generation = frame.session_generation;
    receipt.c017_pixel_hash = frame.hud_inventory_pixel_hash;
    receipt.c040_pixel_hash = draw_resurrect_panel
        ? frame.hud_resurrect_pixel_hash : 0u;
    receipt.panel_hash = csb_v1_startup_frame_hash_step_pc34(
        csb_v1_startup_frame_hash_step_pc34(raster.pixel_hash,
                                            receipt.c017_pixel_hash),
        receipt.c040_pixel_hash);
    receipt.source_evidence =
        "ReDMCSB PANEL.C F0347/F0346 lines 2376-2448; "
        "CSBWin Character.cpp TAG0189a8 lines 3836-3844";
    receipt.valid = receipt.real_asset_matched && receipt.c017_presented &&
        receipt.no_legacy_wrappers && receipt.no_synthetic_surface &&
        receipt.c017_pixel_hash != 0u &&
        (!receipt.c040_presented || receipt.c040_pixel_hash != 0u) &&
        receipt.panel_hash != 0u;
    if (!receipt.valid ||
        !csb_v1_f0347_inventory_draw_panel_pc34(
            &receipt, draw_resurrect_panel ? 1 : 0, &f0347_receipt) ||
        !f0347_receipt.valid ||
        f0347_receipt.c017_pixel_hash != receipt.c017_pixel_hash ||
        f0347_receipt.c040_pixel_hash != receipt.c040_pixel_hash ||
        f0347_receipt.panel_hash != receipt.panel_hash) {
        csb_v1_boot_startup_runtime_raster_release_pc34(&raster);
        return 0;
    }

    /* Copy only PANEL.C's source-owned 224x136 region after F0347/F0346
     * accepts the exact C017/C040 receipt. The dungeon page behind it remains
     * the M11 viewport result; no host panel or generated HUD substitute is
     * introduced. */
    for (row = 0; row < CSB_V1_STARTUP_HUD_INVENTORY_HEIGHT_PC34; ++row) {
        memcpy(destination +
                   (size_t)(CSB_V1_STARTUP_HUD_SCREEN_Y_PC34 + row) *
                       (size_t)destination_width +
                   CSB_V1_STARTUP_HUD_SCREEN_X_PC34,
               raster.pixels +
                   (size_t)(CSB_V1_STARTUP_HUD_SCREEN_Y_PC34 + row) *
                       CSB_V1_STARTUP_RUNTIME_RASTER_WIDTH_PC34 +
                   CSB_V1_STARTUP_HUD_SCREEN_X_PC34,
               CSB_V1_STARTUP_HUD_INVENTORY_WIDTH_PC34);
    }
    csb_v1_boot_startup_runtime_raster_release_pc34(&raster);
    *out_receipt = receipt;
    return 1;
}

static int csb_v1_startup_frame_title_phase_mask_pc34(
    CSB_V1_StartupStage_PC34 stage)
{
    if (stage == CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34) {
        return 0x01;
    }
    if (stage == CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34) {
        return 0x02;
    }
    if (stage == CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34) {
        return 0x08;
    }
    return 0;
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
    out_frame->real_asset_matched = session->real_asset_matched ? 1 : 0;
    out_frame->title_sequence_ready =
        session->title_presents_ready && session->title_chaos_ready &&
                session->title_strikes_back_ready
            ? 1
            : 0;
    out_frame->entrance_ready = session->entrance_assets_ready ? 1 : 0;
    out_frame->door_ready = session->door_assets_ready ? 1 : 0;
    out_frame->no_legacy_wrappers =
        session->rejects_legacy_wrappers ? 1 : 0;
    out_frame->special_palette = plan->special_palette;
    out_frame->title_special_palette = plan->title_special_palette;
    out_frame->source_tick = source_tick;
    out_frame->session_generation = session->generation;
    out_frame->stage = (CSB_V1_StartupStage_PC34)plan->title_stage;
    out_frame->special_palette = plan->special_palette;
    out_frame->opening_step = plan->opening_door_step;
    out_frame->uses_verified_hud_bindings = session->hud_assets_bound;
    out_frame->left_door_surface = &session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34];
    out_frame->right_door_surface = &session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34];
    out_frame->hud_inventory_surface = &session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34];
    out_frame->hud_resurrect_surface = &session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34];
    if (out_frame->hud_inventory_surface->valid &&
        out_frame->hud_inventory_surface->pixels) {
        out_frame->hud_inventory_pixel_hash = csb_v1_startup_raster_hash_pc34(
            out_frame->hud_inventory_surface->pixels,
            (size_t)out_frame->hud_inventory_surface->width *
                (size_t)out_frame->hud_inventory_surface->height);
    }
    if (out_frame->hud_resurrect_surface->valid &&
        out_frame->hud_resurrect_surface->pixels) {
        out_frame->hud_resurrect_pixel_hash = csb_v1_startup_raster_hash_pc34(
            out_frame->hud_resurrect_surface->pixels,
            (size_t)out_frame->hud_resurrect_surface->width *
                (size_t)out_frame->hud_resurrect_surface->height);
    }
    out_frame->hud_binding_hash = csb_v1_startup_frame_hash_step_pc34(
        out_frame->hud_inventory_pixel_hash,
        out_frame->hud_resurrect_pixel_hash);
    out_frame->hud_source_receipt_hash = session->hud_source_receipt_hash;
    if (out_frame->hud_source_receipt_hash != 0u) {
        out_frame->hud_binding_hash = csb_v1_startup_frame_hash_step_pc34(
            out_frame->hud_binding_hash, out_frame->hud_source_receipt_hash);
    }
    if (plan->surface == CSB_V1_STARTUP_RENDER_TITLE_PC34) {
        out_frame->title_phase_tick = plan->title_source_step;
        out_frame->title_phase_tick_count = csb_v1_startup_title_total_ticks_pc34();
        out_frame->title_phase_mask =
            csb_v1_startup_frame_title_phase_mask_pc34(out_frame->stage);
        /* TITLE.C F0437's plan addresses the resident C001 bitmap.  The
         * retained phase crops prove package geometry at load time, but are
         * not a second coordinate system for M11 presentation. */
        out_frame->title_surface = &session->surfaces.surfaces[
            CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34];
    } else if (plan->surface == CSB_V1_STARTUP_RENDER_ENTRANCE_CREDITS_PC34) {
        out_frame->entrance_surface = &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_CREDITS_PC34];
    } else if (plan->surface != CSB_V1_STARTUP_RENDER_NONE_PC34 &&
               plan->surface != CSB_V1_STARTUP_RENDER_ENTRANCE_BLACK_PC34) {
        out_frame->entrance_surface = &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34];
        if (plan->title_stage != CSB_V1_STARTUP_STAGE_DUNGEON_RUNTIME_PC34) {
            session->playback.entrance_scene_presented = 1;
            session->playback.entrance_special_palette = plan->special_palette;
            if (plan->surface == CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34 ||
                plan->surface == CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34) {
                session->playback.door_frame_presented = 1;
            }
            if (plan->surface == CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34 &&
                plan->opening_door_step > 0) {
                session->playback.last_door_opening_step = plan->opening_door_step;
                session->playback.next_door_opening_step =
                    plan->opening_door_step + 1;
            }
        }
    }
    out_frame->frame_route_hash =
        csb_v1_startup_frame_hash_step_pc34(2166136261u,
                                            (uint32_t)out_frame->source_tick);
    out_frame->frame_route_hash = csb_v1_startup_frame_hash_step_pc34(
        out_frame->frame_route_hash,
        (uint32_t)out_frame->session_generation);
    out_frame->frame_route_hash = csb_v1_startup_frame_hash_step_pc34(
        out_frame->frame_route_hash, (uint32_t)plan->surface);
    out_frame->frame_route_hash = csb_v1_startup_frame_hash_step_pc34(
        out_frame->frame_route_hash, (uint32_t)out_frame->stage);
    out_frame->frame_route_hash = csb_v1_startup_frame_hash_step_pc34(
        out_frame->frame_route_hash, (uint32_t)out_frame->opening_step);
    out_frame->frame_route_hash = csb_v1_startup_frame_hash_step_pc34(
        out_frame->frame_route_hash, (uint32_t)out_frame->title_phase_mask);
    if (plan->surface == CSB_V1_STARTUP_RENDER_TITLE_PC34 ||
        plan->surface == CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34) {
        out_frame->frame_route_hash = csb_v1_startup_frame_hash_step_pc34(
            out_frame->frame_route_hash,
            (uint32_t)(out_frame->special_palette + 1));
        out_frame->frame_route_hash = csb_v1_startup_frame_hash_step_pc34(
            out_frame->frame_route_hash,
            (uint32_t)(out_frame->title_special_palette + 1));
    }
    out_frame->frame_route_hash = csb_v1_startup_frame_hash_step_pc34(
        out_frame->frame_route_hash, out_frame->hud_binding_hash);
    out_frame->valid =
        out_frame->real_asset_matched &&
        out_frame->title_sequence_ready &&
        out_frame->entrance_ready &&
        out_frame->door_ready &&
        out_frame->no_legacy_wrappers &&
        (out_frame->title_surface || out_frame->entrance_surface) &&
        out_frame->left_door_surface->valid &&
        out_frame->right_door_surface->valid &&
        (plan->title_stage != CSB_V1_STARTUP_STAGE_DUNGEON_RUNTIME_PC34 ||
         (out_frame->uses_verified_hud_bindings &&
          out_frame->hud_inventory_surface->valid &&
          out_frame->hud_resurrect_surface->valid)) &&
        out_frame->frame_route_hash != 0u;
    return out_frame->valid;
}

int csb_v1_boot_startup_runtime_host_surface_receipt_from_session_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    uint32_t source_tick,
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 receipt;
    uint32_t hash = 2166136261u;
    int expected_opening_surface_count;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    expected_opening_surface_count = 0;
    if (!session || !plan) {
        return 0;
    }
    /* ReDMCSB ENTRANCE.C F0438 presents C004/C002/C003 with
     * C28_ENTRANCE_CSB. F0807 releases that temporary palette before PANEL.C
     * draws the first C017/C040 page. Reject before frame creation so a
     * forged palette cannot mutate session presentation state. */
    if ((plan->surface == CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34 &&
         (plan->special_palette != VGA_PALETTE_PC34_SPECIAL_ENTRANCE ||
          plan->title_special_palette != -1)) ||
        (plan->title_stage == CSB_V1_STARTUP_STAGE_DUNGEON_RUNTIME_PC34 &&
         (plan->surface != CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34 ||
          plan->special_palette != -1 || plan->title_special_palette != -1))) {
        return 0;
    }
    if (!csb_v1_boot_startup_runtime_asset_session_frame_pc34(
            session, plan, source_tick, &receipt.frame)) {
        return 0;
    }

    receipt.real_asset_matched = receipt.frame.real_asset_matched;
    receipt.no_legacy_wrappers = receipt.frame.no_legacy_wrappers;
    receipt.no_synthetic_surface = 1;
    receipt.uses_c017_inventory =
        receipt.frame.hud_inventory_surface &&
        receipt.frame.hud_inventory_surface->source_asset_id == 17 &&
        receipt.frame.hud_inventory_pixel_hash != 0u;
    receipt.uses_c040_resurrect =
        receipt.frame.hud_resurrect_surface &&
        receipt.frame.hud_resurrect_surface->source_asset_id == 40 &&
        receipt.frame.hud_resurrect_surface->transparent_color == 6 &&
        receipt.frame.hud_resurrect_pixel_hash != 0u;
    receipt.special_palette = receipt.frame.special_palette;
    receipt.title_special_palette = receipt.frame.title_special_palette;

    if (plan->title_stage == CSB_V1_STARTUP_STAGE_DUNGEON_RUNTIME_PC34) {
        /* ReDMCSB ENTRANCE.C F0806 leaves the temporary entrance loop before
         * PANEL.C consumes the live C017/C040 HUD. Capture that exact indexed
         * presentation rather than handing the host an unpresented pair. */
        if (session->playback.stage != CSB_V1_STARTUP_PLAYBACK_STAGE_HUD_PC34 ||
            !receipt.frame.uses_verified_hud_bindings ||
            !receipt.uses_c017_inventory || !receipt.uses_c040_resurrect) {
            return 0;
        }
        if (!csb_v1_boot_startup_runtime_hud_frame_rasterize_pc34(
                &receipt.frame, 1, &receipt.raster) || !receipt.raster.valid ||
            !receipt.raster.real_asset_matched ||
            receipt.raster.source_surface_count != 2) {
            csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(
                &receipt);
            return 0;
        }
        receipt.host_surface = CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_HUD_PC34;
        receipt.runtime_hud_decision = 1;
    } else {
        if (!csb_v1_boot_startup_runtime_frame_rasterize_pc34(
                &receipt.frame, plan, &receipt.raster) ||
            !receipt.raster.valid || !receipt.raster.real_asset_matched) {
            csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(
                &receipt);
            return 0;
        }
        if (plan->surface == CSB_V1_STARTUP_RENDER_TITLE_PC34) {
            if (!receipt.raster.title_composited ||
                receipt.raster.source_surface_count != 1 ||
                receipt.special_palette < 0 ||
                receipt.special_palette != receipt.title_special_palette) {
                csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(
                    &receipt);
                return 0;
            }
            receipt.host_surface = CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_TITLE_PC34;
        } else if (plan->surface ==
                   CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34) {
            /* ENTRANCE.C F0438 clips C002 away after source step 26.  The
             * final five pages therefore contain C004 plus C003, not a
             * fictitious third surface.  Count only the source rectangles
             * which F0807 actually submits to the raster. */
            expected_opening_surface_count = 1;
            if (plan->opening_left_w > 0) expected_opening_surface_count++;
            if (plan->opening_right_w > 0) expected_opening_surface_count++;
            if (!plan->opening_composite_valid || !receipt.raster.door_composited ||
                receipt.raster.source_surface_count != expected_opening_surface_count ||
                receipt.special_palette !=
                    VGA_PALETTE_PC34_SPECIAL_ENTRANCE ||
                receipt.title_special_palette != -1) {
                csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(
                    &receipt);
                return 0;
            }
            receipt.host_surface =
                CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_DOOR_OPENING_PC34;
            receipt.door_opening_decision = 1;
        } else if (plan->surface ==
                   CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34) {
            if (!receipt.raster.entrance_composited ||
                !receipt.raster.door_composited ||
                receipt.raster.source_surface_count != 3) {
                csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(
                    &receipt);
                return 0;
            }
            receipt.host_surface =
                CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_ENTRANCE_PC34;
        } else if (plan->surface ==
                   CSB_V1_STARTUP_RENDER_ENTRANCE_CREDITS_PC34) {
            /* ReDMCSB ENTRANCE.C F0442 presents C005 as one full-screen
             * package surface while credits are active. */
            if (!receipt.raster.entrance_composited ||
                receipt.raster.door_composited ||
                receipt.raster.source_surface_count != 1) {
                csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(
                    &receipt);
                return 0;
            }
            receipt.host_surface =
                CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_CREDITS_PC34;
        } else {
            csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(
                &receipt);
            return 0;
        }
    }
    hash = csb_v1_startup_frame_hash_step_pc34(hash,
                                                 receipt.frame.frame_route_hash);
    hash = csb_v1_startup_frame_hash_step_pc34(hash,
                                                 receipt.raster.route_hash);
    hash = csb_v1_startup_frame_hash_step_pc34(hash,
                                                 receipt.raster.pixel_hash);
    hash = csb_v1_startup_frame_hash_step_pc34(hash,
                                                 (uint32_t)receipt.host_surface);
    hash = csb_v1_startup_frame_hash_step_pc34(
        hash, receipt.frame.hud_binding_hash);
    if (receipt.host_surface == CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_TITLE_PC34 ||
        receipt.host_surface ==
            CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_DOOR_OPENING_PC34) {
        hash = csb_v1_startup_frame_hash_step_pc34(
            hash, (uint32_t)(receipt.special_palette + 1));
        hash = csb_v1_startup_frame_hash_step_pc34(
            hash, (uint32_t)(receipt.title_special_palette + 1));
    }
    receipt.host_surface_hash = hash;
    receipt.source_evidence =
        "ReDMCSB TITLE.C F0437 lines 424-463; ENTRANCE.C F0806 lines "
        "721-826,850-889 and F0442; DUNVIEW.C F0111 lines 4248-4313";
    receipt.valid = receipt.real_asset_matched && receipt.no_legacy_wrappers &&
        receipt.no_synthetic_surface && receipt.host_surface !=
            CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_NONE_PC34 &&
        receipt.raster.pixel_hash != 0u &&
        receipt.host_surface_hash != 0u;
    if (!receipt.valid) {
        csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(
            &receipt);
        return 0;
    }
    *out_receipt = receipt;
    return 1;
}

int csb_v1_boot_startup_door_opening_capture_from_session_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    uint32_t first_source_tick,
    CSB_V1_StartupDoorOpeningCaptureReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupDoorOpeningCaptureReceipt_PC34 receipt;
    CSB_V1_StartupRuntimeAssetSession_PC34 capture_session;
    CSB_V1_StartupRenderState_PC34 state;
    CSB_V1_StartupRenderPlan_PC34 plan;
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 frame_receipt;
    uint32_t hash = 2166136261u;
    int step;
    int prior;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    if (!session || !session->valid || !session->real_asset_matched ||
        !session->entrance_assets_ready || !session->door_assets_ready ||
        !session->rejects_legacy_wrappers ||
        session->playback.stage != CSB_V1_STARTUP_PLAYBACK_STAGE_ENTRANCE_PC34 ||
        !session->playback.entrance_music_active ||
        session->playback.title_phase_mask != 0x0f ||
        first_source_tick > UINT32_MAX -
            CSB_V1_STARTUP_DOOR_OPENING_CAPTURE_FRAME_COUNT_PC34 + 1u) {
        return 0;
    }

    receipt.real_asset_matched = 1;
    receipt.no_legacy_wrappers = 1;
    receipt.no_synthetic_surface = 1;
    receipt.source_step_count =
        CSB_V1_STARTUP_DOOR_OPENING_CAPTURE_FRAME_COUNT_PC34;
    receipt.first_step = 1;
    receipt.last_step =
        CSB_V1_STARTUP_DOOR_OPENING_CAPTURE_FRAME_COUNT_PC34;
    receipt.session_generation = session->generation;
    /* F0438 copies G562[8] into the temporary G562[9] page for every door
     * step. Capture is likewise observational: it consumes verified
     * C004/C002/C003 material without advancing live Entrance playback. */
    capture_session = *session;

    for (step = receipt.first_step; step <= receipt.last_step; ++step) {
        memset(&state, 0, sizeof(state));
        memset(&plan, 0, sizeof(plan));
        memset(&frame_receipt, 0, sizeof(frame_receipt));
        state.entrance_active = 1;
        state.entrance_source_step =
            csb_v1_startup_entrance_wait_stage_pc34();
        state.opening_active = 1;
        state.opening_step = step;
        if (!csb_v1_startup_source_render_plan_from_state_pc34(
                &state, &plan) ||
            plan.surface != CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34 ||
            plan.opening_door_step != step || !plan.opening_composite_valid ||
            !csb_v1_boot_startup_runtime_host_surface_receipt_from_session_pc34(
                &capture_session, &plan,
                first_source_tick + (uint32_t)(step - 1),
                &frame_receipt) ||
            !frame_receipt.valid || !frame_receipt.real_asset_matched ||
            !frame_receipt.no_legacy_wrappers ||
            !frame_receipt.no_synthetic_surface ||
            frame_receipt.host_surface !=
                CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_DOOR_OPENING_PC34 ||
            !frame_receipt.door_opening_decision ||
            frame_receipt.frame.opening_step != step ||
            frame_receipt.raster.pixel_hash == 0u ||
            frame_receipt.host_surface_hash == 0u) {
            csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(
                &frame_receipt);
            return 0;
        }
        receipt.frame_route_hashes[step - 1] = frame_receipt.host_surface_hash;
        receipt.raster_pixel_hashes[step - 1] = frame_receipt.raster.pixel_hash;
        for (prior = 0; prior < step - 1; ++prior) {
            if (receipt.frame_route_hashes[prior] ==
                receipt.frame_route_hashes[step - 1]) {
                csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(
                    &frame_receipt);
                return 0;
            }
        }
        hash = csb_v1_startup_frame_hash_step_pc34(
            hash, receipt.frame_route_hashes[step - 1]);
        hash = csb_v1_startup_frame_hash_step_pc34(
            hash, receipt.raster_pixel_hashes[step - 1]);
        receipt.captured_frame_count = step;
        csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(
            &frame_receipt);
    }
    receipt.capture_sequence_hash = hash;
    receipt.source_evidence =
        "ReDMCSB ENTRANCE.C F0438/F0807: C004 with C002/C003 door "
        "strips across the 31 source animation steps";
    receipt.valid = receipt.real_asset_matched && receipt.no_legacy_wrappers &&
        receipt.no_synthetic_surface && receipt.captured_frame_count ==
            CSB_V1_STARTUP_DOOR_OPENING_CAPTURE_FRAME_COUNT_PC34 &&
        receipt.session_generation != 0u && receipt.capture_sequence_hash != 0u;
    if (!receipt.valid) return 0;
    *out_receipt = receipt;
    return 1;
}

int csb_v1_boot_startup_runtime_host_surface_matches_indexed_frame_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    uint32_t source_tick,
    const unsigned char *indexed_pixels,
    int width,
    int height,
    int special_palette)
{
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 receipt;
    int matches = 0;

    if (!indexed_pixels || width != CSB_V1_STARTUP_RUNTIME_RASTER_WIDTH_PC34 ||
        height != CSB_V1_STARTUP_RUNTIME_RASTER_HEIGHT_PC34) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!csb_v1_boot_startup_runtime_host_surface_receipt_from_session_pc34(
            session, plan, source_tick, &receipt) || !receipt.valid ||
        !receipt.raster.valid || !receipt.raster.real_asset_matched ||
        !receipt.raster.pixels || receipt.raster.width != width ||
        receipt.raster.height != height ||
        receipt.special_palette != special_palette ||
        receipt.frame.special_palette != special_palette) {
        csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(
            &receipt);
        return 0;
    }

    /* TITLE.C F0437 and ENTRANCE.C F0441/F0807 each compose their complete
     * PC3.4 page before VBlank presentation.  Compare every source byte, not
     * a crop/hash, so a stale phase or a wrapper-owned page fails closed. */
    matches = memcmp(indexed_pixels,
                     receipt.raster.pixels,
                     (size_t)width * (size_t)height) == 0;
    csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(&receipt);
    return matches;
}

int csb_v1_boot_startup_full_runtime_receipt_from_session_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    CSB_V1_StartupFullRuntimeReceipt_PC34 *out_receipt)
{
    uint32_t hash = 2166136261u;

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
    out_receipt->hud_ready = session->hud_assets_bound &&
        session->surfaces.hud_surfaces_ready &&
        session->surfaces.surfaces[
            CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34].valid &&
        session->surfaces.surfaces[
            CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34].valid;
    out_receipt->door_ready = session->door_assets_ready;
    out_receipt->no_legacy_wrappers = session->rejects_legacy_wrappers;
    out_receipt->session_generation = session->generation;
    out_receipt->playback_reaches_title = out_receipt->title_sequence_ready;
    out_receipt->playback_reaches_entrance =
        out_receipt->playback_reaches_title && out_receipt->entrance_ready &&
        out_receipt->door_ready;
    out_receipt->playback_reaches_hud =
        out_receipt->playback_reaches_entrance && out_receipt->hud_ready;
    out_receipt->title_to_hud_same_session =
        out_receipt->playback_reaches_hud &&
        out_receipt->session_generation != 0u;
    out_receipt->playback_route_ready =
        out_receipt->title_to_hud_same_session &&
        out_receipt->no_legacy_wrappers;
    hash ^= out_receipt->session_generation;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->playback_reaches_title;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->playback_reaches_entrance;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->playback_reaches_hud;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->title_to_hud_same_session;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->no_legacy_wrappers;
    out_receipt->playback_route_hash = hash ? hash : 1u;
    out_receipt->source_evidence =
        "ReDMCSB TITLE.C F0437; ENTRANCE.C F0806; CSBWin Graphics.cpp ReadGraphic";
    out_receipt->valid =
        out_receipt->real_asset_matched && out_receipt->title_sequence_ready &&
        out_receipt->entrance_ready && out_receipt->hud_ready &&
        out_receipt->door_ready && out_receipt->playback_route_ready &&
        out_receipt->playback_route_hash != 0u;
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

void csb_v1_boot_startup_presented_app_capture_receipt_init_pc34(
    CSB_V1_StartupPresentedAppCaptureReceipt_PC34 *receipt)
{
    if (!receipt) return;
    memset(receipt, 0, sizeof(*receipt));
    csb_v1_boot_startup_release_app_capture_receipt_init_pc34(
        &receipt->release_app_capture);
    receipt->source_evidence =
        "ReDMCSB TITLE.C F0437 lines 424-463; "
        "ENTRANCE.C F0441/F0806 lines 620-883; "
        "CSBWin Graphics.cpp ReadGraphic and Viewport.cpp host presentation";
}

int csb_v1_boot_startup_presented_app_capture_facts_from_indexed_frame_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const unsigned char *indexed_pixels,
    int width,
    int height,
    int running_from_macos_app_bundle,
    int mac_window_capture_ready,
    CSB_V1_StartupPresentedAppCaptureFacts_PC34 *out_facts)
{
    CSB_V1_StartupFullRuntimeReceipt_PC34 full_runtime;
    uint32_t hash = 2166136261u;
    size_t pixel_count;
    size_t i;

    if (out_facts) {
        memset(out_facts, 0, sizeof(*out_facts));
    }
    if (!session || !indexed_pixels || !out_facts || width != 320 ||
        height != 200 ||
        !csb_v1_boot_startup_full_runtime_receipt_from_session_pc34(
            session, &full_runtime) ||
        !full_runtime.valid || !full_runtime.title_sequence_ready ||
        !full_runtime.entrance_ready || !full_runtime.door_ready ||
        !full_runtime.hud_ready || !full_runtime.no_legacy_wrappers) {
        return 0;
    }

    pixel_count = (size_t)width * (size_t)height;
    for (i = 0; i < pixel_count; ++i) {
        hash ^= indexed_pixels[i];
        hash *= 16777619u;
    }
    if (hash == 0u) {
        hash = 1u;
    }

    /* ReDMCSB TITLE.C F0437 retains C001 through PRESENTS/CHAOS/STRIKES;
     * ENTRANCE.C F0438/F0807 reaches C017/C040 after its door frames. The
     * complete verified session owns every one of those phases from launch,
     * so presentation can record each real indexed M11 raster without
     * waiting for the terminal HUD state or admitting a substitute buffer. */
    out_facts->running_from_macos_app_bundle =
        running_from_macos_app_bundle ? 1 : 0;
    out_facts->mac_window_capture_ready =
        mac_window_capture_ready ? 1 : 0;
    out_facts->presented_frame_captured = 1;
    out_facts->presented_frame_width = width;
    out_facts->presented_frame_height = height;
    out_facts->presented_frame_indexed_pixels = 1;
    out_facts->presented_frame_uses_real_csb_assets = 1;
    out_facts->presented_frame_hash = hash;
    return 1;
}

int csb_v1_boot_startup_presented_app_capture_receipt_from_release_pc34(
    const CSB_V1_StartupReleaseAppCaptureReceipt_PC34 *release_app_capture,
    const CSB_V1_StartupPresentedAppCaptureFacts_PC34 *presented_facts,
    CSB_V1_StartupPresentedAppCaptureReceipt_PC34 *out_receipt)
{
    uint32_t hash = 2166136261u;

    if (!out_receipt) return 0;
    csb_v1_boot_startup_presented_app_capture_receipt_init_pc34(out_receipt);
    if (!release_app_capture || !presented_facts) return 0;

    out_receipt->release_app_capture = *release_app_capture;
    out_receipt->release_app_capture_valid =
        release_app_capture->valid &&
                release_app_capture->release_app_capture_ready
            ? 1
            : 0;
    out_receipt->running_from_macos_app_bundle =
        presented_facts->running_from_macos_app_bundle ? 1 : 0;
    out_receipt->mac_window_capture_ready =
        presented_facts->mac_window_capture_ready ? 1 : 0;
    out_receipt->presented_frame_captured =
        presented_facts->presented_frame_captured ? 1 : 0;
    out_receipt->presented_frame_geometry_ready =
        presented_facts->presented_frame_width == 320 &&
                presented_facts->presented_frame_height == 200
            ? 1
            : 0;
    out_receipt->presented_frame_pixels_ready =
        presented_facts->presented_frame_indexed_pixels &&
                presented_facts->presented_frame_hash != 0u
            ? 1
            : 0;
    out_receipt->presented_frame_real_asset_ready =
        presented_facts->presented_frame_uses_real_csb_assets &&
                release_app_capture->release_app_real_asset_capture_ready
            ? 1
            : 0;
    out_receipt->presented_title_sequence_ready =
        release_app_capture->title_sequence_capture_ready &&
                release_app_capture->title_sequence_host_consumer_ready &&
                release_app_capture->title_sequence_same_capture_route &&
                release_app_capture->title_sequence_capture_hash != 0u
            ? 1
            : 0;
    out_receipt->presented_title_phase_mask_ready =
        release_app_capture->title_runtime_phase_mask ==
                    release_app_capture->title_runtime_expected_phase_mask &&
                release_app_capture->title_runtime_phase_hash_count ==
                    CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34 &&
                release_app_capture->title_runtime_phase_hash != 0u
            ? 1
            : 0;
    out_receipt->presented_hud_door_ready =
        release_app_capture->hud_door_capture_ready &&
                release_app_capture->hud_door_host_consumers_ready &&
                release_app_capture->hud_door_same_capture_route &&
                release_app_capture->hud_door_capture_hash != 0u
            ? 1
            : 0;
    out_receipt->presented_hud_door_route_hash_ready =
        release_app_capture->hud_door_same_capture_route &&
                release_app_capture->hud_door_capture_hash != 0u &&
                release_app_capture->hud_door_capture_hash !=
                    release_app_capture->title_sequence_capture_hash &&
                release_app_capture->hud_door_capture_hash !=
                    release_app_capture->release_app_capture_hash
            ? 1
            : 0;
    out_receipt->presented_credits_ready =
        release_app_capture->credits_release_app_capture_ready &&
                release_app_capture->credits_host_consumer_ready &&
                release_app_capture->credits_packaged_capture_hash != 0u
            ? 1
            : 0;
    out_receipt->presented_credits_route_hash_ready =
        out_receipt->presented_credits_ready &&
                release_app_capture->credits_packaged_capture_hash !=
                    release_app_capture->title_sequence_capture_hash &&
                release_app_capture->credits_packaged_capture_hash !=
                    release_app_capture->hud_door_capture_hash &&
                release_app_capture->credits_packaged_capture_hash !=
                    release_app_capture->release_app_capture_hash
            ? 1
            : 0;
    out_receipt->presented_route_aggregates_ready =
        out_receipt->presented_title_sequence_ready &&
                out_receipt->presented_title_phase_mask_ready &&
                out_receipt->presented_hud_door_ready &&
                out_receipt->presented_hud_door_route_hash_ready &&
                out_receipt->presented_credits_ready &&
                out_receipt->presented_credits_route_hash_ready
            ? 1
            : 0;
    out_receipt->presented_wrapper_cleanup_ready =
        release_app_capture->host_route_wrappers_retired &&
                release_app_capture->no_loose_render_plan_exports &&
                release_app_capture->no_wrapper_fallback_routes &&
                release_app_capture->no_fallback_callbacks
            ? 1
            : 0;
    out_receipt->presented_runtime_capture_boundary_ready =
        out_receipt->release_app_capture_valid &&
                out_receipt->presented_route_aggregates_ready &&
                out_receipt->presented_wrapper_cleanup_ready &&
                release_app_capture->runtime_host_routes_ready &&
                release_app_capture->route_specific_host_consumers_ready &&
                release_app_capture->draw_consumes_receipt_only &&
                release_app_capture->input_consumes_receipt_only &&
                release_app_capture->no_fallback_callbacks &&
                release_app_capture->no_wrapper_fallback_routes
            ? 1
            : 0;
    out_receipt->release_app_capture_hash =
        release_app_capture->release_app_capture_hash;
    out_receipt->title_sequence_capture_hash =
        release_app_capture->title_sequence_capture_hash;
    out_receipt->hud_door_capture_hash =
        release_app_capture->hud_door_capture_hash;
    out_receipt->credits_capture_hash =
        release_app_capture->credits_packaged_capture_hash;
    out_receipt->presented_frame_route_hash =
        presented_facts->presented_frame_route_hash
            ? presented_facts->presented_frame_route_hash
            : release_app_capture->release_app_capture_hash;
    out_receipt->presented_frame_route_hash_ready =
        out_receipt->presented_frame_route_hash != 0u &&
                (out_receipt->presented_frame_route_hash ==
                     out_receipt->title_sequence_capture_hash ||
                 out_receipt->presented_frame_route_hash ==
                     out_receipt->hud_door_capture_hash ||
                 out_receipt->presented_frame_route_hash ==
                     out_receipt->credits_capture_hash ||
                 out_receipt->presented_frame_route_hash ==
                     out_receipt->release_app_capture_hash)
            ? 1
            : 0;
    out_receipt->presented_wrapper_cleanup_hash =
        release_app_capture->runtime_host_gate_hash;
    out_receipt->presented_wrapper_cleanup_hash *= 16777619u;
    out_receipt->presented_wrapper_cleanup_hash ^=
        release_app_capture->complete_support_hash;
    out_receipt->presented_wrapper_cleanup_hash *= 16777619u;
    out_receipt->presented_wrapper_cleanup_hash ^=
        (uint32_t)out_receipt->presented_wrapper_cleanup_ready;
    if (out_receipt->presented_wrapper_cleanup_hash == 0u) {
        out_receipt->presented_wrapper_cleanup_hash = 1u;
    }
    out_receipt->presented_frame_hash = presented_facts->presented_frame_hash;

    hash ^= out_receipt->release_app_capture_hash;
    hash *= 16777619u;
    hash ^= out_receipt->presented_frame_hash;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->running_from_macos_app_bundle;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->mac_window_capture_ready;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->presented_frame_captured;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->presented_frame_geometry_ready;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->presented_frame_pixels_ready;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->presented_frame_real_asset_ready;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->presented_frame_route_hash_ready;
    hash *= 16777619u;
    hash ^= out_receipt->presented_frame_route_hash;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->presented_title_sequence_ready;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->presented_title_phase_mask_ready;
    hash *= 16777619u;
    hash ^= out_receipt->title_sequence_capture_hash;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->presented_hud_door_ready;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->presented_hud_door_route_hash_ready;
    hash *= 16777619u;
    hash ^= out_receipt->hud_door_capture_hash;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->presented_credits_ready;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->presented_credits_route_hash_ready;
    hash *= 16777619u;
    hash ^= out_receipt->credits_capture_hash;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->presented_route_aggregates_ready;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->presented_wrapper_cleanup_ready;
    hash *= 16777619u;
    hash ^= out_receipt->presented_wrapper_cleanup_hash;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->presented_runtime_capture_boundary_ready;
    out_receipt->presented_app_capture_hash = hash ? hash : 1u;

    out_receipt->valid =
        out_receipt->presented_runtime_capture_boundary_ready &&
                out_receipt->running_from_macos_app_bundle &&
                out_receipt->mac_window_capture_ready &&
                out_receipt->presented_frame_captured &&
                out_receipt->presented_frame_geometry_ready &&
                out_receipt->presented_frame_pixels_ready &&
                out_receipt->presented_frame_real_asset_ready &&
                out_receipt->presented_frame_route_hash_ready &&
                out_receipt->presented_title_sequence_ready &&
                out_receipt->presented_title_phase_mask_ready &&
                out_receipt->presented_hud_door_ready &&
                out_receipt->presented_hud_door_route_hash_ready &&
                out_receipt->presented_credits_ready &&
                out_receipt->presented_credits_route_hash_ready &&
                out_receipt->credits_capture_hash != 0u &&
                out_receipt->presented_route_aggregates_ready &&
                out_receipt->presented_wrapper_cleanup_ready &&
                out_receipt->presented_wrapper_cleanup_hash != 0u &&
                out_receipt->presented_app_capture_hash != 0u
            ? 1
            : 0;
    /* ReDMCSB keeps the CSB title/HUD/door startup chain in TITLE.C F0437
     * and ENTRANCE.C F0441/F0806. CSBWin separates graphic archive reads
     * from host viewport presentation. This receipt is deliberately stricter
     * than the release-app route proof: it requires an actual Mac app window
     * frame carrying real CSB indexed pixels through one of the same
     * receipt-owned title/HUD/credits route hashes before capture is
     * promoted. */
    return out_receipt->valid;
}

int csb_v1_boot_startup_release_app_capture_receipt_from_complete_support_pc34(
    const CSB_V1_StartupCompleteSupportReceipt_PC34 *complete_support,
    CSB_V1_StartupReleaseAppCaptureReceipt_PC34 *out_receipt)
{
    const CSB_V1_BootStartupRuntimeHostCaptureGateReceipt_PC34 *host_gate;
    uint32_t hash = 2166136261u;
    int title_phase_i;

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
    out_receipt->full_runtime_real_asset_matched =
        complete_support->full_runtime.real_asset_matched ? 1 : 0;
    out_receipt->host_runtime_visual_real_asset_matched =
        host_gate->runtime_visual.real_asset_matched ? 1 : 0;
    out_receipt->real_startup_assets_bound =
        complete_support->real_startup_assets_bound &&
                host_gate->real_startup_assets_bound
            ? 1
            : 0;
    out_receipt->release_app_real_asset_capture_ready =
        complete_support->real_asset_matched &&
                out_receipt->full_runtime_real_asset_matched &&
                out_receipt->host_runtime_visual_real_asset_matched &&
                out_receipt->real_startup_assets_bound &&
                complete_support->real_startup_asset_binding_hash != 0u
            ? 1
            : 0;
    out_receipt->title_runtime_phase_mask =
        complete_support->title_runtime_phase_mask;
    out_receipt->title_runtime_expected_phase_mask =
        complete_support->title_runtime_expected_phase_mask;
    out_receipt->title_runtime_phase_hash_count =
        complete_support->title_runtime_phase_hash_count;
    /* ReDMCSB TITLE.C F0437 renders PRESENTS, CHAOS zoom/hold, then
     * STRIKES BACK as distinct startup phases; release capture keeps each
     * phase hash visible instead of accepting one collapsed title proof. */
    for (title_phase_i = 0;
         title_phase_i < CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34;
         ++title_phase_i) {
        out_receipt->title_runtime_phase_hashes[title_phase_i] =
            complete_support->title_runtime_phase_hashes[title_phase_i];
    }
    out_receipt->title_runtime_phase_hash =
        complete_support->title_runtime_phase_hash;
    out_receipt->title_packaged_capture_hash =
        host_gate->title_packaged_capture_hash;
    out_receipt->closed_door_packaged_capture_hash =
        host_gate->closed_door_packaged_capture_hash;
    out_receipt->utility_packaged_capture_hash =
        host_gate->utility_packaged_capture_hash;
    out_receipt->door_opening_packaged_capture_hash =
        host_gate->door_opening_packaged_capture_hash;
    out_receipt->credits_packaged_capture_hash =
        host_gate->credits_packaged_capture_hash;
    out_receipt->release_app_real_asset_capture_hash =
        complete_support->real_startup_asset_binding_hash;
    out_receipt->release_app_real_asset_capture_hash ^=
        (uint32_t)out_receipt->full_runtime_real_asset_matched;
    out_receipt->release_app_real_asset_capture_hash *= 16777619u;
    out_receipt->release_app_real_asset_capture_hash ^=
        (uint32_t)out_receipt->host_runtime_visual_real_asset_matched;
    out_receipt->release_app_real_asset_capture_hash *= 16777619u;
    if (out_receipt->release_app_real_asset_capture_hash == 0u) {
        out_receipt->release_app_real_asset_capture_hash = 1u;
    }
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
    /* ReDMCSB: ENTRANCE.C F0806 keeps credits inside the entrance loop via
     * M567_COMMAND_ENTRANCE_DRAW_CREDITS, so release/app capture must prove
     * the credits surface uses the same receipt-owned path as title/HUD/door. */
    out_receipt->credits_release_app_capture_ready =
        host_gate->credits_runtime_captured &&
                host_gate->credits_packaged_capture_hash != 0u
            ? 1
            : 0;
    out_receipt->title_host_consumer_ready =
        host_gate->title_host_ownership_valid &&
                host_gate->title_host_draw_consumes_receipt_only &&
                host_gate->title_host_input_consumes_receipt_only &&
                host_gate->title_packaged_capture_hash != 0u
            ? 1
            : 0;
    out_receipt->title_sequence_capture_ready =
        out_receipt->title_release_app_capture_ready &&
                out_receipt->title_phase_route_complete &&
                out_receipt->title_runtime_phase_mask ==
                    out_receipt->title_runtime_expected_phase_mask &&
                out_receipt->title_runtime_phase_hash_count ==
                    CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34 &&
                out_receipt->title_runtime_phase_hash != 0u
            ? 1
            : 0;
    out_receipt->title_sequence_host_consumer_ready =
        out_receipt->title_host_consumer_ready ? 1 : 0;
    out_receipt->title_sequence_same_capture_route =
        complete_support->playback_route_ready &&
                complete_support->title_to_hud_same_session &&
                out_receipt->title_sequence_capture_ready &&
                out_receipt->title_sequence_host_consumer_ready
            ? 1
            : 0;
    out_receipt->title_sequence_capture_hash =
        out_receipt->title_packaged_capture_hash;
    out_receipt->title_sequence_capture_hash *= 16777619u;
    out_receipt->title_sequence_capture_hash ^=
        out_receipt->title_runtime_phase_hash;
    out_receipt->title_sequence_capture_hash *= 16777619u;
    out_receipt->title_sequence_capture_hash ^=
        (uint32_t)out_receipt->title_runtime_phase_mask;
    out_receipt->title_sequence_capture_hash *= 16777619u;
    out_receipt->title_sequence_capture_hash ^=
        (uint32_t)out_receipt->title_runtime_phase_hash_count;
    out_receipt->title_sequence_capture_hash *= 16777619u;
    out_receipt->title_sequence_capture_hash ^=
        complete_support->playback_route_hash;
    if (out_receipt->title_sequence_capture_hash == 0u) {
        out_receipt->title_sequence_capture_hash = 1u;
    }
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
    out_receipt->credits_host_consumer_ready =
        host_gate->credits_host_ownership_valid &&
                host_gate->credits_host_draw_consumes_receipt_only &&
                host_gate->credits_host_input_consumes_receipt_only &&
                host_gate->credits_packaged_capture_hash != 0u
            ? 1
            : 0;
    out_receipt->route_specific_host_consumers_ready =
        out_receipt->title_host_consumer_ready &&
                out_receipt->closed_door_host_consumer_ready &&
                out_receipt->utility_host_consumer_ready &&
                out_receipt->door_opening_host_consumer_ready &&
                out_receipt->credits_host_consumer_ready
            ? 1
            : 0;
    out_receipt->hud_door_capture_ready =
        out_receipt->closed_door_release_app_capture_ready &&
                out_receipt->utility_release_app_capture_ready &&
                out_receipt->door_opening_release_app_capture_ready
            ? 1
            : 0;
    out_receipt->hud_door_host_consumers_ready =
        out_receipt->closed_door_host_consumer_ready &&
                out_receipt->utility_host_consumer_ready &&
                out_receipt->door_opening_host_consumer_ready
            ? 1
            : 0;
    out_receipt->hud_door_same_capture_route =
        complete_support->playback_route_ready &&
                complete_support->title_to_hud_same_session &&
                out_receipt->runtime_host_routes_ready &&
                out_receipt->hud_door_capture_ready &&
                out_receipt->hud_door_host_consumers_ready
            ? 1
            : 0;
    out_receipt->hud_door_capture_hash =
        out_receipt->closed_door_packaged_capture_hash;
    out_receipt->hud_door_capture_hash *= 16777619u;
    out_receipt->hud_door_capture_hash ^=
        out_receipt->utility_packaged_capture_hash;
    out_receipt->hud_door_capture_hash *= 16777619u;
    out_receipt->hud_door_capture_hash ^=
        out_receipt->door_opening_packaged_capture_hash;
    out_receipt->hud_door_capture_hash *= 16777619u;
    out_receipt->hud_door_capture_hash ^=
        complete_support->playback_route_hash;
    if (out_receipt->hud_door_capture_hash == 0u) {
        out_receipt->hud_door_capture_hash = 1u;
    }
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
    hash ^= out_receipt->credits_packaged_capture_hash;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->release_app_real_asset_capture_ready;
    hash *= 16777619u;
    hash ^= out_receipt->release_app_real_asset_capture_hash;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->title_phase_route_complete;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->title_runtime_phase_mask;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->title_runtime_expected_phase_mask;
    hash *= 16777619u;
    hash ^= out_receipt->title_runtime_phase_hash;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->title_runtime_phase_hash_count;
    hash *= 16777619u;
    for (title_phase_i = 0;
         title_phase_i < CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34;
         ++title_phase_i) {
        hash ^= out_receipt->title_runtime_phase_hashes[title_phase_i];
        hash *= 16777619u;
    }
    hash ^= (uint32_t)out_receipt->runtime_host_routes_ready;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->host_route_wrappers_retired;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->no_loose_render_plan_exports;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->route_specific_host_consumers_ready;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->title_sequence_capture_ready;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->title_sequence_host_consumer_ready;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->title_sequence_same_capture_route;
    hash *= 16777619u;
    hash ^= out_receipt->title_sequence_capture_hash;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->hud_door_capture_ready;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->hud_door_host_consumers_ready;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->hud_door_same_capture_route;
    hash *= 16777619u;
    hash ^= out_receipt->hud_door_capture_hash;
    out_receipt->release_app_capture_hash = hash ? hash : 1u;
    out_receipt->release_app_capture_ready =
        out_receipt->complete_support_valid &&
                out_receipt->host_capture_gate_valid &&
                out_receipt->title_release_app_capture_ready &&
                out_receipt->closed_door_release_app_capture_ready &&
                out_receipt->utility_release_app_capture_ready &&
                out_receipt->door_opening_release_app_capture_ready &&
                out_receipt->credits_release_app_capture_ready &&
                out_receipt->title_phase_route_complete &&
                out_receipt->title_runtime_phase_mask ==
                    out_receipt->title_runtime_expected_phase_mask &&
                out_receipt->title_runtime_phase_hash_count ==
                    CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34 &&
                out_receipt->title_runtime_phase_hash != 0u &&
                out_receipt->title_sequence_capture_ready &&
                out_receipt->title_sequence_host_consumer_ready &&
                out_receipt->title_sequence_same_capture_route &&
                out_receipt->title_sequence_capture_hash != 0u &&
                out_receipt->runtime_host_routes_ready &&
                out_receipt->draw_consumes_receipt_only &&
                out_receipt->input_consumes_receipt_only &&
                out_receipt->route_specific_host_consumers_ready &&
                out_receipt->hud_door_capture_ready &&
                out_receipt->hud_door_host_consumers_ready &&
                out_receipt->hud_door_same_capture_route &&
                out_receipt->hud_door_capture_hash != 0u &&
                out_receipt->no_fallback_callbacks &&
                out_receipt->no_wrapper_fallback_routes &&
                out_receipt->host_route_wrappers_retired &&
                out_receipt->no_loose_render_plan_exports &&
                out_receipt->release_app_real_asset_capture_ready &&
                out_receipt->release_app_real_asset_capture_hash != 0u &&
                out_receipt->real_startup_assets_bound &&
                out_receipt->release_app_capture_hash != 0u
            ? 1
            : 0;
    out_receipt->valid = out_receipt->release_app_capture_ready;
    /* ReDMCSB keeps title, entrance HUD and opening-door presentation under
     * TITLE.C F0437 lines 424-463, ENTRANCE.C F0441 lines 620-950 and
     * F0580/F0581 lines 1123-1165.  DUNVIEW.C wall/door tables around
     * lines 150-240 keep door/HUD ownership data-driven, so release capture
     * now requires each host route to consume its own receipt-owned package.
     * The release/app gate also requires real-data evidence from both the
     * full runtime session and host visual capture before it can stand in for
     * a Mac/app capture receipt. */
    return out_receipt->valid;
}

static int csb_v1_boot_startup_title_phase_hashes_distinct_pc34(
    const CSB_V1_StartupCompleteSupportReceipt_PC34 *receipt)
{
    int i;
    int j;

    if (!receipt || receipt->title_runtime_phase_hash_count !=
            CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34) return 0;
    for (i = 0; i < CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34; ++i) {
        if (receipt->title_runtime_phase_hashes[i] == 0u) return 0;
        for (j = 0; j < i; ++j) {
            if (receipt->title_runtime_phase_hashes[i] ==
                receipt->title_runtime_phase_hashes[j]) return 0;
        }
    }
    return 1;
}

int csb_v1_boot_startup_complete_support_receipt_from_runtime_and_host_pc34(
    const CSB_V1_StartupFullRuntimeReceipt_PC34 *full_runtime,
    const CSB_V1_BootStartupRuntimeHostCaptureGateReceipt_PC34 *host_capture_gate,
    CSB_V1_StartupCompleteSupportReceipt_PC34 *out_receipt)
{
    uint32_t hash = 2166136261u;
    int title_phase_i;

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
    out_receipt->playback_route_ready =
        full_runtime->playback_route_ready &&
                host_capture_gate->title_runtime_phase_route_complete &&
                host_capture_gate->closed_door_hud_runtime_captured &&
                host_capture_gate->utility_hud_runtime_captured &&
                host_capture_gate->door_opening_runtime_captured
            ? 1
            : 0;
    out_receipt->title_to_hud_same_session =
        full_runtime->title_to_hud_same_session &&
                out_receipt->playback_route_ready &&
                full_runtime->session_generation != 0u
            ? 1
            : 0;
    out_receipt->runtime_host_routes_ready =
        host_capture_gate->route_hardening_valid &&
                host_capture_gate->all_runtime_routes_consumed &&
                host_capture_gate->title_host_ownership_valid &&
                host_capture_gate->closed_door_host_ownership_valid &&
                host_capture_gate->utility_host_ownership_valid &&
                host_capture_gate->door_opening_host_ownership_valid &&
                host_capture_gate->credits_host_ownership_valid
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
    out_receipt->title_runtime_phase_mask =
        host_capture_gate->title_runtime_phase_mask;
    out_receipt->title_runtime_expected_phase_mask =
        host_capture_gate->title_runtime_expected_phase_mask;
    out_receipt->title_runtime_phase_hash_count =
        host_capture_gate->title_runtime_phase_hash_count;
    /* ReDMCSB TITLE.C F0437 has four visible title phases. Preserve the
     * per-phase hashes through complete-support so host/app gates cannot
     * collapse PRESENTS, CHAOS zoom, CHAOS hold, and STRIKES BACK. */
    for (title_phase_i = 0;
         title_phase_i < CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34;
         ++title_phase_i) {
        out_receipt->title_runtime_phase_hashes[title_phase_i] =
            host_capture_gate->title_runtime_phase_hashes[title_phase_i];
    }
    out_receipt->title_runtime_phase_hash =
        host_capture_gate->title_runtime_phase_hash;
    out_receipt->credits_packaged_capture_hash =
        host_capture_gate->credits_packaged_capture_hash;
    out_receipt->real_startup_asset_binding_hash =
        host_capture_gate->real_startup_asset_binding_hash;
    out_receipt->session_generation = full_runtime->session_generation;
    out_receipt->playback_route_hash = full_runtime->playback_route_hash;
    out_receipt->runtime_host_gate_hash =
        host_capture_gate->runtime_host_gate_hash;
    hash ^= full_runtime->session_generation;
    hash *= 16777619u;
    hash ^= host_capture_gate->runtime_host_gate_hash;
    hash *= 16777619u;
    hash ^= host_capture_gate->title_runtime_phase_hash;
    hash *= 16777619u;
    hash ^= (uint32_t)host_capture_gate->title_runtime_phase_hash_count;
    hash *= 16777619u;
    for (title_phase_i = 0;
         title_phase_i < CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34;
         ++title_phase_i) {
        hash ^= host_capture_gate->title_runtime_phase_hashes[title_phase_i];
        hash *= 16777619u;
    }
    hash ^= (uint32_t)host_capture_gate->title_runtime_phase_mask;
    hash *= 16777619u;
    hash ^= (uint32_t)host_capture_gate->title_runtime_expected_phase_mask;
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
    hash *= 16777619u;
    hash ^= full_runtime->playback_route_hash;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->playback_route_ready;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->title_to_hud_same_session;
    out_receipt->complete_support_hash = hash ? hash : 1u;
    out_receipt->valid =
        out_receipt->full_runtime_valid &&
                out_receipt->host_capture_gate_valid &&
                out_receipt->real_asset_matched &&
                out_receipt->title_sequence_ready &&
                out_receipt->title_runtime_phase_mask ==
                    out_receipt->title_runtime_expected_phase_mask &&
                out_receipt->title_runtime_phase_hash_count ==
                    CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34 &&
                csb_v1_boot_startup_title_phase_hashes_distinct_pc34(
                    out_receipt) &&
                out_receipt->title_runtime_phase_hash != 0u &&
                out_receipt->title_presents_ready &&
                out_receipt->title_chaos_ready &&
                out_receipt->title_strikes_back_ready &&
                out_receipt->entrance_ready &&
                out_receipt->hud_ready &&
                out_receipt->door_ready &&
                out_receipt->playback_route_ready &&
                out_receipt->title_to_hud_same_session &&
                out_receipt->playback_route_hash != 0u &&
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
