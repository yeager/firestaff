#include "csb/csb_v1_viewport_d1c_f0111_door_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *A_D1C_CALL =
    "ReDMCSB DUNVIEW.C:F0124_DUNGEONVIEW_DrawSquareD1C:7905-7908";
static const char *A_DEFS =
    "ReDMCSB DEFS.H:M075_BITMAP_BYTE_COUNT:2159; "
    "DEFS.H:C2_VIEW_DOOR_ORNAMENT_D1LCR:2791; "
    "DEFS.H:M631_ZONE_DOOR_D1C:4259";
static const char *A_F0124_ORDER =
    "ReDMCSB DUNVIEW.C:F0124_DUNGEONVIEW_DrawSquareD1C:"
    "7784-7872,7873-7911,7937-7937";
static const char *A_F0128 =
    "ReDMCSB DUNVIEW.C:F0128_DUNGEONVIEW_Draw_CPSF:8524-8533";
static const char *A_LINEAGE =
    "CSB-lineage Viewport.cpp:StdDrawF1DoorFacing:1903-1915";

static int g_assertions = 0;
static int g_failures = 0;

static int expect_int(const char *label, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want, anchor);
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_contains(const char *label, const char *haystack,
                           const char *needle, const char *anchor)
{
    return expect_int(label, haystack && needle &&
                         strstr(haystack, needle) != NULL, 1, anchor);
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

static int test_contract_identity(void)
{
    int ok = 1;
    const CSB_V1_ViewportD1CF0111DoorPc34Contract *c =
        csb_v1_viewport_d1c_f0111_door_pc34_contract();

    ok &= expect_int("contract.non_null", c != NULL, 1, A_D1C_CALL);
    ok &= expect_int("contract.only", c ? c->contract_only : 0, 1, A_D1C_CALL);
    ok &= expect_int("view_square.d1c", c ? c->view_square_d1c : -1, 3, A_DEFS);
    ok &= expect_int("view_depth.d1", c ? c->view_depth : -1, 1,
                     "ReDMCSB DUNVIEW.C:G2027_ac_ViewSquareIndexToViewDepth:372-372");
    ok &= expect_int("view_lane.center", c ? c->view_lane : -9, 0,
                     "ReDMCSB DUNVIEW.C:G2026_ac_ViewSquareIndexToViewLane:371-371");
    ok &= expect_int("element.door_front", c ? c->element_door_front : -1, 17,
                     "ReDMCSB DUNVIEW.C:F0124_DUNGEONVIEW_DrawSquareD1C:7873-7873");

    return ok;
}

static int test_d1c_f0111_call_contract(void)
{
    int ok = 1;
    const CSB_V1_ViewportD1CF0111DoorPc34Contract *c =
        csb_v1_viewport_d1c_f0111_door_pc34_contract();

    ok &= expect_contains("bitmap.index.symbol", c ? c->door_bitmap_index_symbol : NULL,
                          "G0695_ai_DoorNativeBitmapIndex_Front_D1LCR",
                          A_D1C_CALL);
    ok &= expect_int("native.width", c ? c->door_native_width : -1, 96,
                     A_D1C_CALL);
    ok &= expect_int("native.height", c ? c->door_native_height : -1, 88,
                     A_D1C_CALL);
    ok &= expect_int("native.byte_count", c ? c->door_native_byte_count : -1,
                     4224, A_DEFS);
    ok &= expect_contains("byte.count.macro", c ? c->door_byte_count_macro : NULL,
                          "M075_BITMAP_BYTE_COUNT(96, 88)", A_D1C_CALL);
    ok &= expect_int("view.ornament.value",
                     c ? c->view_door_ornament_d1lcr : -1, 2, A_DEFS);
    ok &= expect_contains("view.ornament.symbol", c ? c->door_view_symbol : NULL,
                          "C2_VIEW_DOOR_ORNAMENT_D1LCR", A_D1C_CALL);
    ok &= expect_contains("frame.symbol", c ? c->door_frame_symbol : NULL,
                          "G0186_s_Graphic558_Frames_Door_D1C", A_D1C_CALL);
    ok &= expect_int("zone.d1c", c ? c->door_zone_d1c : -1, 3790, A_DEFS);

    return ok;
}

static int test_order_and_non_route_contract(void)
{
    int ok = 1;
    const CSB_V1_ViewportD1CF0111DoorPc34Contract *c =
        csb_v1_viewport_d1c_f0111_door_pc34_contract();

    ok &= expect_int("doorpass1.order", c ? c->doorpass1_order : -1, 0x0218,
                     A_F0124_ORDER);
    ok &= expect_int("doorpass2.order", c ? c->doorpass2_order : -1, 0x0349,
                     A_F0124_ORDER);
    ok &= expect_int("wall.precedes.door",
                     c ? c->f0124_wall_case_precedes_door_case : 0, 1,
                     A_F0124_ORDER);
    ok &= expect_int("door.precedes.terminal_f0115",
                     c ? c->f0124_door_precedes_terminal_f0115 : 0, 1,
                     A_F0124_ORDER);
    ok &= expect_contains("order.anchor.wall", c ? c->redmcsb_f0124_order_anchor : NULL,
                          "7784-7872", A_F0124_ORDER);
    ok &= expect_contains("order.anchor.door", c ? c->redmcsb_f0124_order_anchor : NULL,
                          "7873-7911", A_F0124_ORDER);
    ok &= expect_contains("order.anchor.things", c ? c->redmcsb_f0124_order_anchor : NULL,
                          "7937-7937", A_F0124_ORDER);
    ok &= expect_int("f0128.after.d1l_d1r", c ? c->f0128_dispatch_after_d1l_d1r : 0,
                     1, A_F0128);
    ok &= expect_int("f0128.dispatches.d1c", c ? c->f0128_dispatches_d1c : 0,
                     1, A_F0128);
    ok &= expect_int("not.f0122.d1l", c ? c->uses_f0122_d1l : 1, 0,
                     A_F0128);
    ok &= expect_int("not.f0123.d1r", c ? c->uses_f0123_d1r : 1, 0,
                     A_F0128);

    return ok;
}

static int test_real_graphics_dat_d1c_door_receipt(void)
{
    int ok = 1;
    const char *path = getenv("FIRESTAFF_CSB_GRAPHICS_DAT");
    const CSB_V1_ViewportD1CF0111DoorPc34Contract *c =
        csb_v1_viewport_d1c_f0111_door_pc34_contract();
    CSB_V1_ViewportD1CF0111DoorRealAssetReceiptPc34 receipt;
    size_t payload_size = 0u;
    uint32_t payload_hash = 0u;

    if (!path || !path[0]) {
        path = "/Users/bosse/.firestaff/data/csb/GRAPHICS.DAT";
    }

    ok &= expect_int("real.hash.read",
                     read_real_graphics_item_hash(path, 248u,
                                                  &payload_size, &payload_hash),
                     1, "DMCSB1 real GRAPHICS.DAT item 248");
    ok &= expect_int("real.payload.nonzero", payload_size > 0u, 1,
                     "DMCSB1 real GRAPHICS.DAT item 248");
    ok &= expect_int("real.hash.nonzero", payload_hash != 0u, 1,
                     "DMCSB1 real GRAPHICS.DAT item 248");
    ok &= expect_int("real.receipt.ok",
        csb_v1_viewport_d1c_f0111_door_real_asset_receipt_pc34(
            c, 1, 1, 1, 248, payload_size, payload_hash, &receipt),
        1, A_D1C_CALL);
    ok &= expect_int("real.receipt.valid", receipt.valid, 1, A_D1C_CALL);
    ok &= expect_int("real.receipt.item", receipt.source_graphics_item_index,
                     248, A_D1C_CALL);
    ok &= expect_int("real.receipt.hash", receipt.source_payload_hash == payload_hash,
                     1, A_D1C_CALL);
    ok &= expect_int("real.receipt.width", receipt.door_native_width, 96,
                     A_D1C_CALL);
    ok &= expect_int("real.receipt.height", receipt.door_native_height, 88,
                     A_D1C_CALL);
    ok &= expect_int("real.receipt.byte_count", receipt.door_native_byte_count,
                     4224, A_DEFS);
    ok &= expect_int("real.receipt.zone", receipt.door_zone_d1c, 3790, A_DEFS);
    ok &= expect_int("real.receipt.no_synthetic", receipt.no_synthetic_pixels,
                     1, A_D1C_CALL);
    ok &= expect_int("real.receipt.no_fallback", receipt.no_fallback_visuals,
                     1, A_D1C_CALL);

    ok &= expect_int("real.reject.no_source",
        csb_v1_viewport_d1c_f0111_door_real_asset_receipt_pc34(
            c, 0, 1, 1, 248, payload_size, payload_hash, &receipt),
        0, A_D1C_CALL);
    ok &= expect_int("real.reject.item557",
        csb_v1_viewport_d1c_f0111_door_real_asset_receipt_pc34(
            c, 1, 1, 1, 557, payload_size, payload_hash, &receipt),
        0, A_D1C_CALL);
    ok &= expect_int("real.reject.item559",
        csb_v1_viewport_d1c_f0111_door_real_asset_receipt_pc34(
            c, 1, 1, 1, 559, payload_size, payload_hash, &receipt),
        0, A_D1C_CALL);
    ok &= expect_int("real.reject.synthetic",
        csb_v1_viewport_d1c_f0111_door_real_asset_receipt_pc34(
            c, 1, 0, 1, 248, payload_size, payload_hash, &receipt),
        0, A_D1C_CALL);
    ok &= expect_int("real.reject.fallback",
        csb_v1_viewport_d1c_f0111_door_real_asset_receipt_pc34(
            c, 1, 1, 0, 248, payload_size, payload_hash, &receipt),
        0, A_D1C_CALL);
    ok &= expect_int("real.reject.zero_hash",
        csb_v1_viewport_d1c_f0111_door_real_asset_receipt_pc34(
            c, 1, 1, 1, 248, payload_size, 0u, &receipt),
        0, A_D1C_CALL);

    return ok;
}

static int test_evidence_strings(void)
{
    int ok = 1;
    const CSB_V1_ViewportD1CF0111DoorPc34Contract *c =
        csb_v1_viewport_d1c_f0111_door_pc34_contract();
    const char *e = csb_v1_viewport_d1c_f0111_door_pc34_source_evidence();

    ok &= expect_contains("call.anchor", c ? c->redmcsb_d1c_call_anchor : NULL,
                          "7905-7908", A_D1C_CALL);
    ok &= expect_contains("defs.anchor.byte_count", c ? c->redmcsb_defs_anchor : NULL,
                          "M075_BITMAP_BYTE_COUNT:2159", A_DEFS);
    ok &= expect_contains("defs.anchor.ornament", c ? c->redmcsb_defs_anchor : NULL,
                          "C2_VIEW_DOOR_ORNAMENT_D1LCR:2791", A_DEFS);
    ok &= expect_contains("defs.anchor.zone", c ? c->redmcsb_defs_anchor : NULL,
                          "M631_ZONE_DOOR_D1C:4259", A_DEFS);
    ok &= expect_contains("f0128.anchor", c ? c->redmcsb_f0128_dispatch_anchor : NULL,
                          "8524-8533", A_F0128);
    ok &= expect_contains("lineage.anchor",
                          c ? c->csb_lineage_viewport_anchor : NULL,
                          "StdDrawF1DoorFacing:1903-1915", A_LINEAGE);
    ok &= expect_contains("evidence.contract", e, "contract_only=1", A_D1C_CALL);
    ok &= expect_contains("evidence.real_asset", e, "real-asset receipt",
                          A_D1C_CALL);
    ok &= expect_contains("evidence.item248", e, "GRAPHICS.DAT DoorSet-0 item 248",
                          A_D1C_CALL);
    ok &= expect_contains("evidence.bitmap", e,
                          "G0695_ai_DoorNativeBitmapIndex_Front_D1LCR",
                          A_D1C_CALL);
    ok &= expect_contains("evidence.byte_count", e,
                          "M075_BITMAP_BYTE_COUNT(96, 88)", A_D1C_CALL);
    ok &= expect_contains("evidence.view", e, "C2_VIEW_DOOR_ORNAMENT_D1LCR",
                          A_D1C_CALL);
    ok &= expect_contains("evidence.frame", e,
                          "G0186_s_Graphic558_Frames_Door_D1C", A_D1C_CALL);
    ok &= expect_contains("evidence.f0122.excluded", e,
                          "does not use F0122_DUNGEONVIEW_DrawSquareD1L",
                          A_F0128);
    ok &= expect_contains("evidence.f0123.excluded", e,
                          "F0123_DUNGEONVIEW_DrawSquareD1R", A_F0128);
    ok &= expect_contains("evidence.lineage", e, "Viewport.cpp:StdDrawF1DoorFacing",
                          A_LINEAGE);

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d1c_f0111_door_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d1c_f0111_door_pc34_source_evidence());

    ok &= test_contract_identity();
    ok &= test_d1c_f0111_call_contract();
    ok &= test_order_and_non_route_contract();
    ok &= test_real_graphics_dat_d1c_door_receipt();
    ok &= test_evidence_strings();
    ok &= expect_int("assertion_count_at_least_50", g_assertions >= 50, 1,
                     A_D1C_CALL);

    printf("assertions=%d failures=%d\n", g_assertions, g_failures);
    if (ok && g_failures == 0) {
        printf("PASS csb_v1_viewport_d1c_f0111_door_pc34_compat assertions=%d failures=0\n",
               g_assertions);
    }

    return (ok && g_failures == 0) ? 0 : 1;
}
