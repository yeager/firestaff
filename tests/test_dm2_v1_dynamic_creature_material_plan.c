#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>
#include <string.h>

static int fail(const char *message)
{
    fprintf(stderr, "FAIL: %s\n", message);
    return 1;
}

int main(void)
{
    DM2_V1_ViewportState viewport;
    DM2_V1_CreatureRenderPlan plan;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    viewport.creature_count = 1;
    viewport.creatures[0].creature_type = 7;
    viewport.creatures[0].frame_index = 9;
    viewport.creatures[0].direction = 2;
    viewport.creatures[0].source_kind = 1;
    viewport.creatures[0].source_material_proven = 1;
    viewport.creatures[0].gdat_image_field = 0x12;
    viewport.creatures[0].source_material_hash = 0x43524541u;
    viewport.creatures[0].screen_x = DM2_VP_WIDTH / 2;
    viewport.creatures[0].screen_y = DM2_VP_HEIGHT / 2;

    if (!dm2_v1_viewport_build_creature_render_plan(&viewport, &plan) ||
        plan.creature_count != 1) {
        return fail("dynamic creature plan was not built");
    }
    if (plan.creatures[0].gdat_index !=
            dm2_v1_viewport_creature_field_graphic_index(7, 0x12) ||
        plan.creatures[0].material_frame_index != 0 ||
        plan.creatures[0].frame_index != 9) {
        return fail("dynamic creature did not retain its FD image owner");
    }

    viewport.creatures[0].source_material_proven = 0;
    if (!dm2_v1_viewport_build_creature_render_plan(&viewport, &plan) ||
        plan.creature_count != 1 || plan.creatures[0].gdat_index != 0) {
        return fail("unproven dynamic creature received a drawable key");
    }
    puts("PASS: live creature plan consumes only a proven direct GDAT image field");
    return 0;
}
