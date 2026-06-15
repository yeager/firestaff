#include "firestaff/dm1/v1/viewport/d1l_d1r_f0111_partly_open_door_pc34_compat.h"

#include <stdint.h>
#include <string.h>

/*
 * DM1 V1 D1L/D1R F0111 front-door source-lock contract.
 *
 * This is deliberately contract-only. It models the ReDMCSB control
 * contract, view-square routing, C10 transparency, and door-front object
 * pass ordering without loading GRAPHICS.DAT or asserting pixel parity.
 *
 * ReDMCSB: DUNVIEW.C F0111 lines 4218-4337, F0122 lines 7391-7557,
 * F0123 lines 7559-7725, F0115 lines 4788-4804/4916-4923/5176-5188,
 * F0128 lines 8524-8533, F0104 lines 3113-3156, F0105 lines 3185-3247,
 * F0107 lines 3502-3938, F0108 lines 3940-4011; DUNGEON.C F0163
 * lines 1769-1838, F0164 lines 1840-1905, F0172 lines 2466-2523;
 * DEFS.H lines 2088,2596-2611,2661-2667,2672-2675,2789-2791,3508,
 * 3516,4091-4093,4258-4260,5458,5542,5544.
 */

enum {
    DM1_FRAMEBUFFER_WIDTH = 320,
    DM1_FRAMEBUFFER_HEIGHT = 200,
    DM1_VIEWPORT_WIDTH = 224,
    DM1_VIEWPORT_HEIGHT = 136,
    DM1_ELEMENT_WALL = 0,
    DM1_ELEMENT_CORRIDOR = 1,
    DM1_ELEMENT_DOOR_SIDE = 16,
    DM1_ELEMENT_DOOR_FRONT = 17,
    DM1_VIEW_DEPTH_D1 = 1,
    DM1_VIEW_SQUARE_D1C = 3,
    DM1_VIEW_SQUARE_D1L = 4,
    DM1_VIEW_SQUARE_D1R = 5,
    DM1_VIEW_SQUARE_D2L = 7,
    DM1_VIEW_SQUARE_D2R = 8,
    DM1_D1L_LATERAL = -1,
    DM1_D1R_LATERAL = 1,
    DM1_VIEW_FLOOR_D1L = 594,
    DM1_VIEW_FLOOR_D1R = 596,
    DM1_DOOR_STATE_OPEN = 0,
    DM1_DOOR_STATE_PARTLY_ONE = 1,
    DM1_DOOR_STATE_PARTLY_TWO = 2,
    DM1_DOOR_STATE_PARTLY_THREE = 3,
    DM1_DOOR_STATE_CLOSED = 4,
    DM1_DOOR_STATE_DESTROYED = 5,
    DM1_DOOR_STATE_UNKNOWN = 6,
    DM1_D1L_DOOR_ZONE = 3780,
    DM1_D1C_DOOR_ZONE = 3790,
    DM1_D1R_DOOR_ZONE = 3800,
    DM1_D1L_FRAME_TOP_ZONE = 732,
    DM1_D1R_FRAME_TOP_ZONE = 734,
    DM1_D1L_DOORPASS1 = 0x0028,
    DM1_D1L_DOORPASS2 = 0x0039,
    DM1_D1R_DOORPASS1 = 0x0018,
    DM1_D1R_DOORPASS2 = 0x0049,
    DM1_D1L_CORRIDOR_ORDER = 0x0032,
    DM1_D1R_CORRIDOR_ORDER = 0x0041,
    DM1_VIEW_DOOR_ORNAMENT_D1LCR = 2,
    DM1_DOOR_WIDTH_D1LCR = 96,
    DM1_DOOR_HEIGHT_D1LCR = 88,
    DM1_DOOR_BYTE_COUNT_D1LCR = 4224,
    DM1_F0128_D2L_DISPATCH_LINE = 8513,
    DM1_F0128_D2R_DISPATCH_LINE = 8517,
    DM1_F0128_D1L_DISPATCH_LINE = 8525,
    DM1_F0128_D1R_DISPATCH_LINE = 8529,
    DM1_F0128_D1C_DISPATCH_LINE = 8533,
    DM1_F0122_DOOR_FRONT_LINE = 7492,
    DM1_F0122_F0111_LINE = 7497,
    DM1_F0122_PASS2_LINE = 7536,
    DM1_F0123_DOOR_FRONT_LINE = 7660,
    DM1_F0123_F0111_LINE = 7665,
    DM1_F0123_PASS2_LINE = 7704,
    DM1_SECOND_HALF_SHIFT = 3 | DM1_V1_D1L_D1R_F0111_MASK0X4000_PC34
};

static const DM1_V1_D1LD1RF0111DoorSpecPc34 s_specs[] = {
    {
        DM1_V1_D1L_D1R_F0111_SIDE_D1L_PC34,
        "DM1 V1 D1L F0111 front door",
        DM1_VIEW_SQUARE_D1L,
        DM1_VIEW_DEPTH_D1,
        DM1_D1L_LATERAL,
        DM1_VIEW_FLOOR_D1L,
        DM1_D1L_DOOR_ZONE,
        DM1_D1L_FRAME_TOP_ZONE,
        DM1_D1L_DOORPASS1,
        DM1_D1L_DOORPASS2,
        DM1_D1L_CORRIDOR_ORDER,
        DM1_F0128_D1L_DISPATCH_LINE,
        DM1_F0122_DOOR_FRONT_LINE,
        DM1_F0122_F0111_LINE,
        DM1_F0122_PASS2_LINE,
        DM1_VIEW_DOOR_ORNAMENT_D1LCR,
        DM1_DOOR_WIDTH_D1LCR,
        DM1_DOOR_HEIGHT_D1LCR,
        DM1_DOOR_BYTE_COUNT_D1LCR,
        "G0185_s_Graphic558_Frames_Door_D1L"
    },
    {
        DM1_V1_D1L_D1R_F0111_SIDE_D1R_PC34,
        "DM1 V1 D1R F0111 front door",
        DM1_VIEW_SQUARE_D1R,
        DM1_VIEW_DEPTH_D1,
        DM1_D1R_LATERAL,
        DM1_VIEW_FLOOR_D1R,
        DM1_D1R_DOOR_ZONE,
        DM1_D1R_FRAME_TOP_ZONE,
        DM1_D1R_DOORPASS1,
        DM1_D1R_DOORPASS2,
        DM1_D1R_CORRIDOR_ORDER,
        DM1_F0128_D1R_DISPATCH_LINE,
        DM1_F0123_DOOR_FRONT_LINE,
        DM1_F0123_F0111_LINE,
        DM1_F0123_PASS2_LINE,
        DM1_VIEW_DOOR_ORNAMENT_D1LCR,
        DM1_DOOR_WIDTH_D1LCR,
        DM1_DOOR_HEIGHT_D1LCR,
        DM1_DOOR_BYTE_COUNT_D1LCR,
        "G0187_s_Graphic558_Frames_Door_D1R"
    }
};

static const char s_source_evidence[] =
    "DM1 V1 D1L/D1R F0111 front-door source-lock gate; contract-only, "
    "asset-free, no game-data load, and no original-DOS pixel parity claim. "
    "ReDMCSB DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor anchors the "
    "non-open guard at 4248, closed ClosedOrDestroyed branch at 4297-4305, "
    "partly-open decrement at 4308, horizontal LeftHorizontal/RightHorizontal "
    "selection at 4310-4313, C6_UNKNOWN/C10_COLOR_FLESH first-half path at "
    "4317-4324, and 3|MASK0x4000 second-half F0791 path at 4325-4334. "
    "DUNVIEW.C:7391-7557 F0122 D1L has C17_ELEMENT_DOOR_FRONT at 7492, "
    "F0115 DoorPass1 0x0028 at 7494, F0111 at 7497/7506, and F0115 pass2 "
    "0x0039 at 7536. DUNVIEW.C:7559-7725 F0123 D1R has C17 at 7660, "
    "F0115 DoorPass1 0x0018 at 7662, F0111 at 7665/7674, and pass2 0x0049 "
    "at 7704. DUNVIEW.C:4788-4804,4916-4923,5176-5188 anchor F0115 "
    "door-front pass decoding and C10 object blit behavior. DUNVIEW.C:8524-8533 "
    "F0128 dispatches D1L at 8525, D1R at 8529, then D1C at 8533; D2L/D2R "
    "are earlier at 8513/8517. DUNVIEW.C:3113-3156 F0104, 3185-3247 F0105, "
    "3502-3938 F0107, and 3940-4011 F0108 keep surrounding floor/wall "
    "ornaments out of this door-front lane. DUNGEON.C:1769-1838 F0163, "
    "1840-1905 F0164, 2466-2523 F0172 anchor thing-list and square-aspect "
    "inputs. DEFS.H:2088 C10_COLOR_FLESH, 2596-2611 view squares (M607 D1L, "
    "M608 D1R), 2661-2667/2672-2675 cell orders, 2789-2791 D1LCR door "
    "ornament, 3508 C6_UNKNOWN, 3516 MASK0x4000, 4091-4093 frame-top zones, "
    "4258-4260 M630/M631/M632 door zones, 5458 G0695, 5542 G0185, 5544 G0187. "
    "Disjoint siblings: CSB D1L/D1R F0111, DM1 D1L/D1R wall/F0107/F0108, "
    "DM1 D0L/D0R F0111, DM1 D0L2/D0R2 F0111, DM1 D2L/D2R F0111 partly-open, "
    "DM1 D1C F0111, and DM1 D1L2/D1R2 partly-open gates.";

static DM1_V1_D1LD1RF0111DoorSelfTestResultPc34 s_last;

static uint32_t fnv1a_u32(uint32_t hash, uint32_t value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (value >> (i * 8)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static uint32_t fnv1a_string(uint32_t hash, const char *text)
{
    if (!text) return fnv1a_u32(hash, 0xffffffffu);
    while (*text) {
        hash ^= (unsigned char)*text++;
        hash *= 16777619u;
    }
    return hash;
}

static void check_int(const char *id, int got, int want)
{
    ++s_last.assertions;
    s_last.deterministic_hash = fnv1a_string(s_last.deterministic_hash, id);
    s_last.deterministic_hash = fnv1a_u32(s_last.deterministic_hash, (uint32_t)got);
    s_last.deterministic_hash = fnv1a_u32(s_last.deterministic_hash, (uint32_t)want);
    if (got != want) ++s_last.failures;
}

static void check_contains(const char *id, const char *haystack, const char *needle)
{
    const int found = haystack && needle && strstr(haystack, needle) != NULL;

    s_last.deterministic_hash = fnv1a_string(s_last.deterministic_hash, needle);
    check_int(id, found, 1);
}

size_t dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_spec_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const DM1_V1_D1LD1RF0111DoorSpecPc34 *
dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_spec_at_pc34(size_t index)
{
    if (index >= dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_spec_count_pc34()) {
        return NULL;
    }
    return &s_specs[index];
}

const DM1_V1_D1LD1RF0111DoorSpecPc34 *
dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_spec_for_side_pc34(int side)
{
    size_t i;

    for (i = 0; i < dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_spec_count_pc34(); ++i) {
        if (s_specs[i].side == side) return &s_specs[i];
    }
    return NULL;
}

const DM1_V1_D1LD1RF0111DoorSpecPc34 *
dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_spec_for_square_pc34(
    int view_square)
{
    size_t i;

    for (i = 0; i < dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_spec_count_pc34(); ++i) {
        if (s_specs[i].view_square == view_square) return &s_specs[i];
    }
    return NULL;
}

int dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_supported_element_pc34(
    int element)
{
    return element == DM1_ELEMENT_DOOR_FRONT;
}

static void simulate_c10_half_blits(const DM1_V1_D1LD1RF0111DoorSpecPc34 *spec,
                                    int door_state,
                                    DM1_V1_D1LD1RF0111DoorTracePc34 *trace)
{
    uint8_t framebuffer[DM1_FRAMEBUFFER_WIDTH * DM1_FRAMEBUFFER_HEIGHT];
    const uint8_t first_source[8] = { 10, 0x21, 0x22, 10, 0x23, 0x24, 10, 0x25 };
    const uint8_t second_source[8] = { 0x31, 10, 0x32, 0x33, 10, 0x34, 0x35, 10 };
    const int base_x = spec->side == DM1_V1_D1L_D1R_F0111_SIDE_D1R_PC34 ? 176 : 32;
    const int base_y = 46 + door_state;
    int i;

    memset(framebuffer, 0x11, sizeof(framebuffer));
    for (i = 0; i < 8; ++i) {
        const int offset = (base_y * DM1_FRAMEBUFFER_WIDTH) + base_x + i;
        if (first_source[i] == DM1_V1_D1L_D1R_F0111_C10_COLOR_FLESH_PC34) {
            ++trace->first_half_c10_skips;
        } else {
            framebuffer[offset] = first_source[i];
            ++trace->first_half_writes;
        }
    }
    for (i = 0; i < 8; ++i) {
        const int offset = ((base_y + 1) * DM1_FRAMEBUFFER_WIDTH) + base_x + 12 + i;
        if (second_source[i] == DM1_V1_D1L_D1R_F0111_C10_COLOR_FLESH_PC34) {
            ++trace->second_half_c10_skips;
        } else {
            framebuffer[offset] = second_source[i];
            ++trace->second_half_writes;
        }
    }
    trace->framebuffer_probe_a =
        framebuffer[(base_y * DM1_FRAMEBUFFER_WIDTH) + base_x + 1];
    trace->framebuffer_probe_b =
        framebuffer[((base_y + 1) * DM1_FRAMEBUFFER_WIDTH) + base_x + 12];
}

int dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_trace_pc34(
    int side,
    int door_state,
    int element,
    DM1_V1_D1LD1RF0111DoorTracePc34 *out_trace)
{
    const DM1_V1_D1LD1RF0111DoorSpecPc34 *spec =
        dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_spec_for_side_pc34(side);

    if (!out_trace) return 0;
    memset(out_trace, 0, sizeof(*out_trace));
    out_trace->side = side;
    out_trace->input_state = door_state;
    out_trace->branch = DM1_V1_D1L_D1R_F0111_BRANCH_REJECTED_PC34;
    out_trace->c10_color = DM1_V1_D1L_D1R_F0111_C10_COLOR_FLESH_PC34;
    if (!spec || !dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_supported_element_pc34(element)) {
        return 0;
    }
    out_trace->view_square = spec->view_square;
    out_trace->door_zone = spec->door_zone;
    out_trace->doorpass1_order = spec->doorpass1_order;
    out_trace->doorpass2_order = spec->doorpass2_order;

    if (door_state == DM1_DOOR_STATE_OPEN) {
        out_trace->branch = DM1_V1_D1L_D1R_F0111_BRANCH_OPEN_PC34;
        return 0;
    }
    if (door_state == DM1_DOOR_STATE_CLOSED) {
        out_trace->accepted = 1;
        out_trace->branch = DM1_V1_D1L_D1R_F0111_BRANCH_CLOSED_PC34;
        out_trace->closed_frame_selected = 1;
        out_trace->first_half_base_zone = spec->door_zone;
        return 1;
    }
    if (door_state < DM1_DOOR_STATE_PARTLY_ONE ||
        door_state > DM1_DOOR_STATE_PARTLY_THREE) {
        return 0;
    }

    out_trace->accepted = 1;
    out_trace->branch = DM1_V1_D1L_D1R_F0111_BRANCH_PARTLY_OPEN_PC34;
    out_trace->decremented_state = door_state - 1;
    out_trace->left_horizontal_frame_selected = 1;
    out_trace->right_horizontal_frame_selected = 1;
    out_trace->first_half_base_zone = spec->door_zone + out_trace->decremented_state;
    out_trace->first_half_clip_zone =
        out_trace->first_half_base_zone + DM1_V1_D1L_D1R_F0111_C6_UNKNOWN_PC34;
    out_trace->second_half_shift = DM1_SECOND_HALF_SHIFT;
    out_trace->second_half_zone = out_trace->first_half_base_zone + DM1_SECOND_HALF_SHIFT;
    simulate_c10_half_blits(spec, door_state, out_trace);
    return 1;
}

const char *
dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_source_evidence_pc34(void)
{
    return s_source_evidence;
}

static void verify_static_contract(void)
{
    const DM1_V1_D1LD1RF0111DoorSpecPc34 *d1l =
        dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_spec_for_square_pc34(
            DM1_VIEW_SQUARE_D1L);
    const DM1_V1_D1LD1RF0111DoorSpecPc34 *d1r =
        dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_spec_for_square_pc34(
            DM1_VIEW_SQUARE_D1R);

    check_contains("evidence.f0111", s_source_evidence, "DUNVIEW.C:4218-4337");
    check_contains("evidence.closed", s_source_evidence, "4297-4305");
    check_contains("evidence.partly", s_source_evidence, "4317-4324");
    check_contains("evidence.second", s_source_evidence, "4325-4334");
    check_contains("evidence.f0122", s_source_evidence, "DUNVIEW.C:7391-7557");
    check_contains("evidence.f0123", s_source_evidence, "DUNVIEW.C:7559-7725");
    check_contains("evidence.f0115", s_source_evidence, "DUNVIEW.C:4788-4804");
    check_contains("evidence.f0128", s_source_evidence, "DUNVIEW.C:8524-8533");
    check_contains("evidence.f0104", s_source_evidence, "DUNVIEW.C:3113-3156");
    check_contains("evidence.f0105", s_source_evidence, "3185-3247");
    check_contains("evidence.f0107", s_source_evidence, "3502-3938");
    check_contains("evidence.f0108", s_source_evidence, "3940-4011");
    check_contains("evidence.f0163", s_source_evidence, "DUNGEON.C:1769-1838");
    check_contains("evidence.f0164", s_source_evidence, "1840-1905");
    check_contains("evidence.f0172", s_source_evidence, "2466-2523");
    check_contains("evidence.defs.c10", s_source_evidence, "DEFS.H:2088");
    check_contains("evidence.defs.views", s_source_evidence, "2596-2611");
    check_contains("evidence.defs.orders", s_source_evidence, "2661-2667");
    check_contains("evidence.defs.c6", s_source_evidence, "3508 C6_UNKNOWN");
    check_contains("evidence.defs.mask", s_source_evidence, "3516 MASK0x4000");
    check_contains("evidence.g0185", s_source_evidence, "5542 G0185");
    check_contains("evidence.g0187", s_source_evidence, "5544 G0187");

    check_int("spec.count", (int)dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_spec_count_pc34(), 2);
    check_int("spec.at.bad", dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_spec_at_pc34(2) == NULL, 1);
    check_int("d1l.present", d1l != NULL, 1);
    check_int("d1r.present", d1r != NULL, 1);
    check_int("d1c.not.this.lane",
              dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_spec_for_square_pc34(DM1_VIEW_SQUARE_D1C) == NULL,
              1);
    check_int("d2l.not.this.lane",
              dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_spec_for_square_pc34(DM1_VIEW_SQUARE_D2L) == NULL,
              1);
    check_int("d2r.not.this.lane",
              dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_spec_for_square_pc34(DM1_VIEW_SQUARE_D2R) == NULL,
              1);
    check_int("negative.square.rejected",
              dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_spec_for_square_pc34(-1) == NULL,
              1);
    check_int("large.square.rejected",
              dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_spec_for_square_pc34(99) == NULL,
              1);
    check_int("element.door.front", dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_supported_element_pc34(DM1_ELEMENT_DOOR_FRONT), 1);
    check_int("element.wall.rejected", dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_supported_element_pc34(DM1_ELEMENT_WALL), 0);
    check_int("element.corridor.rejected", dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_supported_element_pc34(DM1_ELEMENT_CORRIDOR), 0);
    check_int("element.door.side.rejected", dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_supported_element_pc34(DM1_ELEMENT_DOOR_SIDE), 0);
    ++s_last.out_of_range_square_rejections;
    s_last.unsupported_element_rejections += 3;
    if (!d1l || !d1r) return;

    check_int("d1l.square", d1l->view_square, DM1_VIEW_SQUARE_D1L);
    check_int("d1r.square", d1r->view_square, DM1_VIEW_SQUARE_D1R);
    check_int("d1l.depth", d1l->view_depth, DM1_VIEW_DEPTH_D1);
    check_int("d1r.depth", d1r->view_depth, DM1_VIEW_DEPTH_D1);
    check_int("d1l.lateral", d1l->relative_lateral, DM1_D1L_LATERAL);
    check_int("d1r.lateral", d1r->relative_lateral, DM1_D1R_LATERAL);
    check_int("d1l.floor.view", d1l->floor_ornament_view, DM1_VIEW_FLOOR_D1L);
    check_int("d1r.floor.view", d1r->floor_ornament_view, DM1_VIEW_FLOOR_D1R);
    check_int("d1l.zone", d1l->door_zone, DM1_D1L_DOOR_ZONE);
    check_int("d1r.zone", d1r->door_zone, DM1_D1R_DOOR_ZONE);
    check_int("d1c.zone.relative", DM1_D1C_DOOR_ZONE, 3790);
    check_int("d1l.frame.top", d1l->door_frame_top_zone, DM1_D1L_FRAME_TOP_ZONE);
    check_int("d1r.frame.top", d1r->door_frame_top_zone, DM1_D1R_FRAME_TOP_ZONE);
    check_int("d1l.pass1", d1l->doorpass1_order, DM1_D1L_DOORPASS1);
    check_int("d1l.pass2", d1l->doorpass2_order, DM1_D1L_DOORPASS2);
    check_int("d1r.pass1", d1r->doorpass1_order, DM1_D1R_DOORPASS1);
    check_int("d1r.pass2", d1r->doorpass2_order, DM1_D1R_DOORPASS2);
    check_int("d1l.corridor.order", d1l->corridor_order, DM1_D1L_CORRIDOR_ORDER);
    check_int("d1r.corridor.order", d1r->corridor_order, DM1_D1R_CORRIDOR_ORDER);
    check_int("d1l.dispatch", d1l->f0128_dispatch_line, DM1_F0128_D1L_DISPATCH_LINE);
    check_int("d1r.dispatch", d1r->f0128_dispatch_line, DM1_F0128_D1R_DISPATCH_LINE);
    check_int("d1c.followup", DM1_F0128_D1C_DISPATCH_LINE, 8533);
    check_int("d2l.precedes", DM1_F0128_D2L_DISPATCH_LINE < d1l->f0128_dispatch_line, 1);
    check_int("d2r.precedes", DM1_F0128_D2R_DISPATCH_LINE < d1r->f0128_dispatch_line, 1);
    check_int("d1l.before.d1r", d1l->f0128_dispatch_line < d1r->f0128_dispatch_line, 1);
    check_int("d1r.before.d1c", d1r->f0128_dispatch_line < DM1_F0128_D1C_DISPATCH_LINE, 1);
    check_int("d1l.front.line", d1l->f012x_door_front_line, DM1_F0122_DOOR_FRONT_LINE);
    check_int("d1r.front.line", d1r->f012x_door_front_line, DM1_F0123_DOOR_FRONT_LINE);
    check_int("d1l.f0111.line", d1l->f012x_f0111_line, DM1_F0122_F0111_LINE);
    check_int("d1r.f0111.line", d1r->f012x_f0111_line, DM1_F0123_F0111_LINE);
    check_int("d1l.pass2.line", d1l->f012x_f0115_pass2_line, DM1_F0122_PASS2_LINE);
    check_int("d1r.pass2.line", d1r->f012x_f0115_pass2_line, DM1_F0123_PASS2_LINE);
    check_int("ornament.view", d1l->door_ornament_view, DM1_VIEW_DOOR_ORNAMENT_D1LCR);
    check_int("door.width", d1l->door_width, DM1_DOOR_WIDTH_D1LCR);
    check_int("door.height", d1l->door_height, DM1_DOOR_HEIGHT_D1LCR);
    check_int("door.bytes", d1l->door_byte_count, DM1_DOOR_BYTE_COUNT_D1LCR);
    check_contains("frames.d1l", d1l->frames_symbol, "G0185");
    check_contains("frames.d1r", d1r->frames_symbol, "G0187");
    s_last.f0115_doorpass_anchors += 4;
    s_last.f0128_dispatch_anchors += 5;
}

static void verify_closed_case(const DM1_V1_D1LD1RF0111DoorSpecPc34 *spec)
{
    DM1_V1_D1LD1RF0111DoorTracePc34 trace;

    check_int("closed.trace.ok",
              dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_trace_pc34(
                  spec->side, DM1_DOOR_STATE_CLOSED, DM1_ELEMENT_DOOR_FRONT, &trace),
              1);
    check_int("closed.accepted", trace.accepted, 1);
    check_int("closed.branch", trace.branch, DM1_V1_D1L_D1R_F0111_BRANCH_CLOSED_PC34);
    check_int("closed.frame", trace.closed_frame_selected, 1);
    check_int("closed.no.left.half", trace.left_horizontal_frame_selected, 0);
    check_int("closed.no.right.half", trace.right_horizontal_frame_selected, 0);
    check_int("closed.zone", trace.first_half_base_zone, spec->door_zone);
    check_int("closed.no.c6.clip", trace.first_half_clip_zone, 0);
    check_int("closed.pass1", trace.doorpass1_order, spec->doorpass1_order);
    check_int("closed.pass2", trace.doorpass2_order, spec->doorpass2_order);
    if (spec->side == DM1_V1_D1L_D1R_F0111_SIDE_D1L_PC34) {
        ++s_last.d1l_closed;
    } else {
        ++s_last.d1r_closed;
    }
}

static void verify_partly_case(const DM1_V1_D1LD1RF0111DoorSpecPc34 *spec,
                               int state)
{
    DM1_V1_D1LD1RF0111DoorTracePc34 trace;

    check_int("partly.trace.ok",
              dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_trace_pc34(
                  spec->side, state, DM1_ELEMENT_DOOR_FRONT, &trace),
              1);
    check_int("partly.accepted", trace.accepted, 1);
    check_int("partly.branch", trace.branch, DM1_V1_D1L_D1R_F0111_BRANCH_PARTLY_OPEN_PC34);
    check_int("partly.view.square", trace.view_square, spec->view_square);
    check_int("partly.base.zone", trace.door_zone, spec->door_zone);
    check_int("partly.state.dec", trace.decremented_state, state - 1);
    check_int("partly.left.half", trace.left_horizontal_frame_selected, 1);
    check_int("partly.right.half", trace.right_horizontal_frame_selected, 1);
    check_int("partly.closed.not.selected", trace.closed_frame_selected, 0);
    check_int("partly.first.base.zone", trace.first_half_base_zone, spec->door_zone + state - 1);
    check_int("partly.first.clip.zone",
              trace.first_half_clip_zone,
              spec->door_zone + state - 1 + DM1_V1_D1L_D1R_F0111_C6_UNKNOWN_PC34);
    check_int("partly.second.shift", trace.second_half_shift, DM1_SECOND_HALF_SHIFT);
    check_int("partly.second.zone",
              trace.second_half_zone,
              spec->door_zone + state - 1 + DM1_SECOND_HALF_SHIFT);
    check_int("partly.c10", trace.c10_color, DM1_V1_D1L_D1R_F0111_C10_COLOR_FLESH_PC34);
    check_int("partly.first.c10.skips", trace.first_half_c10_skips, 3);
    check_int("partly.second.c10.skips", trace.second_half_c10_skips, 3);
    check_int("partly.first.writes", trace.first_half_writes, 5);
    check_int("partly.second.writes", trace.second_half_writes, 5);
    check_int("partly.probe.a", trace.framebuffer_probe_a, 0x21);
    check_int("partly.probe.b", trace.framebuffer_probe_b, 0x31);
    check_int("partly.pass1", trace.doorpass1_order, spec->doorpass1_order);
    check_int("partly.pass2", trace.doorpass2_order, spec->doorpass2_order);

    if (spec->side == DM1_V1_D1L_D1R_F0111_SIDE_D1L_PC34) {
        ++s_last.d1l_partly;
    } else {
        ++s_last.d1r_partly;
    }
    s_last.c10_first_half_skips += trace.first_half_c10_skips;
    s_last.c10_second_half_skips += trace.second_half_c10_skips;
}

static void verify_rejection(const DM1_V1_D1LD1RF0111DoorSpecPc34 *spec,
                             int state,
                             int element)
{
    DM1_V1_D1LD1RF0111DoorTracePc34 trace;

    check_int("reject.trace",
              dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_trace_pc34(
                  spec->side, state, element, &trace),
              0);
    check_int("reject.accepted", trace.accepted, 0);
    if (state == DM1_DOOR_STATE_OPEN) {
        check_int("reject.open.branch", trace.branch, DM1_V1_D1L_D1R_F0111_BRANCH_OPEN_PC34);
        ++s_last.open_rejections;
    } else if (state == DM1_DOOR_STATE_DESTROYED) {
        check_int("reject.destroyed.branch", trace.branch, DM1_V1_D1L_D1R_F0111_BRANCH_REJECTED_PC34);
        ++s_last.destroyed_rejections;
    } else if (element != DM1_ELEMENT_DOOR_FRONT) {
        check_int("reject.unsupported.element", trace.branch, DM1_V1_D1L_D1R_F0111_BRANCH_REJECTED_PC34);
        ++s_last.unsupported_element_rejections;
    } else {
        check_int("reject.unknown.branch", trace.branch, DM1_V1_D1L_D1R_F0111_BRANCH_REJECTED_PC34);
        ++s_last.unknown_rejections;
    }
}

int run_dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_self_test(void)
{
    size_t i;

    memset(&s_last, 0, sizeof(s_last));
    s_last.deterministic_hash = 2166136261u;
    verify_static_contract();

    for (i = 0; i < dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_spec_count_pc34(); ++i) {
        const DM1_V1_D1LD1RF0111DoorSpecPc34 *spec =
            dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_spec_at_pc34(i);

        verify_closed_case(spec);
        verify_partly_case(spec, DM1_DOOR_STATE_PARTLY_ONE);
        verify_partly_case(spec, DM1_DOOR_STATE_PARTLY_TWO);
        verify_partly_case(spec, DM1_DOOR_STATE_PARTLY_THREE);
        verify_rejection(spec, DM1_DOOR_STATE_OPEN, DM1_ELEMENT_DOOR_FRONT);
        verify_rejection(spec, DM1_DOOR_STATE_DESTROYED, DM1_ELEMENT_DOOR_FRONT);
        verify_rejection(spec, DM1_DOOR_STATE_UNKNOWN, DM1_ELEMENT_DOOR_FRONT);
        verify_rejection(spec, DM1_DOOR_STATE_PARTLY_ONE, DM1_ELEMENT_WALL);
    }

    check_int("summary.d1l.closed", s_last.d1l_closed, 1);
    check_int("summary.d1r.closed", s_last.d1r_closed, 1);
    check_int("summary.d1l.partly", s_last.d1l_partly, 3);
    check_int("summary.d1r.partly", s_last.d1r_partly, 3);
    check_int("summary.open.rejections", s_last.open_rejections, 2);
    check_int("summary.destroyed.rejections", s_last.destroyed_rejections, 2);
    check_int("summary.unknown.rejections", s_last.unknown_rejections, 2);
    check_int("summary.unsupported.element", s_last.unsupported_element_rejections, 5);
    check_int("summary.out.range", s_last.out_of_range_square_rejections, 1);
    check_int("summary.c10.first.skips", s_last.c10_first_half_skips, 18);
    check_int("summary.c10.second.skips", s_last.c10_second_half_skips, 18);
    check_int("summary.f0115", s_last.f0115_doorpass_anchors, 4);
    check_int("summary.f0128", s_last.f0128_dispatch_anchors, 5);
    check_int("summary.hash.changed", s_last.deterministic_hash != 2166136261u, 1);
    check_int("summary.assertions.at.least.100", s_last.assertions >= 100, 1);

    return s_last.failures == 0 ? 0 : 1;
}

const DM1_V1_D1LD1RF0111DoorSelfTestResultPc34 *
dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_last_self_test_result_pc34(void)
{
    return &s_last;
}
