#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>

static int check_source(int square, int side, int field, int rect,
                        int mirror, int offset_x)
{
    int got_field, got_rect, got_mirror, got_x, got_y;
    return dm2_v1_viewport_door_side_frame_source(
               square, side, &got_field, &got_rect, &got_mirror, &got_x,
               &got_y) &&
           got_field == field && got_rect == rect && got_mirror == mirror &&
           got_x == offset_x && got_y == 4;
}

int main(void)
{
    DM2_V1_ViewportState viewport;
    DM2_V1_DoorRenderPlan plan;
    unsigned char framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];

    if (!check_source(DM2_SQ_D0C, 0, 0xd3, 5010, 0, -2) ||
        !check_source(DM2_SQ_D0C, 1, 0xd4, 5014, 1, 2) ||
        !check_source(DM2_SQ_D1C, 0, 0x07, 5085, 0, -2) ||
        !check_source(DM2_SQ_D1C, 1, 0x08, 5089, 1, 2) ||
        !check_source(DM2_SQ_D2C, 0, 0x09, 5160, 0, -2) ||
        !check_source(DM2_SQ_D2C, 1, 0x0a, 5164, 1, 2) ||
        !check_source(DM2_SQ_D3C, 0, 0x0b, 5285, 0, -2) ||
        !check_source(DM2_SQ_D3C, 1, 0x0c, 5289, 1, 2)) {
        fputs("skproject side-frame table mismatch\n", stderr);
        return 1;
    }
    {
        int field, rect, mirror, offset_x, offset_y;
        if (!dm2_v1_viewport_door_side_frame_source_for_movement(
                DM2_SQ_D1C, 0, 1, &field, &rect, &mirror, &offset_x,
                &offset_y) || field != 0x08 || rect != 5085 || mirror != 0 ||
            offset_x != -2 || offset_y != 4 ||
            !dm2_v1_viewport_door_side_frame_source_for_movement(
                DM2_SQ_D1C, 1, 1, &field, &rect, &mirror, &offset_x,
                &offset_y) || field != 0x07 || rect != 5089 || mirror != 1 ||
            offset_x != 2 || offset_y != 4) {
            fputs("skproject moving side-frame table mismatch\n", stderr);
            return 1;
        }
    }
    {
        int a, b, c, d, e;
        if (dm2_v1_viewport_door_side_frame_source(DM2_SQ_D1L, 0,
                                                    &a, &b, &c, &d, &e)) {
            fputs("unsupported side square admitted\n", stderr);
            return 1;
        }
    }
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    viewport.squares[DM2_SQ_D1C].flags = DM2_SQF_HAS_DOOR;
    if (!dm2_v1_viewport_build_door_render_plan(&viewport, &plan) ||
        plan.door_count != 1 ||
        plan.doors[0].side_frame_graphicsset_field[0] != 0x07 ||
        plan.doors[0].side_frame_graphicsset_field[1] != 0x08 ||
        plan.doors[0].side_frame_rect_number[0] != 5085 ||
        plan.doors[0].side_frame_rect_number[1] != 5089) {
        fputs("side-frame route did not reach door plan\n", stderr);
        return 1;
    }
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, 1, 0x534b504au, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    viewport.squares[DM2_SQ_D3C].flags = DM2_SQF_HAS_DOOR;
    viewport.squares[DM2_SQ_D2C].flags = DM2_SQF_HAS_DOOR;
    viewport.squares[DM2_SQ_D1C].flags = DM2_SQF_HAS_DOOR;
    viewport.squares[DM2_SQ_D0C].flags = DM2_SQF_HAS_DOOR;
    viewport.squares[DM2_SQ_D3C].door_gfx_admitted = 1;
    viewport.squares[DM2_SQ_D2C].door_gfx_admitted = 1;
    viewport.squares[DM2_SQ_D1C].door_gfx_admitted = 1;
    viewport.squares[DM2_SQ_D0C].door_gfx_admitted = 1;
    /* A source-required viewport needs the authenticated RAW4 panel
     * rectangle before it can draw a door.  This fixture deliberately owns
     * only scheduler flags, so admitting a guessed box would be a rendering
     * regression.  The source pass table remains independently observable. */
    if (!dm2_v1_viewport_build_door_render_plan(&viewport, &plan) ||
        plan.door_count != 0 ||
        dm2_v1_viewport_draw_dungeon_tiles_pass_for_cell(11) != 9 ||
        dm2_v1_viewport_draw_dungeon_tiles_pass_for_cell(6) != 14 ||
        dm2_v1_viewport_draw_dungeon_tiles_pass_for_cell(3) != 17 ||
        dm2_v1_viewport_draw_dungeon_tiles_pass_for_cell(0) >= 0) {
        fputs("source door scheduler fail-closed route mismatch\n", stderr);
        return 1;
    }
    puts("dm2 side-frame source route passed");
    return 0;
}
