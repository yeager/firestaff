#include "dm2_v1_skproject_core.h"

#include <stdio.h>
#include <string.h>

static int failed;

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        ++failed; \
        printf("FAIL: %s\n", msg); \
    } else { \
        printf("PASS: %s\n", msg); \
    } \
} while (0)

static void test_between_value(void)
{
    CHECK(dm2_v1_skproject_between_value(10, 9, 20) == 10,
          "BETWEEN_VALUE clamps below min");
    CHECK(dm2_v1_skproject_between_value(10, 10, 20) == 10,
          "BETWEEN_VALUE admits min");
    CHECK(dm2_v1_skproject_between_value(10, 17, 20) == 17,
          "BETWEEN_VALUE admits middle");
    CHECK(dm2_v1_skproject_between_value(10, 20, 20) == 20,
          "BETWEEN_VALUE admits max");
    CHECK(dm2_v1_skproject_between_value(10, 21, 20) == 20,
          "BETWEEN_VALUE clamps above max");
    CHECK(dm2_v1_skproject_dm2_between_value(-1, 127, 200) == 127,
          "DM2_BETWEEN_VALUE wrapper uses v5 argument order");
}

static void test_temp_rect_ring(void)
{
    DM2_V1_SkprojectTempRectRing ring;
    DM2_V1_SkprojectTempRectReceipt receipt[5];

    dm2_v1_skproject_temp_rect_ring_init(&ring);
    memset(receipt, 0, sizeof(receipt));

    CHECK(dm2_v1_skproject_alloc_temp_rect(&ring, 1, 2, 3, 4,
                                           &receipt[0]) == 1,
          "ALLOC_TEMP_RECT accepts first rect");
    CHECK(receipt[0].slot == 0 && receipt[0].next_slot == 1 &&
              receipt[0].rect.x == 1 && receipt[0].rect.y == 2 &&
              receipt[0].rect.w == 3 && receipt[0].rect.h == 4,
          "ALLOC_TEMP_RECT writes slot 0 and advances ring");
    CHECK(dm2_v1_skproject_alloc_temp_origin_rect(&ring, 5, 6,
                                                  &receipt[1]) == 1,
          "ALLOC_TEMP_ORIGIN_RECT delegates to temp rect");
    CHECK(receipt[1].slot == 1 && receipt[1].next_slot == 2 &&
              receipt[1].rect.x == 0 && receipt[1].rect.y == 0 &&
              receipt[1].rect.w == 5 && receipt[1].rect.h == 6,
          "ALLOC_TEMP_ORIGIN_RECT writes origin rectangle");
    CHECK(dm2_v1_skproject_alloc_temp_rect(&ring, 7, 8, 9, 10,
                                           &receipt[2]) == 1 &&
              receipt[2].slot == 2 && receipt[2].next_slot == 3,
          "third temp rect uses slot 2");
    CHECK(dm2_v1_skproject_alloc_temp_rect(&ring, 11, 12, 13, 14,
                                           &receipt[3]) == 1 &&
              receipt[3].slot == 3 && receipt[3].next_slot == 0,
          "fourth temp rect wraps next index to zero");
    CHECK(dm2_v1_skproject_alloc_temp_rect(&ring, 15, 16, 17, 18,
                                           &receipt[4]) == 1 &&
              receipt[4].slot == 0 && receipt[4].next_slot == 1 &&
              ring.rects[0].x == 15 && ring.rects[0].h == 18,
          "fifth temp rect overwrites slot 0 like skproject ringbuffer");
    CHECK(receipt[0].receipt_hash != 0u && receipt[4].receipt_hash != 0u &&
              receipt[0].receipt_hash != receipt[4].receipt_hash,
          "temp rect receipts carry nonzero slot/value hash");
    CHECK(dm2_v1_skproject_alloc_temp_rect(0, 1, 2, 3, 4,
                                           &receipt[0]) == 0,
          "ALLOC_TEMP_RECT rejects missing ring");
    CHECK(dm2_v1_skproject_alloc_temp_rect(&ring, 1, 2, 3, 4, 0) == 0,
          "ALLOC_TEMP_RECT rejects missing receipt");
}

static void test_random_helpers(void)
{
    DM2_V1_SkprojectRandomData randdat;

    dm2_v1_skproject_random_init(&randdat);
    CHECK(randdat.random == 0u, "c_random init clears random seed");
    CHECK(dm2_v1_skproject_rand(&randdat) == 0u &&
              randdat.random == 11u,
          "DM2_RAND returns seed*magic+11 shifted by 8");
    CHECK(dm2_v1_skproject_rand16(&randdat, 10u) == 9u,
          "DM2_RAND16 modulo uses the 24-bit DM2_RAND value");
    CHECK(dm2_v1_skproject_randbit(&randdat) == 0,
          "DM2_RANDBIT masks one random bit");
    CHECK(dm2_v1_skproject_randdir(&randdat) == 0u,
          "DM2_RANDDIR masks two random direction bits");
    CHECK(dm2_v1_skproject_rand(&randdat) == 13344383u,
          "c_random sequence advances through every helper");
    CHECK(dm2_v1_skproject_rand16(&randdat, 0u) == 0u,
          "DM2_RAND16 zero range returns zero");
}

static void test_util_helpers(void)
{
    DM2_V1_SkprojectRandomData randdat;
    DM2_V1_SkprojectVectorDirReceipt dir_receipt;
    DM2_V1_SkprojectFillI16TableReceipt fill_receipt;
    int16_t table[5] = { 1, 2, 3, 4, 5 };

    CHECK(dm2_v1_skproject_abs(-12) == 12 &&
              dm2_v1_skproject_abs(7) == 7,
          "DM2_ABS returns source signed absolute value");
    CHECK(dm2_v1_skproject_calc_square_distance(7, -3, 2, 5) == 13,
          "DM2_CALC_SQUARE_DISTANCE sums absolute x/y deltas");

    CHECK(dm2_v1_skproject_calc_vector_dir(
              0, 10, 20, 4, 18, &dir_receipt) == 1 &&
              dir_receipt.valid && dir_receipt.dir == 3u &&
              !dir_receipt.consumed_randbit &&
              dir_receipt.delta_x == 6 && dir_receipt.delta_y == 2,
          "DM2_CALC_VECTOR_DIR chooses west/east axis without random on non-tie");
    CHECK(dm2_v1_skproject_calc_vector_dir(
              0, 4, 1, 4, 9, &dir_receipt) == 1 &&
              dir_receipt.valid && dir_receipt.dir == 2u,
          "DM2_CALC_VECTOR_DIR chooses north/south axis by y delta");

    dm2_v1_skproject_random_init(&randdat);
    CHECK(dm2_v1_skproject_calc_vector_dir(
              &randdat, 5, 5, 1, 1, &dir_receipt) == 1 &&
              dir_receipt.valid && dir_receipt.tied_axes &&
              dir_receipt.consumed_randbit && dir_receipt.randbit == 0u &&
              dir_receipt.abs_delta_x == 4 &&
              dir_receipt.abs_delta_y == 5 &&
              dir_receipt.dir == 0u,
          "DM2_CALC_VECTOR_DIR consumes DM2_RANDBIT to break diagonal ties");
    CHECK(dm2_v1_skproject_calc_vector_dir(
              0, 5, 5, 1, 1, &dir_receipt) == 0 &&
              dir_receipt.blocked_missing_random,
          "DM2_CALC_VECTOR_DIR fails closed on tied axes without random state");

    CHECK(dm2_v1_skproject_compute_power_4_within(0x2au, 1) == 0x02,
          "DM2_COMPUTE_POWER_4_WITHIN returns first set power");
    CHECK(dm2_v1_skproject_compute_power_4_within(0x2au, 3) == 0x20,
          "DM2_COMPUTE_POWER_4_WITHIN returns requested set-bit ordinal");
    CHECK(dm2_v1_skproject_compute_power_4_within(0x2au, 4) == 0,
          "DM2_COMPUTE_POWER_4_WITHIN returns shifted-out zero when ordinal is absent");

    CHECK(dm2_v1_skproject_fill_i16table(
              table, -9, 5u, &fill_receipt) == 1 &&
              fill_receipt.valid && fill_receipt.written_entries == 5u &&
              table[0] == -9 && table[4] == -9,
          "DM2_FILL_I16TABLE writes every source table entry");
    CHECK(dm2_v1_skproject_fill_i16table(
              0, 4, 0u, &fill_receipt) == 1 &&
              fill_receipt.valid && fill_receipt.written_entries == 0u,
          "DM2_FILL_I16TABLE accepts zero entries without a table");
    CHECK(dm2_v1_skproject_fill_i16table(
              0, 4, 1u, &fill_receipt) == 0 &&
              fill_receipt.blocked_missing_table,
          "DM2_FILL_I16TABLE rejects missing table when entries are required");

    CHECK(dm2_v1_skproject_atimesb_rshiftc(300, 3, 7) == 262,
          "DM2_ATIMESB_RSHIFTC multiplies unsigned 16-bit inputs before shift");
    CHECK(dm2_v1_skproject_atimesb_rshiftc(-2, 4, 2) == 8191,
          "DM2_ATIMESB_RSHIFTC preserves unsignedlong conversion of signed words");
}

static void test_palette_helpers(void)
{
    DM2_V1_SkprojectDriverPaletteReceipt driver_receipt;
    DM2_V1_SkprojectPaletteSetReceipt set_receipt;
    DM2_V1_SkprojectXlatPaletteReceipt xlat_receipt;
    uint8_t alpha_rgb[1024];
    uint8_t palette[4] = { 1u, 2u, 3u, 4u };
    uint8_t conv[256];
    uint8_t byte = 0u;
    const uint8_t *selected_palette = 0;

    for (uint16_t i = 0; i < 256u; ++i) {
        alpha_rgb[i * 4u + 0u] = 0xaau;
        alpha_rgb[i * 4u + 1u] = (uint8_t)i;
        alpha_rgb[i * 4u + 2u] = (uint8_t)(255u - i);
        alpha_rgb[i * 4u + 3u] = (uint8_t)(i * 3u);
        conv[i] = (uint8_t)(255u - i);
    }

    CHECK(dm2_v1_skproject_palettecolor_from_color(42u, &byte) == 1 &&
              byte == 42u,
          "color_to_palettecolor is the skproject byte wrapper");
    CHECK(dm2_v1_skproject_palettecolor_from_ui8(43u, &byte) == 1 &&
              byte == 43u,
          "ui8_to_palettecolor preserves the palette byte");
    CHECK(dm2_v1_skproject_palettecolor_to_ui8(44u, &byte) == 1 &&
              byte == 44u,
          "palettecolor_to_ui8 preserves the palette byte");
    CHECK(dm2_v1_skproject_palettecolor_to_pixel(45u, &byte) == 1 &&
              byte == 45u,
          "palettecolor_to_pixel preserves the palette byte");

    CHECK(dm2_v1_skproject_convert_driverpalette(
              alpha_rgb, 1, &driver_receipt) == 1 &&
              driver_receipt.valid &&
              driver_receipt.converted_entries == 256u &&
              driver_receipt.driver_setcolors_requested &&
              driver_receipt.dmpal6[1][0] == (1u >> 2) &&
              driver_receipt.dmpal6[1][1] == (254u >> 2) &&
              driver_receipt.dmpal6[255][2] == ((uint8_t)(255u * 3u) >> 2) &&
              driver_receipt.dmpal_hash != 0u,
          "DM2_CONVERT_DRIVERPALETTE skips alpha and converts RGB8 to DMPAL RGB6");
    CHECK(dm2_v1_skproject_convert_driverpalette(
              0, 0, &driver_receipt) == 0 &&
              driver_receipt.blocked_missing_input,
          "DM2_CONVERT_DRIVERPALETTE fails closed without source palette");

    CHECK(dm2_v1_skproject_select_palette_set(0, &set_receipt) == 1 &&
              set_receipt.valid && set_receipt.fade_to_black_requested &&
              !set_receipt.immediate_colors_after &&
              set_receipt.vsync_waits == 2000u,
          "DM2_SELECT_PALETTE_SET(0) records source fade-to-black timing");
    CHECK(dm2_v1_skproject_select_palette_set(1, &set_receipt) == 1 &&
              set_receipt.valid && set_receipt.driver_setcolors_requested &&
              set_receipt.immediate_colors_after,
          "DM2_SELECT_PALETTE_SET(1) requests immediate driver colors");
    CHECK(dm2_v1_skproject_select_palette_set(2, &set_receipt) == 1 &&
              set_receipt.valid && !set_receipt.immediate_colors_after &&
              !set_receipt.driver_setcolors_requested,
          "DM2_SELECT_PALETTE_SET preserves source no-op behavior for other modes");

    CHECK(dm2_v1_skproject_update_blit_palette(
              palette, 4u, &selected_palette) == 1 &&
              selected_palette == palette,
          "DM2_UPDATE_BLIT_PALETTE assigns the caller palette pointer");
    CHECK(dm2_v1_skproject_update_blit_palette(
              palette, 4u, 0) == 0,
          "DM2_UPDATE_BLIT_PALETTE rejects missing output slot");

    CHECK(dm2_v1_skproject_xlat_palette(
              palette, 4u, conv, &xlat_receipt) == 1 &&
              xlat_receipt.valid && xlat_receipt.colors_after == 4u &&
              xlat_receipt.converted_colors == 4u &&
              xlat_receipt.palette[0] == 254u &&
              xlat_receipt.palette[3] == 251u &&
              !xlat_receipt.large_palette_copy,
          "DM2_xlat_palette applies the conversion table to explicit colors");
    CHECK(dm2_v1_skproject_xlat_palette(
              palette, 0u, conv, &xlat_receipt) == 1 &&
              xlat_receipt.valid && xlat_receipt.colors_after == 256u &&
              xlat_receipt.large_palette_copy &&
              xlat_receipt.palette[0] == 255u &&
              xlat_receipt.palette[255] == 0u,
          "DM2_xlat_palette copies the large palette when colors is zero");
    CHECK(dm2_v1_skproject_xlat_palette(
              0, 4u, conv, &xlat_receipt) == 0 &&
              xlat_receipt.blocked_missing_output,
          "DM2_xlat_palette rejects missing source palette for explicit colors");
}

static void test_move_admission_helpers(void)
{
    DM2_V1_SkprojectMoveSideOffsetReceipt side;
    DM2_V1_SkprojectMoveAdmissionRequest request;
    DM2_V1_SkprojectMoveAdmissionReceipt receipt;

    memset(&request, 0, sizeof(request));
    request.creature_at_destination = -1;
    request.secondary_query_creature = -1;

    CHECK(dm2_v1_skproject_move_side_offset(
              0x0100u, 0, 7u, &side) == 0 &&
              side.valid && side.facing == 1u &&
              side.relative_direction == 1u &&
              side.side_direction &&
              side.creature_x_offset == 0 &&
              !side.side_offset_nonzero,
          "DM2_12b4_0953 admits side direction and returns zero at 5x5 center");
    CHECK(dm2_v1_skproject_move_side_offset(
              0x0100u, 0, 5u, &side) == 1 &&
              side.valid && side.creature_x_offset == -2 &&
              side.side_offset_nonzero,
          "DM2_12b4_0953 returns nonzero for side creature offset");
    CHECK(dm2_v1_skproject_move_side_offset(
              0x0000u, 0, 5u, &side) == 0 &&
              side.valid && side.relative_direction == 0u &&
              !side.side_direction,
          "DM2_12b4_0953 returns zero when relative direction is not side");

    request.requested_move = 2u;
    request.current_tile_value = (uint16_t)(3u << 5);
    CHECK(dm2_v1_skproject_move_admission(&request, &receipt) == 1 &&
              receipt.valid && receipt.result_code == 1u &&
              receipt.current_tile_type == 3u,
          "DM2_12b4_0881 code 1 fires for requested move 2 on tile type 3");

    memset(&request, 0, sizeof(request));
    request.creature_at_destination = -1;
    request.secondary_query_creature = -1;
    request.destination_tile_value = (uint16_t)(3u << 5);
    CHECK(dm2_v1_skproject_move_admission(&request, &receipt) == 2 &&
              receipt.result_code == 2u &&
              receipt.destination_tile_type == 3u,
          "DM2_12b4_0881 code 2 fires for destination tile type 3");

    memset(&request, 0, sizeof(request));
    request.creature_at_destination = -1;
    request.secondary_query_creature = -1;
    request.destination_tile_blocked = 1;
    CHECK(dm2_v1_skproject_move_admission(&request, &receipt) == 3 &&
              receipt.result_code == 3u,
          "DM2_12b4_0881 code 3 fires for blocked destination tile");

    memset(&request, 0, sizeof(request));
    request.creature_at_destination = 0x1234;
    request.creature_ai_flags = 0u;
    request.side_offset_nonzero = 0;
    CHECK(dm2_v1_skproject_move_admission(&request, &receipt) == 4 &&
              receipt.result_code == 4u &&
              receipt.stored_creature == 0x1234 &&
              receipt.used_side_offset_test,
          "DM2_12b4_0881 code 4 fires for creature blocked by side-offset test");

    request.side_offset_nonzero = 1;
    CHECK(dm2_v1_skproject_move_admission(&request, &receipt) == 5 &&
              receipt.result_code == 5u &&
              receipt.used_side_offset_test,
          "DM2_12b4_0881 code 5 fires when side-offset test admits creature");

    memset(&request, 0, sizeof(request));
    request.creature_at_destination = -1;
    request.secondary_query_creature = -1;
    CHECK(dm2_v1_skproject_move_admission(&request, &receipt) == 6 &&
              receipt.result_code == 6u &&
              receipt.used_secondary_query,
          "DM2_12b4_0881 code 6 fires when secondary query has no creature");

    request.secondary_query_creature = 0x22;
    request.secondary_query_ai_flags = 0u;
    CHECK(dm2_v1_skproject_move_admission(&request, &receipt) == 5 &&
              receipt.result_code == 5u &&
              receipt.used_secondary_query,
          "DM2_12b4_0881 secondary creature without 0x8000 flag returns 5");
    request.secondary_query_ai_flags = 0x8000u;
    CHECK(dm2_v1_skproject_move_admission(&request, &receipt) == 6 &&
              receipt.result_code == 6u,
          "DM2_12b4_0881 secondary creature with 0x8000 flag returns 6");
}

static void test_map_helpers(void)
{
    DM2_V1_SkprojectMinionDestinationReceipt dest;
    DM2_V1_SkprojectMapRandomReceipt random;
    DM2_V1_SkprojectMapTileVectorReceipt tile;
    DM2_V1_SkprojectMap3B001Receipt map3b001;
    uint8_t tiles[16];

    for (uint8_t i = 0; i < 16u; ++i)
        tiles[i] = (uint8_t)(0x80u + i);

    CHECK(dm2_v1_skproject_set_destination_of_minion_map(
              0xffffu, 4, 17, 9, 12, 32, 32, &dest) == 1 &&
              dest.valid && dest.in_bounds && dest.previous_map == 4 &&
              dest.new_record_w6 ==
                  (uint16_t)((12u << 10) | (9u << 5) | 17u),
          "DM2_SET_DESTINATION_OF_MINION_MAP packs x/y/map into record word 6");
    CHECK(dm2_v1_skproject_set_destination_of_minion_map(
              0x1234u, 4, 32, 9, 12, 32, 32, &dest) == 0 &&
              dest.valid && !dest.in_bounds &&
              dest.new_record_w6 == 0x1234u,
          "DM2_SET_DESTINATION_OF_MINION_MAP rejects out-of-bounds destination");

    CHECK(dm2_v1_skproject_map_0cee_17e7(
              0x07d3u, 0x0123u, 30u, 0x4567u, &random) ==
              (int)(((((uint32_t)0x07d3u * 0x7ab9u) / 2u) +
                     11u * 0x0123u + 0x4567u) >> 2) % 30 &&
              random.valid && random.result < 30u,
          "DM2_map_0cee_17e7 preserves source mixed modulo formula");
    CHECK(dm2_v1_skproject_map_0cee_17e7(
              1u, 2u, 0u, 3u, &random) == 0 &&
              !random.valid,
          "DM2_map_0cee_17e7 rejects zero modulo range");

    CHECK(dm2_v1_skproject_map_0cee_04e5(
              tiles, 4, 4, 0, 1, 1, 1, 2, &tile) == 0x86 &&
              tile.valid && tile.tile_x == 2 && tile.tile_y == 1 &&
              tile.tile_value == 0x86u,
          "DM2_map_0cee_04e5 applies CALC_VECTOR_W_DIR before tile lookup");
    CHECK(dm2_v1_skproject_map_0cee_04e5(
              tiles, 4, 4, 3, 3, 0, 0, 0, &tile) == 0 &&
              tile.blocked_out_of_bounds,
          "DM2_map_0cee_04e5 fails closed outside the caller tile map");
    CHECK(dm2_v1_skproject_map_0cee_04e5(
              0, 4, 4, 0, 0, 0, 0, 0, &tile) == 0 &&
              tile.blocked_missing_tiles,
          "DM2_map_0cee_04e5 rejects missing tile storage");

    CHECK(dm2_v1_skproject_map_3b001(
              7, 11, 12, &map3b001) == 1 &&
              map3b001.valid && map3b001.new_v1e0270 == 11 &&
              map3b001.new_v1e0272 == 12 &&
              map3b001.requested_change_to_previous_map,
          "DM2_map_3B001 stores v1e0270/v1e0272 and requests map restore");
    CHECK(dm2_v1_skproject_map_3b001(
              -1, 1, 2, &map3b001) == 1 &&
              !map3b001.requested_change_to_previous_map,
          "DM2_map_3B001 preserves no-op restore when previous map is -1");

    {
        uint8_t candidates[30];
        uint8_t alcoves[256] = { 0 };
        uint16_t words[4] = { 0 };
        uint8_t tmpmap[64] = { 0 };
        DM2_V1_SkprojectMap1815Receipt map1815;
        DM2_V1_SkprojectMap185AReceipt map185a;
        DM2_V1_SkprojectTmpmapFlagReceipt tmpflag;

        for (uint8_t i = 0; i < 30u; ++i)
            candidates[i] = (uint8_t)(0x40u + i);
        CHECK(dm2_v1_skproject_map_0cee_1815(
                  1, 12, 8, 3, 4, 5, 30, 0x2222u,
                  candidates, 30u, &map1815) ==
                  candidates[map1815.selected_index] &&
                  map1815.valid && map1815.selected_index < 30u &&
                  map1815.mixed_random != 0u,
              "DM2_map_0cee_1815 selects v1e02cc entry through skproject random mix");
        CHECK(dm2_v1_skproject_map_0cee_1815(
                  0, 12, 8, 3, 4, 5, 30, 0x2222u,
                  candidates, 30u, &map1815) == -1 &&
                  map1815.blocked_zero_gate,
              "DM2_map_0cee_1815 returns -1 when source gate is zero");

        alcoves[0xffu] = 1u;
        candidates[0] = 0xffu;
        CHECK(dm2_v1_skproject_map_0cee_185a(
                  words, 12, 8, 3, 1, 1, 1, 1, 20, -1, 0, 0,
                  0u, candidates, 1u, alcoves, &map185a) == 1 &&
                  map185a.valid && words[0] == 0x00ffu &&
                  words[1] == 0x00ffu && map185a.sanitized[0] &&
                  map185a.sanitized[3],
              "DM2_map_0cee_185a fills four words and sanitizes ornate alcoves outside map bounds");
        CHECK(dm2_v1_skproject_map_0cee_185a(
                  0, 12, 8, 3, 1, 1, 1, 1, 20, -1, 0, 0,
                  0u, candidates, 1u, alcoves, &map185a) == 0 &&
                  map185a.blocked_missing_output,
              "DM2_map_0cee_185a rejects missing output words");

        CHECK(dm2_v1_skproject_tmpmap_or_flag(
                  tmpmap, sizeof(tmpmap), 2, 3, 1, &tmpflag) == 1 &&
                  tmpflag.valid && tmpflag.offset == 21u &&
                  tmpmap[21] == 2u,
              "SKW_2066_1ea3 ORs bit 1 at the tmpmap pointer-derived byte");
        tmpmap[21] = 5u;
        CHECK(dm2_v1_skproject_tmpmap_or_flag(
                  tmpmap, sizeof(tmpmap), 2, 3, 1, &tmpflag) == 1 &&
                  tmpflag.previous_value == 5u && tmpflag.new_value == 7u,
              "SKW_2066_1ea3 preserves existing tmpmap flags");
        CHECK(dm2_v1_skproject_tmpmap_or_flag(
                  tmpmap, sizeof(tmpmap), 9, 9, 9, &tmpflag) == 0 &&
                  tmpflag.blocked_out_of_bounds,
              "SKW_2066_1ea3 rejects out-of-range tmpmap writes");
    }

    {
        DM2_V1_SkprojectMapRecord records[8];
        DM2_V1_SkprojectMap20661F37Receipt f37;
        DM2_V1_SkprojectMap20661EC9Receipt ec9;
        int16_t counter = 0;

        memset(records, 0, sizeof(records));
        records[0].next = 1u;
        records[1].next = 2u;
        records[1].record_type = 3u;
        records[1].w2 = 0x0027u;
        records[2].next = DM2_V1_SKPROJECT_MAP_RECORD_END;
        records[2].record_type = 3u;
        records[2].w2 = 0x00a7u;
        CHECK(dm2_v1_skproject_map_2066_1f37(
                  records, 8u, 0u, 4u, &counter, &f37) == 1 &&
                  f37.valid && f37.scanned_records == 2u &&
                  f37.matched_records == 2u &&
                  f37.updated_records == 1u && counter == 1 &&
                  records[1].w2 == (uint16_t)(0x0027u | (5u << 7)) &&
                  records[2].w2 == 0x00a7u,
              "DM2_map_2066_1f37 scans linked records and only updates unset type-0x27 records");

        memset(records, 0, sizeof(records));
        records[1].next = 2u;
        records[1].record_type = 2u;
        records[2].next = 6u;
        records[2].record_type = 3u;
        records[6].next = DM2_V1_SKPROJECT_MAP_RECORD_END;
        records[6].record_type = 5u;
        CHECK(dm2_v1_skproject_map_2066_1ec9(
                  records, 8u, 5u, 1u, &ec9) == 2u &&
                  ec9.valid && ec9.rewired_records == 2u &&
                  ec9.appended_tail == 6u &&
                  records[2].next == 6u &&
                  records[1].next == 5u,
              "DM2_map_2066_1ec9 prepends low-type record chain before existing head");
        CHECK(dm2_v1_skproject_map_2066_1ec9(
                  records, 8u, DM2_V1_SKPROJECT_MAP_RECORD_END, 3u,
                  &ec9) == 3u &&
                  ec9.valid && ec9.returned_head == 3u,
              "DM2_map_2066_1ec9 returns append chain when existing head is end marker");
    }

    {
        DM2_V1_SkprojectMapDescriptor maps[4];
        uint8_t cursor[4] = { 2u, 1u, 3u, 0xffu };
        DM2_V1_SkprojectLocateOtherLevelReceipt locate;
        DM2_V1_SkprojectMap3BF83Receipt map3bf83;
        uint16_t resume = 0u;
        int16_t x = 4;
        int16_t y = 2;

        memset(maps, 0, sizeof(maps));
        maps[0].map_id = 0u;
        maps[0].world_x = 20;
        maps[0].world_y = 40;
        maps[0].width = 10;
        maps[0].height = 8;
        maps[1].map_id = 1u;
        maps[1].world_x = 23;
        maps[1].world_y = 41;
        maps[1].width = 7;
        maps[1].height = 6;
        maps[1].tile_type_at_local = 5u;
        maps[1].teleporter_record_active = 1u;
        maps[2].map_id = 2u;
        maps[2].world_x = 24;
        maps[2].world_y = 42;
        maps[2].width = 8;
        maps[2].height = 7;
        maps[2].tile_type_at_local = 7u;
        maps[3].map_id = 3u;
        maps[3].world_x = 22;
        maps[3].world_y = 39;
        maps[3].width = 8;
        maps[3].height = 9;
        maps[3].tile_type_at_local = 1u;

        CHECK(dm2_v1_skproject_locate_other_level(
                  maps, 4u, 0, 1, &x, &y, cursor, 4u, 0u, &resume,
                  &locate) == 3 &&
                  locate.valid && locate.found &&
                  locate.scanned_candidates == 3u &&
                  locate.rejected_teleporter &&
                  locate.selected_x == 2 && locate.selected_y == 3 &&
                  x == 2 && y == 3 && resume == 3u,
              "DM_LOCATE_OTHER_LEVEL scans cursor maps, rejects wall/active teleporter, and returns local coordinates");
        CHECK(dm2_v1_skproject_locate_other_level(
                  maps, 4u, 0, 1, &x, &y, cursor, 4u, resume, &resume,
                  &locate) == -1 &&
                  locate.valid && !locate.found &&
                  locate.used_resume_cursor && resume == 0u,
              "DM_LOCATE_OTHER_LEVEL resumes after caller cursor and fails closed at terminator");
        CHECK(dm2_v1_skproject_locate_other_level(
                  maps, 4u, 8, 0, &x, &y, cursor, 4u, 0u, &resume,
                  &locate) == -1 &&
                  locate.blocked_missing_descriptors,
              "DM_LOCATE_OTHER_LEVEL rejects missing source map descriptor");

        CHECK(dm2_v1_skproject_map_3bf83(
                  5, 6, 2, 7, 2, 3, 4, 12, 9, &map3bf83) == 1 &&
                  map3bf83.valid && map3bf83.in_bounds &&
                  !map3bf83.target_differs_from_current &&
                  map3bf83.move_to_x == 5 && map3bf83.move_to_y == 6 &&
                  map3bf83.requested_party_rotate &&
                  map3bf83.rotation == 3,
              "DM2_map_3BF83 same-map route moves record to target square and rotates party");
        CHECK(dm2_v1_skproject_map_3bf83(
                  5, 6, 3, 1, 2, 3, 4, 12, 9, &map3bf83) == 1 &&
                  map3bf83.target_differs_from_current &&
                  map3bf83.requested_change_to_target &&
                  map3bf83.requested_restore_current &&
                  map3bf83.requested_load_newmap &&
                  map3bf83.move_from_x == 3 &&
                  map3bf83.move_from_y == 4 &&
                  map3bf83.move_to_x == -1 &&
                  map3bf83.move_to_y == 6,
              "DM2_map_3BF83 cross-map route plans restore, LOAD_NEWMAP, and source-shaped move target");
        CHECK(dm2_v1_skproject_map_3bf83(
                  13, 6, 3, 1, 2, 3, 4, 12, 9, &map3bf83) == 0 &&
                  map3bf83.valid && !map3bf83.in_bounds &&
                  map3bf83.requested_restore_current,
              "DM2_map_3BF83 out-of-bounds cross-map route only restores current map");
    }

    {
        DM2_V1_SkprojectArrowHighlightReceipt arrow;
        DM2_V1_SkprojectOtherLevelReceipt other_level;
        DM2_V1_SkprojectLiftRequest lift;
        DM2_V1_SkprojectLiftReceipt lift_receipt;
        DM2_V1_SkprojectWallAlcoveReceipt alcove;

        CHECK(dm2_v1_skproject_move_12b4_0092(
                  1u, 7u, -3, &arrow) == 1 &&
                  arrow.valid && arrow.requested_highlight &&
                  arrow.arrow_panel == 7u && arrow.highlight_param == -3,
              "DM2_move_12b4_0092 requests arrow-panel highlight only while v1e0534 is active");
        CHECK(dm2_v1_skproject_move_12b4_0092(
                  0u, 7u, -3, &arrow) == 0 &&
                  arrow.valid && !arrow.requested_highlight,
              "DM2_move_12b4_0092 is a no-op when v1e0534 is clear");

        CHECK(dm2_v1_skproject_move_12b4_00af(
                  0, 3, 10, 11, 4, 12, 13, 2, &other_level) == 1 &&
                  other_level.valid && other_level.requested_drop_record &&
                  other_level.locate_delta == 1 &&
                  other_level.located_map == 4 &&
                  other_level.final_party_dir == 2 &&
                  other_level.requested_restore_source_map,
              "DM2_move_12b4_00af plans source drop, locate-other-level, rotation, and source-map restore");
        CHECK(dm2_v1_skproject_move_12b4_00af(
                  1, 3, 10, 11, 4, 12, 13, 7, &other_level) == 1 &&
                  other_level.locate_delta == -1 &&
                  other_level.final_party_dir == 3,
              "DM2_move_12b4_00af reverses locate delta for forward transition and masks rotation");

        {
            DM2_V1_SkprojectAttackDoorReceipt door;
            DM2_V1_SkprojectWallAttackRecord wall_records[4];
            DM2_V1_SkprojectAttackWallReceipt wall;

            CHECK(dm2_v1_skproject_attack_door(
                      4u, 0u, 0u, 10u, 5u, 0, 0, 0u, 6, 7,
                      &door) == 0 &&
                      door.valid && door.blocked_door_closed_flag &&
                      door.test_byte_offset == 3u &&
                      door.tested_flag_mask == 1u,
                  "DM2_ATTACK_DOOR rejects byte3 bit0 gate when normal door flag is clear");
            CHECK(dm2_v1_skproject_attack_door(
                      4u, 0u, 1u, 4u, 5u, 0, 0, 0u, 6, 7,
                      &door) == 0 &&
                      door.blocked_attack_power &&
                      door.attack_power == 4u &&
                      door.required_power == 5u,
                  "DM2_ATTACK_DOOR rejects attacks below GET_DOOR_STAT_0X10 threshold");
            CHECK(dm2_v1_skproject_attack_door(
                      3u, 0u, 1u, 10u, 5u, 0, 0, 0u, 6, 7,
                      &door) == 0 &&
                      door.blocked_tile_type &&
                      door.tile_type_before == 3u &&
                      door.tile_type_after == 3u,
                  "DM2_ATTACK_DOOR rejects non-door tile types before mutation");
            CHECK(dm2_v1_skproject_attack_door(
                      4u, 0x80u, 0u, 10u, 5u, 1, 1, 3u, 6, 7,
                      &door) == 1 &&
                      door.admitted && door.queued_timer &&
                      !door.changed_tile_type &&
                      door.rebirth_altar &&
                      door.test_byte_offset == 2u &&
                      door.tested_flag_mask == 0x80u &&
                      door.timer_ticks == 3u,
                  "DM2_ATTACK_DOOR queues timer route for byte2-gated delayed destruction");
            CHECK(dm2_v1_skproject_attack_door(
                      4u, 0u, 1u, 10u, 5u, 0, 0, 0u, 6, 7,
                      &door) == 1 &&
                      door.admitted && door.changed_tile_type &&
                      !door.queued_timer &&
                      door.tile_type_after == 5u &&
                      door.x == 6 && door.y == 7,
                  "DM2_ATTACK_DOOR opens tile type 4 to 5 when no timer delay is requested");

            memset(wall_records, 0, sizeof(wall_records));
            wall_records[0].link_word = (uint16_t)(1u << 14);
            wall_records[0].alcove_data_index = 0x222u;
            CHECK(dm2_v1_skproject_attack_wall(
                      wall_records, 1u, 0x0400u, 0x33u, 3, 0u, 8, 9,
                      &wall) == 1 &&
                      wall.valid && wall.found_effect &&
                      wall.used_alcove_relocation &&
                      wall.projectile_cut &&
                      wall.projectile_side_after == 1u &&
                      wall.matching_side_records == 1u,
                  "DM2_ATTACK_WALL relocates missile through ornate alcove only on matching side and RANDDIR zero");

            memset(wall_records, 0, sizeof(wall_records));
            wall_records[0].link_word = (uint16_t)(1u << 14);
            wall_records[0].record_type = 3u;
            wall_records[0].actuator_class = 0x22u;
            wall_records[0].required_item_type = 0x44u;
            wall_records[0].consume_projectile = 1u;
            CHECK(dm2_v1_skproject_attack_wall(
                      wall_records, 1u, 0x0400u, 0x44u, 3, 2u, 8, 9,
                      &wall) == 1 &&
                      wall.invoked_actuator &&
                      wall.projectile_cut &&
                      wall.projectile_side_after == 1u &&
                      !wall.used_alcove_relocation,
                  "DM2_ATTACK_WALL invokes class-0x22 actuator and consumes matching projectile");

            wall_records[0].target_flag = 1u;
            CHECK(dm2_v1_skproject_attack_wall(
                      wall_records, 1u, 0x0400u, 0x44u, 3, 2u, 8, 9,
                      &wall) == 0 &&
                      !wall.found_effect &&
                      wall.matching_side_records == 1u,
                  "DM2_ATTACK_WALL honours class-0x22 target flag inversion");

            memset(wall_records, 0, sizeof(wall_records));
            wall_records[0].link_word = (uint16_t)(1u << 14);
            wall_records[0].record_type = 3u;
            wall_records[0].actuator_class = 0x23u;
            wall_records[0].required_item_type = 0x01ffu;
            wall_records[0].tile_type_at_destination = 0u;
            wall_records[0].destination_x = 12;
            wall_records[0].destination_y = 14;
            wall_records[0].side_when_tile_zero = 2u;
            CHECK(dm2_v1_skproject_attack_wall(
                      wall_records, 1u, 0x0400u, 0x77u, 3, 2u, 8, 9,
                      &wall) == 1 &&
                      wall.used_teleport_relocation &&
                      wall.projectile_cut &&
                      wall.target_x == 12 && wall.target_y == 14 &&
                      wall.projectile_side_after == 2u,
                  "DM2_ATTACK_WALL teleports wildcard class-0x23 projectile using destination tile side rule");
        }

        memset(&lift, 0, sizeof(lift));
        lift.creature_weight = 40u;
        lift.event_hero_index = 1u;
        lift.hero_count = 3u;
        lift.heroes[0].alive = 1u;
        lift.heroes[0].strength = 20u;
        lift.heroes[0].stamina_adjusted_strength = 10u;
        lift.heroes[0].max_stamina = 160u;
        lift.heroes[0].cur_stamina = 80u;
        lift.rand16_values[0] = 2u;
        lift.heroes[1].alive = 1u;
        lift.heroes[1].strength = 36u;
        lift.heroes[1].max_stamina = 160u;
        lift.heroes[1].cur_stamina = 80u;
        lift.rand16_values[1] = 2u;
        CHECK(dm2_v1_skproject_move_12b4_099e(
                  &lift, &lift_receipt) == 1 &&
                  lift_receipt.valid && lift_receipt.can_lift &&
                  lift_receipt.checked_heroes == 2u &&
                  lift_receipt.stamina_adjustments[0] == 10u &&
                  lift_receipt.stamina_adjustments[1] == 10u,
              "DM2_move_12b4_099e admits lift through adjusted event-hero strength and drains living stamina");
        lift.creature_weight = 300u;
        CHECK(dm2_v1_skproject_move_12b4_099e(
                  &lift, &lift_receipt) == 0 &&
                  lift_receipt.valid &&
                  lift_receipt.blocked_overweight_creature,
              "DM2_move_12b4_099e rejects creature weights above 0xfd");

        CHECK(dm2_v1_skproject_wall_ornate_alcove_data_index(
                  1, 0x12, 0x345u, &alcove) == 1 &&
                  alcove.valid && alcove.ornate_alcove &&
                  alcove.cls2 == 0x12u &&
                  alcove.data_index == 0x345u,
              "DM2_0cee_317f queries GDAT data index for ornate wall alcove cls2");
        CHECK(dm2_v1_skproject_wall_ornate_alcove_data_index(
                  1, -1, 0x345u, &alcove) == 0 &&
                  alcove.cls2_missing,
              "DM2_0cee_317f rejects missing cls2 before GDAT lookup");
        CHECK(dm2_v1_skproject_wall_ornate_alcove_data_index(
                  0, 0x12, 0x345u, &alcove) == 0 &&
                  !alcove.valid && !alcove.ornate_alcove,
              "DM2_0cee_317f returns zero for non-ornate wall records");
    }
}

static void test_calc_vector_w_dir(void)
{
    DM2_V1_SkprojectVectorWDirReceipt receipt;
    int16_t x;
    int16_t y;

    x = 10;
    y = 20;
    CHECK(dm2_v1_skproject_calc_vector_w_dir(
              0, 3, 2, &x, &y, &receipt) == 1 &&
              x == 12 && y == 17 &&
              receipt.valid && receipt.dir == 0u &&
              receipt.forward_dx == 0 && receipt.forward_dy == -3 &&
              receipt.side_dx == 2 && receipt.side_dy == 0 &&
              receipt.initial_x == 10 && receipt.initial_y == 20,
          "CALC_VECTOR_W_DIR dir north adds forward and right-hand side deltas");

    x = 10;
    y = 20;
    CHECK(dm2_v1_skproject_calc_vector_w_dir(
              1, -1, 4, &x, &y, &receipt) == 1 &&
              x == 9 && y == 24 &&
              receipt.forward_dx == -1 && receipt.forward_dy == 0 &&
              receipt.side_dx == 0 && receipt.side_dy == 4,
          "CALC_VECTOR_W_DIR dir east preserves signed source operands");

    x = 10;
    y = 20;
    CHECK(dm2_v1_skproject_calc_vector_w_dir(
              3, 5, -2, &x, &y, &receipt) == 1 &&
              x == 5 && y == 22 &&
              receipt.forward_dx == -5 && receipt.forward_dy == 0 &&
              receipt.side_dx == 0 && receipt.side_dy == 2,
          "CALC_VECTOR_W_DIR dir west wraps side direction to north");

    x = -7;
    y = 8;
    CHECK(dm2_v1_skproject_calc_vector_w_dir(
              5, 2, 3, &x, &y, &receipt) == 1 &&
              x == -5 && y == 11 && receipt.dir == 1u,
          "CALC_VECTOR_W_DIR masks direction like the source table index");

    CHECK(dm2_v1_skproject_calc_vector_w_dir(
              0, 1, 1, 0, &y, &receipt) == 0 &&
              receipt.blocked_missing_output,
          "CALC_VECTOR_W_DIR rejects missing output accumulator");
}

static void test_cache_hash_helpers(void)
{
    DM2_V1_SkprojectCacheState state;
    uint16_t ici = 0xffffu;
    uint16_t cache_index = 0xffffu;
    uint8_t *buff;

    dm2_v1_skproject_cache_state_init(&state, 4, 3, 4);
    state.raw_to_mement[2] = 3u;

    CHECK(state.cache_capacity == 4 && state.raw_count == 3 &&
              state.mement_count == 4,
          "cache state clamps source table sizes");
    CHECK(dm2_v1_skproject_find_ici_from_cache_hash(
              &state, 0x2000u, &ici) == 0 && ici == 0u,
          "FIND_ICI_FROM_CACHE_HASH returns insertion slot for empty table");
    CHECK(dm2_v1_skproject_insert_cache_hash_at(
              &state, 0x2000u, ici) == 0u,
          "INSERT_CACHE_HASH_AT inserts the first cache hash");
    CHECK(dm2_v1_skproject_find_ici_from_cache_hash(
              &state, 0x1000u, &ici) == 0 && ici == 0u,
          "FIND_ICI_FROM_CACHE_HASH finds lower insertion point");
    CHECK(dm2_v1_skproject_insert_cache_hash_at(
              &state, 0x1000u, ici) == 1u,
          "INSERT_CACHE_HASH_AT preserves sorted hash order");
    CHECK(state.cache_count == 2 &&
              state.sorted_cache_indices[0] == 1u &&
              state.sorted_cache_indices[1] == 0u,
          "sorted cache index table mirrors skproject indirect order");
    CHECK(dm2_v1_skproject_find_ici_from_cache_hash(
              &state, 0x2000u, &ici) == 1 && ici == 1u,
          "FIND_ICI_FROM_CACHE_HASH returns existing sorted index");
    CHECK(dm2_v1_skproject_add_cache_hash(
              &state, 0x2000u, &cache_index) == 1 &&
              cache_index == 0u,
          "ADD_CACHE_HASH returns existing cache index");
    CHECK(dm2_v1_skproject_add_cache_hash(
              &state, 0x3000u, &cache_index) == 0 &&
              cache_index == 2u,
          "ADD_CACHE_HASH inserts a new cache index");
    CHECK(dm2_v1_skproject_query_mementi_from(&state, 0x8000u) == 0u &&
              dm2_v1_skproject_query_mementi_from(&state, 0x8001u) == 1u &&
              dm2_v1_skproject_query_mementi_from(&state, 2u) == 3u,
          "QUERY_MEMENTI_FROM handles cache-index and raw-data routes");
    CHECK(dm2_v1_skproject_query_mementi_from(&state, 0x8008u) ==
              DM2_V1_SKPROJECT_MEMENT_NONE,
          "QUERY_MEMENTI_FROM rejects out-of-range cache index");
    buff = dm2_v1_skproject_query_mement_buff_from_cache_index(&state, 1u);
    CHECK(buff == state.mement_buffers[1],
          "QUERY_MEMENT_BUFF_FROM_CACHE_INDEX returns mement payload bytes");
    CHECK(dm2_v1_skproject_get_temp_cache_hash(&state) == 0xffff0000u,
          "GET_TEMP_CACHE_HASH starts in the source temp hash range");
    CHECK(dm2_v1_skproject_alloc_temp_cache_index(&state) == 3u &&
              state.hashes[3] == 0xffff0000u &&
              state.temp_hash_counter == 1u,
          "ALLOC_TEMP_CACHE_INDEX allocates a temp hash through ADD_CACHE_HASH");
    CHECK(dm2_v1_skproject_alloc_temp_cache_index(&state) ==
              DM2_V1_SKPROJECT_MEMENT_NONE,
          "ALLOC_TEMP_CACHE_INDEX fails closed when cache table is full");
}

static void test_picture_mement_helpers(void)
{
    DM2_V1_SkprojectCacheState state;
    DM2_V1_SkprojectNewPictReceipt new_pict;
    DM2_V1_SkprojectExtendedPictureRef ext;
    DM2_V1_SkprojectImageMementRequest image;
    DM2_V1_SkprojectPictureRef pict;
    DM2_V1_SkprojectImageMementReceipt image_receipt;
    DM2_V1_SkprojectPictMementReceipt pict_receipt;
    DM2_V1_SkprojectFreeImageMementReceipt free_receipt;
    DM2_V1_SkprojectRecycleMementReceipt recycle_receipt;
    uint16_t pinned_entry = DM2_V1_SKPROJECT_MEMENT_NONE;

    dm2_v1_skproject_cache_state_init(&state, 4, 8, 4);
    CHECK(dm2_v1_skproject_test_mement(-20, -20) == 1,
          "TEST_MEMENT accepts matching stored length");
    CHECK(dm2_v1_skproject_test_mement(-20, -24) == 0,
          "TEST_MEMENT rejects mismatched stored length");
    CHECK(dm2_v1_skproject_alloc_new_pict(
              7u, 13u, 5u, 4u, &new_pict) == 1 &&
              new_pict.payload_bytes == 35u &&
              new_pict.header_width == 13u &&
              new_pict.header_height == 5u &&
              new_pict.header_bpp == 4u,
          "ALLOC_NEW_PICT stores headers and 4bpp rounded row bytes");
    CHECK(dm2_v1_skproject_alloc_new_pict(
              8u, 13u, 5u, 8u, &new_pict) == 1 &&
              new_pict.payload_bytes == 65u,
          "ALLOC_NEW_PICT keeps 8bpp row bytes unrounded");

    ext.w6 = 0x3fffu;
    ext.w52 = 0x00ffu;
    ext.w54 = 0x003fu;
    CHECK(dm2_v1_skproject_calc_pict_ent_hash(&ext) ==
              (((uint32_t)0x1fffu << 12) | ((uint32_t)0x7fu << 5) | 0x1fu),
          "CALC_PICT_ENT_HASH masks and packs w6/w52/w54");

    memset(&image, 0, sizeof(image));
    image.cls1 = 1u;
    image.cls2 = 2u;
    image.cls4 = 3u;
    image.data_index = 5u;
    image.fallback_data_index = 9u;
    image.y_offset = -32;
    image.bits_pixel = 8u;
    image.existing_mementi = DM2_V1_SKPROJECT_MEMENT_NONE;
    CHECK(dm2_v1_skproject_alloc_image_mement(
              &state, &image, &pinned_entry, &image_receipt) == 1 &&
              image_receipt.status ==
                  DM2_V1_SKPROJECT_IMAGE_MEMENT_PINNED_ENTRY &&
              pinned_entry == 5u,
          "ALLOC_IMAGE_MEMENT pins 8bpp Y=-32 real image entry");
    image.y_offset = -31;
    pinned_entry = DM2_V1_SKPROJECT_MEMENT_NONE;
    CHECK(dm2_v1_skproject_alloc_image_mement(
              &state, &image, &pinned_entry, &image_receipt) == 1 &&
              image_receipt.status ==
                  DM2_V1_SKPROJECT_IMAGE_MEMENT_REJECT_Y_OFFSET &&
              pinned_entry == DM2_V1_SKPROJECT_MEMENT_NONE,
          "ALLOC_IMAGE_MEMENT rejects non-source Y offset");
    image.y_offset = -32;
    image.bits_pixel = 4u;
    CHECK(dm2_v1_skproject_alloc_image_mement(
              &state, &image, &pinned_entry, &image_receipt) == 1 &&
              image_receipt.status ==
                  DM2_V1_SKPROJECT_IMAGE_MEMENT_REJECT_BPP,
          "ALLOC_IMAGE_MEMENT rejects non-8bpp image mement");
    image.bits_pixel = 8u;
    image.data_absent = 1;
    image.fallback_absent = 0;
    CHECK(dm2_v1_skproject_alloc_image_mement(
              &state, &image, &pinned_entry, &image_receipt) == 1 &&
              image_receipt.status ==
                  DM2_V1_SKPROJECT_IMAGE_MEMENT_PINNED_ENTRY &&
              image_receipt.selected_data_index == 9u,
          "ALLOC_IMAGE_MEMENT falls back to default when primary is absent");
    image.data_index = DM2_V1_SKPROJECT_MEMENT_NONE;
    CHECK(dm2_v1_skproject_alloc_image_mement(
              &state, &image, &pinned_entry, &image_receipt) == 1 &&
              image_receipt.status == DM2_V1_SKPROJECT_IMAGE_MEMENT_NO_ENTRY,
          "ALLOC_IMAGE_MEMENT does not fabricate fallback for missing primary");
    image.data_index = 5u;
    image.data_absent = 0;
    image.fallback_absent = 1;
    image.existing_mementi = 2u;
    CHECK(dm2_v1_skproject_alloc_image_mement(
              &state, &image, &pinned_entry, &image_receipt) == 1 &&
              image_receipt.status ==
                  DM2_V1_SKPROJECT_IMAGE_MEMENT_TOUCHED_EXISTING &&
              image_receipt.touched_mementi == 2u,
          "ALLOC_IMAGE_MEMENT touches existing mement instead of pinning");

    CHECK(dm2_v1_skproject_recycle_mementi(
              &state, 2u, DM2_V1_SKPROJECT_MEMENT_NONE, 0u,
              &recycle_receipt) == 1 &&
              recycle_receipt.valid &&
              recycle_receipt.recycled_to_free_list,
          "RECYCLE_MEMENTI records free-list recycle for w4=0xffff");

    image.existing_mementi = 1u;
    state.raw_to_mement[5] = 1u;
    pinned_entry = 5u;
    CHECK(dm2_v1_skproject_free_image_mement(
              &state, &image, &pinned_entry, &free_receipt) == 1 &&
              free_receipt.cleared_pinned_entry &&
              free_receipt.recycled_existing &&
              state.raw_to_mement[5] == DM2_V1_SKPROJECT_MEMENT_NONE,
          "FREE_IMAGE_MEMENT clears pinned entry and recycles existing mement");

    memset(&pict, 0, sizeof(pict));
    pict.w4 = 0x0004u;
    image.existing_mementi = DM2_V1_SKPROJECT_MEMENT_NONE;
    pinned_entry = DM2_V1_SKPROJECT_MEMENT_NONE;
    CHECK(dm2_v1_skproject_alloc_pict_mement(
              &state, &pict, &image, &pinned_entry, &pict_receipt) == 1 &&
              pict_receipt.route == DM2_V1_SKPROJECT_PICT_MEMENT_IMAGE,
          "ALLOC_PICT_MEMENT routes image-backed pictures to image mement");
    pict.w4 = 0x0008u;
    pict.w12 = 2u;
    CHECK(dm2_v1_skproject_alloc_pict_mement(
              &state, &pict, &image, &pinned_entry, &pict_receipt) == 1 &&
              pict_receipt.route == DM2_V1_SKPROJECT_PICT_MEMENT_CACHE &&
              pict_receipt.cache_index == 2u && state.cache_count == 0u,
          "ALLOC_PICT_MEMENT routes cache-backed pictures by w12 index");
    state.cache_to_mement[2] = 2u;
    CHECK(dm2_v1_skproject_free_pict_mement(
              &state, &pict, &image, &pinned_entry, &free_receipt) == 1 &&
              state.cache_to_mement[2] == DM2_V1_SKPROJECT_MEMENT_NONE,
          "FREE_PICT_MEMENT frees cache-backed pictures by w12 index");
}

static void test_item_charge_helpers(void)
{
    DM2_V1_SkprojectItemChargeReceipt receipt;
    uint16_t w2;

    CHECK(dm2_v1_skproject_get_max_charge(0x1400u) == 15u,
          "GET_MAX_CHARGE returns 15 for DB5 weapon");
    CHECK(dm2_v1_skproject_get_max_charge(0x1800u) == 15u,
          "GET_MAX_CHARGE returns 15 for DB6 cloth");
    CHECK(dm2_v1_skproject_get_max_charge(0x2800u) == 3u,
          "GET_MAX_CHARGE returns 3 for DB10 miscellaneous item");
    CHECK(dm2_v1_skproject_get_max_charge(0xffffu) == 0u,
          "GET_MAX_CHARGE returns zero for OBJECT_NULL");
    CHECK(dm2_v1_skproject_get_max_charge(0x0800u) == 0u,
          "GET_MAX_CHARGE returns zero for unsupported DB type");

    w2 = (uint16_t)(7u << 10);
    CHECK(dm2_v1_skproject_add_item_charge(0x1400u, &w2, 3, &receipt) ==
              10u &&
              receipt.valid && receipt.db_type == 5 &&
              receipt.previous_charge == 7u && receipt.new_charge == 10u &&
              receipt.max_charge == 15u && ((w2 >> 10) & 0x0fu) == 10u,
          "ADD_ITEM_CHARGE updates DB5 weapon charges in bits 10..13");

    w2 = (uint16_t)(14u << 10);
    CHECK(dm2_v1_skproject_add_item_charge(0x1400u, &w2, 9, &receipt) ==
              15u &&
              ((w2 >> 10) & 0x0fu) == 15u,
          "ADD_ITEM_CHARGE clamps DB5 weapon charges to 15");

    w2 = (uint16_t)(2u << 9);
    CHECK(dm2_v1_skproject_add_item_charge(0x1800u, &w2, -5, &receipt) ==
              0u &&
              receipt.valid && receipt.db_type == 6 &&
              ((w2 >> 9) & 0x0fu) == 0u,
          "ADD_ITEM_CHARGE clamps DB6 cloth charges to zero");

    w2 = (uint16_t)(1u << 14);
    CHECK(dm2_v1_skproject_add_item_charge(0x2800u, &w2, 5, &receipt) ==
              3u &&
              receipt.valid && receipt.db_type == 10 &&
              receipt.max_charge == 3u && ((w2 >> 14) & 0x03u) == 3u,
          "ADD_ITEM_CHARGE clamps DB10 miscellaneous charges to 3");

    w2 = 0xaaaau;
    CHECK(dm2_v1_skproject_add_item_charge(0xffffu, &w2, 1, &receipt) ==
              0u &&
              receipt.blocked_null_object && w2 == 0xaaaau,
          "ADD_ITEM_CHARGE rejects OBJECT_NULL without mutation");

    w2 = 0x5555u;
    CHECK(dm2_v1_skproject_add_item_charge(0x0800u, &w2, 1, &receipt) ==
              0u &&
              receipt.blocked_unsupported_db_type && w2 == 0x5555u,
          "ADD_ITEM_CHARGE rejects unsupported DB type without mutation");
}

static void test_item_value_weight_helpers(void)
{
    DM2_V1_SkprojectItemValueRecord records[8];
    DM2_V1_SkprojectItemValueWorld world;
    DM2_V1_SkprojectItemValueReceipt receipt;

    memset(records, 0, sizeof(records));
    world.records = records;
    world.record_count = 8u;

    records[0].object_id = 0x1400u;
    records[0].w2 = (uint16_t)(3u << 10);
    records[0].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[0].gdat_word_values[1] = 4u;
    records[0].gdat_word_values[2] = 20u;
    records[0].gdat_word_values[0x34] = 2u;
    records[0].gdat_word_values[0x35] = 5u;

    records[1].object_id = 0x2001u;
    records[1].w2 = 128u;
    records[1].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[1].gdat_word_values[2] = 100u;

    records[2].object_id = 0x2402u;
    records[2].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[2].contained_object_id = 0x1400u;
    records[2].container_type = 0u;
    records[2].gdat_word_values[1] = 10u;

    records[3].object_id = 0x2403u;
    records[3].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[3].contained_object_id = 0x2804u;
    records[3].container_type = 0u;
    records[3].is_moneybox = 1u;
    records[3].gdat_word_values[1] = 1u;

    records[4].object_id = 0x2804u;
    records[4].w2 = (uint16_t)(2u << 14);
    records[4].next_object_id = 0x2805u;
    records[4].gdat_word_values[1] = 5u;
    records[4].gdat_word_values[2] = 7u;

    records[5].object_id = 0x2805u;
    records[5].w2 = 0u;
    records[5].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[5].gdat_word_values[1] = 6u;
    records[5].gdat_word_values[2] = 11u;

    records[6].object_id = 0x2406u;
    records[6].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[6].container_type = 1u;
    records[6].gdat_word_values[1] = 9u;

    records[7].object_id = 0x1807u;
    records[7].w2 = (uint16_t)(1u << 9);
    records[7].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[7].gdat_word_values[1] = 3u;
    records[7].gdat_word_values[0x34] = 4u;

    CHECK(dm2_v1_skproject_query_item_value(
              &world, 0x1400u, 1u, &receipt) == 10 &&
              receipt.valid && receipt.base_value == 4 &&
              receipt.charge == 3u && receipt.charge_value_added == 6,
          "QUERY_ITEM_VALUE adds cls4=0x34 weight per charge");
    CHECK(dm2_v1_skproject_query_item_value(
              &world, 0x1400u, 2u, &receipt) == 35 &&
              receipt.valid && receipt.base_value == 20 &&
              receipt.charge_value_added == 15,
          "QUERY_ITEM_VALUE adds cls4=0x35 money per charge");
    CHECK(dm2_v1_skproject_query_item_value(
              &world, 0x2001u, 2u, &receipt) == 75 &&
              receipt.potion_value_before_scale == 100 &&
              receipt.potion_value_after_scale == 75,
          "QUERY_ITEM_VALUE scales potion money by low-byte power");
    CHECK(dm2_v1_skproject_query_item_value(
              &world, 0x2402u, 1u, &receipt) == 20 &&
              receipt.contained_recursive_value == 10,
          "QUERY_ITEM_VALUE recurses normal container contents");
    CHECK(dm2_v1_skproject_query_item_value(
              &world, 0x2403u, 1u, &receipt) == 6 &&
              receipt.moneybox_contained_value == 21 &&
              receipt.moneybox_rounding_value == 5,
          "QUERY_ITEM_VALUE rounds moneybox weight as (sum+4)/5");
    CHECK(dm2_v1_skproject_query_item_value(
              &world, 0x2403u, 2u, &receipt) == 32 &&
              receipt.moneybox_contained_value == 32 &&
              receipt.moneybox_rounding_value == 32,
          "QUERY_ITEM_VALUE adds moneybox non-weight value directly");
    CHECK(dm2_v1_skproject_query_item_weight(
              &world, 0x1807u, &receipt) == 7 &&
              receipt.charge_multiplier_cls4 == 0x34u,
          "QUERY_ITEM_WEIGHT delegates to QUERY_ITEM_VALUE cls4=1");
    CHECK(dm2_v1_skproject_query_item_value(
              &world, DM2_V1_SKPROJECT_MEMENT_NONE, 1u, &receipt) == 0 &&
              receipt.blocked_null_object,
          "QUERY_ITEM_VALUE rejects OBJECT_NULL");
    CHECK(dm2_v1_skproject_query_item_value(
              &world, 0x1410u, 1u, &receipt) == 0 &&
              receipt.blocked_missing_record,
          "QUERY_ITEM_VALUE rejects missing source record");
}

static void test_player_weight_helper(void)
{
    DM2_V1_SkprojectItemValueRecord records[4];
    DM2_V1_SkprojectItemValueWorld world;
    DM2_V1_SkprojectPlayerWeightRequest request;
    DM2_V1_SkprojectPlayerWeightReceipt receipt;

    memset(records, 0, sizeof(records));
    memset(&request, 0xff, sizeof(request));
    world.records = records;
    world.record_count = 4u;

    records[0].object_id = 0x1400u;
    records[0].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[0].gdat_word_values[1] = 4u;
    records[1].object_id = 0x1801u;
    records[1].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[1].gdat_word_values[1] = 5u;
    records[2].object_id = 0x2402u;
    records[2].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[2].container_type = 0u;
    records[3].object_id = 0x2803u;
    records[3].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[3].gdat_word_values[1] = 6u;

    request.inventory[0] = 0x1400u;
    request.inventory[1] = 0x1801u;
    for (uint16_t i = 2u; i < DM2_V1_SKPROJECT_PLAYER_INVENTORY_SLOTS; ++i)
        request.inventory[i] = DM2_V1_SKPROJECT_MEMENT_NONE;
    request.current_container_items[0] = 0x2803u;
    for (uint16_t i = 1u; i < DM2_V1_SKPROJECT_CURRENT_CONTAINER_SLOTS; ++i)
        request.current_container_items[i] = DM2_V1_SKPROJECT_MEMENT_NONE;
    request.selected_hand_items[0] = 0x2402u;
    request.selected_hand_items[1] = DM2_V1_SKPROJECT_MEMENT_NONE;
    request.selected_hand_action = 0u;
    request.selected_player_plus_one = 1u;

    CHECK(dm2_v1_skproject_calc_player_weight(
              &world, 0u, &request, &receipt) == 1 &&
              receipt.valid && receipt.inventory_weight == 9u &&
              receipt.open_chest_weight == 6u &&
              receipt.final_weight == 15u &&
              receipt.included_open_chest_overlay &&
              receipt.hero_flag_or == 0x1000u,
          "CALC_PLAYER_WEIGHT sums inventory and selected open chest overlay");

    request.selected_hand_action = 2u;
    CHECK(dm2_v1_skproject_calc_player_weight(
              &world, 0u, &request, &receipt) == 1 &&
              receipt.final_weight == 9u &&
              receipt.blocked_selected_hand_action &&
              !receipt.included_open_chest_overlay,
          "CALC_PLAYER_WEIGHT skips overlay when selected hand action is not 0/1");

    request.selected_hand_action = 0u;
    request.selected_player_plus_one = 2u;
    CHECK(dm2_v1_skproject_calc_player_weight(
              &world, 0u, &request, &receipt) == 1 &&
              receipt.final_weight == 9u &&
              receipt.blocked_player_not_selected,
          "CALC_PLAYER_WEIGHT skips overlay for non-selected player");
}

static void test_count_by_coin_types(void)
{
    DM2_V1_SkprojectItemValueRecord records[6];
    DM2_V1_SkprojectItemValueWorld world;
    DM2_V1_SkprojectCountByCoinTypesReceipt receipt;
    uint16_t money_ids[DM2_V1_SKPROJECT_MONEY_ITEM_MAX] = {
        0x10u, 0x20u, 0x20u, 0x30u, 0x40u,
        0x50u, 0x60u, 0x70u, 0x80u, 0x90u
    };
    int16_t counts[DM2_V1_SKPROJECT_MONEY_ITEM_MAX];

    memset(records, 0, sizeof(records));
    for (uint16_t i = 0; i < DM2_V1_SKPROJECT_MONEY_ITEM_MAX; ++i)
        counts[i] = -7;
    world.records = records;
    world.record_count = 6u;

    records[0].object_id = 0x2400u;
    records[0].contained_object_id = 0x2801u;
    records[0].container_type = 0u;
    records[0].is_moneybox = 1u;

    records[1].object_id = 0x2801u;
    records[1].w2 = (uint16_t)(2u << 14);
    records[1].next_object_id = 0x2802u;
    records[1].distinctive_item_type = 0x20u;
    records[1].is_currency = 1u;

    records[2].object_id = 0x2802u;
    records[2].w2 = 0u;
    records[2].next_object_id = 0x2803u;
    records[2].distinctive_item_type = 0x10u;
    records[2].is_currency = 1u;

    records[3].object_id = 0x2803u;
    records[3].w2 = (uint16_t)(3u << 14);
    records[3].next_object_id = 0x1404u;
    records[3].distinctive_item_type = 0x99u;
    records[3].is_currency = 1u;

    records[4].object_id = 0x1404u;
    records[4].w2 = (uint16_t)(1u << 10);
    records[4].next_object_id = 0x2805u;
    records[4].distinctive_item_type = 0x30u;
    records[4].is_currency = 1u;

    records[5].object_id = 0x2805u;
    records[5].w2 = (uint16_t)(1u << 14);
    records[5].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[5].distinctive_item_type = 0x30u;
    records[5].is_currency = 0u;

    CHECK(dm2_v1_skproject_count_by_coin_types(
              &world, 0x2400u, money_ids,
              DM2_V1_SKPROJECT_MONEY_ITEM_MAX, counts, &receipt) == 1 &&
              receipt.valid && receipt.visited_records == 5u &&
              receipt.currency_records == 3u &&
              receipt.matched_currency_records == 3u &&
              counts[0] == 1 && counts[1] == 3 && counts[2] == 3 &&
              counts[3] == 0 && counts[9] == 0,
          "COUNT_BY_COIN_TYPES zeroes ten slots and adds charge+1 by money type");

    CHECK(dm2_v1_skproject_count_by_coin_types(
              &world, 0x2400u, money_ids, 2u, counts, &receipt) == 1 &&
              receipt.valid && receipt.money_item_count == 2u &&
              counts[0] == 1 && counts[1] == 3 && counts[2] == 0,
          "COUNT_BY_COIN_TYPES honors caller money table count after zeroing");

    records[5].next_object_id = 0x2810u;
    CHECK(dm2_v1_skproject_count_by_coin_types(
              &world, 0x2400u, money_ids,
              DM2_V1_SKPROJECT_MONEY_ITEM_MAX, counts, &receipt) == 0 &&
              receipt.blocked_missing_record,
          "COUNT_BY_COIN_TYPES rejects missing source-shaped chain record");
    records[5].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;

    CHECK(dm2_v1_skproject_count_by_coin_types(
              &world, 0x2400u, money_ids,
              DM2_V1_SKPROJECT_MONEY_ITEM_MAX, 0, &receipt) == 0 &&
              receipt.blocked_missing_output,
          "COUNT_BY_COIN_TYPES rejects missing output counter table");
}

static void test_boost_attribute(void)
{
    DM2_V1_SkprojectChampionAttribute attributes[4];
    DM2_V1_SkprojectBoostAttributeReceipt receipt;

    memset(attributes, 0, sizeof(attributes));
    attributes[1].current = 100u;
    attributes[1].maximum = 150u;
    CHECK(dm2_v1_skproject_boost_attribute(
              attributes, 1u, 100, &receipt) == 1 &&
              receipt.valid && receipt.previous_current == 100u &&
              receipt.maximum == 150u && receipt.source_si == 50 &&
              receipt.reduced_delta == 57 &&
              attributes[1].current == 157u,
          "BOOST_ATTRIBUTE reduces large positive boosts by source 20-point buckets");

    attributes[1].current = 100u;
    attributes[1].maximum = 150u;
    CHECK(dm2_v1_skproject_boost_attribute(
              attributes, 1u, -80, &receipt) == 1 &&
              receipt.valid && receipt.source_si == -130 &&
              receipt.reduced_delta == -15 &&
              attributes[1].current == 85u,
          "BOOST_ATTRIBUTE reduces large negative boosts symmetrically");

    attributes[2].current = 218u;
    attributes[2].maximum = 20u;
    CHECK(dm2_v1_skproject_boost_attribute(
              attributes, 2u, 20, &receipt) == 1 &&
              attributes[2].current == 220u,
          "BOOST_ATTRIBUTE clamps boosted attribute to 220");

    attributes[3].current = 12u;
    attributes[3].maximum = 80u;
    CHECK(dm2_v1_skproject_boost_attribute(
              attributes, 3u, -20, &receipt) == 1 &&
              attributes[3].current == 10u,
          "BOOST_ATTRIBUTE clamps reduced attribute to 10");

    CHECK(dm2_v1_skproject_boost_attribute(
              0, 0u, 1, &receipt) == 0 &&
              receipt.blocked_missing_attribute,
          "BOOST_ATTRIBUTE rejects missing caller champion attributes");
}

static void test_adjust_ui_event(void)
{
    DM2_V1_SkprojectUiChampionState champions[4];
    int16_t positions[4] = { 2, -1, 0, 1 };
    DM2_V1_SkprojectUiEvent event;
    DM2_V1_SkprojectAdjustUiEventReceipt receipt;

    memset(champions, 0, sizeof(champions));
    for (uint16_t i = 0; i < 4u; ++i) {
        champions[i].present = 1u;
        champions[i].hand_activable[0] = 1u;
        champions[i].hand_activable[1] = 1u;
    }

    memset(&event, 0, sizeof(event));
    event.event = 116u;
    CHECK(dm2_v1_skproject_adjust_ui_event(
              &event, 0u, positions, champions, 4u, &receipt) == 1 &&
              event.event == 120u && receipt.mapped_player == 2 &&
              receipt.mapped_hand == 0u,
          "ADJUST_UI_EVENT maps champion hand event through player direction");

    event.event = 117u;
    champions[2].hand_cooldown[1] = 3u;
    CHECK(dm2_v1_skproject_adjust_ui_event(
              &event, 0u, positions, champions, 4u, &receipt) == 0 &&
              event.event == 0u && receipt.blocked_hand_cooldown,
          "ADJUST_UI_EVENT cancels hand event while hand cooldown is active");
    champions[2].hand_cooldown[1] = 0u;

    event.event = 95u;
    event.x = 18;
    event.y = 18;
    event.rect.x = 10;
    event.rect.y = 10;
    event.rect.w = 20;
    event.rect.h = 20;
    CHECK(dm2_v1_skproject_adjust_ui_event(
              &event, 0u, positions, champions, 4u, &receipt) == 1 &&
              event.event == 16u && receipt.selected_spell_triangle &&
              receipt.diagonal_w3 <= receipt.diagonal_w2,
          "ADJUST_UI_EVENT converts spell/leader triangle click to spell event");

    event.event = 95u;
    event.x = 11;
    event.y = 28;
    CHECK(dm2_v1_skproject_adjust_ui_event(
              &event, 0u, positions, champions, 4u, &receipt) == 1 &&
              event.event == 95u && !receipt.selected_spell_triangle &&
              receipt.diagonal_w3 > receipt.diagonal_w2,
          "ADJUST_UI_EVENT keeps leader event when click falls outside spell triangle");

    event.event = 95u;
    champions[2].hand_cooldown[2] = 1u;
    CHECK(dm2_v1_skproject_adjust_ui_event(
              &event, 0u, positions, champions, 4u, &receipt) == 0 &&
              event.event == 0u && receipt.blocked_leader_hand_cooldown,
          "ADJUST_UI_EVENT cancels leader event when leader hand cooldown is active");
    champions[2].hand_cooldown[2] = 0u;

    event.event = 50u;
    CHECK(dm2_v1_skproject_adjust_ui_event(
              &event, 0u, positions, champions, 4u, &receipt) == 1 &&
              event.event == 50u && receipt.untouched_non_adjustable_event,
          "ADJUST_UI_EVENT leaves unrelated UI events untouched");
}

static void test_gfx_str_helpers(void)
{
    uint8_t font_plane[DM2_V1_SKPROJECT_FONT_PLANE_BYTES];
    DM2_V1_SkprojectFontReceipt font_receipt;
    DM2_V1_SkprojectTextMetricsReceipt metrics;
    DM2_V1_SkprojectTextDrawReceipt draw;
    DM2_V1_SkprojectFormatTextReceipt fmt;
    DM2_V1_SkprojectHintLineReceipt hint;
    char text[64];
    char line[32];
    uint8_t encrypted[6];

    memset(font_plane, 0, sizeof(font_plane));
    for (uint8_t row = 0; row < 6u; ++row)
        font_plane[(uint16_t)row * 128u + 'A'] = 0x18u;
    CHECK(dm2_v1_skproject_query_font(
              font_plane, 'A', 0x0eu, 0x01u, &font_receipt) == 1 &&
              font_receipt.valid && font_receipt.written_pixels == 18u &&
              font_receipt.pixels[0] == 0xeeu &&
              font_receipt.pixels[1] == 0x11u,
          "c_gfx_str QUERY_FONT expands six source rows into 18 packed pixels");
    CHECK(dm2_v1_skproject_query_font(
              0, 'A', 0x0eu, 0x01u, &font_receipt) == 0 &&
              font_receipt.blocked_missing_font_plane,
          "c_gfx_str QUERY_FONT rejects missing font plane");

    CHECK(dm2_v1_skproject_query_str_metrics(
              "DM2", &metrics) == 1 &&
              metrics.valid && metrics.text_len == 3u &&
              metrics.width == 17 && metrics.height == 5,
          "c_gfx_str QUERY_STR_METRICS uses -gfxstrw2 plus glyph stride");
    CHECK(dm2_v1_skproject_query_str_metrics(
              "", &metrics) == 0 && !metrics.valid,
          "c_gfx_str QUERY_STR_METRICS rejects zero-width strings");

    CHECK(dm2_v1_skproject_plan_draw_string(
              DM2_V1_SKPROJECT_TEXT_ROUTE_STRING, 320, 10, 20,
              3u, 0x4004u, "AB", &draw) == 1 &&
              draw.valid && draw.dest_is_screen &&
              draw.uses_alpha_mask && draw.draw_y == 15 &&
              draw.first_char_x == 10 && draw.last_char_x == 16 &&
              draw.char_count == 2u,
          "c_gfx_str DRAW_STRING plan preserves baseline and alpha-mask flag");
    CHECK(dm2_v1_skproject_plan_draw_string(
              DM2_V1_SKPROJECT_TEXT_ROUTE_STRONG, 112, 30, 40,
              2u, 4u, "AB", &draw) == 1 &&
              draw.strong_shadow_passes == 2 &&
              draw.fill_background &&
              draw.fill_x == 29 && draw.fill_y == 35 &&
              draw.fill_w == 13 && draw.fill_h == 7,
          "c_gfx_str DRAW_STRONG_TEXT plan includes source background fill rect");
    CHECK(dm2_v1_skproject_plan_draw_string(
              DM2_V1_SKPROJECT_TEXT_ROUTE_BUTTON, 80, 4, 12,
              1u, 2u, "OK", &draw) == 1 &&
              draw.adjusts_button_rect,
          "c_gfx_str DRAW_BUTTON_STR plan marks button rect adjustment");
    CHECK(dm2_v1_skproject_plan_draw_string(
              DM2_V1_SKPROJECT_TEXT_ROUTE_NAME, 80, 4, 12,
              1u, 2u, "NAME", &draw) == 1 &&
              draw.adjusts_button_rect && draw.strong_shadow_passes == 2,
          "c_gfx_str DRAW_NAME_STR combines button placement with strong text");
    CHECK(dm2_v1_skproject_plan_draw_string(
              DM2_V1_SKPROJECT_TEXT_ROUTE_BACKBUFF, 128, 64, 50,
              1u, 2u, "ABCD", &draw) == 1 &&
              draw.centered_by_metrics && draw.draw_x == 62,
          "c_gfx_str DRAW_TEXT_TO_BACKBUFF plan centers by measured text height");

    CHECK(dm2_v1_skproject_format_skstr_literal(
              "SAVE GAME", text, sizeof(text), &fmt) == 1 &&
              fmt.valid && strcmp(text, "SAVE GAME") == 0 &&
              fmt.consumed_bytes == 9u && fmt.written_bytes == 9u,
          "c_gfx_str FORMAT_SKSTR literal path copies source text");
    CHECK(dm2_v1_skproject_format_skstr_literal(
              "VALUE .Z001", text, sizeof(text), &fmt) == 0 &&
              fmt.blocked_unimplemented_substitution &&
              strcmp(text, "VALUE ") == 0,
          "c_gfx_str FORMAT_SKSTR substitution path fails closed");

    for (uint16_t i = 0; i < 5u; ++i)
        encrypted[i] = (uint8_t)(~((uint8_t)"HELLO"[i] + (uint8_t)i));
    encrypted[5] = 0u;
    CHECK(dm2_v1_skproject_decode_gdat_text_literal(
              encrypted, 5u, 1, text, sizeof(text), &fmt) == 1 &&
              strcmp(text, "HELLO") == 0,
          "c_gfx_str QUERY_GDAT_TEXT decrypts bit-0x08 literal bytes");

    CHECK(dm2_v1_skproject_split_hint_line(
              "ALPHA BETA GAMMA", 0u, 60, line, sizeof(line), &hint) == 1 &&
              hint.valid && hint.split_at_space &&
              strcmp(line, "ALPHA BETA") == 0 && hint.next_offset == 11u,
          "c_gfx_str DM2_gfxstr_3929_04e2 wraps at the last fitting space");
    CHECK(dm2_v1_skproject_split_hint_line(
              "ONE\nTWO", 0u, 60, line, sizeof(line), &hint) == 1 &&
              hint.stopped_at_newline && strcmp(line, "ONE") == 0 &&
              hint.next_offset == 4u,
          "c_gfx_str DISPLAY_HINT_TEXT line splitter consumes explicit newline");
}

int main(void)
{
    test_between_value();
    test_temp_rect_ring();
    test_random_helpers();
    test_util_helpers();
    test_palette_helpers();
    test_move_admission_helpers();
    test_map_helpers();
    test_calc_vector_w_dir();
    test_cache_hash_helpers();
    test_picture_mement_helpers();
    test_item_charge_helpers();
    test_item_value_weight_helpers();
    test_player_weight_helper();
    test_count_by_coin_types();
    test_boost_attribute();
    test_adjust_ui_event();
    test_gfx_str_helpers();
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "ALLOC_TEMP_RECT") != 0,
          "source evidence names ALLOC_TEMP_RECT");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_BETWEEN_VALUE") != 0,
          "source evidence names DM2_BETWEEN_VALUE");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_RAND16") != 0,
          "source evidence names c_random helpers");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_CALC_SQUARE_DISTANCE") != 0,
          "source evidence names utility helpers");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "CALC_VECTOR_W_DIR") != 0,
          "source evidence names vector direction helper");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "ADD_CACHE_HASH") != 0,
          "source evidence names cache hash helpers");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "ALLOC_IMAGE_MEMENT") != 0,
          "source evidence names picture mement helpers");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "ADD_ITEM_CHARGE") != 0,
          "source evidence names item charge helper");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "GET_MAX_CHARGE") != 0,
          "source evidence names max charge helper");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "QUERY_ITEM_VALUE") != 0,
          "source evidence names item value helper");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "CALC_PLAYER_WEIGHT") != 0,
          "source evidence names player weight helper");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "COUNT_BY_COIN_TYPES") != 0,
          "source evidence names coin count helper");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "BOOST_ATTRIBUTE") != 0,
          "source evidence names attribute boost helper");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "ADJUST_UI_EVENT") != 0,
          "source evidence names UI event adjustment helper");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_QUERY_FONT") != 0,
          "source evidence names c_gfx_str helpers");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_12b4_0881") != 0,
          "source evidence names c_move admission helpers");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_ATTACK_WALL") != 0,
          "source evidence names c_move wall attack helper");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_ATTACK_DOOR") != 0,
          "source evidence names c_move door attack helper");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_SET_DESTINATION_OF_MINION_MAP") != 0,
          "source evidence names c_map helpers");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM_LOCATE_OTHER_LEVEL") != 0,
          "source evidence names other-level locator");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_map_3BF83") != 0,
          "source evidence names cross-map record mover");

    if (failed) {
        printf("%d failure(s)\n", failed);
        return 1;
    }
    puts("all DM2 skproject core helper checks passed");
    return 0;
}
