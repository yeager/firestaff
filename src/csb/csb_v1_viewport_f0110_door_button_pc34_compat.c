#include "csb_v1_viewport_f0110_door_button_pc34_compat.h"
#include "dm1_v1_viewport_3d_pc34_compat.h"

#include <string.h>

/*
 * Source-locked to ReDMCSB DUNVIEW.C F0110_DUNGEONVIEW_DrawDoorButton
 * lines 4119-4216.
 *
 * F0110 receives a 1-based door_button_ordinal (0 = no button) and a
 * view_door_button_index (C0=D3R, C1=D3C, C2=D2C, C3=D1C).  It
 * decrements the ordinal, fetches the native bitmap at
 * ordinal + M634_GRAPHIC_FIRST_DOOR_BUTTON, and selects the coordinate
 * set from G0208[G0197[ordinal]][viewIndex].
 *
 * At D1C (depth 1 center): native bitmap, no palette remap, copies
 * coordinate set to clickable box C05.
 * At D2C: derived (scaled) bitmap with palette remap G0199.
 * At D3R/D3C: derived (scaled) bitmap with palette remap G0198.
 * Transparency color: C10_COLOR_FLESH (10) in all cases.
 */

static const char s_source_evidence[] =
    "ReDMCSB DUNVIEW.C F0110_DUNGEONVIEW_DrawDoorButton lines 4119-4216. "
    "F0110 decrements door_button_ordinal (1-based, 0=none), fetches native "
    "bitmap at ordinal-1 + M634_GRAPHIC_FIRST_DOOR_BUTTON. "
    "G0208[G0197[ordinal]][viewIndex] selects coordinate set: "
    "D3R={199,204,41,44,8,4}, D3C={136,141,41,44,8,4}, "
    "D2C={144,155,42,47,8,6}, D1C={160,175,44,52,8,9}. "
    "At D1C: native bitmap, coord set copied to clickable box C05. "
    "At D2/D3: derived bitmap with palette remap (G0199 for D2, G0198 for D3). "
    "Transparency: C10_COLOR_FLESH (10). "
    "DEFS.H:2088 anchors C10_COLOR_FLESH. "
    "DUNVIEW.C:1210-1216 anchors G0208 coordinate sets.";

/* Depth lookup: D3R=3, D3C=3, D2C=2, D1C=1 */
static const int s_view_index_to_depth[DM1_VIEW_DOOR_BUTTON_COUNT] = {
    3, /* D3R */
    3, /* D3C */
    2, /* D2C */
    1  /* D1C */
};

void csb_v1_viewport_f0110_door_button_plan_pc34(
    int door_button_ordinal,
    int view_door_button_index,
    CSB_V1_ViewportDoorButtonPlanPc34 *out_plan)
{
    memset(out_plan, 0, sizeof(*out_plan));
    out_plan->source_evidence = s_source_evidence;

    if (door_button_ordinal <= 0) return;
    if (view_door_button_index < 0 ||
        view_door_button_index >= DM1_VIEW_DOOR_BUTTON_COUNT) return;

    DM1_ViewDoorButtonIndex vi = (DM1_ViewDoorButtonIndex)view_door_button_index;

    /* Delegate to DM1 for the coordinate set (G0208). */
    const DM1_WallFrame *frame =
        dm1_v1_viewport_get_door_button_frame_pc34(door_button_ordinal, vi);
    if (!frame) return;

    /* Delegate to DM1 for the palette remap (G0198/G0199). */
    const uint8_t *remap =
        dm1_v1_viewport_get_door_button_palette_remap_pc34(vi);

    out_plan->valid = true;
    out_plan->ordinal = door_button_ordinal;
    out_plan->view_index = view_door_button_index;
    out_plan->depth = s_view_index_to_depth[view_door_button_index];
    out_plan->coord_x1 = frame->left_x;
    out_plan->coord_x2 = frame->right_x;
    out_plan->coord_y1 = frame->top_y;
    out_plan->coord_y2 = frame->bottom_y;
    out_plan->byte_width = frame->byte_width;
    out_plan->height = frame->height;

    /* ReDMCSB F0110:4181-4192: D1C uses native bitmap; D2/D3 use derived. */
    out_plan->needs_scaling = (vi != DM1_VIEW_DOOR_BUTTON_D1C);

    /* ReDMCSB F0110:4159-4163: bitmap index = (ordinal-1) + M634. */
    out_plan->native_bitmap_index = door_button_ordinal - 1;

    out_plan->transparent_color = CSB_V1_F0110_DOOR_BUTTON_PC34_TRANSPARENT_COLOR;

    /* ReDMCSB F0110:4181-4184: D1C copies coord set to clickable box C05. */
    out_plan->is_clickable = (vi == DM1_VIEW_DOOR_BUTTON_D1C);

    out_plan->palette_remap = remap;
}

const void *csb_v1_viewport_f0110_door_button_coord_set_pc34(
    int door_button_ordinal,
    int view_door_button_index)
{
    if (door_button_ordinal <= 0) return NULL;
    if (view_door_button_index < 0 ||
        view_door_button_index >= DM1_VIEW_DOOR_BUTTON_COUNT) return NULL;

    return dm1_v1_viewport_get_door_button_frame_pc34(
        door_button_ordinal, (DM1_ViewDoorButtonIndex)view_door_button_index);
}

int csb_v1_viewport_f0110_door_button_raster_pc34(
    const uint8_t *source_pixels, int source_width, int source_height,
    int view_door_button_index, uint8_t *destination, int dst_stride,
    int viewport_width, int viewport_height)
{
    CSB_V1_ViewportDoorButtonPlanPc34 plan;
    const DM1_WallFrame *zone;
    int dst_width, dst_height, written = 0, y;
    uint32_t x_step = 0u, y_step = 0u, y_position;

    if (!source_pixels || source_width <= 0 || source_height <= 0 ||
        !destination || dst_stride < viewport_width || viewport_width <= 0 ||
        viewport_height <= 0) return 0;
    csb_v1_viewport_f0110_door_button_plan_pc34(
        1, view_door_button_index, &plan);
    zone = (const DM1_WallFrame *)
        csb_v1_viewport_f0110_door_button_coord_set_pc34(
            1, view_door_button_index);
    if (!plan.valid || !zone) return 0;
    dst_width = (int)zone->right_x - (int)zone->left_x + 1;
    dst_height = (int)zone->bottom_y - (int)zone->top_y + 1;
    if (dst_width <= 0 || dst_height <= 0 ||
        zone->left_x < 0 || zone->top_y < 0 ||
        zone->right_x >= viewport_width || zone->bottom_y >= viewport_height)
        return 0;

    if (view_door_button_index != DM1_VIEW_DOOR_BUTTON_D1C) {
        /* ReDMCSB BLTSHRNK.C F0129 uses a 6.6 source step and starts each
         * axis at half one source step (F0720/F0721); its G0198/G0199 map is
         * applied after sampling. */
        x_step = (uint32_t)(((uint64_t)(unsigned int)source_width << 6) /
                            (unsigned int)dst_width);
        y_step = (uint32_t)(((uint64_t)(unsigned int)source_height << 6) /
                            (unsigned int)dst_height);
        if (x_step == 0u || y_step == 0u) return 0;
    } else if (dst_width > source_width || dst_height > source_height) {
        return 0;
    }

    y_position = (view_door_button_index == DM1_VIEW_DOOR_BUTTON_D1C)
        ? 0u : y_step >> 1;
    for (y = 0; y < dst_height; ++y) {
        uint32_t sy = view_door_button_index == DM1_VIEW_DOOR_BUTTON_D1C
            ? (uint32_t)y : y_position >> 6;
        uint32_t x_position = view_door_button_index == DM1_VIEW_DOOR_BUTTON_D1C
            ? 0u : x_step >> 1;
        int x;
        if (sy >= (uint32_t)source_height) return 0;
        for (x = 0; x < dst_width; ++x) {
            uint32_t sx = view_door_button_index == DM1_VIEW_DOOR_BUTTON_D1C
                ? (uint32_t)x : x_position >> 6;
            uint8_t pixel;
            if (sx >= (uint32_t)source_width) return 0;
            pixel = source_pixels[(size_t)sy * (size_t)source_width + sx];
            if (pixel != (uint8_t)plan.transparent_color) {
                if (plan.palette_remap)
                    pixel = plan.palette_remap[pixel & 0x0fu];
                destination[(size_t)(zone->top_y + y) * (size_t)dst_stride +
                            (size_t)zone->left_x + (size_t)x] =
                    (uint8_t)(0x10u | (pixel & 0x0fu));
                ++written;
            }
            if (view_door_button_index != DM1_VIEW_DOOR_BUTTON_D1C)
                x_position += x_step;
        }
        if (view_door_button_index != DM1_VIEW_DOOR_BUTTON_D1C)
            y_position += y_step;
    }
    return written;
}
