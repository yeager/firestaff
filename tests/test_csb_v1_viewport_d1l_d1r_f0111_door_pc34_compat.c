#include "csb/csb_v1_viewport_d1l_d1r_f0111_door_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * CSB V1 viewport D1L/D1R F0111 door source-lock contract.
 *
 * Required anchors:
 * - ReDMCSB DUNVIEW.C F0111 lines 4218-4337: door-front composition,
 *   open-door skip, partial-state frame decrement, LeftHorizontal and
 *   RightHorizontal selection, zone + state math, MASK 0x4000 second-half
 *   zone shift, and C10 transparent F0791_DUNGEONVIEW_DrawBitmapXX blit.
 * - ReDMCSB DUNVIEW.C F0128 lines 8318-8542: viewport dispatch reaches
 *   F0122 D1L at line 8525 and F0123 D1R at line 8529.
 * - ReDMCSB DUNVIEW.C F0104 lines 3113-3156, F0105 lines 3185-3225,
 *   F0107 lines 3502-3590, and F0108 lines 3940-4011: wall/floor/
 *   ornament composition callers used around the D1 wall cells.
 * - ReDMCSB DUNGEON.C F0163 lines 1769-1838, F0164 lines 1840-1878,
 *   and F0172 lines 2466-2621: thing-list and square-aspect zone inputs.
 * - ReDMCSB DEFS.H lines 2088, 2596-2611, 2662, 2668-2677, 4045-4046,
 *   and 4139-4153: C10, CSB view-square values, order constants, wall
 *   zones, and CSB zone block.
 * - CSB-lineage Viewport.cpp lines 1192-1209, 1865-1879, 1903-1915,
 *   1930-1944, and 6507-6548: open/door-facing composition and masked
 *   decoration application. D1L/D1R side door-facing arrays are pinned at
 *   Viewport.cpp lines 1892-1900 and 1919-1927.
 */

static int g_assertions;
static uint32_t g_hash = 2166136261u;

static uint32_t hash_u32(uint32_t hash, uint32_t value)
{
    hash ^= value & 0xffu;
    hash *= 16777619u;
    hash ^= (value >> 8) & 0xffu;
    hash *= 16777619u;
    hash ^= (value >> 16) & 0xffu;
    hash *= 16777619u;
    hash ^= (value >> 24) & 0xffu;
    hash *= 16777619u;
    return hash;
}

static int expect_int(const char *label, int got, int want, const char *anchor)
{
    ++g_assertions;
    g_hash = hash_u32(g_hash, (uint32_t)got);
    g_hash = hash_u32(g_hash, (uint32_t)want);
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want, anchor);
        return 0;
    }
    return 1;
}

static int expect_contains(const char *label, const char *haystack,
                           const char *needle, const char *anchor)
{
    const int got = haystack && needle && strstr(haystack, needle) != NULL;
    return expect_int(label, got, 1, anchor);
}

static unsigned read_be16(const unsigned char *p)
{
    return ((unsigned)p[0] << 8) | (unsigned)p[1];
}

static uint32_t fnv1a32(const unsigned char *data, size_t size)
{
    uint32_t hash = 2166136261u;
    size_t i;

    for (i = 0; i < size; ++i) {
        hash ^= data[i];
        hash *= 16777619u;
    }
    return hash;
}

static int read_real_graphics_item_hash(const char *path,
                                        unsigned item_index,
                                        size_t *out_size,
                                        uint32_t *out_hash)
{
    FILE *fp;
    unsigned char header[4];
    unsigned char *table = NULL;
    unsigned char *payload = NULL;
    unsigned count;
    size_t table_bytes;
    size_t data_offset;
    size_t payload_offset;
    size_t payload_size;
    unsigned i;
    int ok = 0;

    if (!path || !out_size || !out_hash) return 0;
    fp = fopen(path, "rb");
    if (!fp) return 0;
    if (fread(header, 1u, sizeof(header), fp) != sizeof(header)) {
        fclose(fp);
        return 0;
    }
    if (read_be16(header) != 0x8001u) {
        fclose(fp);
        return 0;
    }
    count = read_be16(header + 2u);
    if (count == 0u || item_index >= count || count > 2048u) {
        fclose(fp);
        return 0;
    }

    table_bytes = (size_t)count * 4u;
    table = (unsigned char *)malloc(table_bytes);
    if (!table || fread(table, 1u, table_bytes, fp) != table_bytes) {
        free(table);
        fclose(fp);
        return 0;
    }

    data_offset = 4u + table_bytes;
    payload_offset = data_offset;
    for (i = 0; i < item_index; ++i) {
        payload_offset += read_be16(table + (size_t)i * 2u);
    }
    payload_size = read_be16(table + (size_t)item_index * 2u);
    if (payload_size == 0u ||
        read_be16(table + (size_t)count * 2u + (size_t)item_index * 2u) == 0u ||
        fseek(fp, (long)payload_offset, SEEK_SET) != 0) {
        free(table);
        fclose(fp);
        return 0;
    }

    payload = (unsigned char *)malloc(payload_size);
    if (payload &&
        fread(payload, 1u, payload_size, fp) == payload_size) {
        *out_size = payload_size;
        *out_hash = fnv1a32(payload, payload_size);
        ok = *out_hash != 0u;
    }
    free(payload);
    free(table);
    fclose(fp);
    return ok;
}

static int test_route_identity(void)
{
    int ok = 1;
    const CSB_V1_ViewportD1LD1RF0111Route *d1l =
        csb_v1_viewport_d1l_d1r_f0111_door_pc34_route_for_square(4);
    const CSB_V1_ViewportD1LD1RF0111Route *d1r =
        csb_v1_viewport_d1l_d1r_f0111_door_pc34_route_for_square(5);

    ok &= expect_int("route.count",
                     (int)csb_v1_viewport_d1l_d1r_f0111_door_pc34_route_count(),
                     2, "DEFS.H:2596-2611");
    ok &= expect_int("step.count",
                     (int)csb_v1_viewport_d1l_d1r_f0111_door_pc34_step_count(),
                     5, "DUNVIEW.C:7493-7508/7661-7676");
    ok &= expect_int("d1l.present", d1l != NULL, 1, "DUNVIEW.C:8525");
    ok &= expect_int("d1r.present", d1r != NULL, 1, "DUNVIEW.C:8529");
    ok &= expect_int("bad.square.absent",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_route_for_square(6) ==
                         NULL,
                     1, "DEFS.H:2596-2611");
    ok &= expect_int("route0.is.d1l",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_route_at(0) == d1l,
                     1, "DUNVIEW.C:7492");
    ok &= expect_int("route1.is.d1r",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_route_at(1) == d1r,
                     1, "DUNVIEW.C:7660");
    ok &= expect_int("route2.null",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_route_at(2) == NULL,
                     1, "DEFS.H:2596-2611");
    ok &= expect_int("bad.step.null",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_step_at(0, 5) ==
                         NULL,
                     1, "DUNVIEW.C:7493-7508");
    ok &= expect_int("bad.route.step.null",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_step_at(2, 0) ==
                         NULL,
                     1, "DUNVIEW.C:7493-7508");
    return ok;
}

static int test_route_constants(const CSB_V1_ViewportD1LD1RF0111Route *route,
                                int is_right)
{
    int ok = 1;
    const int view_square = is_right ? 5 : 4;
    const int lane = is_right ? 1 : -1;
    const int map_side = is_right ? 1 : -1;
    const int floor_view = is_right ? 596 : 594;
    const int field_zone = is_right ? 714 : 713;
    const int door_zone = is_right ? 3800 : 3780;
    const int top_track = is_right ? 734 : 732;
    const int rear_order = is_right ? 0x0018 : 0x0028;
    const int front_order = is_right ? 0x0049 : 0x0039;
    const int corridor_order = is_right ? 0x0041 : 0x0032;
    const int dispatch_line = is_right ? 8529 : 8525;
    const int start_line = is_right ? 7660 : 7492;
    const int f0111_line = is_right ? 7674 : 7506;
    const int lineage_line = is_right ? 1919 : 1892;

    ok &= expect_int("route.view_square", route ? route->view_square : -1,
                     view_square, "DEFS.H:2596-2611");
    ok &= expect_int("route.depth", route ? route->depth : -1, 1,
                     "DUNVIEW.C F0128 depth-1 dispatch");
    ok &= expect_int("route.lane", route ? route->lane : 0, lane,
                     "DUNVIEW.C F0128 side dispatch");
    ok &= expect_int("route.forward", route ? route->map_forward : -1, 1,
                     "DUNVIEW.C:8524-8529");
    ok &= expect_int("route.side", route ? route->map_side : 0, map_side,
                     "DUNVIEW.C:8524-8529");
    ok &= expect_int("route.floor_view", route ? route->floor_view : -1,
                     floor_view, "DUNVIEW.C:7493/7661");
    ok &= expect_int("route.field_zone", route ? route->field_zone : -1,
                     field_zone, "DEFS.H:4053-4054");
    ok &= expect_int("route.door_zone", route ? route->door_zone : -1,
                     door_zone, "DEFS.H D1 door zone");
    ok &= expect_int("route.top_track", route ? route->top_track_zone : -1,
                     top_track, "DEFS.H:4091-4093");
    ok &= expect_int("route.rear_order", route ? route->rear_order : -1,
                     rear_order, "DEFS.H:2661-2667");
    ok &= expect_int("route.front_order", route ? route->front_order : -1,
                     front_order, "DEFS.H:2661-2667");
    ok &= expect_int("route.corridor_order",
                     route ? route->corridor_order : -1, corridor_order,
                     "DUNVIEW.C:7523/7691");
    ok &= expect_int("route.dispatch_line",
                     route ? route->f0128_dispatch_line : -1, dispatch_line,
                     "DUNVIEW.C:8525/8529");
    ok &= expect_int("route.start_line", route ? route->f012x_start_line : -1,
                     start_line, "DUNVIEW.C:7492/7660");
    ok &= expect_int("route.f0111_line", route ? route->f012x_f0111_line : -1,
                     f0111_line, "DUNVIEW.C:7506/7674");
    ok &= expect_int("route.lineage_line",
                     route ? route->lineage_start_line : -1, lineage_line,
                     "Viewport.cpp:1892/1919");
    return ok;
}

static int test_lineage_constants(
    const CSB_V1_ViewportD1LD1RF0111Route *route,
    int is_right)
{
    int ok = 1;
    const int rear_order = is_right ? 0x0018 : 0x0028;
    const int top_rect = is_right ? 22 : 21;
    const int record = is_right ? 32 : 31;
    const int state = is_right ? 42 : 41;
    const int door_rects = is_right ? 12 : 11;
    const int front_order = is_right ? 0x0049 : 0x0039;

    ok &= expect_int("lineage.rear_order",
                     route ? route->lineage_rear_order : -1, rear_order,
                     "Viewport.cpp:1895/1922");
    ok &= expect_int("lineage.top_rect",
                     route ? route->lineage_top_track_rect : -1, top_rect,
                     "Viewport.cpp:1896/1923");
    ok &= expect_int("lineage.door_record",
                     route ? route->lineage_door_record : -1, record,
                     "Viewport.cpp:1897/1924");
    ok &= expect_int("lineage.door_state",
                     route ? route->lineage_door_state : -1, state,
                     "Viewport.cpp:1897/1924");
    ok &= expect_int("lineage.door_graphics",
                     route ? route->lineage_door_graphics : -1, 1,
                     "Viewport.cpp:1897/1924 StdDoorGraphicsF1");
    ok &= expect_int("lineage.door_rects",
                     route ? route->lineage_door_rects : -1, door_rects,
                     "Viewport.cpp:1897/1924 StdDoorRectsF1L1/F1R1");
    ok &= expect_int("lineage.front_order",
                     route ? route->lineage_front_order : -1, front_order,
                     "Viewport.cpp:1899/1926");
    ok &= expect_contains("route.redmcsb.dispatch",
                          route ? route->redmcsb_dispatch : NULL,
                          is_right ? "7660-7676" : "7492-7508",
                          "DUNVIEW.C F0122/F0123");
    ok &= expect_contains("route.lineage.dispatch",
                          route ? route->lineage_dispatch : NULL,
                          is_right ? "1919-1927" : "1892-1900",
                          "Viewport.cpp D1 side door-facing");
    return ok;
}

static int test_steps(size_t route_index, int is_right)
{
    int ok = 1;
    const int view_square = is_right ? 5 : 4;
    const int floor_view = is_right ? 596 : 594;
    const int rear_order = is_right ? 0x0018 : 0x0028;
    const int top_track = is_right ? 734 : 732;
    const int door_zone = is_right ? 3800 : 3780;
    const int door_rects = is_right ? 12 : 11;
    const int front_order = is_right ? 0x0049 : 0x0039;
    const int lines[5] = {
        is_right ? 7661 : 7493,
        is_right ? 7662 : 7494,
        is_right ? 7671 : 7503,
        is_right ? 7674 : 7506,
        is_right ? 7704 : 7536
    };

    for (size_t i = 0; i < 5; ++i) {
        const CSB_V1_ViewportD1LD1RF0111Step *step =
            csb_v1_viewport_d1l_d1r_f0111_door_pc34_step_at(route_index, i);
        ok &= expect_int("step.kind", step ? (int)step->kind : -1, (int)i,
                         "DUNVIEW.C D1 door-front composition order");
        ok &= expect_int("step.view_square", step ? step->view_square : -1,
                         view_square, "DEFS.H:2596-2611");
        ok &= expect_int("step.transparent_color",
                         step ? step->transparent_color : -1, 10,
                         "DEFS.H:2088");
        ok &= expect_int("step.source_line", step ? step->source_line : -1,
                         lines[i], "DUNVIEW.C:7493-7508/7661-7676");
        ok &= expect_contains("step.name", step ? step->name : NULL,
                              i == 3 ? "F0111" : "F0",
                              "DUNVIEW.C composition call");
    }

    ok &= expect_int("step.floor.view",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_step_at(
                         route_index, 0)
                         ->view_floor,
                     floor_view, "DUNVIEW.C:7493/7661");
    ok &= expect_int("step.rear.order",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_step_at(
                         route_index, 1)
                         ->order,
                     rear_order, "DUNVIEW.C:7494/7662");
    ok &= expect_int("step.top.zone",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_step_at(
                         route_index, 2)
                         ->zone,
                     top_track, "DUNVIEW.C:7503/7671");
    ok &= expect_int("step.door.zone",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_step_at(
                         route_index, 3)
                         ->zone,
                     door_zone, "DUNVIEW.C:7506/7674");
    ok &= expect_int("step.door.graphics",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_step_at(
                         route_index, 3)
                         ->door_graphics,
                     1, "Viewport.cpp:1897/1924 StdDoorGraphicsF1");
    ok &= expect_int("step.door.rects",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_step_at(
                         route_index, 3)
                         ->door_rects,
                     door_rects, "Viewport.cpp:1897/1924 StdDoorRectsF1L1/F1R1");
    ok &= expect_int("step.front.order",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_step_at(
                         route_index, 4)
                         ->order,
                     front_order, "DUNVIEW.C:7536/7704");
    return ok;
}

static int test_door_state_math(
    const CSB_V1_ViewportD1LD1RF0111Route *route,
    int is_right)
{
    int ok = 1;
    const int base = is_right ? 3800 : 3780;

    ok &= expect_int("state.open.frame",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_frame_index_for_state(
                         0),
                     -1, "DUNVIEW.C:4248-4253");
    ok &= expect_int("state.one.frame",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_frame_index_for_state(
                         1),
                     0, "DUNVIEW.C:4308-4313");
    ok &= expect_int("state.two.frame",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_frame_index_for_state(
                         2),
                     1, "DUNVIEW.C:4308-4313");
    ok &= expect_int("state.three.frame",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_frame_index_for_state(
                         3),
                     2, "DUNVIEW.C:4308-4313");
    ok &= expect_int("state.closed.frame",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_frame_index_for_state(
                         4),
                     4, "DUNVIEW.C:4297-4299");
    ok &= expect_int("state.destroyed.frame",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_frame_index_for_state(
                         5),
                     5, "DUNVIEW.C:4301-4304");
    ok &= expect_int("state.bad.frame",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_frame_index_for_state(
                         6),
                     -1, "DUNVIEW.C F0111 guard");
    ok &= expect_int("state.open.zone",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_zone_for_state(
                         route, 0),
                     -1, "DUNVIEW.C:4248-4253");
    ok &= expect_int("state.one.zone",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_zone_for_state(
                         route, 1),
                     base + 1, "DUNVIEW.C:4317-4319");
    ok &= expect_int("state.two.zone",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_zone_for_state(
                         route, 2),
                     base + 2, "DUNVIEW.C:4317-4319");
    ok &= expect_int("state.three.zone",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_zone_for_state(
                         route, 3),
                     base + 3, "DUNVIEW.C:4317-4319");
    ok &= expect_int("state.closed.zone",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_zone_for_state(
                         route, 4),
                     base, "DUNVIEW.C:4297-4299");
    ok &= expect_int("state.destroyed.zone",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_zone_for_state(
                         route, 5),
                     base, "DUNVIEW.C:4301-4304");
    ok &= expect_int("state.bad.zone",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_zone_for_state(
                         route, 6),
                     -1, "DUNVIEW.C F0111 guard");
    ok &= expect_int("state.null.zone",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_zone_for_state(
                         NULL, 2),
                     -1, "DUNVIEW.C F0111 guard");
    return ok;
}

static int test_horizontal_zone_and_blit(
    const CSB_V1_ViewportD1LD1RF0111Route *route,
    int is_right)
{
    int ok = 1;
    const int base = is_right ? 3800 : 3780;

    ok &= expect_int("horizontal.left.state2",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_horizontal_zone(
                         route, 2, 0),
                     base + 8, "DUNVIEW.C:4322 C6_UNKNOWN");
    ok &= expect_int("horizontal.right.state2",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_horizontal_zone(
                         route, 2, 1),
                     base + 2 + (3 | 0x4000), "DUNVIEW.C:4325 MASK0x4000");
    ok &= expect_int("horizontal.open.reject",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_horizontal_zone(
                         route, 0, 0),
                     -1, "DUNVIEW.C:4248-4253");
    ok &= expect_int("horizontal.closed.reject",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_horizontal_zone(
                         route, 4, 1),
                     -1, "DUNVIEW.C:4317-4327");
    ok &= expect_int("horizontal.null.reject",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_horizontal_zone(
                         NULL, 2, 1),
                     -1, "DUNVIEW.C:4317-4327");
    return ok;
}

static int test_real_graphics_dat_d1lr_door_receipt(void)
{
    int ok = 1;
    const char *path = getenv("FIRESTAFF_CSB_GRAPHICS_DAT");
    const CSB_V1_ViewportD1LD1RF0111Route *d1l =
        csb_v1_viewport_d1l_d1r_f0111_door_pc34_route_for_square(4);
    const CSB_V1_ViewportD1LD1RF0111Route *d1r =
        csb_v1_viewport_d1l_d1r_f0111_door_pc34_route_for_square(5);
    CSB_V1_ViewportD1LD1RF0111RealAssetReceiptPc34 receipt;
    size_t payload_size = 0u;
    uint32_t payload_hash = 0u;

    if (!path || !path[0]) {
        path = "/Users/bosse/.firestaff/data/csb/GRAPHICS.DAT";
    }

    ok &= expect_int("real.hash.read",
                     read_real_graphics_item_hash(path, 248u,
                                                  &payload_size, &payload_hash),
                     1, "DMCSB1 real GRAPHICS.DAT D1 DoorSet 0 item 248");
    ok &= expect_int("real.payload.nonzero", payload_size > 0u, 1,
                     "DMCSB1 real GRAPHICS.DAT D1 DoorSet 0 item 248");
    ok &= expect_int("real.hash.nonzero", payload_hash != 0u, 1,
                     "DMCSB1 real GRAPHICS.DAT D1 DoorSet 0 item 248");
    ok &= expect_int("real.receipt.ok",
        csb_v1_viewport_d1l_d1r_f0111_door_real_asset_receipt_pc34(
            d1l, d1r, 1, 1, 1, 248, payload_size, payload_hash, &receipt),
        1, "D1L/D1R StdDoorGraphicsF1 real receipt");
    ok &= expect_int("real.receipt.valid", receipt.valid, 1,
                     "D1L/D1R StdDoorGraphicsF1 real receipt");
    ok &= expect_int("real.receipt.item", receipt.source_graphics_item_index,
                     248, "D1L/D1R G0695 real receipt");
    ok &= expect_int("real.receipt.hash",
                     receipt.source_payload_hash == payload_hash, 1,
                     "D1L/D1R StdDoorGraphicsF1 real receipt");
    ok &= expect_int("real.receipt.d1l_square", receipt.d1l_view_square, 4,
                     "DUNVIEW.C:8525");
    ok &= expect_int("real.receipt.d1r_square", receipt.d1r_view_square, 5,
                     "DUNVIEW.C:8529");
    ok &= expect_int("real.receipt.d1l_zone", receipt.d1l_door_zone, 3780,
                     "DUNVIEW.C:7506");
    ok &= expect_int("real.receipt.d1r_zone", receipt.d1r_door_zone, 3800,
                     "DUNVIEW.C:7674");
    ok &= expect_int("real.receipt.std_graphics",
                     receipt.standard_door_graphics_f1, 1,
                     "Viewport.cpp:1897/1924 StdDoorGraphicsF1");
    ok &= expect_int("real.receipt.c10", receipt.c10_transparency, 10,
                     "DEFS.H:2088");

    ok &= expect_int("real.reject.no_source",
        csb_v1_viewport_d1l_d1r_f0111_door_real_asset_receipt_pc34(
            d1l, d1r, 0, 1, 1, 248, payload_size, payload_hash, &receipt),
        0, "fail closed without source");
    ok &= expect_int("real.reject.d2_item247",
        csb_v1_viewport_d1l_d1r_f0111_door_real_asset_receipt_pc34(
            d1l, d1r, 1, 1, 1, 247, payload_size, payload_hash, &receipt),
        0, "fail closed wrong item");
    ok &= expect_int("real.reject.next_set_d3_item249",
        csb_v1_viewport_d1l_d1r_f0111_door_real_asset_receipt_pc34(
            d1l, d1r, 1, 1, 1, 249, payload_size, payload_hash, &receipt),
        0, "fail closed wrong item");
    ok &= expect_int("real.reject.synthetic",
        csb_v1_viewport_d1l_d1r_f0111_door_real_asset_receipt_pc34(
            d1l, d1r, 1, 0, 1, 248, payload_size, payload_hash, &receipt),
        0, "fail closed synthetic");
    ok &= expect_int("real.reject.fallback",
        csb_v1_viewport_d1l_d1r_f0111_door_real_asset_receipt_pc34(
            d1l, d1r, 1, 1, 0, 248, payload_size, payload_hash, &receipt),
        0, "fail closed fallback");
    ok &= expect_int("real.reject.zero_hash",
        csb_v1_viewport_d1l_d1r_f0111_door_real_asset_receipt_pc34(
            d1l, d1r, 1, 1, 1, 248, payload_size, 0u, &receipt),
        0, "fail closed zero hash");
    ok &= expect_int("real.reject.same_route_twice",
        csb_v1_viewport_d1l_d1r_f0111_door_real_asset_receipt_pc34(
            d1l, d1l, 1, 1, 1, 248, payload_size, payload_hash, &receipt),
        0, "fail closed without both D1 side routes");

    return ok;
}

static int test_evidence_strings(void)
{
    int ok = 1;
    const CSB_V1_ViewportD1LD1RF0111Evidence *ev =
        csb_v1_viewport_d1l_d1r_f0111_door_pc34_evidence();
    const char *header =
        csb_v1_viewport_d1l_d1r_f0111_door_pc34_source_lock_header();

    ok &= expect_contains("header.f0111", header, "F0111 lines 4218-4337",
                          "DUNVIEW.C F0111");
    ok &= expect_contains("header.f0128", header, "F0128 lines 8318-8542",
                          "DUNVIEW.C F0128");
    ok &= expect_contains("header.f0104", header, "F0104 lines 3113-3156",
                          "DUNVIEW.C F0104");
    ok &= expect_contains("header.f0105", header, "F0105 lines 3185-3225",
                          "DUNVIEW.C F0105");
    ok &= expect_contains("header.f0107", header, "F0107 lines 3502-3590",
                          "DUNVIEW.C F0107");
    ok &= expect_contains("header.f0108", header, "F0108 lines 3940-4011",
                          "DUNVIEW.C F0108");
    ok &= expect_contains("header.f0163", header, "F0163 lines 1769-1838",
                          "DUNGEON.C F0163");
    ok &= expect_contains("header.f0164", header, "F0164 lines 1840-1878",
                          "DUNGEON.C F0164");
    ok &= expect_contains("header.f0172", header, "F0172 lines 2466-2621",
                          "DUNGEON.C F0172");
    ok &= expect_contains("header.defs", header, "DEFS.H lines 2088",
                          "DEFS.H anchors");
    ok &= expect_contains("header.viewport", header, "Viewport.cpp lines 1192-1209",
                          "Viewport.cpp anchors");
    ok &= expect_contains("header.item248", header, "GRAPHICS.DAT item 248",
                          "real item receipt");
    ok &= expect_contains("evidence.scope", ev ? ev->scope : NULL,
                          "real GRAPHICS.DAT", "contract scope");
    ok &= expect_contains("evidence.scope.no_fallback", ev ? ev->scope : NULL,
                          "fallback visuals", "contract scope");
    ok &= expect_contains("evidence.f0111", ev ? ev->redmcsb_dunview_f0111 : NULL,
                          "MASK 0x4000", "DUNVIEW.C:4325");
    ok &= expect_contains("evidence.dispatch",
                          ev ? ev->redmcsb_dunview_f0128 : NULL, "D1R line 8529",
                          "DUNVIEW.C:8529");
    ok &= expect_contains("evidence.wall_callers",
                          ev ? ev->redmcsb_wall_callers : NULL, "F0107",
                          "DUNVIEW.C:3502");
    ok &= expect_contains("evidence.dungeon",
                          ev ? ev->redmcsb_dungeon_zone_math : NULL, "F0172",
                          "DUNGEON.C:2466");
    ok &= expect_contains("evidence.defs", ev ? ev->redmcsb_defs : NULL,
                          "2596-2611", "DEFS.H:2596-2611");
    ok &= expect_contains("evidence.lineage",
                          ev ? ev->csb_lineage_viewport : NULL, "6507-6548",
                          "Viewport.cpp:6507-6548");
    return ok;
}

int main(void)
{
    int ok = 1;
    const CSB_V1_ViewportD1LD1RF0111Route *d1l =
        csb_v1_viewport_d1l_d1r_f0111_door_pc34_route_for_square(4);
    const CSB_V1_ViewportD1LD1RF0111Route *d1r =
        csb_v1_viewport_d1l_d1r_f0111_door_pc34_route_for_square(5);

    printf("probe=csb_v1_viewport_d1l_d1r_f0111_door_pc34_compat\n");
    printf("source_lock=%s\n",
           csb_v1_viewport_d1l_d1r_f0111_door_pc34_source_lock_header());

    ok &= test_route_identity();
    ok &= test_route_constants(d1l, 0);
    ok &= test_route_constants(d1r, 1);
    ok &= test_lineage_constants(d1l, 0);
    ok &= test_lineage_constants(d1r, 1);
    ok &= test_steps(0, 0);
    ok &= test_steps(1, 1);
    ok &= test_door_state_math(d1l, 0);
    ok &= test_door_state_math(d1r, 1);
    ok &= test_horizontal_zone_and_blit(d1l, 0);
    ok &= test_horizontal_zone_and_blit(d1r, 1);
    ok &= test_real_graphics_dat_d1lr_door_receipt();
    ok &= test_evidence_strings();

    ok &= expect_int("assertion_count_at_least_115", g_assertions >= 115, 1,
                     "pass738 scope");

    printf("assertions=%d\n", g_assertions);
    printf("deterministic_hash=0x%08x\n", g_hash);
    printf("%s csb_v1_viewport_d1l_d1r_f0111_door_pc34_compat failures=%d\n",
           ok ? "PASS" : "FAIL", ok ? 0 : 1);
    return ok ? 0 : 1;
}
