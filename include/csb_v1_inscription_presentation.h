#ifndef CSB_V1_INSCRIPTION_PRESENTATION_H
#define CSB_V1_INSCRIPTION_PRESENTATION_H

#include "csb_v1_runtime_pc34_compat.h"

/* ReDMCSB DEFS.H MEDIA020/MEDIA720 and DUNVIEW.C F0107.  This describes
 * source-owned material only; callers must still obtain the pixels from the
 * selected CSB archive and must not substitute a DM1 cache entry. */
typedef struct CSB_V1_InscriptionPresentationPlan {
    int valid;
    unsigned int font_graphic;
    int glyph_width;
    int glyph_height;
    int transparent_colour;
    int fixed_geometry;
    int line_y[4];
} CSB_V1_InscriptionPresentationPlan;

typedef struct CSB_V1_VisibleInscriptionReceipt {
    int valid;
    int front_wall_has_inscription;
    int map_x;
    int map_y;
    uint16_t selected_text_thing;
    char source_text[256];
} CSB_V1_VisibleInscriptionReceipt;

typedef struct CSB_V1_WallAspectInscriptionReceipt {
    int valid;
    int view_wall_index;
    int wall_cell;
    int map_x;
    int map_y;
    uint16_t selected_text_thing;
    int line_count;
    int unreadable_shift_x_is_raster_width;
    int unreadable_shift_y;
} CSB_V1_WallAspectInscriptionReceipt;

typedef struct CSB_V1_F31InventorySlotRectangle {
    int x;
    int y;
    int width;
    int height;
} CSB_V1_F31InventorySlotRectangle;

typedef struct CSB_V1_F31JPrintableSubstringReceipt {
    int valid;
    size_t source_begin;
    size_t source_end;
    size_t output_size;
    int pixel_width;
    int japanese_character_count;
    int ank_character_count;
    int stopped_at_explicit_break;
} CSB_V1_F31JPrintableSubstringReceipt;

int csb_v1_inscription_presentation_plan(
    CSB_V1_VariantId variant,
    CSB_V1_InscriptionPresentationPlan *out);

int csb_v1_visible_front_inscription_receipt(
    const CSB_V1_RuntimeProfile *runtime,
    CSB_V1_VisibleInscriptionReceipt *out);

int csb_v1_inscription_media720_f0635_lines(
    const uint8_t *layout, size_t size, int big_endian, int out_bottom_y[4]);

/* F31J DUNGEON.C F0168's second pass.  The first F0168 pass represents each
 * source byte as two letters A..P (apart from literal separators/control
 * bytes); this pass restores the original Shift-JIS byte stream in place.
 * The terminal 0x81 inscription marker is not a character and is omitted. */
int csb_v1_f31j_unpack_f0168_text(
    const uint8_t *first_pass, size_t first_pass_size,
    uint8_t *output, size_t output_capacity, size_t *out_size);

/* TEXT.C F0646 for MEDIA686_F31J.  It copies the largest complete substring
 * whose next glyph would not reach max_width.  Shift-JIS pairs are 16 pixels,
 * ANK bytes are 8, 0x1b/0x7c are zero-width controls, and '|' is an explicit
 * break opportunity.  Invalid/truncated Shift-JIS fails closed. */
int csb_v1_f31j_f0646_printable_substring(
    const uint8_t *source, size_t source_size, size_t *in_out_index,
    int max_width, uint8_t *output, size_t output_capacity,
    CSB_V1_F31JPrintableSubstringReceipt *out_receipt);

/* ReDMCSB COORD.C F0639/F0635 over the selected F31 GRAPHICS.DAT item 696.
 * C507..C536 are children of the C105 16x16 pointer-box record.  The call
 * rejects any other record graph instead of falling back to Atari/PC data. */
int csb_v1_media720_f0635_f31_inventory_rectangles(
    const uint8_t *layout, size_t size, int big_endian,
    CSB_V1_F31InventorySlotRectangle out_rectangles[30]);

/* C537..C544 chest children of C106, relative to centered C101/C100. */
int csb_v1_media720_f0635_f31_chest_rectangles(
    const uint8_t *layout, size_t size, int big_endian,
    CSB_V1_F31InventorySlotRectangle out_rectangles[8]);

int csb_v1_wall_aspect_inscription_receipt(
    const CSB_V1_RuntimeProfile *runtime, int map_x, int map_y,
    int view_wall_index, CSB_V1_WallAspectInscriptionReceipt *out);

/* DM1_ViewportWallAspectCallback-compatible adapter.  Global ornament zero
 * is represented as ordinal one, exactly as F0173 exposes M615 to F0107. */
int csb_v1_wall_aspect_inscription_ordinal_callback(
    void *user_data, int map_x, int map_y, int view_wall_index,
    int *out_inscription_line_count);

int csb_v1_unreadable_inscription_shift(
    int view_wall_index, int line_count, int raster_width,
    int *out_shift_x, int *out_shift_y);

#endif
