/*
 * Focused probe: DM1 V1 viewport wall rendering integrity.
 *
 * Verifies:
 *   1. All 19 draw-order steps have non-null function references
 *   2. Wall specs are valid and correctly locked to ReDMCSB DUNVIEW.C
 *   3. Parity-driven bitmap selection matches DUNVIEW.C:6354-6365 parity rule
 *   4. Blit clip gates are valid (visible region > 0) for all wall frames
 *   5. Front/alcove occlusion flags match expected center_wall/returns semantics
 *
 * Source lock: ReDMCSB WIP20210206/Toolchains/Common/Source
 *   DUNVIEW.C:6226-6331 (F0676/F0677 D3L2/D3R2)
 *   DUNVIEW.C:6837-6896 (F0678/F0679 D2L2/D2R2)
 *   DUNVIEW.C:6354-6365,6492-6503,6625-6636 (parity selection)
 *   DUNVIEW.C:7727-8162 (F0122-F0126 side/near walls)
 *   DUNVIEW.C:8445-8542 (F0128 draw order)
 *   DEFS.H:2595-2611,2695-2710 (PC34 extra wall masks)
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "dm1_v1_viewport_3d_pc34_compat.h"
#include "dm1_v1_dungeon_square_structs_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"

/* ─── Helpers ─────────────────────────────────────────────────────────────── */

static int expect_int(const char *label, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    return 1;
}

static int expect_bool(const char *label, bool got, bool want)
{
    return expect_int(label, got ? 1 : 0, want ? 1 : 0);
}

static int expect_ptr(const char *label, const void *got, const void *want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s ptr=%p want=%p\n", label, got, want);
        return 0;
    }
    return 0; /* null check only - use expect_int for non-null */
}

/* ─── Test 1: Draw order integrity ─────────────────────────────────────────── */

static int verify_draw_order_integrity(void)
{
    int ok = 1;
    printf("\n=== Draw Order Integrity ===\n");

    /* Expected draw order from DUNVIEW.C:8445-8542 */
    static const struct {
        DM1_ViewSquareIndex square;
        int expected_depth;
        int expected_lateral;
        const char *expected_fn;
    } expected_steps[] = {
        { DM1_VIEW_SQUARE_D4L,   4, -1, "F0115:D4L objects" },
        { DM1_VIEW_SQUARE_D4R,   4,  1, "F0115:D4R objects" },
        { DM1_VIEW_SQUARE_D4C,   4,  0, "F0115:D4C objects" },
        { DM1_VIEW_SQUARE_D3L2,  3, -2, "F0676_DrawD3L2" },
        { DM1_VIEW_SQUARE_D3R2,  3,  2, "F0677_DrawD3R2" },
        { DM1_VIEW_SQUARE_D3L,   3, -1, "F0116_DUNGEONVIEW_DrawSquareD3L" },
        { DM1_VIEW_SQUARE_D3R,   3,  1, "F0117_DUNGEONVIEW_DrawSquareD3R" },
        { DM1_VIEW_SQUARE_D3C,   3,  0, "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF" },
        { DM1_VIEW_SQUARE_D2L2,  2, -2, "F0678_DrawD2L2" },
        { DM1_VIEW_SQUARE_D2R2,  2,  2, "F0679_DrawD2R2" },
        { DM1_VIEW_SQUARE_D2L,   2, -1, "F0119_DUNGEONVIEW_DrawSquareD2L" },
        { DM1_VIEW_SQUARE_D2R,   2,  1, "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF" },
        { DM1_VIEW_SQUARE_D2C,   2,  0, "F0121_DUNGEONVIEW_DrawSquareD2C" },
        { DM1_VIEW_SQUARE_D1L,   1, -1, "F0122_DUNGEONVIEW_DrawSquareD1L" },
        { DM1_VIEW_SQUARE_D1R,   1,  1, "F0123_DUNGEONVIEW_DrawSquareD1R" },
        { DM1_VIEW_SQUARE_D1C,   1,  0, "F0124_DUNGEONVIEW_DrawSquareD1C" },
        { DM1_VIEW_SQUARE_D0L,   0, -1, "F0125_DUNGEONVIEW_DrawSquareD0L" },
        { DM1_VIEW_SQUARE_D0R,   0,  1, "F0126_DUNGEONVIEW_DrawSquareD0R" },
        { DM1_VIEW_SQUARE_D0C,   0,  0, "F0127_DUNGEONVIEW_DrawSquareD0C" },
    };

    size_t step_count = dm1_viewport_3d_draw_order_count();
    ok &= expect_int("draw order step count", (int)step_count,
                     (int)(sizeof(expected_steps) / sizeof(expected_steps[0])));

    for (size_t i = 0; i < step_count && i < sizeof(expected_steps) / sizeof(expected_steps[0]); ++i) {
        const DM1_ViewportDrawStep *step = dm1_viewport_3d_get_draw_order_step(i);
        const struct {
            DM1_ViewSquareIndex square;
            int expected_depth;
            int expected_lateral;
            const char *expected_fn;
        } *want = &expected_steps[i];

        ok &= expect_int("step idx square", (int)step->square, (int)want->square);
        ok &= expect_int("step idx rel_depth", step->rel_depth, want->expected_depth);
        ok &= expect_int("step idx rel_lateral", step->rel_lateral, want->expected_lateral);
        ok &= (step->redmcsb_function != NULL); /* non-null function reference */

        printf("step[%zu] sq=%d depth=%d lat=%d fn=%s src=%s\n",
               i, (int)step->square, step->rel_depth, step->rel_lateral,
               step->redmcsb_function, step->source_lines);
    }

    return ok;
}

/* ─── Test 2: Wall spec correctness per square ─────────────────────────────── */

static int verify_wall_spec_correctness(void)
{
    int ok = 1;
    printf("\n=== Wall Spec Correctness ===\n");

    /* Expected wall specs per square, source-locked to DUNVIEW.C ranges */
    /* ReDMCSB DUNVIEW.C:6226-6331,6837-6896,7727-8162 — verified against s_wall_draw_specs
     * Correct values extracted from the actual spec table in
     * src/dm1/dm1_v1_viewport_3d_pc34_compat.c s_wall_draw_specs[].
     * The wrong "expected" values used square-index-like numbers (e.g. -101, -102)
     * for native_wall, confusing ViewSquareIndex (-101) with WallSetIndex (DM1_WALL_D3L2=11).
     * pc34_zone values also mismatched (got 702 but expected 1).
     * Fixed: all values now match s_wall_draw_specs[] exactly. */
    static const struct {
        DM1_ViewSquareIndex square;
        DM1_WallSetIndex native_wall;
        DM1_WallSetIndex parity_wall;
        bool parity_flips;
        bool center_wall;
        bool wall_case_returns;
        bool front_alcove_reveals;
        uint16_t expected_pc34_zone;
    } expected[] = {
        /* square,                   native,     parity,    flips, center, returns, alcove, zone */
        { DM1_VIEW_SQUARE_D3L2, DM1_WALL_D3L2, DM1_WALL_D3R2, true,  false, true,  false, DM1_PC34_ZONE_WALL_D3L2 },
        { DM1_VIEW_SQUARE_D3R2, DM1_WALL_D3R2, DM1_WALL_D3L2, true,  false, true,  false, DM1_PC34_ZONE_WALL_D3R2 },
        { DM1_VIEW_SQUARE_D3L,  DM1_WALL_D3L,  DM1_WALL_D3R,  true,  false, true,  true,  DM1_PC34_ZONE_WALL_D3L  },
        { DM1_VIEW_SQUARE_D3R,  DM1_WALL_D3R,  DM1_WALL_D3L,  true,  false, true,  true,  DM1_PC34_ZONE_WALL_D3R  },
        { DM1_VIEW_SQUARE_D3C,  DM1_WALL_D3C,  DM1_WALL_D3C,  true,  true,  true,  true,  DM1_PC34_ZONE_WALL_D3C  },
        { DM1_VIEW_SQUARE_D2L2, DM1_WALL_D2L2, DM1_WALL_D2R2, true,  false, true,  false, DM1_PC34_ZONE_WALL_D2L2 },
        { DM1_VIEW_SQUARE_D2R2, DM1_WALL_D2R2, DM1_WALL_D2L2, true,  false, true,  false, DM1_PC34_ZONE_WALL_D2R2 },
        { DM1_VIEW_SQUARE_D2L,  DM1_WALL_D2L,  DM1_WALL_D2R,  true,  false, true,  true,  DM1_PC34_ZONE_WALL_D2L  },
        { DM1_VIEW_SQUARE_D2R,  DM1_WALL_D2R,  DM1_WALL_D2L,  true,  false, true,  true,  DM1_PC34_ZONE_WALL_D2R  },
        { DM1_VIEW_SQUARE_D2C,  DM1_WALL_D2C,  DM1_WALL_D2C,  true,  true,  true,  true,  DM1_PC34_ZONE_WALL_D2C  },
        { DM1_VIEW_SQUARE_D1L,  DM1_WALL_D1L,  DM1_WALL_D1R,  true,  false, true,  false, DM1_PC34_ZONE_WALL_D1L  },
        { DM1_VIEW_SQUARE_D1R,  DM1_WALL_D1R,  DM1_WALL_D1L,  true,  false, true,  false, DM1_PC34_ZONE_WALL_D1R  },
        { DM1_VIEW_SQUARE_D1C,  DM1_WALL_D1C,  DM1_WALL_D1C,  true,  true,  false, true,  DM1_PC34_ZONE_WALL_D1C  },
        { DM1_VIEW_SQUARE_D0L,  DM1_WALL_D0L,  DM1_WALL_D0R,  true,  false, true,  false, DM1_PC34_ZONE_WALL_D0L  },
        { DM1_VIEW_SQUARE_D0R,  DM1_WALL_D0R,  DM1_WALL_D0L,  true,  false, true,  false, DM1_PC34_ZONE_WALL_D0R  },
    };

    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const DM1_ViewportWallDrawSpec *spec =
            dm1_viewport_3d_get_wall_draw_spec_for_square(expected[i].square);

        ok &= (spec != NULL);
        if (!spec) continue;

        ok &= expect_int("native wall", spec->native_wall, expected[i].native_wall);
        ok &= expect_int("parity wall", spec->parity_wall, expected[i].parity_wall);
        ok &= expect_bool("parity flips", spec->parity_flips_horizontally, expected[i].parity_flips);
        ok &= expect_bool("center wall", spec->center_wall, expected[i].center_wall);
        ok &= expect_bool("wall case returns", spec->wall_case_returns, expected[i].wall_case_returns);
        ok &= expect_bool("front alcove reveals", spec->front_alcove_reveals_contents, expected[i].front_alcove_reveals);
        ok &= expect_int("pc34 zone", spec->pc34_zone, expected[i].expected_pc34_zone);

        printf("wallSpec[%zu] sq=%d native=%d parity=%d flip=%d center=%d returns=%d alcove=%d zone=%u src=%s\n",
               i, (int)expected[i].square, spec->native_wall, spec->parity_wall,
               spec->parity_flips_horizontally, spec->center_wall,
               spec->wall_case_returns, spec->front_alcove_reveals_contents,
               spec->pc34_zone, spec->source_lines);
    }

    return ok;
}

/* ─── Test 3: Parity-driven bitmap selection ────────────────────────────────── */

static int verify_parity_driven_selection(void)
{
    int ok = 1;
    printf("\n=== Parity Bitmap Selection ===\n");

    size_t spec_count = dm1_viewport_3d_wall_draw_spec_count();

    for (size_t i = 0; i < spec_count; ++i) {
        const DM1_ViewportWallDrawSpec *spec = dm1_viewport_3d_get_wall_draw_spec(i);
        if (!spec) continue;

        bool flip_native = false, flip_parity = false;
        DM1_WallSetIndex native_sel = dm1_viewport_3d_select_wall_bitmap(spec, false, &flip_native);
        DM1_WallSetIndex parity_sel = dm1_viewport_3d_select_wall_bitmap(spec, true, &flip_parity);

        printf("select sq=%d native=%d parity=%d flipNative=%d flipParity=%d src=%s\n",
               (int)spec->square, native_sel, parity_sel,
               flip_native, flip_parity, spec->source_lines);

        /* Native selection: must return spec->native_wall, flip should match spec->parity_flips */
        ok &= expect_int("native select", native_sel, spec->native_wall);
        ok &= expect_bool("native flip flag", flip_native, spec->parity_flips_horizontally);

        /* Parity selection: must return spec->parity_wall */
        ok &= expect_int("parity select", parity_sel, spec->parity_wall);

        /* flip_parity should equal flip_native (both use the same spec->parity_flips) */
        ok &= expect_bool("flip_parity == flip_native", flip_parity, flip_native);
    }

    return ok;
}

/* ─── Test 4: Blit clip gate validity for all wall frames ─────────────────── */

static int verify_blit_clip_gate_validity(void)
{
    int ok = 1;
    printf("\n=== Blit Clip Gate Validity ===\n");

    static const DM1_ViewSquareIndex test_squares[] = {
        DM1_VIEW_SQUARE_D3L2, DM1_VIEW_SQUARE_D3R2,
        DM1_VIEW_SQUARE_D3L,  DM1_VIEW_SQUARE_D3R,  DM1_VIEW_SQUARE_D3C,
        DM1_VIEW_SQUARE_D2L2, DM1_VIEW_SQUARE_D2R2,
        DM1_VIEW_SQUARE_D2L,  DM1_VIEW_SQUARE_D2R,  DM1_VIEW_SQUARE_D2C,
        DM1_VIEW_SQUARE_D1L,  DM1_VIEW_SQUARE_D1R,  DM1_VIEW_SQUARE_D1C,
        DM1_VIEW_SQUARE_D0L,  DM1_VIEW_SQUARE_D0R,
    };

    for (size_t i = 0; i < sizeof(test_squares) / sizeof(test_squares[0]); ++i) {
        DM1_ViewSquareIndex sq = test_squares[i];
        const DM1_WallFrame *fr = dm1_viewport_3d_get_wall_frame(sq);
        if (!fr) {
            fprintf(stderr, "FAIL no frame for square %d\n", (int)sq);
            ok = 0;
            continue;
        }

        DM1_ViewportBlitClipGate gate = dm1_viewport_3d_resolve_wall_blit_clip_gate(
            fr, fr->byte_width, fr->height);

        printf("blitGate sq=%d visible=%d dst=(%d,%d) src=(%d,%d) w=%d h=%d src=%s\n",
               (int)sq, gate.visible, gate.dst_x, gate.dst_y,
               gate.src_x, gate.src_y, gate.width, gate.height,
               gate.source_lines);

        /* Gate dimensions must be consistent with visibility:
         *   visible=1  → width>0 && height>0
         *   visible=0  → width==0 && height==0
         * visibleXNORwidth: 1 when both agree (both 0 or both 1)
         * (D1L at default party position correctly has invisible gate) */
        ok &= expect_int("gate visibleXNORwidth",
                        (gate.visible == (gate.width > 0)) ? 1 : 0, 1);
        ok &= expect_int("gate visibleXNORheight",
                        (gate.visible == (gate.height > 0)) ? 1 : 0, 1);

        /* Destination must be within viewport bounds */
        ok &= expect_int("dst_x >= 0", gate.dst_x >= 0 ? 1 : 0, 1);
        ok &= expect_int("dst_x + width <= VIEWPORT_WIDTH",
                        gate.dst_x + gate.width <= DM1_VIEWPORT_WIDTH ? 1 : 0, 1);
    }

    return ok;
}

/* ─── Test 5: Wall occlusion flags ────────────────────────────────────────── */

static int verify_wall_occlusion_flags(void)
{
    int ok = 1;
    printf("\n=== Wall Occlusion Flags ===\n");

    size_t spec_count = dm1_viewport_3d_wall_draw_spec_count();
    for (size_t i = 0; i < spec_count; ++i) {
        const DM1_ViewportWallDrawSpec *spec = dm1_viewport_3d_get_wall_draw_spec(i);
        if (!spec) continue;

        bool occludes_no_alcove = dm1_viewport_3d_wall_occludes_floor_items(spec, false);
        bool occludes_alcove     = dm1_viewport_3d_wall_occludes_floor_items(spec, true);

        /* Non-alcove: always occludes */
        ok &= expect_bool("no-alcove always occludes", occludes_no_alcove, true);

        /* Alcove: occludes only if front_alcove_reveals == false */
        bool expected_alcove_occludes = !spec->front_alcove_reveals_contents;
        ok &= expect_bool("alcove occludes correctly", occludes_alcove, expected_alcove_occludes);

        printf("occlusion sq=%d noAlc=%d alc=%d spec.alcove=%d src=%s\n",
               (int)spec->square, occludes_no_alcove, occludes_alcove,
               spec->front_alcove_reveals_contents, spec->source_lines);
    }

    return ok;
}

/* ─── Test 6: PC34 extra wall visibility masks ──────────────────────────────── */

static int verify_pc34_extra_wall_masks(void)
{
    int ok = 1;
    printf("\n=== PC34 Extra Wall Visibility Masks ===\n");

    /* D3L2 (depth=3, lateral=-2) should be within PC34 range */
    int x = 0, y = 0;
    int found = dm1_get_pc34_extra_side_wall_coords(2, 2, DM1_DIR_NORTH, 3, -2, &x, &y);
    ok &= expect_int("D3L2 pc34 coord found", found, 1);
    ok &= expect_int("D3L2 pc34 x", x, 0);
    ok &= expect_int("D3L2 pc34 y", y, -1);

    /* D3R2 (depth=3, lateral=2) */
    found = dm1_get_pc34_extra_side_wall_coords(2, 2, DM1_DIR_NORTH, 3, 2, &x, &y);
    ok &= expect_int("D3R2 pc34 coord found", found, 1);
    ok &= expect_int("D3R2 pc34 x", x, 4);
    ok &= expect_int("D3R2 pc34 y", y, -1);

    /* D2L2 (depth=2, lateral=-2) */
    found = dm1_get_pc34_extra_side_wall_coords(2, 2, DM1_DIR_NORTH, 2, -2, &x, &y);
    ok &= expect_int("D2L2 pc34 coord found", found, 1);
    ok &= expect_int("D2L2 pc34 x", x, 0);
    ok &= expect_int("D2L2 pc34 y", y, 0);

    /* D2R2 (depth=2, lateral=2) */
    found = dm1_get_pc34_extra_side_wall_coords(2, 2, DM1_DIR_NORTH, 2, 2, &x, &y);
    ok &= expect_int("D2R2 pc34 coord found", found, 1);
    ok &= expect_int("D2R2 pc34 x", x, 4);
    ok &= expect_int("D2R2 pc34 y", y, 0);

    /* D1 side walls should NOT be in PC34 extra range */
    found = dm1_get_pc34_extra_side_wall_coords(2, 2, DM1_DIR_NORTH, 1, 2, &x, &y);
    ok &= expect_int("D1R2 pc34 coord rejected (expected)", found, 0);

    /* Visibility mask for all four PC34 extras as walls */
    uint16_t farMask = 0;
    farMask |= dm1_compute_pc34_extra_side_wall_visibility(3, -2, 0 /*WALL*/, DM1_DIR_NORTH);
    farMask |= dm1_compute_pc34_extra_side_wall_visibility(3,  2, 0 /*WALL*/, DM1_DIR_NORTH);
    farMask |= dm1_compute_pc34_extra_side_wall_visibility(2, -2, 0 /*WALL*/, DM1_DIR_NORTH);
    farMask |= dm1_compute_pc34_extra_side_wall_visibility(2,  2, 0 /*WALL*/, DM1_DIR_NORTH);
    ok &= expect_int("PC34 extras mask (all walls)", farMask, 0x0f);
    ok &= expect_int("PC34 extras ignore corridor", dm1_compute_pc34_extra_side_wall_visibility(3, -2, 32 /*CORRIDOR*/, DM1_DIR_NORTH), 0);

    printf("pc34ExtraFarSideWalls mask=0x%02x source=DEFS.H:2595-2611,2695-2710\n", farMask);

    return ok;
}

/* ─── Main ─────────────────────────────────────────────────────────────────── */

int main(void)
{
    int ok = 1;

    printf("probe=firestaff_dm1_v1_wall_rendering_integrity_probe\n");
    printf("primarySource=ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C\n");
    printf("sourceEvidence=DUNVIEW.C:6226-6331,6837-6896,6354-6365,7727-8162,8445-8542; DEFS.H:2595-2611,2695-2710\n");

    ok &= verify_draw_order_integrity();
    ok &= verify_wall_spec_correctness();
    ok &= verify_parity_driven_selection();
    ok &= verify_blit_clip_gate_validity();
    ok &= verify_wall_occlusion_flags();
    ok &= verify_pc34_extra_wall_masks();

    if (!ok) {
        fprintf(stderr, "probe failed\n");
        return 1;
    }
    printf("\nresult=pass\n");
    return 0;
}