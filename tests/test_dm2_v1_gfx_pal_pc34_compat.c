/* skproject: c_gfx_pal.cpp */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "dm2_v1_gfx_pal_pc34_compat.h"

/* --- Mock callback state --- */
static float mock_colors[256][3];
static int mock_vsync_count;
static int mock_set_count;
static int mock_get_count;

static void mock_wait_for_vsync(void *ctx __attribute__((unused))) {
    mock_vsync_count++;
}

static void mock_set_screen_color(void *ctx __attribute__((unused)),
    int index, float r, float g, float b)
{
    mock_colors[index][0] = r;
    mock_colors[index][1] = g;
    mock_colors[index][2] = b;
    mock_set_count++;
}

static void mock_get_screen_color(void *ctx __attribute__((unused)),
    int index, float *r, float *g, float *b)
{
    *r = mock_colors[index][0];
    *g = mock_colors[index][1];
    *b = mock_colors[index][2];
    mock_get_count++;
}

static void reset_mocks(void) {
    memset(mock_colors, 0, sizeof(mock_colors));
    mock_vsync_count = 0;
    mock_set_count = 0;
    mock_get_count = 0;
}

static DM2_V1_GfxPalCallbacks test_callbacks = {
    mock_wait_for_vsync,
    mock_set_screen_color,
    mock_get_screen_color
};

/* --- Tests --- */

static void test_palettecolor_roundtrip(void) {
    printf("  palettecolor roundtrip...\n");

    DM2_V1_Palette pal;

    /* ui8 roundtrip */
    dm2_v1_ui8_to_palettecolor(&pal, 42);
    assert(dm2_v1_palettecolor_to_ui8(pal) == 42);

    dm2_v1_ui8_to_palettecolor(&pal, 0);
    assert(dm2_v1_palettecolor_to_ui8(pal) == 0);

    dm2_v1_ui8_to_palettecolor(&pal, 255);
    assert(dm2_v1_palettecolor_to_ui8(pal) == 255);

    /* color enum roundtrip */
    dm2_v1_color_to_palettecolor(&pal, DM2_V1_E_COL05);
    assert(dm2_v1_palettecolor_to_ui8(pal) == 0x05);

    /* pixel extraction */
    dm2_v1_ui8_to_palettecolor(&pal, 123);
    DM2_V1_Pixel256 px = dm2_v1_palettecolor_to_pixel(pal);
    assert(px.p == 123);

    printf("    PASS\n");
}

static void test_palette_conv(void) {
    printf("  palette_conv lookup...\n");

    /* Build a simple swap table: 0->10, 10->0, others identity */
    DM2_V1_ColorConv conv[256];
    for (int i = 0; i < 256; i++) conv[i].p = (uint8_t)i;
    conv[0].p = 10;
    conv[10].p = 0;

    DM2_V1_Palette pal;
    dm2_v1_ui8_to_palettecolor(&pal, 0);
    dm2_v1_palette_conv(&pal, conv);
    assert(dm2_v1_palettecolor_to_ui8(pal) == 10);

    dm2_v1_ui8_to_palettecolor(&pal, 10);
    dm2_v1_palette_conv(&pal, conv);
    assert(dm2_v1_palettecolor_to_ui8(pal) == 0);

    dm2_v1_ui8_to_palettecolor(&pal, 42);
    dm2_v1_palette_conv(&pal, conv);
    assert(dm2_v1_palettecolor_to_ui8(pal) == 42);

    printf("    PASS\n");
}

static void test_palette_data_init(void) {
    printf("  palette_data_init zeroes...\n");

    DM2_V1_PaletteData pd;
    memset(&pd, 0xFF, sizeof(pd));
    dm2_v1_palette_data_init(&pd);

    assert(pd.palette == NULL);
    assert(pd.pal16to256ptr == NULL);
    assert(pd.glbl_pal1 == NULL);
    assert(pd.glbl_pal2 == NULL);
    assert(pd.immediate_colors == false);
    for (int i = 0; i < 16; i++) {
        assert(pd.small_palette[i].c.p == 0);
    }

    printf("    PASS\n");
}

static void test_convert_driver_palette(void) {
    printf("  convert_driver_palette shifts...\n");
    reset_mocks();

    /* Build test ARGB data: alpha=0xFF, R=0xFC, G=0x80, B=0x40 for entry 0 */
    uint8_t pb[256 * 4];
    memset(pb, 0, sizeof(pb));
    pb[0] = 0xFF; /* alpha */
    pb[1] = 0xFC; /* R */
    pb[2] = 0x80; /* G */
    pb[3] = 0x40; /* B */

    /* Entry 1: alpha=0x00, R=0xFF, G=0x00, B=0xFF */
    pb[4] = 0x00;
    pb[5] = 0xFF;
    pb[6] = 0x00;
    pb[7] = 0xFF;

    int8_t dmpal[768];
    memset(dmpal, 0, sizeof(dmpal));

    DM2_V1_ConvertDriverPaletteReceipt r =
        dm2_v1_convert_driver_palette(&test_callbacks, NULL, pb, dmpal, false);

    assert(r.converted == true);
    assert(r.entries_count == 256);

    /* R=0xFC >> 2 = 0x3F, G=0x80 >> 2 = 0x20, B=0x40 >> 2 = 0x10 */
    assert((uint8_t)dmpal[0] == 0x3F);
    assert((uint8_t)dmpal[1] == 0x20);
    assert((uint8_t)dmpal[2] == 0x10);

    /* Entry 1: R=0xFF>>2=0x3F, G=0x00>>2=0x00, B=0xFF>>2=0x3F */
    assert((uint8_t)dmpal[3] == 0x3F);
    assert((uint8_t)dmpal[4] == 0x00);
    assert((uint8_t)dmpal[5] == 0x3F);

    printf("    PASS\n");
}

static void test_select_palette_set(void) {
    printf("  select_palette_set modes...\n");
    reset_mocks();

    bool immediate = true;

    /* Mode 0: fade to black */
    /* Pre-set some colors */
    mock_colors[0][0] = 100.0f;
    mock_colors[0][1] = 200.0f;
    mock_colors[0][2] = 50.0f;

    DM2_V1_SelectPaletteSetReceipt r0 =
        dm2_v1_select_palette_set(&test_callbacks, NULL, 0, NULL, &immediate);

    assert(r0.applied == true);
    assert(r0.mode == 0);
    assert(immediate == false);
    /* After fade to black, colors should be ~0 */
    assert(mock_colors[0][0] < 0.01f);

    /* Mode 1: restore */
    reset_mocks();
    immediate = false;
    DM2_V1_SelectPaletteSetReceipt r1 =
        dm2_v1_select_palette_set(&test_callbacks, NULL, 1,
            DM2_V1_DMPAL_DEFAULT, &immediate);

    assert(r1.applied == true);
    assert(r1.mode == 1);
    assert(immediate == true);

    printf("    PASS\n");
}

static void test_xlat_palette(void) {
    printf("  xlat_palette...\n");

    /* Build conv table and palette */
    DM2_V1_ColorConv conv[256];
    for (int i = 0; i < 256; i++) conv[i].p = (uint8_t)(255 - i);

    DM2_V1_Palette pal[256];
    for (int i = 0; i < 256; i++) {
        pal[i].c.p = (uint8_t)i;
    }

    /* Positive colors: translate 16 entries */
    int16_t colors = 16;
    DM2_V1_XlatPaletteReceipt r = dm2_v1_xlat_palette(pal, conv, &colors);
    assert(r.translated == true);
    assert(r.final_colors == 16);
    assert(pal[0].c.p == 255);
    assert(pal[1].c.p == 254);
    assert(pal[15].c.p == 240);
    /* Entry 16 should be untouched */
    assert(pal[16].c.p == 16);

    /* Non-positive colors: sets to 256 */
    colors = 0;
    r = dm2_v1_xlat_palette(pal, conv, &colors);
    assert(r.translated == true);
    assert(r.final_colors == 256);
    assert(colors == 256);

    printf("    PASS\n");
}

static void test_update_blit_palette(void) {
    printf("  update_blit_palette...\n");

    DM2_V1_PaletteData pd;
    dm2_v1_palette_data_init(&pd);
    assert(pd.palette == NULL);

    DM2_V1_Palette pal;
    dm2_v1_ui8_to_palettecolor(&pal, 7);
    dm2_v1_update_blit_palette(&pd, &pal);
    assert(pd.palette == &pal);

    printf("    PASS\n");
}

static void test_dmpal_default_spot_checks(void) {
    printf("  DMPAL_DEFAULT spot checks...\n");

    /* First triplet: 0,0,0 */
    assert(DM2_V1_DMPAL_DEFAULT[0] == 0x00);
    assert(DM2_V1_DMPAL_DEFAULT[1] == 0x00);
    assert(DM2_V1_DMPAL_DEFAULT[2] == 0x00);

    /* Second triplet: 3,2,0 */
    assert(DM2_V1_DMPAL_DEFAULT[3] == 0x03);
    assert(DM2_V1_DMPAL_DEFAULT[4] == 0x02);
    assert(DM2_V1_DMPAL_DEFAULT[5] == 0x00);

    /* Third triplet: 7,4,0 */
    assert(DM2_V1_DMPAL_DEFAULT[6] == 0x07);
    assert(DM2_V1_DMPAL_DEFAULT[7] == 0x04);
    assert(DM2_V1_DMPAL_DEFAULT[8] == 0x00);

    /* Last triplet (index 255): 0x3f,0x3f,0x3f */
    assert(DM2_V1_DMPAL_DEFAULT[765] == 0x3f);
    assert(DM2_V1_DMPAL_DEFAULT[766] == 0x3f);
    assert(DM2_V1_DMPAL_DEFAULT[767] == 0x3f);

    /* Row 2 (palette 1), entry 0: 0,0,0 */
    assert(DM2_V1_DMPAL_DEFAULT[48] == 0x00);
    assert(DM2_V1_DMPAL_DEFAULT[49] == 0x00);
    assert(DM2_V1_DMPAL_DEFAULT[50] == 0x00);

    /* Row 2, entry 1: 2,1,3 */
    assert(DM2_V1_DMPAL_DEFAULT[51] == 0x02);
    assert(DM2_V1_DMPAL_DEFAULT[52] == 0x01);
    assert(DM2_V1_DMPAL_DEFAULT[53] == 0x03);

    /* Constants */
    assert(DM2_V1_PAL16 == 16);
    assert(DM2_V1_PAL256 == 256);
    assert(fabs(DM2_V1_DMFCOL - (255.0 / 63.0)) < 0.001);

    printf("    PASS\n");
}

int main(void) {
    printf("test_dm2_v1_gfx_pal_pc34_compat\n");

    test_palettecolor_roundtrip();
    test_palette_conv();
    test_palette_data_init();
    test_convert_driver_palette();
    test_select_palette_set();
    test_xlat_palette();
    test_update_blit_palette();
    test_dmpal_default_spot_checks();

    printf("All tests passed.\n");
    return 0;
}
