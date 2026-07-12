#include "dm1_v1_wall_ornament_pc34_compat.h"

#include <stdio.h>

static int g_passed;
static int g_failed;

static void expect_int(const char *name, int got, int want)
{
    if (got != want) {
        printf("FAIL %s got=%d want=%d\n", name, got, want);
        ++g_failed;
        return;
    }
    printf("PASS %s == %d\n", name, want);
    ++g_passed;
}

static void expect_zone(const char *name,
                        int coordSet,
                        int viewWallIndex,
                        int x,
                        int y,
                        int w,
                        int h)
{
    DM1_WallOrnamentZoneBlitPc34 blit;
    int ok = dm1_v1_wall_ornament_zone_pc34(
        coordSet, viewWallIndex, &blit);
    expect_int(name, ok, 1);
    if (!ok) {
        return;
    }
    expect_int("zone.srcX", blit.srcX, 0);
    expect_int("zone.srcY", blit.srcY, 0);
    expect_int("zone.dstX", blit.dstX, x);
    expect_int("zone.dstY", blit.dstY, y);
    expect_int("zone.width", blit.width, w);
    expect_int("zone.height", blit.height, h);
}

int main(void)
{
    DM1_WallOrnamentZoneBlitPc34 blit;
    DM1_WallOrnamentViewSpecPc34 spec;
    DM1_WallOrnamentRenderPlanPc34 plan;
    DM1_FrontMirrorRenderPlanPc34 mirrorPlan;

    /* ReDMCSB DUNVIEW.C G0194: wall ornament global index to G0205
     * coordinate-set index. */
    expect_int("coord_set.out_of_range.low",
               dm1_v1_wall_ornament_coord_set_index_pc34(-1), 0);
    expect_int("coord_set.0",
               dm1_v1_wall_ornament_coord_set_index_pc34(0), 1);
    expect_int("coord_set.11",
               dm1_v1_wall_ornament_coord_set_index_pc34(11), 2);
    expect_int("coord_set.43",
               dm1_v1_wall_ornament_coord_set_index_pc34(43), 5);
    expect_int("coord_set.59",
               dm1_v1_wall_ornament_coord_set_index_pc34(59), 7);
    expect_int("coord_set.out_of_range.high",
               dm1_v1_wall_ornament_coord_set_index_pc34(60), 0);

    /* ReDMCSB DUNVIEW.C G0205: {X1, X2, Y1, Y2, ByteWidth, Height};
     * destination width/height are derived from inclusive X/Y bounds. */
    expect_zone("zone.coord0.d1c",
                0, 12, 96, 36, 32, 28);
    expect_zone("zone.coord5.d1c_mirror",
                5, 12, 80, 29, 64, 43);
    expect_zone("zone.coord7.fullscreen_d1c",
                7, 12, 32, 9, 160, 111);
    expect_int("zone.null",
               dm1_v1_wall_ornament_zone_pc34(0, 0, NULL), 0);
    expect_int("zone.bad_coord",
               dm1_v1_wall_ornament_zone_pc34(-1, 0, &blit), 0);
    expect_int("zone.bad_view",
               dm1_v1_wall_ornament_zone_pc34(0, 13, &blit), 0);

    /* ReDMCSB DUNVIEW.C F0107 PC34/I34E right-side left-wall flips. */
    expect_int("flip.d3r_left",
               dm1_v1_wall_ornament_flip_horizontal_pc34(1), 1);
    expect_int("flip.d2r_left",
               dm1_v1_wall_ornament_flip_horizontal_pc34(6), 1);
    expect_int("flip.d1r_left",
               dm1_v1_wall_ornament_flip_horizontal_pc34(11), 1);
    expect_int("flip.front",
               dm1_v1_wall_ornament_flip_horizontal_pc34(12), 0);
    expect_int("flip.left_side",
               dm1_v1_wall_ornament_flip_horizontal_pc34(10), 0);

    /* ReDMCSB DUNVIEW.C G0192 + DUNGEON.C F0149 alcove indices. */
    expect_int("alcove.0",
               dm1_v1_wall_ornament_is_alcove_global_pc34(0), 0);
    expect_int("alcove.1",
               dm1_v1_wall_ornament_is_alcove_global_pc34(1), 1);
    expect_int("alcove.2",
               dm1_v1_wall_ornament_is_alcove_global_pc34(2), 1);
    expect_int("alcove.3",
               dm1_v1_wall_ornament_is_alcove_global_pc34(3), 1);
    expect_int("alcove.4",
               dm1_v1_wall_ornament_is_alcove_global_pc34(4), 0);

    /* DM1-owned F0107 projection list. */
    expect_int("view_spec.count",
               dm1_v1_wall_ornament_view_spec_count_pc34(), 13);
    expect_int("view_spec.null",
               dm1_v1_wall_ornament_view_spec_pc34(0, NULL), 0);
    expect_int("view_spec.bad_low",
               dm1_v1_wall_ornament_view_spec_pc34(-1, &spec), 0);
    expect_int("view_spec.bad_high",
               dm1_v1_wall_ornament_view_spec_pc34(13, &spec), 0);
    expect_int("view_spec.0.ok",
               dm1_v1_wall_ornament_view_spec_pc34(0, &spec), 1);
    expect_int("view_spec.0.forward", spec.relForward, 3);
    expect_int("view_spec.0.side", spec.relSide, -1);
    expect_int("view_spec.0.view", spec.viewWallIndex, 0);
    expect_int("view_spec.0.compact", spec.unreadableInscriptionCompactBox, 1);
    expect_int("view_spec.1.ok",
               dm1_v1_wall_ornament_view_spec_pc34(1, &spec), 1);
    expect_int("view_spec.1.forward", spec.relForward, 3);
    expect_int("view_spec.1.side", spec.relSide, 1);
    expect_int("view_spec.1.view", spec.viewWallIndex, 1);
    expect_int("view_spec.12.ok",
               dm1_v1_wall_ornament_view_spec_pc34(12, &spec), 1);
    expect_int("view_spec.12.forward", spec.relForward, 1);
    expect_int("view_spec.12.side", spec.relSide, 0);
    expect_int("view_spec.12.view", spec.viewWallIndex, 12);

    /* Render plans own native graphic binding, palette, transparency,
     * flip, and optional unreadable-inscription height clamp. */
    expect_int("plan.null",
               dm1_v1_wall_ornament_render_plan_pc34(0, 12, 0, NULL), 0);
    expect_int("plan.bad_global",
               dm1_v1_wall_ornament_render_plan_pc34(-1, 12, 0, &plan), 0);
    expect_int("plan.bad_view",
               dm1_v1_wall_ornament_render_plan_pc34(0, 13, 0, &plan), 0);

    expect_int("plan.inscription.d1c.ok",
               dm1_v1_wall_ornament_render_plan_pc34(0, 12, 0, &plan), 1);
    expect_int("plan.inscription.d1c.graphic", plan.graphicIndex, 260);
    expect_int("plan.inscription.d1c.dstX", plan.dstX, 64);
    expect_int("plan.inscription.d1c.dstY", plan.dstY, 36);
    expect_int("plan.inscription.d1c.width", plan.width, 96);
    expect_int("plan.inscription.d1c.height", plan.height, 56);
    expect_int("plan.inscription.d1c.transparent", plan.transparentColor, 10);
    expect_int("plan.inscription.d1c.flip", plan.flipHorizontal, 0);
    expect_int("plan.inscription.d1c.palette", plan.paletteMapValid, 1);
    expect_int("plan.inscription.d1c.palette1", plan.paletteMap[1], 12);
    expect_int("plan.inscription.d1c.palette14", plan.paletteMap[14], 14);

    expect_int("plan.inscription.clamp.ok",
               dm1_v1_wall_ornament_render_plan_pc34(0, 12, 9, &plan), 1);
    expect_int("plan.inscription.clamp.height", plan.height, 9);

    expect_int("plan.mirror.d1c.ok",
               dm1_v1_wall_ornament_render_plan_pc34(43, 12, 0, &plan), 1);
    expect_int("plan.mirror.d1c.graphic", plan.graphicIndex, 346);
    expect_int("plan.mirror.d1c.dstX", plan.dstX, 80);
    expect_int("plan.mirror.d1c.dstY", plan.dstY, 29);
    expect_int("plan.mirror.d1c.width", plan.width, 64);
    expect_int("plan.mirror.d1c.height", plan.height, 43);

    expect_int("plan.d3r_left.ok",
               dm1_v1_wall_ornament_render_plan_pc34(1, 1, 0, &plan), 1);
    expect_int("plan.d3r_left.graphic", plan.graphicIndex, 261);
    expect_int("plan.d3r_left.flip", plan.flipHorizontal, 1);
    expect_int("plan.d3r_left.palette2", plan.paletteMap[2], 12);
    expect_int("plan.d3r_left.alcove", plan.isAlcove, 1);

    expect_int("plan.d2r_left.ok",
               dm1_v1_wall_ornament_render_plan_pc34(1, 6, 0, &plan), 1);
    expect_int("plan.d2r_left.graphic", plan.graphicIndex, 261);
    expect_int("plan.d2r_left.flip", plan.flipHorizontal, 1);
    expect_int("plan.d2r_left.palette1", plan.paletteMap[1], 12);

    /* DUNVIEW.C:3913-3928 front champion mirror render plan:
     * C346 ornament + C026 atlas portrait inside G0109. */
    expect_int("mirror.plan.null",
               dm1_v1_front_mirror_render_plan_pc34(0, NULL), 0);
    expect_int("mirror.plan.bad_ordinal",
               dm1_v1_front_mirror_render_plan_pc34(-1, &mirrorPlan), 0);
    expect_int("mirror.plan.18.ok",
               dm1_v1_front_mirror_render_plan_pc34(18, &mirrorPlan), 1);
    expect_int("mirror.plan.18.orn_graphic",
               mirrorPlan.ornament.graphicIndex, 346);
    expect_int("mirror.plan.18.orn_dstX", mirrorPlan.ornament.dstX, 80);
    expect_int("mirror.plan.18.orn_dstY", mirrorPlan.ornament.dstY, 29);
    expect_int("mirror.plan.18.orn_w", mirrorPlan.ornament.width, 64);
    expect_int("mirror.plan.18.orn_h", mirrorPlan.ornament.height, 43);
    expect_int("mirror.plan.18.orn_transparent",
               mirrorPlan.ornament.transparentColor, 10);
    expect_int("mirror.plan.18.orn_palette1",
               mirrorPlan.ornament.paletteMap[1], 12);
    expect_int("mirror.plan.18.portrait_graphic",
               mirrorPlan.portraitGraphicIndex, 26);
    expect_int("mirror.plan.18.srcX", mirrorPlan.portraitSrcX, 64);
    expect_int("mirror.plan.18.srcY", mirrorPlan.portraitSrcY, 58);
    expect_int("mirror.plan.18.dstX", mirrorPlan.portraitDstX, 96);
    expect_int("mirror.plan.18.dstY", mirrorPlan.portraitDstY, 35);
    expect_int("mirror.plan.18.w", mirrorPlan.portraitWidth, 32);
    expect_int("mirror.plan.18.h", mirrorPlan.portraitHeight, 29);
    expect_int("mirror.plan.18.transparent",
               mirrorPlan.portraitTransparentColor, 1);
    expect_int("mirror.plan.18.backingX", mirrorPlan.backingDstX, 80);
    expect_int("mirror.plan.18.backingY", mirrorPlan.backingDstY, 29);
    expect_int("mirror.plan.18.backingW", mirrorPlan.backingWidth, 64);
    expect_int("mirror.plan.18.backingH", mirrorPlan.backingHeight, 43);

    printf("# passed=%d failed=%d\n", g_passed, g_failed);
    return g_failed == 0 ? 0 : 1;
}
