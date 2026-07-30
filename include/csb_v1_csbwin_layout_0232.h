#ifndef FIRESTAFF_CSB_V1_CSBWIN_LAYOUT_0232_H
#define FIRESTAFF_CSB_V1_CSBWIN_LAYOUT_0232_H

#include <stddef.h>
#include <stdint.h>

/* CSBWin expands GRAPHICS.DAT item 0x232 to Data::Byte1830.  It is a
 * 0x722-byte configuration record, not a bitmap.  These offsets are relative
 * to Byte1830 and follow Data.h / CSBCode.cpp's post-expand byte swaps. */
#define CSB_V1_CSBWIN_LAYOUT_0232_DECODED_SIZE 0x722u

typedef struct {
    int16_t x1;
    int16_t x2;
    int16_t y1;
    int16_t y2;
} CSB_V1_CSBWinRect0232;

#define CSB_V1_CSBWIN_LAYOUT_0232_ICON_COUNT 46u
#define CSB_V1_CSBWIN_LAYOUT_0232_OBJECT_GRAPHIC_GROUPS 7u
#define CSB_V1_CSBWIN_LAYOUT_0232_DEFAULT_GRAPHIC_COUNT 70u
#define CSB_V1_CSBWIN_LAYOUT_0232_HUD_MATERIAL_COUNT 10u

typedef struct {
    int16_t pixel_x;
    int16_t pixel_y;
    int16_t object_type;
} CSB_V1_CSBWinIconDisplay0232;

typedef struct {
    int valid;
    CSB_V1_CSBWinRect0232 party_direction[4];
    CSB_V1_CSBWinRect0232 eye_box;
    CSB_V1_CSBWinRect0232 mouth_box;
    CSB_V1_CSBWinRect0232 poison_box;
    CSB_V1_CSBWinRect0232 food_water_box;
    CSB_V1_CSBWinRect0232 food_label_box;
    CSB_V1_CSBWinRect0232 water_label_box;
    CSB_V1_CSBWinRect0232 movement_box;
    CSB_V1_CSBWinRect0232 magic_box;
    CSB_V1_CSBWinIconDisplay0232 icon_display[
        CSB_V1_CSBWIN_LAYOUT_0232_ICON_COUNT];
    uint16_t object_graphic_first[
        CSB_V1_CSBWIN_LAYOUT_0232_OBJECT_GRAPHIC_GROUPS];
    uint16_t default_graphic_list[
        CSB_V1_CSBWIN_LAYOUT_0232_DEFAULT_GRAPHIC_COUNT];
} CSB_V1_CSBWinLayout0232;

typedef enum {
    CSB_V1_CSBWIN_HUD_MATERIAL_DIRECTION = 0,
    CSB_V1_CSBWIN_HUD_MATERIAL_FOOD_WATER,
    CSB_V1_CSBWIN_HUD_MATERIAL_FOOD_LABEL,
    CSB_V1_CSBWIN_HUD_MATERIAL_WATER_LABEL,
    CSB_V1_CSBWIN_HUD_MATERIAL_POISON,
    CSB_V1_CSBWIN_HUD_MATERIAL_MOVEMENT,
    CSB_V1_CSBWIN_HUD_MATERIAL_MAGIC
} CSB_V1_CSBWinHudMaterialKind0232;

typedef struct {
    CSB_V1_CSBWinHudMaterialKind0232 kind;
    uint16_t graphic_index;
    uint16_t source_x;
    CSB_V1_CSBWinRect0232 destination;
} CSB_V1_CSBWinHudMaterial0232;

typedef struct {
    int valid;
    size_t count;
    CSB_V1_CSBWinHudMaterial0232 entries[
        CSB_V1_CSBWIN_LAYOUT_0232_HUD_MATERIAL_COUNT];
} CSB_V1_CSBWinHudMaterialPlan0232;

/* Decode only the source-owned layout record.  The caller owns decompression
 * of graphic 0x232 and can decide independently whether its graphics package
 * is admitted for runtime use. */
int csb_v1_csbwin_layout_0232_decode(
    const uint8_t *decoded_graphic, size_t decoded_size,
    CSB_V1_CSBWinLayout0232 *out_layout);

/* Read and LZW-decompress standard CSBWin/Atari GRAPHICS.DAT item 0x232,
 * then decode its source layout. The file must expose the original 563-item
 * DMCSB1 index and item 0x232 must expand to exactly 0x722 bytes. */
int csb_v1_csbwin_layout_0232_read_graphics_dat(
    const char *graphics_dat_path, CSB_V1_CSBWinLayout0232 *out_layout);

/* Rectangles use inclusive CSBWin screen coordinates. */
int csb_v1_csbwin_layout_0232_rect_is_screen_valid(
    const CSB_V1_CSBWinRect0232 *rect);

/* Build CSBWin's source-owned HUD material composition. Direction uses a
 * 19-pixel source slice from C028; other entries use their complete graphic. */
int csb_v1_csbwin_layout_0232_build_hud_material_plan(
    const CSB_V1_CSBWinLayout0232 *layout,
    CSB_V1_CSBWinHudMaterialPlan0232 *out_plan);

#endif
