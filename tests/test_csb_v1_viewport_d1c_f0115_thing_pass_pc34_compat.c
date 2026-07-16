#include "firestaff/csb/v1/viewport/d1c_f0115_thing_pass_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *A_F0124 =
    "ReDMCSB DUNVIEW.C F0124:7873-7911,7910-7937 D1C door-front F0115";
static const char *A_F0115_ORDER =
    "ReDMCSB DUNVIEW.C F0115:4547-4581 thing-pass order";
static const char *A_F0115_DOOR =
    "ReDMCSB DUNVIEW.C F0115:4795-4800 MASK 0x0008 door-marker decode";
static const char *A_F0115_ITEM =
    "ReDMCSB DUNVIEW.C F0115:4923 item cell guard";
static const char *A_F0115_OBJECT =
    "ReDMCSB DUNVIEW.C F0115:5180-5188 C10 object blit";
static const char *A_F0115_CREATURE =
    "ReDMCSB DUNVIEW.C F0115:5211-5214 creature row guard";
static const char *A_F0115_PROJECTILE =
    "ReDMCSB DUNVIEW.C F0115:5668-5671 projectile row guard";
static const char *A_DEFS =
    "ReDMCSB DEFS.H:2669/2672 C0x0218/C0x0349 DOORPASS1/2";

static int g_assertions;

static int expect_int(const char *label, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want, anchor);
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_uint(const char *label, unsigned int got, unsigned int want,
                       const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=0x%04x want=0x%04x anchor=%s\n",
               label, got, want, anchor);
        return 0;
    }
    printf("PASS %s=0x%04x anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_contains(const char *label, const char *haystack,
                           const char *needle, const char *anchor)
{
    return expect_int(label,
                      haystack && needle && strstr(haystack, needle) != 0,
                      1, anchor);
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

static int test_contract(void)
{
    int ok = 1;
    const CSB_V1_D1CF0115ThingPassContractPc34 *c =
        csb_v1_viewport_d1c_f0115_thing_pass_contract_pc34();

    ok &= expect_int("contract.present", c != 0, 1, A_F0124);
    ok &= expect_int("contract.contract_only", c ? c->contract_only : 0, 1,
                     A_F0124);
    ok &= expect_int("contract.no_game_data_load", c ? c->no_game_data_load : 0,
                     1, A_F0124);
    ok &= expect_int("contract.no_graphics_dat_load",
                     c ? c->no_graphics_dat_load : 0, 1, A_F0124);
    ok &= expect_int("contract.no_dungeon_dat_load",
                     c ? c->no_dungeon_dat_load : 0, 1, A_F0124);
    ok &= expect_int("contract.no_real_asset_pixels",
                     c ? c->no_real_asset_pixels : 0, 1, A_F0124);
    ok &= expect_int("contract.disjoint_custom_backgrounds",
                     c ? c->disjoint_from_custom_backgrounds : 0, 1, A_F0124);
    ok &= expect_int("contract.disjoint_f0108",
                     c ? c->disjoint_from_f0108_floor_ceiling_ornament : 0, 1,
                     A_F0124);
    ok &= expect_int("contract.disjoint_f0111_partly_open",
                     c ? c->disjoint_from_f0111_partly_open_door : 0, 1,
                     A_F0124);
    ok &= expect_int("contract.d1c_view_square", c ? c->d1c_view_square : -1,
                     3, A_DEFS);
    ok &= expect_int("contract.f0115_call_count",
                     c ? c->f0115_call_count : -1, 2, A_F0124);
    ok &= expect_uint("contract.first_back_pass_order",
                      c ? c->first_back_pass_order : 0u, 0x0218u, A_DEFS);
    ok &= expect_uint("contract.second_front_pass_order",
                      c ? c->second_front_pass_order : 0u, 0x0349u, A_DEFS);
    ok &= expect_uint("contract.door_marker_mask",
                      c ? c->door_marker_mask : 0u, 0x0008u, A_F0115_DOOR);
    ok &= expect_uint("contract.door_marker_nibble_pass1",
                      c ? c->door_marker_nibble_pass1 : 0u, 0x0008u,
                      A_F0115_DOOR);
    ok &= expect_uint("contract.door_marker_nibble_pass2",
                      c ? c->door_marker_nibble_pass2 : 0u, 0x0009u,
                      A_F0115_DOOR);
    ok &= expect_contains("contract.f0124_anchor", c ? c->redmcsb_f0124_anchor : 0,
                          "7873-7911", A_F0124);
    ok &= expect_contains("contract.f0115_anchor", c ? c->redmcsb_f0115_anchor : 0,
                          "5668-5671", A_F0115_PROJECTILE);
    ok &= expect_contains("contract.defs_anchor", c ? c->redmcsb_defs_anchor : 0,
                          "2669/2672", A_DEFS);
    ok &= expect_contains("contract.summary", c ? c->source_summary : 0,
                          "BACK before", A_F0124);

    return ok;
}

static int test_accessors(void)
{
    int ok = 1;
    const CSB_V1_D1CF0115ThingPassPc34 *back =
        csb_v1_viewport_d1c_f0115_thing_pass_for_pass_pc34(1);
    const CSB_V1_D1CF0115ThingPassPc34 *front =
        csb_v1_viewport_d1c_f0115_thing_pass_for_pass_pc34(2);

    ok &= expect_int("count", (int)csb_v1_viewport_d1c_f0115_thing_pass_count_pc34(),
                     2, A_F0124);
    ok &= expect_int("back.present", back != 0, 1, A_F0124);
    ok &= expect_int("front.present", front != 0, 1, A_F0124);
    ok &= expect_int("unknown.null",
                     csb_v1_viewport_d1c_f0115_thing_pass_for_pass_pc34(3) == 0,
                     1, A_F0124);
    ok &= expect_int("index0.back",
                     csb_v1_viewport_d1c_f0115_thing_pass_at_pc34(0) == back,
                     1, A_F0124);
    ok &= expect_int("index1.front",
                     csb_v1_viewport_d1c_f0115_thing_pass_at_pc34(1) == front,
                     1, A_F0124);
    ok &= expect_int("index2.null",
                     csb_v1_viewport_d1c_f0115_thing_pass_at_pc34(2) == 0,
                     1, A_F0124);

    return ok;
}

static int test_pass_specs(void)
{
    int ok = 1;

    for (size_t i = 0; i < csb_v1_viewport_d1c_f0115_thing_pass_count_pc34(); ++i) {
        const CSB_V1_D1CF0115ThingPassPc34 *p =
            csb_v1_viewport_d1c_f0115_thing_pass_at_pc34(i);
        const int is_back = p && p->door_pass == CSB_V1_D1C_F0115_PASS_BACK_PC34;
        char label[96];

        snprintf(label, sizeof(label), "pass%zu.door_pass", i);
        ok &= expect_int(label, p ? p->door_pass : -1, is_back ? 1 : 2,
                         A_F0115_DOOR);
        snprintf(label, sizeof(label), "pass%zu.d1c_view_square", i);
        ok &= expect_int(label, p ? p->d1c_view_square : -1, 3, A_DEFS);
        snprintf(label, sizeof(label), "pass%zu.order_word", i);
        ok &= expect_uint(label, p ? p->order_word : 0u,
                          is_back ? 0x0218u : 0x0349u, A_DEFS);
        snprintf(label, sizeof(label), "pass%zu.order_after_marker", i);
        ok &= expect_uint(label, p ? p->order_after_marker : 0u,
                          is_back ? 0x0021u : 0x0034u, A_F0115_DOOR);
        snprintf(label, sizeof(label), "pass%zu.door_marker_nibble", i);
        ok &= expect_uint(label, p ? p->door_marker_nibble : 0u,
                          is_back ? 0x0008u : 0x0009u, A_F0115_DOOR);
        snprintf(label, sizeof(label), "pass%zu.expected_door_pass", i);
        ok &= expect_int(label, p ? p->expected_door_pass : -1, is_back ? 1 : 2,
                         A_F0115_DOOR);
        snprintf(label, sizeof(label), "pass%zu.cell_count", i);
        ok &= expect_int(label, p ? p->cell_count : -1, 2, A_F0115_ORDER);
        snprintf(label, sizeof(label), "pass%zu.cell0", i);
        ok &= expect_int(label, p ? p->cells[0] : -1, is_back ? 1 : 4,
                         A_F0115_ORDER);
        snprintf(label, sizeof(label), "pass%zu.cell1", i);
        ok &= expect_int(label, p ? p->cells[1] : -1, is_back ? 2 : 3,
                         A_F0115_ORDER);
        snprintf(label, sizeof(label), "pass%zu.cell2.zero", i);
        ok &= expect_int(label, p ? p->cells[2] : -1, 0, A_F0115_ORDER);
        snprintf(label, sizeof(label), "pass%zu.back_cell_count", i);
        ok &= expect_int(label, p ? p->back_cell_count : -1, is_back ? 2 : 0,
                         A_F0115_ORDER);
        snprintf(label, sizeof(label), "pass%zu.front_cell_count", i);
        ok &= expect_int(label, p ? p->front_cell_count : -1, is_back ? 0 : 2,
                         A_F0115_ORDER);
        snprintf(label, sizeof(label), "pass%zu.before_door_frame", i);
        ok &= expect_int(label, p ? p->draws_before_door_frame : -1,
                         is_back ? 1 : 0, A_F0124);
        snprintf(label, sizeof(label), "pass%zu.after_door_frame", i);
        ok &= expect_int(label, p ? p->draws_after_door_frame : -1,
                         is_back ? 0 : 1, A_F0124);
        snprintf(label, sizeof(label), "pass%zu.call_ordinal", i);
        ok &= expect_int(label, p ? p->f0115_call_ordinal : -1,
                         is_back ? 1 : 2, A_F0124);
        snprintf(label, sizeof(label), "pass%zu.pass_name", i);
        ok &= expect_contains(label, p ? p->pass_name : 0,
                              is_back ? "BACK" : "FRONT", A_F0124);
    }

    return ok;
}

static int test_f0115_draw_contract(void)
{
    int ok = 1;

    for (size_t i = 0; i < csb_v1_viewport_d1c_f0115_thing_pass_count_pc34(); ++i) {
        const CSB_V1_D1CF0115ThingPassPc34 *p =
            csb_v1_viewport_d1c_f0115_thing_pass_at_pc34(i);
        char label[96];

        snprintf(label, sizeof(label), "pass%zu.object_first", i);
        ok &= expect_int(label, p ? p->object_pass_first : 0, 1, A_F0115_ORDER);
        snprintf(label, sizeof(label), "pass%zu.creature_second", i);
        ok &= expect_int(label, p ? p->creature_pass_second : 0, 1,
                         A_F0115_CREATURE);
        snprintf(label, sizeof(label), "pass%zu.projectile_third", i);
        ok &= expect_int(label, p ? p->projectile_pass_third : 0, 1,
                         A_F0115_PROJECTILE);
        snprintf(label, sizeof(label), "pass%zu.explosion_last", i);
        ok &= expect_int(label, p ? p->explosion_pass_last : 0, 1,
                         A_F0115_ORDER);
        snprintf(label, sizeof(label), "pass%zu.item_row_guard", i);
        ok &= expect_int(label, p ? p->item_row_guard : 0, 1, A_F0115_ITEM);
        snprintf(label, sizeof(label), "pass%zu.creature_row_guard", i);
        ok &= expect_int(label, p ? p->creature_row_guard : 0, 1,
                         A_F0115_CREATURE);
        snprintf(label, sizeof(label), "pass%zu.projectile_row_guard", i);
        ok &= expect_int(label, p ? p->projectile_row_guard : 0, 1,
                         A_F0115_PROJECTILE);
        snprintf(label, sizeof(label), "pass%zu.c10_transparency", i);
        ok &= expect_int(label, p ? p->c10_transparency : 0, 1,
                         A_F0115_OBJECT);
        snprintf(label, sizeof(label), "pass%zu.call_anchor", i);
        ok &= expect_contains(label, p ? p->redmcsb_call_anchor : 0,
                              p && p->door_pass == 1 ? "7874-7875" : "7910-7937",
                              A_F0124);
        snprintf(label, sizeof(label), "pass%zu.order_anchor", i);
        ok &= expect_contains(label, p ? p->redmcsb_order_anchor : 0,
                              p && p->door_pass == 1 ? "C0x0218" : "C0x0349",
                              A_DEFS);
        snprintf(label, sizeof(label), "pass%zu.source_lines", i);
        ok &= expect_contains(label, p ? p->source_lines : 0,
                              p && p->door_pass == 1 ? "BACK cells" : "FRONT cells",
                              A_F0115_ORDER);
    }

    return ok;
}

static int test_decode_orders(void)
{
    int ok = 1;
    CSB_V1_D1CF0115DecodedOrderPc34 back;
    CSB_V1_D1CF0115DecodedOrderPc34 front;
    CSB_V1_D1CF0115DecodedOrderPc34 corridor;
    CSB_V1_D1CF0115DecodedOrderPc34 invalid;

    ok &= expect_int("decode.back.ok",
                     csb_v1_viewport_d1c_f0115_decode_order_pc34(0x0218u, &back),
                     1, A_F0115_DOOR);
    ok &= expect_int("decode.front.ok",
                     csb_v1_viewport_d1c_f0115_decode_order_pc34(0x0349u, &front),
                     1, A_F0115_DOOR);
    ok &= expect_int("decode.corridor.ok",
                     csb_v1_viewport_d1c_f0115_decode_order_pc34(0x3421u, &corridor),
                     1, A_F0115_ORDER);
    ok &= expect_int("decode.invalid.ok",
                     csb_v1_viewport_d1c_f0115_decode_order_pc34(0x0005u, &invalid),
                     1, A_F0115_ORDER);
    ok &= expect_int("decode.null.reject",
                     csb_v1_viewport_d1c_f0115_decode_order_pc34(0x0218u, 0),
                     0, A_F0115_DOOR);

    ok &= expect_uint("back.input", back.input_order, 0x0218u, A_DEFS);
    ok &= expect_int("back.has_marker", back.has_door_marker, 1, A_F0115_DOOR);
    ok &= expect_int("back.door_pass", back.door_pass, 1, A_F0115_DOOR);
    ok &= expect_uint("back.stripped", back.stripped_order, 0x0021u,
                      A_F0115_DOOR);
    ok &= expect_int("back.cell_count", back.cell_count, 2, A_F0115_ORDER);
    ok &= expect_int("back.cell0", back.cells[0], 1, A_DEFS);
    ok &= expect_int("back.cell1", back.cells[1], 2, A_DEFS);
    ok &= expect_int("back.back_cell_count", back.back_cell_count, 2,
                     A_F0115_ORDER);
    ok &= expect_int("back.front_cell_count", back.front_cell_count, 0,
                     A_F0115_ORDER);
    ok &= expect_int("back.stops_at_zero", back.stops_at_zero_nibble, 1,
                     A_F0115_ORDER);
    ok &= expect_int("back.invalid", back.invalid_cell_seen, 0, A_F0115_ORDER);

    ok &= expect_uint("front.input", front.input_order, 0x0349u, A_DEFS);
    ok &= expect_int("front.has_marker", front.has_door_marker, 1, A_F0115_DOOR);
    ok &= expect_int("front.door_pass", front.door_pass, 2, A_F0115_DOOR);
    ok &= expect_uint("front.stripped", front.stripped_order, 0x0034u,
                      A_F0115_DOOR);
    ok &= expect_int("front.cell_count", front.cell_count, 2, A_F0115_ORDER);
    ok &= expect_int("front.cell0", front.cells[0], 4, A_DEFS);
    ok &= expect_int("front.cell1", front.cells[1], 3, A_DEFS);
    ok &= expect_int("front.back_cell_count", front.back_cell_count, 0,
                     A_F0115_ORDER);
    ok &= expect_int("front.front_cell_count", front.front_cell_count, 2,
                     A_F0115_ORDER);
    ok &= expect_int("front.stops_at_zero", front.stops_at_zero_nibble, 1,
                     A_F0115_ORDER);
    ok &= expect_int("front.invalid", front.invalid_cell_seen, 0, A_F0115_ORDER);

    ok &= expect_int("corridor.no_marker", corridor.has_door_marker, 0,
                     A_F0115_DOOR);
    ok &= expect_int("corridor.door_pass", corridor.door_pass, 0,
                     A_F0115_DOOR);
    ok &= expect_int("corridor.cell_count", corridor.cell_count, 4,
                     A_F0115_ORDER);
    ok &= expect_int("corridor.cell0", corridor.cells[0], 1, A_DEFS);
    ok &= expect_int("corridor.cell1", corridor.cells[1], 2, A_DEFS);
    ok &= expect_int("corridor.cell2", corridor.cells[2], 4, A_DEFS);
    ok &= expect_int("corridor.cell3", corridor.cells[3], 3, A_DEFS);
    ok &= expect_int("corridor.back_cell_count", corridor.back_cell_count, 2,
                     A_F0115_ORDER);
    ok &= expect_int("corridor.front_cell_count", corridor.front_cell_count, 2,
                     A_F0115_ORDER);
    ok &= expect_int("invalid.invalid_cell_seen", invalid.invalid_cell_seen, 1,
                     A_F0115_ORDER);

    return ok;
}

static int test_evidence_strings(void)
{
    int ok = 1;
    const char *e =
        csb_v1_viewport_d1c_f0115_thing_pass_source_evidence_pc34();

    ok &= expect_contains("evidence.contract_only", e, "contract-only", A_F0124);
    ok &= expect_contains("evidence.real_graphics", e, "real-asset receipt",
                          A_F0124);
    ok &= expect_contains("evidence.object_family", e, "498..583", A_F0115_OBJECT);
    ok &= expect_contains("evidence.no_dungeon", e, "no DUNGEON.DAT", A_F0124);
    ok &= expect_contains("evidence.no_pixels", e, "no decoded real-asset pixels",
                          A_F0124);
    ok &= expect_contains("evidence.f0124.back", e, "7873-7911", A_F0124);
    ok &= expect_contains("evidence.f0124.front", e, "7910-7937", A_F0124);
    ok &= expect_contains("evidence.f0115.order", e, "4547-4581",
                          A_F0115_ORDER);
    ok &= expect_contains("evidence.f0115.door", e, "4795-4800",
                          A_F0115_DOOR);
    ok &= expect_contains("evidence.f0115.item", e, "4923", A_F0115_ITEM);
    ok &= expect_contains("evidence.f0115.object", e, "5180-5188",
                          A_F0115_OBJECT);
    ok &= expect_contains("evidence.f0115.creature", e, "5211-5214",
                          A_F0115_CREATURE);
    ok &= expect_contains("evidence.f0115.projectile", e, "5668-5671",
                          A_F0115_PROJECTILE);
    ok &= expect_contains("evidence.defs.2669", e, "2669", A_DEFS);
    ok &= expect_contains("evidence.defs.2672", e, "2672", A_DEFS);
    ok &= expect_contains("evidence.mask", e, "MASK 0x0008", A_F0115_DOOR);
    ok &= expect_contains("evidence.back", e, "BACK pass", A_F0124);
    ok &= expect_contains("evidence.front", e, "FRONT pass", A_F0124);
    ok &= expect_contains("evidence.disjoint_custom", e, "CustomBackgrounds",
                          A_F0124);
    ok &= expect_contains("evidence.disjoint_d1c_f0108", e, "D1C F0108",
                          A_F0124);
    ok &= expect_contains("evidence.disjoint_dm1", e, "DM1 D0C", A_F0124);

    return ok;
}

static int test_real_graphics_dat_object_receipts(void)
{
    int ok = 1;
    const char *path = getenv("FIRESTAFF_CSB_GRAPHICS_DAT");
    const int graphic_indices[2] = { 498, 583 };

    if (!path || !path[0]) {
        path = "/Users/bosse/.firestaff/data/csb/GRAPHICS.DAT";
    }

    for (size_t i = 0; i < csb_v1_viewport_d1c_f0115_thing_pass_count_pc34(); ++i) {
        const CSB_V1_D1CF0115ThingPassPc34 *p =
            csb_v1_viewport_d1c_f0115_thing_pass_at_pc34(i);
        CSB_V1_D1CF0115ThingPassRealAssetReceiptPc34 receipt;
        size_t payload_size = 0u;
        uint32_t payload_hash = 0u;
        char label[96];
        int graphic_index = graphic_indices[i];

        snprintf(label, sizeof(label), "real.pass%zu.hash.read", i);
        ok &= expect_int(label,
                         read_real_graphics_item_hash(path,
                             (unsigned)graphic_index,
                             &payload_size, &payload_hash),
                         1, "DMCSB1 real GRAPHICS.DAT native object item");
        snprintf(label, sizeof(label), "real.pass%zu.payload.nonzero", i);
        ok &= expect_int(label, payload_size > 0u, 1,
                         "DMCSB1 real GRAPHICS.DAT native object item");
        snprintf(label, sizeof(label), "real.pass%zu.hash.nonzero", i);
        ok &= expect_int(label, payload_hash != 0u, 1,
                         "DMCSB1 real GRAPHICS.DAT native object item");

        snprintf(label, sizeof(label), "real.pass%zu.receipt.ok", i);
        ok &= expect_int(label,
            csb_v1_viewport_d1c_f0115_thing_pass_real_asset_receipt_pc34(
                p, 1, 1, 1, 1, graphic_index, payload_size, payload_hash,
                &receipt),
            1, A_F0115_OBJECT);
        snprintf(label, sizeof(label), "real.pass%zu.receipt.valid", i);
        ok &= expect_int(label, receipt.valid, 1, A_F0115_OBJECT);
        snprintf(label, sizeof(label), "real.pass%zu.receipt.graphic", i);
        ok &= expect_int(label, receipt.source_graphics_item_index,
                         graphic_index, A_F0115_OBJECT);
        snprintf(label, sizeof(label), "real.pass%zu.receipt.hash", i);
        ok &= expect_int(label, receipt.source_payload_hash == payload_hash, 1,
                         A_F0115_OBJECT);
        snprintf(label, sizeof(label), "real.pass%zu.receipt.min", i);
        ok &= expect_int(label, receipt.native_object_graphic_min, 498,
                         A_F0115_OBJECT);
        snprintf(label, sizeof(label), "real.pass%zu.receipt.max", i);
        ok &= expect_int(label, receipt.native_object_graphic_max, 583,
                         A_F0115_OBJECT);
        snprintf(label, sizeof(label), "real.pass%zu.receipt.no_synth", i);
        ok &= expect_int(label, receipt.no_synthetic_pixels, 1, A_F0115_OBJECT);
        snprintf(label, sizeof(label), "real.pass%zu.receipt.no_fallback", i);
        ok &= expect_int(label, receipt.no_fallback_visuals, 1, A_F0115_OBJECT);

        snprintf(label, sizeof(label), "real.pass%zu.reject.no_source", i);
        ok &= expect_int(label,
            csb_v1_viewport_d1c_f0115_thing_pass_real_asset_receipt_pc34(
                p, 0, 1, 1, 1, graphic_index, payload_size, payload_hash,
                &receipt),
            0, A_F0115_OBJECT);
        snprintf(label, sizeof(label), "real.pass%zu.reject.outside_family", i);
        ok &= expect_int(label,
            csb_v1_viewport_d1c_f0115_thing_pass_real_asset_receipt_pc34(
                p, 1, 1, 1, 1, 584, payload_size, payload_hash, &receipt),
            0, A_F0115_OBJECT);
        snprintf(label, sizeof(label), "real.pass%zu.reject.synthetic", i);
        ok &= expect_int(label,
            csb_v1_viewport_d1c_f0115_thing_pass_real_asset_receipt_pc34(
                p, 1, 1, 0, 1, graphic_index, payload_size, payload_hash,
                &receipt),
            0, A_F0115_OBJECT);
        snprintf(label, sizeof(label), "real.pass%zu.reject.fallback", i);
        ok &= expect_int(label,
            csb_v1_viewport_d1c_f0115_thing_pass_real_asset_receipt_pc34(
                p, 1, 1, 1, 0, graphic_index, payload_size, payload_hash,
                &receipt),
            0, A_F0115_OBJECT);
        snprintf(label, sizeof(label), "real.pass%zu.reject.zero_hash", i);
        ok &= expect_int(label,
            csb_v1_viewport_d1c_f0115_thing_pass_real_asset_receipt_pc34(
                p, 1, 1, 1, 1, graphic_index, payload_size, 0u, &receipt),
            0, A_F0115_OBJECT);
    }

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d1c_f0115_thing_pass_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d1c_f0115_thing_pass_source_evidence_pc34());

    ok &= test_contract();
    ok &= test_accessors();
    ok &= test_pass_specs();
    ok &= test_f0115_draw_contract();
    ok &= test_decode_orders();
    ok &= test_real_graphics_dat_object_receipts();
    ok &= test_evidence_strings();

    ok &= expect_int("assertion_count_at_least_110", g_assertions >= 110, 1,
                     A_F0115_ORDER);
    printf("assertionCount=%d\n", g_assertions);
    if (ok) {
        printf("PASS csb_v1_viewport_d1c_f0115_thing_pass_pc34_compat %d/%d assertions\n",
               g_assertions, g_assertions);
    }
    return ok ? 0 : 1;
}
