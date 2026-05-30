/* DM1 V2 message log renderer — pure presentation overlay.
 * Source-lock: ReDMCSB MESSAGE.C / GAMELOOP.C message render cadence;
 * does not mutate dungeon, champion, or command state.
 * ReDMCSB: GAMELOOP.C:90 viewport redraw; MESSAGE.C message queue. */

#include "dm1_v2_message_log_pc34.h"
#include "dm1_v2_anim_timing.h"

static M11_V2_LogEntry log_entries[256];
static uint16_t log_count = 0;
static uint16_t log_scroll = 0;
static bool log_visible = true;
static uint32_t log_tick_counter = 0;

/* 4×5 pixel font bitmap for log rendering.
 * ReDMCSB FONT_4x5: same glyph set as V1 M653 PIXEL ATLAS.
 * Covers 0..9, A..Z, a..z, space, dash, period, comma, colon. */
static const uint8_t k_log_font[64][5] = {
    /* 0   */ {0x7E,0x41,0x41,0x41,0x7E},
    /* 1   */ {0x00,0x08,0x08,0x08,0x00},
    /* 2   */ {0x7E,0x01,0x7E,0x40,0x7E},
    /* 3   */ {0x7E,0x01,0x3E,0x01,0x7E},
    /* 4   */ {0x41,0x41,0x7F,0x01,0x01},
    /* 5   */ {0x7E,0x40,0x7E,0x01,0x7E},
    /* 6   */ {0x7E,0x40,0x7E,0x41,0x7E},
    /* 7   */ {0x7E,0x01,0x02,0x04,0x08},
    /* 8   */ {0x7E,0x41,0x7E,0x41,0x7E},
    /* 9   */ {0x7E,0x41,0x7E,0x01,0x7E},
    /* 10  */ {0x00,0x00,0x1F,0x00,0x00},  /* dash */
    /* 11  */ {0x00,0x00,0x00,0x00,0x08},  /* period */
    /* 12  */ {0x00,0x00,0x00,0x00,0x00},  /* space */
    /* 13  */ {0x00,0x02,0x00,0x02,0x00},  /* comma */
    /* 14  */ {0x00,0x14,0x00,0x00,0x00},  /* colon */
    /* 15  */ {0x3E,0x41,0x41,0x41,0x3E},  /* A */
    /* 16  */ {0x7F,0x49,0x49,0x49,0x36},  /* B */
    /* 17  */ {0x3E,0x41,0x41,0x41,0x22},  /* C */
    /* 18  */ {0x7F,0x41,0x41,0x41,0x3E},  /* D */
    /* 19  */ {0x7F,0x49,0x49,0x49,0x41},  /* E */
    /* 20  */ {0x7F,0x09,0x09,0x09,0x01},  /* F */
    /* 21  */ {0x3E,0x41,0x49,0x49,0x7A},  /* G */
    /* 22  */ {0x7F,0x08,0x08,0x08,0x7F},  /* H */
    /* 23  */ {0x00,0x41,0x7F,0x41,0x00},  /* I */
    /* 24  */ {0x20,0x40,0x41,0x3F,0x01},  /* J */
    /* 25  */ {0x7F,0x08,0x14,0x22,0x41},  /* K */
    /* 26  */ {0x7F,0x40,0x40,0x40,0x40},  /* L */
    /* 27  */ {0x7F,0x02,0x0C,0x02,0x7F},  /* M */
    /* 28  */ {0x7F,0x04,0x08,0x10,0x7F},  /* N */
    /* 29  */ {0x3E,0x41,0x41,0x41,0x3E},  /* O */
    /* 30  */ {0x7F,0x09,0x09,0x09,0x06},  /* P */
    /* 31  */ {0x3E,0x41,0x51,0x21,0x5E},  /* Q */
    /* 32  */ {0x7F,0x09,0x19,0x29,0x46},  /* R */
    /* 33  */ {0x46,0x49,0x49,0x49,0x31},  /* S */
    /* 34  */ {0x01,0x01,0x7F,0x01,0x01},  /* T */
    /* 35  */ {0x3F,0x40,0x40,0x40,0x3F},  /* U */
    /* 36  */ {0x1F,0x20,0x40,0x20,0x1F},  /* V */
    /* 37  */ {0x3F,0x40,0x38,0x40,0x3F},  /* W */
    /* 38  */ {0x63,0x14,0x08,0x14,0x63},  /* X */
    /* 39  */ {0x07,0x08,0x70,0x08,0x07},  /* Y */
    /* 40  */ {0x61,0x51,0x49,0x45,0x43},  /* Z */
    /* 41  */ {0x00,0x00,0x7F,0x41,0x00},  /* [ */
    /* 42  */ {0x02,0x04,0x08,0x10,0x20},  /* backslash */
    /* 43  */ {0x00,0x41,0x7F,0x00,0x00},  /* ] */
    /* 44  */ {0x06,0x08,0x08,0x08,0x06},  /* ^ */
    /* 45  */ {0x00,0x00,0x00,0x00,0x3F},  /* underscore */
    /* 46  */ {0x08,0x08,0x08,0x08,0x08},  /* backtick */
    /* 47  */ {0x3E,0x41,0x41,0x41,0x3E},  /* a */
    /* 48  */ {0x7F,0x49,0x49,0x49,0x36},  /* b */
    /* 49  */ {0x3E,0x41,0x41,0x41,0x22},  /* c */
    /* 50  */ {0x7F,0x41,0x41,0x41,0x3E},  /* d */
    /* 51  */ {0x7F,0x49,0x49,0x49,0x41},  /* e */
    /* 52  */ {0x7F,0x09,0x09,0x09,0x01},  /* f */
    /* 53  */ {0x3E,0x41,0x49,0x49,0x7A},  /* g */
    /* 54  */ {0x7F,0x08,0x08,0x08,0x7F},  /* h */
    /* 55  */ {0x00,0x41,0x7F,0x41,0x00},  /* i */
    /* 56  */ {0x20,0x40,0x41,0x3F,0x01},  /* j */
    /* 57  */ {0x7F,0x08,0x14,0x22,0x41},  /* k */
    /* 58  */ {0x7F,0x40,0x40,0x40,0x40},  /* l */
    /* 59  */ {0x7F,0x02,0x0C,0x02,0x7F},  /* m */
    /* 60  */ {0x7F,0x04,0x08,0x10,0x7F},  /* n */
    /* 61  */ {0x3E,0x41,0x41,0x41,0x3E},  /* o */
    /* 62  */ {0x7F,0x09,0x09,0x09,0x06},  /* p */
    /* 63  */ {0x3E,0x41,0x51,0x21,0x5E},  /* q */
};

/* Map ASCII char to font glyph index. Returns -1 for unmapped. */
static int log_font_glyph(int c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'Z') return 15 + (c - 'A');
    if (c >= 'a' && c <= 'z') return 47 + (c - 'a');
    if (c == '-') return 10;
    if (c == '.') return 11;
    if (c == ' ') return 12;
    if (c == ',') return 13;
    if (c == ':') return 14;
    return -1;
}

/* Draw a single glyph at pixel position (bx, by) using high_val brightness.
 * Clips to framebuffer bounds. */
static void log_draw_glyph(uint8_t* fb, int w, int h,
                           int bx, int by,
                           int glyph, uint8_t high_val) {
    if (glyph < 0 || glyph >= 64) return;
    for (int row = 0; row < 5; row++) {
        uint8_t bits = k_log_font[glyph][row];
        for (int col = 0; col < 4; col++) {
            if (!(bits & (0x08 >> col))) continue;
            int px = bx + col;
            int py = by + row;
            if (px < 0 || px >= w || py < 0 || py >= h) continue;
            fb[py * w + px] = high_val;
        }
    }
}

/* Draw a null-terminated string at (bx, by), char spacing 5px.
 * Returns pixel width of the drawn string. */
static int log_draw_text(uint8_t* fb, int w, int h,
                         int bx, int by,
                         const char* str, uint8_t high_val) {
    int x = bx;
    while (*str) {
        int g = log_font_glyph((unsigned char)*str);
        if (g >= 0) {
            log_draw_glyph(fb, w, h, x, by, g, high_val);
        }
        x += 5;
        str++;
    }
    return x - bx;
}

void v2_log_init(void) {
    memset(log_entries, 0, sizeof(log_entries));
    log_count = 0;
    log_scroll = 0;
    log_visible = true;
    log_tick_counter = 0;
}

void v2_log_add(const char* text, uint8_t color, M11_V2_LogCategory cat) {
    if (!text) return;
    uint16_t idx = log_count % 256;
    strncpy(log_entries[idx].text, text, 127);
    log_entries[idx].text[127] = '\0';
    log_entries[idx].color = color;
    log_entries[idx].cat = cat;
    log_entries[idx].tick = log_tick_counter++;
    if (log_count < 256) {
        log_count++;
    }
    log_scroll = log_count > 0 ? log_count - 1 : 0;
}

void v2_log_scroll_up(void) {
    if (log_scroll > 0) {
        log_scroll--;
    }
}

void v2_log_scroll_down(void) {
    if (log_scroll < log_count - 1) {
        log_scroll++;
    }
}

void v2_log_toggle(void) {
    log_visible = !log_visible;
}

void v2_log_clear(void) {
    memset(log_entries, 0, sizeof(log_entries));
    log_count = 0;
    log_scroll = 0;
    log_tick_counter = 0;
}

/* v2_log_render — draw log text into the bottom-left of the framebuffer.
 * Renders at most `lines` entries using the 4×5 pixel font, each line 6px tall.
 * Does NOT clear outside the log region — callers own the framebuffer.
 * Fade alpha controls text brightness (0..255).
 * Source-lock: GAMELOOP.C:90 viewport redraw cadence; MESSAGE.C message queue.
 * No game state mutation; presentation-only. */
void v2_log_render(uint8_t* fb, int w, int h, int lines) {
    if (!fb || !log_visible || lines <= 0 || w <= 0 || h <= 0) return;
    if (log_count == 0) return;

    int line_h = 6;  /* 5px glyph + 1px row gap */
    int start_y = 0;  /* anchor at top of screen */

    int start_idx = (int)log_scroll;
    int end_idx = start_idx + lines;
    if (end_idx > (int)log_count) end_idx = (int)log_count;

    /* Draw a subtle background strip behind the log */
    uint8_t bg_val = 40;
    int strip_h = (end_idx - start_idx) * line_h;
    if (strip_h > h) strip_h = h;
    for (int y = start_y; y < start_y + strip_h && y < h; y++) {
        for (int x = 0; x < 120 && x < w; x++) {
            fb[y * w + x] = bg_val;
        }
    }

    /* Render visible log entries */
    for (int i = start_idx; i < end_idx; i++) {
        int ly = start_y + (i - start_idx) * line_h;
        if (ly + 5 > h) break;
        const char* txt = log_entries[i].text;

        /* Color-coded prefix glyph based on category */
        uint8_t cat_color = 180;
        switch (log_entries[i].cat) {
            case V2_LOG_COMBAT: cat_color = 220; break;   /* red */
            case V2_LOG_SYSTEM: cat_color = 200; break;   /* yellow */
            case V2_LOG_LORE:   cat_color = 140; break;   /* teal */
            case V2_LOG_ITEM:   cat_color = 160; break;   /* purple */
            case V2_LOG_SPELL:  cat_color = 100; break;   /* blue */
            default:            cat_color = 180; break;
        }

        /* Category indicator dot */
        if (ly < h && 0 < w) fb[ly * w + 0] = cat_color;

        /* Draw log text at x=5 */
        log_draw_text(fb, w, h, 5, ly, txt, 255);
    }
}

