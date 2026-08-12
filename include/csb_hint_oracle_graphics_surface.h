#ifndef FIRESTAFF_CSB_HINT_ORACLE_GRAPHICS_SURFACE_H
#define FIRESTAFF_CSB_HINT_ORACLE_GRAPHICS_SURFACE_H
#include "csb_hint_oracle_dat_real_scan.h"
#include "csb_hint_oracle_htc.h"
#define CSB_HINT_ORACLE_GRAPHICS_SURFACE_WIDTH 320u
#define CSB_HINT_ORACLE_GRAPHICS_SURFACE_HEIGHT 200u
#define CSB_HINT_ORACLE_FONT_SURFACE_WIDTH 256u
#define CSB_HINT_ORACLE_FONT_SURFACE_HEIGHT 27u
#define CSB_HINT_ORACLE_GRAPHIC_CONTROL_WORDS 50u
typedef struct {
    CSB_HintOracleDAT_RealCache source;
    uint8_t *pixels;
    uint8_t *font_pixels;
    /* HINTTEXT.C C26_SET_FONT_COLOR remaps this bitmap through F0129.
     * Preserve the decoded font and carry the resulting source-index map
     * separately for each text blit. */
    uint8_t font_palette_indices[16];
    int font_palette_indices_valid;
    uint8_t rgb4[48];
    uint16_t controls[CSB_HINT_ORACLE_GRAPHIC_CONTROL_WORDS];
    uint16_t width,height;
    uint16_t font_width,font_height;
} CSB_HintOracleGraphicsSurface;
void csb_hint_oracle_graphics_surface_init(CSB_HintOracleGraphicsSurface *surface);
void csb_hint_oracle_graphics_surface_free(CSB_HintOracleGraphicsSurface *surface);
int csb_hint_oracle_graphics_surface_load(CSB_HintOracleGraphicsSurface *surface,const char *data_dir,int max_depth,const char *expected_md5);
/* ReDMCSB HINTTEXT.C F1882_PrintTextString() for the ST font raster:
 * printable ASCII indexes an 8x9 glyph at (character-' ') and advances 9px.
 * Colour 12 is the source F0132 transparent colour. */
int csb_hint_oracle_graphics_surface_blit_st_text(
    const CSB_HintOracleGraphicsSurface *surface, uint8_t *frame,
    size_t frame_size, int x, int y, const char *text);
int csb_hint_oracle_graphics_surface_blit_st_centered_box(
    const CSB_HintOracleGraphicsSurface *surface, uint8_t *frame,
    size_t frame_size, int left, int right, int top, int bottom,
    const char *text);
int csb_hint_oracle_graphics_surface_render_st_hint_page(
    const CSB_HintOracleGraphicsSurface *surface,
    const CSB_HintOracleHTC *htc, size_t hint_index, size_t page_number,
    uint8_t *frame, size_t frame_size);
#endif
