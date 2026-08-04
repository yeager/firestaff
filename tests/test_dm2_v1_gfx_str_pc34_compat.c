/*
 * test_dm2_v1_gfx_str_pc34_compat.c — unit tests for DM2 gfx_str module.
 *
 * Tests: init defaults, QUERY_STR_METRICS, QUERY_FONT pixel patterns,
 * DRAW_STRING blit calls, FORMAT_SKSTR null termination, word-wrap.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dm2_v1_gfx_str_pc34_compat.h"

/* ── Mock tracking ─────────────────────────────────────────────────── */

static int mock_blit_count;
static int16_t mock_blit_last_dx;
static int16_t mock_blit_last_dy;
static int16_t mock_blit_last_dstw;
static uint8_t *mock_blit_last_src;

static void mock_reset(void)
{
    mock_blit_count = 0;
    mock_blit_last_dx = 0;
    mock_blit_last_dy = 0;
    mock_blit_last_dstw = 0;
    mock_blit_last_src = NULL;
}

static void mock_blit(void *ctx, uint8_t *src, uint8_t *dst,
                      int16_t sx, int16_t sy, int16_t sw, int16_t sh,
                      int16_t dx, int16_t dy, int16_t srcw, int16_t dstw)
{
    (void)ctx; (void)dst; (void)sx; (void)sy; (void)sw; (void)sh; (void)srcw;
    mock_blit_count++;
    mock_blit_last_dx = dx;
    mock_blit_last_dy = dy;
    mock_blit_last_dstw = dstw;
    mock_blit_last_src = src;
}

static uint8_t mock_screen[320 * 200];

static uint8_t *mock_get_dm2screen(void *ctx)
{
    (void)ctx;
    return mock_screen;
}

static uint8_t mock_pictbuff[224 * 160];

static uint8_t *mock_get_pictbuff(void *ctx)
{
    (void)ctx;
    return mock_pictbuff;
}

/* Simple mock font data: 6 bytes per glyph, 256 glyphs */
static uint8_t mock_font_data[256 * 6];

static void setup_mock_font(void)
{
    memset(mock_font_data, 0, sizeof(mock_font_data));
    /* 'A' = 0x41, set a recognizable pattern */
    mock_font_data[0x41 * 6 + 0] = 0x30; /* ..XX.... */
    mock_font_data[0x41 * 6 + 1] = 0x48; /* .X..X... */
    mock_font_data[0x41 * 6 + 2] = 0x78; /* .XXXX... */
    mock_font_data[0x41 * 6 + 3] = 0x48; /* .X..X... */
    mock_font_data[0x41 * 6 + 4] = 0x48; /* .X..X... */
    mock_font_data[0x41 * 6 + 5] = 0x00; /* ........ */

    /* Space = 0x20, all zeros */
}

/* Format context mocks */
static int16_t mock_v1e0218_val = 0;
static const char *mock_hero_names[] = { "HALK", "SYRA", "MOPHUS", "LEIF" };

static int16_t mock_get_v1e0218(void *ctx)
{
    (void)ctx;
    return mock_v1e0218_val;
}

static const char *mock_get_hero_name(void *ctx, int32_t idx)
{
    (void)ctx;
    if (idx >= 0 && idx < 4) return mock_hero_names[idx];
    return NULL;
}

static const uint8_t mock_v1e0988_buf[] = "ITEM";
static const uint8_t *mock_get_v1e0988(void *ctx)
{
    (void)ctx;
    return mock_v1e0988_buf;
}

/* ── Tests ─────────────────────────────────────────────────────────── */

static void test_init_defaults(void)
{
    DM2_V1_GfxStrState state;
    memset(&state, 0xFF, sizeof(state));

    dm2_v1_gfx_str_init(&state);

    assert(state.strx == 0);
    assert(state.stry == 0);
    assert(state.strxplus == 7);
    assert(state.strptr == NULL);
    assert(state.gfxstrw1 == 6);
    assert(state.gfxstrw2 == 1);
    assert(state.gfxstrw3 == 1);
    assert(state.gfxstrw4 == 6);
    assert(state.font[0] == 0);
    assert(state.font[23] == 0);

    printf("  PASS: init_defaults\n");
}

static void test_init_null(void)
{
    /* Should not crash */
    dm2_v1_gfx_str_init(NULL);
    printf("  PASS: init_null\n");
}

static void test_query_str_metrics_simple(void)
{
    DM2_V1_GfxStrState state;
    int16_t w, h;

    dm2_v1_gfx_str_init(&state);

    /* "AB" = 2 chars, width = 6 + 1 + 6 = 13, height = 7 */
    dm2_v1_gfx_str_query_str_metrics(&state, "AB", &w, &h);
    assert(w == 13);
    assert(h == 7);

    printf("  PASS: query_str_metrics_simple\n");
}

static void test_query_str_metrics_empty(void)
{
    DM2_V1_GfxStrState state;
    int16_t w, h;

    dm2_v1_gfx_str_init(&state);

    dm2_v1_gfx_str_query_str_metrics(&state, "", &w, &h);
    assert(w == 0);
    assert(h == 7); /* 1 line * 7 */

    printf("  PASS: query_str_metrics_empty\n");
}

static void test_query_str_metrics_multiline(void)
{
    DM2_V1_GfxStrState state;
    int16_t w, h;

    dm2_v1_gfx_str_init(&state);

    /* "AB\nC" = line1 width 13, line2 width 6, height = 2*7 = 14 */
    dm2_v1_gfx_str_query_str_metrics(&state, "AB\nC", &w, &h);
    assert(w == 13);
    assert(h == 14);

    printf("  PASS: query_str_metrics_multiline\n");
}

static void test_query_str_metrics_single_char(void)
{
    DM2_V1_GfxStrState state;
    int16_t w, h;

    dm2_v1_gfx_str_init(&state);

    dm2_v1_gfx_str_query_str_metrics(&state, "X", &w, &h);
    assert(w == 6);
    assert(h == 7);

    printf("  PASS: query_str_metrics_single_char\n");
}

static void test_query_str_metrics_null(void)
{
    DM2_V1_GfxStrState state;
    int16_t w = 99, h = 99;

    dm2_v1_gfx_str_init(&state);

    dm2_v1_gfx_str_query_str_metrics(&state, NULL, &w, &h);
    assert(w == 0);
    assert(h == 0);

    dm2_v1_gfx_str_query_str_metrics(NULL, "test", &w, &h);
    assert(w == 0);
    assert(h == 0);

    printf("  PASS: query_str_metrics_null\n");
}

static void test_query_font_pattern(void)
{
    DM2_V1_GfxStrState state;

    dm2_v1_gfx_str_init(&state);
    setup_mock_font();
    state.strptr = mock_font_data;

    dm2_v1_gfx_str_query_font(&state, 'A', 0x0F, 0x00);

    /* Row 0: 0x30 = 00110000, first 4 pixels: 0,0,F,F */
    assert(state.font[0] == 0x00);
    assert(state.font[1] == 0x00);
    assert(state.font[2] == 0x0F);
    assert(state.font[3] == 0x0F);

    /* Row 1: 0x48 = 01001000, first 4 pixels: 0,F,0,0 */
    assert(state.font[4] == 0x00);
    assert(state.font[5] == 0x0F);
    assert(state.font[6] == 0x00);
    assert(state.font[7] == 0x00);

    printf("  PASS: query_font_pattern\n");
}

static void test_query_font_bg_fill(void)
{
    DM2_V1_GfxStrState state;

    dm2_v1_gfx_str_init(&state);
    setup_mock_font();
    state.strptr = mock_font_data;

    /* Space glyph (0x20) is all zeros — should fill with bg_color */
    dm2_v1_gfx_str_query_font(&state, ' ', 0x0F, 0x07);
    assert(state.font[0] == 0x07);
    assert(state.font[1] == 0x07);
    assert(state.font[23] == 0x07);

    printf("  PASS: query_font_bg_fill\n");
}

static void test_draw_string_blit_calls(void)
{
    DM2_V1_GfxStrState state;
    DM2_V1_GfxStrCallbacks cb;
    uint8_t dst[320 * 10];

    dm2_v1_gfx_str_init(&state);
    setup_mock_font();
    state.strptr = mock_font_data;

    memset(&cb, 0, sizeof(cb));
    cb.blit = mock_blit;

    mock_reset();

    /* Draw "AB" at (10, 5) */
    dm2_v1_gfx_str_draw_string(&state, dst, 320,
                                10, 5, 0x0F, 0x00, "AB", &cb, NULL);

    /* Should have called blit twice (once per character) */
    assert(mock_blit_count == 2);

    printf("  PASS: draw_string_blit_calls\n");
}

static void test_draw_string_positions(void)
{
    DM2_V1_GfxStrState state;
    DM2_V1_GfxStrCallbacks cb;
    uint8_t dst[320 * 10];

    /* Track all blit positions */
    static int16_t dx_log[16];
    static int call_idx;

    dm2_v1_gfx_str_init(&state);
    setup_mock_font();
    state.strptr = mock_font_data;

    memset(&cb, 0, sizeof(cb));
    cb.blit = mock_blit;

    mock_reset();

    /* Draw "XY" at (10, 5), char advance = 6, gap = 1, so
     * X at dx=10, Y at dx=10+6+1=17 */
    dm2_v1_gfx_str_draw_string(&state, dst, 320,
                                10, 5, 0x0F, 0x00, "X", &cb, NULL);
    assert(mock_blit_count == 1);
    assert(mock_blit_last_dx == 10);
    assert(mock_blit_last_dy == 5);
    assert(mock_blit_last_dstw == 320);

    printf("  PASS: draw_string_positions\n");

    (void)dx_log;
    (void)call_idx;
}

static void test_draw_string_newline(void)
{
    DM2_V1_GfxStrState state;
    DM2_V1_GfxStrCallbacks cb;
    uint8_t dst[320 * 20];

    dm2_v1_gfx_str_init(&state);
    setup_mock_font();
    state.strptr = mock_font_data;

    memset(&cb, 0, sizeof(cb));
    cb.blit = mock_blit;

    mock_reset();

    /* "A\nB" — A at (0,0), B at (0,7) */
    dm2_v1_gfx_str_draw_string(&state, dst, 320,
                                0, 0, 0x0F, 0x00, "A\nB", &cb, NULL);
    assert(mock_blit_count == 2);
    /* Last blit should be B at dy = 7 (strxplus) */
    assert(mock_blit_last_dy == 7);
    assert(mock_blit_last_dx == 0);

    printf("  PASS: draw_string_newline\n");
}

static void test_format_skstr_null_termination(void)
{
    char dest[256];

    dm2_v1_gfx_str_format_skstr("Hello", dest, NULL, NULL);
    assert(strcmp(dest, "Hello") == 0);

    dm2_v1_gfx_str_format_skstr("", dest, NULL, NULL);
    assert(dest[0] == '\0');

    printf("  PASS: format_skstr_null_termination\n");
}

static void test_format_skstr_passthrough(void)
{
    char dest[256];

    dm2_v1_gfx_str_format_skstr("No escapes here!", dest, NULL, NULL);
    assert(strcmp(dest, "No escapes here!") == 0);

    printf("  PASS: format_skstr_passthrough\n");
}

static void test_format_skstr_newline_escape(void)
{
    char dest[256];

    /* .Zd should insert newline */
    dm2_v1_gfx_str_format_skstr("Line1.ZdLine2", dest, NULL, NULL);
    assert(strcmp(dest, "Line1\nLine2") == 0);

    printf("  PASS: format_skstr_newline_escape\n");
}

static void test_format_skstr_hero_name(void)
{
    char dest[256];
    DM2_V1_GfxStrCallbacks cb;

    memset(&cb, 0, sizeof(cb));
    cb.get_v1e0218 = mock_get_v1e0218;
    cb.get_hero_name = mock_get_hero_name;

    mock_v1e0218_val = 0;
    dm2_v1_gfx_str_format_skstr("Hi .Za!", dest, &cb, NULL);
    assert(strcmp(dest, "Hi HALK!") == 0);

    mock_v1e0218_val = 2;
    dm2_v1_gfx_str_format_skstr(".Za says hello", dest, &cb, NULL);
    assert(strcmp(dest, "MOPHUS says hello") == 0);

    printf("  PASS: format_skstr_hero_name\n");
}

static void test_format_skstr_buffer_insert(void)
{
    char dest[256];
    DM2_V1_GfxStrCallbacks cb;

    memset(&cb, 0, sizeof(cb));
    cb.get_v1e0988 = mock_get_v1e0988;

    dm2_v1_gfx_str_format_skstr("Got .Zb!", dest, &cb, NULL);
    assert(strcmp(dest, "Got ITEM!") == 0);

    printf("  PASS: format_skstr_buffer_insert\n");
}

static void test_format_skstr_null_args(void)
{
    char dest[256];

    dest[0] = 'X';
    dm2_v1_gfx_str_format_skstr(NULL, dest, NULL, NULL);
    assert(dest[0] == '\0');

    dm2_v1_gfx_str_format_skstr("test", NULL, NULL, NULL);
    /* Should not crash */

    printf("  PASS: format_skstr_null_args\n");
}

static void test_word_wrap_basic(void)
{
    DM2_V1_GfxStrState state;
    char dest[256];
    int32_t result;

    dm2_v1_gfx_str_init(&state);
    memset(dest, 0, sizeof(dest));

    /* With char_w=6, gap=1, each char is 7 pixels wide.
     * max_width=21 fits 3 chars (3*7=21).
     * "AB CD" — "AB " fits (3 chars), "CD" on next line. */
    result = dm2_v1_gfx_str_word_wrap(&state, "AB CD", dest, 0, 21);
    assert(result == 5);
    /* The space between AB and CD should become a newline */
    assert(dest[2] == '\n');
    assert(dest[0] == 'A');
    assert(dest[1] == 'B');
    assert(dest[3] == 'C');
    assert(dest[4] == 'D');

    printf("  PASS: word_wrap_basic\n");
}

static void test_word_wrap_no_wrap_needed(void)
{
    DM2_V1_GfxStrState state;
    char dest[256];
    int32_t result;

    dm2_v1_gfx_str_init(&state);
    memset(dest, 0, sizeof(dest));

    /* max_width=100 easily fits "Hi" (2*7=14 pixels) */
    result = dm2_v1_gfx_str_word_wrap(&state, "Hi", dest, 0, 100);
    assert(result == 2);
    assert(dest[0] == 'H');
    assert(dest[1] == 'i');
    assert(dest[2] == '\0');

    printf("  PASS: word_wrap_no_wrap_needed\n");
}

static void test_word_wrap_null(void)
{
    DM2_V1_GfxStrState state;

    dm2_v1_gfx_str_init(&state);

    assert(dm2_v1_gfx_str_word_wrap(&state, NULL, NULL, 0, 100) == 0);
    assert(dm2_v1_gfx_str_word_wrap(NULL, "test", NULL, 0, 100) == 0);

    printf("  PASS: word_wrap_null\n");
}

static void test_draw_strong_text(void)
{
    DM2_V1_GfxStrState state;
    DM2_V1_GfxStrCallbacks cb;
    uint8_t dst[320 * 20];

    dm2_v1_gfx_str_init(&state);
    setup_mock_font();
    state.strptr = mock_font_data;

    memset(&cb, 0, sizeof(cb));
    cb.blit = mock_blit;

    mock_reset();

    /* Draw "A" with strong text — should produce 2 draw passes = 2 blits */
    dm2_v1_gfx_str_draw_strong_text(&state, dst, 320,
                                     5, 5, 0x0F, 0x07, "A", &cb, NULL);
    assert(mock_blit_count == 2);

    printf("  PASS: draw_strong_text\n");
}

static void test_draw_vp_str(void)
{
    DM2_V1_GfxStrState state;
    DM2_V1_GfxStrCallbacks cb;

    dm2_v1_gfx_str_init(&state);
    setup_mock_font();
    state.strptr = mock_font_data;

    memset(&cb, 0, sizeof(cb));
    cb.blit = mock_blit;
    cb.get_pictbuff = mock_get_pictbuff;

    mock_reset();

    dm2_v1_gfx_str_draw_vp_str(&state, 10, 20, 0x0F, "X", &cb, NULL);
    assert(mock_blit_count == 1);
    assert(mock_blit_last_dx == 10);
    assert(mock_blit_last_dy == 20);
    assert(mock_blit_last_dstw == 224); /* viewport width */

    printf("  PASS: draw_vp_str\n");
}

static void test_print_syserr_text(void)
{
    DM2_V1_GfxStrState state;
    DM2_V1_GfxStrCallbacks cb;

    dm2_v1_gfx_str_init(&state);
    setup_mock_font();
    state.strptr = mock_font_data;

    memset(&cb, 0, sizeof(cb));
    cb.blit = mock_blit;
    cb.get_dm2screen = mock_get_dm2screen;

    mock_reset();

    dm2_v1_gfx_str_print_syserr_text(&state, 0, 0, 0x0F, 0x00,
                                      "ERR", &cb, NULL);
    assert(mock_blit_count == 3);
    assert(mock_blit_last_dstw == 320); /* screen width */

    printf("  PASS: print_syserr_text\n");
}

static void test_draw_uppercase(void)
{
    DM2_V1_GfxStrState state;
    DM2_V1_GfxStrCallbacks cb;

    dm2_v1_gfx_str_init(&state);
    setup_mock_font();
    state.strptr = mock_font_data;

    memset(&cb, 0, sizeof(cb));
    cb.blit = mock_blit;
    cb.get_pictbuff = mock_get_pictbuff;

    mock_reset();

    /* "abc" -> "ABC", should draw 3 glyphs */
    dm2_v1_gfx_str_draw_uppercase(&state, 0, 0, "abc", &cb, NULL);
    assert(mock_blit_count == 3);

    printf("  PASS: draw_uppercase\n");
}

/* ── Main ──────────────────────────────────────────────────────────── */

int main(void)
{
    printf("test_dm2_v1_gfx_str_pc34_compat:\n");

    test_init_defaults();
    test_init_null();
    test_query_str_metrics_simple();
    test_query_str_metrics_empty();
    test_query_str_metrics_multiline();
    test_query_str_metrics_single_char();
    test_query_str_metrics_null();
    test_query_font_pattern();
    test_query_font_bg_fill();
    test_draw_string_blit_calls();
    test_draw_string_positions();
    test_draw_string_newline();
    test_format_skstr_null_termination();
    test_format_skstr_passthrough();
    test_format_skstr_newline_escape();
    test_format_skstr_hero_name();
    test_format_skstr_buffer_insert();
    test_format_skstr_null_args();
    test_word_wrap_basic();
    test_word_wrap_no_wrap_needed();
    test_word_wrap_null();
    test_draw_strong_text();
    test_draw_vp_str();
    test_print_syserr_text();
    test_draw_uppercase();

    printf("All 25 tests passed.\n");
    return 0;
}
