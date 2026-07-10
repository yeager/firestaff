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

int csb_v1_boot_startup_runtime_surfaces_materialize_pc34(
    const CSB_V1_BootProfile *profile, const CSB_V1_StartupRenderPlan_PC34 *plan,
    CSB_V1_StartupRuntimeSurfaceSet_PC34 *out)
{
    unsigned char *title = NULL;
    unsigned char *left = NULL;
    unsigned char *right = NULL;
    int title_w, title_h, left_w, left_h, right_w, right_h;
    int ok = 0;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!profile || !plan || !profile->assets_verified || !profile->graphics_verified ||
        !profile->startup_assets.real_graphics_available ||
        !csb_v1_boot_startup_render_plan_uses_real_assets_pc34(profile, plan)) return 0;
    if (!csb_v1_startup_surface_load_graphic_pc34(profile->graphics_path, 1u,
            &title, &title_w, &title_h)) goto done;
    out->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34].pixels = title;
    out->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34].width = title_w;
    out->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34].height = title_h;
    out->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34].source_asset_id = 1;
    out->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34].valid = 1;
    title = NULL;
    if (!csb_v1_startup_surface_crop_pc34(&out->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_PRESENTS_PC34],
            out->surfaces[0].pixels, title_w, title_h, 1, 0, 137, 320, 16, -1) ||
        !csb_v1_startup_surface_crop_pc34(&out->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_CHAOS_PC34],
            out->surfaces[0].pixels, title_w, title_h, 1, 0, 0, 320, 80, -1) ||
        !csb_v1_startup_surface_crop_pc34(&out->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_STRIKES_BACK_PC34],
            out->surfaces[0].pixels, title_w, title_h, 1, 0, 80, 320, 57, 0)) goto done;
    out->title_regions_ready = 1;
    if (plan->surface == CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34 &&
        plan->opening_door_valid) {
        if (!csb_v1_startup_surface_load_graphic_pc34(profile->graphics_path, 2u, &left, &left_w, &left_h) ||
            !csb_v1_startup_surface_load_graphic_pc34(profile->graphics_path, 3u, &right, &right_w, &right_h) ||
            !csb_v1_startup_surface_crop_pc34(&out->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34], left, left_w, left_h, 2,
                plan->opening_left_source_x, plan->opening_left_source_y, plan->opening_left_w, plan->opening_left_h, -1) ||
            !csb_v1_startup_surface_crop_pc34(&out->surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34], right, right_w, right_h, 3,
                plan->opening_right_source_x, plan->opening_right_source_y, plan->opening_right_w, plan->opening_right_h, -1)) goto done;
        out->opening_frame_ready = 1;
    }
    out->real_asset_matched = 1;
    out->valid = 1;
    ok = 1;
done:
    free(title); free(left); free(right);
    if (!ok) csb_v1_boot_startup_runtime_surface_set_release_pc34(out);
    return ok;
}
