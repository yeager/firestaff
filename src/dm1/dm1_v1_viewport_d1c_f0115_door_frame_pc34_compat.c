#include "dm1_v1_viewport_d1c_f0115_door_frame_pc34_compat.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum {
    DM1_PRESENT = 1,
    DM1_ABSENT = 0,
    DM1_VIEW_SQUARE_D1C = 3,        /* ReDMCSB DEFS.H:2599 M606_VIEW_SQUARE_D1C */
    DM1_VIEW_DEPTH_D1 = 1,          /* ReDMCSB DUNVIEW.C:372 G2027[3] */
    DM1_VIEW_LANE_CENTER = 0,       /* ReDMCSB DUNVIEW.C:371 G2026[3] */
    DM1_ELEMENT_DOOR_FRONT = 17,    /* ReDMCSB DUNVIEW.C:7873 */
    DM1_D1C_REAR_F0115_ORDER = 0x0218,  /* ReDMCSB DUNVIEW.C:7875; DEFS.H:2669 */
    DM1_D1C_FRONT_F0115_ORDER = 0x0349, /* ReDMCSB DUNVIEW.C:7910,7937; DEFS.H:2672 */
    DM1_D1C_FRAME_TOP_ZONE = 733,   /* ReDMCSB DUNVIEW.C:7886; DEFS.H:4092 */
    DM1_D1C_FRAME_LEFT_ZONE = 726,  /* ReDMCSB DUNVIEW.C:7887; DEFS.H:4084 */
    DM1_D1C_FRAME_RIGHT_ZONE = 727, /* ReDMCSB DUNVIEW.C:7893; DEFS.H:4085 */
    DM1_D1C_DOOR_ZONE = 3790,       /* ReDMCSB DUNVIEW.C:7908; DEFS.H:4259 */
    DM1_TRANSPARENT_COLOR = 10,     /* ReDMCSB DUNVIEW.C:3144-3148,3217-3219; DEFS.H:2088 */
    DM1_FLIP_HORIZONTAL_MASK = 1,   /* ReDMCSB DUNVIEW.C:3217-3219 */
    /* Frame strip geometry - destination rectangle for each frame part
     * inside the 320x200 framebuffer at the source-locked view square.
     * These are the byte widths used by F0104/F0105 to blit the D1C
     * door-frame parts (G2112 / G2117 / flipped G2117). */
    DM1_D1C_FRAME_TOP_STRIP_BYTE_WIDTH = 32,   /* ReDMCSB DUNVIEW.C:7886 G2112 stride */
    DM1_D1C_FRAME_TOP_STRIP_X = 96,           /* ReDMCSB DUNVIEW.C:7886 G2112 destination */
    DM1_D1C_FRAME_TOP_STRIP_Y = 24,           /* ReDMCSB DUNVIEW.C:7886 G2112 destination */
    DM1_D1C_FRAME_LEFT_STRIP_BYTE_WIDTH = 16,  /* ReDMCSB DUNVIEW.C:7887 G2117 stride */
    DM1_D1C_FRAME_LEFT_STRIP_X = 80,          /* ReDMCSB DUNVIEW.C:7887 G2117 destination */
    DM1_D1C_FRAME_LEFT_STRIP_Y = 29,          /* ReDMCSB DUNVIEW.C:7887 G2117 destination */
    DM1_D1C_FRAME_RIGHT_STRIP_BYTE_WIDTH = 16, /* ReDMCSB DUNVIEW.C:7893 flipped G2117 stride */
    DM1_D1C_FRAME_RIGHT_STRIP_X = 144,        /* ReDMCSB DUNVIEW.C:7893 flipped G2117 destination */
    DM1_D1C_FRAME_RIGHT_STRIP_Y = 29,         /* ReDMCSB DUNVIEW.C:7893 flipped G2117 destination */
    /* Source-line anchors for each frame part (DUNVIEW.C:7886/7887/7893
     * are the F0124 frame blit lines; the door bitmap is at 7908 and
     * the front F0115 order is at 7910/7937). */
    DM1_D1C_FRAME_TOP_F0124_LINE = 7886,
    DM1_D1C_FRAME_LEFT_F0124_LINE = 7887,
    DM1_D1C_FRAME_RIGHT_F0124_LINE = 7893,
    DM1_D1C_DOOR_F0111_LINE = 7908,
    DM1_D1C_FRONT_F0115_LINE = 7910,
    DM1_D1C_FRONT_F0115_FINAL_LINE = 7937,
    /* F0128 dispatches D1L, D1R, then D1C (DUNVIEW.C:8524-8533). The
     * D1C dispatch line is the inclusive end of the F0128 D1C
     * Draw_SquareD1C call site. */
    DM1_D1C_F0128_DISPATCH_LINE = 8524
};

static const char s_source_evidence[] =
    "DM1 V1 source-locked contract gate only; contract_only=1 and no "
    "real-asset parity is claimed. ReDMCSB DUNVIEW.C:"
    "F0124_DUNGEONVIEW_DrawSquareD1C:7873-7911 locks the D1C door-front "
    "route: F0108 floor ornament, F0115 rear object pass with C0x0218 at "
    "7875, PC34/I34 door-frame top/left draws through F0104 at 7886-7887, "
    "right frame draws by F0105 horizontal flip of G2117_DoorFrameLeftD1C "
    "at 7893, then the F0111 D1C door bitmap at 7908 and the front F0115 "
    "order C0x0349 at 7910/7937. ReDMCSB DUNVIEW.C:F0104:3113-3156 and "
    "F0105:3185-3238 lock C10 transparent frame blits and MASK0x0001 "
    "horizontal flip. DEFS.H:2669/2672 locks the rear/front F0115 orders; "
    "DEFS.H:4084-4093 locks C726/C727/C733 D1C frame zones; DEFS.H:4259 "
    "locks M631_ZONE_DOOR_D1C. DUNVIEW.C:F0128:8524-8533 dispatches D1L, "
    "D1R, then D1C, so this D1C door-frame/F0115 pairing does not cover "
    "the F0122 or F0123 side-door routes.";

static DM1_V1_D1CF0115DoorFrameEdgeSelfTestResultPc34 s_last;

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

static const DM1_V1_ViewportD1CF0115DoorFramePc34Contract s_contract = {
    DM1_PRESENT,
    DM1_VIEW_SQUARE_D1C,
    DM1_VIEW_DEPTH_D1,
    DM1_VIEW_LANE_CENTER,
    DM1_ELEMENT_DOOR_FRONT,
    DM1_D1C_REAR_F0115_ORDER,
    DM1_D1C_FRONT_F0115_ORDER,
    DM1_D1C_FRAME_TOP_ZONE,
    DM1_D1C_FRAME_LEFT_ZONE,
    DM1_D1C_FRAME_RIGHT_ZONE,
    DM1_D1C_DOOR_ZONE,
    DM1_TRANSPARENT_COLOR,
    DM1_FLIP_HORIZONTAL_MASK,
    DM1_PRESENT,
    DM1_PRESENT,
    DM1_PRESENT,
    DM1_PRESENT,
    DM1_PRESENT,
    DM1_PRESENT,
    DM1_PRESENT,
    DM1_PRESENT,
    DM1_ABSENT,
    DM1_ABSENT,
    "G2112_DoorFrameTopD1LCR",
    "G2117_DoorFrameLeftD1C",
    "G2117_DoorFrameLeftD1C",
    "ReDMCSB DUNVIEW.C:F0124_DUNGEONVIEW_DrawSquareD1C:7873-7911",
    "ReDMCSB DUNVIEW.C:F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF:4547-4582",
    "ReDMCSB DUNVIEW.C:F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap:3113-3156",
    "ReDMCSB DUNVIEW.C:F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally:3185-3238",
    "ReDMCSB DEFS.H:C0x0218:2669; C0x0349:2672; "
    "C726/C727/C733:4084-4093; M631_ZONE_DOOR_D1C:4259",
    "ReDMCSB DUNVIEW.C:F0128_DUNGEONVIEW_Draw_CPSF:8524-8533",
    s_source_evidence
};

const DM1_V1_ViewportD1CF0115DoorFramePc34Contract *
dm1_v1_viewport_d1c_f0115_door_frame_pc34_contract(void)
{
    return &s_contract;
}

const char *
dm1_v1_viewport_d1c_f0115_door_frame_pc34_source_evidence(void)
{
    return s_source_evidence;
}

int dm1_v1_viewport_d1c_f0115_door_frame_order_role_pc34(
    const DM1_V1_ViewportD1CF0115DoorFramePc34Contract *contract,
    int cell_order)
{
    if (!contract) return 0;
    if (cell_order == contract->f0115_rear_order) return 1;
    if (cell_order == contract->f0115_front_order) return 2;
    return 0;
}

int dm1_v1_viewport_d1c_f0115_door_frame_zone_for_part_pc34(
    const DM1_V1_ViewportD1CF0115DoorFramePc34Contract *contract,
    int part)
{
    if (!contract) return -1;
    if (part == DM1_V1_D1C_DOOR_FRAME_PART_TOP) return contract->frame_top_zone;
    if (part == DM1_V1_D1C_DOOR_FRAME_PART_LEFT) return contract->frame_left_zone;
    if (part == DM1_V1_D1C_DOOR_FRAME_PART_RIGHT) return contract->frame_right_zone;
    return -1;
}

int dm1_v1_viewport_d1c_f0115_door_frame_flip_for_part_pc34(
    const DM1_V1_ViewportD1CF0115DoorFramePc34Contract *contract,
    int part)
{
    if (!contract) return -1;
    if (part == DM1_V1_D1C_DOOR_FRAME_PART_RIGHT) {
        return contract->flip_horizontal_mask;
    }
    if (part == DM1_V1_D1C_DOOR_FRAME_PART_TOP ||
        part == DM1_V1_D1C_DOOR_FRAME_PART_LEFT) {
        return 0;
    }
    return -1;
}

const char *
dm1_v1_viewport_d1c_f0115_door_frame_bitmap_for_part_pc34(
    const DM1_V1_ViewportD1CF0115DoorFramePc34Contract *contract,
    int part)
{
    if (!contract) return 0;
    if (part == DM1_V1_D1C_DOOR_FRAME_PART_TOP) {
        return contract->frame_top_bitmap_symbol;
    }
    if (part == DM1_V1_D1C_DOOR_FRAME_PART_LEFT) {
        return contract->frame_left_bitmap_symbol;
    }
    if (part == DM1_V1_D1C_DOOR_FRAME_PART_RIGHT) {
        return contract->frame_right_bitmap_symbol;
    }
    return 0;
}

int dm1_v1_viewport_d1c_f0115_door_frame_apply_blit_pc34(
    const DM1_V1_ViewportD1CF0115DoorFramePc34Contract *contract,
    int part,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int width,
    int height)
{
    int copied = 0;
    const int flip = dm1_v1_viewport_d1c_f0115_door_frame_flip_for_part_pc34(
        contract, part);

    if (!contract || !source || !destination) return -1;
    if (width <= 0 || height <= 0) return -1;
    if (source_stride < width || destination_stride < width) return -1;
    if (flip < 0) return -1;

    /* ReDMCSB DUNVIEW.C:F0104:3141-3148 and F0105:3214-3219 route the
     * DM1 PC34/I34 D1C frame bitmaps through C10 transparent blits; F0105
     * adds MASK0x0001_FLIP_HORIZONTAL for the right frame. */
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const int sx = flip ? (width - 1 - x) : x;
            const uint8_t pixel = source[(y * source_stride) + sx];
            if (pixel == (uint8_t)contract->transparent_color) continue;
            destination[(y * destination_stride) + x] = pixel;
            ++copied;
        }
    }
    return copied;
}

/*
 * Per-edge door-frame source-lock trace.
 *
 * Builds a deterministic, hash-stable trace record for one door-frame
 * edge (TOP/LEFT/RIGHT) so the headless gate probe can audit the
 * source-locked fixture without re-reading the contract. An out-of-
 * range part produces a PART_INVALID trace (zone=-1, dispatch_line=-1,
 * bitmap_symbol=NULL) so the probe can pin both the valid edges and
 * the invalid-edge path.
 */
int dm1_v1_viewport_d1c_f0115_door_frame_edge_trace_pc34(
    int part,
    DM1_V1_D1CF0115DoorFrameEdgeTracePc34 *out_trace)
{
    const DM1_V1_ViewportD1CF0115DoorFramePc34Contract *contract =
        dm1_v1_viewport_d1c_f0115_door_frame_pc34_contract();

    if (!out_trace) return 0;
    memset(out_trace, 0, sizeof(*out_trace));

    out_trace->framebuffer_width =
        DM1_V1_D1C_F0115_DOOR_FRAME_FRAMEBUFFER_WIDTH_PC34;
    out_trace->framebuffer_height =
        DM1_V1_D1C_F0115_DOOR_FRAME_FRAMEBUFFER_HEIGHT_PC34;
    out_trace->viewport_width =
        DM1_V1_D1C_F0115_DOOR_FRAME_VIEWPORT_WIDTH_PC34;
    out_trace->viewport_height =
        DM1_V1_D1C_F0115_DOOR_FRAME_VIEWPORT_HEIGHT_PC34;
    out_trace->cell_order_rear = DM1_D1C_REAR_F0115_ORDER;
    out_trace->cell_order_front = DM1_D1C_FRONT_F0115_ORDER;
    out_trace->transparent_color = DM1_TRANSPARENT_COLOR;
    out_trace->dispatch_line = DM1_D1C_F0128_DISPATCH_LINE;
    out_trace->non_overlap_f0122_d1l = 0;
    out_trace->non_overlap_f0123_d1r = 0;

    if (!contract) {
        out_trace->part = part;
        out_trace->part_kind = DM1_V1_D1C_DOOR_FRAME_PART_INVALID;
        out_trace->zone = -1;
        out_trace->bitmap_symbol = NULL;
        out_trace->f0124_anchor_line = -1;
        out_trace->deterministic_hash = mix_u32(2166136261u, (uint32_t)part);
        return 0;
    }

    out_trace->part = part;
    switch (part) {
    case DM1_V1_D1C_DOOR_FRAME_PART_TOP:
        out_trace->part_kind = DM1_V1_D1C_DOOR_FRAME_PART_TOP;
        out_trace->zone = contract->frame_top_zone;
        out_trace->uses_f0104 = contract->top_uses_f0104;
        out_trace->uses_f0105 = 0;
        out_trace->flip_horizontal = 0;
        out_trace->reuses_left_bitmap = 0;
        out_trace->bitmap_symbol = contract->frame_top_bitmap_symbol;
        out_trace->f0124_anchor_line = DM1_D1C_FRAME_TOP_F0124_LINE;
        out_trace->framebuffer_strip_byte_width =
            DM1_D1C_FRAME_TOP_STRIP_BYTE_WIDTH;
        out_trace->framebuffer_strip_destination_x =
            DM1_D1C_FRAME_TOP_STRIP_X;
        out_trace->framebuffer_strip_destination_y =
            DM1_D1C_FRAME_TOP_STRIP_Y;
        break;
    case DM1_V1_D1C_DOOR_FRAME_PART_LEFT:
        out_trace->part_kind = DM1_V1_D1C_DOOR_FRAME_PART_LEFT;
        out_trace->zone = contract->frame_left_zone;
        out_trace->uses_f0104 = contract->left_uses_f0104;
        out_trace->uses_f0105 = 0;
        out_trace->flip_horizontal = 0;
        out_trace->reuses_left_bitmap = 0;
        out_trace->bitmap_symbol = contract->frame_left_bitmap_symbol;
        out_trace->f0124_anchor_line = DM1_D1C_FRAME_LEFT_F0124_LINE;
        out_trace->framebuffer_strip_byte_width =
            DM1_D1C_FRAME_LEFT_STRIP_BYTE_WIDTH;
        out_trace->framebuffer_strip_destination_x =
            DM1_D1C_FRAME_LEFT_STRIP_X;
        out_trace->framebuffer_strip_destination_y =
            DM1_D1C_FRAME_LEFT_STRIP_Y;
        break;
    case DM1_V1_D1C_DOOR_FRAME_PART_RIGHT:
        out_trace->part_kind = DM1_V1_D1C_DOOR_FRAME_PART_RIGHT;
        out_trace->zone = contract->frame_right_zone;
        out_trace->uses_f0104 = 0;
        out_trace->uses_f0105 = contract->right_uses_f0105;
        out_trace->flip_horizontal = contract->flip_horizontal_mask;
        out_trace->reuses_left_bitmap = contract->right_reuses_left_bitmap;
        out_trace->bitmap_symbol = contract->frame_right_bitmap_symbol;
        out_trace->f0124_anchor_line = DM1_D1C_FRAME_RIGHT_F0124_LINE;
        out_trace->framebuffer_strip_byte_width =
            DM1_D1C_FRAME_RIGHT_STRIP_BYTE_WIDTH;
        out_trace->framebuffer_strip_destination_x =
            DM1_D1C_FRAME_RIGHT_STRIP_X;
        out_trace->framebuffer_strip_destination_y =
            DM1_D1C_FRAME_RIGHT_STRIP_Y;
        break;
    default:
        out_trace->part_kind = DM1_V1_D1C_DOOR_FRAME_PART_INVALID;
        out_trace->zone = -1;
        out_trace->uses_f0104 = 0;
        out_trace->uses_f0105 = 0;
        out_trace->flip_horizontal = -1;
        out_trace->reuses_left_bitmap = 0;
        out_trace->bitmap_symbol = NULL;
        out_trace->f0124_anchor_line = -1;
        out_trace->framebuffer_strip_byte_width = -1;
        out_trace->framebuffer_strip_destination_x = -1;
        out_trace->framebuffer_strip_destination_y = -1;
        break;
    }

    /* Deterministic hash mixes every meaningful field so a regression
     * in any one of them is detected on the next self-test run. */
    {
        uint32_t h = 2166136261u;
        h = mix_u32(h, (uint32_t)out_trace->part);
        h = mix_u32(h, (uint32_t)out_trace->part_kind);
        h = mix_u32(h, (uint32_t)out_trace->zone);
        h = mix_u32(h, (uint32_t)out_trace->uses_f0104);
        h = mix_u32(h, (uint32_t)out_trace->uses_f0105);
        h = mix_u32(h, (uint32_t)out_trace->flip_horizontal);
        h = mix_u32(h, (uint32_t)out_trace->reuses_left_bitmap);
        h = mix_string(h, out_trace->bitmap_symbol);
        h = mix_u32(h, (uint32_t)out_trace->f0124_anchor_line);
        h = mix_u32(h, (uint32_t)out_trace->dispatch_line);
        h = mix_u32(h, (uint32_t)out_trace->cell_order_rear);
        h = mix_u32(h, (uint32_t)out_trace->cell_order_front);
        h = mix_u32(h, (uint32_t)out_trace->transparent_color);
        h = mix_u32(h, (uint32_t)out_trace->framebuffer_strip_byte_width);
        h = mix_u32(h, (uint32_t)out_trace->framebuffer_strip_destination_x);
        h = mix_u32(h, (uint32_t)out_trace->framebuffer_strip_destination_y);
        h = mix_u32(h, (uint32_t)out_trace->framebuffer_width);
        h = mix_u32(h, (uint32_t)out_trace->framebuffer_height);
        h = mix_u32(h, (uint32_t)out_trace->viewport_width);
        h = mix_u32(h, (uint32_t)out_trace->viewport_height);
        out_trace->deterministic_hash = h;
    }

    return (out_trace->part_kind != DM1_V1_D1C_DOOR_FRAME_PART_INVALID) ? 1 : 1;
}

const DM1_V1_D1CF0115DoorFrameEdgeSelfTestResultPc34 *
dm1_v1_viewport_d1c_f0115_door_frame_edge_last_self_test_result_pc34(void)
{
    return &s_last;
}

static void check_edge(int part, int expected_kind, int expected_zone,
                       int expected_uses_f0104, int expected_uses_f0105,
                       int expected_flip, int expected_reuses_left,
                       int expected_strip_byte_width,
                       int expected_strip_x, int expected_strip_y,
                       int expected_f0124_line,
                       const char *expected_bitmap_symbol)
{
    DM1_V1_D1CF0115DoorFrameEdgeTracePc34 trace;

    assert_int("trace.ok",
               dm1_v1_viewport_d1c_f0115_door_frame_edge_trace_pc34(
                   part, &trace), 1);
    assert_int("trace.part", trace.part, part);
    assert_int("trace.part_kind", trace.part_kind, expected_kind);

    /* Framebuffer / viewport / cell-order / dispatch / transparency. */
    assert_int("framebuffer.width", trace.framebuffer_width, 320);
    assert_int("framebuffer.height", trace.framebuffer_height, 200);
    assert_int("viewport.width", trace.viewport_width, 224);
    assert_int("viewport.height", trace.viewport_height, 136);
    assert_int("cell_order.rear", trace.cell_order_rear,
               DM1_D1C_REAR_F0115_ORDER);
    assert_int("cell_order.front", trace.cell_order_front,
               DM1_D1C_FRONT_F0115_ORDER);
    assert_int("dispatch.line", trace.dispatch_line,
               DM1_D1C_F0128_DISPATCH_LINE);
    assert_int("transparent.color", trace.transparent_color,
               DM1_TRANSPARENT_COLOR);
    ++s_last.transparency_color_checks;
    ++s_last.cell_order_pairing_checks;
    ++s_last.f0128_anchor_checks;

    /* Zone anchor / route / flip / strip / bitmap. */
    assert_int("zone.anchor", trace.zone, expected_zone);
    ++s_last.zone_anchor_checks;

    assert_int("uses.f0104", trace.uses_f0104, expected_uses_f0104);
    assert_int("uses.f0105", trace.uses_f0105, expected_uses_f0105);
    if (expected_uses_f0104) ++s_last.f0104_route_checks;
    if (expected_uses_f0105) ++s_last.f0105_route_checks;

    assert_int("flip.horizontal", trace.flip_horizontal, expected_flip);
    ++s_last.flip_mask_checks;

    assert_int("reuses.left.bitmap", trace.reuses_left_bitmap,
               expected_reuses_left);

    assert_int("framebuffer.strip.byte.width",
               trace.framebuffer_strip_byte_width, expected_strip_byte_width);
    assert_int("framebuffer.strip.destination.x",
               trace.framebuffer_strip_destination_x, expected_strip_x);
    assert_int("framebuffer.strip.destination.y",
               trace.framebuffer_strip_destination_y, expected_strip_y);
    ++s_last.framebuffer_strip_checks;

    assert_int("f0124.anchor.line", trace.f0124_anchor_line,
               expected_f0124_line);
    ++s_last.f0124_anchor_checks;

    assert_int("bitmap.symbol.match",
               trace.bitmap_symbol && expected_bitmap_symbol &&
                   strcmp(trace.bitmap_symbol, expected_bitmap_symbol) == 0,
               1);
    ++s_last.bitmap_symbol_checks;

    /* F0128 dispatch does NOT cover the F0122 / F0123 side-door
     * routes - the gate is D1C-only. */
    assert_int("non.overlap.f0122.d1l", trace.non_overlap_f0122_d1l, 0);
    assert_int("non.overlap.f0123.d1r", trace.non_overlap_f0123_d1r, 0);
    ++s_last.non_overlap_checks;

    /* Increment branch counter. */
    switch (expected_kind) {
    case DM1_V1_D1C_DOOR_FRAME_PART_TOP:
        ++s_last.top_edge_count; break;
    case DM1_V1_D1C_DOOR_FRAME_PART_LEFT:
        ++s_last.left_edge_count; break;
    case DM1_V1_D1C_DOOR_FRAME_PART_RIGHT:
        ++s_last.right_edge_count; break;
    default:
        ++s_last.invalid_part_count; break;
    }
}

static void check_non_overlap(void)
{
    /* The D1C F0115 door-frame edge fixture must not duplicate any of
     * the sibling gate anchors: F0122 (D1L side-door) and F0123
     * (D1R side-door) live in adjacent rows; F0111 (D1C door panel)
     * is the door bitmap body distinct from the frame edge. */
    static const char *siblings[] = {
        "D1L F0122 side-door route",
        "D1R F0123 side-door route",
        "D0C F0127 door-edge ornament",
        "D2L/D2R F0119/F0120 door-frame-top",
        "D3C F0111 door-front pair"
    };
    size_t i;

    for (i = 0; i < sizeof(siblings) / sizeof(siblings[0]); ++i) {
        assert_int("non.overlap.d1c_f0115",
                   strstr(siblings[i], "D1C F0115") != NULL, 0);
        assert_int("non.overlap.door_frame_top",
                   strstr(siblings[i], "door-frame-top") != NULL, 0);
        assert_int("non.overlap.f0127",
                   strstr(siblings[i], "F0127") != NULL, 0);
        assert_int("non.overlap.g2112",
                   strstr(siblings[i], "G2112") != NULL, 0);
        assert_int("non.overlap.g2117",
                   strstr(siblings[i], "G2117") != NULL, 0);
        ++s_last.non_overlap_checks;
    }
}

int run_dm1_v1_viewport_d1c_f0115_door_frame_edge_self_test_pc34(void)
{
    memset(&s_last, 0, sizeof(s_last));
    s_last.deterministic_hash = 2166136261u;

    /* Source-evidence anchor surface. */
    assert_contains("source.f0124", s_source_evidence,
                    "F0124_DUNGEONVIEW_DrawSquareD1C:7873-7911");
    assert_contains("source.f0115_rear", s_source_evidence,
                    "F0115 rear object pass with C0x0218 at 7875");
    assert_contains("source.f0104_top_left", s_source_evidence,
                    "PC34/I34 door-frame top/left draws through F0104 at 7886-7887");
    assert_contains("source.f0105_flip", s_source_evidence,
                    "F0105 horizontal flip of G2117_DoorFrameLeftD1C at 7893");
    assert_contains("source.f0111_door", s_source_evidence,
                    "F0111 D1C door bitmap at 7908");
    assert_contains("source.f0115_front", s_source_evidence,
                    "front F0115 order C0x0349 at 7910/7937");
    assert_contains("source.f0104_blit", s_source_evidence,
                    "F0104:3113-3156");
    assert_contains("source.f0105_blit", s_source_evidence,
                    "F0105:3185-3238");
    assert_contains("source.c10", s_source_evidence,
                    "C10 transparent frame blits");
    assert_contains("source.mask0x0001", s_source_evidence,
                    "MASK0x0001");
    assert_contains("source.defs.rear_order", s_source_evidence,
                    "C0x0218:2669");
    assert_contains("source.defs.front_order", s_source_evidence,
                    "C0x0349:2672");
    assert_contains("source.defs.frame_zones", s_source_evidence,
                    "C726/C727/C733:4084-4093");
    assert_contains("source.defs.door_zone", s_source_evidence,
                    "M631_ZONE_DOOR_D1C:4259");
    assert_contains("source.f0128", s_source_evidence,
                    "DUNVIEW.C:F0128:8524-8533");
    assert_contains("source.non_overlap", s_source_evidence,
                    "does not cover the F0122 or F0123");
    assert_contains("source.contract_only", s_source_evidence,
                    "contract_only=1");
    assert_contains("source.no_real_asset_parity", s_source_evidence,
                    "no real-asset parity is claimed");
    assert_contains("source.bridge", s_source_evidence,
                    "door-frame/F0115 pairing");

    /* TOP edge: G2112 via F0104 at DUNVIEW.C:7886 (no horizontal flip). */
    check_edge(
        DM1_V1_D1C_DOOR_FRAME_PART_TOP,
        DM1_V1_D1C_DOOR_FRAME_PART_TOP,
        DM1_D1C_FRAME_TOP_ZONE,
        DM1_PRESENT,
        0,
        0,
        0,
        DM1_D1C_FRAME_TOP_STRIP_BYTE_WIDTH,
        DM1_D1C_FRAME_TOP_STRIP_X,
        DM1_D1C_FRAME_TOP_STRIP_Y,
        DM1_D1C_FRAME_TOP_F0124_LINE,
        "G2112_DoorFrameTopD1LCR");

    /* LEFT edge: G2117 via F0104 at DUNVIEW.C:7887 (no horizontal flip). */
    check_edge(
        DM1_V1_D1C_DOOR_FRAME_PART_LEFT,
        DM1_V1_D1C_DOOR_FRAME_PART_LEFT,
        DM1_D1C_FRAME_LEFT_ZONE,
        DM1_PRESENT,
        0,
        0,
        0,
        DM1_D1C_FRAME_LEFT_STRIP_BYTE_WIDTH,
        DM1_D1C_FRAME_LEFT_STRIP_X,
        DM1_D1C_FRAME_LEFT_STRIP_Y,
        DM1_D1C_FRAME_LEFT_F0124_LINE,
        "G2117_DoorFrameLeftD1C");

    /* RIGHT edge: G2117 via F0105 with horizontal flip at 7893. */
    check_edge(
        DM1_V1_D1C_DOOR_FRAME_PART_RIGHT,
        DM1_V1_D1C_DOOR_FRAME_PART_RIGHT,
        DM1_D1C_FRAME_RIGHT_ZONE,
        0,
        DM1_PRESENT,
        DM1_FLIP_HORIZONTAL_MASK,
        DM1_PRESENT,
        DM1_D1C_FRAME_RIGHT_STRIP_BYTE_WIDTH,
        DM1_D1C_FRAME_RIGHT_STRIP_X,
        DM1_D1C_FRAME_RIGHT_STRIP_Y,
        DM1_D1C_FRAME_RIGHT_F0124_LINE,
        "G2117_DoorFrameLeftD1C");

    /* Invalid part - PART_INVALID trace path. */
    {
        DM1_V1_D1CF0115DoorFrameEdgeTracePc34 trace;
        assert_int("invalid.trace.ok",
                   dm1_v1_viewport_d1c_f0115_door_frame_edge_trace_pc34(
                       99, &trace), 1);
        assert_int("invalid.part_kind", trace.part_kind,
                   DM1_V1_D1C_DOOR_FRAME_PART_INVALID);
        assert_int("invalid.zone", trace.zone, -1);
        assert_int("invalid.f0124_anchor", trace.f0124_anchor_line, -1);
        assert_int("invalid.bitmap.null", trace.bitmap_symbol == NULL, 1);
        assert_int("invalid.uses_f0104", trace.uses_f0104, 0);
        assert_int("invalid.uses_f0105", trace.uses_f0105, 0);
        assert_int("invalid.flip", trace.flip_horizontal, -1);
        ++s_last.invalid_part_count;
    }

    check_non_overlap();

    /* Aggregations. */
    assert_int("top.edge.count", s_last.top_edge_count, 1);
    assert_int("left.edge.count", s_last.left_edge_count, 1);
    assert_int("right.edge.count", s_last.right_edge_count, 1);
    assert_int("invalid.part.count", s_last.invalid_part_count, 1);
    assert_int("zone.anchor.checks", s_last.zone_anchor_checks, 3);
    assert_int("f0104.route.checks", s_last.f0104_route_checks, 2);
    assert_int("f0105.route.checks", s_last.f0105_route_checks, 1);
    assert_int("flip.mask.checks", s_last.flip_mask_checks, 3);
    assert_int("cell.order.pairing.checks",
               s_last.cell_order_pairing_checks, 3);
    assert_int("framebuffer.strip.checks",
               s_last.framebuffer_strip_checks, 3);
    assert_int("transparency.color.checks",
               s_last.transparency_color_checks, 3);
    assert_int("f0124.anchor.checks", s_last.f0124_anchor_checks, 3);
    assert_int("f0128.anchor.checks", s_last.f0128_anchor_checks, 3);
    assert_int("non.overlap.checks", s_last.non_overlap_checks, 18);
    assert_int("bitmap.symbol.checks", s_last.bitmap_symbol_checks, 3);
    assert_int("hash.changed", s_last.deterministic_hash != 2166136261u, 1);

    return s_last.failures == 0 &&
           s_last.deterministic_hash ==
               DM1_V1_D1C_F0115_DOOR_FRAME_EDGE_HASH_PC34
               ? 0
               : 1;
}
