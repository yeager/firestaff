/*
 * DM1 V1 door-button D1C "pressed/unpressed" frame transition gate.
 *
 * Isolates the D1C wall-button blit (the only D1 view cell that F0110
 * draws, the closest visible door button, the only entry in
 * s_door_button_frames that is wider than 8 px) and pins down the
 * "press/unpress" frame transition at the compat layer:
 *
 *   - press frame   : door_button_ordinal == M000_INDEX_TO_ORDINAL(C0_DOOR_BUTTON)
 *                     == 1, the C0 button is the only C001_DOOR_BUTTON_COUNT
 *                     defined for DM1 in DEFS.H:2432, and F0110_P0122_i_DoorButtonOrdinal
 *                     is what F0110 compares against zero at DUNVIEW.C:4159.
 *   - unpress frame : door_button_ordinal == 0 — F0110 early-exits without
 *                     touching the G0296 viewport, which is the literal
 *                     "no button drawn" output state.
 *
 * Source-locked references (ReDMCSB WIP20210206/Toolchains/Common/Source):
 *
 *   DUNVIEW.C:1210-1216 G0208_aaauc_Graphic558_DoorButtonCoordinateSets —
 *       the one DM1 C0_DOOR_BUTTON coordinate set, in view-index order
 *       (C0 D3R, C1 D3C, C2 D2C, C3 D1C).  The D1C row is
 *       { 160, 175, 44, 52, 8, 9 } — left/right 160/175 (width 16),
 *       top/bottom 44/52 (height 9), byte_width 8, blit 9 lines.
 *
 *   DUNVIEW.C:4119-4207 F0110_DUNGEONVIEW_DrawDoorButton — the
 *       if (P0122_i_DoorButtonOrdinal) gate at line 4159 produces the
 *       unpress output; line 4163 selects the C0 view coordinate set
 *       G0208[0][P0123_i_ViewDoorButtonIndex]; line 4166 updates the
 *       G0291_aauc_DungeonViewClickableBoxes[C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT]
 *       rectangle for the D1C branch only (the door-button clickable
 *       target).  The MEDIA529 blit at line 4210 routes
 *       F0791_DUNGEONVIEW_DrawBitmapXX with C10_COLOR_FLESH
 *       transparency.
 *
 *   CLIKVIEW.C:311-385 F0377_COMMAND_ProcessType80_ClickInDungeonView —
 *       the player mouse-button "press" against the D1C wall button.
 *       Line 365 hits C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT inside
 *       G2210_aai_XYZ_DungeonViewClickable (PC34), plays C01_SOUND_SWITCH,
 *       then F0268_SENSOR_AddEvent(C10_EVENT_DOOR, …, C02_EFFECT_TOGGLE).
 *       The "press" therefore flips the door from CLOSED (button visible)
 *       to OPEN (button not redrawn), and back — the canonical "pressed /
 *       unpressed" frame transition observed at the viewport.
 *
 *   DEFS.H:2432 C001_DOOR_BUTTON_COUNT 1 — only one door button ordinal
 *       exists in DM1, so ordinal > 1 must remain an unpress output.
 *
 * The probe exercises the two output states of the D1C blit directly
 * (press / unpress) and the corresponding frame lookup, so any
 * regression that reorders s_door_button_frames, widens / narrows the
 * D1C row, or weakens the M000_INDEX_TO_ORDINAL(0) early-exit will fail
 * here without needing real DM1 game data.
 *
 * The known route pinned by this gate is the D1C center wall button on
 * the front-D1 depth — the only depth-1 view cell that calls
 * F0110_DUNGEONVIEW_DrawDoorButton from F0124_DUNGEONVIEW_DrawSquareD1C
 * (DUNVIEW.C:7902), and the same cell whose G0291 clickable box the
 * F0377 press flow resolves against.  D1L, D1R, D2C, D2L, D2R, D3C,
 * D3L, D3R are not exercised here — those have their own dedicated
 * tests (tests/test_dm1_v1_door_button_viewport_pc34_compat.c).
 */

#include "dm1_v1_viewport_3d_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* D1C blit rectangle: matches G0208[0][C3_VIEW_DOOR_BUTTON_D1C] bytes
 * { 160, 175, 44, 52, 8, 9 }.  Left/right 160..175, top/bottom 44..52.
 * Source bitmap is pre-scaled 16x9 (cf. m11_draw_dm1_center_door_buttons
 * in m11_game_view.c, which feeds the same destination 16x9 from the
 * 8x9 native door-button graphic at slot M11_GFX_DOOR_BUTTON_BASE).
 * 48x60 fits the 16-wide D1C span and leaves 4-px right/bottom gutters
 * to verify outside-blit is untouched. */
enum {
    DST_W = 48,
    DST_H = 60,
    DST_SIZE = DST_W * DST_H,
    D1C_DEST_LEFT = 12,    /* X1 — frame.left_x mapping */
    D1C_DEST_TOP  = 7,     /* Y1 — frame.top_y mapping  */
    D1C_DEST_WIDTH  = 16,  /* X2 - X1 + 1                */
    D1C_DEST_HEIGHT = 9,   /* Y2 - Y1 + 1                */
    SEED_PIXEL = 0xFFu,
    OPAQUE_PIXEL = 0x37u,
    TRANSPARENT_COLOR = 10 /* C10_COLOR_FLESH, F0110 line 4204-4207 */
};

static int g_failures = 0;

static void check_int(const char *id, int got, int want)
{
    if (got != want) {
        printf("FAIL %s got=%d want=%d\n", id, got, want);
        ++g_failures;
    } else {
        printf("PASS %s == %d\n", id, want);
    }
}

static void check_contains(const char *id, const char *hay, const char *needle)
{
    int ok = (hay != NULL) && (needle != NULL) && (strstr(hay, needle) != NULL);
    if (!ok) {
        printf("FAIL %s missing substring: %s\n", id, needle ? needle : "(null)");
        ++g_failures;
    } else {
        printf("PASS %s contains %s\n", id, needle);
    }
}

static void seed_dst(uint8_t *dst)
{
    /* Seed with SEED_PIXEL so any untouched pixel stays 0xFF, and any
     * blit source pixel < 16 with C10 transparency skipped lands on
     * SEED_PIXEL too. */
    for (int i = 0; i < DST_SIZE; ++i) dst[i] = SEED_PIXEL;
}

static int dst_pixel(const uint8_t *dst, int x, int y)
{
    if (x < 0 || x >= DST_W || y < 0 || y >= DST_H) return -1;
    return dst[y * DST_W + x];
}

/* Build a 16x9 pre-scaled D1C span.  Mirrors the format
 * s_door_button_frames[DM1_VIEW_DOOR_BUTTON_D1C] = { 160, 175, 44, 52, 8, 9 }
 * from src/dm1/dm1_v1_viewport_3d_pc34_compat.c, with the destination
 * rectangle translated into the local DST_W x DST_H buffer.
 *
 * Layout: bytes [y*16+x] are 10 (transparent) everywhere except
 * (sx, sy) = (3, 2) which carries OPAQUE_PIXEL.  That puts the
 * visible pixel at destination (D1C_DEST_LEFT+3, D1C_DEST_TOP+2) —
 * off the rectangle corner, well inside the 16x9 blit, and far from
 * both the top/left edges (to keep the C10 skip observable on
 * (D1C_DEST_LEFT+0, D1C_DEST_TOP+0)) and the right/bottom edges (to
 * keep the clipped-source guard meaningful for D2C in the sibling
 * probe). */
static void build_d1c_span(DM1_DoorButtonBitmapSpan *span,
                           const uint8_t **pixels_out)
{
    static uint8_t d1c_pixels[D1C_DEST_WIDTH * D1C_DEST_HEIGHT];

    for (int i = 0; i < D1C_DEST_WIDTH * D1C_DEST_HEIGHT; ++i) {
        d1c_pixels[i] = (uint8_t)TRANSPARENT_COLOR;
    }
    d1c_pixels[2 * D1C_DEST_WIDTH + 3] = OPAQUE_PIXEL; /* (3, 2) inside */

    span->frame.left_x  = (uint8_t)D1C_DEST_LEFT;
    span->frame.right_x = (uint8_t)(D1C_DEST_LEFT + D1C_DEST_WIDTH - 1);
    span->frame.top_y   = (uint8_t)D1C_DEST_TOP;
    span->frame.bottom_y = (uint8_t)(D1C_DEST_TOP + D1C_DEST_HEIGHT - 1);
    span->frame.byte_width = (uint8_t)D1C_DEST_WIDTH;
    span->frame.height = (uint8_t)D1C_DEST_HEIGHT;
    span->frame.blit_x = 0;
    span->frame.blit_y = 0;
    span->pixels = d1c_pixels;
    span->source_width = (int16_t)D1C_DEST_WIDTH;
    span->source_height = (int16_t)D1C_DEST_HEIGHT;

    *pixels_out = d1c_pixels;
}

/* ReDMCSB DUNVIEW.C:1210-1216 G0208[0][C3_VIEW_DOOR_BUTTON_D1C] — the
 * only D1 view cell in the C0_DOOR_BUTTON coordinate set, the most
 * prominent door-button blit, and the only entry whose destination
 * width is 16 (8 src bytes × 2 scale).  This pins down that the
 * helper exposes exactly the {160, 175, 44, 52, 8, 9} bytes for the
 * D1C slot. */
static void verify_d1c_frame_lookup(void)
{
    const DM1_WallFrame *frame =
        dm1_v1_viewport_get_door_button_frame_pc34(1, DM1_VIEW_DOOR_BUTTON_D1C);

    /* ReDMCSB: DUNVIEW.C:1215 G0208[0][C3] = { 160, 175, 44, 52, 8, 9 }. */
    check_int("d1c.lookup.nonnull", frame != NULL, 1);
    check_int("d1c.lookup.x1",  frame ? frame->left_x   : -1, 160);
    check_int("d1c.lookup.x2",  frame ? frame->right_x  : -1, 175);
    check_int("d1c.lookup.y1",  frame ? frame->top_y    : -1,  44);
    check_int("d1c.lookup.y2",  frame ? frame->bottom_y : -1,  52);
    check_int("d1c.lookup.byte_width", frame ? frame->byte_width : 0, 8);
    check_int("d1c.lookup.height",    frame ? frame->height     : 0, 9);
}

/* ReDMCSB F0110 line 4159 `if (P0122_i_DoorButtonOrdinal)` is the
 * unpress-output early exit; the helper mirrors it as
 * `if (door_button_ordinal <= 0) return 0;`.  Ordinal == 0 is the
 * post-press open state (the button is no longer drawn once the
 * door toggles open), ordinal == -1 is a defensive lower bound, and
 * ordinal == 2 must remain an unpress output because
 * C001_DOOR_BUTTON_COUNT == 1 (DEFS.H:2432) — only C0 exists. */
static void verify_unpress_output_for_invalid_ordinals(
    uint8_t *dst,
    const DM1_DoorButtonBitmapSpan *spans)
{
    /* Ordinal 0 — canonical "unpress" output: F0110 line 4159 early
     * exit.  D1C span is valid; result must be 0 pixels written. */
    seed_dst(dst);
    check_int("unpress.ordinal_zero.pixels_drawn",
              dm1_v1_viewport_draw_door_button_pc34(dst, DST_W, DST_H, DST_W,
                                                    0, DM1_VIEW_DOOR_BUTTON_D1C,
                                                    spans, DM1_VIEW_DOOR_BUTTON_COUNT),
              0);
    check_int("unpress.ordinal_zero.dest_untouched",
              dst_pixel(dst, D1C_DEST_LEFT + 3, D1C_DEST_TOP + 2),
              SEED_PIXEL);

    /* Ordinal -1 — defensive lower bound; the helper rejects <= 0. */
    seed_dst(dst);
    check_int("unpress.ordinal_negative.pixels_drawn",
              dm1_v1_viewport_draw_door_button_pc34(dst, DST_W, DST_H, DST_W,
                                                    -1, DM1_VIEW_DOOR_BUTTON_D1C,
                                                    spans, DM1_VIEW_DOOR_BUTTON_COUNT),
              0);
    check_int("unpress.ordinal_negative.dest_untouched",
              dst_pixel(dst, D1C_DEST_LEFT + 3, D1C_DEST_TOP + 2),
              SEED_PIXEL);

    /* Ordinal 2 — no second door button exists in DM1.  Lookup
     * returns NULL so the helper must also refuse to draw. */
    {
        const DM1_WallFrame *frame_ordinal2 =
            dm1_v1_viewport_get_door_button_frame_pc34(2, DM1_VIEW_DOOR_BUTTON_D1C);
        check_int("unpress.ordinal_two.lookup_null", frame_ordinal2 == NULL, 1);

        seed_dst(dst);
        check_int("unpress.ordinal_two.pixels_drawn",
                  dm1_v1_viewport_draw_door_button_pc34(dst, DST_W, DST_H, DST_W,
                                                        2, DM1_VIEW_DOOR_BUTTON_D1C,
                                                        spans, DM1_VIEW_DOOR_BUTTON_COUNT),
                  0);
        check_int("unpress.ordinal_two.dest_untouched",
                  dst_pixel(dst, D1C_DEST_LEFT + 3, D1C_DEST_TOP + 2),
                  SEED_PIXEL);
    }
}

/* ReDMCSB F0110 line 4204-4207 routes the D1C branch through the
 * C10_COLOR_FLESH transparent blit: the opaque pixel at (3, 2) lands
 * at (D1C_DEST_LEFT+3, D1C_DEST_TOP+2); C10 (color 10) is preserved
 * as the seeded SEED_PIXEL; everything else outside the 16x9
 * rectangle stays seeded.  This is the "press" output state of the
 * D1C frame transition. */
static void verify_d1c_press_blit(uint8_t *dst,
                                  const DM1_DoorButtonBitmapSpan *spans)
{
    int written;

    seed_dst(dst);
    written = dm1_v1_viewport_draw_door_button_pc34(
        dst, DST_W, DST_H, DST_W, 1, DM1_VIEW_DOOR_BUTTON_D1C,
        spans, DM1_VIEW_DOOR_BUTTON_COUNT);

    check_int("press.d1c.pixels_drawn", written, 1);
    check_int("press.d1c.opaque_at_d1c_3_2",
              dst_pixel(dst, D1C_DEST_LEFT + 3, D1C_DEST_TOP + 2),
              OPAQUE_PIXEL);
    /* C10 transparent guard at the rectangle corner. */
    check_int("press.d1c.transparent_at_corner_0_0",
              dst_pixel(dst, D1C_DEST_LEFT, D1C_DEST_TOP),
              SEED_PIXEL);
    /* Opposite corner stays transparent (still inside the 16x9
     * rectangle, source pixel at (15, 8) == C10). */
    check_int("press.d1c.transparent_at_corner_15_8",
              dst_pixel(dst, D1C_DEST_LEFT + D1C_DEST_WIDTH - 1,
                            D1C_DEST_TOP + D1C_DEST_HEIGHT - 1),
              SEED_PIXEL);
    /* Just outside the rectangle, top-left and bottom-right gutters. */
    check_int("press.d1c.untouched_left_gutter",
              dst_pixel(dst, D1C_DEST_LEFT - 1, D1C_DEST_TOP + 2),
              SEED_PIXEL);
    check_int("press.d1c.untouched_right_gutter",
              dst_pixel(dst, D1C_DEST_LEFT + D1C_DEST_WIDTH, D1C_DEST_TOP + 2),
              SEED_PIXEL);
    check_int("press.d1c.untouched_top_gutter",
              dst_pixel(dst, D1C_DEST_LEFT + 3, D1C_DEST_TOP - 1),
              SEED_PIXEL);
    check_int("press.d1c.untouched_bottom_gutter",
              dst_pixel(dst, D1C_DEST_LEFT + 3, D1C_DEST_TOP + D1C_DEST_HEIGHT),
              SEED_PIXEL);
}

/* ReDMCSB DUNVIEW.C:1215 G0208[0][C3] defines one D1C row, but
 * DM1_VIEW_DOOR_BUTTON_COUNT == 4.  Out-of-range view_index must be
 * rejected at both the lookup and the blit to keep the press/unpress
 * transition from accidentally drawing on D2C/D3C/D3R for an invalid
 * view. */
static void verify_invalid_view_index_rejected(void)
{
    int neg_view = -1;
    int over_view = DM1_VIEW_DOOR_BUTTON_COUNT;
    int over_view_far = DM1_VIEW_DOOR_BUTTON_COUNT + 5;

    check_int("view.negative.lookup_null",
              dm1_v1_viewport_get_door_button_frame_pc34(
                  1, (DM1_ViewDoorButtonIndex)neg_view) == NULL,
              1);
    check_int("view.count.lookup_null",
              dm1_v1_viewport_get_door_button_frame_pc34(
                  1, (DM1_ViewDoorButtonIndex)over_view) == NULL,
              1);
    check_int("view.far.lookup_null",
              dm1_v1_viewport_get_door_button_frame_pc34(
                  1, (DM1_ViewDoorButtonIndex)over_view_far) == NULL,
              1);
}

/* The press/unpress transition is driven by the door-button click
 * event flow in CLIKVIEW.C:311-385.  Anchor the probe to the
 * viewport's source_evidence string so the F0124 D1C anchor and the
 * door-front occlusion references for D1C (DUNVIEW.C:7874-7937
 * "frame/button/door") must remain — without those, the compat layer
 * has lost the visual half of the click event. */
static void verify_source_evidence_anchors(void)
{
    const char *evidence = dm1_viewport_3d_source_evidence();

    /* D1C is the only D1 view cell in F0124 (DUNVIEW.C:7727).  F0110
     * is invoked from DUNVIEW.C:7902 inside F0124's D1C case. */
    check_contains("press_evidence.f0124_d1c", evidence,
                   "DUNVIEW.C:7727");
    /* D1C door-front occlusion cites the frame/button/door order. */
    check_contains("press_evidence.d1c_door_front_occlusion", evidence,
                   "DUNVIEW.C:7874-7937");
    check_contains("press_evidence.d1c_frame_button_door", evidence,
                   "frame/button/door");
}

int main(void)
{
    DM1_DoorButtonBitmapSpan spans[DM1_VIEW_DOOR_BUTTON_COUNT];
    uint8_t dst[DST_SIZE];
    const uint8_t *d1c_pixels = NULL;
    size_t i;

    printf("probe=dm1_v1_door_button_d1c_press_frame_gate\n");

    /* Initialize every span slot to zero — the D1C blit is the only
     * one the probe exercises, but the helper indexes by
     * (door_button_ordinal - 1) * DM1_VIEW_DOOR_BUTTON_COUNT +
     * view_index, so a stray D2C/D3C/D3R slot at ordinal 0 would be
     * skipped anyway by the door_button_ordinal <= 0 check.  Keeping
     * the rest zeroed makes the failure mode obvious if that ever
     * changes. */
    memset(spans, 0, sizeof(spans));
    for (i = 0; i < DM1_VIEW_DOOR_BUTTON_COUNT; ++i) {
        spans[i].frame.left_x = (uint8_t)(D1C_DEST_LEFT + (int)i);
        spans[i].frame.right_x = (uint8_t)(D1C_DEST_LEFT + 5);
        spans[i].frame.top_y = (uint8_t)(D1C_DEST_TOP + (int)i);
        spans[i].frame.bottom_y = (uint8_t)(D1C_DEST_TOP + 3);
        spans[i].frame.byte_width = 6;
        spans[i].frame.height = 4;
    }

    build_d1c_span(&spans[DM1_VIEW_DOOR_BUTTON_D1C], &d1c_pixels);
    (void)d1c_pixels;

    verify_d1c_frame_lookup();
    verify_unpress_output_for_invalid_ordinals(dst, spans);
    verify_d1c_press_blit(dst, spans);
    verify_invalid_view_index_rejected();
    verify_source_evidence_anchors();

    if (g_failures) {
        printf("FAIL: %d D1C press/unpress frame gate check(s) failed\n",
               g_failures);
        return 1;
    }
    printf("ok: DM1 V1 D1C door-button press/unpress frame gate holds\n");
    return 0;
}
