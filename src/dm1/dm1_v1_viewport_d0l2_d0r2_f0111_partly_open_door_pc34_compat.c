#include "firestaff/dm1/v1/viewport/d0l2_d0r2_f0111_partly_open_door_pc34_compat.h"

#include "dm1_v1_door_bash_feedback_pc34_compat.h"

#include <stdint.h>
#include <string.h>

enum {
    DM1_PRESENT = 1,
    DM1_ABSENT = 0,
    DM1_SIDE_D0L2 = 1,
    DM1_SIDE_D0R2 = 2,
    DM1_VIEW_SQUARE_D0L = 1,
    DM1_VIEW_SQUARE_D0R = 2,
    DM1_F0125 = 125,
    DM1_F0126 = 126,
    DM1_D0L_DISPATCH_ORDER = 1,
    DM1_D0R_DISPATCH_ORDER = 2,
    DM1_D0L_DISPATCH_LINE = 8536,
    DM1_D0R_DISPATCH_LINE = 8540,
    DM1_D0_DEPTH = 0,
    DM1_D0L2_LANE = -2,
    DM1_D0R2_LANE = 2,
    DM1_ZONE_WALL_D0L = 716,
    DM1_ZONE_WALL_D0R = 717,
    DM1_ZONE_DOOR_D0L2 = 3720,
    DM1_ZONE_DOOR_D0R2 = 3740,
    DM1_ORDER_REAR_D0L2 = 0x0028,
    DM1_ORDER_REAR_D0R2 = 0x0018,
    DM1_ORDER_FRONT_D0L2 = 0x0039,
    DM1_ORDER_FRONT_D0R2 = 0x0049,
    DM1_DOOR_STATE_OPEN = 0,
    DM1_DOOR_STATE_PARTLY_ONE = 1,
    DM1_DOOR_STATE_PARTLY_TWO = 2,
    DM1_DOOR_STATE_PARTLY_THREE = 3,
    DM1_DOOR_STATE_CLOSED = 4,
    DM1_DOOR_STATE_DESTROYED = 5,
    DM1_OPEN_DEGREE_CLOSED = 0,
    DM1_OPEN_DEGREE_STEP_ONE = 8,
    DM1_OPEN_DEGREE_STEP_TWO = 16,
    DM1_OPEN_DEGREE_STEP_THREE = 24,
    DM1_OPEN_DEGREE_STEP_FOUR = 32,
    DM1_OPEN_DEGREE_OPEN = 64,
    DM1_C10_COLOR_FLESH = 10,
    DM1_FIRST_HALF_OFFSET = 6,
    DM1_SECOND_HALF_OFFSET = 3,
    DM1_MASK0X4000 = 0x4000
};

#define D0_SPEC(side_value, route, square, func, order, line, lane, wall_zone, \
                door_zone, rear_order, front_order, left_name, right_name) \
    { \
        side_value, route, DM1_PRESENT, DM1_PRESENT, DM1_PRESENT, square, \
        func, order, line, DM1_D0_DEPTH, lane, rear_order, front_order, \
        door_zone, wall_zone, DM1_DOOR_STATE_OPEN, DM1_DOOR_STATE_PARTLY_ONE, \
        DM1_DOOR_STATE_PARTLY_TWO, DM1_DOOR_STATE_PARTLY_THREE, \
        DM1_DOOR_STATE_CLOSED, DM1_DOOR_STATE_DESTROYED, \
        DM1_OPEN_DEGREE_CLOSED, DM1_OPEN_DEGREE_STEP_ONE, \
        DM1_OPEN_DEGREE_STEP_TWO, DM1_OPEN_DEGREE_STEP_THREE, \
        DM1_OPEN_DEGREE_STEP_FOUR, DM1_OPEN_DEGREE_OPEN, \
        DM1_FIRST_HALF_OFFSET, DM1_SECOND_HALF_OFFSET, DM1_MASK0X4000, \
        DM1_C10_COLOR_FLESH, DM1_PRESENT, DM1_PRESENT, DM1_PRESENT, \
        left_name, right_name, \
        "ReDMCSB: DUNVIEW.C F0111 lines 4218-4339, partly-open flow 4308-4334", \
        "ReDMCSB: DUNVIEW.C F0125/F0126 lines 7960-8162", \
        "ReDMCSB: DUNVIEW.C F0128 lines 8318-8542, D0L/D0R lines 8536-8541", \
        "ReDMCSB: DUNGEON.C F0172 lines 2466-2722", \
        "ReDMCSB: DEFS.H lines 1039-1047,2596-2601,2657-2675,4056-4057", \
        "ReDMCSB: MENU.C 1311-1319 + PROJEXPL.C F0232 lines 1554-1600" \
    }

static const DM1_V1_D0L2D0R2F0111PartlyOpenSpecPc34 s_specs[] = {
    D0_SPEC(DM1_SIDE_D0L2,
            "D0L2 F0111 partly-open corridor-side door band",
            DM1_VIEW_SQUARE_D0L,
            DM1_F0125,
            DM1_D0L_DISPATCH_ORDER,
            DM1_D0L_DISPATCH_LINE,
            DM1_D0L2_LANE,
            DM1_ZONE_WALL_D0L,
            DM1_ZONE_DOOR_D0L2,
            DM1_ORDER_REAR_D0L2,
            DM1_ORDER_FRONT_D0L2,
            "G0185_s_Graphic558_Frames_Door_D0L2.LeftHorizontal[state-1]",
            "G0185_s_Graphic558_Frames_Door_D0L2.RightHorizontal[state-1]"),
    D0_SPEC(DM1_SIDE_D0R2,
            "D0R2 F0111 partly-open corridor-side door band",
            DM1_VIEW_SQUARE_D0R,
            DM1_F0126,
            DM1_D0R_DISPATCH_ORDER,
            DM1_D0R_DISPATCH_LINE,
            DM1_D0R2_LANE,
            DM1_ZONE_WALL_D0R,
            DM1_ZONE_DOOR_D0R2,
            DM1_ORDER_REAR_D0R2,
            DM1_ORDER_FRONT_D0R2,
            "G0187_s_Graphic558_Frames_Door_D0R2.LeftHorizontal[state-1]",
            "G0187_s_Graphic558_Frames_Door_D0R2.RightHorizontal[state-1]")
};

#undef D0_SPEC

static const char s_source_evidence[] =
    "DM1 V1 D0L2/D0R2 F0111 partly-open source-lock gate; "
    "source_locked_contract_only=1; no_real_asset_pixel_parity=1; "
    "no_game_data_load=1. ReDMCSB DUNVIEW.C F0111:4218-4339 anchors "
    "the partly-open door band: 4248 open skip, 4308 state decrement, "
    "4312-4313 LeftHorizontal/RightHorizontal, 4317-4325 C/C5 "
    "transparent zone flow, and 4334 C10_COLOR_FLESH draw. DUNVIEW.C "
    "F0125:7960-8062 and F0126:8064-8162 are the D0L/D0R callers; "
    "F0128:8318-8542 dispatches left before right, with D0L at 8536 "
    "and D0R at 8540, matching the side-pair L/R ordering pattern at "
    "8511-8521. DUNGEON.C F0172:2466-2722 provides C16_ELEMENT_DOOR_SIDE, "
    "M556_DOOR_STATE, M557_DOOR_THING_INDEX, and the fakewall conversion "
    "guard. DEFS.H:1039-1047 pins C0/C4/C5 and M036_DOOR_STATE; "
    "DEFS.H:2596-2601 pins M609/M610/M611 ordinals; DEFS.H:2657-2675 "
    "pins C0x0218/C0x0349 door-pass constants; DEFS.H:4056-4057 pins "
    "D0L/D0R wall zones. Door-bash integration is delegated to "
    "pass797_dm1_v1_door_bash_feedback_source_lock_pc34_compat "
    "(MENU.C:1311-1319 and PROJEXPL.C F0232:1554-1600).";

static DM1_V1_D0L2D0R2F0111SelfTestResultPc34 s_last;

static uint32_t hash_u32(uint32_t hash, uint32_t value)
{
    hash ^= value;
    hash *= 16777619u;
    return hash;
}

static uint32_t hash_string(uint32_t hash, const char *text)
{
    if (!text) return hash_u32(hash, 0xffffffffu);
    while (*text) {
        hash ^= (uint8_t)*text++;
        hash *= 16777619u;
    }
    return hash;
}

static void check_int(const char *id, int got, int want)
{
    ++s_last.assertions;
    s_last.deterministic_hash = hash_string(s_last.deterministic_hash, id);
    s_last.deterministic_hash = hash_u32(s_last.deterministic_hash, (uint32_t)got);
    s_last.deterministic_hash = hash_u32(s_last.deterministic_hash, (uint32_t)want);
    if (got != want) ++s_last.failures;
}

static int open_degree_to_state(int open_degree)
{
    switch (open_degree) {
    case DM1_OPEN_DEGREE_CLOSED:
        return DM1_DOOR_STATE_CLOSED;
    case DM1_OPEN_DEGREE_STEP_ONE:
        return DM1_DOOR_STATE_PARTLY_ONE;
    case DM1_OPEN_DEGREE_STEP_TWO:
        return DM1_DOOR_STATE_PARTLY_TWO;
    case DM1_OPEN_DEGREE_STEP_THREE:
    case DM1_OPEN_DEGREE_STEP_FOUR:
        return DM1_DOOR_STATE_PARTLY_THREE;
    case DM1_OPEN_DEGREE_OPEN:
        return DM1_DOOR_STATE_OPEN;
    default:
        return -1;
    }
}

static int partly_zone_step(int open_degree)
{
    const int state = open_degree_to_state(open_degree);
    if (state == DM1_DOOR_STATE_PARTLY_THREE &&
        open_degree == DM1_OPEN_DEGREE_STEP_FOUR) {
        return 3;
    }
    if (state >= DM1_DOOR_STATE_PARTLY_ONE &&
        state <= DM1_DOOR_STATE_PARTLY_THREE) {
        return state - 1;
    }
    return -1;
}

size_t dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const DM1_V1_D0L2D0R2F0111PartlyOpenSpecPc34 *
dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_at_pc34(size_t index)
{
    if (index >= dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_count_pc34()) {
        return 0;
    }
    return &s_specs[index];
}

const DM1_V1_D0L2D0R2F0111PartlyOpenSpecPc34 *
dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_for_side_pc34(int side)
{
    for (size_t i = 0;
         i < dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_count_pc34();
         ++i) {
        if (s_specs[i].side == side) return &s_specs[i];
    }
    return 0;
}

int dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_branch_pc34(
    const DM1_V1_D0L2D0R2F0111PartlyOpenSpecPc34 *spec,
    int open_degree,
    int neighbor_wall_is_fakewall)
{
    const int state = open_degree_to_state(open_degree);

    if (!spec || state < 0) {
        return DM1_V1_D0L2_D0R2_F0111_BRANCH_INVALID_PC34;
    }
    /*
     * ReDMCSB: DUNGEON.C F0172 lines ~2651-2661 converts a closed
     * fakewall to a wall before the draw path can treat it as a door side.
     */
    if (spec->side == DM1_SIDE_D0L2 && neighbor_wall_is_fakewall) {
        return DM1_V1_D0L2_D0R2_F0111_BRANCH_FAKEWALL_REJECT_PC34;
    }
    if (state == spec->open_state) {
        return DM1_V1_D0L2_D0R2_F0111_BRANCH_OPEN_PC34;
    }
    if (state == spec->closed_state) {
        return DM1_V1_D0L2_D0R2_F0111_BRANCH_CLOSED_PC34;
    }
    if (state == spec->destroyed_state) {
        return DM1_V1_D0L2_D0R2_F0111_BRANCH_DESTROYED_PC34;
    }
    return DM1_V1_D0L2_D0R2_F0111_BRANCH_PARTLY_OPEN_PC34;
}

int dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_row_guard_pc34(
    const DM1_V1_D0L2D0R2F0111PartlyOpenSpecPc34 *spec,
    int thing_kind,
    int behind_partly_open_door_band)
{
    if (!spec || !behind_partly_open_door_band) return 0;
    if (thing_kind == DM1_V1_D0L2_D0R2_F0111_THING_ITEM_PC34) {
        return spec->row_guard_item;
    }
    if (thing_kind == DM1_V1_D0L2_D0R2_F0111_THING_CREATURE_PC34) {
        return spec->row_guard_creature;
    }
    if (thing_kind == DM1_V1_D0L2_D0R2_F0111_THING_PROJECTILE_PC34) {
        return spec->row_guard_projectile;
    }
    return 0;
}

static int resolve_door_bash_chain(int open_degree)
{
    DM1_V1_DoorBashInputPc34 input;
    DM1_V1_DoorBashResultPc34 result;

    memset(&input, 0, sizeof(input));
    input.door_type = DM1_V1_DOOR_INFO_WOODEN_PC34;
    input.door_state = (uint8_t)open_degree_to_state(open_degree);
    input.target_element = DM1_V1_ELEMENT_DOOR_PC34;
    input.action_strength = 50;
    input.is_door_target = true;

    if (!M11_GameView_DoorBashResolvePc34(&input, &result)) return 0;
    if (open_degree == DM1_OPEN_DEGREE_CLOSED) {
        return result.scheduled_destruction_event &&
               result.destruction_delay_ticks == DM1_V1_DOOR_BASH_DESTRUCTION_DELAY_TICKS_PC34 &&
               result.door_state_after == DM1_V1_DOOR_STATE_CLOSED_PC34;
    }
    return result.outcome == DM1_V1_DOOR_BASH_OUTCOME_NOT_CLOSED_PC34 &&
           result.scheduled_destruction_event == false &&
           result.door_state_after == input.door_state;
}

int dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_trace_pc34(
    const DM1_V1_D0L2D0R2F0111PartlyOpenSpecPc34 *spec,
    int open_degree,
    int neighbor_wall_is_fakewall,
    DM1_V1_D0L2D0R2F0111TracePc34 *out_trace)
{
    DM1_V1_D0L2D0R2F0111TracePc34 trace;
    const uint8_t source[8] = { 1, 10, 2, 3, 4, 5, 10, 6 };
    const int branch =
        dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_branch_pc34(
            spec, open_degree, neighbor_wall_is_fakewall);
    const int zone_step = partly_zone_step(open_degree);

    if (!out_trace) return -1;
    memset(&trace, 0, sizeof(trace));
    trace.branch = branch;
    trace.side = spec ? spec->side : 0;
    trace.open_degree = open_degree;
    trace.first_half_zone = -1;
    trace.second_half_zone = -1;
    trace.deterministic_hash =
        hash_u32(dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_hash_pc34(),
                 (uint32_t)(open_degree + 97));

    if (!spec) {
        *out_trace = trace;
        return -1;
    }
    if (branch == DM1_V1_D0L2_D0R2_F0111_BRANCH_FAKEWALL_REJECT_PC34) {
        trace.mutation_rejections = 1;
        *out_trace = trace;
        return 0;
    }
    if (branch == DM1_V1_D0L2_D0R2_F0111_BRANCH_OPEN_PC34) {
        trace.ok = 1;
        trace.door_bash_chain_ok = resolve_door_bash_chain(open_degree);
        *out_trace = trace;
        return 0;
    }
    if (branch == DM1_V1_D0L2_D0R2_F0111_BRANCH_CLOSED_PC34) {
        trace.ok = 1;
        trace.door_band_count = 1;
        trace.first_half_zone = spec->door_zone_base;
        trace.door_bash_chain_ok = resolve_door_bash_chain(open_degree);
        *out_trace = trace;
        return 1;
    }
    if (branch != DM1_V1_D0L2_D0R2_F0111_BRANCH_PARTLY_OPEN_PC34 ||
        zone_step < 0) {
        *out_trace = trace;
        return -1;
    }

    /*
     * ReDMCSB: DUNVIEW.C F0111 lines ~4317-4334 draws two transparent
     * horizontal door halves for a partly-open horizontal door.
     */
    trace.ok = 1;
    trace.door_band_count = 2;
    trace.first_half_zone = spec->door_zone_base + zone_step + spec->first_half_zone_offset;
    trace.second_half_zone =
        spec->door_zone_base + zone_step +
        (spec->second_half_zone_offset | spec->second_half_zone_mask);
    for (int i = 0; i < 8; ++i) {
        if (source[i] == (uint8_t)spec->c10_transparent_color) {
            if (i < 4) ++trace.first_half_c10_skips;
            else ++trace.second_half_c10_skips;
        } else {
            ++trace.copied_pixels;
        }
    }
    trace.row_guard_item =
        dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_row_guard_pc34(
            spec, DM1_V1_D0L2_D0R2_F0111_THING_ITEM_PC34, DM1_PRESENT);
    trace.row_guard_creature =
        dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_row_guard_pc34(
            spec, DM1_V1_D0L2_D0R2_F0111_THING_CREATURE_PC34, DM1_PRESENT);
    trace.row_guard_projectile =
        dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_row_guard_pc34(
            spec, DM1_V1_D0L2_D0R2_F0111_THING_PROJECTILE_PC34, DM1_PRESENT);
    trace.door_bash_chain_ok = resolve_door_bash_chain(open_degree);
    *out_trace = trace;
    return trace.door_band_count;
}

uint32_t dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_hash_pc34(void)
{
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < sizeof(s_specs) / sizeof(s_specs[0]); ++i) {
        hash = hash_u32(hash, (uint32_t)s_specs[i].side);
        hash = hash_u32(hash, (uint32_t)s_specs[i].view_square);
        hash = hash_u32(hash, (uint32_t)s_specs[i].f0128_dispatch_line);
        hash = hash_u32(hash, (uint32_t)s_specs[i].door_zone_base);
        hash = hash_u32(hash, (uint32_t)s_specs[i].rear_cell_order);
        hash = hash_u32(hash, (uint32_t)s_specs[i].front_cell_order);
    }
    hash = hash_u32(hash, DM1_C10_COLOR_FLESH);
    hash = hash_u32(hash, DM1_MASK0X4000);
    hash = hash_u32(hash, DM1_OPEN_DEGREE_OPEN);
    return hash;
}

const char *
dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_source_evidence_pc34(void)
{
    return s_source_evidence;
}

static void verify_ramp_pair(int open_degree)
{
    const DM1_V1_D0L2D0R2F0111PartlyOpenSpecPc34 *left = &s_specs[0];
    const DM1_V1_D0L2D0R2F0111PartlyOpenSpecPc34 *right = &s_specs[1];
    DM1_V1_D0L2D0R2F0111TracePc34 left_trace;
    DM1_V1_D0L2D0R2F0111TracePc34 right_trace;

    check_int("trace.left.rc",
              dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_trace_pc34(
                  left, open_degree, DM1_ABSENT, &left_trace),
              2);
    check_int("trace.right.rc",
              dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_trace_pc34(
                  right, open_degree, DM1_ABSENT, &right_trace),
              2);
    check_int("ramp.left.branch", left_trace.branch,
              DM1_V1_D0L2_D0R2_F0111_BRANCH_PARTLY_OPEN_PC34);
    check_int("ramp.right.branch", right_trace.branch,
              DM1_V1_D0L2_D0R2_F0111_BRANCH_PARTLY_OPEN_PC34);
    check_int("ramp.band.count", left_trace.door_band_count + right_trace.door_band_count, 4);
    check_int("ramp.left.before.right",
              left->f0128_dispatch_order < right->f0128_dispatch_order, 1);
    check_int("ramp.c10.left", left_trace.first_half_c10_skips + left_trace.second_half_c10_skips, 2);
    check_int("ramp.c10.right", right_trace.first_half_c10_skips + right_trace.second_half_c10_skips, 2);
    check_int("ramp.row.item", left_trace.row_guard_item && right_trace.row_guard_item, 1);
    check_int("ramp.row.creature", left_trace.row_guard_creature && right_trace.row_guard_creature, 1);
    check_int("ramp.row.projectile", left_trace.row_guard_projectile && right_trace.row_guard_projectile, 1);
    check_int("ramp.bash.left", left_trace.door_bash_chain_ok, 1);
    check_int("ramp.bash.right", right_trace.door_bash_chain_ok, 1);

    ++s_last.ramp_ticks_checked;
    s_last.total_door_bands += left_trace.door_band_count + right_trace.door_band_count;
    s_last.d0l2_first_dispatch_ok +=
        left->f0128_dispatch_order < right->f0128_dispatch_order;
    s_last.d0r2_second_dispatch_ok +=
        right->f0128_dispatch_order == DM1_D0R_DISPATCH_ORDER;
    s_last.c10_transparency_ok +=
        (left_trace.first_half_c10_skips + left_trace.second_half_c10_skips +
         right_trace.first_half_c10_skips + right_trace.second_half_c10_skips) == 4;
    s_last.row_guards_ok +=
        left_trace.row_guard_item && left_trace.row_guard_creature &&
        left_trace.row_guard_projectile && right_trace.row_guard_item &&
        right_trace.row_guard_creature && right_trace.row_guard_projectile;
    s_last.door_bash_chain_ok +=
        left_trace.door_bash_chain_ok && right_trace.door_bash_chain_ok;
}

int run_dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_door_self_test(void)
{
    DM1_V1_D0L2D0R2F0111TracePc34 trace;
    const int ramp[] = {
        DM1_OPEN_DEGREE_STEP_ONE,
        DM1_OPEN_DEGREE_STEP_TWO,
        DM1_OPEN_DEGREE_STEP_THREE,
        DM1_OPEN_DEGREE_STEP_FOUR
    };

    memset(&s_last, 0, sizeof(s_last));
    s_last.deterministic_hash = dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_hash_pc34();

    check_int("spec.count",
              (int)dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_count_pc34(),
              2);
    check_int("spec.left.side", s_specs[0].side, DM1_SIDE_D0L2);
    check_int("spec.right.side", s_specs[1].side, DM1_SIDE_D0R2);
    check_int("spec.left.order", s_specs[0].f0128_dispatch_order, DM1_D0L_DISPATCH_ORDER);
    check_int("spec.right.order", s_specs[1].f0128_dispatch_order, DM1_D0R_DISPATCH_ORDER);
    check_int("spec.c10", s_specs[0].c10_transparent_color, DM1_C10_COLOR_FLESH);
    check_int("macro.c0218",
              DM1_V1_D0L2_D0R2_F0111_PARTLY_OPEN_C0X0218_PC34, 0x0218);
    check_int("macro.c0349",
              DM1_V1_D0L2_D0R2_F0111_PARTLY_OPEN_C0X0349_PC34, 0x0349);

    for (size_t i = 0; i < sizeof(ramp) / sizeof(ramp[0]); ++i) {
        verify_ramp_pair(ramp[i]);
    }

    check_int("edge.closed.rc",
              dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_trace_pc34(
                  &s_specs[0], DM1_OPEN_DEGREE_CLOSED, DM1_ABSENT, &trace),
              1);
    check_int("edge.closed.branch", trace.branch,
              DM1_V1_D0L2_D0R2_F0111_BRANCH_CLOSED_PC34);
    check_int("edge.closed.bands", trace.door_band_count, 1);
    check_int("edge.closed.bash", trace.door_bash_chain_ok, 1);
    s_last.closed_edge_ok = trace.door_band_count == 1 && trace.door_bash_chain_ok;

    check_int("edge.open.rc",
              dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_trace_pc34(
                  &s_specs[0], DM1_OPEN_DEGREE_OPEN, DM1_ABSENT, &trace),
              0);
    check_int("edge.open.branch", trace.branch,
              DM1_V1_D0L2_D0R2_F0111_BRANCH_OPEN_PC34);
    check_int("edge.open.bands", trace.door_band_count, 0);
    check_int("edge.open.bash", trace.door_bash_chain_ok, 1);
    s_last.open_edge_ok = trace.door_band_count == 0 && trace.door_bash_chain_ok;

    check_int("fakewall.reject.rc",
              dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_trace_pc34(
                  &s_specs[0], DM1_OPEN_DEGREE_STEP_TWO, DM1_PRESENT, &trace),
              0);
    check_int("fakewall.reject.branch", trace.branch,
              DM1_V1_D0L2_D0R2_F0111_BRANCH_FAKEWALL_REJECT_PC34);
    check_int("fakewall.reject.bands", trace.door_band_count, 0);
    check_int("fakewall.reject.mutations", trace.mutation_rejections, 1);
    s_last.fakewall_rejection_ok =
        trace.branch == DM1_V1_D0L2_D0R2_F0111_BRANCH_FAKEWALL_REJECT_PC34 &&
        trace.door_band_count == 0 && trace.mutation_rejections == 1;

    return s_last.failures == 0 ? 0 : 1;
}

const DM1_V1_D0L2D0R2F0111SelfTestResultPc34 *
dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_door_last_self_test_result_pc34(void)
{
    return &s_last;
}
