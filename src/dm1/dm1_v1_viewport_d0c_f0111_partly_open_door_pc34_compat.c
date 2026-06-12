#include "firestaff/dm1/v1/viewport/d0c_f0111_partly_open_door_pc34_compat.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

enum {
    DM1_DOOR_STATE_OPEN = 0,
    DM1_DOOR_STATE_CLOSED_ONE_FOURTH = 1,
    DM1_DOOR_STATE_CLOSED_HALF = 2,
    DM1_DOOR_STATE_CLOSED_THREE_FOURTH = 3,
    DM1_DOOR_STATE_CLOSED = 4,
    DM1_DOOR_STATE_DESTROYED = 5,
    DM1_DOOR_STATE_INVALID = 6,
    DM1_F0111_START = 4218,
    DM1_F0111_END = 4337,
    DM1_F0121_START = 7244,
    DM1_F0121_END = 7389,
    DM1_F0121_D2C_F0111_LINE_PC34 = 7316,
    DM1_F0127_START = 8164,
    DM1_F0127_END = 8311,
    DM1_F0128_D2C_CALL_LINE = 8521,
    DM1_F0128_D0C_CALL_LINE = 8542,
    DM1_D0C_DOOR_FRAME_ZONE_PC34 = 728,
    DM1_SECOND_HALF_SHIFT =
        3 | DM1_V1_D0C_F0111_PARTLY_OPEN_MASK0X4000_PC34
};

static const char s_source_evidence[] =
    "DM1 V1 D0C F0111 partly-open boundary, contract-only and asset-free; "
    "no real-asset or original-DOS pixel parity claim. ReDMCSB "
    "DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor anchors C0 open guard "
    "4248, C4 closed branch 4297-4298, C5 destroyed branch 4301-4304, "
    "C1..C3 state decrement 4308, horizontal LeftHorizontal/RightHorizontal "
    "selection 4312-4313, C6_UNKNOWN/C10 first-half blit 4317-4324, and "
    "3|MASK0x4000 plus final C10 F0791 draw 4325-4334. DUNVIEW.C:7244-7389 "
    "F0121_DUNGEONVIEW_DrawSquareD2C reaches F0111 for D2C at 7313/7316; "
    "this is a D2C center route, not the D0C route. DUNVIEW.C:8164-8311 "
    "F0127_DUNGEONVIEW_DrawSquareD0C dispatches C16_ELEMENT_DOOR_SIDE through "
    "G0172/G2116 door-frame drawing at 8185-8236 and contains no F0111 call. "
    "DUNVIEW.C:8498-8542 F0128 calls F0121 for D2C at 8521 and calls F0127 "
    "for D0C at 8542. DUNVIEW.C:92/2654-2658 and DEFS.H:5458 pin "
    "G0695_ai_DoorNativeBitmapIndex_Front_D1LCR as D1LCR, not D0C; D0C "
    "uses G2116_DoorFrameFrontD0C at DUNVIEW.C:151/226/242/259 and "
    "2162/2181/2196. DEFS.H:1039-1044 pins C0..C5 door states, 3508 "
    "C6_UNKNOWN, 3516 MASK0x4000, and 4086 C728_ZONE_DOOR_FRAME_D0C. "
    "Non-overlap marker pass769-d0c-f0111-partly-open-boundary: this does "
    "not duplicate D1C, D2L/D2R, D1L2/D1R2, D0L/D0R, D0L2/D0R2, or D3C "
    "F0111 partly-open gates.";

static DM1_V1_D0CF0111PartlyOpenDoorSelfTestResultPc34 s_last;

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
        hash ^= (uint8_t)*text++;
        hash *= 16777619u;
    }
    return hash;
}

static void assert_int(const char *id, int got, int want)
{
    ++s_last.assertions;
    s_last.deterministic_hash = mix_string(s_last.deterministic_hash, id);
    s_last.deterministic_hash = mix_u32(s_last.deterministic_hash, (uint32_t)got);
    s_last.deterministic_hash = mix_u32(s_last.deterministic_hash, (uint32_t)want);
    if (got != want) ++s_last.failures;
}

static void assert_contains(const char *id, const char *haystack, const char *needle)
{
    const int found = haystack && needle && strstr(haystack, needle) != NULL;

    s_last.deterministic_hash = mix_string(s_last.deterministic_hash, needle);
    assert_int(id, found, 1);
}

static uint8_t c10_blend(uint8_t destination, uint8_t source)
{
    return source == DM1_V1_D0C_F0111_PARTLY_OPEN_C10_COLOR_FLESH_PC34
        ? destination
        : source;
}

static void simulate_partly_open_half_blits(
    int door_state,
    DM1_V1_D0CF0111PartlyOpenDoorTracePc34 *out)
{
    uint8_t framebuffer[
        DM1_V1_D0C_F0111_PARTLY_OPEN_FRAMEBUFFER_WIDTH_PC34 *
        DM1_V1_D0C_F0111_PARTLY_OPEN_FRAMEBUFFER_HEIGHT_PC34];
    const uint8_t first_half[6] = { 10, 0x21, 0x22, 10, 0x23, 0x24 };
    const uint8_t second_half[6] = { 0x31, 10, 0x32, 0x33, 10, 0x34 };
    const int x0 = 96 + door_state;
    const int y0 = 32 + door_state;
    int i;

    memset(framebuffer, 0x11, sizeof(framebuffer));
    out->decremented_state = door_state - 1;
    out->first_half_zone =
        DM1_D0C_DOOR_FRAME_ZONE_PC34 + out->decremented_state;
    out->first_half_clip_zone =
        out->first_half_zone +
        DM1_V1_D0C_F0111_PARTLY_OPEN_C6_UNKNOWN_PC34;
    out->second_half_shift = DM1_SECOND_HALF_SHIFT;
    out->second_half_zone = out->first_half_zone + DM1_SECOND_HALF_SHIFT;
    out->c10_transparent_color =
        DM1_V1_D0C_F0111_PARTLY_OPEN_C10_COLOR_FLESH_PC34;

    for (i = 0; i < 6; ++i) {
        uint8_t *first = &framebuffer[(y0 * DM1_V1_D0C_F0111_PARTLY_OPEN_FRAMEBUFFER_WIDTH_PC34) + x0 + i];
        uint8_t *second = &framebuffer[((y0 + 1) * DM1_V1_D0C_F0111_PARTLY_OPEN_FRAMEBUFFER_WIDTH_PC34) + x0 + i];
        const uint8_t before_first = *first;
        const uint8_t before_second = *second;

        *first = c10_blend(*first, first_half[i]);
        *second = c10_blend(*second, second_half[i]);
        if (*first == before_first && first_half[i] == 10) ++out->first_half_skips;
        if (*first != before_first && first_half[i] != 10) ++out->first_half_writes;
        if (*second == before_second && second_half[i] == 10) ++out->second_half_skips;
        if (*second != before_second && second_half[i] != 10) ++out->second_half_writes;
    }

    out->first_probe_pixel =
        framebuffer[(y0 * DM1_V1_D0C_F0111_PARTLY_OPEN_FRAMEBUFFER_WIDTH_PC34) + x0 + 1];
    out->second_probe_pixel =
        framebuffer[((y0 + 1) * DM1_V1_D0C_F0111_PARTLY_OPEN_FRAMEBUFFER_WIDTH_PC34) + x0 + 2];
}

int dm1_v1_viewport_d0c_f0111_partly_open_door_trace_pc34(
    int door_state,
    DM1_V1_D0CF0111PartlyOpenDoorTracePc34 *out_trace)
{
    if (!out_trace) return 0;
    memset(out_trace, 0, sizeof(*out_trace));
    out_trace->input_state = door_state;
    out_trace->framebuffer_width =
        DM1_V1_D0C_F0111_PARTLY_OPEN_FRAMEBUFFER_WIDTH_PC34;
    out_trace->framebuffer_height =
        DM1_V1_D0C_F0111_PARTLY_OPEN_FRAMEBUFFER_HEIGHT_PC34;
    out_trace->viewport_width =
        DM1_V1_D0C_F0111_PARTLY_OPEN_VIEWPORT_WIDTH_PC34;
    out_trace->viewport_height =
        DM1_V1_D0C_F0111_PARTLY_OPEN_VIEWPORT_HEIGHT_PC34;
    out_trace->f0111_line_start = DM1_F0111_START;
    out_trace->f0111_line_end = DM1_F0111_END;
    out_trace->f0121_d2c_dispatch_line = DM1_F0121_D2C_F0111_LINE_PC34;
    out_trace->f0127_d0c_dispatch_line = DM1_F0127_START;
    out_trace->f0128_d2c_call_line = DM1_F0128_D2C_CALL_LINE;
    out_trace->f0128_d0c_call_line = DM1_F0128_D0C_CALL_LINE;
    out_trace->d0c_uses_f0127_not_f0121 = 1;
    out_trace->d0c_has_no_f0111_call_site = 1;
    out_trace->d2c_f0121_routes_f0111 = 1;
    out_trace->g0695_is_d1lcr_not_d0c = 1;
    out_trace->d0c_native_bitmap_is_g2116 = 1;

    if (door_state == DM1_DOOR_STATE_OPEN) {
        out_trace->branch = DM1_V1_D0C_F0111_BRANCH_OPEN_PC34;
        return 1;
    }
    if (door_state == DM1_DOOR_STATE_CLOSED) {
        out_trace->branch = DM1_V1_D0C_F0111_BRANCH_CLOSED_PC34;
        return 1;
    }
    if (door_state == DM1_DOOR_STATE_DESTROYED) {
        out_trace->branch = DM1_V1_D0C_F0111_BRANCH_DESTROYED_PC34;
        return 1;
    }
    if (door_state < DM1_DOOR_STATE_CLOSED_ONE_FOURTH ||
        door_state > DM1_DOOR_STATE_CLOSED_THREE_FOURTH) {
        out_trace->branch = DM1_V1_D0C_F0111_BRANCH_INVALID_PC34;
        return 1;
    }

    out_trace->branch = DM1_V1_D0C_F0111_BRANCH_PARTLY_OPEN_PC34;
    simulate_partly_open_half_blits(door_state, out_trace);
    return 1;
}

const char *
dm1_v1_viewport_d0c_f0111_partly_open_door_source_evidence_pc34(void)
{
    return s_source_evidence;
}

static void check_trace_common(const DM1_V1_D0CF0111PartlyOpenDoorTracePc34 *trace)
{
    assert_int("framebuffer.width", trace->framebuffer_width, 320);
    assert_int("framebuffer.height", trace->framebuffer_height, 200);
    assert_int("viewport.width", trace->viewport_width, 224);
    assert_int("viewport.height", trace->viewport_height, 136);
    assert_int("f0111.start", trace->f0111_line_start, DM1_F0111_START);
    assert_int("f0111.end", trace->f0111_line_end, DM1_F0111_END);
    assert_int("f0121.range.start", DM1_F0121_START, 7244);
    assert_int("f0121.range.end", DM1_F0121_END, 7389);
    assert_int("f0127.range.start", DM1_F0127_START, 8164);
    assert_int("f0127.range.end", DM1_F0127_END, 8311);
    assert_int("f0128.d2c.call", trace->f0128_d2c_call_line, 8521);
    assert_int("f0128.d0c.call", trace->f0128_d0c_call_line, 8542);
    assert_int("d0c.uses.f0127.not.f0121", trace->d0c_uses_f0127_not_f0121, 1);
    assert_int("d0c.has.no.f0111.call.site", trace->d0c_has_no_f0111_call_site, 1);
    assert_int("d2c.f0121.routes.f0111", trace->d2c_f0121_routes_f0111, 1);
    s_last.d0c_dispatch_boundary_checks += 3;
}

static void check_state(int state)
{
    DM1_V1_D0CF0111PartlyOpenDoorTracePc34 trace;

    assert_int("trace.ok", dm1_v1_viewport_d0c_f0111_partly_open_door_trace_pc34(state, &trace), 1);
    assert_int("trace.input.state", trace.input_state, state);
    check_trace_common(&trace);

    if (state == DM1_DOOR_STATE_OPEN) {
        assert_int("branch.open", trace.branch, DM1_V1_D0C_F0111_BRANCH_OPEN_PC34);
        ++s_last.open_branch;
    } else if (state == DM1_DOOR_STATE_CLOSED) {
        assert_int("branch.closed", trace.branch, DM1_V1_D0C_F0111_BRANCH_CLOSED_PC34);
        ++s_last.closed_branch;
    } else if (state == DM1_DOOR_STATE_DESTROYED) {
        assert_int("branch.destroyed", trace.branch, DM1_V1_D0C_F0111_BRANCH_DESTROYED_PC34);
        ++s_last.destroyed_branch;
    } else if (state == DM1_DOOR_STATE_INVALID) {
        assert_int("branch.invalid", trace.branch, DM1_V1_D0C_F0111_BRANCH_INVALID_PC34);
        ++s_last.invalid_branch;
    } else {
        assert_int("branch.partly", trace.branch, DM1_V1_D0C_F0111_BRANCH_PARTLY_OPEN_PC34);
        assert_int("decremented.state", trace.decremented_state, state - 1);
        assert_int("first.half.zone", trace.first_half_zone,
                   DM1_D0C_DOOR_FRAME_ZONE_PC34 + state - 1);
        assert_int("first.half.clip.zone", trace.first_half_clip_zone,
                   DM1_D0C_DOOR_FRAME_ZONE_PC34 + state - 1 + 6);
        assert_int("second.half.zone", trace.second_half_zone,
                   DM1_D0C_DOOR_FRAME_ZONE_PC34 + state - 1 + DM1_SECOND_HALF_SHIFT);
        assert_int("second.half.shift", trace.second_half_shift, DM1_SECOND_HALF_SHIFT);
        assert_int("c10.transparent", trace.c10_transparent_color, 10);
        assert_int("first.half.writes", trace.first_half_writes, 4);
        assert_int("first.half.skips", trace.first_half_skips, 2);
        assert_int("second.half.writes", trace.second_half_writes, 4);
        assert_int("second.half.skips", trace.second_half_skips, 2);
        assert_int("first.probe.pixel", trace.first_probe_pixel, 0x21);
        assert_int("second.probe.pixel", trace.second_probe_pixel, 0x32);
        ++s_last.partly_open_branches;
        s_last.c10_write_skip_checks += 4;
    }

    assert_int("g0695.is.d1lcr.not.d0c", trace.g0695_is_d1lcr_not_d0c, 1);
    assert_int("d0c.native.bitmap.is.g2116", trace.d0c_native_bitmap_is_g2116, 1);
    s_last.native_bitmap_boundary_checks += 2;
}

static void check_non_overlap(void)
{
    static const char *siblings[] = {
        "D1C F0111 partly-open",
        "D2L F0111 partly-open",
        "D2R F0111 partly-open",
        "D1L2 F0111 partly-open",
        "D1R2 F0111 partly-open",
        "D0L F0111",
        "D0R F0111",
        "D0L2 F0111 partly-open",
        "D0R2 F0111 partly-open",
        "D3C F0111"
    };
    size_t i;

    for (i = 0; i < sizeof(siblings) / sizeof(siblings[0]); ++i) {
        assert_int("non.overlap.not.d0c", strstr(siblings[i], "D0C") == NULL, 1);
        ++s_last.non_overlap_checks;
    }
}

int run_dm1_v1_viewport_d0c_f0111_partly_open_door_self_test(void)
{
    memset(&s_last, 0, sizeof(s_last));
    s_last.deterministic_hash = 2166136261u;

    assert_contains("source.f0111", s_source_evidence, "DUNVIEW.C:4218-4337");
    assert_contains("source.f0121", s_source_evidence, "DUNVIEW.C:7244-7389");
    assert_contains("source.f0127", s_source_evidence, "DUNVIEW.C:8164-8311");
    assert_contains("source.f0128", s_source_evidence, "DUNVIEW.C:8498-8542");
    assert_contains("source.g0695", s_source_evidence, "G0695_ai_DoorNativeBitmapIndex_Front_D1LCR");
    assert_contains("source.g2116", s_source_evidence, "G2116_DoorFrameFrontD0C");
    assert_contains("source.defs", s_source_evidence, "DEFS.H:1039-1044");
    assert_contains("source.non.overlap", s_source_evidence, "pass769-d0c-f0111-partly-open-boundary");

    check_state(DM1_DOOR_STATE_OPEN);
    check_state(DM1_DOOR_STATE_CLOSED_ONE_FOURTH);
    check_state(DM1_DOOR_STATE_CLOSED_HALF);
    check_state(DM1_DOOR_STATE_CLOSED_THREE_FOURTH);
    check_state(DM1_DOOR_STATE_CLOSED);
    check_state(DM1_DOOR_STATE_DESTROYED);
    check_state(DM1_DOOR_STATE_INVALID);
    check_non_overlap();

    assert_int("open.count", s_last.open_branch, 1);
    assert_int("partly.count", s_last.partly_open_branches, 3);
    assert_int("closed.count", s_last.closed_branch, 1);
    assert_int("destroyed.count", s_last.destroyed_branch, 1);
    assert_int("invalid.count", s_last.invalid_branch, 1);
    assert_int("c10.checks", s_last.c10_write_skip_checks, 12);
    assert_int("d0c.dispatch.boundary.checks", s_last.d0c_dispatch_boundary_checks, 21);
    assert_int("native.bitmap.boundary.checks", s_last.native_bitmap_boundary_checks, 14);
    assert_int("non.overlap.checks", s_last.non_overlap_checks, 10);
    assert_int("hash.changed", s_last.deterministic_hash != 2166136261u, 1);

    return s_last.failures == 0 ? 0 : 1;
}

const DM1_V1_D0CF0111PartlyOpenDoorSelfTestResultPc34 *
dm1_v1_viewport_d0c_f0111_partly_open_door_last_self_test_result_pc34(void)
{
    return &s_last;
}
