#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>
#include <string.h>

static int fetches;

static int fetch(void *user, int index, const uint8_t **pixels,
                 int *width, int *height, int *stride)
{
    static const uint8_t data[4] = { 1, 2, 3, 4 };
    (void)user; (void)index;
    ++fetches;
    *pixels = data; *width = 2; *height = 2; *stride = 2;
    return 0;
}

int main(void)
{
    DM2_V1_ViewportState viewport;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];

    memset(framebuffer, 0x5a, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    viewport.squares[DM2_SQ_D0C].flags = DM2_SQF_HAS_DOOR;
    viewport.squares[DM2_SQ_D0C].door_gfx_admitted = 1;
    dm2_v1_viewport_set_asset_provider(&viewport, fetch, NULL);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_render_doors(&viewport);
    if (fetches != 0 || viewport.asset_door_panel_drawn_count != 0 ||
        viewport.asset_door_frame_drawn_count != 0 ||
        (viewport.blocked_material_mask & DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR) == 0u) {
        fputs("FAIL: door accepted a graphics-set without a G1 scene receipt\n", stderr);
        return 1;
    }
    puts("PASS: source door requires live G1 graphics-set receipt");
    return 0;
}
