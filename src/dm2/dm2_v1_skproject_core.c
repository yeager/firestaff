#include "dm2_v1_skproject_core.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define DM2_V1_SKPROJECT_RANDOM_MAGIC 0xbb40e62du

static uint32_t dm2_v1_skproject_rect_hash(
    uint8_t slot,
    uint8_t next_slot,
    const DM2_V1_SkprojectRect *rect)
{
    uint32_t hash = 2166136261u;

    hash = (hash ^ slot) * 16777619u;
    hash = (hash ^ next_slot) * 16777619u;
    hash = (hash ^ (uint16_t)rect->x) * 16777619u;
    hash = (hash ^ (uint16_t)rect->y) * 16777619u;
    hash = (hash ^ (uint16_t)rect->w) * 16777619u;
    hash = (hash ^ (uint16_t)rect->h) * 16777619u;
    return hash ? hash : 1u;
}

static uint32_t dm2_v1_skproject_hash_bytes(const void *data, size_t size)
{
    const uint8_t *bytes = (const uint8_t *)data;
    uint32_t hash = 2166136261u;

    for (size_t i = 0; i < size; ++i)
        hash = (hash ^ bytes[i]) * 16777619u;
    return hash ? hash : 1u;
}

int16_t dm2_v1_skproject_between_value(int16_t minv,
                                       int16_t newv,
                                       int16_t maxv)
{
    if (newv < minv) return minv;
    if (newv > maxv) return maxv;
    return newv;
}

int16_t dm2_v1_skproject_dm2_between_value(int16_t minv,
                                           int16_t maxv,
                                           int16_t value)
{
    return dm2_v1_skproject_between_value(minv, value, maxv);
}

int16_t dm2_v1_skproject_abs(int16_t value)
{
    return (value >= 0) ? value : (int16_t)-value;
}

int16_t dm2_v1_skproject_calc_square_distance(int16_t from_x,
                                               int16_t from_y,
                                               int16_t to_x,
                                               int16_t to_y)
{
    return (int16_t)(dm2_v1_skproject_abs((int16_t)(from_x - to_x)) +
                     dm2_v1_skproject_abs((int16_t)(from_y - to_y)));
}

void dm2_v1_skproject_temp_rect_ring_init(
    DM2_V1_SkprojectTempRectRing *ring)
{
    if (!ring) return;
    memset(ring, 0, sizeof(*ring));
}

int dm2_v1_skproject_alloc_temp_rect(
    DM2_V1_SkprojectTempRectRing *ring,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    DM2_V1_SkprojectTempRectReceipt *out_receipt)
{
    uint8_t slot;
    DM2_V1_SkprojectRect *rect;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!ring || !out_receipt || ring->next_index >= 4u) return 0;

    slot = ring->next_index;
    rect = &ring->rects[slot];
    ring->next_index = (uint8_t)((slot + 1u) & 3u);

    rect->x = x;
    rect->y = y;
    rect->w = w;
    rect->h = h;

    out_receipt->valid = 1;
    out_receipt->slot = slot;
    out_receipt->next_slot = ring->next_index;
    out_receipt->rect = *rect;
    out_receipt->receipt_hash =
        dm2_v1_skproject_rect_hash(slot, ring->next_index, rect);
    return 1;
}

int dm2_v1_skproject_alloc_temp_origin_rect(
    DM2_V1_SkprojectTempRectRing *ring,
    int16_t w,
    int16_t h,
    DM2_V1_SkprojectTempRectReceipt *out_receipt)
{
    return dm2_v1_skproject_alloc_temp_rect(ring, 0, 0, w, h, out_receipt);
}

void dm2_v1_skproject_random_init(DM2_V1_SkprojectRandomData *randdat)
{
    if (!randdat) return;
    randdat->random = 0u;
}

uint32_t dm2_v1_skproject_rand(DM2_V1_SkprojectRandomData *randdat)
{
    uint32_t value;

    if (!randdat) return 0u;
    value = randdat->random * DM2_V1_SKPROJECT_RANDOM_MAGIC + 11u;
    randdat->random = value;
    return value >> 8;
}

uint16_t dm2_v1_skproject_rand16(DM2_V1_SkprojectRandomData *randdat,
                                 uint16_t max_value)
{
    if (!randdat || max_value == 0u) return 0u;
    return (uint16_t)(dm2_v1_skproject_rand(randdat) % (uint32_t)max_value);
}

int dm2_v1_skproject_randbit(DM2_V1_SkprojectRandomData *randdat)
{
    return (int)(dm2_v1_skproject_rand(randdat) & 1u);
}

uint8_t dm2_v1_skproject_randdir(DM2_V1_SkprojectRandomData *randdat)
{
    return (uint8_t)(dm2_v1_skproject_rand(randdat) & 3u);
}

int dm2_v1_skproject_calc_vector_dir(
    DM2_V1_SkprojectRandomData *randdat,
    int16_t from_x,
    int16_t from_y,
    int16_t to_x,
    int16_t to_y,
    DM2_V1_SkprojectVectorDirReceipt *out_receipt)
{
    DM2_V1_SkprojectVectorDirReceipt receipt;
    int16_t abs_x;
    int16_t abs_y;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.from_x = from_x;
    receipt.from_y = from_y;
    receipt.to_x = to_x;
    receipt.to_y = to_y;
    receipt.delta_x = (int16_t)(from_x - to_x);
    receipt.delta_y = (int16_t)(from_y - to_y);
    abs_x = dm2_v1_skproject_abs(receipt.delta_x);
    abs_y = dm2_v1_skproject_abs(receipt.delta_y);
    receipt.abs_delta_x = abs_x;
    receipt.abs_delta_y = abs_y;

    if (abs_x == abs_y) {
        receipt.tied_axes = 1;
        if (!randdat) {
            receipt.blocked_missing_random = 1;
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
        receipt.randbit = (uint8_t)(dm2_v1_skproject_randbit(randdat) ? 1 : 0);
        receipt.consumed_randbit = 1;
        if (receipt.randbit == 0)
            abs_y++;
        else
            abs_x++;
        receipt.abs_delta_x = abs_x;
        receipt.abs_delta_y = abs_y;
    }

    if (abs_x >= abs_y)
        receipt.dir = (receipt.delta_x <= 0) ? 1u : 3u;
    else
        receipt.dir = (receipt.delta_y <= 0) ? 2u : 0u;
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_skproject_calc_vector_w_dir(
    int16_t dir,
    int16_t xx,
    int16_t yy,
    int16_t *x,
    int16_t *y,
    DM2_V1_SkprojectVectorWDirReceipt *out_receipt)
{
    static const int16_t x_delta[4] = { 0, 1, 0, -1 };
    static const int16_t y_delta[4] = { -1, 0, 1, 0 };
    DM2_V1_SkprojectVectorWDirReceipt receipt;
    uint8_t idx;
    uint8_t side_idx;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.dir = (uint8_t)((uint16_t)dir & 3u);
    receipt.input_xx = xx;
    receipt.input_yy = yy;
    if (!x || !y) {
        receipt.blocked_missing_output = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    idx = receipt.dir;
    side_idx = (uint8_t)((idx + 1u) & 3u);
    receipt.initial_x = *x;
    receipt.initial_y = *y;
    receipt.forward_dx = (int16_t)(xx * x_delta[idx]);
    receipt.forward_dy = (int16_t)(xx * y_delta[idx]);
    receipt.side_dx = (int16_t)(yy * x_delta[side_idx]);
    receipt.side_dy = (int16_t)(yy * y_delta[side_idx]);
    *x = (int16_t)(*x + receipt.forward_dx + receipt.side_dx);
    *y = (int16_t)(*y + receipt.forward_dy + receipt.side_dy);
    receipt.final_x = *x;
    receipt.final_y = *y;
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int32_t dm2_v1_skproject_compute_power_4_within(int16_t mask,
                                                int16_t ordinal)
{
    uint32_t result = 1u;
    int16_t remaining = ordinal;

    for (int16_t n = 0; n < 32; ++n) {
        if ((result & (uint16_t)mask) != 0u) {
            remaining = (int16_t)(remaining - 1);
            if (remaining == 0)
                return (int32_t)result;
        }
        result <<= 1;
    }
    return (int32_t)result;
}

int dm2_v1_skproject_fill_i16table(
    int16_t *table,
    int16_t value,
    uint16_t entries,
    DM2_V1_SkprojectFillI16TableReceipt *out_receipt)
{
    DM2_V1_SkprojectFillI16TableReceipt receipt;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.value = value;
    receipt.entries = entries;
    if (!table && entries != 0u) {
        receipt.blocked_missing_table = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    for (uint16_t i = 0; i < entries; ++i)
        table[i] = value;
    receipt.written_entries = entries;
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_skproject_palettecolor_from_color(uint8_t color,
                                             uint8_t *out_palette)
{
    if (!out_palette) return 0;
    *out_palette = color;
    return 1;
}

int dm2_v1_skproject_palettecolor_from_ui8(uint8_t color,
                                           uint8_t *out_palette)
{
    if (!out_palette) return 0;
    *out_palette = color;
    return 1;
}

int dm2_v1_skproject_palettecolor_to_ui8(uint8_t palette,
                                         uint8_t *out_color)
{
    if (!out_color) return 0;
    *out_color = palette;
    return 1;
}

int dm2_v1_skproject_palettecolor_to_pixel(uint8_t palette,
                                           uint8_t *out_pixel)
{
    if (!out_pixel) return 0;
    *out_pixel = palette;
    return 1;
}

int dm2_v1_skproject_convert_driverpalette(
    const uint8_t *alpha_rgb8_palette,
    int immediate_colors_before,
    DM2_V1_SkprojectDriverPaletteReceipt *out_receipt)
{
    DM2_V1_SkprojectDriverPaletteReceipt receipt;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.immediate_colors_before = immediate_colors_before ? 1 : 0;
    if (!alpha_rgb8_palette) {
        receipt.blocked_missing_input = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    for (uint32_t i = 0; i < 256u; ++i) {
        const uint8_t *src = alpha_rgb8_palette + i * 4u;
        receipt.dmpal6[i][0] = (uint8_t)(src[1] >> 2);
        receipt.dmpal6[i][1] = (uint8_t)(src[2] >> 2);
        receipt.dmpal6[i][2] = (uint8_t)(src[3] >> 2);
    }
    receipt.converted_entries = 256u;
    receipt.driver_setcolors_requested = receipt.immediate_colors_before;
    receipt.dmpal_hash =
        dm2_v1_skproject_hash_bytes(receipt.dmpal6, sizeof(receipt.dmpal6));
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_skproject_select_palette_set(
    int16_t set,
    DM2_V1_SkprojectPaletteSetReceipt *out_receipt)
{
    DM2_V1_SkprojectPaletteSetReceipt receipt;
    uint8_t bmode = (uint8_t)set;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.set = bmode;
    if (bmode == 0u) {
        receipt.fade_to_black_requested = 1;
        receipt.vsync_waits = 2000u;
    } else if (bmode == 1u) {
        receipt.driver_setcolors_requested = 1;
    }
    receipt.immediate_colors_after = bmode == 1u;
    receipt.valid = 1;
    receipt.receipt_hash = dm2_v1_skproject_hash_bytes(&receipt.set,
                                                       sizeof(receipt) -
                                                           offsetof(DM2_V1_SkprojectPaletteSetReceipt, set));
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_skproject_update_blit_palette(
    const uint8_t *palette,
    uint16_t colors,
    const uint8_t **out_palette_ptr)
{
    (void)colors;
    if (!out_palette_ptr) return 0;
    *out_palette_ptr = palette;
    return 1;
}

int dm2_v1_skproject_xlat_palette(
    const uint8_t *palette,
    uint16_t colors,
    const uint8_t *conversion_table,
    DM2_V1_SkprojectXlatPaletteReceipt *out_receipt)
{
    DM2_V1_SkprojectXlatPaletteReceipt receipt;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.colors_before = colors;
    if (!conversion_table || (colors != 0u && !palette) || colors > 256u) {
        receipt.blocked_missing_output = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    if (colors == 0u) {
        memcpy(receipt.palette, conversion_table, sizeof(receipt.palette));
        receipt.colors_after = 256u;
        receipt.converted_colors = 256u;
        receipt.large_palette_copy = 1u;
    } else {
        for (uint16_t i = 0; i < colors; ++i)
            receipt.palette[i] = conversion_table[palette[i]];
        receipt.colors_after = colors;
        receipt.converted_colors = colors;
    }
    receipt.palette_hash =
        dm2_v1_skproject_hash_bytes(receipt.palette, sizeof(receipt.palette));
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_skproject_move_side_offset(
    uint16_t record_word_e,
    int16_t direction_delta,
    uint8_t creature_5x5_pos,
    DM2_V1_SkprojectMoveSideOffsetReceipt *out_receipt)
{
    DM2_V1_SkprojectMoveSideOffsetReceipt receipt;
    uint8_t facing;
    uint8_t relative_direction;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.record_word_e = record_word_e;
    receipt.direction_delta = direction_delta;
    receipt.creature_5x5_pos = creature_5x5_pos;

    facing = (uint8_t)((record_word_e >> 8) & 3u);
    relative_direction =
        (uint8_t)((facing + (uint8_t)direction_delta) & 3u);
    receipt.facing = facing;
    receipt.relative_direction = relative_direction;
    if (relative_direction == 1u || relative_direction == 3u) {
        receipt.side_direction = 1;
        receipt.creature_x_offset =
            (int8_t)((int)(creature_5x5_pos % 5u) - 2);
        receipt.side_offset_nonzero = receipt.creature_x_offset != 0;
    }
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return receipt.side_offset_nonzero;
}

int dm2_v1_skproject_move_admission(
    const DM2_V1_SkprojectMoveAdmissionRequest *request,
    DM2_V1_SkprojectMoveAdmissionReceipt *out_receipt)
{
    DM2_V1_SkprojectMoveAdmissionReceipt receipt;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.stored_creature = -1;
    if (!request) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    receipt.current_tile_type =
        (uint8_t)(((uint8_t)request->current_tile_value >> 5) & 0xffu);
    receipt.destination_tile_type =
        (uint8_t)(((uint8_t)request->destination_tile_value >> 5) & 0xffu);

    if (receipt.current_tile_type == 3u && request->requested_move == 2u) {
        receipt.result_code = 1u;
    } else if (receipt.destination_tile_type == 3u) {
        receipt.result_code = 2u;
    } else if (request->destination_tile_blocked) {
        receipt.result_code = 3u;
    } else {
        receipt.stored_creature = request->creature_at_destination;
        if (request->creature_at_destination != -1) {
            if ((request->creature_ai_flags & 0x8000u) == 0u) {
                receipt.used_side_offset_test = 1;
                receipt.result_code =
                    request->side_offset_nonzero ? 5u : 4u;
            }
        } else {
            receipt.used_secondary_query = 1;
            if (request->secondary_query_creature == -1) {
                receipt.result_code = 6u;
            } else {
                receipt.result_code =
                    (request->secondary_query_ai_flags & 0x8000u) == 0u ?
                        5u : 6u;
            }
        }
    }

    if (receipt.result_code == 0u)
        receipt.result_code = 6u;
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return (int)receipt.result_code;
}

int dm2_v1_skproject_set_destination_of_minion_map(
    uint16_t previous_record_w6,
    int16_t current_map,
    int16_t destination_x,
    int16_t destination_y,
    int16_t selected_map,
    int16_t map_width,
    int16_t map_height,
    DM2_V1_SkprojectMinionDestinationReceipt *out_receipt)
{
    DM2_V1_SkprojectMinionDestinationReceipt receipt;
    uint16_t word;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.previous_map = current_map;
    receipt.selected_map = selected_map;
    receipt.destination_x = destination_x;
    receipt.destination_y = destination_y;
    receipt.previous_record_w6 = previous_record_w6;

    receipt.in_bounds = destination_x >= 0 && destination_x < map_width &&
                        destination_y >= 0 && destination_y < map_height;
    if (receipt.in_bounds) {
        word = (uint16_t)(previous_record_w6 & 0xfc00u);
        word = (uint16_t)((word & 0xffe0u) |
                          ((uint16_t)destination_x & 0x001fu));
        word = (uint16_t)((word & 0xfc1fu) |
                          (((uint16_t)destination_y & 0x001fu) << 5));
        word = (uint16_t)((word & 0x03ffu) |
                          (((uint16_t)selected_map & 0x003fu) << 10));
        receipt.new_record_w6 = word;
    } else {
        receipt.new_record_w6 = previous_record_w6;
    }
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return receipt.in_bounds;
}

int dm2_v1_skproject_map_0cee_17e7(
    uint16_t random_input,
    uint16_t divisor,
    uint16_t range_input,
    uint16_t savegame_seed,
    DM2_V1_SkprojectMapRandomReceipt *out_receipt)
{
    DM2_V1_SkprojectMapRandomReceipt receipt;
    uint32_t mixed;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.random_input = random_input;
    receipt.divisor = divisor;
    receipt.range_input = range_input;
    receipt.savegame_seed = savegame_seed;
    if (range_input == 0u) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    mixed = ((uint32_t)random_input * 0x7ab9u) / 2u;
    mixed += 11u * (uint32_t)divisor;
    mixed += savegame_seed;
    mixed >>= 2;
    receipt.mixed_value = mixed;
    receipt.result = (uint16_t)(mixed % (uint32_t)range_input);
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return (int)receipt.result;
}

int dm2_v1_skproject_map_0cee_04e5(
    const uint8_t *tiles,
    int16_t width,
    int16_t height,
    int16_t dir,
    int16_t forward,
    int16_t side,
    int16_t x,
    int16_t y,
    DM2_V1_SkprojectMapTileVectorReceipt *out_receipt)
{
    DM2_V1_SkprojectMapTileVectorReceipt receipt;
    int16_t tile_x = x;
    int16_t tile_y = y;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.dir = dir;
    receipt.forward = forward;
    receipt.side = side;
    receipt.input_x = x;
    receipt.input_y = y;
    dm2_v1_skproject_calc_vector_w_dir(dir, forward, side,
                                       &tile_x, &tile_y, NULL);
    receipt.tile_x = tile_x;
    receipt.tile_y = tile_y;
    if (!tiles || width <= 0 || height <= 0) {
        receipt.blocked_missing_tiles = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (tile_x < 0 || tile_y < 0 || tile_x >= width || tile_y >= height) {
        receipt.blocked_out_of_bounds = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    receipt.tile_value = tiles[(int)tile_y * (int)width + (int)tile_x];
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return (int)receipt.tile_value;
}

int dm2_v1_skproject_map_3b001(
    int16_t current_map,
    int16_t value_0270,
    int16_t value_0272,
    DM2_V1_SkprojectMap3B001Receipt *out_receipt)
{
    DM2_V1_SkprojectMap3B001Receipt receipt;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.previous_current_map = current_map;
    receipt.new_v1e0270 = value_0270;
    receipt.new_v1e0272 = value_0272;
    receipt.requested_change_to_previous_map = current_map != -1;
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_skproject_map_0cee_1815(
    int gate,
    int16_t map_width,
    int16_t map_height,
    int16_t v1d3248,
    int16_t x,
    int16_t y,
    int16_t selector,
    uint16_t savegame_seed,
    const uint8_t *candidate_table,
    uint16_t candidate_count,
    DM2_V1_SkprojectMap1815Receipt *out_receipt)
{
    DM2_V1_SkprojectMap1815Receipt receipt;
    uint16_t divisor;
    uint16_t random_input;
    int selected;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.candidate_count = candidate_count;
    if (!gate) {
        receipt.blocked_zero_gate = 1u;
        if (out_receipt) *out_receipt = receipt;
        return -1;
    }
    if (!candidate_table || candidate_count == 0u) {
        receipt.blocked_empty_table = 1u;
        if (out_receipt) *out_receipt = receipt;
        return -1;
    }

    divisor = (uint16_t)((uint16_t)(v1d3248 << 6) + 0x0bb8u +
                         (uint16_t)map_width + (uint16_t)map_height);
    random_input = (uint16_t)(32u * (uint16_t)x + 0x07d0u +
                              (uint16_t)y);
    selected = dm2_v1_skproject_map_0cee_17e7(
        random_input, divisor, (uint16_t)selector, savegame_seed, NULL);
    receipt.mixed_random =
        ((((uint32_t)random_input * 0x7ab9u) / 2u) +
         11u * (uint32_t)divisor + savegame_seed) >> 2;
    if (selected < 0 || selected >= (int)candidate_count) {
        receipt.blocked_empty_table = 1u;
        if (out_receipt) *out_receipt = receipt;
        return -1;
    }

    receipt.selected_index = (uint16_t)selected;
    receipt.selected_value = candidate_table[selected];
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return (int)receipt.selected_value;
}

int dm2_v1_skproject_map_0cee_185a(
    uint16_t *out_words4,
    int16_t map_width,
    int16_t map_height,
    int16_t v1d3248,
    int16_t gate0,
    int16_t gate1,
    int16_t gate2,
    int16_t gate3,
    int16_t x,
    int16_t y,
    int16_t rotation,
    int16_t step,
    uint16_t savegame_seed,
    const uint8_t *candidate_table,
    uint16_t candidate_count,
    const uint8_t *ornate_alcove_flags,
    DM2_V1_SkprojectMap185AReceipt *out_receipt)
{
    DM2_V1_SkprojectMap185AReceipt receipt;
    int16_t gates[4] = { gate0, gate1, gate2, gate3 };
    int16_t step_plus_one = (int16_t)(step + 1);
    int16_t slot = rotation;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    if (!out_words4) {
        receipt.blocked_missing_output = 1u;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    for (uint16_t i = 0; i < 4u; ++i) {
        int16_t selector =
            (int16_t)((((slot & 3) + 1) * step_plus_one) & 0xffff);
        int value = dm2_v1_skproject_map_0cee_1815(
            gates[i], map_width, map_height, v1d3248, x, y, selector,
            savegame_seed, candidate_table, candidate_count, NULL);
        out_words4[i] = (uint16_t)(uint8_t)value;
        receipt.values[i] = out_words4[i];
        slot++;
    }

    if (x < 0 || x >= map_width || step < 0 || step >= map_height) {
        for (uint16_t i = 0; i < 4u; ++i) {
            uint16_t value = out_words4[i] & 0xffu;
            if (ornate_alcove_flags && value < 256u &&
                ornate_alcove_flags[value]) {
                out_words4[i] = 0x00ffu;
                receipt.values[i] = 0x00ffu;
                receipt.sanitized[i] = 1u;
            }
        }
    }

    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

static int dm2_v1_skproject_map_record_valid(uint16_t record,
                                             uint16_t count)
{
    return record != DM2_V1_SKPROJECT_MAP_RECORD_END && record < count;
}

static uint8_t dm2_v1_skproject_map_record_type(
    const DM2_V1_SkprojectMapRecord *records,
    uint16_t record)
{
    return records[record].record_type;
}

int dm2_v1_skproject_map_2066_1f37(
    DM2_V1_SkprojectMapRecord *records,
    uint16_t record_count,
    uint16_t head,
    uint16_t value,
    int16_t *counter,
    DM2_V1_SkprojectMap20661F37Receipt *out_receipt)
{
    DM2_V1_SkprojectMap20661F37Receipt receipt;
    uint16_t current = head;
    int found = 0;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    if (!records || !counter) {
        receipt.blocked_missing_records = 1u;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    while (dm2_v1_skproject_map_record_valid(current, record_count)) {
        uint16_t next = records[current].next;
        current = next;
        if (!dm2_v1_skproject_map_record_valid(current, record_count))
            break;
        receipt.scanned_records++;
        if (dm2_v1_skproject_map_record_type(records, current) == 3u) {
            uint16_t w2 = records[current].w2;
            if ((w2 & 0x007fu) == 0x0027u) {
                found = 1;
                receipt.matched_records++;
                if ((w2 & 0x0080u) == 0u) {
                    records[current].w2 =
                        (uint16_t)((w2 & 0x007fu) |
                                   (((uint16_t)(value + 1u) & 0x01ffu)
                                    << 7));
                    (*counter)++;
                    receipt.counter_increment++;
                    receipt.updated_records++;
                }
            }
        }
    }

    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return found;
}

uint16_t dm2_v1_skproject_map_2066_1ec9(
    DM2_V1_SkprojectMapRecord *records,
    uint16_t record_count,
    uint16_t head,
    uint16_t append,
    DM2_V1_SkprojectMap20661EC9Receipt *out_receipt)
{
    DM2_V1_SkprojectMap20661EC9Receipt receipt;
    uint16_t result = head;
    uint16_t current = append;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.returned_head = head;
    if (!records) {
        receipt.blocked_missing_records = 1u;
        if (out_receipt) *out_receipt = receipt;
        return head;
    }
    if (head == DM2_V1_SKPROJECT_MAP_RECORD_END) {
        receipt.returned_head = append;
        receipt.valid = 1;
        if (out_receipt) *out_receipt = receipt;
        return append;
    }
    if (append == DM2_V1_SKPROJECT_MAP_RECORD_END) {
        receipt.valid = 1;
        if (out_receipt) *out_receipt = receipt;
        return head;
    }

    while (dm2_v1_skproject_map_record_valid(current, record_count) &&
           dm2_v1_skproject_map_record_type(records, current) < 4u) {
        uint16_t next = records[current].next;
        uint16_t previous_result = result;
        result = current;
        records[current].next = previous_result;
        current = next;
        receipt.rewired_records++;
    }
    if (dm2_v1_skproject_map_record_valid(result, record_count))
        records[result].next = current;
    receipt.appended_tail = current;
    receipt.returned_head = result;
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return result;
}

int dm2_v1_skproject_tmpmap_or_flag(
    uint8_t *tmpmap,
    uint16_t tmpmap_size,
    int16_t y,
    int16_t x,
    int16_t offset,
    DM2_V1_SkprojectTmpmapFlagReceipt *out_receipt)
{
    DM2_V1_SkprojectTmpmapFlagReceipt receipt;
    int32_t index = ((int32_t)y << 2) + ((int32_t)x << 2) + offset;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    if (!tmpmap) {
        receipt.blocked_missing_tmpmap = 1u;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (index < 0 || index >= tmpmap_size) {
        receipt.blocked_out_of_bounds = 1u;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    receipt.offset = (uint16_t)index;
    receipt.previous_value = tmpmap[index];
    tmpmap[index] = (uint8_t)(tmpmap[index] | 2u);
    receipt.new_value = tmpmap[index];
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_skproject_locate_other_level(
    const DM2_V1_SkprojectMapDescriptor *maps,
    uint16_t map_count,
    int16_t source_map,
    int16_t locate_delta,
    int16_t *x,
    int16_t *y,
    const uint8_t *candidate_cursor,
    uint16_t candidate_count,
    uint16_t resume_offset,
    uint16_t *out_resume_offset,
    DM2_V1_SkprojectLocateOtherLevelReceipt *out_receipt)
{
    DM2_V1_SkprojectLocateOtherLevelReceipt receipt;
    int16_t world_x;
    int16_t world_y;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.source_map = source_map;
    receipt.locate_delta = locate_delta;
    if (!maps || source_map < 0 || source_map >= (int16_t)map_count) {
        receipt.blocked_missing_descriptors = 1u;
        if (out_resume_offset) *out_resume_offset = 0u;
        if (out_receipt) *out_receipt = receipt;
        return -1;
    }
    if (!x || !y) {
        receipt.blocked_missing_output = 1u;
        if (out_resume_offset) *out_resume_offset = 0u;
        if (out_receipt) *out_receipt = receipt;
        return -1;
    }

    world_x = (int16_t)(*x + maps[source_map].world_x);
    world_y = (int16_t)(*y + maps[source_map].world_y);
    receipt.source_world_x = world_x;
    receipt.source_world_y = world_y;

    if (!candidate_cursor || candidate_count == 0u) {
        candidate_cursor = &maps[source_map].map_id;
        candidate_count = 1u;
    }
    if (resume_offset < candidate_count)
        receipt.used_resume_cursor = resume_offset != 0u;
    else
        resume_offset = 0u;

    for (uint16_t i = resume_offset; i < candidate_count; ++i) {
        uint8_t map_id = candidate_cursor[i];
        const DM2_V1_SkprojectMapDescriptor *candidate;
        int16_t local_x;
        int16_t local_y;

        if (map_id == 0xffu)
            break;
        receipt.scanned_candidates++;
        if (map_id >= map_count)
            continue;
        candidate = &maps[map_id];
        local_x = (int16_t)(world_x - candidate->world_x);
        local_y = (int16_t)(world_y - candidate->world_y);
        if (local_x < -1 || local_y < -1 ||
            local_x > candidate->width + 1 ||
            local_y > candidate->height + 1)
            continue;
        if (candidate->tile_type_at_local == 7u ||
            (candidate->tile_type_at_local == 5u &&
             candidate->teleporter_record_active)) {
            receipt.rejected_teleporter = 1u;
            continue;
        }

        *x = local_x;
        *y = local_y;
        if (out_resume_offset)
            *out_resume_offset = (uint16_t)(i + 1u);
        receipt.selected_map = map_id;
        receipt.selected_x = local_x;
        receipt.selected_y = local_y;
        receipt.selected_tile_type = candidate->tile_type_at_local;
        receipt.found = 1;
        receipt.valid = 1;
        if (out_receipt) *out_receipt = receipt;
        return map_id;
    }

    if (out_resume_offset) *out_resume_offset = 0u;
    receipt.selected_map = -1;
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return -1;
}

int dm2_v1_skproject_map_3bf83(
    int16_t x,
    int16_t y,
    int16_t target_map,
    int16_t rotation,
    int16_t current_map,
    int16_t current_x,
    int16_t current_y,
    int16_t target_width,
    int16_t target_height,
    DM2_V1_SkprojectMap3BF83Receipt *out_receipt)
{
    DM2_V1_SkprojectMap3BF83Receipt receipt;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.current_map = current_map;
    receipt.target_map = target_map;
    receipt.x = x;
    receipt.y = y;
    receipt.rotation = (int16_t)(rotation & 3);
    receipt.move_from_x = current_x;
    receipt.move_from_y = current_y;
    receipt.move_to_x = x;
    receipt.move_to_y = y;
    receipt.target_differs_from_current = target_map != current_map;

    if (x < 0 || y < 0 || x >= target_width || y >= target_height) {
        if (receipt.target_differs_from_current)
            receipt.requested_restore_current = 1u;
        receipt.valid = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    receipt.in_bounds = 1u;
    if (receipt.target_differs_from_current) {
        receipt.requested_change_to_target = 1u;
        receipt.requested_restore_current = 1u;
        receipt.requested_load_newmap = 1u;
        receipt.move_to_x = -1;
        receipt.move_to_y = y;
    }
    receipt.requested_party_rotate = 1u;
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_skproject_move_12b4_0092(
    uint16_t active_v1e0534,
    uint16_t arrow_panel,
    int16_t highlight_param,
    DM2_V1_SkprojectArrowHighlightReceipt *out_receipt)
{
    DM2_V1_SkprojectArrowHighlightReceipt receipt;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.active_v1e0534 = active_v1e0534;
    receipt.arrow_panel = arrow_panel;
    receipt.highlight_param = highlight_param;
    receipt.requested_highlight = active_v1e0534 != 0u;
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return receipt.requested_highlight;
}

int dm2_v1_skproject_move_12b4_00af(
    int enter_forward,
    int16_t source_map,
    int16_t source_x,
    int16_t source_y,
    int16_t located_map,
    int16_t located_x,
    int16_t located_y,
    int16_t query_rotation,
    DM2_V1_SkprojectOtherLevelReceipt *out_receipt)
{
    DM2_V1_SkprojectOtherLevelReceipt receipt;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.requested_drop_record = 1u;
    receipt.source_map = source_map;
    receipt.source_x = source_x;
    receipt.source_y = source_y;
    receipt.locate_delta = enter_forward ? -1 : 1;
    receipt.located_map = located_map;
    receipt.located_x = located_x;
    receipt.located_y = located_y;
    receipt.query_rotation = query_rotation;
    receipt.final_party_dir = (int16_t)(query_rotation & 3);
    receipt.requested_restore_source_map = 1u;
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_skproject_move_12b4_023f(
    int16_t x,
    int16_t y,
    int16_t arg0,
    int16_t arg1,
    const int16_t direction_champions[4],
    const uint8_t champion_hero_types[4],
    const uint8_t wound_results[4],
    DM2_V1_SkprojectMove12B4023FReceipt *out_receipt)
{
    DM2_V1_SkprojectMove12B4023FReceipt receipt;
    int16_t candidates[2];
    uint8_t directions[2];
    unsigned i;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.x = x;
    receipt.y = y;
    receipt.arg0 = arg0;
    receipt.arg1 = arg1;
    receipt.first_candidate = -1;
    receipt.second_candidate = -1;
    receipt.wounded_champions[0] = -1;
    receipt.wounded_champions[1] = -1;
    if (!direction_champions || !champion_hero_types || !wound_results) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    directions[0] = (uint8_t)((arg1 + arg0 + 2) & 3);
    directions[1] = (uint8_t)((arg1 + arg0 + 3) & 3);
    receipt.first_direction = directions[0];
    receipt.second_direction = directions[1];
    candidates[0] = direction_champions[directions[0]];
    candidates[1] = direction_champions[directions[1]];
    receipt.first_candidate = candidates[0];
    receipt.second_candidate = candidates[1];
    if (candidates[0] == candidates[1])
        candidates[1] = -1;

    for (i = 0; i < 2u; ++i) {
        int16_t champion = candidates[i];
        if (champion < 0 || champion >= 4)
            continue;
        ++receipt.candidate_count;
        ++receipt.wound_attempts;
        if (wound_results[(uint8_t)champion]) {
            unsigned out = receipt.wound_successes;
            if (out < 2u) {
                receipt.wounded_champions[out] = champion;
                receipt.noise_hero_types[out] = champion_hero_types[(uint8_t)champion];
            }
            ++receipt.wound_successes;
            ++receipt.noise_requests;
        }
    }

    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return receipt.wound_successes != 0u;
}

int dm2_v1_skproject_attack_door(
    uint8_t tile_type,
    uint8_t record_byte2,
    uint8_t record_byte3,
    uint16_t attack_power,
    uint16_t required_power,
    int use_byte2_gate,
    int rebirth_altar,
    uint16_t timer_delay,
    int16_t x,
    int16_t y,
    DM2_V1_SkprojectAttackDoorReceipt *out_receipt)
{
    DM2_V1_SkprojectAttackDoorReceipt receipt;
    uint8_t gate_byte = use_byte2_gate ? record_byte2 : record_byte3;
    uint8_t gate_mask = use_byte2_gate ? 0x80u : 0x01u;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.tile_type_before = tile_type;
    receipt.tile_type_after = tile_type;
    receipt.attack_power = attack_power;
    receipt.required_power = required_power;
    receipt.rebirth_altar = rebirth_altar ? 1u : 0u;
    receipt.test_byte_offset = use_byte2_gate ? 2u : 3u;
    receipt.tested_flag_mask = gate_mask;
    receipt.timer_ticks = timer_delay;
    receipt.x = x;
    receipt.y = y;
    receipt.valid = 1;

    if ((gate_byte & gate_mask) == 0u) {
        receipt.blocked_door_closed_flag = 1u;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (attack_power < required_power) {
        receipt.blocked_attack_power = 1u;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (tile_type != 4u) {
        receipt.blocked_tile_type = 1u;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    receipt.admitted = 1u;
    if (timer_delay != 0u) {
        receipt.queued_timer = 1u;
    } else {
        receipt.changed_tile_type = 1u;
        receipt.tile_type_after = 5u;
    }
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

static int dm2_v1_skproject_wall_item_matches(uint16_t required_item_type,
                                              uint16_t projectile_item_type)
{
    return required_item_type == 0x01ffu ||
           required_item_type == projectile_item_type;
}

int dm2_v1_skproject_attack_wall(
    const DM2_V1_SkprojectWallAttackRecord *records,
    uint16_t record_count,
    uint16_t projectile_record_word,
    uint16_t projectile_item_type,
    int16_t attack_dir,
    uint8_t randdir,
    int16_t source_x,
    int16_t source_y,
    DM2_V1_SkprojectAttackWallReceipt *out_receipt)
{
    DM2_V1_SkprojectAttackWallReceipt receipt;
    uint8_t requested_side = (uint8_t)((attack_dir + 2) & 3);
    int projectile_cut = 0;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.requested_side = requested_side;
    receipt.projectile_side_after = (uint8_t)(projectile_record_word >> 14);
    receipt.projectile_item_type = projectile_item_type;
    receipt.source_x = source_x;
    receipt.source_y = source_y;
    receipt.target_x = source_x;
    receipt.target_y = source_y;

    if (!records && record_count != 0u) {
        receipt.blocked_no_records = 1u;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    for (uint16_t i = 0; i < record_count; ++i) {
        const DM2_V1_SkprojectWallAttackRecord *record = &records[i];
        uint8_t wall_side = (uint8_t)(record->link_word >> 14);

        receipt.checked_records++;
        if (wall_side != requested_side)
            continue;
        receipt.matching_side_records++;
        receipt.wall_side = wall_side;

        if (!projectile_cut &&
            ((projectile_record_word >> 10) & 0x0fu) != 0x0fu &&
            record->alcove_data_index != 0u &&
            randdir == 0u) {
            projectile_cut = 1;
            receipt.projectile_cut = 1u;
            receipt.projectile_side_after = wall_side;
            receipt.used_alcove_relocation = 1u;
            receipt.found_effect = 1u;
            continue;
        }

        if (record->record_type != 3u)
            continue;

        if (record->actuator_class == 0x22u) {
            uint8_t active =
                dm2_v1_skproject_wall_item_matches(
                    record->required_item_type, projectile_item_type) ? 1u : 0u;
            active ^= (record->target_flag ? 1u : 0u);
            if (!active)
                continue;
            receipt.invoked_actuator = 1u;
            receipt.activated_record_index = (uint8_t)i;
            receipt.found_effect = 1u;
            if (!projectile_cut && record->consume_projectile) {
                projectile_cut = 1;
                receipt.projectile_cut = 1u;
                receipt.projectile_side_after = requested_side;
            }
        } else if (record->actuator_class == 0x23u &&
                   !projectile_cut &&
                   dm2_v1_skproject_wall_item_matches(
                       record->required_item_type, projectile_item_type)) {
            projectile_cut = 1;
            receipt.projectile_cut = 1u;
            receipt.used_teleport_relocation = 1u;
            receipt.found_effect = 1u;
            receipt.activated_record_index = (uint8_t)i;
            receipt.target_x = record->destination_x;
            receipt.target_y = record->destination_y;
            receipt.projectile_side_after =
                record->tile_type_at_destination != 0u
                    ? (record->side_when_tile_nonzero & 3u)
                    : (record->side_when_tile_zero & 3u);
        }
    }

    if (out_receipt) *out_receipt = receipt;
    return receipt.found_effect ? 1 : 0;
}

int dm2_v1_skproject_move_12b4_099e(
    const DM2_V1_SkprojectLiftRequest *request,
    DM2_V1_SkprojectLiftReceipt *out_receipt)
{
    DM2_V1_SkprojectLiftReceipt receipt;
    uint16_t hero_count;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    if (!request) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (request->creature_weight > 0x00fdu) {
        receipt.blocked_overweight_creature = 1u;
        receipt.valid = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    hero_count = request->hero_count;
    if (hero_count > DM2_V1_SKPROJECT_PARTY_HERO_LIMIT)
        hero_count = DM2_V1_SKPROJECT_PARTY_HERO_LIMIT;
    for (uint16_t i = 0; i < hero_count; ++i) {
        const DM2_V1_SkprojectLiftHero *hero = &request->heroes[i];
        uint16_t strength;

        receipt.checked_heroes++;
        if (!hero->alive)
            continue;
        strength = hero->strength;
        if (i == request->event_hero_index)
            strength = (uint16_t)(strength + strength / 8u);
        if (request->creature_weight <= 0x002du)
            strength = (uint16_t)(strength + strength / 4u);
        if ((hero->stamina_adjusted_strength != 0u ?
             hero->stamina_adjusted_strength : strength) >=
                request->creature_weight ||
            request->rand16_values[i] == 0u) {
            receipt.can_lift = 1u;
            break;
        }
    }

    for (uint16_t i = 0; i < hero_count; ++i) {
        const DM2_V1_SkprojectLiftHero *hero = &request->heroes[i];
        if (!hero->alive)
            continue;
        if (hero->cur_stamina > hero->max_stamina >> 4) {
            uint16_t amount = 5u;
            if (receipt.can_lift) {
                uint16_t quarter = request->creature_weight / 4u;
                amount = quarter > 5u ? quarter : 5u;
            }
            receipt.stamina_adjustments[i] = amount;
        }
    }

    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return receipt.can_lift;
}

int dm2_v1_skproject_wall_ornate_alcove_data_index(
    int ornate_alcove_from_record,
    int16_t cls2,
    uint16_t gdat_data_index,
    DM2_V1_SkprojectWallAlcoveReceipt *out_receipt)
{
    DM2_V1_SkprojectWallAlcoveReceipt receipt;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.ornate_alcove = ornate_alcove_from_record != 0;
    if (!receipt.ornate_alcove) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (cls2 < 0 || cls2 > 0xff) {
        receipt.cls2_missing = 1u;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    receipt.cls2 = (uint8_t)cls2;
    receipt.data_index = gdat_data_index;
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return gdat_data_index != 0u;
}

int32_t dm2_v1_skproject_atimesb_rshiftc(int16_t a,
                                         int8_t c,
                                         int16_t b)
{
    uint32_t product = (uint32_t)(uint16_t)a * (uint32_t)(uint16_t)b;
    uint8_t shift = (uint8_t)c;

    if (shift >= 32u) return 0;
    return (int32_t)(product >> shift);
}

void dm2_v1_skproject_cache_state_init(
    DM2_V1_SkprojectCacheState *state,
    uint16_t cache_capacity,
    uint16_t raw_count,
    uint16_t mement_count)
{
    if (!state) return;
    memset(state, 0, sizeof(*state));
    if (cache_capacity > DM2_V1_SKPROJECT_CACHE_LIMIT)
        cache_capacity = DM2_V1_SKPROJECT_CACHE_LIMIT;
    if (raw_count > DM2_V1_SKPROJECT_RAW_MEMENT_LIMIT)
        raw_count = DM2_V1_SKPROJECT_RAW_MEMENT_LIMIT;
    if (mement_count > DM2_V1_SKPROJECT_CACHE_LIMIT)
        mement_count = DM2_V1_SKPROJECT_CACHE_LIMIT;
    state->cache_capacity = cache_capacity;
    state->raw_count = raw_count;
    state->mement_count = mement_count;
    for (uint16_t i = 0; i < DM2_V1_SKPROJECT_CACHE_LIMIT; ++i)
        state->cache_to_mement[i] = DM2_V1_SKPROJECT_MEMENT_NONE;
    for (uint16_t i = 0; i < DM2_V1_SKPROJECT_RAW_MEMENT_LIMIT; ++i)
        state->raw_to_mement[i] = DM2_V1_SKPROJECT_MEMENT_NONE;
}

int dm2_v1_skproject_find_ici_from_cache_hash(
    const DM2_V1_SkprojectCacheState *state,
    uint32_t cache_hash,
    uint16_t *out_ici)
{
    int di = -1;
    int cx;

    if (!state || !out_ici || state->cache_count > state->cache_capacity)
        return 0;
    cx = (int)state->cache_count;
    while (1) {
        int si = (di + cx) >> 1;
        uint16_t cache_index;
        uint32_t stored_hash;

        if (si == di) {
            *out_ici = (uint16_t)(si + 1);
            return 0;
        }
        if (si < 0 || si >= (int)state->cache_count)
            return 0;
        cache_index = state->sorted_cache_indices[si];
        if (cache_index >= state->cache_capacity)
            return 0;
        stored_hash = state->hashes[cache_index];
        if (cache_hash < stored_hash) {
            cx = si;
        } else if (cache_hash > stored_hash) {
            di = si;
        } else {
            *out_ici = (uint16_t)si;
            return 1;
        }
    }
}

static uint16_t dm2_v1_skproject_first_free_cache_index(
    const DM2_V1_SkprojectCacheState *state)
{
    for (uint16_t i = 0; i < state->cache_capacity; ++i) {
        int used = 0;
        for (uint16_t j = 0; j < state->cache_count; ++j) {
            if (state->sorted_cache_indices[j] == i) {
                used = 1;
                break;
            }
        }
        if (!used) return i;
    }
    return DM2_V1_SKPROJECT_MEMENT_NONE;
}

uint16_t dm2_v1_skproject_insert_cache_hash_at(
    DM2_V1_SkprojectCacheState *state,
    uint32_t cache_hash,
    uint16_t ici)
{
    uint16_t cache_index;

    if (!state || state->cache_count >= state->cache_capacity ||
        state->cache_count >= DM2_V1_SKPROJECT_CACHE_LIMIT ||
        ici > state->cache_count) {
        return DM2_V1_SKPROJECT_MEMENT_NONE;
    }
    cache_index = dm2_v1_skproject_first_free_cache_index(state);
    if (cache_index == DM2_V1_SKPROJECT_MEMENT_NONE)
        return DM2_V1_SKPROJECT_MEMENT_NONE;
    for (uint16_t i = state->cache_count; i > ici; --i)
        state->sorted_cache_indices[i] = state->sorted_cache_indices[i - 1u];
    state->sorted_cache_indices[ici] = cache_index;
    state->hashes[cache_index] = cache_hash;
    state->cache_to_mement[cache_index] =
        cache_index < state->mement_count ? cache_index :
        DM2_V1_SKPROJECT_MEMENT_NONE;
    state->cache_count++;
    return cache_index;
}

uint16_t dm2_v1_skproject_query_mementi_from(
    const DM2_V1_SkprojectCacheState *state,
    uint16_t index)
{
    uint16_t plain_index;

    if (!state) return DM2_V1_SKPROJECT_MEMENT_NONE;
    plain_index = (uint16_t)(index & 0x7fffu);
    if ((index & 0x8000u) != 0u) {
        if (plain_index >= state->cache_capacity)
            return DM2_V1_SKPROJECT_MEMENT_NONE;
        return state->cache_to_mement[plain_index];
    }
    if (plain_index >= state->raw_count)
        return DM2_V1_SKPROJECT_MEMENT_NONE;
    return state->raw_to_mement[plain_index];
}

int dm2_v1_skproject_add_cache_hash(
    DM2_V1_SkprojectCacheState *state,
    uint32_t cache_hash,
    uint16_t *out_cache_index)
{
    uint16_t ici = 0u;
    uint16_t cache_index;

    if (!state || !out_cache_index) return -1;
    if (dm2_v1_skproject_find_ici_from_cache_hash(
            state, cache_hash, &ici) != 0) {
        cache_index = state->sorted_cache_indices[ici];
        if (cache_index >= state->cache_capacity)
            return -1;
        *out_cache_index = cache_index;
        return 1;
    }
    cache_index =
        dm2_v1_skproject_insert_cache_hash_at(state, cache_hash, ici);
    if (cache_index == DM2_V1_SKPROJECT_MEMENT_NONE)
        return -1;
    *out_cache_index = cache_index;
    return 0;
}

uint8_t *dm2_v1_skproject_query_mement_buff_from_cache_index(
    DM2_V1_SkprojectCacheState *state,
    uint16_t cache_index)
{
    uint16_t mementi;

    if (!state) return 0;
    mementi = dm2_v1_skproject_query_mementi_from(
        state, (uint16_t)(cache_index | 0x8000u));
    if (mementi == DM2_V1_SKPROJECT_MEMENT_NONE ||
        mementi >= state->mement_count) {
        return 0;
    }
    return state->mement_buffers[mementi];
}

uint32_t dm2_v1_skproject_get_temp_cache_hash(
    const DM2_V1_SkprojectCacheState *state)
{
    uint16_t ici = 0u;
    uint32_t hash;

    if (!state) return 0u;
    for (uint32_t guard = 0; guard < 0x10000u; ++guard) {
        hash = 0xffff0000u |
               (uint32_t)((state->temp_hash_counter + guard) & 0xffffu);
        if (dm2_v1_skproject_find_ici_from_cache_hash(
                state, hash, &ici) == 0) {
            return hash;
        }
    }
    return 0u;
}

uint16_t dm2_v1_skproject_alloc_temp_cache_index(
    DM2_V1_SkprojectCacheState *state)
{
    uint16_t cache_index = DM2_V1_SKPROJECT_MEMENT_NONE;
    uint32_t hash;

    if (!state) return DM2_V1_SKPROJECT_MEMENT_NONE;
    hash = dm2_v1_skproject_get_temp_cache_hash(state);
    if (hash == 0u) return DM2_V1_SKPROJECT_MEMENT_NONE;
    if (dm2_v1_skproject_add_cache_hash(state, hash, &cache_index) < 0)
        return DM2_V1_SKPROJECT_MEMENT_NONE;
    state->temp_hash_counter++;
    return cache_index;
}

int dm2_v1_skproject_test_mement(int32_t dw0, int32_t stored_len)
{
    int32_t probe_offset;

    probe_offset = abs((int)-dw0) - 4;
    if (probe_offset < 0 || probe_offset >= 65536)
        return 0;
    return dw0 == stored_len;
}

int dm2_v1_skproject_recycle_mementi(
    DM2_V1_SkprojectCacheState *state,
    uint16_t mementi,
    uint16_t previous_w4,
    uint16_t yy,
    DM2_V1_SkprojectRecycleMementReceipt *out_receipt)
{
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!state || !out_receipt || mementi >= state->mement_count)
        return 0;
    out_receipt->valid = 1;
    out_receipt->mementi = mementi;
    out_receipt->previous_w4 = previous_w4;
    out_receipt->yy = yy;
    out_receipt->recycled_to_free_list =
        previous_w4 == DM2_V1_SKPROJECT_MEMENT_NONE;
    if (out_receipt->recycled_to_free_list) {
        for (uint16_t i = 0; i < state->cache_capacity; ++i) {
            if (state->cache_to_mement[i] == mementi)
                state->cache_to_mement[i] = DM2_V1_SKPROJECT_MEMENT_NONE;
        }
        for (uint16_t i = 0; i < state->raw_count; ++i) {
            if (state->raw_to_mement[i] == mementi)
                state->raw_to_mement[i] = DM2_V1_SKPROJECT_MEMENT_NONE;
        }
    }
    return 1;
}

int dm2_v1_skproject_alloc_new_pict(
    uint16_t index,
    uint16_t width,
    uint16_t height,
    uint16_t bpp,
    DM2_V1_SkprojectNewPictReceipt *out_receipt)
{
    uint32_t row_bytes;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt || height == 0u)
        return 0;
    row_bytes = bpp == 4u ? (uint32_t)(((width + 1u) & 0xfffeu) >> 1) :
                            (uint32_t)width;
    out_receipt->index = index;
    out_receipt->width = width;
    out_receipt->height = height;
    out_receipt->bpp = bpp;
    out_receipt->payload_bytes = row_bytes * (uint32_t)height;
    out_receipt->header_width = width;
    out_receipt->header_height = height;
    out_receipt->header_bpp = bpp;
    return 1;
}

uint32_t dm2_v1_skproject_calc_pict_ent_hash(
    const DM2_V1_SkprojectExtendedPictureRef *ref)
{
    if (!ref) return 0u;
    return ((uint32_t)(ref->w6 & 0x1fffu) << 12) |
           ((uint32_t)(ref->w52 & 0x007fu) << 5) |
           (uint32_t)(ref->w54 & 0x001fu);
}

static uint16_t dm2_v1_skproject_select_image_mement_entry(
    const DM2_V1_SkprojectImageMementRequest *request,
    int *absent)
{
    if (absent) *absent = 1;
    if (!request) return DM2_V1_SKPROJECT_MEMENT_NONE;
    if (request->data_index != DM2_V1_SKPROJECT_MEMENT_NONE &&
        !request->data_absent) {
        if (absent) *absent = 0;
        return request->data_index;
    }
    if (request->data_index == DM2_V1_SKPROJECT_MEMENT_NONE)
        return DM2_V1_SKPROJECT_MEMENT_NONE;
    if (request->fallback_data_index != DM2_V1_SKPROJECT_MEMENT_NONE &&
        !request->fallback_absent) {
        if (absent) *absent = 0;
        return request->fallback_data_index;
    }
    if (request->data_index != DM2_V1_SKPROJECT_MEMENT_NONE)
        return request->data_index;
    return request->fallback_data_index;
}

int dm2_v1_skproject_alloc_image_mement(
    DM2_V1_SkprojectCacheState *state,
    const DM2_V1_SkprojectImageMementRequest *request,
    uint16_t *pinned_entry,
    DM2_V1_SkprojectImageMementReceipt *out_receipt)
{
    int absent = 1;
    uint16_t selected;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!state || !request || !out_receipt || !pinned_entry)
        return 0;
    out_receipt->selected_data_index = DM2_V1_SKPROJECT_MEMENT_NONE;
    out_receipt->touched_mementi = DM2_V1_SKPROJECT_MEMENT_NONE;
    out_receipt->pinned_entry_index = DM2_V1_SKPROJECT_MEMENT_NONE;

    selected = dm2_v1_skproject_select_image_mement_entry(request, &absent);
    out_receipt->selected_data_index = selected;
    if (selected == DM2_V1_SKPROJECT_MEMENT_NONE) {
        out_receipt->status = DM2_V1_SKPROJECT_IMAGE_MEMENT_NO_ENTRY;
        return 1;
    }
    if (request->existing_mementi != DM2_V1_SKPROJECT_MEMENT_NONE) {
        out_receipt->status = DM2_V1_SKPROJECT_IMAGE_MEMENT_TOUCHED_EXISTING;
        out_receipt->touched_mementi = request->existing_mementi;
        return request->existing_mementi < state->mement_count;
    }
    if (absent) {
        out_receipt->status = DM2_V1_SKPROJECT_IMAGE_MEMENT_ABSENT;
        return 1;
    }
    if (request->y_offset != -32) {
        out_receipt->status = DM2_V1_SKPROJECT_IMAGE_MEMENT_REJECT_Y_OFFSET;
        return 1;
    }
    if (request->bits_pixel != 8u) {
        out_receipt->status = DM2_V1_SKPROJECT_IMAGE_MEMENT_REJECT_BPP;
        return 1;
    }
    *pinned_entry = selected;
    out_receipt->status = DM2_V1_SKPROJECT_IMAGE_MEMENT_PINNED_ENTRY;
    out_receipt->pinned_entry_index = selected;
    return 1;
}

int dm2_v1_skproject_alloc_pict_mement(
    DM2_V1_SkprojectCacheState *state,
    const DM2_V1_SkprojectPictureRef *ref,
    const DM2_V1_SkprojectImageMementRequest *image_request,
    uint16_t *pinned_entry,
    DM2_V1_SkprojectPictMementReceipt *out_receipt)
{
    uint16_t cache_index;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!state || !ref || !out_receipt)
        return 0;
    if ((ref->w4 & 0x0004u) != 0u) {
        out_receipt->route = DM2_V1_SKPROJECT_PICT_MEMENT_IMAGE;
        return dm2_v1_skproject_alloc_image_mement(
            state, image_request, pinned_entry, &out_receipt->image);
    }
    if ((ref->w4 & 0x0008u) != 0u) {
        out_receipt->route = DM2_V1_SKPROJECT_PICT_MEMENT_CACHE;
        cache_index = ref->w12;
        if (cache_index >= state->cache_capacity)
            return 0;
        out_receipt->cache_index = cache_index;
        return 1;
    }
    out_receipt->route = DM2_V1_SKPROJECT_PICT_MEMENT_NONE;
    return 1;
}

int dm2_v1_skproject_free_image_mement(
    DM2_V1_SkprojectCacheState *state,
    const DM2_V1_SkprojectImageMementRequest *request,
    uint16_t *pinned_entry,
    DM2_V1_SkprojectFreeImageMementReceipt *out_receipt)
{
    int absent = 1;
    uint16_t selected;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!state || !request || !pinned_entry || !out_receipt)
        return 0;
    out_receipt->selected_data_index = DM2_V1_SKPROJECT_MEMENT_NONE;
    out_receipt->recycled_mementi = DM2_V1_SKPROJECT_MEMENT_NONE;
    selected = dm2_v1_skproject_select_image_mement_entry(request, &absent);
    out_receipt->selected_data_index = selected;
    if (selected == DM2_V1_SKPROJECT_MEMENT_NONE)
        return 1;
    if (*pinned_entry == selected) {
        *pinned_entry = DM2_V1_SKPROJECT_MEMENT_NONE;
        out_receipt->cleared_pinned_entry = 1;
    }
    if (request->existing_mementi != DM2_V1_SKPROJECT_MEMENT_NONE &&
        request->existing_mementi < state->mement_count) {
        DM2_V1_SkprojectRecycleMementReceipt recycle;

        dm2_v1_skproject_recycle_mementi(
            state, request->existing_mementi, DM2_V1_SKPROJECT_MEMENT_NONE,
            0u, &recycle);
        out_receipt->recycled_existing = recycle.valid;
        out_receipt->recycled_mementi = request->existing_mementi;
    }
    return 1;
}

int dm2_v1_skproject_free_pict_mement(
    DM2_V1_SkprojectCacheState *state,
    const DM2_V1_SkprojectPictureRef *ref,
    const DM2_V1_SkprojectImageMementRequest *image_request,
    uint16_t *pinned_entry,
    DM2_V1_SkprojectFreeImageMementReceipt *out_receipt)
{
    uint16_t cache_index;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!state || !ref || !out_receipt)
        return 0;
    if ((ref->w4 & 0x0004u) != 0u)
        return dm2_v1_skproject_free_image_mement(
            state, image_request, pinned_entry, out_receipt);
    if ((ref->w4 & 0x0008u) != 0u && ref->w12 < state->cache_capacity) {
        cache_index = ref->w12;
        state->cache_to_mement[cache_index] =
            DM2_V1_SKPROJECT_MEMENT_NONE;
    }
    return 1;
}

uint16_t dm2_v1_skproject_add_item_charge(
    uint16_t object_id,
    uint16_t *record_w2,
    int16_t delta,
    DM2_V1_SkprojectItemChargeReceipt *out_receipt)
{
    DM2_V1_SkprojectItemChargeReceipt receipt;
    uint16_t charge = 0u;
    uint16_t max_charge = 0u;
    uint16_t mask = 0u;
    unsigned shift = 0u;
    int db_type;
    int adjusted;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.object_id = object_id;
    receipt.delta = delta;
    db_type = (int)((object_id >> 10) & 0x0fu);
    receipt.db_type = db_type;
    if (object_id == DM2_V1_SKPROJECT_MEMENT_NONE) {
        receipt.blocked_null_object = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0u;
    }
    if (!record_w2) {
        receipt.blocked_unsupported_db_type = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0u;
    }
    receipt.previous_w2 = *record_w2;
    switch (db_type) {
    case 5:
        shift = 10u;
        max_charge = 0x000fu;
        mask = (uint16_t)(0x000fu << 10);
        break;
    case 6:
        shift = 9u;
        max_charge = 0x000fu;
        mask = (uint16_t)(0x000fu << 9);
        break;
    case 10:
        shift = 14u;
        max_charge = 0x0003u;
        mask = (uint16_t)(0x0003u << 14);
        break;
    default:
        receipt.blocked_unsupported_db_type = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0u;
    }
    charge = (uint16_t)((*record_w2 & mask) >> shift);
    adjusted = (int)charge + (int)delta;
    charge = (uint16_t)dm2_v1_skproject_between_value(
        0, (int16_t)adjusted, (int16_t)max_charge);
    *record_w2 = (uint16_t)((*record_w2 & (uint16_t)~mask) |
                            (uint16_t)(charge << shift));
    receipt.valid = 1;
    receipt.previous_charge = (uint16_t)((receipt.previous_w2 & mask) >> shift);
    receipt.new_charge = charge;
    receipt.max_charge = max_charge;
    receipt.new_w2 = *record_w2;
    if (out_receipt) *out_receipt = receipt;
    return charge;
}

uint16_t dm2_v1_skproject_get_max_charge(uint16_t object_id)
{
    int db_type;

    if (object_id == DM2_V1_SKPROJECT_MEMENT_NONE)
        return 0u;
    db_type = (int)((object_id >> 10) & 0x0fu);
    if (db_type == 5 || db_type == 6)
        return 15u;
    if (db_type == 10)
        return 3u;
    return 0u;
}

static int dm2_v1_skproject_item_db_type(uint16_t object_id)
{
    return (int)((object_id >> 10) & 0x0fu);
}

static const DM2_V1_SkprojectItemValueRecord *
dm2_v1_skproject_find_item_record(
    const DM2_V1_SkprojectItemValueWorld *world,
    uint16_t object_id)
{
    if (!world || !world->records) return 0;
    for (uint16_t i = 0; i < world->record_count; ++i) {
        if (world->records[i].object_id == object_id)
            return &world->records[i];
    }
    return 0;
}

static int dm2_v1_skproject_is_container_chest_record(
    const DM2_V1_SkprojectItemValueWorld *world,
    uint16_t object_id)
{
    const DM2_V1_SkprojectItemValueRecord *record;

    if (object_id == DM2_V1_SKPROJECT_MEMENT_NONE ||
        dm2_v1_skproject_item_db_type(object_id) != 9) {
        return 0;
    }
    record = dm2_v1_skproject_find_item_record(world, object_id);
    return record && record->container_type == 0u && !record->is_moneybox;
}

static uint16_t dm2_v1_skproject_gdat_word_value(
    const DM2_V1_SkprojectItemValueRecord *record,
    uint8_t cls4)
{
    if (!record || cls4 >= 0x36u) return 0u;
    return record->gdat_word_values[cls4];
}

static int32_t dm2_v1_skproject_query_item_value_depth(
    const DM2_V1_SkprojectItemValueWorld *world,
    uint16_t object_id,
    uint8_t cls4,
    unsigned depth,
    DM2_V1_SkprojectItemValueReceipt *out_receipt)
{
    DM2_V1_SkprojectItemValueReceipt receipt;
    const DM2_V1_SkprojectItemValueRecord *record;
    int32_t value;
    int db_type;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.object_id = object_id;
    receipt.cls4 = cls4;
    db_type = dm2_v1_skproject_item_db_type(object_id);
    receipt.db_type = db_type;
    if (object_id == DM2_V1_SKPROJECT_MEMENT_NONE) {
        receipt.blocked_null_object = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (depth > DM2_V1_SKPROJECT_ITEM_VALUE_RECORD_LIMIT) {
        receipt.blocked_recursion_limit = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    record = dm2_v1_skproject_find_item_record(world, object_id);
    if (!record) {
        receipt.blocked_missing_record = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    value = dm2_v1_skproject_gdat_word_value(record, cls4);
    receipt.base_value = value;

    if (cls4 == 1u || cls4 == 2u) {
        uint8_t multiplier_cls4 = cls4 == 1u ? 0x34u : 0x35u;
        uint16_t multiplier =
            dm2_v1_skproject_gdat_word_value(record, multiplier_cls4);

        receipt.charge_multiplier_cls4 = multiplier_cls4;
        if (multiplier > 0u) {
            uint16_t w2 = record->w2;
            DM2_V1_SkprojectItemChargeReceipt charge_receipt;

            receipt.charge = dm2_v1_skproject_add_item_charge(
                object_id, &w2, 0, &charge_receipt);
            receipt.charge_value_added =
                (int32_t)receipt.charge * (int32_t)multiplier;
            value += receipt.charge_value_added;
        }
    }
    if (cls4 == 2u && db_type == 8 && value > 1) {
        int32_t half = value >> 1;

        receipt.potion_value_before_scale = value;
        value = half + ((int32_t)(record->w2 & 0x00ffu) * half) / 255;
        receipt.potion_value_after_scale = value;
    }
    if (db_type == 9 && record->container_type == 0u) {
        uint16_t child = record->contained_object_id;
        int32_t moneybox_sum = 0;
        unsigned guard = 0;

        while (child != DM2_V1_SKPROJECT_MEMENT_NONE) {
            const DM2_V1_SkprojectItemValueRecord *child_record;
            int child_db_type;

            if (++guard > DM2_V1_SKPROJECT_ITEM_VALUE_RECORD_LIMIT) {
                receipt.blocked_recursion_limit = 1;
                if (out_receipt) *out_receipt = receipt;
                return value;
            }
            child_record = dm2_v1_skproject_find_item_record(world, child);
            if (!child_record) {
                receipt.blocked_missing_record = 1;
                if (out_receipt) *out_receipt = receipt;
                return value;
            }
            child_db_type = dm2_v1_skproject_item_db_type(child);
            if (record->is_moneybox && child_db_type == 10) {
                uint16_t w2 = child_record->w2;
                uint16_t charge = dm2_v1_skproject_add_item_charge(
                    child, &w2, 0, 0);

                moneybox_sum +=
                    (int32_t)dm2_v1_skproject_gdat_word_value(
                        child_record, cls4) *
                    (int32_t)(charge + 1u);
            } else {
                int32_t child_value =
                    dm2_v1_skproject_query_item_value_depth(
                        world, child, cls4, depth + 1u, 0);

                value += child_value;
                receipt.contained_recursive_value += child_value;
            }
            child = child_record->next_object_id;
        }
        if (record->is_moneybox) {
            receipt.moneybox_contained_value = moneybox_sum;
            receipt.moneybox_rounding_value =
                cls4 == 1u ? (moneybox_sum + 4) / 5 : moneybox_sum;
            value += receipt.moneybox_rounding_value;
        }
    }
    receipt.valid = 1;
    receipt.final_value = value;
    if (out_receipt) *out_receipt = receipt;
    return value;
}

int32_t dm2_v1_skproject_query_item_value(
    const DM2_V1_SkprojectItemValueWorld *world,
    uint16_t object_id,
    uint8_t cls4,
    DM2_V1_SkprojectItemValueReceipt *out_receipt)
{
    return dm2_v1_skproject_query_item_value_depth(
        world, object_id, cls4, 0u, out_receipt);
}

int32_t dm2_v1_skproject_query_item_weight(
    const DM2_V1_SkprojectItemValueWorld *world,
    uint16_t object_id,
    DM2_V1_SkprojectItemValueReceipt *out_receipt)
{
    return dm2_v1_skproject_query_item_value(world, object_id, 1u,
                                             out_receipt);
}

int dm2_v1_skproject_calc_player_weight(
    const DM2_V1_SkprojectItemValueWorld *world,
    uint16_t player,
    const DM2_V1_SkprojectPlayerWeightRequest *request,
    DM2_V1_SkprojectPlayerWeightReceipt *out_receipt)
{
    DM2_V1_SkprojectPlayerWeightReceipt receipt;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.player = player;
    receipt.hero_flag_or = 0x1000u;
    if (!world || !request || !out_receipt) {
        receipt.blocked_missing_request = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    for (uint16_t i = 0; i < DM2_V1_SKPROJECT_PLAYER_INVENTORY_SLOTS; ++i) {
        receipt.inventory_weight +=
            (uint32_t)dm2_v1_skproject_query_item_weight(
                world, request->inventory[i], 0);
    }
    if (request->selected_player_plus_one != (uint16_t)(player + 1u)) {
        receipt.blocked_player_not_selected = 1;
    } else if (request->selected_hand_action >= 2u) {
        receipt.blocked_selected_hand_action = 1;
    } else if (!dm2_v1_skproject_is_container_chest_record(
                   world,
                   request->selected_hand_items[
                       request->selected_hand_action])) {
        receipt.blocked_selected_hand_not_chest = 1;
    } else {
        receipt.included_open_chest_overlay = 1;
        for (uint16_t i = 0;
             i < DM2_V1_SKPROJECT_CURRENT_CONTAINER_SLOTS;
             ++i) {
            receipt.open_chest_weight +=
                (uint32_t)dm2_v1_skproject_query_item_weight(
                    world, request->current_container_items[i], 0);
        }
    }
    receipt.final_weight = receipt.inventory_weight + receipt.open_chest_weight;
    receipt.valid = 1;
    *out_receipt = receipt;
    return 1;
}

int dm2_v1_skproject_count_by_coin_types(
    const DM2_V1_SkprojectItemValueWorld *world,
    uint16_t moneybox_object_id,
    const uint16_t *money_item_ids,
    uint16_t money_item_count,
    int16_t *out_counts,
    DM2_V1_SkprojectCountByCoinTypesReceipt *out_receipt)
{
    DM2_V1_SkprojectCountByCoinTypesReceipt receipt;
    const DM2_V1_SkprojectItemValueRecord *moneybox;
    uint16_t child;
    uint16_t guard = 0u;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.moneybox_object_id = moneybox_object_id;
    if (money_item_count > DM2_V1_SKPROJECT_MONEY_ITEM_MAX)
        money_item_count = DM2_V1_SKPROJECT_MONEY_ITEM_MAX;
    receipt.money_item_count = money_item_count;
    if (!out_counts) {
        receipt.blocked_missing_output = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    memset(out_counts, 0,
           sizeof(out_counts[0]) * DM2_V1_SKPROJECT_MONEY_ITEM_MAX);
    if (!world || !money_item_ids) {
        receipt.blocked_missing_record = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    moneybox = dm2_v1_skproject_find_item_record(world, moneybox_object_id);
    if (!moneybox || dm2_v1_skproject_item_db_type(moneybox_object_id) != 9) {
        receipt.blocked_missing_record = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    child = moneybox->contained_object_id;
    while (child != DM2_V1_SKPROJECT_MEMENT_NONE) {
        const DM2_V1_SkprojectItemValueRecord *record;

        if (++guard > DM2_V1_SKPROJECT_ITEM_VALUE_RECORD_LIMIT) {
            receipt.blocked_recursion_limit = 1;
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
        record = dm2_v1_skproject_find_item_record(world, child);
        if (!record) {
            receipt.blocked_missing_record = 1;
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
        receipt.visited_records++;
        if (dm2_v1_skproject_item_db_type(child) == 10 &&
            record->is_currency) {
            uint16_t w2 = record->w2;
            uint16_t charge =
                dm2_v1_skproject_add_item_charge(child, &w2, 0, 0);

            receipt.currency_records++;
            for (uint16_t i = 0; i < money_item_count; ++i) {
                if (money_item_ids[i] == record->distinctive_item_type) {
                    out_counts[i] =
                        (int16_t)(out_counts[i] + (int16_t)(charge + 1u));
                    receipt.counts[i] = out_counts[i];
                    receipt.matched_currency_records++;
                }
            }
        }
        child = record->next_object_id;
    }
    receipt.valid = 1;
    for (uint16_t i = 0; i < money_item_count; ++i)
        receipt.counts[i] = out_counts[i];
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_skproject_boost_attribute(
    DM2_V1_SkprojectChampionAttribute *attributes,
    uint8_t attribute_index,
    int16_t delta,
    DM2_V1_SkprojectBoostAttributeReceipt *out_receipt)
{
    DM2_V1_SkprojectBoostAttributeReceipt receipt;
    DM2_V1_SkprojectChampionAttribute *attribute;
    int16_t si;
    int16_t reduced_delta;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.attribute_index = attribute_index;
    receipt.delta_input = delta;
    if (!attributes) {
        receipt.blocked_missing_attribute = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    attribute = &attributes[attribute_index];
    receipt.previous_current = attribute->current;
    receipt.maximum = attribute->maximum;
    si = (int16_t)((int)attribute->current + (int)delta -
                   (int)attribute->maximum);
    reduced_delta = delta;
    receipt.source_si = si;
    if ((si < 0) == (delta < 0)) {
        int16_t remaining = si < 0 ? (int16_t)-si : si;

        while (remaining > 20) {
            reduced_delta = (int16_t)(reduced_delta - reduced_delta / 4);
            remaining = (int16_t)(remaining - 20);
        }
    }
    attribute->current = (uint8_t)dm2_v1_skproject_between_value(
        10, (int16_t)((int)attribute->current + (int)reduced_delta), 220);
    receipt.valid = 1;
    receipt.reduced_delta = reduced_delta;
    receipt.final_current = attribute->current;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_skproject_adjust_ui_event(
    DM2_V1_SkprojectUiEvent *event,
    uint16_t player_dir,
    const int16_t player_at_position[4],
    const DM2_V1_SkprojectUiChampionState *champions,
    uint16_t champion_count,
    DM2_V1_SkprojectAdjustUiEventReceipt *out_receipt)
{
    DM2_V1_SkprojectAdjustUiEventReceipt receipt;
    uint16_t idx;
    int adjusted = 0;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.player_dir = (uint16_t)(player_dir & 3u);
    if (!event) {
        receipt.blocked_missing_event = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    idx = event->event;
    receipt.input_event = idx;
    receipt.output_event = idx;
    receipt.mapped_player = -1;
    if (!player_at_position || !champions) {
        receipt.blocked_missing_party = 1;
        event->event = 0u;
        receipt.output_event = 0u;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    if (idx >= 116u && idx <= 123u) {
        uint16_t slot = (uint16_t)(idx - 116u);
        uint8_t hand = (uint8_t)(slot & 1u);
        int16_t player = player_at_position[
            (uint16_t)((receipt.player_dir + slot / 2u) & 3u)];

        receipt.mapped_player = player;
        receipt.mapped_hand = hand;
        if (player < 0 || (uint16_t)player >= champion_count ||
            !champions[player].present) {
            receipt.blocked_no_player = 1;
        } else if (champions[player].hand_cooldown[hand] != 0u) {
            receipt.blocked_hand_cooldown = 1;
        } else if (champions[player].hand_activable[hand] == 0u) {
            receipt.blocked_hand_not_activable = 1;
        } else {
            idx = (uint16_t)(2u * (uint16_t)player + 116u + hand);
            adjusted = 1;
        }
    } else if (idx >= 95u && idx <= 98u) {
        int16_t player = player_at_position[
            (uint16_t)((idx - 95u + receipt.player_dir) & 3u)];

        receipt.mapped_player = player;
        if (player < 0 || (uint16_t)player >= champion_count ||
            !champions[player].present) {
            receipt.blocked_no_player = 1;
        } else {
            int16_t w2;
            int16_t w3;

            if (idx > 96u)
                w2 = (int16_t)(event->y - event->rect.y);
            else
                w2 = (int16_t)(event->rect.h + event->rect.y - 1 -
                               event->y);
            if (idx != 96u && idx != 97u)
                w3 = (int16_t)(event->rect.x + event->rect.w - 1 -
                               event->x);
            else
                w3 = (int16_t)(event->x - event->rect.x);
            receipt.diagonal_w2 = w2;
            receipt.diagonal_w3 = w3;
            if (w3 > w2) {
                if (champions[player].hand_cooldown[2] == 0u) {
                    adjusted = 1;
                } else {
                    receipt.blocked_leader_hand_cooldown = 1;
                }
            } else {
                idx = (uint16_t)(idx - 79u);
                receipt.selected_spell_triangle = 1;
                adjusted = 1;
            }
        }
    } else {
        receipt.valid = 1;
        receipt.untouched_non_adjustable_event = 1;
        receipt.output_event = idx;
        if (out_receipt) *out_receipt = receipt;
        return 1;
    }

    if (!adjusted)
        idx = 0u;
    event->event = idx;
    receipt.valid = 1;
    receipt.output_event = idx;
    if (out_receipt) *out_receipt = receipt;
    return adjusted ? 1 : 0;
}

int dm2_v1_skproject_query_font(
    const uint8_t *font_plane,
    uint8_t glyph,
    uint8_t foreground,
    uint8_t background,
    DM2_V1_SkprojectFontReceipt *out_receipt)
{
    DM2_V1_SkprojectFontReceipt receipt;
    uint8_t written = 0u;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.glyph = glyph;
    receipt.foreground = foreground;
    receipt.background = background;
    for (uint8_t i = 0; i < DM2_V1_SKPROJECT_FONT_PIXELS; ++i)
        receipt.pixels[i] = background;
    if (!font_plane) {
        receipt.blocked_missing_font_plane = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    for (uint8_t row = 0; row < 6u; ++row) {
        uint8_t bits = font_plane[(uint16_t)row * 128u + glyph];

        for (uint8_t pair = 0; pair < 3u; ++pair) {
            uint8_t high = (bits & 0x10u) ? foreground : background;
            uint8_t low = (bits & 0x08u) ? foreground : background;

            receipt.pixels[written++] = (uint8_t)((high << 4) | low);
            bits = (uint8_t)(bits << 2);
        }
    }
    receipt.written_pixels = written;
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

static uint16_t dm2_v1_skproject_text_len(const char *text,
                                          uint16_t max_len)
{
    uint16_t len = 0u;

    if (!text) return 0u;
    while (len < max_len && text[len] != '\0')
        ++len;
    return len;
}

int dm2_v1_skproject_query_str_metrics(
    const char *text,
    DM2_V1_SkprojectTextMetricsReceipt *out_receipt)
{
    DM2_V1_SkprojectTextMetricsReceipt receipt;
    uint16_t len;
    int16_t width;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    if (!text) {
        receipt.blocked_missing_text = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    len = dm2_v1_skproject_text_len(text, DM2_V1_SKPROJECT_TEXT_LIMIT);
    receipt.text_len = len;
    width = (int16_t)(-1 + (int16_t)len * 6);
    if (width <= 0) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    receipt.width = width;
    receipt.height = 5;
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_skproject_plan_draw_string(
    DM2_V1_SkprojectTextRoute route,
    int16_t dest_width,
    int16_t x,
    int16_t baseline_y,
    uint16_t foreground,
    uint16_t background,
    const char *text,
    DM2_V1_SkprojectTextDrawReceipt *out_receipt)
{
    DM2_V1_SkprojectTextDrawReceipt receipt;
    DM2_V1_SkprojectTextMetricsReceipt metrics;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.route = route;
    receipt.dest_width = dest_width;
    receipt.input_x = x;
    receipt.input_baseline_y = baseline_y;
    receipt.foreground = foreground;
    receipt.background = background;
    receipt.char_w = 6;
    receipt.char_h = 6;

    if (!text) {
        receipt.blocked_missing_text = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (!dm2_v1_skproject_query_str_metrics(text, &metrics)) {
        receipt.blocked_empty_text = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    receipt.text_w = metrics.width;
    receipt.text_h = metrics.height;
    receipt.char_count = metrics.text_len;
    receipt.draw_x = x;
    receipt.draw_y = (int16_t)(baseline_y + 1 - 6);
    receipt.first_char_x = x;
    receipt.last_char_x = (int16_t)(x + (int16_t)(metrics.text_len - 1u) * 6);
    receipt.uses_alpha_mask = (background & 0x4000u) ? 1 : 0;

    switch (route) {
    case DM2_V1_SKPROJECT_TEXT_ROUTE_STRONG:
    case DM2_V1_SKPROJECT_TEXT_ROUTE_NAME:
    case DM2_V1_SKPROJECT_TEXT_ROUTE_LOCAL:
    case DM2_V1_SKPROJECT_TEXT_ROUTE_BACKBUFF:
        receipt.strong_shadow_passes = 2;
        receipt.fill_background = (background & 0x4000u) ? 0 : 1;
        receipt.fill_x = (int16_t)(x - 1);
        receipt.fill_y = (int16_t)(baseline_y - metrics.height);
        receipt.fill_w = (int16_t)(metrics.width + 2);
        receipt.fill_h = (int16_t)(metrics.height + 2);
        break;
    default:
        break;
    }
    if (route == DM2_V1_SKPROJECT_TEXT_ROUTE_BUTTON ||
        route == DM2_V1_SKPROJECT_TEXT_ROUTE_NAME)
        receipt.adjusts_button_rect = 1;
    if (route == DM2_V1_SKPROJECT_TEXT_ROUTE_BACKBUFF) {
        receipt.centered_by_metrics = 1;
        receipt.draw_x = (int16_t)(x - metrics.height / 2);
    }
    if (dest_width == 320)
        receipt.dest_is_screen = 1;
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_skproject_format_skstr_literal(
    const char *source,
    char *dest,
    uint16_t dest_capacity,
    DM2_V1_SkprojectFormatTextReceipt *out_receipt)
{
    DM2_V1_SkprojectFormatTextReceipt receipt;
    uint16_t read = 0u;
    uint16_t written = 0u;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    if (!source) {
        receipt.blocked_missing_text = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (!dest || dest_capacity == 0u) {
        receipt.blocked_missing_output = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    while (source[read] != '\0') {
        if ((source[read] == '.' && source[read + 1u] == 'Z') ||
            (uint8_t)source[read] == 1u) {
            receipt.blocked_unimplemented_substitution = 1;
            break;
        }
        if (written + 1u >= dest_capacity)
            break;
        dest[written++] = source[read++];
    }
    dest[written] = '\0';
    receipt.consumed_bytes = read;
    receipt.written_bytes = written;
    receipt.valid = receipt.blocked_unimplemented_substitution ? 0 : 1;
    if (out_receipt) *out_receipt = receipt;
    return receipt.valid;
}

int dm2_v1_skproject_decode_gdat_text_literal(
    const uint8_t *source,
    uint16_t source_len,
    int encrypted,
    char *dest,
    uint16_t dest_capacity,
    DM2_V1_SkprojectFormatTextReceipt *out_receipt)
{
    char temp[DM2_V1_SKPROJECT_TEXT_LIMIT + 1u];
    uint16_t len;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!source) {
        if (out_receipt) out_receipt->blocked_missing_text = 1;
        return 0;
    }
    len = source_len;
    if (len > DM2_V1_SKPROJECT_TEXT_LIMIT)
        len = DM2_V1_SKPROJECT_TEXT_LIMIT;
    for (uint16_t i = 0; i < len; ++i) {
        uint8_t value = source[i];

        if (encrypted)
            value = (uint8_t)(~value - (uint8_t)i);
        temp[i] = (char)value;
    }
    temp[len] = '\0';
    return dm2_v1_skproject_format_skstr_literal(
        temp, dest, dest_capacity, out_receipt);
}

int dm2_v1_skproject_split_hint_line(
    const char *source,
    uint16_t start_offset,
    int16_t max_width,
    char *dest,
    uint16_t dest_capacity,
    DM2_V1_SkprojectHintLineReceipt *out_receipt)
{
    DM2_V1_SkprojectHintLineReceipt receipt;
    uint16_t in = start_offset;
    uint16_t out = 0u;
    uint16_t last_space_in = DM2_V1_SKPROJECT_TEXT_LIMIT + 1u;
    uint16_t last_space_out = 0u;
    int16_t width = 0;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.start_offset = start_offset;
    if (!source) {
        receipt.blocked_missing_text = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (!dest || dest_capacity == 0u) {
        receipt.blocked_missing_output = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    while (source[in] != '\0' && source[in] != '\n' &&
           out + 1u < dest_capacity) {
        int16_t next_width = (int16_t)(width + 6);

        if (source[in] == ' ') {
            last_space_in = in;
            last_space_out = out;
            receipt.consumed_width = width;
        }
        if (next_width > max_width) {
            if (last_space_in <= in) {
                dest[last_space_out] = '\0';
                receipt.copied_bytes = last_space_out;
                receipt.next_offset = (uint16_t)(last_space_in + 1u);
                receipt.split_at_space = 1;
                receipt.valid = 1;
                if (out_receipt) *out_receipt = receipt;
                return 1;
            }
            break;
        }
        dest[out++] = source[in++];
        width = next_width;
    }
    dest[out] = '\0';
    receipt.copied_bytes = out;
    receipt.next_offset = in;
    receipt.consumed_width = width;
    receipt.stopped_at_newline = source[in] == '\n';
    receipt.stopped_at_nul = source[in] == '\0';
    if (receipt.stopped_at_newline)
        receipt.next_offset = (uint16_t)(in + 1u);
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

const char *dm2_v1_skproject_core_source_evidence(void)
{
    return "skproject SKWINSPX/src/v4/skcore.cpp "
           "ALLOC_TEMP_RECT/ALLOC_TEMP_ORIGIN_RECT/BETWEEN_VALUE; "
           "SKWINSPX/src/v5/skrect.cpp alloc_tmprect/alloc_origin_tmprect; "
           "SKWINSPX/src/v5/util.cpp DM2_BETWEEN_VALUE; "
           "SKULLWIN/c_random.cpp DM2_RAND16/DM2_RANDBIT/DM2_RANDDIR; "
           "SKULLWIN/util.cpp DM2_ABS/DM2_CALC_SQUARE_DISTANCE/"
           "DM2_CALC_VECTOR_DIR/DM2_CALC_VECTOR_W_DIR/"
           "DM2_COMPUTE_POWER_4_WITHIN/DM2_FILL_I16TABLE/"
           "DM2_ATIMESB_RSHIFTC and "
           "SKWIN/SkWinCore.cpp CALC_VECTOR_W_DIR; "
           "SKWIN/SkWinCore.cpp FIND_ICI_FROM_CACHE_HASH/"
           "INSERT_CACHE_HASH_AT/QUERY_MEMENTI_FROM/ADD_CACHE_HASH/"
           "QUERY_MEMENT_BUFF_FROM_CACHE_INDEX/GET_TEMP_CACHE_HASH/"
           "ALLOC_TEMP_CACHE_INDEX/RECYCLE_MEMENTI/TEST_MEMENT/"
           "ALLOC_NEW_PICT/ALLOC_IMAGE_MEMENT/ALLOC_PICT_MEMENT/"
           "CALC_PICT_ENT_HASH/FREE_IMAGE_MEMENT/FREE_PICT_MEMENT; "
           "SKWIN/SkWinCore.cpp ADD_ITEM_CHARGE/GET_MAX_CHARGE/"
           "QUERY_ITEM_VALUE/QUERY_ITEM_WEIGHT/CALC_PLAYER_WEIGHT/"
           "COUNT_BY_COIN_TYPES and "
           "SKULLWIN/c_item.cpp DM2_ADD_ITEM_CHARGE/DM2_GET_MAX_CHARGE/"
           "DM2_QUERY_ITEM_VALUE/DM2_QUERY_ITEM_WEIGHT; "
           "SKULLWIN/c_querydb.cpp DM2_COUNT_BY_COIN_TYPES; "
           "SKWIN/SkWinCore.cpp BOOST_ATTRIBUTE and ADJUST_UI_EVENT; "
           "SKULLWIN/c_input.cpp DM2_ADJUST_UI_EVENT; "
           "SKULLWIN/c_gfx_str.cpp c_stringdata::init/"
           "DM2_QUERY_FONT/DM2_QUERY_STR_METRICS/DM2_DRAW_STRING/"
           "DM2_DRAW_STRONG_TEXT/DM2_DRAW_BUTTON_STR/DM2_DRAW_NAME_STR/"
           "DM2_DRAW_VP_STR/DM2_DRAW_GUIDED_STR/DM2_PRINT_SYSERR_TEXT/"
           "DM2_DRAW_VP_RC_STR/DM2_DRAW_LOCAL_TEXT/DM2_FORMAT_SKSTR/"
           "DM2_QUERY_GDAT_TEXT/DM2_DRAW_TEXT_TO_BACKBUFF/"
           "DM2_gfxstr_3929_04e2/DM2_gfxstr_24a5_0732/"
           "DM2_DISPLAY_HINT_TEXT/DM2_SCROLLBOX_MESSAGE; "
           "SKULLWIN/c_gfx_pal.cpp color_to_palettecolor/"
           "DM2_CONVERT_DRIVERPALETTE/DM2_SELECT_PALETTE_SET/"
           "DM2_UPDATE_BLIT_PALETTE/DM2_xlat_palette; "
           "SKULLWIN/c_move.cpp DM2_12b4_0953/DM2_12b4_0881/"
           "DM2_ATTACK_WALL/DM2_ATTACK_DOOR; "
           "SKULLWIN/c_map.cpp DM2_SET_DESTINATION_OF_MINION_MAP/"
           "DM2_map_0cee_17e7/DM2_map_0cee_04e5/DM2_map_3B001/"
           "DM_LOCATE_OTHER_LEVEL/DM2_map_3BF83";
}
