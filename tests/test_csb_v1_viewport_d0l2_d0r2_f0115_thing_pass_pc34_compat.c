#include "csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_pc34_compat.h"
#include "dm1_v1_graphics_loader_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * CSB V1 D0L2/D0R2 corridor thing-pass source-lock gate.
 * ReDMCSB: DUNVIEW.C:7960-8062 F0125 and 8064-8162 F0126 route the D0
 * near side lanes through F0115 at 8005/8115 after the per-frame bitmap
 * copy in F0674_F0128_sub. G0163 rows 10/11 describe the clipped D0L/D0R
 * wall-frame rectangles. CSB-lineage Viewport.cpp:1192-1209 confirms the
 * F0L1/F0R1 open room-object routes, and 1903-1915 is retained as the
 * separate center door-facing two-pass dispatch cross-reference.
 */

static const char *A_D0L =
    "ReDMCSB DUNVIEW.C:7960-8062 F0125_DUNGEONVIEW_DrawSquareD0L";
static const char *A_D0R =
    "ReDMCSB DUNVIEW.C:8064-8162 F0126_DUNGEONVIEW_DrawSquareD0R";
static const char *A_DISPATCH =
    "ReDMCSB DUNVIEW.C:8536-8541 F0128 D0L/D0R dispatch";
static const char *A_F0674 =
    "ReDMCSB DUNVIEW.C:2995-3015 F0674_F0128_sub";
static const char *A_FRAMES =
    "ReDMCSB DUNVIEW.C:581-594 G0163 D0L/D0R frame rows";
static const char *A_F0115 =
    "ReDMCSB DUNVIEW.C:4547-4581 F0115 draw order";
static const char *A_F0115_ITEM =
    "ReDMCSB DUNVIEW.C:4806-4811,4923 G2028 item gate";
static const char *A_F0115_CREATURE =
    "ReDMCSB DUNVIEW.C:5201-5214,5295,5615-5617 C3200 creature gate";
static const char *A_F0115_PROJECTILE =
    "ReDMCSB DUNVIEW.C:5668-5683 C2900 projectile gate";
static const char *A_F0115_EXPLOSION =
    "ReDMCSB DUNVIEW.C:5916-5923,5998-5999,6107,6122 explosion gate";
static const char *A_DEFS =
    "ReDMCSB DEFS.H:2088,2596-2598,2642-2660,4056-4057,4228-4236";
static const char *A_LINEAGE_OPEN =
    "CSB-lineage Viewport.cpp:1192-1209 F0L1/F0R1 open routes";
static const char *A_LINEAGE_CENTER =
    "CSB-lineage Viewport.cpp:1903-1915 center door-facing dispatch";

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d anchor=%s\n", id, want, anchor);
    }
}

static void expect_contains(const char *id, const char *haystack,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing=%s anchor=%s\n",
               id, needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains=%s anchor=%s\n", id, needle, anchor);
    }
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

static void write_be16(unsigned char *bytes, size_t offset, unsigned value)
{
    bytes[offset] = (unsigned char)((value >> 8) & 0xffu);
    bytes[offset + 1u] = (unsigned char)(value & 0xffu);
}

/* The native decoder uses LSB-first 9-bit codes. Reset before the dictionary
 * grows: this is a real, bounded CSBgraphics.dat LZW payload, not a decoded
 * pixel injection. */
static unsigned char *encode_lzw_literal_stream(const unsigned char *input,
                                                 size_t input_size,
                                                 size_t *out_size)
{
    size_t bit_count;
    size_t bit_pos = 0u;
    size_t i = 0u;
    unsigned char *output;

    if (!input || input_size == 0u || !out_size) return NULL;
    bit_count = (input_size + (input_size / 200u) + 2u) * 9u;
    output = (unsigned char *)calloc((bit_count + 7u) / 8u, 1u);
    if (!output) return NULL;

#define WRITE_CODE(value) do { \
        unsigned code_value = (unsigned)(value); \
        int bit; \
        for (bit = 0; bit < 9; ++bit, ++bit_pos) { \
            if (code_value & (1u << (unsigned)bit)) { \
                output[bit_pos >> 3] |= (unsigned char)(1u << (bit_pos & 7u)); \
            } \
        } \
    } while (0)
    while (i < input_size) {
        size_t end = i + 200u;
        if (end > input_size) end = input_size;
        WRITE_CODE(DM1_GFX_LZW_CLEAR_CODE);
        while (i < end) {
            WRITE_CODE(input[i]);
            ++i;
        }
    }
    WRITE_CODE(DM1_GFX_LZW_END_CODE);
#undef WRITE_CODE
    *out_size = (bit_pos + 7u) / 8u;
    return output;
}

static unsigned char *build_cache_graphics(const unsigned char *palette,
                                           const unsigned char *field,
                                           size_t *out_size)
{
    unsigned char *compressed_palette = NULL;
    unsigned char *compressed_field = NULL;
    unsigned char *bytes = NULL;
    size_t palette_size = 0u;
    size_t field_size = 0u;
    const size_t header_size = 10u;

    if (!palette || !field || !out_size) return NULL;
    compressed_palette = encode_lzw_literal_stream(
        palette, CSB_V1_CSBGRAPHICS_DAT_PALETTE_BYTES, &palette_size);
    compressed_field = encode_lzw_literal_stream(field, 32u * 136u, &field_size);
    if (!compressed_palette || !compressed_field || palette_size > 65535u ||
        field_size > 65535u) {
        free(compressed_palette);
        free(compressed_field);
        return NULL;
    }
    bytes = (unsigned char *)calloc(header_size + palette_size + field_size, 1u);
    if (!bytes) {
        free(compressed_palette);
        free(compressed_field);
        return NULL;
    }
    write_be16(bytes, 0u, 2u);
    write_be16(bytes, 2u, (unsigned)palette_size);
    write_be16(bytes, 4u, (unsigned)field_size);
    write_be16(bytes, 6u, CSB_V1_CSBGRAPHICS_DAT_PALETTE_BYTES);
    write_be16(bytes, 8u, 32u * 136u);
    memcpy(bytes + header_size, compressed_palette, palette_size);
    memcpy(bytes + header_size + palette_size, compressed_field, field_size);
    *out_size = header_size + palette_size + field_size;
    free(compressed_palette);
    free(compressed_field);
    return bytes;
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

static const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture_for_index(int index)
{
    return csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_for_square_pc34(index + 1);
}

static const char *anchor_for_index(int index)
{
    return index == 0 ? A_D0L : A_D0R;
}

static void test_accessors_and_contract_markers(void)
{
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *d0l2;
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *d0r2;

    expect_int("init",
               csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_init_pc34(),
               1, A_DISPATCH);

    d0l2 = csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_for_square_pc34(1);
    d0r2 = csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_for_square_pc34(2);

    expect_int("fixture.count",
               (int)csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_count_pc34(),
               2, A_DEFS);
    expect_int("d0l2.present", d0l2 != NULL, 1, A_D0L);
    expect_int("d0r2.present", d0r2 != NULL, 1, A_D0R);
    expect_int("unknown.null",
               csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_for_square_pc34(3) == NULL,
               1, A_DEFS);
    expect_int("index0.d0l2",
               csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_at_pc34(0) == d0l2,
               1, A_D0L);
    expect_int("index1.d0r2",
               csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_at_pc34(1) == d0r2,
               1, A_D0R);
    expect_int("past.end.null",
               csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_at_pc34(2) == NULL,
               1, A_DEFS);

    for (int i = 0; i < 2; ++i) {
        const CSB_V1_D0L2D0R2F0115ThingPassPc34 *f = fixture_for_index(i);
        const char *anchor = anchor_for_index(i);
        char label[80];

        snprintf(label, sizeof(label), "side%d.side", i + 1);
        expect_int(label, f ? f->side : -1, i + 1, anchor);
        snprintf(label, sizeof(label), "side%d.contract_only", i + 1);
        expect_int(label, f ? f->source_locked_contract_only : 0, 1, A_F0115);
        snprintf(label, sizeof(label), "side%d.no_asset_parity", i + 1);
        expect_int(label, f ? f->no_real_asset_bitmap_parity : 0, 1, A_F0115);
        snprintf(label, sizeof(label), "side%d.no_game_data", i + 1);
        expect_int(label, f ? f->no_game_data_load : 0, 1, A_F0115);
        snprintf(label, sizeof(label), "side%d.route_count", i + 1);
        expect_int(label, f ? f->route_count : -1, 1, anchor);
        snprintf(label, sizeof(label), "side%d.f0115_call_count", i + 1);
        expect_int(label, f ? f->f0115_call_count : -1, 1, anchor);
        snprintf(label, sizeof(label), "side%d.c10_transparency_flag", i + 1);
        expect_int(label, f ? f->c10_transparency_flag : 0, 1, A_DEFS);
        snprintf(label, sizeof(label), "side%d.transparent_color", i + 1);
        expect_int(label, f ? f->transparent_color : -1, 10, A_DEFS);
    }
}

static void test_view_square_frame_and_route_metadata(void)
{
    static const int want_view_square[2] = { 1, 2 };
    static const int want_lane[2] = { -1, 1 };
    static const int want_order[2] = { 0x0002, 0x0001 };
    static const int want_cell[2] = { 2, 3 };
    static const int want_frame_row[2] = { 10, 11 };
    static const int want_x1[2] = { 0, 192 };
    static const int want_x2[2] = { 31, 223 };
    static const int want_wall_zone[2] = { 716, 717 };
    static const int want_ceiling_zone[2] = { 870, 872 };

    for (int i = 0; i < 2; ++i) {
        const CSB_V1_D0L2D0R2F0115ThingPassPc34 *f = fixture_for_index(i);
        const char *anchor = anchor_for_index(i);
        char label[96];

        snprintf(label, sizeof(label), "side%d.view_square", i + 1);
        expect_int(label, f ? f->view_square_index : -1, want_view_square[i],
                   A_DEFS);
        snprintf(label, sizeof(label), "side%d.view_depth", i + 1);
        expect_int(label, f ? f->view_depth : -1, 0,
                   "ReDMCSB DUNVIEW.C:372 G2027[1/2]");
        snprintf(label, sizeof(label), "side%d.view_lane", i + 1);
        expect_int(label, f ? f->view_lane : 99, want_lane[i],
                   "ReDMCSB DUNVIEW.C:371 G2026[1/2]");
        snprintf(label, sizeof(label), "side%d.cell_order", i + 1);
        expect_int(label, f ? (int)f->f0115_cell_order : -1, want_order[i],
                   anchor);
        snprintf(label, sizeof(label), "side%d.first_cell", i + 1);
        expect_int(label, f ? f->f0115_first_cell : -1, want_cell[i], A_DEFS);
        snprintf(label, sizeof(label), "side%d.cell_count", i + 1);
        expect_int(label, f ? f->f0115_cell_count : -1, 1, A_F0115);
        snprintf(label, sizeof(label), "side%d.frame_row", i + 1);
        expect_int(label, f ? f->wall_frame_row : -1, want_frame_row[i],
                   A_FRAMES);
        snprintf(label, sizeof(label), "side%d.frame_x1", i + 1);
        expect_int(label, f ? f->wall_frame_x1 : -1, want_x1[i], A_FRAMES);
        snprintf(label, sizeof(label), "side%d.frame_x2", i + 1);
        expect_int(label, f ? f->wall_frame_x2 : -1, want_x2[i], A_FRAMES);
        snprintf(label, sizeof(label), "side%d.frame_y1", i + 1);
        expect_int(label, f ? f->wall_frame_y1 : -1, 0, A_FRAMES);
        snprintf(label, sizeof(label), "side%d.frame_y2", i + 1);
        expect_int(label, f ? f->wall_frame_y2 : -1, 135, A_FRAMES);
        snprintf(label, sizeof(label), "side%d.frame_byte_width", i + 1);
        expect_int(label, f ? f->wall_frame_byte_width : -1, 16, A_FRAMES);
        snprintf(label, sizeof(label), "side%d.frame_height", i + 1);
        expect_int(label, f ? f->wall_frame_height : -1, 136, A_FRAMES);
        snprintf(label, sizeof(label), "side%d.frame_source_x", i + 1);
        expect_int(label, f ? f->wall_frame_source_x : -1, 0, A_FRAMES);
        snprintf(label, sizeof(label), "side%d.frame_source_y", i + 1);
        expect_int(label, f ? f->wall_frame_source_y : -1, 0, A_FRAMES);
        snprintf(label, sizeof(label), "side%d.wall_zone", i + 1);
        expect_int(label, f ? f->wall_zone : -1, want_wall_zone[i], A_DEFS);
        snprintf(label, sizeof(label), "side%d.ceiling_pit_zone", i + 1);
        expect_int(label, f ? f->ceiling_pit_zone : -1, want_ceiling_zone[i],
                   anchor);
        snprintf(label, sizeof(label), "side%d.f0112_before_f0115", i + 1);
        expect_int(label, f ? f->f0112_before_f0115 : 0, 1, anchor);
        snprintf(label, sizeof(label), "side%d.teleporter_field_after", i + 1);
        expect_int(label, f ? f->teleporter_field_after_f0115 : 0, 1, anchor);
    }

    expect_int("draw_order.d0l_before_d0r",
               (fixture_for_index(0) ? fixture_for_index(0)->view_square_index : 99) <
                   (fixture_for_index(1) ? fixture_for_index(1)->view_square_index : -1),
               1, A_DISPATCH);
}

static void test_clip_no_write_and_blend_contract(void)
{
    static const int inside_x[2] = { 12, 208 };
    static const int outside_left_x[2] = { -1, 191 };
    static const int outside_right_x[2] = { 32, 224 };

    for (int i = 0; i < 2; ++i) {
        const CSB_V1_D0L2D0R2F0115ThingPassPc34 *f = fixture_for_index(i);
        unsigned char dst;
        char label[96];

        snprintf(label, sizeof(label), "side%d.viewport_clip_x1", i + 1);
        expect_int(label, f ? f->viewport_clip_x1 : -1, i == 0 ? 0 : 192,
                   A_FRAMES);
        snprintf(label, sizeof(label), "side%d.viewport_clip_x2", i + 1);
        expect_int(label, f ? f->viewport_clip_x2 : -1, i == 0 ? 31 : 223,
                   A_FRAMES);
        snprintf(label, sizeof(label), "side%d.viewport_clip_y1", i + 1);
        expect_int(label, f ? f->viewport_clip_y1 : -1, 0, A_FRAMES);
        snprintf(label, sizeof(label), "side%d.viewport_clip_y2", i + 1);
        expect_int(label, f ? f->viewport_clip_y2 : -1, 135, A_FRAMES);
        snprintf(label, sizeof(label), "side%d.source_clip_y1", i + 1);
        expect_int(label, f ? f->source_clip_y1 : -1, 0, A_FRAMES);
        snprintf(label, sizeof(label), "side%d.source_clip_y2", i + 1);
        expect_int(label, f ? f->source_clip_y2 : -1, 135, A_FRAMES);
        snprintf(label, sizeof(label), "side%d.clip.inside", i + 1);
        expect_int(label,
                   csb_v1_viewport_d0l2_d0r2_f0115_viewport_clip_contains_pc34(
                       f, inside_x[i], 64),
                   1, A_FRAMES);
        snprintf(label, sizeof(label), "side%d.clip.outside_left", i + 1);
        expect_int(label,
                   csb_v1_viewport_d0l2_d0r2_f0115_viewport_clip_contains_pc34(
                       f, outside_left_x[i], 64),
                   0, A_FRAMES);
        snprintf(label, sizeof(label), "side%d.clip.outside_right", i + 1);
        expect_int(label,
                   csb_v1_viewport_d0l2_d0r2_f0115_viewport_clip_contains_pc34(
                       f, outside_right_x[i], 64),
                   0, A_FRAMES);
        snprintf(label, sizeof(label), "side%d.clip.outside_top", i + 1);
        expect_int(label,
                   csb_v1_viewport_d0l2_d0r2_f0115_viewport_clip_contains_pc34(
                       f, inside_x[i], -1),
                   0, A_FRAMES);
        snprintf(label, sizeof(label), "side%d.clip.outside_bottom", i + 1);
        expect_int(label,
                   csb_v1_viewport_d0l2_d0r2_f0115_viewport_clip_contains_pc34(
                       f, inside_x[i], 136),
                   0, A_FRAMES);
        snprintf(label, sizeof(label), "side%d.source_y.visible", i + 1);
        expect_int(label,
                   csb_v1_viewport_d0l2_d0r2_f0115_source_y_visible_pc34(f, 32),
                   1, A_FRAMES);
        snprintf(label, sizeof(label), "side%d.source_y.before", i + 1);
        expect_int(label,
                   csb_v1_viewport_d0l2_d0r2_f0115_source_y_visible_pc34(f, -1),
                   0, A_FRAMES);
        snprintf(label, sizeof(label), "side%d.source_y.after", i + 1);
        expect_int(label,
                   csb_v1_viewport_d0l2_d0r2_f0115_source_y_visible_pc34(f, 136),
                   0, A_FRAMES);
        snprintf(label, sizeof(label), "side%d.no_write_transparent_flag", i + 1);
        expect_int(label, f ? f->no_write_on_transparent : 0, 1, A_DEFS);
        snprintf(label, sizeof(label), "side%d.no_write_viewport_flag", i + 1);
        expect_int(label, f ? f->no_write_outside_viewport_clip : 0, 1,
                   A_FRAMES);
        snprintf(label, sizeof(label), "side%d.no_write_source_y_flag", i + 1);
        expect_int(label, f ? f->no_write_outside_source_y_clip : 0, 1,
                   A_FRAMES);
        snprintf(label, sizeof(label), "side%d.blend.transparent", i + 1);
        expect_int(label,
                   csb_v1_viewport_d0l2_d0r2_f0115_blend_pixel_pc34(f, 77, 10),
                   77, A_DEFS);
        snprintf(label, sizeof(label), "side%d.blend.opaque", i + 1);
        expect_int(label,
                   csb_v1_viewport_d0l2_d0r2_f0115_blend_pixel_pc34(f, 77, 42),
                   42, A_DEFS);
        dst = 77;
        snprintf(label, sizeof(label), "side%d.apply.opaque.wrote", i + 1);
        expect_int(label,
                   csb_v1_viewport_d0l2_d0r2_f0115_apply_pixel_pc34(
                       f, inside_x[i], 64, 64, 42, &dst),
                   1, A_DEFS);
        snprintf(label, sizeof(label), "side%d.apply.opaque.value", i + 1);
        expect_int(label, dst, 42, A_DEFS);
        dst = 77;
        snprintf(label, sizeof(label), "side%d.apply.transparent.no_write", i + 1);
        expect_int(label,
                   csb_v1_viewport_d0l2_d0r2_f0115_apply_pixel_pc34(
                       f, inside_x[i], 64, 64, 10, &dst),
                   0, A_DEFS);
        snprintf(label, sizeof(label), "side%d.apply.transparent.value", i + 1);
        expect_int(label, dst, 77, A_DEFS);
        dst = 77;
        snprintf(label, sizeof(label), "side%d.apply.outside_view.no_write", i + 1);
        expect_int(label,
                   csb_v1_viewport_d0l2_d0r2_f0115_apply_pixel_pc34(
                       f, outside_right_x[i], 64, 64, 42, &dst),
                   0, A_FRAMES);
        snprintf(label, sizeof(label), "side%d.apply.outside_view.value", i + 1);
        expect_int(label, dst, 77, A_FRAMES);
        dst = 77;
        snprintf(label, sizeof(label), "side%d.apply.outside_source.no_write", i + 1);
        expect_int(label,
                   csb_v1_viewport_d0l2_d0r2_f0115_apply_pixel_pc34(
                       f, inside_x[i], 64, 136, 42, &dst),
                   0, A_FRAMES);
        snprintf(label, sizeof(label), "side%d.apply.outside_source.value", i + 1);
        expect_int(label, dst, 77, A_FRAMES);
    }

    expect_int("null.clip",
               csb_v1_viewport_d0l2_d0r2_f0115_viewport_clip_contains_pc34(
                   NULL, 0, 0),
               0, "invalid input guard");
    expect_int("null.source_y",
               csb_v1_viewport_d0l2_d0r2_f0115_source_y_visible_pc34(NULL, 0),
               0, "invalid input guard");
    expect_int("null.blend",
               csb_v1_viewport_d0l2_d0r2_f0115_blend_pixel_pc34(NULL, 77, 42),
               77, "invalid input guard");
    expect_int("null.apply",
               csb_v1_viewport_d0l2_d0r2_f0115_apply_pixel_pc34(
                   NULL, 0, 0, 0, 42, NULL),
               0, "invalid input guard");
}

static void test_zone_bindings_and_draw_order(void)
{
    static const int want_creature_row[2] = { 11, 12 };
    static const int want_creature_cell[2] = { 2, 3 };
    static const int want_explosion_row[2] = { 15, 16 };
    static const int want_field[2] = { 14, 15 };

    for (int i = 0; i < 2; ++i) {
        const CSB_V1_D0L2D0R2F0115ThingPassPc34 *f = fixture_for_index(i);
        char label[96];

        snprintf(label, sizeof(label), "side%d.item_row.disabled", i + 1);
        expect_int(label, f ? f->item_projectile_row : 0, -1, A_F0115_ITEM);
        snprintf(label, sizeof(label), "side%d.item_projectile_disabled_flag", i + 1);
        expect_int(label, f ? f->item_projectile_disabled_by_g2028 : 0, 1,
                   A_F0115_ITEM);
        snprintf(label, sizeof(label), "side%d.item_zone.disabled", i + 1);
        expect_int(label,
                   csb_v1_viewport_d0l2_d0r2_f0115_item_zone_pc34(
                       f, want_creature_cell[i]),
                   -1, A_F0115_ITEM);
        snprintf(label, sizeof(label), "side%d.projectile_zone.disabled", i + 1);
        expect_int(label,
                   csb_v1_viewport_d0l2_d0r2_f0115_projectile_zone_pc34(
                       f, want_creature_cell[i]),
                   -1, A_F0115_PROJECTILE);
        snprintf(label, sizeof(label), "side%d.creature_row", i + 1);
        expect_int(label, f ? f->creature_row : -1, want_creature_row[i],
                   A_F0115_CREATURE);
        snprintf(label, sizeof(label), "side%d.creature_cell_gate", i + 1);
        expect_int(label, f ? f->creature_cell_gate : -1, want_creature_cell[i],
                   A_F0115_CREATURE);
        snprintf(label, sizeof(label), "side%d.creature_zone.good", i + 1);
        expect_int(label,
                   csb_v1_viewport_d0l2_d0r2_f0115_creature_zone_pc34(
                       f, want_creature_cell[i]),
                   0x8000 | (3200 + want_creature_row[i] * 5 + want_creature_cell[i]),
                   A_F0115_CREATURE);
        snprintf(label, sizeof(label), "side%d.creature_zone.bad", i + 1);
        expect_int(label,
                   csb_v1_viewport_d0l2_d0r2_f0115_creature_zone_pc34(
                       f, i == 0 ? 3 : 2),
                   -1, A_F0115_CREATURE);
        snprintf(label, sizeof(label), "side%d.explosion_row", i + 1);
        expect_int(label, f ? f->explosion_row : -1, want_explosion_row[i],
                   A_F0115_EXPLOSION);
        snprintf(label, sizeof(label), "side%d.centered_explosion", i + 1);
        expect_int(label,
                   csb_v1_viewport_d0l2_d0r2_f0115_centered_explosion_zone_pc34(f),
                   3014 + want_explosion_row[i], A_F0115_EXPLOSION);
        snprintf(label, sizeof(label), "side%d.side_explosion.front_left", i + 1);
        expect_int(label,
                   csb_v1_viewport_d0l2_d0r2_f0115_side_explosion_zone_pc34(f, 0),
                   3031 + want_explosion_row[i] * 2, A_F0115_EXPLOSION);
        snprintf(label, sizeof(label), "side%d.side_explosion.front_right", i + 1);
        expect_int(label,
                   csb_v1_viewport_d0l2_d0r2_f0115_side_explosion_zone_pc34(f, 1),
                   3031 + want_explosion_row[i] * 2 + 1, A_F0115_EXPLOSION);
        snprintf(label, sizeof(label), "side%d.side_explosion.bad_cell", i + 1);
        expect_int(label,
                   csb_v1_viewport_d0l2_d0r2_f0115_side_explosion_zone_pc34(f, 2),
                   -1, A_F0115_EXPLOSION);
        snprintf(label, sizeof(label), "side%d.field_aspect", i + 1);
        expect_int(label, f ? f->field_aspect_index : -1, want_field[i],
                   "ReDMCSB DUNVIEW.C:377 G2035[1/2]");
        snprintf(label, sizeof(label), "side%d.wall_route_excluded", i + 1);
        expect_int(label, f ? f->wall_route_excluded : 0, 1, anchor_for_index(i));
        snprintf(label, sizeof(label), "side%d.no_f0107", i + 1);
        expect_int(label, f ? f->no_f0107_contract : 0, 1, anchor_for_index(i));
        snprintf(label, sizeof(label), "side%d.no_f0111", i + 1);
        expect_int(label, f ? f->no_f0111_contract : 0, 1, anchor_for_index(i));
        snprintf(label, sizeof(label), "side%d.no_custom_backgrounds", i + 1);
        expect_int(label, f ? f->no_custom_backgrounds_contract : 0, 1,
                   A_LINEAGE_OPEN);
        snprintf(label, sizeof(label), "side%d.draw_order.objects", i + 1);
        expect_int(label, f ? f->f0115_draw_order_objects_first : 0, 1, A_F0115);
        snprintf(label, sizeof(label), "side%d.draw_order.creatures", i + 1);
        expect_int(label, f ? f->f0115_draw_order_creatures_second : 0, 1,
                   A_F0115_CREATURE);
        snprintf(label, sizeof(label), "side%d.draw_order.projectiles", i + 1);
        expect_int(label, f ? f->f0115_draw_order_projectiles_third : 0, 1,
                   A_F0115_PROJECTILE);
        snprintf(label, sizeof(label), "side%d.draw_order.explosions", i + 1);
        expect_int(label, f ? f->f0115_draw_order_explosions_last : 0, 1,
                   A_F0115_EXPLOSION);
        snprintf(label, sizeof(label), "side%d.f0674_copy_marker", i + 1);
        expect_int(label, f ? f->f0674_per_frame_bitmap_copy : 0, 1, A_F0674);
        snprintf(label, sizeof(label), "side%d.wall_frame_field_marker", i + 1);
        expect_int(label, f ? f->wall_set_frame_used_for_field : 0, 1, A_FRAMES);
        snprintf(label, sizeof(label), "side%d.wall_order_marker", i + 1);
        expect_int(label, f ? f->wall_set_draw_order_d0l_before_d0r : 0, 1,
                   A_DISPATCH);
    }

    expect_int("null.item",
               csb_v1_viewport_d0l2_d0r2_f0115_item_zone_pc34(NULL, 0),
               -1, "invalid input guard");
    expect_int("null.projectile",
               csb_v1_viewport_d0l2_d0r2_f0115_projectile_zone_pc34(NULL, 0),
               -1, "invalid input guard");
    expect_int("null.creature",
               csb_v1_viewport_d0l2_d0r2_f0115_creature_zone_pc34(NULL, 2),
               -1, "invalid input guard");
    expect_int("null.centered_explosion",
               csb_v1_viewport_d0l2_d0r2_f0115_centered_explosion_zone_pc34(NULL),
               -1, "invalid input guard");
    expect_int("null.side_explosion",
               csb_v1_viewport_d0l2_d0r2_f0115_side_explosion_zone_pc34(NULL, 0),
               -1, "invalid input guard");
}

static void test_csb_lineage_cross_references(void)
{
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *d0l2 = fixture_for_index(0);
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *d0r2 = fixture_for_index(1);

    expect_int("d0l2.lineage.relative_cell",
               d0l2 ? d0l2->csb_lineage_relative_cell : -1, 18,
               A_LINEAGE_OPEN);
    expect_int("d0r2.lineage.relative_cell",
               d0r2 ? d0r2->csb_lineage_relative_cell : -1, 19,
               A_LINEAGE_OPEN);
    expect_int("d0l2.lineage.contents",
               d0l2 ? d0l2->csb_lineage_contents_opcode : -1, 60128,
               A_LINEAGE_OPEN);
    expect_int("d0r2.lineage.contents",
               d0r2 ? d0r2->csb_lineage_contents_opcode : -1, 60130,
               A_LINEAGE_OPEN);
    expect_int("d0l2.lineage.draw_order",
               d0l2 ? d0l2->csb_lineage_draw_order_opcode : -1, 60288,
               A_LINEAGE_OPEN);
    expect_int("d0r2.lineage.draw_order",
               d0r2 ? d0r2->csb_lineage_draw_order_opcode : -1, 60287,
               A_LINEAGE_OPEN);
    expect_int("d0l2.lineage.std_room_objects",
               d0l2 ? d0l2->csb_lineage_std_draw_room_objects_opcode : -1,
               60006, A_LINEAGE_OPEN);
    expect_int("d0r2.lineage.std_room_objects",
               d0r2 ? d0r2->csb_lineage_std_draw_room_objects_opcode : -1,
               60006, A_LINEAGE_OPEN);
    expect_int("d0l2.center_door.first_order",
               d0l2 ? d0l2->csb_lineage_center_door_draw_order_first : -1,
               60279, A_LINEAGE_CENTER);
    expect_int("d0r2.center_door.first_order",
               d0r2 ? d0r2->csb_lineage_center_door_draw_order_first : -1,
               60279, A_LINEAGE_CENTER);
    expect_int("d0l2.center_door.second_order",
               d0l2 ? d0l2->csb_lineage_center_door_draw_order_second : -1,
               60280, A_LINEAGE_CENTER);
    expect_int("d0r2.center_door.second_order",
               d0r2 ? d0r2->csb_lineage_center_door_draw_order_second : -1,
               60280, A_LINEAGE_CENTER);
    expect_contains("d0l2.dispatch_anchor",
                    d0l2 ? d0l2->redmcsb_dispatch_anchor : NULL,
                    "8003/8005/8059", A_D0L);
    expect_contains("d0r2.dispatch_anchor",
                    d0r2 ? d0r2->redmcsb_dispatch_anchor : NULL,
                    "8113/8115/8159", A_D0R);
    expect_contains("d0l2.f0115_anchor",
                    d0l2 ? d0l2->redmcsb_f0115_anchor : NULL,
                    "5295", A_F0115_CREATURE);
    expect_contains("d0r2.f0115_anchor",
                    d0r2 ? d0r2->redmcsb_f0115_anchor : NULL,
                    "5668-5683", A_F0115_PROJECTILE);
}

static void test_real_graphics_dat_frame_receipts(void)
{
    const char *path = getenv("FIRESTAFF_CSB_GRAPHICS_DAT");
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *d0l2 = fixture_for_index(0);
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *d0r2 = fixture_for_index(1);
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixtures[2] = {d0l2, d0r2};
    int i;

    if (!path || path[0] == '\0') {
        path = "/Users/bosse/.firestaff/data/csb/GRAPHICS.DAT";
    }

    for (i = 0; i < 2; ++i) {
        const CSB_V1_D0L2D0R2F0115ThingPassPc34 *f = fixtures[i];
        CSB_V1_D0L2D0R2F0115ThingPassRealAssetReceiptPc34 receipt;
        size_t payload_size = 0u;
        uint32_t payload_hash = 0u;
        char label[96];

        if (!f ||
            !read_real_graphics_item_hash(path, (unsigned)f->wall_frame_row,
                                          &payload_size, &payload_hash)) {
            printf("SKIP real GRAPHICS.DAT frame row %d path=%s\n",
                   f ? f->wall_frame_row : -1, path);
            continue;
        }

        snprintf(label, sizeof(label), "real_frame%d.size_positive", i + 1);
        expect_int(label, payload_size > 0u, 1, A_FRAMES);
        snprintf(label, sizeof(label), "real_frame%d.hash_nonzero", i + 1);
        expect_int(label, payload_hash != 0u, 1,
                   "real GRAPHICS.DAT payload hash, no decoded-pixel claim");
        snprintf(label, sizeof(label), "real_frame%d.receipt", i + 1);
        expect_int(label,
                   csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_real_asset_receipt_pc34(
                       f, 1, 1, 1, f->wall_frame_row, payload_size,
                       payload_hash, &receipt),
                   1, A_F0674);
        snprintf(label, sizeof(label), "real_frame%d.valid", i + 1);
        expect_int(label, receipt.valid, 1, A_F0674);
        snprintf(label, sizeof(label), "real_frame%d.item", i + 1);
        expect_int(label, receipt.source_graphics_item_index,
                   f->wall_frame_row, A_FRAMES);
        snprintf(label, sizeof(label), "real_frame%d.hash_preserved", i + 1);
        expect_int(label, receipt.source_payload_hash == payload_hash, 1,
                   "receipt preserves measured GRAPHICS.DAT payload hash");
        snprintf(label, sizeof(label), "real_frame%d.no_synthetic", i + 1);
        expect_int(label, receipt.no_synthetic_pixels, 1,
                   "no generated viewport pixels");
        snprintf(label, sizeof(label), "real_frame%d.no_fallback", i + 1);
        expect_int(label, receipt.no_fallback_visuals, 1,
                   "fallback visuals remain closed");
        snprintf(label, sizeof(label), "real_frame%d.g2028_disabled", i + 1);
        expect_int(label, receipt.item_projectile_disabled_by_g2028, 1,
                   A_F0115_ITEM);

        snprintf(label, sizeof(label), "real_frame%d.wrong_index", i + 1);
        expect_int(label,
                   csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_real_asset_receipt_pc34(
                       f, 1, 1, 1, f->wall_frame_row + 1, payload_size,
                       payload_hash, &receipt),
                   0, "fail closed on wrong GRAPHICS.DAT item index");
        snprintf(label, sizeof(label), "real_frame%d.synthetic", i + 1);
        expect_int(label,
                   csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_real_asset_receipt_pc34(
                       f, 1, 0, 1, f->wall_frame_row, payload_size,
                       payload_hash, &receipt),
                   0, "fail closed on synthetic pixels");
        snprintf(label, sizeof(label), "real_frame%d.fallback", i + 1);
        expect_int(label,
                   csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_real_asset_receipt_pc34(
                       f, 1, 1, 0, f->wall_frame_row, payload_size,
                       payload_hash, &receipt),
                   0, "fail closed on fallback visuals");
        snprintf(label, sizeof(label), "real_frame%d.zero_hash", i + 1);
        expect_int(label,
                   csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_real_asset_receipt_pc34(
                       f, 1, 1, 1, f->wall_frame_row, payload_size,
                       0u, &receipt),
                   0, "fail closed on missing payload hash");
    }
}

static void test_evidence_strings(void)
{
    const char *e =
        csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_source_evidence_pc34();
    const CSB_V1_D0L2D0R2F0115ThingPassEvidencePc34 *ev =
        csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_evidence_pc34();

    expect_contains("evidence.contract", e, "contract-only", A_F0115);
    expect_contains("evidence.contract_marker", e, "source_locked_contract_only=1",
                    A_F0115);
    expect_contains("evidence.no_asset", e, "no_real_asset_bitmap_parity=1",
                    A_F0115);
    expect_contains("evidence.no_game_data", e, "no_game_data_load=1",
                    A_F0115);
    expect_contains("evidence.f0125", e, "DUNVIEW.C:7960-8062 F0125", A_D0L);
    expect_contains("evidence.f0126", e, "DUNVIEW.C:8064-8162 F0126", A_D0R);
    expect_contains("evidence.f0128", e, "DUNVIEW.C:8536-8541", A_DISPATCH);
    expect_contains("evidence.f0674", e, "DUNVIEW.C:2995-3015", A_F0674);
    expect_contains("evidence.frames", e, "DUNVIEW.C:581-594 G0163", A_FRAMES);
    expect_contains("evidence.frame_left", e, "{0,31,0,135,16,136,0,0}",
                    A_FRAMES);
    expect_contains("evidence.frame_right", e, "{192,223,0,135,16,136,0,0}",
                    A_FRAMES);
    expect_contains("evidence.f0115", e, "DUNVIEW.C:4547-4581", A_F0115);
    expect_contains("evidence.item", e, "4923", A_F0115_ITEM);
    expect_contains("evidence.projectile", e, "5668-5683", A_F0115_PROJECTILE);
    expect_contains("evidence.creature", e, "C3200", A_F0115_CREATURE);
    expect_contains("evidence.explosion", e, "C3000/C3014/C3031",
                    A_F0115_EXPLOSION);
    expect_contains("evidence.defs_c10", e, "DEFS.H:2088", A_DEFS);
    expect_contains("evidence.defs_views", e, "DEFS.H:2596-2598", A_DEFS);
    expect_contains("evidence.defs_cells", e, "DEFS.H:2642-2660", A_DEFS);
    expect_contains("evidence.defs_zones", e, "DEFS.H:4056-4057", A_DEFS);
    expect_contains("evidence.lineage_open", e, "Viewport.cpp:1192-1209",
                    A_LINEAGE_OPEN);
    expect_contains("evidence.lineage_center", e, "Viewport.cpp:1903-1915",
                    A_LINEAGE_CENTER);
    expect_contains("struct.scope", ev ? ev->contract_scope : NULL,
                    "no real-asset", A_F0115);
    expect_contains("struct.f0125", ev ? ev->f0125_d0l_lines : NULL,
                    "F0125_DUNGEONVIEW_DrawSquareD0L", A_D0L);
    expect_contains("struct.f0126", ev ? ev->f0126_d0r_lines : NULL,
                    "F0126_DUNGEONVIEW_DrawSquareD0R", A_D0R);
    expect_contains("struct.dispatch", ev ? ev->f0128_dispatch_lines : NULL,
                    "8536-8541", A_DISPATCH);
    expect_contains("struct.f0115", ev ? ev->f0115_lines : NULL,
                    "5668-5683", A_F0115_PROJECTILE);
    expect_contains("struct.f0674", ev ? ev->f0674_lines : NULL,
                    "F0674_F0128_sub", A_F0674);
    expect_contains("struct.frames", ev ? ev->frame_lines : NULL,
                    "G0163", A_FRAMES);
    expect_contains("struct.defs", ev ? ev->defs_lines : NULL,
                    "4228-4236", A_DEFS);
    expect_contains("struct.lineage_open", ev ? ev->csb_lineage_open_lines : NULL,
                    "1192-1209", A_LINEAGE_OPEN);
    expect_contains("struct.lineage_center",
                    ev ? ev->csb_lineage_center_door_lines : NULL,
                    "1903-1915", A_LINEAGE_CENTER);
}

static void test_source_bound_teleporter_field_composite(void)
{
    unsigned char source[32u * 136u];
    unsigned char viewport[224u * 136u];
    CSB_V1_D0L2D0R2F0115TeleporterSourceRasterPc34 raster;
    CSB_V1_D0L2D0R2F0115TeleporterRenderReceiptPc34 receipt;
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *left = fixture_for_index(0);
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *right = fixture_for_index(1);
    size_t i;

    memset(source, 0x6au, sizeof(source));
    memset(viewport, 0x55, sizeof(viewport));
    source[0] = 10u;
    raster = (CSB_V1_D0L2D0R2F0115TeleporterSourceRasterPc34){
        1, 1, 1, left->field_aspect_index, source, sizeof(source),
        32, 136, 32, fnv1a32(source, sizeof(source))
    };
    expect_int("teleporter.left.render",
               csb_v1_viewport_d0l2_d0r2_f0115_render_teleporter_field_pc34(
                   left, &raster, viewport, sizeof(viewport), 224, &receipt),
               1, A_D0L);
    expect_int("teleporter.left.receipt.valid", receipt.valid, 1, A_D0L);
    expect_int("teleporter.left.receipt.field", receipt.field_aspect_index,
               left->field_aspect_index, A_D0L);
    expect_int("teleporter.left.transparent", (int)receipt.transparent_pixels,
               1, A_DEFS);
    expect_int("teleporter.left.copied", (int)receipt.copied_pixels,
               32 * 136 - 1, A_D0L);
    expect_int("teleporter.left.c10_kept", viewport[0], 0x55, A_DEFS);
    expect_int("teleporter.left.real_pixel", viewport[1], 0x6a, A_D0L);
    expect_int("teleporter.left.outside_kept", viewport[32], 0x55, A_D0L);

    memset(viewport, 0x55, sizeof(viewport));
    raster.source_graphics_item_index = right->field_aspect_index;
    expect_int("teleporter.right.render",
               csb_v1_viewport_d0l2_d0r2_f0115_render_teleporter_field_pc34(
                   right, &raster, viewport, sizeof(viewport), 224, &receipt),
               1, A_D0R);
    expect_int("teleporter.right.origin", viewport[192], 0x55, A_DEFS);
    expect_int("teleporter.right.pixel", viewport[193], 0x6a, A_D0R);
    expect_int("teleporter.right.outside_kept", viewport[191], 0x55, A_D0R);

    raster.source_graphics_item_index = left->field_aspect_index;
    raster.no_synthetic_pixels = 0;
    expect_int("teleporter.reject_synthetic",
               csb_v1_viewport_d0l2_d0r2_f0115_render_teleporter_field_pc34(
                   left, &raster, viewport, sizeof(viewport), 224, &receipt),
               0, "source-owned teleporter material only");
    raster.no_synthetic_pixels = 1;
    raster.width = 31;
    expect_int("teleporter.reject_partial_geometry",
               csb_v1_viewport_d0l2_d0r2_f0115_render_teleporter_field_pc34(
                   left, &raster, viewport, sizeof(viewport), 224, &receipt),
               0, "complete G0163 lane required");

    /* Ensure the fixture filled every source pixel deterministically. */
    for (i = 1u; i < sizeof(source); ++i) {
        if (source[i] != 0x6au) {
            expect_int("teleporter.fixture.source", 0, 1, A_D0L);
            break;
        }
    }
}

static void test_cache_bound_teleporter_field_decode(void)
{
    unsigned char palette[CSB_V1_CSBGRAPHICS_DAT_PALETTE_BYTES];
    unsigned char field[32u * 136u];
    unsigned char viewport[224u * 136u];
    unsigned char *cache_bytes;
    size_t cache_size = 0u;
    CSB_V1_CSBGraphicsDatRealCache cache;
    CSB_V1_CSBGraphicsDatPaletteSourceReceipt palette_receipt;
    CSB_V1_CSBGraphicsEntrySpan palette_span;
    CSB_V1_D0L2D0R2F0115CacheTeleporterRequestPc34 request;
    CSB_V1_D0L2D0R2F0115CacheTeleporterReceiptPc34 receipt;
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *left = fixture_for_index(0);
    uint32_t before;
    size_t i;

    for (i = 0u; i < sizeof(palette); ++i) palette[i] = (unsigned char)(i & 0x3fu);
    memset(field, 0x6au, sizeof(field));
    field[0] = 10u;
    cache_bytes = build_cache_graphics(palette, field, &cache_size);
    expect_int("cache_field.fixture", cache_bytes != NULL, 1, A_F0115);
    if (!cache_bytes) return;

    memset(&cache, 0, sizeof(cache));
    cache.file_buffer = cache_bytes;
    cache.file_size = cache_size;
    cache.loaded = 1;
    strcpy(cache.resolved_path, "/verified/CSBGRAPHICS.DAT");
    strcpy(cache.matched_md5, "0123456789abcdef0123456789abcdef");
    expect_int("cache_field.classify",
               csb_v1_csbgraphics_dat_classify(cache.file_buffer, cache.file_size,
                                                &cache.index),
               CSB_V1_CSBGRAPHICS_CLASSIFY_OK, A_F0115);
    expect_int("cache_field.palette_span",
               csb_v1_csbgraphics_dat_entry_span(cache.file_buffer, cache.file_size,
                                                  0u, &palette_span),
               CSB_V1_CSBGRAPHICS_CLASSIFY_OK, A_F0115);

    memset(&palette_receipt, 0, sizeof(palette_receipt));
    palette_receipt.valid = 1;
    strcpy(palette_receipt.source_path, cache.resolved_path);
    strcpy(palette_receipt.source_md5, cache.matched_md5);
    palette_receipt.entry_span = palette_span;
    memcpy(palette_receipt.decoded_bytes, palette, sizeof(palette));
    palette_receipt.decoded_fnv1a = fnv1a32(palette, sizeof(palette));
    request.csbgraphics_entry_index = 1u;
    request.palette_receipt = &palette_receipt;
    memset(viewport, 0x55, sizeof(viewport));
    expect_int("cache_field.render",
               csb_v1_viewport_d0l2_d0r2_f0115_render_teleporter_field_from_cache_pc34(
                   left, &cache, &request, viewport, sizeof(viewport), 224, &receipt),
               1, A_D0L);
    expect_int("cache_field.receipt", receipt.valid, 1, A_D0L);
    expect_int("cache_field.archive", receipt.consumed_hash_admitted_csbgraphics_dat,
               1, A_F0115);
    expect_int("cache_field.entry", (int)receipt.csbgraphics_entry_index, 1, A_F0115);
    expect_int("cache_field.transparent", (int)receipt.transparent_pixels, 1, A_DEFS);
    expect_int("cache_field.pixel", viewport[1], 0x6a, A_D0L);
    expect_int("cache_field.c10_kept", viewport[0], 0x55, A_DEFS);

    before = fnv1a32(viewport, sizeof(viewport));
    palette_receipt.decoded_bytes[0] ^= 0x01u;
    expect_int("cache_field.reject_stale_palette",
               csb_v1_viewport_d0l2_d0r2_f0115_render_teleporter_field_from_cache_pc34(
                   left, &cache, &request, viewport, sizeof(viewport), 224, &receipt),
               0, "same archive palette receipt required");
    expect_int("cache_field.reject_keeps_viewport",
               fnv1a32(viewport, sizeof(viewport)) == before, 1,
               "no draw before source admission");

    free(cache_bytes);
}

int main(void)
{
    printf("probe=csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_source_evidence_pc34());

    test_accessors_and_contract_markers();
    test_view_square_frame_and_route_metadata();
    test_clip_no_write_and_blend_contract();
    test_zone_bindings_and_draw_order();
    test_csb_lineage_cross_references();
    test_real_graphics_dat_frame_receipts();
    test_source_bound_teleporter_field_composite();
    test_cache_bound_teleporter_field_decode();
    test_evidence_strings();

    expect_int("assertion_count_at_least_100", g_assertions >= 100, 1, A_F0115);
    if (g_failures) {
        printf("FAIL csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_pc34_compat "
               "failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_pc34_compat "
           "%d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
