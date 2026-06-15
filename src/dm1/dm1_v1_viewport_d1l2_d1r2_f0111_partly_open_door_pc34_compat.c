/*
 * DM1 V1 PC 3.4 F0111 partly-open D1L2/D1R2 lateral door gate.
 *
 * Source-lock evidence:
 * - ReDMCSB DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor; line 4248
 *   is the open-door guard, line 4308 decrements C1/C2/C3 partly-open
 *   states before frame selection, lines 4312-4313 select the
 *   horizontal halves (LeftHorizontal and RightHorizontal), lines
 *   4317-4324 do P2084_i_ZoneIndex += decremented state and then
 *   F0635/F0654 through zone + C6_UNKNOWN using C10_COLOR_FLESH, and
 *   lines 4325-4334 add 3 | MASK0x4000 before F0791 with C10.
 * - ReDMCSB DUNVIEW.C:7391-7557 F0122_DUNGEONVIEW_DrawSquareD1L and
 *   7559-7725 F0123_DUNGEONVIEW_DrawSquareD1R; the D1L2/D1R2 lateral
 *   door fronts ride these bodies when the door square is in
 *   L2464_ai_SquareAspect[M556_DOOR_STATE] 1..3.
 * - ReDMCSB DUNVIEW.C:8524-8542 F0128_DUNGEONVIEW_Draw_CPSF dispatches
 *   D1L then D1R, then D1C (D0L, D0R, D0C); D0C line 8549 is the
 *   F0127 object-pass boundary at line 8294.
 * - ReDMCSB DEFS.H:1039-1043 door states, 2088 C10_COLOR_FLESH,
 *   2599-2601 M606/M607/M608 view squares, 2600-2601 M607_D1L /
 *   M608_D1R primary view squares, 2670-2676 DoorPass anchors,
 *   2791 C2_VIEW_DOOR_ORNAMENT_D1LCR, 3508 C6_UNKNOWN, 3516 MASK0x4000,
 *   4053-4054 C713/C714 wall zones, 4258/4260 M630/M632 door zones,
 *   5458 G0695, 5543/5545 G0186/G0188 symbols.
 * - CSB counterpart: csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_pc34_compat
 *   is the CSB V1 version of this same lateral door pair; this DM1 V1
 *   file is the non-overlap DM1 source-lock sibling. The D1C sibling
 *   is test_dm1_v1_viewport_d1c_f0111_partly_open_door_pc34_compat and
 *   the D2L/D2R sibling is
 *   test_dm1_v1_viewport_d2l_d2r_f0111_partly_open_door_pc34_compat.
 *
 * Contract-only: this file simulates the F0111 partly-open composition
 * in a 320x200 framebuffer with a 224x136 viewport and does not load
 * or compare real game assets.
 */
#include "firestaff/dm1/v1/viewport/d1l2_d1r2_f0111_partly_open_door_pc34_compat.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

enum {
    DM1_FRAMEBUFFER_WIDTH = 320,
    DM1_FRAMEBUFFER_HEIGHT = 200,
    DM1_VIEWPORT_WIDTH = 224,
    DM1_VIEWPORT_HEIGHT = 136,
    DM1_VIEW_DEPTH_D1 = 1,
    DM1_VIEW_SQUARE_D1C = 3,
    DM1_VIEW_SQUARE_D1L = 4,
    DM1_VIEW_SQUARE_D1R = 5,
    DM1_D1L_LANE = -1,
    DM1_D1R_LANE = 1,
    DM1_DOOR_STATE_OPEN = 0,
    DM1_DOOR_STATE_PARTLY_ONE = 1,
    DM1_DOOR_STATE_PARTLY_TWO = 2,
    DM1_DOOR_STATE_PARTLY_THREE = 3,
    DM1_DOOR_STATE_CLOSED = 4,
    DM1_DOOR_STATE_DESTROYED = 5,
    DM1_DOOR_STATE_UNKNOWN = 6,
    DM1_C6_UNKNOWN = 6,
    DM1_C10_COLOR_FLESH = 10,
    DM1_MASK0X4000 = 0x4000,
    DM1_SECOND_HALF_OFFSET = 3,
    DM1_D1L_DOOR_ZONE = 3780,
    DM1_D1R_DOOR_ZONE = 3800,
    DM1_D1L_FRAME_TOP_ZONE = 732,
    DM1_D1R_FRAME_TOP_ZONE = 734,
    DM1_D1L_WALL_ZONE = 713,
    DM1_D1R_WALL_ZONE = 714,
    DM1_F0128_D1L_DISPATCH_LINE = 8526,
    DM1_F0128_D1R_DISPATCH_LINE = 8531,
    DM1_F0128_D1C_FOLLOWUP_LINE = 8536,
    DM1_F0128_D0L_FOLLOWUP_LINE = 8541,
    DM1_F0128_D0R_FOLLOWUP_LINE = 8546,
    DM1_F0128_D0C_FOLLOWUP_LINE = 8549,
    DM1_F0127_OBJECT_PASS_LINE = 8294,
    DM1_F0122_LINE_RANGE = 7391,
    DM1_F0122_LINE_END = 7557,
    DM1_F0123_LINE_RANGE = 7559,
    DM1_F0123_LINE_END = 7725,
    DM1_F0128_LINE_RANGE = 8524,
    DM1_F0128_LINE_END = 8542
};

#define DM1_D1L2_SPEC_INDEX 0
#define DM1_D1R2_SPEC_INDEX 1

static const DM1_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 s_specs[] = {
    {
        DM1_V1_D1L2_D1R2_F0111_PARTLY_OPEN_DOOR_SIDE_D1L2_PC34,
        "D1L2 partly-open F0111 lateral corridor-side door front via F0122 D1L body",
        1, 1, 1,
        DM1_VIEW_SQUARE_D1C, DM1_VIEW_SQUARE_D1L, -1,
        122,
        1, 1, DM1_D1L_LANE,
        2, 3, 4, 5,
        DM1_F0127_OBJECT_PASS_LINE,
        DM1_D1L_DOOR_ZONE, -1,
        DM1_D1L_FRAME_TOP_ZONE, -1,
        DM1_D1L_WALL_ZONE, -1,
        DM1_DOOR_STATE_OPEN, DM1_DOOR_STATE_PARTLY_ONE,
        DM1_DOOR_STATE_PARTLY_TWO, DM1_DOOR_STATE_PARTLY_THREE,
        DM1_DOOR_STATE_CLOSED, DM1_DOOR_STATE_DESTROYED,
        1, 1, 1,
        "G0186_s_Graphic558_Frames_Door_D1L.LeftHorizontal[state-1]",
        "G0186_s_Graphic558_Frames_Door_D1L.RightHorizontal[state-1]",
        DM1_C6_UNKNOWN,
        DM1_SECOND_HALF_OFFSET, DM1_MASK0X4000,
        DM1_C10_COLOR_FLESH, DM1_C10_COLOR_FLESH,
        "ReDMCSB DUNVIEW.C F0111 lines 4218-4337, 4311-4334 partly-open horizontal path",
        "ReDMCSB DUNVIEW.C F0122 lines 7391-7557; door-front lines 7492-7508",
        "ReDMCSB DUNVIEW.C F0128 lines 8524-8542 D1L/D1R dispatch and F0127 follow-up",
        "ReDMCSB DUNVIEW.C F0127 line 8294 object-pass boundary",
        "ReDMCSB DEFS.H lines 1039-1043,2088,2599-2601,2605-2606,2670-2676,2791,3508,3516,4053-4054,4258,4260,5458,5543,5545"
    },
    {
        DM1_V1_D1L2_D1R2_F0111_PARTLY_OPEN_DOOR_SIDE_D1R2_PC34,
        "D1R2 partly-open F0111 lateral corridor-side door front via F0123 D1R body",
        1, 1, 1,
        DM1_VIEW_SQUARE_D1C, -1, DM1_VIEW_SQUARE_D1R,
        123,
        2, 1, DM1_D1R_LANE,
        2, 3, 4, 5,
        DM1_F0127_OBJECT_PASS_LINE,
        -1, DM1_D1R_DOOR_ZONE,
        -1, DM1_D1R_FRAME_TOP_ZONE,
        -1, DM1_D1R_WALL_ZONE,
        DM1_DOOR_STATE_OPEN, DM1_DOOR_STATE_PARTLY_ONE,
        DM1_DOOR_STATE_PARTLY_TWO, DM1_DOOR_STATE_PARTLY_THREE,
        DM1_DOOR_STATE_CLOSED, DM1_DOOR_STATE_DESTROYED,
        1, 1, 1,
        "G0188_s_Graphic558_Frames_Door_D1R.LeftHorizontal[state-1]",
        "G0188_s_Graphic558_Frames_Door_D1R.RightHorizontal[state-1]",
        DM1_C6_UNKNOWN,
        DM1_SECOND_HALF_OFFSET, DM1_MASK0X4000,
        DM1_C10_COLOR_FLESH, DM1_C10_COLOR_FLESH,
        "ReDMCSB DUNVIEW.C F0111 lines 4218-4337, 4311-4334 partly-open horizontal path",
        "ReDMCSB DUNVIEW.C F0123 lines 7559-7725; door-front lines 7660-7676",
        "ReDMCSB DUNVIEW.C F0128 lines 8524-8542 D1L/D1R dispatch and F0127 follow-up",
        "ReDMCSB DUNVIEW.C F0127 line 8294 object-pass boundary",
        "ReDMCSB DEFS.H lines 1039-1043,2088,2599-2601,2605-2606,2670-2676,2791,3508,3516,4053-4054,4258,4260,5458,5543,5545"
    }
};

static const char s_source_evidence[] =
    "DM1 V1 D1L2/D1R2 F0111 partly-open source-lock gate; contract-only, "
    "asset-free, no real-asset pixel parity, and no game-data load. "
    "ReDMCSB DUNVIEW.C:4218-4337 F0111 anchors the horizontal half blit: "
    "4248 open guard, 4308 state decrement, 4312-4313 LeftHorizontal/"
    "RightHorizontal, 4317-4324 P2084_i_ZoneIndex plus C6_UNKNOWN F0635/"
    "F0654 C10_COLOR_FLESH first-half blit, and 4325-4334 3|MASK0x4000 "
    "then F0791_DUNGEONVIEW_DrawBitmapXX with C10. DUNVIEW.C:7391-7557 "
    "F0122_DUNGEONVIEW_DrawSquareD1L and 7559-7725 "
    "F0123_DUNGEONVIEW_DrawSquareD1R are the D1L/D1R body callers; the "
    "D1L2/D1R2 lateral door fronts ride these bodies when the door "
    "square is in L2464_ai_SquareAspect[M556_DOOR_STATE] 1..3. "
    "DUNVIEW.C:8524-8542 F0128_DUNGEONVIEW_Draw_CPSF dispatches D1L "
    "(line 8526) then D1R (line 8531); D1C followup is at line 8536, "
    "D0L at 8541, D0R at 8546, and D0C at 8549; F0127 object-pass "
    "boundary is at 8294. DEFS.H:1039-1043 door states, 2088 "
    "C10_COLOR_FLESH, 2599-2601 M606/M607/M608 view squares, 2600-2601 "
    "M607_D1L/M608_D1R primary view squares, 2670-2676 DoorPass "
    "anchors, 2791 C2_VIEW_DOOR_ORNAMENT_D1LCR, 3508 C6_UNKNOWN, 3516 "
    "MASK0x4000, 4053-4054 C713/C714 wall zones, 4258/4260 M630/M632 "
    "door zones, 5458 G0695, 5543/5545 G0186/G0188 symbols. CSB "
    "counterpart: csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_pc34_compat. "
    "Non-overlap DM1 siblings: D1C "
    "test_dm1_v1_viewport_d1c_f0111_partly_open_door_pc34_compat and "
    "D2L/D2R test_dm1_v1_viewport_d2l_d2r_f0111_partly_open_door_pc34_compat.";

static DM1_V1_D1L2D1R2F0111PartlyOpenDoorSelfTestResultPc34 s_last;

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

static int spec_base_zone(const DM1_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *spec)
{
    if (spec->side == DM1_V1_D1L2_D1R2_F0111_PARTLY_OPEN_DOOR_SIDE_D1L2_PC34) {
        return spec->door_zone_m630_d1l;
    }
    return spec->door_zone_m632_d1r;
}

static int simulate_f0111_partly_open(
    const DM1_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    DM1_V1_D1L2D1R2F0111DoorBranchPc34 *out_branch,
    int *out_decremented_state,
    int *out_first_half_zone,
    int *out_second_half_zone,
    int *out_first_half_c10_skips,
    int *out_second_half_c10_skips,
    int *out_first_half_writes,
    int *out_second_half_writes,
    uint8_t *out_probe_a,
    uint8_t *out_probe_b)
{
    uint8_t framebuffer[DM1_FRAMEBUFFER_WIDTH * DM1_FRAMEBUFFER_HEIGHT];
    const uint8_t first_source[6] = { 10, 0x21, 0x22, 10, 0x23, 0x24 };
    const uint8_t second_source[6] = { 0x31, 10, 0x32, 0x33, 10, 0x34 };
    const int base_x = spec && spec->side == DM1_V1_D1L2_D1R2_F0111_PARTLY_OPEN_DOOR_SIDE_D1R2_PC34 ? 192 : 16;
    const int base_y = 44 + door_state;
    int i;
    int decremented;
    int first_zone;
    int second_zone;
    int base_zone;

    if (!spec) {
        if (out_branch) *out_branch = DM1_V1_D1L2_D1R2_F0111_DOOR_BRANCH_INVALID_PC34;
        return 0;
    }
    memset(framebuffer, 0x11, sizeof(framebuffer));

    if (door_state == DM1_DOOR_STATE_OPEN) {
        if (out_branch) *out_branch = DM1_V1_D1L2_D1R2_F0111_DOOR_BRANCH_OPEN_PC34;
        return 1;
    }
    if (door_state == DM1_DOOR_STATE_CLOSED) {
        if (out_branch) *out_branch = DM1_V1_D1L2_D1R2_F0111_DOOR_BRANCH_CLOSED_PC34;
        return 1;
    }
    if (door_state == DM1_DOOR_STATE_DESTROYED) {
        if (out_branch) *out_branch = DM1_V1_D1L2_D1R2_F0111_DOOR_BRANCH_DESTROYED_PC34;
        return 1;
    }
    if (door_state == DM1_DOOR_STATE_UNKNOWN ||
        (door_state != DM1_DOOR_STATE_PARTLY_ONE &&
         door_state != DM1_DOOR_STATE_PARTLY_TWO &&
         door_state != DM1_DOOR_STATE_PARTLY_THREE)) {
        if (out_branch) *out_branch = DM1_V1_D1L2_D1R2_F0111_DOOR_BRANCH_INVALID_PC34;
        return 1;
    }

    base_zone = spec_base_zone(spec);
    decremented = door_state - 1;
    first_zone = base_zone + decremented;
    second_zone = first_zone + (DM1_SECOND_HALF_OFFSET | DM1_MASK0X4000);

    if (out_branch) *out_branch = DM1_V1_D1L2_D1R2_F0111_DOOR_BRANCH_PARTLY_OPEN_PC34;
    if (out_decremented_state) *out_decremented_state = decremented;
    if (out_first_half_zone) *out_first_half_zone = first_zone;
    if (out_second_half_zone) *out_second_half_zone = second_zone;
    if (out_first_half_c10_skips) *out_first_half_c10_skips = 0;
    if (out_second_half_c10_skips) *out_second_half_c10_skips = 0;
    if (out_first_half_writes) *out_first_half_writes = 0;
    if (out_second_half_writes) *out_second_half_writes = 0;

    for (i = 0; i < 6; ++i) {
        const int x = base_x + i;
        const int offset = (base_y * DM1_FRAMEBUFFER_WIDTH) + x;
        if (first_source[i] == DM1_C10_COLOR_FLESH) {
            if (out_first_half_c10_skips) ++*out_first_half_c10_skips;
        } else {
            framebuffer[offset] = first_source[i];
            if (out_first_half_writes) ++*out_first_half_writes;
        }
    }
    for (i = 0; i < 6; ++i) {
        const int x = base_x + 12 + i;
        const int offset = ((base_y + 1) * DM1_FRAMEBUFFER_WIDTH) + x;
        if (second_source[i] == DM1_C10_COLOR_FLESH) {
            if (out_second_half_c10_skips) ++*out_second_half_c10_skips;
        } else {
            framebuffer[offset] = second_source[i];
            if (out_second_half_writes) ++*out_second_half_writes;
        }
    }

    if (out_probe_a) {
        *out_probe_a = framebuffer[(base_y * DM1_FRAMEBUFFER_WIDTH) + base_x + 1];
    }
    if (out_probe_b) {
        *out_probe_b =
            framebuffer[((base_y + 1) * DM1_FRAMEBUFFER_WIDTH) + base_x + 12];
    }
    return 1;
}

static void verify_partly_case(const DM1_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *spec, int state)
{
    DM1_V1_D1L2D1R2F0111DoorBranchPc34 branch =
        DM1_V1_D1L2_D1R2_F0111_DOOR_BRANCH_INVALID_PC34;
    int decremented = -1;
    int first_zone = -1;
    int second_zone = -1;
    int first_skips = -1;
    int second_skips = -1;
    int first_writes = -1;
    int second_writes = -1;
    uint8_t probe_a = 0;
    uint8_t probe_b = 0;
    const int base_zone = spec_base_zone(spec);
    const int ok = simulate_f0111_partly_open(
        spec, state, &branch, &decremented, &first_zone, &second_zone,
        &first_skips, &second_skips, &first_writes, &second_writes,
        &probe_a, &probe_b);

    check_int("simulate.partly.ok", ok, 1);
    check_int("partly.branch", (int)branch,
              (int)DM1_V1_D1L2_D1R2_F0111_DOOR_BRANCH_PARTLY_OPEN_PC34);
    check_int("decremented.state", decremented, state - 1);
    check_int("first.half.zone", first_zone, base_zone + state - 1);
    check_int("second.half.zone", second_zone,
              base_zone + state - 1 + (DM1_SECOND_HALF_OFFSET | DM1_MASK0X4000));
    check_int("first.c10.skips", first_skips, 2);
    check_int("second.c10.skips", second_skips, 2);
    check_int("first.writes", first_writes, 4);
    check_int("second.writes", second_writes, 4);
    check_int("probe.a", (int)probe_a, 0x21);
    check_int("probe.b", (int)probe_b, 0x31);

    if (spec->side == DM1_V1_D1L2_D1R2_F0111_PARTLY_OPEN_DOOR_SIDE_D1L2_PC34) {
        if (state == DM1_DOOR_STATE_PARTLY_ONE) ++s_last.d1l2_partly_one;
        if (state == DM1_DOOR_STATE_PARTLY_TWO) ++s_last.d1l2_partly_two;
        if (state == DM1_DOOR_STATE_PARTLY_THREE) ++s_last.d1l2_partly_three;
        ++s_last.d1l2_partly;
    } else {
        if (state == DM1_DOOR_STATE_PARTLY_ONE) ++s_last.d1r2_partly_one;
        if (state == DM1_DOOR_STATE_PARTLY_TWO) ++s_last.d1r2_partly_two;
        if (state == DM1_DOOR_STATE_PARTLY_THREE) ++s_last.d1r2_partly_three;
        ++s_last.d1r2_partly;
    }
    s_last.c10_first_half_skips += first_skips;
    s_last.c10_second_half_skips += second_skips;
    s_last.f0128_followup_anchors += 1;
}

static void verify_rejection(const DM1_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *spec, int state,
                             int expected_branch)
{
    DM1_V1_D1L2D1R2F0111DoorBranchPc34 branch =
        DM1_V1_D1L2_D1R2_F0111_DOOR_BRANCH_INVALID_PC34;
    int decremented = -1;
    int first_zone = -1;
    int second_zone = -1;
    int first_skips = -1;
    int second_skips = -1;
    int first_writes = -1;
    int second_writes = -1;
    uint8_t probe_a = 0;
    uint8_t probe_b = 0;
    const int ok = simulate_f0111_partly_open(
        spec, state, &branch, &decremented, &first_zone, &second_zone,
        &first_skips, &second_skips, &first_writes, &second_writes,
        &probe_a, &probe_b);

    check_int("simulate.reject.ok", ok, 1);
    check_int("reject.branch", (int)branch, expected_branch);
    check_int("reject.uninitialized.first_zone", first_zone, -1);
    check_int("reject.uninitialized.second_zone", second_zone, -1);
    if (state == DM1_DOOR_STATE_OPEN) ++s_last.open_rejections;
    else if (state == DM1_DOOR_STATE_CLOSED) ++s_last.closed_rejections;
    else ++s_last.unknown_rejections;
}

static void verify_static_contract(void)
{
    const DM1_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *d1l2 = &s_specs[DM1_D1L2_SPEC_INDEX];
    const DM1_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *d1r2 = &s_specs[DM1_D1R2_SPEC_INDEX];

    check_contains("evidence.f0111", s_source_evidence, "DUNVIEW.C:4218-4337");
    check_contains("evidence.horizontal.second.half", s_source_evidence, "4325-4334");
    check_contains("evidence.f0128.dispatch", s_source_evidence, "8524-8542");
    check_contains("evidence.f0122", s_source_evidence, "F0122_DUNGEONVIEW_DrawSquareD1L");
    check_contains("evidence.f0123", s_source_evidence, "F0123_DUNGEONVIEW_DrawSquareD1R");
    check_contains("evidence.defs.states", s_source_evidence, "DEFS.H:1039-1043");
    check_contains("evidence.c10", s_source_evidence, "C10_COLOR_FLESH");
    check_contains("evidence.mask", s_source_evidence, "MASK0x4000");
    check_contains("evidence.d1c.sibling", s_source_evidence,
                   "test_dm1_v1_viewport_d1c_f0111_partly_open_door_pc34_compat");
    check_contains("evidence.d2lr.sibling", s_source_evidence,
                   "test_dm1_v1_viewport_d2l_d2r_f0111_partly_open_door_pc34_compat");
    check_contains("evidence.csb.counterpart", s_source_evidence,
                   "csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_pc34_compat");

    check_int("spec.count", (int)(sizeof(s_specs) / sizeof(s_specs[0])), 2);
    check_int("d1l2.present", d1l2 != NULL, 1);
    check_int("d1r2.present", d1r2 != NULL, 1);
    if (!d1l2 || !d1r2) return;
    check_int("d1l2.side", d1l2->side,
              DM1_V1_D1L2_D1R2_F0111_PARTLY_OPEN_DOOR_SIDE_D1L2_PC34);
    check_int("d1r2.side", d1r2->side,
              DM1_V1_D1L2_D1R2_F0111_PARTLY_OPEN_DOOR_SIDE_D1R2_PC34);
    check_int("d1l2.contract_only", d1l2->source_locked_contract_only, 1);
    check_int("d1r2.no_game_data", d1r2->no_game_data_load, 1);
    check_int("d1l2.no_pixel", d1l2->no_real_asset_pixel_parity, 1);
    check_int("d1l2.view_square_d1c", d1l2->view_square_m606, DM1_VIEW_SQUARE_D1C);
    check_int("d1r2.view_square_d1c", d1r2->view_square_m606, DM1_VIEW_SQUARE_D1C);
    check_int("d1l2.view_square_d1l", d1l2->view_square_m607, DM1_VIEW_SQUARE_D1L);
    check_int("d1r2.view_square_d1r", d1r2->view_square_m608, DM1_VIEW_SQUARE_D1R);
    check_int("d1l2.view_square_d1r.excluded", d1l2->view_square_m608, -1);
    check_int("d1r2.view_square_d1l.excluded", d1r2->view_square_m607, -1);
    check_int("d1l2.function", d1l2->f0122_f0123_function_number, 122);
    check_int("d1r2.function", d1r2->f0122_f0123_function_number, 123);
    check_int("d1l2.dispatch.order", d1l2->f0128_dispatch_order, 1);
    check_int("d1r2.dispatch.order", d1r2->f0128_dispatch_order, 2);
    check_int("d1l2.dispatch.before.d1r2",
              d1l2->f0128_dispatch_order < d1r2->f0128_dispatch_order, 1);
    check_int("d1l2.depth", d1l2->f0128_relative_depth, 1);
    check_int("d1r2.depth", d1r2->f0128_relative_depth, 1);
    check_int("d1l2.lane", d1l2->f0128_relative_lateral, DM1_D1L_LANE);
    check_int("d1r2.lane", d1r2->f0128_relative_lateral, DM1_D1R_LANE);
    check_int("d1l2.d1c.followup", d1l2->f0128_d1c_followup_order, 2);
    check_int("d1l2.d0l.followup", d1l2->f0128_d0l_followup_order, 3);
    check_int("d1l2.d0r.followup", d1l2->f0128_d0r_followup_order, 4);
    check_int("d1l2.f0127.followup", d1l2->f0127_followup_order, 5);
    check_int("d1l2.f0127.pass.line", d1l2->f0127_object_pass_line, 8294);
    check_int("d1l2.zone", d1l2->door_zone_m630_d1l, DM1_D1L_DOOR_ZONE);
    check_int("d1r2.zone", d1r2->door_zone_m632_d1r, DM1_D1R_DOOR_ZONE);
    check_int("d1l2.frame.top", d1l2->door_frame_top_zone_d1l, DM1_D1L_FRAME_TOP_ZONE);
    check_int("d1r2.frame.top", d1r2->door_frame_top_zone_d1r, DM1_D1R_FRAME_TOP_ZONE);
    check_int("d1l2.wall.zone", d1l2->wall_zone_c713_d1l, DM1_D1L_WALL_ZONE);
    check_int("d1r2.wall.zone", d1r2->wall_zone_c714_d1r, DM1_D1R_WALL_ZONE);
    check_int("d1l2.open.state", d1l2->open_state, DM1_DOOR_STATE_OPEN);
    check_int("d1l2.partly.one", d1l2->partly_open_state_one, DM1_DOOR_STATE_PARTLY_ONE);
    check_int("d1l2.partly.two", d1l2->partly_open_state_two, DM1_DOOR_STATE_PARTLY_TWO);
    check_int("d1l2.partly.three", d1l2->partly_open_state_three, DM1_DOOR_STATE_PARTLY_THREE);
    check_int("d1l2.closed.state", d1l2->closed_state, DM1_DOOR_STATE_CLOSED);
    check_int("d1l2.destroyed.state", d1l2->destroyed_state, DM1_DOOR_STATE_DESTROYED);
    check_int("d1l2.decrements.state", d1l2->decrements_state_before_frame_select, 1);
    check_int("d1l2.selects.left", d1l2->horizontal_door_selects_left_horizontal, 1);
    check_int("d1l2.selects.right", d1l2->horizontal_door_selects_right_horizontal, 1);
    check_int("d1l2.first.half.offset", d1l2->first_half_dest_zone_offset, DM1_C6_UNKNOWN);
    check_int("d1l2.second.half.offset", d1l2->second_half_zone_offset, DM1_SECOND_HALF_OFFSET);
    check_int("d1l2.second.half.mask", d1l2->second_half_zone_mask, DM1_MASK0X4000);
    check_int("d1l2.first.transparent", d1l2->first_half_transparent_color, DM1_C10_COLOR_FLESH);
    check_int("d1l2.second.transparent", d1l2->second_half_transparent_color, DM1_C10_COLOR_FLESH);
    check_contains("d1l2.left.frame", d1l2->left_horizontal_frame_bitmap, "G0186");
    check_contains("d1l2.right.frame", d1l2->right_horizontal_frame_bitmap, "G0186");
    check_contains("d1r2.left.frame", d1r2->left_horizontal_frame_bitmap, "G0188");
    check_contains("d1r2.right.frame", d1r2->right_horizontal_frame_bitmap, "G0188");
    check_contains("d1l2.f0111.anchor", d1l2->f0111_anchor, "4218-4337");
    check_contains("d1l2.d1.body.anchor", d1l2->d1_body_anchor, "7391-7557");
    check_contains("d1r2.d1.body.anchor", d1r2->d1_body_anchor, "7559-7725");
    check_contains("d1l2.f0128.anchor", d1l2->f0128_anchor, "8524-8542");
    check_contains("d1l2.f0127.anchor", d1l2->f0127_anchor, "8294");
    check_contains("d1l2.defs.anchor", d1l2->defs_anchor, "DEFS.H");
    check_int("c10.macro",
              DM1_V1_D1L2_D1R2_F0111_PARTLY_OPEN_C10_COLOR_FLESH_PC34,
              DM1_C10_COLOR_FLESH);
    check_int("mask.macro",
              DM1_V1_D1L2_D1R2_F0111_PARTLY_OPEN_MASK0X4000_PC34,
              DM1_MASK0X4000);
    check_int("c6.macro",
              DM1_V1_D1L2_D1R2_F0111_PARTLY_OPEN_C6_UNKNOWN_PC34,
              DM1_C6_UNKNOWN);
}

size_t dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const DM1_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *
dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_at_pc34(size_t index)
{
    if (index >= dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_count_pc34()) {
        return NULL;
    }
    return &s_specs[index];
}

const DM1_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *
dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_for_side_pc34(int side)
{
    for (size_t i = 0;
         i < dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_count_pc34();
         ++i) {
        if (s_specs[i].side == side) return &s_specs[i];
    }
    return NULL;
}

const DM1_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *
dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_for_square_pc34(
    int view_square)
{
    for (size_t i = 0;
         i < dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_count_pc34();
         ++i) {
        if (s_specs[i].view_square_m607 == view_square) return &s_specs[i];
        if (s_specs[i].view_square_m608 == view_square) return &s_specs[i];
    }
    return NULL;
}

int dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_branch_pc34(
    const DM1_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state)
{
    if (!spec) return DM1_V1_D1L2_D1R2_F0111_DOOR_BRANCH_INVALID_PC34;
    if (door_state == spec->open_state) {
        return DM1_V1_D1L2_D1R2_F0111_DOOR_BRANCH_OPEN_PC34;
    }
    if (door_state == spec->closed_state) {
        return DM1_V1_D1L2_D1R2_F0111_DOOR_BRANCH_CLOSED_PC34;
    }
    if (door_state == spec->destroyed_state) {
        return DM1_V1_D1L2_D1R2_F0111_DOOR_BRANCH_DESTROYED_PC34;
    }
    if (door_state >= spec->partly_open_state_one &&
        door_state <= spec->partly_open_state_three) {
        return DM1_V1_D1L2_D1R2_F0111_DOOR_BRANCH_PARTLY_OPEN_PC34;
    }
    return DM1_V1_D1L2_D1R2_F0111_DOOR_BRANCH_INVALID_PC34;
}

const char *
dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_frame_bitmap_pc34(
    const DM1_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    int right_half)
{
    if (dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_branch_pc34(
            spec, door_state) !=
        DM1_V1_D1L2_D1R2_F0111_DOOR_BRANCH_PARTLY_OPEN_PC34) {
        return NULL;
    }
    return right_half ? spec->right_horizontal_frame_bitmap :
                        spec->left_horizontal_frame_bitmap;
}

int dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_first_half_zone_pc34(
    const DM1_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    int horizontal_door)
{
    if (!horizontal_door) return -1;
    if (dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_branch_pc34(
            spec, door_state) !=
        DM1_V1_D1L2_D1R2_F0111_DOOR_BRANCH_PARTLY_OPEN_PC34) {
        return -1;
    }
    return spec_base_zone(spec) + door_state + spec->first_half_dest_zone_offset;
}

int dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_second_half_zone_pc34(
    const DM1_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    int horizontal_door)
{
    const int base = spec_base_zone(spec);

    if (dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_branch_pc34(
            spec, door_state) !=
        DM1_V1_D1L2_D1R2_F0111_DOOR_BRANCH_PARTLY_OPEN_PC34) {
        return -1;
    }
    if (!horizontal_door) return base + door_state;
    return base + door_state +
           (spec->second_half_zone_offset | spec->second_half_zone_mask);
}

int dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_synthetic_blit_pc34(
    const DM1_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    const uint8_t *source,
    int source_width,
    int source_height,
    int source_stride,
    uint8_t *destination,
    int destination_width,
    int destination_height,
    int destination_stride,
    int *out_c10_skipped)
{
    int copied = 0;
    int skipped = 0;

    if (!spec || !source || !destination) return -1;
    if (source_width <= 0 || source_height <= 0) return -1;
    if (source_stride < source_width || destination_stride < destination_width) {
        return -1;
    }
    if (destination_width < source_width || destination_height < source_height) {
        return -1;
    }
    if (dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_branch_pc34(
            spec, door_state) !=
        DM1_V1_D1L2_D1R2_F0111_DOOR_BRANCH_PARTLY_OPEN_PC34) {
        return 0;
    }

    for (int y = 0; y < source_height; ++y) {
        for (int x = 0; x < source_width; ++x) {
            const uint8_t pixel = source[(y * source_stride) + x];
            if (pixel == (uint8_t)spec->second_half_transparent_color) {
                ++skipped;
                continue;
            }
            destination[(y * destination_stride) + x] = pixel;
            ++copied;
        }
    }

    if (out_c10_skipped) *out_c10_skipped = skipped;
    return copied;
}

int dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_trace_pc34(
    int side,
    int door_state,
    DM1_V1_D1L2D1R2F0111DoorBranchPc34 *out_branch,
    int *out_decremented_state,
    int *out_first_half_zone,
    int *out_second_half_zone,
    int *out_first_half_c10_skips,
    int *out_second_half_c10_skips,
    int *out_first_half_writes,
    int *out_second_half_writes,
    uint8_t *out_probe_a,
    uint8_t *out_probe_b)
{
    const DM1_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *spec =
        dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_for_side_pc34(side);

    return simulate_f0111_partly_open(
        spec, door_state, out_branch, out_decremented_state,
        out_first_half_zone, out_second_half_zone, out_first_half_c10_skips,
        out_second_half_c10_skips, out_first_half_writes, out_second_half_writes,
        out_probe_a, out_probe_b);
}

int run_dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_self_test(void)
{
    size_t side_index;

    memset(&s_last, 0, sizeof(s_last));
    s_last.deterministic_hash = 2166136261u;
    verify_static_contract();

    for (side_index = 0; side_index < sizeof(s_specs) / sizeof(s_specs[0]); ++side_index) {
        const DM1_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *spec = &s_specs[side_index];

        verify_partly_case(spec, DM1_DOOR_STATE_PARTLY_ONE);
        verify_partly_case(spec, DM1_DOOR_STATE_PARTLY_TWO);
        verify_partly_case(spec, DM1_DOOR_STATE_PARTLY_THREE);
        verify_rejection(spec, DM1_DOOR_STATE_OPEN,
                         DM1_V1_D1L2_D1R2_F0111_DOOR_BRANCH_OPEN_PC34);
        verify_rejection(spec, DM1_DOOR_STATE_CLOSED,
                         DM1_V1_D1L2_D1R2_F0111_DOOR_BRANCH_CLOSED_PC34);
        verify_rejection(spec, DM1_DOOR_STATE_DESTROYED,
                         DM1_V1_D1L2_D1R2_F0111_DOOR_BRANCH_DESTROYED_PC34);
        verify_rejection(spec, DM1_DOOR_STATE_UNKNOWN,
                         DM1_V1_D1L2_D1R2_F0111_DOOR_BRANCH_INVALID_PC34);
    }

    check_int("d1l2.partly.one.count", s_last.d1l2_partly_one, 1);
    check_int("d1l2.partly.two.count", s_last.d1l2_partly_two, 1);
    check_int("d1l2.partly.three.count", s_last.d1l2_partly_three, 1);
    check_int("d1r2.partly.one.count", s_last.d1r2_partly_one, 1);
    check_int("d1r2.partly.two.count", s_last.d1r2_partly_two, 1);
    check_int("d1r2.partly.three.count", s_last.d1r2_partly_three, 1);
    check_int("d1l2.partly.count", s_last.d1l2_partly, 3);
    check_int("d1r2.partly.count", s_last.d1r2_partly, 3);
    check_int("closed.rejections.count", s_last.closed_rejections, 2);
    check_int("open.rejections.count", s_last.open_rejections, 2);
    check_int("unknown.rejections.count", s_last.unknown_rejections, 4);
    check_int("c10.first.skips.total", s_last.c10_first_half_skips, 12);
    check_int("c10.second.skips.total", s_last.c10_second_half_skips, 12);
    check_int("f0128.followup.anchors", s_last.f0128_followup_anchors, 6);
    check_int("hash.nonzero", s_last.deterministic_hash != 0u, 1);
    return s_last.failures == 0 ? 0 : 1;
}

const DM1_V1_D1L2D1R2F0111PartlyOpenDoorSelfTestResultPc34 *
dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_last_self_test_result_pc34(void)
{
    return &s_last;
}

const char *
dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_source_evidence_pc34(void)
{
    return s_source_evidence;
}
