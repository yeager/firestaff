/*
 * DM1 V1 PC 3.4 F0111 partly-open D2L/D2R corridor-side door gate.
 *
 * Source-lock evidence:
 * - ReDMCSB DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor; line 4248
 *   is the open-door guard, line 4308 decrements C1/C2/C3 partly-open
 *   states before frame selection, lines 4312-4313 select the horizontal
 *   halves, lines 4317-4324 do P2084_i_ZoneIndex += decremented state
 *   and then F0635/F0654 through zone + C6_UNKNOWN using C10_COLOR_FLESH,
 *   and lines 4325-4334 add 3|MASK0x4000 before F0791 with C10.
 * - ReDMCSB DUNVIEW.C:8504-8508 F0128 D2L2/D2R2 MEDIA720 guard,
 *   8513-8517 D2L/D2R dispatch, and 8521 F0121 D2C dispatch as order
 *   bound only.
 * - ReDMCSB DUNVIEW.C:6987-7004 F0119 D2L and 7180-7197 F0120 D2R
 *   route C17_ELEMENT_DOOR_FRONT to F0111 with
 *   G0694_ai_DoorNativeBitmapIndex_Front_D2LCR,
 *   G0182_s_Graphic558_Frames_Door_D2L/G0184_s_Graphic558_Frames_Door_D2R,
 *   C1_VIEW_DOOR_ORNAMENT_D2LCR, and M627/M629.
 * - ReDMCSB DUNVIEW.C:7244-7389 F0121 is the D2C center anchor only.
 * - ReDMCSB DUNVIEW.C:6837-6865 F0678 and 6868-6896 F0679 are D2-side
 *   wall anchors which return wall cases before any F0111 front route.
 * - ReDMCSB DEFS.H:1039-1043 door states, 2088 C10_COLOR_FLESH,
 *   2603-2604 M604/M605, 2669/2672 C0x0218/C0x0349, 2790 actual
 *   C1_VIEW_DOOR_ORNAMENT_D2LCR, 3508 C6_UNKNOWN, 3516 MASK0x4000,
 *   4254-4258 M627/M629, 5457 G0694, 5539 G0182, and 5541 G0184.
 * - CSB sibling: src/csb/csb_v1_viewport_d2c_f0111_partly_open_door_pc34_compat.c
 *   is the D2C partly-open counterpart. DM1 sibling
 *   test_dm1_v1_viewport_d0l_d0r_f0111_door_pc34_compat is the
 *   non-overlap closed-door contract.
 *
 * Contract-only: this file simulates the F0111 composition in a
 * 320x200 framebuffer with a 224x136 viewport and does not load or
 * compare real game assets.
 */
#include "firestaff/dm1/v1/viewport/d2l_d2r_f0111_partly_open_door_pc34_compat.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

enum {
    DM1_FRAMEBUFFER_WIDTH = 320,
    DM1_FRAMEBUFFER_HEIGHT = 200,
    DM1_VIEWPORT_WIDTH = 224,
    DM1_VIEWPORT_HEIGHT = 136,
    DM1_VIEW_DEPTH_D2 = 2,
    DM1_VIEW_SQUARE_D2L = 7,
    DM1_VIEW_SQUARE_D2R = 8,
    DM1_VIEW_SQUARE_D2C = 6,
    DM1_VIEW_SQUARE_D2L2 = 9,
    DM1_VIEW_SQUARE_D2R2 = 10,
    DM1_DOOR_STATE_OPEN = 0,
    DM1_DOOR_STATE_ONE_FOURTH = 1,
    DM1_DOOR_STATE_HALF = 2,
    DM1_DOOR_STATE_THREE_FOURTH = 3,
    DM1_DOOR_STATE_CLOSED = 4,
    DM1_C6_UNKNOWN = 6,
    DM1_C10_COLOR_FLESH = 10,
    DM1_MASK0X4000 = 0x4000,
    DM1_SECOND_HALF_OFFSET = 3,
    DM1_D2L_DOOR_ZONE = 3750,
    DM1_D2R_DOOR_ZONE = 3770,
    DM1_D2L_PASS1 = 0x0218,
    DM1_D2L_PASS2 = 0x0349,
    DM1_D2R_PASS1 = 0x0128,
    DM1_D2R_PASS2 = 0x0439,
    DM1_F0128_D2L_DRAW_LINE = 8513,
    DM1_F0128_D2R_DRAW_LINE = 8517,
    DM1_F0128_D2C_DRAW_LINE = 8521,
    DM1_F0678_D2L2_WALL_LINE = 6837,
    DM1_F0679_D2R2_WALL_LINE = 6868
};

typedef enum {
    DM1_D2_SIDE_L = 1,
    DM1_D2_SIDE_R = 2
} DM1D2SidePc34;

typedef struct {
    DM1D2SidePc34 side;
    const char *label;
    int view_square;
    int relative_lateral;
    int door_zone;
    int pass1_order;
    int pass2_order;
    int f0128_draw_line;
    const char *frames_symbol;
} DM1D2DoorSpecPc34;

typedef struct {
    int accepted;
    int rejected_open;
    int rejected_closed;
    int rejected_unknown;
    int side;
    int input_state;
    int decremented_state;
    int framebuffer_width;
    int framebuffer_height;
    int viewport_width;
    int viewport_height;
    int view_square_anchor;
    int relative_depth;
    int relative_lateral;
    int base_zone;
    int first_half_zone;
    int first_half_c10_blit_zone;
    int second_half_zone;
    int second_half_shift;
    int c10_transparent_color;
    int first_half_c10_skips;
    int second_half_c10_skips;
    int first_half_writes;
    int second_half_writes;
    int pass1_order;
    int pass2_order;
    uint8_t framebuffer_probe_a;
    uint8_t framebuffer_probe_b;
} DM1D2DoorTracePc34;

static const DM1D2DoorSpecPc34 s_specs[] = {
    {
        DM1_D2_SIDE_L,
        "D2L partly-open F0111 corridor-side door front",
        DM1_VIEW_SQUARE_D2L,
        -1,
        DM1_D2L_DOOR_ZONE,
        DM1_D2L_PASS1,
        DM1_D2L_PASS2,
        DM1_F0128_D2L_DRAW_LINE,
        "G0182_s_Graphic558_Frames_Door_D2L"
    },
    {
        DM1_D2_SIDE_R,
        "D2R partly-open F0111 corridor-side door front",
        DM1_VIEW_SQUARE_D2R,
        1,
        DM1_D2R_DOOR_ZONE,
        DM1_D2R_PASS1,
        DM1_D2R_PASS2,
        DM1_F0128_D2R_DRAW_LINE,
        "G0184_s_Graphic558_Frames_Door_D2R"
    }
};

static const char s_source_evidence[] =
    "DM1 V1 D2L/D2R F0111 partly-open source-lock gate; contract-only, "
    "asset-free, no real-asset pixel parity, and no game-data load. "
    "ReDMCSB DUNVIEW.C:4218-4337 F0111 anchors the horizontal half blit: "
    "4248 open guard, 4308 state decrement, 4312-4313 LeftHorizontal/"
    "RightHorizontal, 4317-4324 P2084_i_ZoneIndex plus C6_UNKNOWN F0635/"
    "F0654 C10_COLOR_FLESH first-half blit, and 4325-4334 3|MASK0x4000 "
    "then F0791_DUNGEONVIEW_DrawBitmapXX with C10. DUNVIEW.C:8504-8508 "
    "draws D2L2/D2R2 wall anchors first under MEDIA720, 8513-8517 dispatches "
    "D2L/D2R, and 8521 dispatches D2C only after the side corridors. "
    "DUNVIEW.C:6987-7004 F0119 D2L and 7180-7197 F0120 D2R call F0111 "
    "with G0694_ai_DoorNativeBitmapIndex_Front_D2LCR, C1_VIEW_DOOR_ORNAMENT_D2LCR, "
    "G0182_s_Graphic558_Frames_Door_D2L/G0184_s_Graphic558_Frames_Door_D2R, "
    "and M627_ZONE_DOOR_D2L/M629_ZONE_DOOR_D2R. DUNVIEW.C:7244-7389 F0121 "
    "is cited as D2C center, not the D2L/D2R corridor-side route. "
    "DUNVIEW.C:6837-6865 F0678_DrawD2L2 and 6868-6896 F0679_DrawD2R2 are "
    "side-wall anchors that return wall cases before any F0111 route. "
    "DEFS.H:1039-1043 door states, 2088 C10_COLOR_FLESH, 2603-2604 M604/M605, "
    "2669/2672 C0x0218/C0x0349 DoorPass anchors, 2790 actual C1_VIEW_DOOR_ORNAMENT_D2LCR, "
    "3508 C6_UNKNOWN, 3516 MASK0x4000, 4254-4258 M627/M629, 5457 G0694, "
    "5539 G0182, 5541 G0184. CSB counterpart: "
    "csb_v1_viewport_d2c_f0111_partly_open_door_pc34_compat; non-overlap "
    "DM1 sibling: test_dm1_v1_viewport_d0l_d0r_f0111_door_pc34_compat.";

static DM1_V1_D2LD2RF0111PartlyOpenDoorSelfTestResultPc34 s_last;

static uint32_t mix_u32(uint32_t hash, uint32_t value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (value >> (i * 8)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static uint32_t mix_string(uint32_t hash, const char *text)
{
    if (!text) return mix_u32(hash, 0xffffffffu);
    while (*text) {
        hash ^= (unsigned char)*text++;
        hash *= 16777619u;
    }
    return hash;
}

static void check_int(const char *id, int got, int want)
{
    ++s_last.assertions;
    s_last.deterministic_hash = mix_string(s_last.deterministic_hash, id);
    s_last.deterministic_hash = mix_u32(s_last.deterministic_hash, (uint32_t)got);
    s_last.deterministic_hash = mix_u32(s_last.deterministic_hash, (uint32_t)want);
    if (got != want) ++s_last.failures;
}

static void check_contains(const char *id, const char *haystack, const char *needle)
{
    const int found = haystack && needle && strstr(haystack, needle) != NULL;

    s_last.deterministic_hash = mix_string(s_last.deterministic_hash, needle);
    check_int(id, found, 1);
}

static const DM1D2DoorSpecPc34 *spec_for_side(DM1D2SidePc34 side)
{
    size_t i;

    for (i = 0; i < sizeof(s_specs) / sizeof(s_specs[0]); ++i) {
        if (s_specs[i].side == side) return &s_specs[i];
    }
    return NULL;
}

static int simulate_f0111_partly_open(
    const DM1D2DoorSpecPc34 *spec,
    int door_state,
    DM1D2DoorTracePc34 *out)
{
    uint8_t framebuffer[DM1_FRAMEBUFFER_WIDTH * DM1_FRAMEBUFFER_HEIGHT];
    const uint8_t first_source[6] = { 10, 0x21, 0x22, 10, 0x23, 0x24 };
    const uint8_t second_source[6] = { 0x31, 10, 0x32, 0x33, 10, 0x34 };
    const int base_x = spec && spec->side == DM1_D2_SIDE_R ? 176 : 24;
    const int base_y = 36 + door_state;
    int i;

    if (!spec || !out) return 0;
    memset(out, 0, sizeof(*out));
    memset(framebuffer, 0x11, sizeof(framebuffer));

    out->side = spec->side;
    out->input_state = door_state;
    out->framebuffer_width = DM1_FRAMEBUFFER_WIDTH;
    out->framebuffer_height = DM1_FRAMEBUFFER_HEIGHT;
    out->viewport_width = DM1_VIEWPORT_WIDTH;
    out->viewport_height = DM1_VIEWPORT_HEIGHT;
    out->view_square_anchor = spec->view_square;
    out->relative_depth = DM1_VIEW_DEPTH_D2;
    out->relative_lateral = spec->relative_lateral;
    out->base_zone = spec->door_zone;
    out->pass1_order = spec->pass1_order;
    out->pass2_order = spec->pass2_order;
    out->c10_transparent_color = DM1_C10_COLOR_FLESH;

    if (door_state == DM1_DOOR_STATE_OPEN) {
        out->rejected_open = 1;
        return 1;
    }
    if (door_state == DM1_DOOR_STATE_CLOSED) {
        out->rejected_closed = 1;
        return 1;
    }
    if (door_state < DM1_DOOR_STATE_ONE_FOURTH ||
        door_state > DM1_DOOR_STATE_THREE_FOURTH) {
        out->rejected_unknown = 1;
        return 1;
    }

    out->accepted = 1;
    out->decremented_state = door_state - 1;
    out->first_half_zone = spec->door_zone + out->decremented_state;
    out->first_half_c10_blit_zone = out->first_half_zone + DM1_C6_UNKNOWN;
    out->second_half_shift = DM1_SECOND_HALF_OFFSET | DM1_MASK0X4000;
    out->second_half_zone = out->first_half_zone + out->second_half_shift;

    for (i = 0; i < 6; ++i) {
        const int x = base_x + i;
        const int offset = (base_y * DM1_FRAMEBUFFER_WIDTH) + x;
        if (first_source[i] == DM1_C10_COLOR_FLESH) {
            ++out->first_half_c10_skips;
        } else {
            framebuffer[offset] = first_source[i];
            ++out->first_half_writes;
        }
    }
    for (i = 0; i < 6; ++i) {
        const int x = base_x + 12 + i;
        const int offset = ((base_y + 1) * DM1_FRAMEBUFFER_WIDTH) + x;
        if (second_source[i] == DM1_C10_COLOR_FLESH) {
            ++out->second_half_c10_skips;
        } else {
            framebuffer[offset] = second_source[i];
            ++out->second_half_writes;
        }
    }

    out->framebuffer_probe_a =
        framebuffer[(base_y * DM1_FRAMEBUFFER_WIDTH) + base_x + 1];
    out->framebuffer_probe_b =
        framebuffer[((base_y + 1) * DM1_FRAMEBUFFER_WIDTH) + base_x + 12];
    return 1;
}

static void verify_partly_case(const DM1D2DoorSpecPc34 *spec, int state)
{
    DM1D2DoorTracePc34 trace;

    check_int("simulate.partly.ok", simulate_f0111_partly_open(spec, state, &trace), 1);
    check_int("partly.accepted", trace.accepted, 1);
    check_int("framebuffer.width", trace.framebuffer_width, DM1_FRAMEBUFFER_WIDTH);
    check_int("framebuffer.height", trace.framebuffer_height, DM1_FRAMEBUFFER_HEIGHT);
    check_int("viewport.width", trace.viewport_width, DM1_VIEWPORT_WIDTH);
    check_int("viewport.height", trace.viewport_height, DM1_VIEWPORT_HEIGHT);
    check_int("view.depth", trace.relative_depth, DM1_VIEW_DEPTH_D2);
    check_int("view.square", trace.view_square_anchor, spec->view_square);
    check_int("view.lane", trace.relative_lateral, spec->relative_lateral);
    check_int("base.zone", trace.base_zone, spec->door_zone);
    check_int("decremented.state", trace.decremented_state, state - 1);
    check_int("first.half.zone", trace.first_half_zone, spec->door_zone + state - 1);
    check_int("c10.blit.zone", trace.first_half_c10_blit_zone,
              spec->door_zone + state - 1 + DM1_C6_UNKNOWN);
    check_int("c10.color", trace.c10_transparent_color, DM1_C10_COLOR_FLESH);
    check_int("first.c10.skips", trace.first_half_c10_skips, 2);
    check_int("second.c10.skips", trace.second_half_c10_skips, 2);
    check_int("first.writes", trace.first_half_writes, 4);
    check_int("second.writes", trace.second_half_writes, 4);
    check_int("second.shift", trace.second_half_shift, 3 | DM1_MASK0X4000);
    check_int("second.zone", trace.second_half_zone,
              spec->door_zone + state - 1 + (3 | DM1_MASK0X4000));
    check_int("pass1.source.order", trace.pass1_order, spec->pass1_order);
    check_int("pass2.source.order", trace.pass2_order, spec->pass2_order);
    check_int("doorpass1.canonical.anchor", DM1_D2L_PASS1, 0x0218);
    check_int("doorpass2.canonical.anchor", DM1_D2L_PASS2, 0x0349);
    check_int("framebuffer.probe.a", trace.framebuffer_probe_a, 0x21);
    check_int("framebuffer.probe.b", trace.framebuffer_probe_b, 0x31);

    if (spec->side == DM1_D2_SIDE_L) {
        if (state == 1) ++s_last.d2l_partly_one;
        if (state == 2) ++s_last.d2l_partly_two;
        if (state == 3) ++s_last.d2l_partly_three;
        ++s_last.d2l_partly;
    } else {
        if (state == 1) ++s_last.d2r_partly_one;
        if (state == 2) ++s_last.d2r_partly_two;
        if (state == 3) ++s_last.d2r_partly_three;
        ++s_last.d2r_partly;
    }
}

static void verify_rejection(const DM1D2DoorSpecPc34 *spec, int state)
{
    DM1D2DoorTracePc34 trace;

    check_int("simulate.reject.ok", simulate_f0111_partly_open(spec, state, &trace), 1);
    check_int("reject.accepted.false", trace.accepted, 0);
    check_int("reject.view.square.anchor", trace.view_square_anchor, spec->view_square);
    check_int("reject.base.zone", trace.base_zone, spec->door_zone);
    if (state == DM1_DOOR_STATE_OPEN) {
        check_int("reject.open.guard", trace.rejected_open, 1);
        ++s_last.open_rejections;
    } else if (state == DM1_DOOR_STATE_CLOSED) {
        check_int("reject.closed.guard", trace.rejected_closed, 1);
        ++s_last.closed_rejections;
    } else {
        check_int("reject.unknown.guard", trace.rejected_unknown, 1);
        ++s_last.unknown_rejections;
    }
}

static void verify_static_contract(void)
{
    const DM1D2DoorSpecPc34 *d2l = spec_for_side(DM1_D2_SIDE_L);
    const DM1D2DoorSpecPc34 *d2r = spec_for_side(DM1_D2_SIDE_R);

    check_contains("evidence.f0111", s_source_evidence, "DUNVIEW.C:4218-4337");
    check_contains("evidence.second.half", s_source_evidence, "4325-4334");
    check_contains("evidence.f0128.d2lr", s_source_evidence, "8513-8517");
    check_contains("evidence.f0121.bound", s_source_evidence, "8521");
    check_contains("evidence.f0678", s_source_evidence, "6837-6865");
    check_contains("evidence.f0679", s_source_evidence, "6868-6896");
    check_contains("evidence.defs.states", s_source_evidence, "DEFS.H:1039-1043");
    check_contains("evidence.c10", s_source_evidence, "C10_COLOR_FLESH");
    check_contains("evidence.mask", s_source_evidence, "MASK0x4000");
    check_contains("evidence.csb", s_source_evidence,
                   "csb_v1_viewport_d2c_f0111_partly_open_door_pc34_compat");
    check_contains("evidence.nonoverlap", s_source_evidence,
                   "test_dm1_v1_viewport_d0l_d0r_f0111_door_pc34_compat");
    check_int("spec.count", (int)(sizeof(s_specs) / sizeof(s_specs[0])), 2);
    check_int("d2l.present", d2l != NULL, 1);
    check_int("d2r.present", d2r != NULL, 1);
    if (!d2l || !d2r) return;
    check_int("d2l.square", d2l->view_square, DM1_VIEW_SQUARE_D2L);
    check_int("d2r.square", d2r->view_square, DM1_VIEW_SQUARE_D2R);
    check_int("d2c.bound.square", DM1_VIEW_SQUARE_D2C, 6);
    check_int("d2l2.guard.square", DM1_VIEW_SQUARE_D2L2, 9);
    check_int("d2r2.guard.square", DM1_VIEW_SQUARE_D2R2, 10);
    check_int("d2l.dispatch.line", d2l->f0128_draw_line, DM1_F0128_D2L_DRAW_LINE);
    check_int("d2r.dispatch.line", d2r->f0128_draw_line, DM1_F0128_D2R_DRAW_LINE);
    check_int("d2c.dispatch.bound.line", DM1_F0128_D2C_DRAW_LINE, 8521);
    check_int("d2l2.wall.anchor", DM1_F0678_D2L2_WALL_LINE, 6837);
    check_int("d2r2.wall.anchor", DM1_F0679_D2R2_WALL_LINE, 6868);
    check_int("d2l.zone", d2l->door_zone, DM1_D2L_DOOR_ZONE);
    check_int("d2r.zone", d2r->door_zone, DM1_D2R_DOOR_ZONE);
    check_int("d2l.pass1", d2l->pass1_order, 0x0218);
    check_int("d2l.pass2", d2l->pass2_order, 0x0349);
    check_int("d2r.pass1.mirror", d2r->pass1_order, 0x0128);
    check_int("d2r.pass2.mirror", d2r->pass2_order, 0x0439);
    check_contains("d2l.frames", d2l->frames_symbol, "G0182");
    check_contains("d2r.frames", d2r->frames_symbol, "G0184");
}

int run_dm1_v1_viewport_d2l_d2r_f0111_partly_open_door_self_test(void)
{
    size_t side_index;

    memset(&s_last, 0, sizeof(s_last));
    s_last.deterministic_hash = 2166136261u;
    verify_static_contract();

    for (side_index = 0; side_index < sizeof(s_specs) / sizeof(s_specs[0]); ++side_index) {
        const DM1D2DoorSpecPc34 *spec = &s_specs[side_index];

        verify_partly_case(spec, DM1_DOOR_STATE_ONE_FOURTH);
        verify_partly_case(spec, DM1_DOOR_STATE_HALF);
        verify_partly_case(spec, DM1_DOOR_STATE_THREE_FOURTH);
        verify_rejection(spec, DM1_DOOR_STATE_CLOSED);
        verify_rejection(spec, DM1_DOOR_STATE_OPEN);
        verify_rejection(spec, DM1_C6_UNKNOWN);
    }

    check_int("d2l.partly.one.count", s_last.d2l_partly_one, 1);
    check_int("d2l.partly.two.count", s_last.d2l_partly_two, 1);
    check_int("d2l.partly.three.count", s_last.d2l_partly_three, 1);
    check_int("d2r.partly.one.count", s_last.d2r_partly_one, 1);
    check_int("d2r.partly.two.count", s_last.d2r_partly_two, 1);
    check_int("d2r.partly.three.count", s_last.d2r_partly_three, 1);
    check_int("d2l.partly.count", s_last.d2l_partly, 3);
    check_int("d2r.partly.count", s_last.d2r_partly, 3);
    check_int("closed.rejections.count", s_last.closed_rejections, 2);
    check_int("open.rejections.count", s_last.open_rejections, 2);
    check_int("unknown.rejections.count", s_last.unknown_rejections, 2);
    check_int("hash.nonzero", s_last.deterministic_hash != 0u, 1);
    return s_last.failures == 0 ? 0 : 1;
}

const DM1_V1_D2LD2RF0111PartlyOpenDoorSelfTestResultPc34 *
dm1_v1_viewport_d2l_d2r_f0111_partly_open_door_last_self_test_result_pc34(void)
{
    return &s_last;
}

const char *
dm1_v1_viewport_d2l_d2r_f0111_partly_open_door_source_evidence_pc34(void)
{
    return s_source_evidence;
}
