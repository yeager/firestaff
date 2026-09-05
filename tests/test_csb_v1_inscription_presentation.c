#include "csb_v1_inscription_presentation.h"
#include "dm1_v1_viewport_wall_ornament_ordinal_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
static void check(int condition, const char *message)
{
    if (!condition) { fprintf(stderr, "FAIL: %s\n", message); ++failures; }
}

static void put16(unsigned char *p, unsigned short v)
{ p[0] = (unsigned char)v; p[1] = (unsigned char)(v >> 8); }

static void make_wall_text_fixture(CSB_V1_RuntimeProfile *runtime,
                                   CSB_V1_DungeonData *dungeon,
                                   unsigned char raw[128], int cell)
{
    unsigned short text_thing = (unsigned short)((cell << 14) | (2 << 10));
    memset(raw, 0, 128);
    memset(dungeon, 0, sizeof(*dungeon));
    csb_v1_runtime_init(runtime, NULL);
    dungeon->level_count = 1;
    dungeon->level_widths[0] = 3;
    dungeon->level_heights[0] = 3;
    dungeon->square_bytes = 1;
    dungeon->raw_data = raw;
    dungeon->raw_size = 128;
    dungeon->square_first_thing_base = 66;
    dungeon->square_first_thing_count = 1;
    dungeon->thing_data_bases[2] = 68;
    dungeon->thing_type_counts[2] = 1;
    dungeon->text_data_base = 96;
    dungeon->text_word_count = 3;
    raw[1] = 0x10; /* (0,1): wall plus source Thing list. */
    put16(raw + 60, 0);
    put16(raw + 66, text_thing);
    put16(raw + 68, 0xfffe);
    put16(raw + 70, 1); /* Atari visible bit, text offset zero. */
    put16(raw + 96, (unsigned short)((14 << 10) | (17 << 5) | 0));
    put16(raw + 98, (unsigned short)((2 << 10) | (11 << 5) | 4));
    put16(raw + 100, (unsigned short)((31 << 10) | (31 << 5) | 31));
    runtime->dungeon_handle = dungeon;
    runtime->current_level = 0;
    runtime->party_dir = 0;
    runtime->variant_id = CSB_V1_VARIANT_ST21_EN;
}

int main(void)
{
    CSB_V1_InscriptionPresentationPlan plan;
    unsigned char layout[52] = { 0x0d, 0xfc, 1, 0, 0xe8, 3, 0xeb, 3 };
    int y[4] = { 0, 0, 0, 0 };
    int i;
    int shift_x, shift_y;
    CSB_V1_RuntimeProfile runtime;
    CSB_V1_DungeonData dungeon;
    CSB_V1_WallAspectInscriptionReceipt receipt;
    unsigned char raw[128];
    unsigned char inventory_layout[260] = { 0 };
    CSB_V1_F31InventorySlotRectangle rectangles[30];
    {
        const unsigned char packed[] = {
            'A', 0x1b, 'I', 'C', 'E', 'C', 'I', 'C', /* 1b,82,42,82 */
            'J', 'C', 0x81, 0 /* 92 */
        };
        unsigned char sjis[32], line[32];
        size_t sjis_size = 0u, at = 0u;
        CSB_V1_F31JPrintableSubstringReceipt f0646;
        check(csb_v1_f31j_unpack_f0168_text(
                  packed, sizeof(packed), sjis, sizeof(sjis), &sjis_size) &&
              sjis_size == 5u && sjis[0] == 0x1bu && sjis[1] == 0x82u &&
              sjis[2] == 0x42u && sjis[3] == 0x82u && sjis[4] == 0x92u,
              "F31J F0168 second pass restores source Shift-JIS bytes");
        check(csb_v1_f31j_f0646_printable_substring(
                  sjis, sjis_size + 1u, &at, 33, line, sizeof(line), &f0646) &&
              f0646.valid && f0646.pixel_width == 32 &&
              f0646.japanese_character_count == 2 &&
              f0646.ank_character_count == 0 && at == sjis_size,
              "F31J F0646 measures complete SJIS pairs and zero-width controls");
        at = 0u;
        check(!csb_v1_f31j_f0646_printable_substring(
                  (const unsigned char *)"\x82", 1u, &at, 40,
                  line, sizeof(line), &f0646),
              "F31J F0646 rejects truncated Shift-JIS instead of inventing a glyph");
    }
    for (i = 0; i < 4; ++i) {
        unsigned char *record = layout + 8 + i * 8;
        record[0] = 7; record[2] = 4;
        record[4] = 112;
        record[6] = (unsigned char)(48 + i * 11);
    }
    layout[8 + 2 * 8 + 6] = 73;
    layout[8 + 3 * 8 + 6] = 85;
    check(csb_v1_inscription_media720_f0635_lines(layout, sizeof(layout), 0, y) &&
          y[0] == 48 && y[1] == 59 && y[2] == 73 && y[3] == 85,
          "MEDIA720 F0639 ranges publish authentic C1000..C1003 anchors");
    put16(inventory_layout, 0xfc0d);
    put16(inventory_layout + 2, 2);
    put16(inventory_layout + 4, 105); put16(inventory_layout + 6, 105);
    put16(inventory_layout + 8, 507); put16(inventory_layout + 10, 536);
    put16(inventory_layout + 12, 9); put16(inventory_layout + 14, 4);
    put16(inventory_layout + 16, 16); put16(inventory_layout + 18, 16);
    for (i = 0; i < 30; ++i) {
        unsigned char *record = inventory_layout + 20 + i * 8;
        put16(record, 1); put16(record + 2, 105);
        put16(record + 4, (unsigned short)(6 + i));
        put16(record + 6, (unsigned short)(16 + i));
    }
    check(csb_v1_media720_f0635_f31_inventory_rectangles(
              inventory_layout, sizeof(inventory_layout), 0, rectangles) &&
          rectangles[0].x == 6 && rectangles[0].y == 16 &&
          rectangles[29].x == 35 && rectangles[29].y == 45 &&
          rectangles[0].width == 16 && rectangles[29].height == 16,
          "F0639/F0635 parser resolves C507..C536 through C105");
    inventory_layout[22] = 104;
    check(!csb_v1_media720_f0635_f31_inventory_rectangles(
              inventory_layout, sizeof(inventory_layout), 0, rectangles),
          "F31 pointer parser rejects a non-C105 child instead of borrowing geometry");
    check(csb_v1_inscription_presentation_plan(CSB_V1_VARIANT_ST21_EN, &plan) &&
          plan.font_graphic == 120u && plan.fixed_geometry &&
          plan.line_y[0] == 48 && plan.line_y[3] == 86,
          "Atari MEDIA020 owns authentic M648 and G0203 geometry");
    check(csb_v1_inscription_presentation_plan(CSB_V1_VARIANT_AMIGA35_EN, &plan) &&
          plan.font_graphic == 258u && !plan.fixed_geometry,
          "Amiga MEDIA720 owns authentic M648 and F0635 geometry");
    check(csb_v1_inscription_presentation_plan(CSB_V1_VARIANT_FMTOWNS_EN, &plan) &&
          plan.font_graphic == 258u && !plan.fixed_geometry,
          "FM Towns English owns authentic MEDIA720 M648");
    check(!csb_v1_inscription_presentation_plan(CSB_V1_VARIANT_FMTOWNS_JA, &plan) &&
          !plan.valid,
          "FM Towns Japanese fails closed instead of borrowing M648");
    check(csb_v1_unreadable_inscription_shift(0, 1, 37, &shift_x, &shift_y) &&
          shift_x == 37 && shift_y == 5,
          "D3 side one-line plaque uses raster-width X and G0204 row zero");
    check(csb_v1_unreadable_inscription_shift(4, 3, 41, &shift_x, &shift_y) &&
          shift_x == 41 && shift_y == 20,
          "D3 front three-line plaque uses G0190 increment one");
    check(csb_v1_unreadable_inscription_shift(7, 2, 53, &shift_x, &shift_y) &&
          shift_x == 53 && shift_y == 12,
          "D2 side two-line plaque uses G0190 increment two");
    check(csb_v1_unreadable_inscription_shift(9, 3, 59, &shift_x, &shift_y) &&
          shift_x == 59 && shift_y == 27,
          "D2 front three-line plaque uses G0190 increment three");
    check(csb_v1_unreadable_inscription_shift(12, 2, 61, &shift_x, &shift_y) &&
          shift_x == 61 && shift_y == 22,
          "D1 side/front row uses G0190 increment four");
    check(!csb_v1_unreadable_inscription_shift(7, 4, 53, &shift_x, &shift_y) &&
          shift_x == 0 && shift_y == 0,
          "four-line plaque does not set MASK0x4000 shifts");
    make_wall_text_fixture(&runtime, &dungeon, raw, 1);
    check(csb_v1_wall_aspect_inscription_receipt(
              &runtime, 0, 1, DM1_V1_VIEW_WALL_D2L_RIGHT_PC34, &receipt) &&
          receipt.wall_cell == 1 && receipt.line_count == 1 &&
          receipt.unreadable_shift_x_is_raster_width &&
          receipt.unreadable_shift_y == 5,
          "F0172 right-face C02 feeds the existing D2L F0107 plaque pass");
    check(!csb_v1_wall_aspect_inscription_receipt(
              &runtime, 0, 1, DM1_V1_VIEW_WALL_D2R_LEFT_PC34, &receipt),
          "opposite side view fails closed instead of borrowing the C02 face");
    if (failures) return 1;
    puts("CSB inscription presentation source plans: PASS");
    return 0;
}
