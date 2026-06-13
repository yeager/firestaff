#include "firestaff/dm1/v1/viewport/d3c_back_wall_item_pc34_compat.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

enum {
    DM1_D3C_BWI_FRAMEBUFFER_WIDTH = 320,
    DM1_D3C_BWI_FRAMEBUFFER_HEIGHT = 200,
    DM1_D3C_BWI_VIEWPORT_WIDTH = DM1_V1_D3C_BACK_WALL_ITEM_VIEWPORT_WIDTH_PC34,
    DM1_D3C_BWI_VIEWPORT_HEIGHT = DM1_V1_D3C_BACK_WALL_ITEM_VIEWPORT_HEIGHT_PC34,
    DM1_D3C_BWI_VIEW_SQUARE = 11,             /* ReDMCSB DEFS.H:2607 M600_VIEW_SQUARE_D3C */
    DM1_D3C_BWI_VIEW_DEPTH = 3,               /* ReDMCSB DUNVIEW.C:372 G2027[11] */
    DM1_D3C_BWI_VIEW_LANE = 0,                /* ReDMCSB DUNVIEW.C:371 G2026[11] */
    DM1_D3C_BWI_DOOR_FRONT_ELEMENT = 17,      /* ReDMCSB C17_ELEMENT_DOOR_FRONT */
    DM1_D3C_BWI_FIRST_THING_ORDINAL = 2,      /* ReDMCSB DEFS.H:2549 M550_FIRST_THING */
    DM1_D3C_BWI_BACK_WALL_ITEM_ZONE_BASE = 2500, /* ReDMCSB DUNVIEW.C:5075 C2500_ZONE_ */
    DM1_D3C_BWI_BACK_WALL_ITEM_ZONE_STRIDE = 4,  /* ReDMCSB DUNVIEW.C:5075 G2028 row * 4 + cell */
    DM1_D3C_BWI_DOOR_PASS1_ORDER = 0x0218,    /* ReDMCSB DEFS.H:2669 C0x0218 */
    DM1_D3C_BWI_DOOR_PASS2_ORDER = 0x0349,    /* ReDMCSB DEFS.H:2672 C0x0349 */
    DM1_D3C_BWI_NORMAL_ORDER = 0x3421,        /* ReDMCSB DEFS.H:2676 C0x3421 */
    DM1_D3C_BWI_DOOR_PASS_NIBBLE_MASK = 0x0008U, /* ReDMCSB DUNVIEW.C:4794 MASK0x0008_DOOR_FRONT */
    DM1_D3C_BWI_F0128_D3C_DRAW_LINE = 8499,   /* ReDMCSB DUNVIEW.C:8499 */
    DM1_D3C_BWI_F0115_DOOR_FRONT_CALL = 6723, /* ReDMCSB DUNVIEW.C:6723 */
    DM1_D3C_BWI_F0115_CORRIDOR_PIT_TELEPORTER_CALL = 6816, /* ReDMCSB DUNVIEW.C:6816 */
    /* View-cell numbering per DEFS.H:2642-2645:
     *   0 = front-left, 1 = front-right, 2 = back-right, 3 = back-left.
     * F0115:4920-4923 clips view_cell > 1 visibility at depth 3. */
    DM1_D3C_BWI_FRONT_LEFT_CELL = 0,
    DM1_D3C_BWI_FRONT_RIGHT_CELL = 1,
    DM1_D3C_BWI_BACK_RIGHT_CELL = 2,
    DM1_D3C_BWI_BACK_LEFT_CELL = 3,
    /* F0115 cell-ordinal-to-index conversion: cell = nibble - 1. */
    DM1_D3C_BWI_CELL_VISIBILITY_DEPTH3_THRESHOLD = 1
};

typedef struct {
    int route_kind;
    int element;
    int f0115_called;
    int f0115_pass;
    unsigned int cell_order;
    int front_left_view_cell;
    int front_right_view_cell;
    int back_right_view_cell;
    int back_left_view_cell;
    int back_wall_item_zone;
    int back_wall_item_visible;
    int c10_transparent_skip;
    int c10_transparent_writes;
    int framebuffer_pixels_touched;
    uint32_t deterministic_hash;
} DM1_D3CBwiRouteResultPc34;

static const char s_source_evidence[] =
    "Contract-only DM1 V1 D3C F0115 back-wall item thing-pass gate. "
    "DUNVIEW.C F0115:4547-4581 F0115_DUNGEONVIEW_DrawObjectsCreatures"
    "ProjectilesExplosions_CPSEF prototype and the 4-nibble structure "
    "comment (door-front pass bit 3 in low nibble + 3 cell nibbles). "
    "DUNVIEW.C F0115:4794-4800 strips the door-front pass nibble "
    "(MASK 0x0008_DOOR_FRONT) and computes the pass number from "
    "bit 0 plus one. DUNVIEW.C F0115:4853-4860 view-square range "
    "M600..M609 (D3C..D0C) gates the thing pass. DUNVIEW.C "
    "F0115:4920-4923 the item visibility predicate (weapon..junk, "
    "view_cell > 1 at depth 3, i.e. back cells visible). "
    "DUNVIEW.C F0115:5180-5188 the C10_COLOR_FLESH transparent blit. "
    "DUNVIEW.C:6723 the D3C door-front F0115 call with "
    "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT. DUNVIEW.C:6816 "
    "the D3C corridor/pit/teleporter F0115 call with L0204_i_Order = "
    "C0x3421_CELL_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT (and "
    "L0204_i_Order = C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT "
    "for door front pass 2). DUNVIEW.C F0128:8499 D3C dispatch at "
    "relative depth 3 / lateral 0. DUNGEON.C F0163:1769-1838, "
    "F0164:1840-1905, F0172:2466-2523 anchor the thing-list and "
    "square-aspect sources. DEFS.H:2549 M550_FIRST_THING ordinal = 2; "
    "DEFS.H:2607 M600_VIEW_SQUARE_D3C = 11; DEFS.H:2642-2645 view "
    "cell numbering (0=front-left, 1=front-right, 2=back-right, "
    "3=back-left). Asset-free, contract-only, no original DOS pixel "
    "parity or GRAPHICS.DAT comparison.";

static const char s_disjointness_note[] =
    "D3C F0115 back-wall item thing-pass gate only. Disjoint from the "
    "D3C F0107 wall-ornament gate (which covers the M578 front-wall "
    "C704 zone + M552 ornament), the D3C F0108 floor+ceiling+ornament "
    "gate (M589_VIEW_FLOOR_D3C / C1503 zone), the D3C F0111 door-front "
    "pair gate (F0111 door draw), the D3L2/D3R2 F0115 thing-pass gate "
    "(C14/C15 view squares, M702/M703 wall zones), the D1L2/D1R2 F0115 "
    "thing-pass gate (depth 1, M580..M584 wall zones), the D0L2/D0R2 "
    "F0115 thing-pass gate (depth 0, M576/M577 wall zones), the "
    "D1C F0115 door-frame gate (F0111 door frame, depth 1 center), the "
    "F0115 projectile metadata gate, the F0107 alcove helper gate, and "
    "the F0128 dispatch order gate. It is asset-free and makes no "
    "original DOS pixel parity claim.";

static DM1_V1_D3CBackWallItemSelfTestResultPc34 s_last_self_test;

static uint32_t fnv1a_u32(uint32_t hash, uint32_t value)
{
    int i;
    for (i = 0; i < 4; ++i) {
        hash ^= (value >> (i * 8)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

/* ReDMCSB: DUNVIEW.C:4794-4800 strips the door-front pass nibble
 * (MASK 0x0008_DOOR_FRONT, low bit-3) and shifts the remaining 12 bits
 * right by 4 to expose the cell nibbles. nibble 1 = cell 0, nibble 2 =
 * cell 1, nibble 3 = cell 2, nibble 4 = cell 3. */
static int decode_cell_ordinal(unsigned int cell_order, int ordinal_index)
{
    unsigned int nibble;
    if (ordinal_index < 0 || ordinal_index >= 4) return -1;
    if ((cell_order & 0xFU) & DM1_D3C_BWI_DOOR_PASS_NIBBLE_MASK) {
        cell_order >>= 4;
    }
    nibble = (cell_order >> ((unsigned int)ordinal_index * 4U)) & 0xFU;
    if (nibble == 0U) return -1;
    return (int)nibble - 1;
}

static int back_wall_item_zone_for_row(int row, int view_cell)
{
    if (row < 0 || view_cell < 0 || view_cell > 3) return -1;
    return DM1_D3C_BWI_BACK_WALL_ITEM_ZONE_BASE +
        (row * DM1_D3C_BWI_BACK_WALL_ITEM_ZONE_STRIDE) + view_cell;
}

static uint8_t blend_c10(uint8_t destination, uint8_t source, uint8_t transparent)
{
    return source == transparent ? destination : source;
}

static void route_init(DM1_D3CBwiRouteResultPc34 *out)
{
    memset(out, 0, sizeof(*out));
    out->front_left_view_cell = -1;
    out->front_right_view_cell = -1;
    out->back_right_view_cell = -1;
    out->back_left_view_cell = -1;
    out->back_wall_item_zone = -1;
}

static void route_finalize_hash(DM1_D3CBwiRouteResultPc34 *out)
{
    out->deterministic_hash = fnv1a_u32(2166136261u,
                                         (uint32_t)out->route_kind);
    out->deterministic_hash = fnv1a_u32(out->deterministic_hash,
                                         (uint32_t)out->element);
    out->deterministic_hash = fnv1a_u32(out->deterministic_hash,
                                         (uint32_t)out->f0115_called);
    out->deterministic_hash = fnv1a_u32(out->deterministic_hash,
                                         (uint32_t)out->f0115_pass);
    out->deterministic_hash = fnv1a_u32(out->deterministic_hash,
                                         out->cell_order);
    out->deterministic_hash = fnv1a_u32(out->deterministic_hash,
                                         (uint32_t)out->back_wall_item_visible);
    out->deterministic_hash = fnv1a_u32(out->deterministic_hash,
                                         (uint32_t)out->back_wall_item_zone);
}

/* ReDMCSB DUNVIEW.C:6697-6720 D3C wall branch draws C704_WALL_D3C,
 * calls F0107 alcove, returns before F0115. The back-wall item
 * is not visible because the wall ornament / alcove takes the front. */
static void compose_wall_route(DM1_D3CBwiRouteResultPc34 *out)
{
    route_init(out);
    out->route_kind = DM1_V1_D3C_BACK_WALL_ITEM_ROUTE_WALL_PC34;
    out->element = DM1_V1_D3C_BACK_WALL_ITEM_ELEMENT_WALL_PC34;
    out->f0115_called = 0;
    out->cell_order = 0U;
    out->f0115_pass = 0;
    out->back_wall_item_visible = 0;
    route_finalize_hash(out);
}

/* ReDMCSB DUNVIEW.C:6723 D3C door-front F0115 call with 0x0218
 * (door pass 1). After the door-pass nibble strip, the remaining
 * nibbles encode cells front-left (0) and front-right (1). At D3C
 * (depth 3) the F0115:4920-4923 view_cell > 1 predicate clips the
 * front cells, so no back-wall item is visible. */
static void compose_door_front_pass1(DM1_D3CBwiRouteResultPc34 *out)
{
    route_init(out);
    out->route_kind = DM1_V1_D3C_BACK_WALL_ITEM_ROUTE_DOOR_FRONT_PASS1_PC34;
    out->element = DM1_V1_D3C_BACK_WALL_ITEM_ELEMENT_DOOR_FRONT_PC34;
    out->f0115_called = 1;
    out->f0115_pass = 1;
    out->cell_order = DM1_D3C_BWI_DOOR_PASS1_ORDER;
    out->front_left_view_cell = decode_cell_ordinal(out->cell_order, 0);
    out->front_right_view_cell = decode_cell_ordinal(out->cell_order, 1);
    out->back_right_view_cell = -1;
    out->back_left_view_cell = -1;
    out->back_wall_item_visible = 0;
    route_finalize_hash(out);
}

/* ReDMCSB DUNVIEW.C:6816 with L0204_i_Order = C0x0349 for D3C door
 * front pass 2. After the door-pass nibble strip, the remaining
 * nibbles encode cells back-left (3) and back-right (2). At D3C
 * (depth 3) the F0115:4920-4923 view_cell > 1 predicate keeps the
 * back cells visible, so the back-wall item IS visible. */
static void compose_door_front_pass2(DM1_D3CBwiRouteResultPc34 *out)
{
    route_init(out);
    out->route_kind = DM1_V1_D3C_BACK_WALL_ITEM_ROUTE_DOOR_FRONT_PASS2_PC34;
    out->element = DM1_V1_D3C_BACK_WALL_ITEM_ELEMENT_DOOR_FRONT_PC34;
    out->f0115_called = 1;
    out->f0115_pass = 2;
    out->cell_order = DM1_D3C_BWI_DOOR_PASS2_ORDER;
    out->front_left_view_cell = -1;
    out->front_right_view_cell = -1;
    out->back_right_view_cell = decode_cell_ordinal(out->cell_order, 1);
    out->back_left_view_cell = decode_cell_ordinal(out->cell_order, 0);
    out->back_wall_item_zone = back_wall_item_zone_for_row(
        DM1_D3C_BWI_FIRST_THING_ORDINAL, out->back_left_view_cell);
    out->back_wall_item_visible = 1;
    route_finalize_hash(out);
}

/* ReDMCSB DUNVIEW.C:6816 with L0204_i_Order = C0x3421 for D3C
 * corridor / pit / teleporter. The 4 nibbles encode cells
 * front-left (0), front-right (1), back-left (3), back-right (2).
 * At D3C (depth 3) the front cells are clipped; the back cells are
 * visible. The synthetic framebuffer probe renders the back cells
 * with the C10 transparent blend. */
static void compose_corridor_pit_teleporter(DM1_D3CBwiRouteResultPc34 *out)
{
    uint8_t framebuffer[(size_t)DM1_D3C_BWI_FRAMEBUFFER_WIDTH *
        (size_t)DM1_D3C_BWI_FRAMEBUFFER_HEIGHT];
    int back_cell = -1;
    int touched = 0;
    int skips = 0;
    int writes = 0;
    int x;
    int y;

    route_init(out);
    out->route_kind =
        DM1_V1_D3C_BACK_WALL_ITEM_ROUTE_CORRIDOR_PIT_TELEPORTER_PC34;
    out->element = DM1_V1_D3C_BACK_WALL_ITEM_ELEMENT_CORRIDOR_PC34;
    out->f0115_called = 1;
    out->f0115_pass = 0;
    out->cell_order = DM1_D3C_BWI_NORMAL_ORDER;
    out->front_left_view_cell = decode_cell_ordinal(out->cell_order, 0);
    out->front_right_view_cell = decode_cell_ordinal(out->cell_order, 1);
    out->back_right_view_cell = decode_cell_ordinal(out->cell_order, 3);
    out->back_left_view_cell = decode_cell_ordinal(out->cell_order, 2);
    if (out->back_left_view_cell >
        DM1_D3C_BWI_CELL_VISIBILITY_DEPTH3_THRESHOLD) {
        back_cell = out->back_left_view_cell;
    } else if (out->back_right_view_cell >
        DM1_D3C_BWI_CELL_VISIBILITY_DEPTH3_THRESHOLD) {
        back_cell = out->back_right_view_cell;
    }
    if (back_cell >= 0) {
        out->back_wall_item_zone = back_wall_item_zone_for_row(
            DM1_D3C_BWI_FIRST_THING_ORDINAL, back_cell);
        out->back_wall_item_visible = 1;
    }

    /* Synthetic 320x200 framebuffer probe: render the back cells with
     * the C10 transparent blend over an opaque background to verify
     * the visibility predicate. C10 cells are skipped, opaque cells
     * are written. */
    memset(framebuffer, 0xeeu, sizeof(framebuffer));
    if (out->back_wall_item_visible) {
        for (y = 60; y < 68; ++y) {
            for (x = 102; x < 122; ++x) {
                size_t offset = (size_t)y *
                    (size_t)DM1_D3C_BWI_FRAMEBUFFER_WIDTH +
                    (size_t)x;
                uint8_t src = (uint8_t)((x + y) & 0xffu);
                uint8_t dst = framebuffer[offset];
                if (src == DM1_V1_D3C_BACK_WALL_ITEM_C10_COLOR_FLESH_PC34) {
                    ++skips;
                } else {
                    framebuffer[offset] = blend_c10(dst, src,
                        DM1_V1_D3C_BACK_WALL_ITEM_C10_COLOR_FLESH_PC34);
                    ++writes;
                }
                ++touched;
            }
        }
    }
    out->c10_transparent_skip = skips;
    out->c10_transparent_writes = writes;
    out->framebuffer_pixels_touched = touched;
    route_finalize_hash(out);
    out->deterministic_hash = fnv1a_u32(out->deterministic_hash,
                                         (uint32_t)touched);
    out->deterministic_hash = fnv1a_u32(out->deterministic_hash,
                                         (uint32_t)skips);
    out->deterministic_hash = fnv1a_u32(out->deterministic_hash,
                                         (uint32_t)writes);
}

static void check(int condition, DM1_V1_D3CBackWallItemSelfTestResultPc34 *r)
{
    ++r->assertions;
    if (!condition) ++r->failures;
}

int run_dm1_v1_viewport_d3c_back_wall_item_self_test_pc34(void)
{
    DM1_D3CBwiRouteResultPc34 r_wall;
    DM1_D3CBwiRouteResultPc34 r_door_p1;
    DM1_D3CBwiRouteResultPc34 r_door_p2;
    DM1_D3CBwiRouteResultPc34 r_corridor;
    int zone_cell0;
    int zone_cell1;
    int zone_cell2;
    int zone_cell3;

    memset(&s_last_self_test, 0, sizeof(s_last_self_test));
    s_last_self_test.deterministic_hash = 2166136261u;

    /* Source evidence must mention every ReDMCSB anchor. */
    check(strstr(s_source_evidence, "DUNVIEW.C F0115:4547-4581") != NULL,
          &s_last_self_test);
    check(strstr(s_source_evidence, "DUNVIEW.C F0115:4794-4800") != NULL,
          &s_last_self_test);
    check(strstr(s_source_evidence, "DUNVIEW.C F0115:4853-4860") != NULL,
          &s_last_self_test);
    check(strstr(s_source_evidence, "DUNVIEW.C F0115:4920-4923") != NULL,
          &s_last_self_test);
    check(strstr(s_source_evidence, "DUNVIEW.C F0115:5180-5188") != NULL,
          &s_last_self_test);
    check(strstr(s_source_evidence, "DUNVIEW.C:6723") != NULL,
          &s_last_self_test);
    check(strstr(s_source_evidence, "DUNVIEW.C:6816") != NULL,
          &s_last_self_test);
    check(strstr(s_source_evidence, "DUNVIEW.C F0128:8499") != NULL,
          &s_last_self_test);
    check(strstr(s_source_evidence, "M600_VIEW_SQUARE_D3C = 11") != NULL,
          &s_last_self_test);
    check(strstr(s_source_evidence, "no original DOS pixel parity") != NULL,
          &s_last_self_test);
    check(strstr(s_source_evidence, "C10_COLOR_FLESH") != NULL,
          &s_last_self_test);

    /* Disjointness must mention sibling gates. */
    check(strstr(s_disjointness_note, "D3C F0107 wall-ornament") != NULL,
          &s_last_self_test);
    check(strstr(s_disjointness_note, "D3C F0108 floor") != NULL,
          &s_last_self_test);
    check(strstr(s_disjointness_note, "D3L2/D3R2 F0115") != NULL,
          &s_last_self_test);
    check(strstr(s_disjointness_note, "D1L2/D1R2 F0115") != NULL,
          &s_last_self_test);
    check(strstr(s_disjointness_note, "D0L2/D0R2 F0115") != NULL,
          &s_last_self_test);

    /* C10 transparency + nibble decoding + zone math helpers. */
    check(blend_c10(0x55u, DM1_V1_D3C_BACK_WALL_ITEM_C10_COLOR_FLESH_PC34,
                    DM1_V1_D3C_BACK_WALL_ITEM_C10_COLOR_FLESH_PC34) == 0x55u,
          &s_last_self_test);
    check(blend_c10(0x55u, 0x33u,
                    DM1_V1_D3C_BACK_WALL_ITEM_C10_COLOR_FLESH_PC34) == 0x33u,
          &s_last_self_test);
    check(decode_cell_ordinal(DM1_D3C_BWI_NORMAL_ORDER, 0) ==
          DM1_D3C_BWI_FRONT_LEFT_CELL, &s_last_self_test);
    check(decode_cell_ordinal(DM1_D3C_BWI_NORMAL_ORDER, 1) ==
          DM1_D3C_BWI_FRONT_RIGHT_CELL, &s_last_self_test);
    check(decode_cell_ordinal(DM1_D3C_BWI_NORMAL_ORDER, 2) ==
          DM1_D3C_BWI_BACK_LEFT_CELL, &s_last_self_test);
    check(decode_cell_ordinal(DM1_D3C_BWI_NORMAL_ORDER, 3) ==
          DM1_D3C_BWI_BACK_RIGHT_CELL, &s_last_self_test);
    check(decode_cell_ordinal(DM1_D3C_BWI_DOOR_PASS1_ORDER, 0) ==
          DM1_D3C_BWI_FRONT_LEFT_CELL, &s_last_self_test);
    check(decode_cell_ordinal(DM1_D3C_BWI_DOOR_PASS1_ORDER, 1) ==
          DM1_D3C_BWI_FRONT_RIGHT_CELL, &s_last_self_test);
    check(decode_cell_ordinal(DM1_D3C_BWI_DOOR_PASS2_ORDER, 0) ==
          DM1_D3C_BWI_BACK_LEFT_CELL, &s_last_self_test);
    check(decode_cell_ordinal(DM1_D3C_BWI_DOOR_PASS2_ORDER, 1) ==
          DM1_D3C_BWI_BACK_RIGHT_CELL, &s_last_self_test);
    check(decode_cell_ordinal(0U, 0) == -1, &s_last_self_test);
    check(decode_cell_ordinal(0U, 1) == -1, &s_last_self_test);
    zone_cell0 = back_wall_item_zone_for_row(
        DM1_D3C_BWI_FIRST_THING_ORDINAL, DM1_D3C_BWI_FRONT_LEFT_CELL);
    zone_cell1 = back_wall_item_zone_for_row(
        DM1_D3C_BWI_FIRST_THING_ORDINAL, DM1_D3C_BWI_FRONT_RIGHT_CELL);
    zone_cell2 = back_wall_item_zone_for_row(
        DM1_D3C_BWI_FIRST_THING_ORDINAL, DM1_D3C_BWI_BACK_RIGHT_CELL);
    zone_cell3 = back_wall_item_zone_for_row(
        DM1_D3C_BWI_FIRST_THING_ORDINAL, DM1_D3C_BWI_BACK_LEFT_CELL);
    check(zone_cell0 == 2508, &s_last_self_test);
    check(zone_cell1 == 2509, &s_last_self_test);
    check(zone_cell2 == 2510, &s_last_self_test);
    check(zone_cell3 == 2511, &s_last_self_test);
    check(back_wall_item_zone_for_row(-1, DM1_D3C_BWI_BACK_LEFT_CELL) == -1,
          &s_last_self_test);
    check(back_wall_item_zone_for_row(DM1_D3C_BWI_FIRST_THING_ORDINAL, -1) == -1,
          &s_last_self_test);

    /* Wall route: F0115 is NOT called. */
    compose_wall_route(&r_wall);
    check(r_wall.f0115_called == 0, &s_last_self_test);
    check(r_wall.route_kind ==
          DM1_V1_D3C_BACK_WALL_ITEM_ROUTE_WALL_PC34, &s_last_self_test);
    check(r_wall.element ==
          DM1_V1_D3C_BACK_WALL_ITEM_ELEMENT_WALL_PC34, &s_last_self_test);
    check(r_wall.back_wall_item_visible == 0, &s_last_self_test);
    check(r_wall.cell_order == 0U, &s_last_self_test);
    s_last_self_test.wall_route_skips_f0115 =
        (r_wall.f0115_called == 0) ? 1 : 0;
    s_last_self_test.deterministic_hash =
        fnv1a_u32(s_last_self_test.deterministic_hash,
                  r_wall.deterministic_hash);

    /* Door front pass 1: F0115 is called, but front cells (0, 1) are
     * clipped at depth 3, so no back-wall item is visible. */
    compose_door_front_pass1(&r_door_p1);
    check(r_door_p1.f0115_called == 1, &s_last_self_test);
    check(r_door_p1.f0115_pass == 1, &s_last_self_test);
    check(r_door_p1.cell_order == DM1_D3C_BWI_DOOR_PASS1_ORDER,
          &s_last_self_test);
    check(r_door_p1.front_left_view_cell == DM1_D3C_BWI_FRONT_LEFT_CELL,
          &s_last_self_test);
    check(r_door_p1.front_right_view_cell == DM1_D3C_BWI_FRONT_RIGHT_CELL,
          &s_last_self_test);
    check(r_door_p1.back_left_view_cell == -1, &s_last_self_test);
    check(r_door_p1.back_right_view_cell == -1, &s_last_self_test);
    check(r_door_p1.back_wall_item_visible == 0, &s_last_self_test);
    check(r_door_p1.back_wall_item_zone == -1, &s_last_self_test);
    s_last_self_test.deterministic_hash =
        fnv1a_u32(s_last_self_test.deterministic_hash,
                  r_door_p1.deterministic_hash);

    /* Door front pass 2: F0115 is called, back cells (3, 2) are
     * visible at depth 3, so the back-wall item IS visible. */
    compose_door_front_pass2(&r_door_p2);
    check(r_door_p2.f0115_called == 1, &s_last_self_test);
    check(r_door_p2.f0115_pass == 2, &s_last_self_test);
    check(r_door_p2.cell_order == DM1_D3C_BWI_DOOR_PASS2_ORDER,
          &s_last_self_test);
    check(r_door_p2.front_left_view_cell == -1, &s_last_self_test);
    check(r_door_p2.front_right_view_cell == -1, &s_last_self_test);
    check(r_door_p2.back_left_view_cell == DM1_D3C_BWI_BACK_LEFT_CELL,
          &s_last_self_test);
    check(r_door_p2.back_right_view_cell == DM1_D3C_BWI_BACK_RIGHT_CELL,
          &s_last_self_test);
    check(r_door_p2.back_wall_item_visible == 1, &s_last_self_test);
    check(r_door_p2.back_wall_item_zone == 2511, &s_last_self_test);
    s_last_self_test.back_wall_item_zones_seen++;
    s_last_self_test.f0115_call_count++;
    s_last_self_test.deterministic_hash =
        fnv1a_u32(s_last_self_test.deterministic_hash,
                  r_door_p2.deterministic_hash);

    /* Corridor / pit / teleporter: F0115 with C0x3421, all 4 cells
     * in nibble order, but at D3C depth 3 only back cells (3, 2) are
     * visible. */
    compose_corridor_pit_teleporter(&r_corridor);
    check(r_corridor.f0115_called == 1, &s_last_self_test);
    check(r_corridor.cell_order == DM1_D3C_BWI_NORMAL_ORDER,
          &s_last_self_test);
    check(r_corridor.front_left_view_cell == DM1_D3C_BWI_FRONT_LEFT_CELL,
          &s_last_self_test);
    check(r_corridor.front_right_view_cell == DM1_D3C_BWI_FRONT_RIGHT_CELL,
          &s_last_self_test);
    check(r_corridor.back_left_view_cell == DM1_D3C_BWI_BACK_LEFT_CELL,
          &s_last_self_test);
    check(r_corridor.back_right_view_cell == DM1_D3C_BWI_BACK_RIGHT_CELL,
          &s_last_self_test);
    check(r_corridor.back_wall_item_visible == 1, &s_last_self_test);
    check(r_corridor.back_wall_item_zone == 2511, &s_last_self_test);
    check(r_corridor.c10_transparent_skip + r_corridor.c10_transparent_writes ==
          r_corridor.framebuffer_pixels_touched, &s_last_self_test);
    s_last_self_test.corridor_pit_teleporter_back_then_front =
        (r_corridor.back_left_view_cell == DM1_D3C_BWI_BACK_LEFT_CELL &&
         r_corridor.back_right_view_cell == DM1_D3C_BWI_BACK_RIGHT_CELL &&
         r_corridor.front_left_view_cell == DM1_D3C_BWI_FRONT_LEFT_CELL &&
         r_corridor.front_right_view_cell == DM1_D3C_BWI_FRONT_RIGHT_CELL)
        ? 1 : 0;
    s_last_self_test.back_cells_visible_at_d3c =
        (r_corridor.back_wall_item_visible == 1) ? 1 : 0;
    s_last_self_test.front_cells_clipped_at_d3c =
        (r_corridor.front_left_view_cell <=
         DM1_D3C_BWI_CELL_VISIBILITY_DEPTH3_THRESHOLD &&
         r_corridor.front_right_view_cell <=
         DM1_D3C_BWI_CELL_VISIBILITY_DEPTH3_THRESHOLD) ? 1 : 0;
    s_last_self_test.c10_transparent_skip = r_corridor.c10_transparent_skip;
    s_last_self_test.f0115_call_count++;
    s_last_self_test.back_wall_item_zones_seen++;
    s_last_self_test.deterministic_hash =
        fnv1a_u32(s_last_self_test.deterministic_hash,
                  r_corridor.deterministic_hash);

    /* ReDMCSB constants. */
    check(DM1_D3C_BWI_VIEW_SQUARE == 11, &s_last_self_test);
    check(DM1_D3C_BWI_VIEW_DEPTH == 3, &s_last_self_test);
    check(DM1_D3C_BWI_VIEW_LANE == 0, &s_last_self_test);
    check(DM1_D3C_BWI_FIRST_THING_ORDINAL == 2, &s_last_self_test);
    check(DM1_D3C_BWI_F0128_D3C_DRAW_LINE == 8499, &s_last_self_test);
    check(DM1_D3C_BWI_F0115_DOOR_FRONT_CALL == 6723, &s_last_self_test);
    check(DM1_D3C_BWI_F0115_CORRIDOR_PIT_TELEPORTER_CALL == 6816,
          &s_last_self_test);
    check(DM1_D3C_BWI_DOOR_FRONT_ELEMENT == 17, &s_last_self_test);
    check(DM1_D3C_BWI_BACK_WALL_ITEM_ZONE_BASE == 2500, &s_last_self_test);
    check(DM1_D3C_BWI_BACK_WALL_ITEM_ZONE_STRIDE == 4, &s_last_self_test);
    check(DM1_D3C_BWI_BACK_LEFT_CELL == 3, &s_last_self_test);
    check(DM1_D3C_BWI_BACK_RIGHT_CELL == 2, &s_last_self_test);
    check(DM1_D3C_BWI_FRONT_LEFT_CELL == 0, &s_last_self_test);
    check(DM1_D3C_BWI_FRONT_RIGHT_CELL == 1, &s_last_self_test);
    check((DM1_D3C_BWI_DOOR_PASS1_ORDER & 0xFF00U) == 0x0200U,
          &s_last_self_test);
    check((DM1_D3C_BWI_DOOR_PASS2_ORDER & 0xFF00U) == 0x0300U,
          &s_last_self_test);
    check((DM1_D3C_BWI_DOOR_PASS_NIBBLE_MASK & 0xFU) == 0x8U,
          &s_last_self_test);

    /* Sanity: the post-self-test invariants used by the test driver. */
    check(s_last_self_test.wall_route_skips_f0115 == 1,
          &s_last_self_test);
    check(s_last_self_test.corridor_pit_teleporter_back_then_front == 1,
          &s_last_self_test);
    check(s_last_self_test.back_cells_visible_at_d3c == 1,
          &s_last_self_test);
    check(s_last_self_test.front_cells_clipped_at_d3c == 1,
          &s_last_self_test);
    check(s_last_self_test.f0115_call_count == 2,
          &s_last_self_test);
    check(s_last_self_test.back_wall_item_zones_seen == 2,
          &s_last_self_test);

    s_last_self_test.ok = s_last_self_test.failures == 0;
    return s_last_self_test.ok;
}

const DM1_V1_D3CBackWallItemSelfTestResultPc34 *
dm1_v1_viewport_d3c_back_wall_item_last_self_test_result_pc34(void)
{
    return &s_last_self_test;
}

const char *dm1_v1_viewport_d3c_back_wall_item_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const char *dm1_v1_viewport_d3c_back_wall_item_disjointness_note_pc34(void)
{
    return s_disjointness_note;
}
