#include "dm1_v1_fmtowns_menu_render.h"
#include "dm1_v1_fmtowns_menu_regions.h"
#include "dm1_v1_fmtowns_dynamenu.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define FB_W 320
#define FB_H 200
static uint8_t g_fb[FB_W * FB_H];

typedef struct {
    unsigned int calls;
    unsigned int slots_seen[3];
    unsigned int last_lang;
    char last_label[128];
    int last_x, last_y;
    uint8_t last_fg, last_bg;
} record_t;

static int record_glyph_draw(void *user, uint8_t *fb, int w, int h, int s,
                             int dx, int dy, uint8_t fg, uint8_t bg,
                             dm1_v1_fmtowns_menu_lang_t lang,
                             unsigned int slot, const char *label) {
    record_t *r = (record_t *)user;
    (void)fb; (void)w; (void)h; (void)s;
    ++r->calls;
    if (slot < 3) r->slots_seen[slot]++;
    r->last_lang = (unsigned int)lang;
    r->last_x = dx; r->last_y = dy;
    r->last_fg = fg; r->last_bg = bg;
    snprintf(r->last_label, sizeof(r->last_label), "%s", label ? label : "");
    return 1; /* report success */
}

static void test_null_guards(void) {
    dm1_v1_fmtowns_menu_render_config_t c = {0};
    static const uint8_t rec[8] = {0,0,0,0,0,0,0,0};
    c.dynamenu_record = rec;
    assert(dm1_v1_fmtowns_menu_render_pc34(NULL, FB_W, FB_H, FB_W, &c, NULL)
           == 0);
    assert(dm1_v1_fmtowns_menu_render_pc34(g_fb, FB_W, FB_H, FB_W, NULL, NULL)
           == 0);
    c.dynamenu_record = NULL;
    assert(dm1_v1_fmtowns_menu_render_pc34(g_fb, FB_W, FB_H, FB_W, &c, NULL)
           == 0);
}

static void test_panel_fill_covers_region_10(void) {
    /* Record with valid label indices (1, 2, 14) so slots draw. */
    static const uint8_t rec[8] = {0, 1, 2, 14, 0, 0, 0, 0};
    dm1_v1_fmtowns_menu_render_config_t c = {0};
    dm1_v1_fmtowns_menu_render_result_t r;
    memset(g_fb, 0xff, sizeof(g_fb));
    c.language = DM1_V1_FMTOWNS_MENU_LANG_EN;
    c.dynamenu_record = rec;
    c.label_fg = 4;
    c.label_bg = 0;
    assert(dm1_v1_fmtowns_menu_render_pc34(g_fb, FB_W, FB_H, FB_W, &c, &r)
           == 1);
    assert(r.panel_filled == 1);
    /* Region 10 is 87 wide x 45 tall = 3915 pixels. */
    assert(r.panel_pixels_written == 87u * 45u);
    /* Anchor (319, 77) with 87-wide panel => x1=232..318, y1=77..121. */
    assert(r.panel_x1 == 232);
    assert(r.panel_y1 == 77);
    assert(r.panel_x2 == 318);
    assert(r.panel_y2 == 121);
    /* Panel colour default (byte 2 and 3 not 0xFF) is 0x0B. */
    assert(r.panel_colour == 0x0B);
    /* Every pixel inside the rectangle must be 0x0B; outside stays 0xff. */
    for (int y = 77; y <= 121; ++y) {
        for (int x = 232; x <= 318; ++x) {
            assert(g_fb[y * FB_W + x] == 0x0B);
        }
    }
    /* Untouched corners still 0xff. */
    assert(g_fb[0] == 0xff);
    assert(g_fb[FB_W * (FB_H - 1) + (FB_W - 1)] == 0xff);
}

static void test_panel_colour_selection(void) {
    /* Byte 2 = 0xFF selects ALT_B panel colour (0x4F).
     * Byte 3 = 0xFF alone would select ALT_A (0x4D); with byte 2
     * also 0xFF, ALT_B wins per DRAW_DMENU compare order. */
    static const uint8_t alt_b[8] = {0, 1, 0xFF, 0xFF, 0, 0, 0, 0};
    dm1_v1_fmtowns_menu_render_config_t c = {0};
    dm1_v1_fmtowns_menu_render_result_t r;
    memset(g_fb, 0, sizeof(g_fb));
    c.dynamenu_record = alt_b;
    assert(dm1_v1_fmtowns_menu_render_pc34(g_fb, FB_W, FB_H, FB_W, &c, &r)
           == 1);
    assert(r.panel_colour == 0x4F);
    for (int y = 77; y <= 121; ++y)
        for (int x = 232; x <= 318; ++x)
            assert(g_fb[y * FB_W + x] == 0x4F);
}

static void test_label_walk_english(void) {
    /* Slots point at (1=BLOCK, 20=FIREBALL, 43=FUSE). */
    static const uint8_t rec[8] = {0, 1, 20, 43, 0, 0, 0, 0};
    record_t r = {0};
    dm1_v1_fmtowns_menu_render_config_t c = {0};
    dm1_v1_fmtowns_menu_render_result_t rr;
    static const uint8_t font[768] = {0};
    memset(g_fb, 0, sizeof(g_fb));
    c.language = DM1_V1_FMTOWNS_MENU_LANG_EN;
    c.dynamenu_record = rec;
    c.menu_font_raster = font;
    c.glyph_draw = record_glyph_draw;
    c.glyph_draw_user = &r;
    c.label_fg = 4; c.label_bg = 0;
    assert(dm1_v1_fmtowns_menu_render_pc34(g_fb, FB_W, FB_H, FB_W, &c, &rr)
           == 1);
    assert(r.calls == 3);
    assert(r.slots_seen[0] == 1 && r.slots_seen[1] == 1 && r.slots_seen[2] == 1);
    assert(rr.label_slots_visited == 3);
    assert(rr.label_slots_drawn == 3);
    /* Last drawn label is slot 2 => index 43 => "FUSE". */
    assert(strcmp(r.last_label, "FUSE") == 0);
    /* Last slot y = y1 + 2*CHAR_Y_HYT = 77 + 14 = 91. */
    assert(r.last_y == 91);
    assert(r.last_lang == DM1_V1_FMTOWNS_MENU_LANG_EN);
    assert(r.last_fg == 4);
}

static void test_label_walk_japanese(void) {
    /* Slot 0 => index 1 = さえぎる in the Japanese pool. */
    static const uint8_t rec[8] = {0, 1, 20, 33, 0, 0, 0, 0};
    record_t r = {0};
    dm1_v1_fmtowns_menu_render_config_t c = {0};
    static const uint8_t font[768] = {0};
    memset(g_fb, 0, sizeof(g_fb));
    c.language = DM1_V1_FMTOWNS_MENU_LANG_JA;
    c.dynamenu_record = rec;
    c.menu_font_raster = font;
    c.glyph_draw = record_glyph_draw;
    c.glyph_draw_user = &r;
    assert(dm1_v1_fmtowns_menu_render_pc34(g_fb, FB_W, FB_H, FB_W, &c, NULL)
           == 1);
    assert(r.calls == 3);
    /* Last label is slot 2 => index 33 (SPELLSHIELD/呪文防御) in JP;
     * the first byte of the Shift-JIS sequence is 0x8e. */
    assert((uint8_t)r.last_label[0] == 0x8e);
    assert(r.last_lang == DM1_V1_FMTOWNS_MENU_LANG_JA);
}

static void test_disabled_slot_skips_draw(void) {
    /* Byte 2 = 0xFF disables slot 1 => GET_LABEL returns empty. */
    static const uint8_t rec[8] = {0, 1, 0xFF, 2, 0, 0, 0, 0};
    record_t r = {0};
    dm1_v1_fmtowns_menu_render_config_t c = {0};
    dm1_v1_fmtowns_menu_render_result_t rr;
    static const uint8_t font[768] = {0};
    memset(g_fb, 0, sizeof(g_fb));
    c.dynamenu_record = rec;
    c.menu_font_raster = font;
    c.glyph_draw = record_glyph_draw;
    c.glyph_draw_user = &r;
    assert(dm1_v1_fmtowns_menu_render_pc34(g_fb, FB_W, FB_H, FB_W, &c, &rr)
           == 1);
    /* Slots 0 and 2 have real labels; slot 1 is disabled. */
    assert(r.slots_seen[0] == 1);
    assert(r.slots_seen[1] == 0);
    assert(r.slots_seen[2] == 1);
    assert(rr.label_slots_visited == 3);
    assert(rr.label_slots_drawn == 2);
}

static void test_no_glyph_draw_still_fills_panel(void) {
    /* Rendering without a glyph_draw callback still paints the panel
     * background — it just doesn't render labels. This mirrors the
     * pre-font-decode state where the panel is visible but empty. */
    static const uint8_t rec[8] = {0, 1, 2, 3, 0, 0, 0, 0};
    dm1_v1_fmtowns_menu_render_config_t c = {0};
    dm1_v1_fmtowns_menu_render_result_t r;
    memset(g_fb, 0, sizeof(g_fb));
    c.dynamenu_record = rec;
    /* glyph_draw and menu_font_raster both NULL. */
    assert(dm1_v1_fmtowns_menu_render_pc34(g_fb, FB_W, FB_H, FB_W, &c, &r)
           == 1);
    assert(r.panel_filled == 1);
    assert(r.panel_pixels_written == 87u * 45u);
    assert(r.label_slots_visited == 3);
    assert(r.label_slots_drawn == 0);
}

int main(void) {
    test_null_guards();
    test_panel_fill_covers_region_10();
    test_panel_colour_selection();
    test_label_walk_english();
    test_label_walk_japanese();
    test_disabled_slot_skips_draw();
    test_no_glyph_draw_still_fills_panel();
    printf("All dm1_v1_fmtowns_menu_render tests passed.\n");
    return 0;
}
