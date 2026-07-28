#ifndef FIRESTAFF_CSB_V1_ATARI_ST_ANIMATION_ASSETS_H
#define FIRESTAFF_CSB_V1_ATARI_ST_ANIMATION_ASSETS_H

#include <stddef.h>
#include <stdint.h>

enum {
    CSB_V1_ATARI_ST_ANIMATION_WIDTH = 320,
    CSB_V1_ATARI_ST_ANIMATION_HEIGHT = 200,
    CSB_V1_ATARI_ST_ANIMATION_RGBA_BYTES =
        CSB_V1_ATARI_ST_ANIMATION_WIDTH * CSB_V1_ATARI_ST_ANIMATION_HEIGHT * 4
};

typedef struct {
    int valid;
    uint16_t data_item_count;
    uint16_t script_instruction_count;
    uint16_t palette_load_count;
    uint16_t image_load_count;
    uint16_t sound_load_count;
    uint16_t invalid_load_count;
} CSB_V1_AtariStAnimationAssetReceipt;

/* Decode one documented Atari ST P4B1 palette: 16 big-endian ST RGB words.
 * Each output entry is RGB888. */
int csb_v1_atari_st_animation_decode_p4b1_palette(
    const uint8_t *bytes, size_t byte_count, uint8_t out_rgb[16][3]);

/* Verify every Load item in an original Atari ST ANIMATE.SCR against the
 * corresponding 87-item ANIMATE.DAT family. No substitute assets are used. */
int csb_v1_atari_st_animation_validate_assets(
    const char *animate_dat_path, const uint8_t *script, size_t script_size,
    CSB_V1_AtariStAnimationAssetReceipt *out_receipt);

/* Render a full-screen original IMG1 item with an original P4B1 palette into
 * a 320x200 RGBA frame. image_item must be a documented full-screen image
 * and palette_item must be a documented 32-byte palette item. */
int csb_v1_atari_st_animation_render_rgba(
    const char *animate_dat_path, uint16_t image_item, uint16_t palette_item,
    uint8_t *out_rgba, size_t out_rgba_size);

#endif
