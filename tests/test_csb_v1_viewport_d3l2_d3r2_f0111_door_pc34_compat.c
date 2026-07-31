#include "csb_v1_viewport_d3l2_d3r2_f0111_door_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *A_BOTH =
    "ReDMCSB DUNVIEW.C:6268-6274 F0676 and 6336-6341 F0677; "
    "DEFS.H:2088,2610-2611,2668-2675,2750-2751,2789,4250-4251,5456; "
    "CSB Viewport.cpp:1813-1820,2267/2271,2281,2386/2387,2568,2596-2616";
static const char *A_D3L2 =
    "ReDMCSB DUNVIEW.C:6268-6274 F0676; "
    "DEFS.H:2088,2610,2669,2672,2750,2789,4250,5456; "
    "CSB Viewport.cpp:1813-1820,2267,2281,2386,2568,2596-2616";
static const char *A_D3R2 =
    "ReDMCSB DUNVIEW.C:6336-6341 F0677; "
    "DEFS.H:2088,2611,2668,2675,2751,2789,4251,5456; "
    "CSB Viewport.cpp:1813-1820,2271,2281,2387,2568,2596-2616";
static const char *A_F0111 =
    "ReDMCSB DUNVIEW.C:4218-4334 F0111 plus F0676/F0677 door-front calls; "
    "DEFS.H:2088,2610-2611,2668-2675,2789,4250-4251,5456; "
    "CSB Viewport.cpp:1813-1820,2281,2386/2387,2568,2596-2616";

static int g_assertions = 0;

static int expect_int(const char *label, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want, anchor);
        return 0;
    }
    printf("ok %s=%d anchor=%s\n", label, got, anchor);
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

static int test_specs_and_f0108_routes(void)
{
    int ok = 1;
    const CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34 *d3l2 =
        csb_v1_viewport_d3l2_d3r2_f0111_door_for_side_pc34(
            CSB_V1_VIEWPORT_D3L2_D3R2_F0111_DOOR_SIDE_D3L2_PC34);
    const CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34 *d3r2 =
        csb_v1_viewport_d3l2_d3r2_f0111_door_for_side_pc34(
            CSB_V1_VIEWPORT_D3L2_D3R2_F0111_DOOR_SIDE_D3R2_PC34);

    ok &= expect_int("spec.count",
                     (int)csb_v1_viewport_d3l2_d3r2_f0111_door_count_pc34(),
                     2, A_BOTH);
    ok &= expect_int("spec.index0.d3l2",
                     csb_v1_viewport_d3l2_d3r2_f0111_door_at_pc34(0) == d3l2,
                     1, A_D3L2);
    ok &= expect_int("spec.index2.null",
                     csb_v1_viewport_d3l2_d3r2_f0111_door_at_pc34(2) == NULL,
                     1, A_BOTH);
    ok &= expect_int("d3l2.present", d3l2 != NULL, 1, A_D3L2);
    ok &= expect_int("d3r2.present", d3r2 != NULL, 1, A_D3R2);
    ok &= expect_int("d3l2.element", d3l2 ? d3l2->element_door_front : -1,
                     17, A_D3L2);
    ok &= expect_int("d3r2.element", d3r2 ? d3r2->element_door_front : -1,
                     17, A_D3R2);
    ok &= expect_int("d3l2.f0108.floor_view",
                     d3l2 ? d3l2->floor_view : -1, 0, A_D3L2);
    ok &= expect_contains("d3l2.f0108.ordinal",
                          d3l2 ? d3l2->floor_ornament_ordinal_slot : NULL,
                          "M552_FRONT_WALL_ORNAMENT_ORDINAL", A_D3L2);
    ok &= expect_int("d3r2.f0108.floor_view",
                     d3r2 ? d3r2->floor_view : -1, 1, A_D3R2);
    ok &= expect_contains("d3r2.f0108.ordinal",
                          d3r2 ? d3r2->floor_ornament_ordinal_slot : NULL,
                          "M558_FLOOR_ORNAMENT_ORDINAL", A_D3R2);

    return ok;
}

static int test_map_door_set_selection(void)
{
    int ok = 1;

    ok &= expect_int("map_door_set.db0_type0_d3",
                     csb_v1_viewport_door_graphic_index_from_map_pc34(
                         1, 3, 0x0000u, 0), 249,
                     "DUNGEON.C F0174; DUNVIEW.C F0096:2651-2658");
    ok &= expect_int("map_door_set.db0_type1_d2",
                     csb_v1_viewport_door_graphic_index_from_map_pc34(
                         1, 3, 0x0001u, 1), 256,
                     "DUNGEON.C F0174; CSBWin Viewport.cpp DB0 door type");
    ok &= expect_int("map_door_set.bad_selected_set",
                     csb_v1_viewport_door_graphic_index_from_map_pc34(
                         1, 4, 0x0001u, 2), -1,
                     "PC3.4 DoorSet range 0..3 fails closed");
    ok &= expect_int("map_door_set.bad_depth",
                     csb_v1_viewport_door_graphic_index_from_map_pc34(
                         1, 3, 0x0000u, 3), -1,
                     "G0693/G0694/G0695 only");
    return ok;
}

static int test_f0115_and_f0111_dispatch(void)
{
    int ok = 1;
    CSB_V1_ViewportD3L2D3R2F0111DoorResultPc34 left_result;
    CSB_V1_ViewportD3L2D3R2F0111DoorResultPc34 right_result;
    const CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34 *d3l2 =
        csb_v1_viewport_d3l2_d3r2_f0111_door_for_side_pc34(
            CSB_V1_VIEWPORT_D3L2_D3R2_F0111_DOOR_SIDE_D3L2_PC34);
    const CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34 *d3r2 =
        csb_v1_viewport_d3l2_d3r2_f0111_door_for_side_pc34(
            CSB_V1_VIEWPORT_D3L2_D3R2_F0111_DOOR_SIDE_D3R2_PC34);

    ok &= expect_int("trace.d3l2",
                     csb_v1_viewport_d3l2_d3r2_f0111_door_trace_pc34(
                         d3l2, 2, &left_result),
                     0, A_D3L2);
    ok &= expect_int("trace.d3r2",
                     csb_v1_viewport_d3l2_d3r2_f0111_door_trace_pc34(
                         d3r2, 2, &right_result),
                     0, A_D3R2);
    ok &= expect_int("d3l2.pass1.called", left_result.f0115_pass1_called,
                     1, A_D3L2);
    ok &= expect_int("d3l2.pass1.square", left_result.f0115_pass1_view_square,
                     14, A_D3L2);
    ok &= expect_int("d3l2.pass1.order", left_result.f0115_pass1_order,
                     0x0218, A_D3L2);
    ok &= expect_int("d3r2.pass1.called", right_result.f0115_pass1_called,
                     1, A_D3R2);
    ok &= expect_int("d3r2.pass1.square", right_result.f0115_pass1_view_square,
                     15, A_D3R2);
    ok &= expect_int("d3r2.pass1.order", right_result.f0115_pass1_order,
                     0x0128, A_D3R2);
    ok &= expect_int("d3l2.f0111.called", left_result.f0111_called, 1,
                     A_D3L2);
    ok &= expect_int("d3r2.f0111.called", right_result.f0111_called, 1,
                     A_D3R2);
    ok &= expect_int("d3l2.f0111.native_index_family",
                     left_result.f0111_native_bitmap_index_family, 246, A_F0111);
    ok &= expect_int("d3r2.f0111.native_index_family",
                     right_result.f0111_native_bitmap_index_family, 246, A_F0111);
    ok &= expect_int("d3l2.f0111.ornament",
                     left_result.f0111_door_ornament_view, 0, A_F0111);
    ok &= expect_int("d3r2.f0111.ornament",
                     right_result.f0111_door_ornament_view, 0, A_F0111);
    ok &= expect_int("d3l2.f0111.zone", left_result.f0111_zone, 3700,
                     A_D3L2);
    ok &= expect_int("d3r2.f0111.zone", right_result.f0111_zone, 3710,
                     A_D3R2);

    return ok;
}

static int test_pass2_c10_frame_and_no_f0107(void)
{
    int ok = 1;
    CSB_V1_ViewportD3L2D3R2F0111DoorResultPc34 left_result;
    CSB_V1_ViewportD3L2D3R2F0111DoorResultPc34 right_result;
    const CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34 *d3l2 =
        csb_v1_viewport_d3l2_d3r2_f0111_door_for_side_pc34(
            CSB_V1_VIEWPORT_D3L2_D3R2_F0111_DOOR_SIDE_D3L2_PC34);
    const CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34 *d3r2 =
        csb_v1_viewport_d3l2_d3r2_f0111_door_for_side_pc34(
            CSB_V1_VIEWPORT_D3L2_D3R2_F0111_DOOR_SIDE_D3R2_PC34);

    ok &= expect_int("trace.d3l2.again",
                     csb_v1_viewport_d3l2_d3r2_f0111_door_trace_pc34(
                         d3l2, 1, &left_result),
                     0, A_D3L2);
    ok &= expect_int("trace.d3r2.again",
                     csb_v1_viewport_d3l2_d3r2_f0111_door_trace_pc34(
                         d3r2, 1, &right_result),
                     0, A_D3R2);
    ok &= expect_int("d3l2.pass2.before_dispatch",
                     left_result.f0115_pass2_order_set_before_dispatch, 1,
                     A_D3L2);
    ok &= expect_int("d3l2.pass2.order", left_result.f0115_pass2_order,
                     0x0349, A_D3L2);
    ok &= expect_int("d3r2.pass2.before_dispatch",
                     right_result.f0115_pass2_order_set_before_dispatch, 1,
                     A_D3R2);
    ok &= expect_int("d3r2.pass2.order", right_result.f0115_pass2_order,
                     0x0439, A_D3R2);
    ok &= expect_int("d3l2.f0128.reached",
                     left_result.f0128_dispatch_reached_after_pass2, 1,
                     A_D3L2);
    ok &= expect_int("d3r2.f0128.reached",
                     right_result.f0128_dispatch_reached_after_pass2, 1,
                     A_D3R2);
    ok &= expect_int("d3l2.c10", left_result.f0111_transparent_color, 10,
                     A_F0111);
    ok &= expect_int("d3r2.c10", right_result.f0111_transparent_color, 10,
                     A_F0111);
    ok &= expect_int("d3l2.native.fetch.f0489",
                     left_result.native_bitmap_fetches_via_f0489, 1, A_F0111);
    ok &= expect_int("d3r2.native.fetch.f0489",
                     right_result.native_bitmap_fetches_via_f0489, 1, A_F0111);
    ok &= expect_int("d3l2.resolved.native.index",
                     left_result.resolved_native_bitmap_index, 247, A_F0111);
    ok &= expect_int("d3r2.resolved.native.index",
                     right_result.resolved_native_bitmap_index, 247, A_F0111);
    ok &= expect_int("d3l2.frame.byte_width",
                     left_result.preserved_frame_byte_width, 44, A_F0111);
    ok &= expect_int("d3r2.frame.byte_width",
                     right_result.preserved_frame_byte_width, 44, A_F0111);
    ok &= expect_int("d3l2.no_f0107", left_result.f0107_called, 0, A_D3L2);
    ok &= expect_int("d3r2.no_f0107", right_result.f0107_called, 0, A_D3R2);

    return ok;
}

static int test_lineage_pixel_and_evidence(void)
{
    int ok = 1;
    uint8_t source[44 * 2];
    uint8_t destination[44 * 2];
    CSB_V1_ViewportD3L2D3R2F0111DoorResultPc34 result;
    const char *e =
        csb_v1_viewport_d3l2_d3r2_f0111_door_source_evidence_pc34();
    const CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34 *d3l2 =
        csb_v1_viewport_d3l2_d3r2_f0111_door_for_side_pc34(
            CSB_V1_VIEWPORT_D3L2_D3R2_F0111_DOOR_SIDE_D3L2_PC34);

    memset(source, 10, sizeof(source));
    memset(destination, 0xee, sizeof(destination));
    source[0] = 1;
    source[47] = 2;
    source[44] = 3;
    source[87] = 4;

    ok &= expect_int("lineage.trace",
                     csb_v1_viewport_d3l2_d3r2_f0111_door_trace_pc34(
                         d3l2, 0, &result),
                     0, A_D3L2);
    ok &= expect_int("lineage.pwall.parity",
                     result.l0201_pwall_parity_preserved, 1, A_BOTH);
    ok &= expect_int("lineage.pwall.left",
                     result.lineage_pwall_left_index, 5, A_BOTH);
    ok &= expect_int("lineage.pwall.right",
                     result.lineage_pwall_right_index, 6, A_BOTH);
    ok &= expect_int("pixel.anchor.ready", result.pixel_anchor_ready, 1,
                     A_F0111);
    ok &= expect_int("pixel.blit.copied",
                     csb_v1_viewport_d3l2_d3r2_f0111_door_apply_c10_blit_pc34(
                         d3l2, source, 44, destination, 44, 44, 2),
                     4, A_F0111);
    ok &= expect_int("pixel.blit.first", destination[0], 1, A_F0111);
    ok &= expect_int("pixel.blit.transparent", destination[1], 0xee, A_F0111);
    ok &= expect_int("pixel.blit.last", destination[87], 4, A_F0111);
    ok &= expect_int("pixel.blit.reject.width",
                     csb_v1_viewport_d3l2_d3r2_f0111_door_apply_c10_blit_pc34(
                         d3l2, source, 44, destination, 44, 45, 1),
                     -1, A_F0111);
    ok &= expect_contains("evidence.path", result.source_lock_evidence,
                          "DUNVIEW.C F0676/F0677 C17_ELEMENT_DOOR_FRONT path",
                          A_BOTH);
    ok &= expect_contains("evidence.f0111", result.source_lock_evidence,
                          "F0111_DUNGEONVIEW_DrawDoor", A_F0111);
    ok &= expect_contains("evidence.global.path", e,
                          "F0111_DUNGEONVIEW_DrawDoor", A_F0111);
    ok &= expect_contains("evidence.no_f0107_contract_source", e,
                          "M552_FRONT_WALL_ORNAMENT_ORDINAL", A_D3L2);
    ok &= expect_contains("evidence.d3r2_floor_ordinal", e,
                          "M558_FLOOR_ORNAMENT_ORDINAL", A_D3R2);

    return ok;
}

static int test_real_graphics_dat_d3lr_door_receipt(void)
{
    int ok = 1;
    const char *path = getenv("FIRESTAFF_CSB_GRAPHICS_DAT");
    const CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34 *d3l2 =
        csb_v1_viewport_d3l2_d3r2_f0111_door_for_side_pc34(
            CSB_V1_VIEWPORT_D3L2_D3R2_F0111_DOOR_SIDE_D3L2_PC34);
    const CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34 *d3r2 =
        csb_v1_viewport_d3l2_d3r2_f0111_door_for_side_pc34(
            CSB_V1_VIEWPORT_D3L2_D3R2_F0111_DOOR_SIDE_D3R2_PC34);
    CSB_V1_ViewportD3L2D3R2F0111DoorRealAssetReceiptPc34 receipt;
    size_t payload_size = 0u;
    uint32_t payload_hash = 0u;

    ok &= expect_int("door_set0.d3.index",
                     csb_v1_viewport_door_graphic_index_pc34(0, 0), 246,
                     A_F0111);
    ok &= expect_int("door_set3.d3.index",
                     csb_v1_viewport_door_graphic_index_pc34(3, 0), 255,
                     A_F0111);
    ok &= expect_int("door_set3.d1.index",
                     csb_v1_viewport_door_graphic_index_pc34(3, 2), 257,
                     A_F0111);
    ok &= expect_int("door_set.reject.out_of_range",
                     csb_v1_viewport_door_graphic_index_pc34(4, 0), -1,
                     A_F0111);
    ok &= expect_int("d3.index.reject.d2",
                     csb_v1_viewport_d3_door_graphic_index_valid_pc34(247), 0,
                     A_F0111);

    if (!path || !path[0]) {
        path = "/Users/bosse/.firestaff/data/csb/GRAPHICS.DAT";
    }

    ok &= expect_int("real.hash.read",
                     read_real_graphics_item_hash(path, 246u,
                                                  &payload_size, &payload_hash),
                     1, "DMCSB1 real GRAPHICS.DAT DoorSet-0 D3 item 246");
    ok &= expect_int("real.payload.nonzero", payload_size > 0u, 1,
                     "DMCSB1 real GRAPHICS.DAT DoorSet-0 D3 item 246");
    ok &= expect_int("real.hash.nonzero", payload_hash != 0u, 1,
                     "DMCSB1 real GRAPHICS.DAT DoorSet-0 D3 item 246");
    ok &= expect_int("real.receipt.ok",
        csb_v1_viewport_d3l2_d3r2_f0111_door_real_asset_receipt_pc34(
            d3l2, d3r2, 1, 1, 1, 246, payload_size, payload_hash, &receipt),
        1, A_F0111);
    ok &= expect_int("real.receipt.valid", receipt.valid, 1, A_F0111);
    ok &= expect_int("real.receipt.item", receipt.source_graphics_item_index,
                     246, A_F0111);
    ok &= expect_int("real.receipt.hash",
                     receipt.source_payload_hash == payload_hash, 1, A_F0111);
    ok &= expect_int("real.receipt.d3l2_square", receipt.d3l2_view_square,
                     14, A_D3L2);
    ok &= expect_int("real.receipt.d3r2_square", receipt.d3r2_view_square,
                     15, A_D3R2);
    ok &= expect_int("real.receipt.d3l2_zone", receipt.d3l2_door_zone, 3700,
                     A_D3L2);
    ok &= expect_int("real.receipt.d3r2_zone", receipt.d3r2_door_zone, 3710,
                     A_D3R2);
    ok &= expect_int("real.receipt.width", receipt.native_bitmap_byte_width,
                     44, A_F0111);
    ok &= expect_int("real.receipt.height", receipt.native_bitmap_height, 38,
                     A_F0111);
    ok &= expect_int("real.receipt.c10", receipt.transparent_color, 10,
                     A_F0111);

    ok &= expect_int("real.reject.no_source",
        csb_v1_viewport_d3l2_d3r2_f0111_door_real_asset_receipt_pc34(
            d3l2, d3r2, 0, 1, 1, 246, payload_size, payload_hash, &receipt),
        0, A_F0111);
    ok &= expect_int("real.reject.item692",
        csb_v1_viewport_d3l2_d3r2_f0111_door_real_asset_receipt_pc34(
            d3l2, d3r2, 1, 1, 1, 247, payload_size, payload_hash, &receipt),
        0, A_F0111);
    ok &= expect_int("real.reject.item694",
        csb_v1_viewport_d3l2_d3r2_f0111_door_real_asset_receipt_pc34(
            d3l2, d3r2, 1, 1, 1, 248, payload_size, payload_hash, &receipt),
        0, A_F0111);
    ok &= expect_int("real.reject.synthetic",
        csb_v1_viewport_d3l2_d3r2_f0111_door_real_asset_receipt_pc34(
            d3l2, d3r2, 1, 0, 1, 246, payload_size, payload_hash, &receipt),
        0, A_F0111);
    ok &= expect_int("real.reject.fallback",
        csb_v1_viewport_d3l2_d3r2_f0111_door_real_asset_receipt_pc34(
            d3l2, d3r2, 1, 1, 0, 246, payload_size, payload_hash, &receipt),
        0, A_F0111);
    ok &= expect_int("real.reject.zero_hash",
        csb_v1_viewport_d3l2_d3r2_f0111_door_real_asset_receipt_pc34(
            d3l2, d3r2, 1, 1, 1, 246, payload_size, 0u, &receipt),
        0, A_F0111);
    ok &= expect_int("real.reject.same_side",
        csb_v1_viewport_d3l2_d3r2_f0111_door_real_asset_receipt_pc34(
            d3l2, d3l2, 1, 1, 1, 246, payload_size, payload_hash, &receipt),
        0, A_F0111);

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d3l2_d3r2_f0111_door_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d3l2_d3r2_f0111_door_source_evidence_pc34());

    ok &= test_specs_and_f0108_routes();
    ok &= test_map_door_set_selection();
    ok &= test_f0115_and_f0111_dispatch();
    ok &= test_pass2_c10_frame_and_no_f0107();
    ok &= test_lineage_pixel_and_evidence();
    ok &= test_real_graphics_dat_d3lr_door_receipt();

    printf("assertions=%d\n", g_assertions);
    ok &= expect_int("assertion_count_at_least_65", g_assertions >= 65, 1,
                     A_BOTH);
    if (ok) {
        printf("PASS csb_v1_viewport_d3l2_d3r2_f0111_door_pc34_compat assertions=%d\n",
               g_assertions);
    }
    return ok ? 0 : 1;
}
