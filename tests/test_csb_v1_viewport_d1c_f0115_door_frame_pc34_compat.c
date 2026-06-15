#include "csb/csb_v1_viewport_d1c_f0115_door_frame_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static const char *A_F0124 =
    "ReDMCSB DUNVIEW.C:F0124_DUNGEONVIEW_DrawSquareD1C:7873-7911";
static const char *A_F0115 =
    "ReDMCSB DUNVIEW.C:F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF:4547-4582";
static const char *A_F0104 =
    "ReDMCSB DUNVIEW.C:F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap:3113-3156";
static const char *A_F0105 =
    "ReDMCSB DUNVIEW.C:F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally:3185-3238";
static const char *A_DEFS =
    "ReDMCSB DEFS.H:C0x0218:2669; C0x0349:2672; C726/C727/C733:4084-4093";
static const char *A_F0128 =
    "ReDMCSB DUNVIEW.C:F0128_DUNGEONVIEW_Draw_CPSF:8524-8533";

static int g_assertions = 0;
static int g_failures = 0;

static int expect_int(const char *label, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want, anchor);
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_str(const char *label, const char *got, const char *want,
                      const char *anchor)
{
    return expect_int(label, got && want && strcmp(got, want) == 0, 1, anchor);
}

static int expect_contains(const char *label, const char *haystack,
                           const char *needle, const char *anchor)
{
    return expect_int(label, haystack && needle &&
                         strstr(haystack, needle) != NULL, 1, anchor);
}

static int test_contract_identity(void)
{
    int ok = 1;
    const CSB_V1_ViewportD1CF0115DoorFramePc34Contract *c =
        csb_v1_viewport_d1c_f0115_door_frame_pc34_contract();

    ok &= expect_int("contract.non_null", c != NULL, 1, A_F0124);
    ok &= expect_int("contract.only", c ? c->contract_only : 0, 1, A_F0124);
    ok &= expect_int("view_square.d1c", c ? c->view_square_d1c : -1, 3,
                     "ReDMCSB DEFS.H:M606_VIEW_SQUARE_D1C:2599");
    ok &= expect_int("view_depth.d1", c ? c->view_depth : -1, 1,
                     "ReDMCSB DUNVIEW.C:G2027_ac_ViewSquareIndexToViewDepth:372");
    ok &= expect_int("view_lane.center", c ? c->view_lane : -9, 0,
                     "ReDMCSB DUNVIEW.C:G2026_ac_ViewSquareIndexToViewLane:371");
    ok &= expect_int("element.door_front", c ? c->element_door_front : -1,
                     17, A_F0124);
    ok &= expect_int("transparent.c10", c ? c->transparent_color : -1, 10,
                     "ReDMCSB DEFS.H:C10_COLOR_FLESH:2088");
    ok &= expect_int("flip.mask.horizontal", c ? c->flip_horizontal_mask : -1,
                     1, A_F0105);
    ok &= expect_int("top.uses.f0104", c ? c->top_uses_f0104 : 0, 1,
                     A_F0124);
    ok &= expect_int("left.uses.f0104", c ? c->left_uses_f0104 : 0, 1,
                     A_F0124);
    ok &= expect_int("right.uses.f0105", c ? c->right_uses_f0105 : 0, 1,
                     A_F0124);
    ok &= expect_int("right.reuses.left.bitmap",
                     c ? c->right_reuses_left_bitmap : 0, 1, A_F0124);

    return ok;
}

static int test_f0115_order_pairing(void)
{
    int ok = 1;
    const CSB_V1_ViewportD1CF0115DoorFramePc34Contract *c =
        csb_v1_viewport_d1c_f0115_door_frame_pc34_contract();

    ok &= expect_int("rear.order.value", c ? c->f0115_rear_order : -1,
                     0x0218, A_DEFS);
    ok &= expect_int("front.order.value", c ? c->f0115_front_order : -1,
                     0x0349, A_DEFS);
    ok &= expect_int("rear.order.role",
                     csb_v1_viewport_d1c_f0115_door_frame_order_role_pc34(c, 0x0218),
                     1, A_F0124);
    ok &= expect_int("front.order.role",
                     csb_v1_viewport_d1c_f0115_door_frame_order_role_pc34(c, 0x0349),
                     2, A_F0124);
    ok &= expect_int("invalid.order.zero",
                     csb_v1_viewport_d1c_f0115_door_frame_order_role_pc34(c, 0),
                     0, A_F0115);
    ok &= expect_int("invalid.order.random",
                     csb_v1_viewport_d1c_f0115_door_frame_order_role_pc34(c, 0x1234),
                     0, A_F0115);
    ok &= expect_int("null.order.role",
                     csb_v1_viewport_d1c_f0115_door_frame_order_role_pc34(NULL, 0x0218),
                     0, A_F0115);
    ok &= expect_int("rear.precedes.frame",
                     c ? c->f0115_rear_precedes_frame : 0, 1, A_F0124);
    ok &= expect_int("frame.precedes.door",
                     c ? c->frame_precedes_door_bitmap : 0, 1, A_F0124);
    ok &= expect_int("door.precedes.front.f0115",
                     c ? c->door_bitmap_precedes_front_f0115 : 0, 1, A_F0124);
    ok &= expect_int("terminal.uses.l0217",
                     c ? c->terminal_f0115_uses_l0217_order : 0, 1, A_F0124);
    ok &= expect_int("not.f0122", c ? c->uses_f0122_d1l : 1, 0, A_F0128);
    ok &= expect_int("not.f0123", c ? c->uses_f0123_d1r : 1, 0, A_F0128);

    return ok;
}

static int test_frame_zones_and_bitmaps(void)
{
    int ok = 1;
    const CSB_V1_ViewportD1CF0115DoorFramePc34Contract *c =
        csb_v1_viewport_d1c_f0115_door_frame_pc34_contract();

    ok &= expect_int("zone.top.field", c ? c->frame_top_zone : -1, 733, A_DEFS);
    ok &= expect_int("zone.left.field", c ? c->frame_left_zone : -1, 726, A_DEFS);
    ok &= expect_int("zone.right.field", c ? c->frame_right_zone : -1, 727, A_DEFS);
    ok &= expect_int("zone.door.field", c ? c->door_zone_d1c : -1, 3790, A_DEFS);
    ok &= expect_int("zone.top.func",
                     csb_v1_viewport_d1c_f0115_door_frame_zone_for_part_pc34(
                         c, CSB_V1_D1C_DOOR_FRAME_PART_TOP),
                     733, A_F0124);
    ok &= expect_int("zone.left.func",
                     csb_v1_viewport_d1c_f0115_door_frame_zone_for_part_pc34(
                         c, CSB_V1_D1C_DOOR_FRAME_PART_LEFT),
                     726, A_F0124);
    ok &= expect_int("zone.right.func",
                     csb_v1_viewport_d1c_f0115_door_frame_zone_for_part_pc34(
                         c, CSB_V1_D1C_DOOR_FRAME_PART_RIGHT),
                     727, A_F0124);
    ok &= expect_int("zone.invalid.func",
                     csb_v1_viewport_d1c_f0115_door_frame_zone_for_part_pc34(c, 99),
                     -1, A_F0124);
    ok &= expect_int("zone.null.func",
                     csb_v1_viewport_d1c_f0115_door_frame_zone_for_part_pc34(NULL, 0),
                     -1, A_F0124);
    ok &= expect_str("bitmap.top",
                     csb_v1_viewport_d1c_f0115_door_frame_bitmap_for_part_pc34(
                         c, CSB_V1_D1C_DOOR_FRAME_PART_TOP),
                     "G2112_DoorFrameTopD1LCR", A_F0124);
    ok &= expect_str("bitmap.left",
                     csb_v1_viewport_d1c_f0115_door_frame_bitmap_for_part_pc34(
                         c, CSB_V1_D1C_DOOR_FRAME_PART_LEFT),
                     "G2117_DoorFrameLeftD1C", A_F0124);
    ok &= expect_str("bitmap.right",
                     csb_v1_viewport_d1c_f0115_door_frame_bitmap_for_part_pc34(
                         c, CSB_V1_D1C_DOOR_FRAME_PART_RIGHT),
                     "G2117_DoorFrameLeftD1C", A_F0124);
    ok &= expect_int("bitmap.invalid.null",
                     csb_v1_viewport_d1c_f0115_door_frame_bitmap_for_part_pc34(c, 99) == NULL,
                     1, A_F0124);
    ok &= expect_int("bitmap.null.contract",
                     csb_v1_viewport_d1c_f0115_door_frame_bitmap_for_part_pc34(NULL, 0) == NULL,
                     1, A_F0124);
    ok &= expect_int("top.flip",
                     csb_v1_viewport_d1c_f0115_door_frame_flip_for_part_pc34(
                         c, CSB_V1_D1C_DOOR_FRAME_PART_TOP),
                     0, A_F0104);
    ok &= expect_int("left.flip",
                     csb_v1_viewport_d1c_f0115_door_frame_flip_for_part_pc34(
                         c, CSB_V1_D1C_DOOR_FRAME_PART_LEFT),
                     0, A_F0104);
    ok &= expect_int("right.flip",
                     csb_v1_viewport_d1c_f0115_door_frame_flip_for_part_pc34(
                         c, CSB_V1_D1C_DOOR_FRAME_PART_RIGHT),
                     1, A_F0105);
    ok &= expect_int("invalid.flip",
                     csb_v1_viewport_d1c_f0115_door_frame_flip_for_part_pc34(c, 99),
                     -1, A_F0105);
    ok &= expect_int("null.flip",
                     csb_v1_viewport_d1c_f0115_door_frame_flip_for_part_pc34(NULL, 0),
                     -1, A_F0105);

    return ok;
}

static int test_frame_blit_contract(void)
{
    int ok = 1;
    const CSB_V1_ViewportD1CF0115DoorFramePc34Contract *c =
        csb_v1_viewport_d1c_f0115_door_frame_pc34_contract();
    const uint8_t source[6] = { 1, 10, 2, 3, 4, 10 };
    uint8_t top_dest[6] = { 9, 9, 9, 9, 9, 9 };
    uint8_t right_dest[6] = { 9, 9, 9, 9, 9, 9 };

    ok &= expect_int("top.blit.copied",
                     csb_v1_viewport_d1c_f0115_door_frame_apply_blit_pc34(
                         c, CSB_V1_D1C_DOOR_FRAME_PART_TOP,
                         source, 3, top_dest, 3, 3, 2),
                     4, A_F0104);
    ok &= expect_int("top.blit.0", top_dest[0], 1, A_F0104);
    ok &= expect_int("top.blit.transparent.1", top_dest[1], 9, A_F0104);
    ok &= expect_int("top.blit.2", top_dest[2], 2, A_F0104);
    ok &= expect_int("top.blit.3", top_dest[3], 3, A_F0104);
    ok &= expect_int("top.blit.4", top_dest[4], 4, A_F0104);
    ok &= expect_int("top.blit.transparent.5", top_dest[5], 9, A_F0104);
    ok &= expect_int("right.blit.copied",
                     csb_v1_viewport_d1c_f0115_door_frame_apply_blit_pc34(
                         c, CSB_V1_D1C_DOOR_FRAME_PART_RIGHT,
                         source, 3, right_dest, 3, 3, 2),
                     4, A_F0105);
    ok &= expect_int("right.blit.0", right_dest[0], 2, A_F0105);
    ok &= expect_int("right.blit.transparent.1", right_dest[1], 9, A_F0105);
    ok &= expect_int("right.blit.2", right_dest[2], 1, A_F0105);
    ok &= expect_int("right.blit.transparent.3", right_dest[3], 9, A_F0105);
    ok &= expect_int("right.blit.4", right_dest[4], 4, A_F0105);
    ok &= expect_int("right.blit.5", right_dest[5], 3, A_F0105);
    ok &= expect_int("blit.invalid.part",
                     csb_v1_viewport_d1c_f0115_door_frame_apply_blit_pc34(
                         c, 99, source, 3, right_dest, 3, 3, 2),
                     -1, A_F0105);
    ok &= expect_int("blit.null.contract",
                     csb_v1_viewport_d1c_f0115_door_frame_apply_blit_pc34(
                         NULL, CSB_V1_D1C_DOOR_FRAME_PART_TOP,
                         source, 3, top_dest, 3, 3, 2),
                     -1, A_F0104);
    ok &= expect_int("blit.null.source",
                     csb_v1_viewport_d1c_f0115_door_frame_apply_blit_pc34(
                         c, CSB_V1_D1C_DOOR_FRAME_PART_TOP,
                         NULL, 3, top_dest, 3, 3, 2),
                     -1, A_F0104);
    ok &= expect_int("blit.null.dest",
                     csb_v1_viewport_d1c_f0115_door_frame_apply_blit_pc34(
                         c, CSB_V1_D1C_DOOR_FRAME_PART_TOP,
                         source, 3, NULL, 3, 3, 2),
                     -1, A_F0104);
    ok &= expect_int("blit.bad.width",
                     csb_v1_viewport_d1c_f0115_door_frame_apply_blit_pc34(
                         c, CSB_V1_D1C_DOOR_FRAME_PART_TOP,
                         source, 3, top_dest, 3, 0, 2),
                     -1, A_F0104);
    ok &= expect_int("blit.bad.height",
                     csb_v1_viewport_d1c_f0115_door_frame_apply_blit_pc34(
                         c, CSB_V1_D1C_DOOR_FRAME_PART_TOP,
                         source, 3, top_dest, 3, 3, 0),
                     -1, A_F0104);
    ok &= expect_int("blit.short.source_stride",
                     csb_v1_viewport_d1c_f0115_door_frame_apply_blit_pc34(
                         c, CSB_V1_D1C_DOOR_FRAME_PART_TOP,
                         source, 2, top_dest, 3, 3, 2),
                     -1, A_F0104);
    ok &= expect_int("blit.short.dest_stride",
                     csb_v1_viewport_d1c_f0115_door_frame_apply_blit_pc34(
                         c, CSB_V1_D1C_DOOR_FRAME_PART_TOP,
                         source, 3, top_dest, 2, 3, 2),
                     -1, A_F0104);

    return ok;
}

static int test_evidence_strings(void)
{
    int ok = 1;
    const CSB_V1_ViewportD1CF0115DoorFramePc34Contract *c =
        csb_v1_viewport_d1c_f0115_door_frame_pc34_contract();
    const char *e =
        csb_v1_viewport_d1c_f0115_door_frame_pc34_source_evidence();

    ok &= expect_contains("anchor.f0124", c ? c->redmcsb_f0124_anchor : NULL,
                          "7873-7911", A_F0124);
    ok &= expect_contains("anchor.f0115", c ? c->redmcsb_f0115_anchor : NULL,
                          "4547-4582", A_F0115);
    ok &= expect_contains("anchor.f0104", c ? c->redmcsb_f0104_anchor : NULL,
                          "3113-3156", A_F0104);
    ok &= expect_contains("anchor.f0105", c ? c->redmcsb_f0105_anchor : NULL,
                          "3185-3238", A_F0105);
    ok &= expect_contains("anchor.defs.order", c ? c->redmcsb_defs_anchor : NULL,
                          "C0x0218:2669", A_DEFS);
    ok &= expect_contains("anchor.defs.zones", c ? c->redmcsb_defs_anchor : NULL,
                          "C726/C727/C733:4084-4093", A_DEFS);
    ok &= expect_contains("anchor.f0128", c ? c->redmcsb_f0128_anchor : NULL,
                          "8524-8533", A_F0128);
    ok &= expect_contains("evidence.contract", e, "contract_only=1", A_F0124);
    ok &= expect_contains("evidence.no.real.asset", e,
                          "no real-asset parity is claimed", A_F0124);
    ok &= expect_contains("evidence.rear", e, "C0x0218 at 7875", A_F0124);
    ok &= expect_contains("evidence.top.left", e,
                          "top/left draws through F0104 at 7886-7887",
                          A_F0124);
    ok &= expect_contains("evidence.right", e,
                          "F0105 horizontal flip of G2117_DoorFrameLeftD1C at 7893",
                          A_F0124);
    ok &= expect_contains("evidence.door", e, "F0111 D1C door bitmap at 7908",
                          A_F0124);
    ok &= expect_contains("evidence.front", e, "C0x0349 at 7910/7937",
                          A_F0124);
    ok &= expect_contains("evidence.c10", e, "C10 transparent frame blits",
                          A_F0104);
    ok &= expect_contains("evidence.mask", e, "MASK0x0001 horizontal flip",
                          A_F0105);
    ok &= expect_contains("evidence.zone", e, "C726/C727/C733 D1C frame zones",
                          A_DEFS);
    ok &= expect_contains("evidence.not.f0122", e, "does not cover the F0122",
                          A_F0128);
    ok &= expect_contains("evidence.not.f0123", e, "F0123 side-door routes",
                          A_F0128);

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d1c_f0115_door_frame_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d1c_f0115_door_frame_pc34_source_evidence());

    ok &= test_contract_identity();
    ok &= test_f0115_order_pairing();
    ok &= test_frame_zones_and_bitmaps();
    ok &= test_frame_blit_contract();
    ok &= test_evidence_strings();
    ok &= expect_int("assertion_count_at_least_50", g_assertions >= 50, 1,
                     A_F0124);

    printf("assertions=%d failures=%d\n", g_assertions, g_failures);
    if (ok && g_failures == 0) {
        printf("PASS csb_v1_viewport_d1c_f0115_door_frame_pc34_compat assertions=%d failures=0\n",
               g_assertions);
    }

    return (ok && g_failures == 0) ? 0 : 1;
}
