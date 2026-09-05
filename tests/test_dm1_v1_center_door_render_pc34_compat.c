#include "dm1_v1_center_door_render_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>

static int g_failed = 0;
static int g_passed = 0;

static void expect_int(const char* name, int got, int expected)
{
    if (got != expected) {
        printf("FAIL %s got=%d expected=%d\n", name, got, expected);
        ++g_failed;
        return;
    }
    printf("PASS %s == %d\n", name, expected);
    ++g_passed;
}

static void expect_blit(const char* name,
                        int depth,
                        int doorState,
                        int doorVertical,
                        int blitIndex,
                        int srcX,
                        int srcY,
                        int dstX,
                        int dstY,
                        int width,
                        int height)
{
    DM1_CenterDoorBlitPc34 blits[2];
    int count = dm1_v1_center_door_panel_blits_for_cell_pc34(
        depth,
        doorState,
        doorVertical,
        blits);
    expect_int(name, count > blitIndex ? 1 : 0, 1);
    if (count <= blitIndex) {
        return;
    }
    expect_int("blit.srcX", blits[blitIndex].srcX, srcX);
    expect_int("blit.srcY", blits[blitIndex].srcY, srcY);
    expect_int("blit.dstX", blits[blitIndex].dstX, dstX);
    expect_int("blit.dstY", blits[blitIndex].dstY, dstY);
    expect_int("blit.width", blits[blitIndex].width, width);
    expect_int("blit.height", blits[blitIndex].height, height);
}

int main(void)
{
    DM1_CenterDoorRenderPlanPc34 plan;
    DM1_CenterDoorBlitPc34 blits[2];
    int consumed = -1;
    struct DungeonMapDesc_Compat maps[2] = {{0}};
    struct DungeonDatState_Compat dungeon = {0};

    expect_int("plan.count", dm1_v1_center_door_render_plan_count_pc34(), 3);

    expect_int("plan.d1c", dm1_v1_center_door_render_plan_for_depth_pc34(0, &plan), 1);
    expect_int("plan.d1c.depth", plan.depthIndex, 0);
    expect_int("plan.d1c.relForward", plan.relForward, 1);
    expect_int("plan.d1c.relSide", plan.relSide, 0);
    expect_int("plan.d1c.frameCount", plan.frameCount, 3);
    expect_int("plan.d1c.frameA.graphic", plan.frameA.graphicIndex, 91);
    expect_int("plan.d1c.frameA.dstX", plan.frameA.dstX, 61);
    expect_int("plan.d1c.closed.graphic", plan.closedPanel.graphicIndex, 248);
    expect_int("plan.d1c.closed.width", plan.closedPanel.width, 96);
    expect_int("plan.d1c.closed.height", plan.closedPanel.height, 86);

    expect_int("plan.d2c", dm1_v1_center_door_render_plan_for_depth_pc34(1, &plan), 1);
    expect_int("plan.d2c.frameCount", plan.frameCount, 3);
    expect_int("plan.d2c.frameB.graphic", plan.frameB.graphicIndex, 88);
    expect_int("plan.d2c.frameB.dstX", plan.frameB.dstX, 65);
    expect_int("plan.d2c.closed.graphic", plan.closedPanel.graphicIndex, 247);

    expect_int("plan.d3c", dm1_v1_center_door_render_plan_for_depth_pc34(2, &plan), 1);
    expect_int("plan.d3c.frameCount", plan.frameCount, 2);
    expect_int("plan.d3c.frameA.graphic", plan.frameA.graphicIndex, 89);
    expect_int("plan.d3c.frameA.dstX", plan.frameA.dstX, 82);
    expect_int("plan.d3c.closed.graphic", plan.closedPanel.graphicIndex, 246);

    expect_blit("d1c.closed", 0, 4, 1, 0, 0, 0, 64, 16, 96, 86);
    expect_blit("d1c.vertical.opening", 0, 2, 1, 0, 0, 43, 64, 15, 96, 45);
    expect_blit("d2c.vertical.opening", 1, 2, 1, 0, 0, 29, 80, 24, 64, 32);
    expect_blit("d3c.horizontal.left", 2, 1, 0, 0, 18, 0, 88, 28, 6, 40);
    expect_blit("d3c.horizontal.right", 2, 1, 0, 1, 24, 0, 130, 28, 6, 40);

    expect_int("doorSet0.d1.graphic",
               dm1_v1_door_panel_graphic_for_set_depth_pc34(0, 0), 248);
    expect_int("doorSet0.d2.graphic",
               dm1_v1_door_panel_graphic_for_set_depth_pc34(0, 1), 247);
    expect_int("doorSet0.d3.graphic",
               dm1_v1_door_panel_graphic_for_set_depth_pc34(0, 2), 246);
    expect_int("doorSet2.d1.graphic",
               dm1_v1_door_panel_graphic_for_set_depth_pc34(2, 0), 254);
    expect_int("doorSet2.d3.graphic",
               dm1_v1_door_panel_graphic_for_set_depth_pc34(2, 2), 252);
    expect_int("ra.h", dm1_v1_f0111_animated_door_flip_mask_pc34(3, 4, 1, &consumed), 1);
    expect_int("ra.consume", consumed, 1);
    expect_int("ra.v", dm1_v1_f0111_animated_door_flip_mask_pc34(3, 2, 2, &consumed), 2);
    expect_int("ra.hv", dm1_v1_f0111_animated_door_flip_mask_pc34(3, 5, 3, &consumed), 3);
    expect_int("ordinary", dm1_v1_f0111_animated_door_flip_mask_pc34(2, 4, 3, &consumed), 0);
    expect_int("ordinary.consume", consumed, 0);
    expect_int("open.ra", dm1_v1_f0111_animated_door_flip_mask_pc34(3, 0, 3, &consumed), 0);
    expect_int("open.ra.consume", consumed, 0);

    /* The retail PC 3.4 DUNGEON.DAT descriptor stream contains both the
     * common 0/1 pair and a real DoorSet0=3 map (raw bitfield D 0x1300).
     * Lock F0174's Type-as-selector rule so that the latter reaches the Ra
     * graphics at M633 + 3*3 instead of silently reusing set 0. */
    dungeon.header.mapCount = 2;
    dungeon.maps = maps;
    maps[0].doorSet0 = 0;
    maps[0].doorSet1 = 1;
    maps[1].doorSet0 = 3;
    maps[1].doorSet1 = 1;
    expect_int("map0.type0.portcullis.d1",
               dm1_v1_door_panel_graphic_for_current_map_depth_pc34(
                   &dungeon, 0, 0, 0), 248);
    expect_int("map0.type1.wood.d3",
               dm1_v1_door_panel_graphic_for_current_map_depth_pc34(
                   &dungeon, 0, 1, 2), 249);
    expect_int("retail.map.type0.ra.d1",
               dm1_v1_door_panel_graphic_for_current_map_depth_pc34(
                   &dungeon, 1, 0, 0), 257);
    expect_int("retail.map.type0.ra.d3",
               dm1_v1_door_panel_graphic_for_current_map_depth_pc34(
                   &dungeon, 1, 0, 2), 255);
    expect_int("retail.map.type1.wood.d2",
               dm1_v1_door_panel_graphic_for_current_map_depth_pc34(
                   &dungeon, 1, 1, 1), 250);
    expect_int("bad.map", dm1_v1_door_panel_graphic_for_current_map_depth_pc34(
                   &dungeon, 2, 0, 0), -1);
    expect_int("bad.type", dm1_v1_door_panel_graphic_for_current_map_depth_pc34(
                   &dungeon, 0, 2, 0), -1);

    expect_int("bad.depth", dm1_v1_center_door_render_plan_for_depth_pc34(3, &plan), 0);
    expect_int("bad.index", dm1_v1_center_door_render_plan_at_pc34(-1, &plan), 0);
    expect_int("bad.null", dm1_v1_center_door_render_plan_for_depth_pc34(0, 0), 0);
    expect_int("open.panel", dm1_v1_center_door_panel_blits_for_cell_pc34(0, 0, 1, blits), 0);
    expect_int("bad.panel.depth", dm1_v1_center_door_panel_blits_for_cell_pc34(-1, 4, 1, blits), 0);
    expect_int("bad.panel.null", dm1_v1_center_door_panel_blits_for_cell_pc34(0, 4, 1, 0), 0);
    expect_int("bad.graphic.depth",
               dm1_v1_door_panel_graphic_for_set_depth_pc34(0, 3), -1);
    expect_int("bad.graphic.set",
               dm1_v1_door_panel_graphic_for_set_depth_pc34(-1, 0), -1);
    {
        DM1_CenterDoorHostMaterialReceiptPc34 receipt;
        expect_int("closed.host.receipt",
                   dm1_v1_center_door_host_material_receipt_pc34(
                       0, 4, 1, 248, &receipt), 1);
        expect_int("closed.host.panelVisible", receipt.panelVisible, 1);
        expect_int("open.host.receipt",
                   dm1_v1_center_door_host_material_receipt_pc34(
                       0, 0, 1, 248, &receipt), 1);
        expect_int("open.host.panelVisible", receipt.panelVisible, 0);
    }
    {
        int sx = -1;
        int sy = -1;
        expect_int("compose.clip.no_flip",
                   dm1_v1_f0111_composed_source_xy_pc34(
                       96, 88, 7, 43, 0, &sx, &sy), 1);
        expect_int("compose.clip.no_flip.x", sx, 7);
        expect_int("compose.clip.no_flip.y", sy, 43);
        expect_int("compose.clip.whole_hflip",
                   dm1_v1_f0111_composed_source_xy_pc34(
                       96, 88, 7, 43, 1, &sx, &sy), 1);
        expect_int("compose.clip.whole_hflip.x", sx, 88);
        expect_int("compose.clip.whole_hflip.y", sy, 43);
        expect_int("compose.clip.whole_vflip",
                   dm1_v1_f0111_composed_source_xy_pc34(
                       96, 88, 7, 43, 2, &sx, &sy), 1);
        expect_int("compose.clip.whole_vflip.x", sx, 7);
        expect_int("compose.clip.whole_vflip.y", sy, 44);
        expect_int("compose.clip.bad_coordinate",
                   dm1_v1_f0111_composed_source_xy_pc34(
                       96, 88, 96, 0, 0, &sx, &sy), 0);
    }

    printf("# passed=%d failed=%d\n", g_passed, g_failed);
    return g_failed == 0 ? 0 : 1;
}
