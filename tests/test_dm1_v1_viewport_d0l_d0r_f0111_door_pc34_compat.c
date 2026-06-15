/*
 * Source-lock header (DM1 V1 D0L/D0R F0111 door-front slice):
 * - ReDMCSB DUNVIEW.C F0111 lines 4218-4337: door-front composition,
 *   door-state decrement branch at 4308, LeftHorizontal/RightHorizontal
 *   select at 4312-4313, C6 horizontal half blit at 4322-4324,
 *   3|MASK0x4000 at 4325, and C10 transparent F0791 blit at 4334.
 * - ReDMCSB DUNVIEW.C F0128 lines 8318-8486 and 8536-8541: D0L/D0R
 *   dispatch after relative movement.
 * - ReDMCSB DUNVIEW.C F0125 lines 7960-8062 and F0126 lines 8064-8162:
 *   D0L/D0R side-square dispatchers.
 * - ReDMCSB DUNVIEW.C F0104 lines 3113-3156, F0105 lines 3185-3247,
 *   and F0107 lines 3502-3938: wall composition callers.
 * - ReDMCSB DUNGEON.C F0163 lines 1769-1838, F0164 lines 1840-1905,
 *   and F0172 lines 2466-2523: thing-list mutation and square-aspect
 *   anchors.
 * - ReDMCSB DEFS.H line 2088 C10_COLOR_FLESH; lines 2596-2611 view
 *   squares; line 2662 and lines 2668-2677 cell orders; line 2662
 *   C0x0021; lines 4045-4046 C705/C706; lines 4139-4153 zone band.
 */
#include "dm1/dm1_v1_viewport_d0l_d0r_f0111_door_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;
static uint32_t g_hash = 2166136261u;

static const char *A_F0111 =
    "ReDMCSB DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor";
static const char *A_F0128 =
    "ReDMCSB DUNVIEW.C:8318-8486/8536-8541 F0128_DUNGEONVIEW_Draw_CPSF";
static const char *A_F0125 =
    "ReDMCSB DUNVIEW.C:7960-8062 F0125_DUNGEONVIEW_DrawSquareD0L";
static const char *A_F0126 =
    "ReDMCSB DUNVIEW.C:8064-8162 F0126_DUNGEONVIEW_DrawSquareD0R";
static const char *A_F0104_F0105_F0107 =
    "ReDMCSB DUNVIEW.C:3113-3156 F0104; 3185-3247 F0105; 3502-3938 F0107";
static const char *A_DUNGEON =
    "ReDMCSB DUNGEON.C:1769-1838 F0163; 1840-1905 F0164; 2466-2523 F0172";
static const char *A_DEFS =
    "ReDMCSB DEFS.H:2088,2596-2611,2662,2668-2677,4045-4046,4139-4153";

static void hash_u32(uint32_t value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        g_hash ^= (value >> (i * 8)) & 0xffu;
        g_hash *= 16777619u;
    }
}

static void hash_string(const char *text)
{
    if (!text) {
        hash_u32(0xffffffffu);
        return;
    }
    while (*text) {
        g_hash ^= (unsigned char)*text++;
        g_hash *= 16777619u;
    }
}

static void check_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    hash_string(id);
    hash_u32((uint32_t)got);
    hash_u32((uint32_t)want);
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n", id, got, want, anchor);
    } else {
        printf("PASS %s=%d anchor=%s\n", id, got, anchor);
    }
}

static void check_bool(const char *id, bool got, bool want, const char *anchor)
{
    check_int(id, got ? 1 : 0, want ? 1 : 0, anchor);
}

static void check_contains(const char *id, const char *haystack,
                           const char *needle, const char *anchor)
{
    const int found = haystack && needle && strstr(haystack, needle) != NULL;

    hash_string(needle);
    check_int(id, found, 1, anchor);
}

static void check_frame(const char *prefix,
                        const DM1_V1_D0LD0RF0111DoorFramePc34 *frame,
                        int x1, int x2, int y1, int y2, int byte_width,
                        int height, int source_x, int source_y,
                        const char *anchor)
{
    char id[96];

    (void)snprintf(id, sizeof(id), "%s.x1", prefix);
    check_int(id, frame->x1, x1, anchor);
    (void)snprintf(id, sizeof(id), "%s.x2", prefix);
    check_int(id, frame->x2, x2, anchor);
    (void)snprintf(id, sizeof(id), "%s.y1", prefix);
    check_int(id, frame->y1, y1, anchor);
    (void)snprintf(id, sizeof(id), "%s.y2", prefix);
    check_int(id, frame->y2, y2, anchor);
    (void)snprintf(id, sizeof(id), "%s.byte_width", prefix);
    check_int(id, frame->byte_width, byte_width, anchor);
    (void)snprintf(id, sizeof(id), "%s.height", prefix);
    check_int(id, frame->height, height, anchor);
    (void)snprintf(id, sizeof(id), "%s.source_x", prefix);
    check_int(id, frame->source_x, source_x, anchor);
    (void)snprintf(id, sizeof(id), "%s.source_y", prefix);
    check_int(id, frame->source_y, source_y, anchor);
}

static void test_identity_and_evidence(void)
{
    const char *e = dm1_v1_viewport_d0l_d0r_f0111_door_source_evidence_pc34();
    const DM1_V1_D0LD0RF0111DoorSpecPc34 *d0l =
        dm1_v1_viewport_d0l_d0r_f0111_door_for_side_pc34(1);
    const DM1_V1_D0LD0RF0111DoorSpecPc34 *d0r =
        dm1_v1_viewport_d0l_d0r_f0111_door_for_side_pc34(2);

    check_int("count", (int)dm1_v1_viewport_d0l_d0r_f0111_door_count_pc34(),
              2, A_F0111);
    check_bool("at0.is.d0l",
               dm1_v1_viewport_d0l_d0r_f0111_door_at_pc34(0) == d0l,
               true, A_F0125);
    check_bool("at1.is.d0r",
               dm1_v1_viewport_d0l_d0r_f0111_door_at_pc34(1) == d0r,
               true, A_F0126);
    check_bool("at2.null",
               dm1_v1_viewport_d0l_d0r_f0111_door_at_pc34(2) == NULL,
               true, A_DEFS);
    check_bool("bad.side.null",
               dm1_v1_viewport_d0l_d0r_f0111_door_for_side_pc34(9) == NULL,
               true, A_DEFS);
    check_contains("evidence.f0111", e, "F0111:4218-4337", A_F0111);
    check_contains("evidence.decrement", e, "4308", A_F0111);
    check_contains("evidence.left.right", e, "4312-4313", A_F0111);
    check_contains("evidence.mask", e, "MASK0x4000", A_F0111);
    check_contains("evidence.c10", e, "C10_COLOR_FLESH", A_DEFS);
    check_contains("evidence.f0128", e, "F0128:8318-8486", A_F0128);
    check_contains("evidence.f0128.d0", e, "8536-8541", A_F0128);
    check_contains("evidence.f0125", e, "F0125:7960-8062", A_F0125);
    check_contains("evidence.f0126", e, "F0126:8064-8162", A_F0126);
    check_contains("evidence.f0104", e, "F0104:3113-3156",
                   A_F0104_F0105_F0107);
    check_contains("evidence.f0105", e, "F0105:3185-3247",
                   A_F0104_F0105_F0107);
    check_contains("evidence.f0107", e, "F0107:3502-3938",
                   A_F0104_F0105_F0107);
    check_contains("evidence.f0163", e, "F0163:1769-1838", A_DUNGEON);
    check_contains("evidence.f0164", e, "F0164:1840-1905", A_DUNGEON);
    check_contains("evidence.f0172", e, "F0172:2466-2523", A_DUNGEON);
    check_contains("evidence.defs", e, "DEFS.H:2088", A_DEFS);
    check_contains("evidence.c705", e, "4045-4046", A_DEFS);
    check_contains("evidence.zone.band", e, "4139-4153", A_DEFS);
}

static void check_spec_common(const DM1_V1_D0LD0RF0111DoorSpecPc34 *s,
                              int side, int requested_square,
                              int source_square, int dispatch_update,
                              int dispatch_draw, int line_start,
                              int line_end, int lateral, int wall_zone,
                              int door_zone, int frame_left,
                              int frame_right, int floor_view,
                              unsigned int pass1, unsigned int pass2,
                              const char *side_anchor)
{
    check_bool("spec.present", s != NULL, true, side_anchor);
    if (!s) return;
    check_int("spec.side", s->side, side, side_anchor);
    check_int("spec.requested_square", s->requested_view_square,
              requested_square, A_DEFS);
    check_int("spec.source_square", s->source_view_square, source_square,
              A_DEFS);
    check_int("spec.dispatch.update", s->f0128_update_line, dispatch_update,
              A_F0128);
    check_int("spec.dispatch.draw", s->f0128_draw_line, dispatch_draw,
              A_F0128);
    check_int("spec.side.line.start", s->f0125_or_f0126_line_start,
              line_start, side_anchor);
    check_int("spec.side.line.end", s->f0125_or_f0126_line_end, line_end,
              side_anchor);
    check_int("spec.requested.depth", s->requested_depth, 0, A_F0128);
    check_int("spec.requested.lateral", s->requested_lateral, lateral,
              A_F0128);
    check_int("spec.source.depth", s->source_depth, 3,
              "ReDMCSB DUNVIEW.C:6442-6460/6578-6602 D3 door-front source");
    check_int("spec.source.lateral", s->source_lateral, lateral,
              "ReDMCSB DUNVIEW.C:6442-6460/6578-6602 D3 left/right source");
    check_int("spec.wall.zone", s->wall_zone, wall_zone, A_DEFS);
    check_int("spec.door.zone", s->door_zone, door_zone, A_F0111);
    check_int("spec.frame.left.zone", s->door_frame_left_zone, frame_left,
              side_anchor);
    check_int("spec.frame.right.zone", s->door_frame_right_zone, frame_right,
              side_anchor);
    check_int("spec.floor.view", s->floor_view, floor_view, side_anchor);
    check_int("spec.door.bitmap", s->door_front_bitmap, 693, A_F0111);
    check_int("spec.door.ornament", s->door_ornament_view, 0, A_F0111);
    check_int("spec.pass1", (int)s->pass1_cell_order, (int)pass1, A_DEFS);
    check_int("spec.pass2", (int)s->pass2_cell_order, (int)pass2, A_DEFS);
    check_bool("spec.contract", s->source_locked_contract_only, true, A_F0111);
    check_bool("spec.no.assets", s->no_real_asset_bitmap_parity, true,
               A_F0111);
    check_bool("spec.no.data", s->no_game_data_load, true, A_F0111);
    check_bool("spec.f0128.dispatch", s->f0128_dispatches_requested_side,
               true, A_F0128);
    check_bool("spec.d0.dispatch.boundary",
               s->d0_side_dispatch_is_not_the_f0111_compositor, true,
               side_anchor);
    check_bool("spec.f0116.f0117.owns",
               s->f0116_or_f0117_owns_door_front, true, A_F0111);
    check_bool("spec.f0104", s->f0104_wall_caller_present, true,
               A_F0104_F0105_F0107);
    check_bool("spec.f0105", s->f0105_mirror_wall_caller_present, true,
               A_F0104_F0105_F0107);
    check_bool("spec.f0107", s->f0107_wall_ornament_caller_present, true,
               A_F0104_F0105_F0107);
    check_bool("spec.f0163.not.called", s->f0163_not_called_by_draw, true,
               A_DUNGEON);
    check_bool("spec.f0164.not.called", s->f0164_not_called_by_draw, true,
               A_DUNGEON);
    check_bool("spec.f0172.source", s->f0172_square_aspect_source, true,
               A_DUNGEON);
    check_contains("spec.anchor.f0111", s->redmcsb_f0111_anchor,
                   "DUNVIEW.C:4218-4337", A_F0111);
    check_contains("spec.anchor.f0128", s->redmcsb_f0128_anchor,
                   "8536-8541", A_F0128);
    check_contains("spec.anchor.side", s->redmcsb_f0125_f0126_anchor,
                   side == 1 ? "F0125" : "F0126", side_anchor);
    check_contains("spec.anchor.wall.callers",
                   s->redmcsb_f0104_f0105_f0107_anchor, "F0107",
                   A_F0104_F0105_F0107);
    check_contains("spec.anchor.dungeon", s->redmcsb_dungeon_anchor,
                   "F0172", A_DUNGEON);
    check_contains("spec.anchor.defs", s->redmcsb_defs_anchor,
                   "4045-4046", A_DEFS);
    check_contains("spec.anchor.caller", s->redmcsb_door_front_caller_anchor,
                   side == 1 ? "6442-6460" : "6578-6602", A_F0111);
}

static void test_specs_and_frames(void)
{
    const DM1_V1_D0LD0RF0111DoorSpecPc34 *d0l =
        dm1_v1_viewport_d0l_d0r_f0111_door_for_side_pc34(1);
    const DM1_V1_D0LD0RF0111DoorSpecPc34 *d0r =
        dm1_v1_viewport_d0l_d0r_f0111_door_for_side_pc34(2);

    check_spec_common(d0l, 1, 1, 12, 8536, 8537, 7960, 8062, -1,
                      705, 3720, 718, 719, 2, 0x0218u, 0x0349u, A_F0125);
    check_frame("d0l.closed", &d0l->closed_or_destroyed, 24, 71, 28, 67,
                24, 41, 0, 0, A_F0111);
    check_frame("d0l.vertical.half", &d0l->vertical[1], 24, 71, 28, 48,
                24, 41, 0, 20, A_F0111);
    check_frame("d0l.left.three_fourth", &d0l->left_horizontal[2], 24, 41,
                28, 67, 24, 41, 6, 0, A_F0111);
    check_frame("d0l.right.three_fourth", &d0l->right_horizontal[2], 54, 71,
                28, 67, 24, 41, 24, 0, A_F0111);

    check_spec_common(d0r, 2, 2, 13, 8540, 8541, 8064, 8162, 1,
                      706, 3740, 720, 721, 4, 0x0128u, 0x0439u, A_F0126);
    check_frame("d0r.closed", &d0r->closed_or_destroyed, 150, 197, 28, 67,
                24, 41, 0, 0, A_F0111);
    check_frame("d0r.vertical.half", &d0r->vertical[1], 150, 197, 28, 48,
                24, 41, 0, 20, A_F0111);
    check_frame("d0r.left.three_fourth", &d0r->left_horizontal[2], 150, 167,
                28, 67, 24, 41, 6, 0, A_F0111);
    check_frame("d0r.right.three_fourth", &d0r->right_horizontal[2], 180, 197,
                28, 67, 24, 41, 24, 0, A_F0111);
}

static void check_composition_one(const DM1_V1_D0LD0RF0111DoorSpecPc34 *s,
                                  uint8_t base)
{
    DM1_V1_D0LD0RF0111DoorPixelTracePc34 p;

    check_bool("compose.closed.ok",
               dm1_v1_viewport_d0l_d0r_f0111_door_compose_closed_pixel_pc34(
                   s, base, 0x21u, 0x31u, 0x41u, 0x51u, &p),
               true, A_F0111);
    check_bool("compose.closed.in_clip", p.in_closed_clip, true, A_F0111);
    check_int("compose.closed.after_floor", p.after_floor, 0x21, A_F0111);
    check_int("compose.closed.after_pass1", p.after_pass1, 0x31, A_F0111);
    check_int("compose.closed.after_door", p.after_door, 0x41, A_F0111);
    check_int("compose.closed.after_pass2", p.after_pass2, 0x51, A_F0111);

    check_bool("compose.transparent.ok",
               dm1_v1_viewport_d0l_d0r_f0111_door_compose_closed_pixel_pc34(
                   s, base, 10u, 10u, 10u, 10u, &p),
               true, A_F0111);
    check_bool("compose.transparent.floor", p.floor_transparent, true,
               A_DEFS);
    check_bool("compose.transparent.pass1", p.pass1_transparent, true,
               A_DEFS);
    check_bool("compose.transparent.door", p.door_transparent, true,
               A_DEFS);
    check_bool("compose.transparent.pass2", p.pass2_transparent, true,
               A_DEFS);
    check_int("compose.transparent.final", p.after_pass2, base, A_DEFS);

    check_bool("compose.door.survives.ok",
               dm1_v1_viewport_d0l_d0r_f0111_door_compose_closed_pixel_pc34(
                   s, base, 10u, 10u, 0x44u, 10u, &p),
               true, A_F0111);
    check_int("compose.door.survives.final", p.after_pass2, 0x44, A_F0111);
}

static void test_pixel_composition_and_cell_orders(void)
{
    const DM1_V1_D0LD0RF0111DoorSpecPc34 *d0l =
        dm1_v1_viewport_d0l_d0r_f0111_door_for_side_pc34(1);
    const DM1_V1_D0LD0RF0111DoorSpecPc34 *d0r =
        dm1_v1_viewport_d0l_d0r_f0111_door_for_side_pc34(2);
    DM1_V1_D0LD0RF0111DoorPixelTracePc34 p;

    check_int("blend.transparent",
              dm1_v1_viewport_d0l_d0r_f0111_door_blend_pc34(0xaa, 10),
              0xaa, A_DEFS);
    check_int("blend.opaque",
              dm1_v1_viewport_d0l_d0r_f0111_door_blend_pc34(0xaa, 0xbb),
              0xbb, A_F0111);
    check_bool("compose.null.spec",
               dm1_v1_viewport_d0l_d0r_f0111_door_compose_closed_pixel_pc34(
                   NULL, 0, 0, 0, 0, 0, &p),
               false, A_F0111);
    check_bool("compose.null.out",
               dm1_v1_viewport_d0l_d0r_f0111_door_compose_closed_pixel_pc34(
                   d0l, 0, 0, 0, 0, 0, NULL),
               false, A_F0111);
    check_composition_one(d0l, 0x66u);
    check_composition_one(d0r, 0x77u);

    check_int("decode.d0l.pass1.0",
              dm1_v1_viewport_d0l_d0r_f0111_door_decode_cell_pc34(0x0218u, 0),
              0, A_DEFS);
    check_int("decode.d0l.pass1.1",
              dm1_v1_viewport_d0l_d0r_f0111_door_decode_cell_pc34(0x0218u, 1),
              1, A_DEFS);
    check_int("decode.d0r.pass1.0",
              dm1_v1_viewport_d0l_d0r_f0111_door_decode_cell_pc34(0x0128u, 0),
              1, A_DEFS);
    check_int("decode.d0r.pass1.1",
              dm1_v1_viewport_d0l_d0r_f0111_door_decode_cell_pc34(0x0128u, 1),
              0, A_DEFS);
    check_int("decode.d0l.pass2.0",
              dm1_v1_viewport_d0l_d0r_f0111_door_decode_cell_pc34(0x0349u, 0),
              3, A_DEFS);
    check_int("decode.d0l.pass2.1",
              dm1_v1_viewport_d0l_d0r_f0111_door_decode_cell_pc34(0x0349u, 1),
              2, A_DEFS);
    check_int("decode.d0r.pass2.0",
              dm1_v1_viewport_d0l_d0r_f0111_door_decode_cell_pc34(0x0439u, 0),
              2, A_DEFS);
    check_int("decode.d0r.pass2.1",
              dm1_v1_viewport_d0l_d0r_f0111_door_decode_cell_pc34(0x0439u, 1),
              3, A_DEFS);
    check_int("decode.zero",
              dm1_v1_viewport_d0l_d0r_f0111_door_decode_cell_pc34(0u, 0),
              -1, A_DEFS);
    check_int("decode.bad.ordinal",
              dm1_v1_viewport_d0l_d0r_f0111_door_decode_cell_pc34(0x0218u, 4),
              -1, A_DEFS);
}

static void check_state_one(const DM1_V1_D0LD0RF0111DoorSpecPc34 *s)
{
    DM1_V1_D0LD0RF0111DoorStateTracePc34 t;

    check_bool("state.open.ok",
               dm1_v1_viewport_d0l_d0r_f0111_door_state_trace_pc34(
                   s, DM1_V1_D0L_D0R_F0111_DOOR_STATE_OPEN_PC34, false, &t),
               true, A_F0111);
    check_bool("state.open.skipped", t.skipped_open_guard, true, A_F0111);
    check_int("state.open.decremented", t.decremented_state, -1, A_F0111);
    check_int("state.open.final.zone", t.final_zone, s->door_zone, A_F0111);
    check_bool("state.open.no.mask", t.mask0x4000_applied, false, A_F0111);

    check_bool("state.closed.ok",
               dm1_v1_viewport_d0l_d0r_f0111_door_state_trace_pc34(
                   s, DM1_V1_D0L_D0R_F0111_DOOR_STATE_CLOSED_PC34, true, &t),
               true, A_F0111);
    check_bool("state.closed.frame", t.closed_or_destroyed_frame_selected,
               true, A_F0111);
    check_int("state.closed.x1", t.selected_closed_or_vertical.x1,
              s->closed_or_destroyed.x1, A_F0111);
    check_int("state.closed.x2", t.selected_closed_or_vertical.x2,
              s->closed_or_destroyed.x2, A_F0111);
    check_int("state.closed.final.zone", t.final_zone, s->door_zone, A_F0111);

    check_bool("state.vertical.open.ok",
               dm1_v1_viewport_d0l_d0r_f0111_door_state_trace_pc34(
                   s, DM1_V1_D0L_D0R_F0111_DOOR_STATE_CLOSED_HALF_PC34,
                   true, &t),
               true, A_F0111);
    check_int("state.vertical.decremented", t.decremented_state, 1, A_F0111);
    check_bool("state.vertical.selected", t.vertical_frame_selected, true,
               A_F0111);
    check_int("state.vertical.y2", t.selected_closed_or_vertical.y2,
              s->vertical[1].y2, A_F0111);
    check_int("state.vertical.final.zone", t.final_zone, s->door_zone + 1,
              A_F0111);
    check_bool("state.vertical.no.mask", t.mask0x4000_applied, false,
               A_F0111);

    check_bool("state.horizontal.open.ok",
               dm1_v1_viewport_d0l_d0r_f0111_door_state_trace_pc34(
                   s,
                   DM1_V1_D0L_D0R_F0111_DOOR_STATE_CLOSED_THREE_FOURTH_PC34,
                   false, &t),
               true, A_F0111);
    check_int("state.horizontal.decremented", t.decremented_state, 2,
              A_F0111);
    check_bool("state.left.selected", t.left_horizontal_selected, true,
               A_F0111);
    check_bool("state.right.selected", t.right_horizontal_selected, true,
               A_F0111);
    check_int("state.left.x2", t.selected_left_horizontal.x2,
              s->left_horizontal[2].x2, A_F0111);
    check_int("state.right.x1", t.selected_right_horizontal.x1,
              s->right_horizontal[2].x1, A_F0111);
    check_bool("state.c6.blit", t.horizontal_c6_transparent_blit, true,
               A_F0111);
    check_int("state.c6.zone", t.c6_zone, s->door_zone + 2 + 6, A_F0111);
    check_int("state.shift.x", t.zone_shift_x, 12, A_F0111);
    check_int("state.shift.y", t.zone_shift_y, 0, A_F0111);
    check_bool("state.mask.applied", t.mask0x4000_applied, true, A_F0111);
    check_int("state.final.zone.masked", t.final_zone,
              s->door_zone + 2 + 3 + 0x4000, A_F0111);

    check_bool("state.invalid",
               dm1_v1_viewport_d0l_d0r_f0111_door_state_trace_pc34(
                   s, 9, false, &t),
               false, A_F0111);
}

static void test_state_machine_and_dispatch(void)
{
    const DM1_V1_D0LD0RF0111DoorSpecPc34 *d0l =
        dm1_v1_viewport_d0l_d0r_f0111_door_for_side_pc34(1);
    const DM1_V1_D0LD0RF0111DoorSpecPc34 *d0r =
        dm1_v1_viewport_d0l_d0r_f0111_door_for_side_pc34(2);
    DM1_V1_D0LD0RF0111DispatchTracePc34 d;
    DM1_V1_D0LD0RF0111DoorStateTracePc34 t;

    check_bool("state.null.spec",
               dm1_v1_viewport_d0l_d0r_f0111_door_state_trace_pc34(
                   NULL, 1, false, &t),
               false, A_F0111);
    check_bool("state.null.out",
               dm1_v1_viewport_d0l_d0r_f0111_door_state_trace_pc34(
                   d0l, 1, false, NULL),
               false, A_F0111);
    check_state_one(d0l);
    check_state_one(d0r);

    check_bool("dispatch.d0l.north",
               dm1_v1_viewport_d0l_d0r_f0111_dispatch_pc34(d0l, 0, 10, 20,
                                                            &d),
               true, A_F0128);
    check_int("dispatch.d0l.x", d.updated_x, 9, A_F0128);
    check_int("dispatch.d0l.y", d.updated_y, 20, A_F0128);
    check_int("dispatch.d0l.update.line", d.f0128_update_line, 8536,
              A_F0128);
    check_int("dispatch.d0l.draw.line", d.f0128_draw_line, 8537, A_F0128);
    check_contains("dispatch.d0l.function", d.draw_function, "F0125",
                   A_F0125);

    check_bool("dispatch.d0r.east",
               dm1_v1_viewport_d0l_d0r_f0111_dispatch_pc34(d0r, 1, 10, 20,
                                                            &d),
               true, A_F0128);
    check_int("dispatch.d0r.x", d.updated_x, 10, A_F0128);
    check_int("dispatch.d0r.y", d.updated_y, 21, A_F0128);
    check_int("dispatch.d0r.update.line", d.f0128_update_line, 8540,
              A_F0128);
    check_int("dispatch.d0r.draw.line", d.f0128_draw_line, 8541, A_F0128);
    check_contains("dispatch.d0r.function", d.draw_function, "F0126",
                   A_F0126);

    check_bool("dispatch.bad.direction",
               dm1_v1_viewport_d0l_d0r_f0111_dispatch_pc34(d0r, 4, 10, 20,
                                                            &d),
               false, A_F0128);
    check_bool("dispatch.null.spec",
               dm1_v1_viewport_d0l_d0r_f0111_dispatch_pc34(NULL, 0, 10, 20,
                                                            &d),
               false, A_F0128);
    check_bool("dispatch.null.out",
               dm1_v1_viewport_d0l_d0r_f0111_dispatch_pc34(d0r, 0, 10, 20,
                                                            NULL),
               false, A_F0128);
}

int main(void)
{
    printf("probe=dm1_v1_viewport_d0l_d0r_f0111_door_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_viewport_d0l_d0r_f0111_door_source_evidence_pc34());

    test_identity_and_evidence();
    test_specs_and_frames();
    test_pixel_composition_and_cell_orders();
    test_state_machine_and_dispatch();

    check_bool("assertion.count.at.least.90", g_assertions >= 90, true,
               "pass737 deterministic source-lock floor");
    printf("RESULT assertions=%d failures=%d hash=0x%08x\n",
           g_assertions, g_failures, g_hash);
    if (g_failures == 0) {
        printf("PASS test_dm1_v1_viewport_d0l_d0r_f0111_door_pc34_compat "
               "assertions=%d failures=0 hash=0x%08x\n",
               g_assertions, g_hash);
        return 0;
    }
    printf("FAIL test_dm1_v1_viewport_d0l_d0r_f0111_door_pc34_compat "
           "assertions=%d failures=%d hash=0x%08x\n",
           g_assertions, g_failures, g_hash);
    return 1;
}
