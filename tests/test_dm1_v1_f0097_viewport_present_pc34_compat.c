#include "dm1_v1_viewport_3d_pc34_compat.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int tests;
static int passes;

#define CHECK(condition, message) do { \
    ++tests; \
    if (condition) ++passes; else printf("FAIL: %s\n", message); \
} while (0)

int main(void)
{
    uint8_t viewport[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    uint8_t screen[DM1_VIEWPORT_SCREEN_WIDTH * DM1_VIEWPORT_SCREEN_HEIGHT];
    DM1_ViewportPresentReceiptPc34 receipt;
    uint8_t rgb6[256][3];
    int cached_palette = -1;
    int x;
    int y;

    for (y = 0; y < DM1_VIEWPORT_HEIGHT; ++y) {
        for (x = 0; x < DM1_VIEWPORT_WIDTH; ++x) {
            viewport[y * DM1_VIEWPORT_WIDTH + x] =
                (uint8_t)((x + y * 3) & 0xff);
        }
    }
    /* Host palette-level bits are not DOS VGA pixel bits. F8161 adds the
     * viewport's separate 0x10 DAC bank to the low source color nibble. */
    viewport[1] = 0xa7;
    memset(screen, 0xa5, sizeof(screen));

    CHECK(dm1_viewport_3d_present_pc34(
              viewport, DM1_VIEWPORT_WIDTH, screen,
              DM1_VIEWPORT_SCREEN_WIDTH, DM1_VIEWPORT_SCREEN_HEIGHT,
              DM1_VIEWPORT_SCREEN_WIDTH, 1, 3, 4, 255, 100, 20,
              &cached_palette, &receipt),
          "F0097 accepts the complete PC3.4 G0296 and C007 surfaces");
    CHECK(receipt.valid && receipt.destination_x == 0 &&
              receipt.destination_y == 33 && receipt.width == 224 &&
              receipt.height == 136,
          "F0097 reports the exact C007 aperture placement");
    CHECK(receipt.mouse_screen_update_enabled &&
              receipt.mouse_screen_update_disabled && !receipt.mouse_hidden,
          "F0097 keeps the mouse screen-update bracket inside C007");
    CHECK(receipt.palette_changed &&
              receipt.palette_action == DM1_VIEWPORT_PRESENT_PALETTE_DUNGEON_PC34 &&
              cached_palette == 3,
          "F0097 selects the changed dungeon palette exactly once");
    CHECK(screen[33 * DM1_VIEWPORT_SCREEN_WIDTH] == 0x10 &&
              screen[33 * DM1_VIEWPORT_SCREEN_WIDTH + 1] == 0x17 &&
              screen[(33 + 135) * DM1_VIEWPORT_SCREEN_WIDTH + 223] ==
                  (uint8_t)(0x10 |
                      (viewport[135 * DM1_VIEWPORT_WIDTH + 223] & 0x0f)) &&
              screen[32 * DM1_VIEWPORT_SCREEN_WIDTH] == 0xa5 &&
              screen[(33 + 136) * DM1_VIEWPORT_SCREEN_WIDTH] == 0xa5 &&
              screen[33 * DM1_VIEWPORT_SCREEN_WIDTH + 224] == 0xa5,
          "F0097 copies only G0296 into the source C007 rectangle");

    CHECK(dm1_viewport_3d_present_pc34(
              viewport, DM1_VIEWPORT_WIDTH, screen,
              DM1_VIEWPORT_SCREEN_WIDTH, DM1_VIEWPORT_SCREEN_HEIGHT,
              DM1_VIEWPORT_SCREEN_WIDTH, 1, 3, 4, 255, 250, 169,
              &cached_palette, &receipt),
          "F0097 accepts a repeated dungeon palette frame");
    CHECK(!receipt.palette_changed && receipt.mouse_hidden,
          "F0097 skips unchanged G2123 palette and hides an out-of-zone mouse");

    CHECK(dm1_viewport_3d_present_pc34(
              viewport, DM1_VIEWPORT_WIDTH, screen,
              DM1_VIEWPORT_SCREEN_WIDTH, DM1_VIEWPORT_SCREEN_HEIGHT,
              DM1_VIEWPORT_SCREEN_WIDTH, 0, 3, 255, 255, -1, -1,
              &cached_palette, &receipt),
          "F0097 accepts entrance non-dungeon palette request");
    CHECK(receipt.palette_changed && cached_palette == -1 &&
              receipt.palette_action == DM1_VIEWPORT_PRESENT_PALETTE_LIGHT0_PC34,
          "F0097 selects C00_LIGHT0 at the entrance and resets G2123");

    memset(screen, 0x4e, sizeof(screen));
    CHECK(!dm1_viewport_3d_present_pc34(
               viewport, DM1_VIEWPORT_WIDTH - 1, screen,
               DM1_VIEWPORT_SCREEN_WIDTH, DM1_VIEWPORT_SCREEN_HEIGHT,
               DM1_VIEWPORT_SCREEN_WIDTH, 1, 0, 0, 255, -1, -1,
               &cached_palette, &receipt),
          "F0097 rejects an undersized real G0296 source");
    CHECK(screen[33 * DM1_VIEWPORT_SCREEN_WIDTH] == 0x4e && !receipt.valid,
          "F0097 failure does not synthesize or partially present a viewport");

    CHECK(!dm1_viewport_3d_build_pc34_indexed_palette(-1, rgb6) &&
              !dm1_viewport_3d_build_pc34_indexed_palette(6, rgb6),
          "I34E indexed palette rejects invalid dungeon light rows");
    CHECK(dm1_viewport_3d_build_pc34_indexed_palette(3, rgb6),
          "I34E indexed palette accepts the active dungeon light row");
    CHECK(rgb6[4][0] == (G9010_auc_VgaPaletteAll_Compat[0][4][0] >> 2) &&
              rgb6[4][1] == (G9010_auc_VgaPaletteAll_Compat[0][4][1] >> 2) &&
              rgb6[4][2] == (G9010_auc_VgaPaletteAll_Compat[0][4][2] >> 2),
          "I34E retains the full-bright top and HUD color bank at 0..15");
    CHECK(rgb6[20][0] == (G9010_auc_VgaPaletteAll_Compat[3][4][0] >> 2) &&
              rgb6[20][1] == (G9010_auc_VgaPaletteAll_Compat[3][4][1] >> 2) &&
              rgb6[20][2] == (G9010_auc_VgaPaletteAll_Compat[3][4][2] >> 2),
          "I34E maps viewport bank 16..31 through its selected light palette");
    CHECK(rgb6[0xff][0] == (G9010_auc_VgaPaletteAll_Compat[5][15][0] >> 2) &&
              rgb6[0xff][1] == (G9010_auc_VgaPaletteAll_Compat[5][15][1] >> 2) &&
              rgb6[0xff][2] == (G9010_auc_VgaPaletteAll_Compat[5][15][2] >> 2),
          "unused host palette metadata remains bounded to the last VGA row");

    printf("%d/%d tests passed\n", passes, tests);
    return passes == tests ? 0 : 1;
}
