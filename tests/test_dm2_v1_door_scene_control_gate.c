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
    DM2_V1_DoorRenderPlan plan;
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

    /* A live source scene but absent GDAT/RAW4 owner used to route the door
     * through a guessed compatibility rectangle. SKWIN DRAW_DOOR has no
     * such substitute; the plan must be empty until the source owner exists. */
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    viewport.squares[DM2_SQ_D0C].flags = DM2_SQF_HAS_DOOR;
    viewport.squares[DM2_SQ_D0C].door_gfx_admitted = 1;
    viewport.squares[DM2_SQ_D0C].door_direct_g1_root = 1;
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, 1, 0x53434e45u,
        0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u);
    if (!dm2_v1_viewport_build_door_render_plan(&viewport, &plan) ||
        plan.door_count != 0) {
        fputs("FAIL: source door accepted guessed RAW4 placement\n", stderr);
        return 1;
    }
    puts("PASS: source door without RAW4 owner is no-draw");
    return 0;
}
