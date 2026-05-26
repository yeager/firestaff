/*
 * dm1_v1_combat_log_pc34_compat — combat log implementation.
 *
 * Ring buffer of up to combatLogMaxLines entries.  Renders the last
 * N entries (where N fits along the bottom of the 320x200 framebuffer)
 * with a half-transparent dithered background.  Uses the M11 original
 * font when available, otherwise falls back to a built-in 3x5 mini font.
 */

#include "dm1_v1_combat_log_pc34_compat.h"
#include "m11_qol_runtime.h"
#include "font_m11.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define COMBAT_LOG_RING_CAP 512   /* hard cap; combatLogMaxLines clamps display */

static M11_CombatLogEntry g_ring[COMBAT_LOG_RING_CAP];
static int                g_count;     /* total pushed (monotonic) */
static int                g_writeIdx;  /* next slot to write */

void DM1_CombatLog_Reset(void) {
    memset(g_ring, 0, sizeof(g_ring));
    g_count = 0;
    g_writeIdx = 0;
}

void DM1_CombatLog_Pushf(uint32_t gameTick,
                         M11_CombatLogType type,
                         const char* fmt, ...) {
    M11_CombatLogEntry* e;
    va_list ap;
    if (!fmt) return;
    e = &g_ring[g_writeIdx];
    e->gameTick = gameTick;
    e->type = (uint8_t)type;
    va_start(ap, fmt);
    vsnprintf(e->text, sizeof(e->text), fmt, ap);
    va_end(ap);
    e->text[sizeof(e->text) - 1] = '\0';
    g_writeIdx = (g_writeIdx + 1) % COMBAT_LOG_RING_CAP;
    if (g_count < COMBAT_LOG_RING_CAP) g_count++;
}

void DM1_CombatLog_OnChampionHit(uint32_t gameTick,
                                 const char* championName,
                                 const char* creatureName,
                                 int damage) {
    DM1_CombatLog_Pushf(gameTick, M11_COMBAT_LOG_TYPE_CHAMP_HIT,
                        "T%u: %s hits %s for %d",
                        (unsigned int)gameTick,
                        championName ? championName : "?",
                        creatureName ? creatureName : "creature",
                        damage);
}

void DM1_CombatLog_OnCreatureAttack(uint32_t gameTick,
                                    const char* creatureName,
                                    const char* championName) {
    DM1_CombatLog_Pushf(gameTick, M11_COMBAT_LOG_TYPE_CREATURE_HIT,
                        "T%u: %s attacks %s",
                        (unsigned int)gameTick,
                        creatureName ? creatureName : "creature",
                        championName ? championName : "party");
}

void DM1_CombatLog_OnSpellCast(uint32_t gameTick,
                               const char* championName,
                               const char* spellName) {
    DM1_CombatLog_Pushf(gameTick, M11_COMBAT_LOG_TYPE_SPELL,
                        "T%u: %s casts %s",
                        (unsigned int)gameTick,
                        championName ? championName : "?",
                        spellName ? spellName : "spell");
}

/* ---- Built-in mini font (3x5) for fallback rendering -------------- */
/* Each glyph: 5 bytes (rows top->bottom); each byte's low 3 bits are
 * the pixel mask (bit 2 = left, bit 0 = right). */
static const unsigned char k_minifont_ascii32_126[][5] = {
    /* space */ {0,0,0,0,0},
    /* ! */     {0x2,0x2,0x2,0,0x2},
    /* " */     {0x5,0x5,0,0,0},
    /* # */     {0x5,0x7,0x5,0x7,0x5},
    /* $ */     {0x3,0x6,0x7,0x3,0x6},
    /* % */     {0x5,0x1,0x2,0x4,0x5},
    /* & */     {0x6,0x4,0x7,0x5,0x7},
    /* ' */     {0x2,0x2,0,0,0},
    /* ( */     {0x2,0x4,0x4,0x4,0x2},
    /* ) */     {0x2,0x1,0x1,0x1,0x2},
    /* * */     {0,0x5,0x2,0x5,0},
    /* + */     {0,0x2,0x7,0x2,0},
    /* , */     {0,0,0,0x2,0x4},
    /* - */     {0,0,0x7,0,0},
    /* . */     {0,0,0,0,0x2},
    /* / */     {0x1,0x1,0x2,0x4,0x4},
    /* 0 */     {0x7,0x5,0x5,0x5,0x7},
    /* 1 */     {0x2,0x6,0x2,0x2,0x7},
    /* 2 */     {0x7,0x1,0x7,0x4,0x7},
    /* 3 */     {0x7,0x1,0x7,0x1,0x7},
    /* 4 */     {0x5,0x5,0x7,0x1,0x1},
    /* 5 */     {0x7,0x4,0x7,0x1,0x7},
    /* 6 */     {0x7,0x4,0x7,0x5,0x7},
    /* 7 */     {0x7,0x1,0x2,0x4,0x4},
    /* 8 */     {0x7,0x5,0x7,0x5,0x7},
    /* 9 */     {0x7,0x5,0x7,0x1,0x7},
    /* : */     {0,0x2,0,0x2,0},
    /* ; */     {0,0x2,0,0x2,0x4},
    /* < */     {0x1,0x2,0x4,0x2,0x1},
    /* = */     {0,0x7,0,0x7,0},
    /* > */     {0x4,0x2,0x1,0x2,0x4},
    /* ? */     {0x7,0x1,0x3,0,0x2},
    /* @ */     {0x7,0x5,0x7,0x4,0x7},
    /* A */     {0x2,0x5,0x7,0x5,0x5},
    /* B */     {0x6,0x5,0x6,0x5,0x6},
    /* C */     {0x3,0x4,0x4,0x4,0x3},
    /* D */     {0x6,0x5,0x5,0x5,0x6},
    /* E */     {0x7,0x4,0x6,0x4,0x7},
    /* F */     {0x7,0x4,0x6,0x4,0x4},
    /* G */     {0x3,0x4,0x5,0x5,0x3},
    /* H */     {0x5,0x5,0x7,0x5,0x5},
    /* I */     {0x7,0x2,0x2,0x2,0x7},
    /* J */     {0x1,0x1,0x1,0x5,0x2},
    /* K */     {0x5,0x5,0x6,0x5,0x5},
    /* L */     {0x4,0x4,0x4,0x4,0x7},
    /* M */     {0x5,0x7,0x5,0x5,0x5},
    /* N */     {0x5,0x7,0x7,0x7,0x5},
    /* O */     {0x2,0x5,0x5,0x5,0x2},
    /* P */     {0x6,0x5,0x6,0x4,0x4},
    /* Q */     {0x2,0x5,0x5,0x7,0x3},
    /* R */     {0x6,0x5,0x6,0x5,0x5},
    /* S */     {0x3,0x4,0x2,0x1,0x6},
    /* T */     {0x7,0x2,0x2,0x2,0x2},
    /* U */     {0x5,0x5,0x5,0x5,0x7},
    /* V */     {0x5,0x5,0x5,0x5,0x2},
    /* W */     {0x5,0x5,0x5,0x7,0x5},
    /* X */     {0x5,0x5,0x2,0x5,0x5},
    /* Y */     {0x5,0x5,0x2,0x2,0x2},
    /* Z */     {0x7,0x1,0x2,0x4,0x7},
    /* [ */     {0x6,0x4,0x4,0x4,0x6},
    /* \ */     {0x4,0x4,0x2,0x1,0x1},
    /* ] */     {0x3,0x1,0x1,0x1,0x3},
    /* ^ */     {0x2,0x5,0,0,0},
    /* _ */     {0,0,0,0,0x7},
    /* ` */     {0x4,0x2,0,0,0}
};

static void put_pixel(unsigned char* fb, int fbW, int fbH,
                      int x, int y, unsigned char color) {
    if (x < 0 || x >= fbW || y < 0 || y >= fbH) return;
    fb[y * fbW + x] = color;
}

static int draw_glyph_minifont(unsigned char* fb, int fbW, int fbH,
                               int x, int y, char ch, unsigned char color) {
    const unsigned char* glyph;
    int gx, gy;
    int idx;
    if (ch >= 'a' && ch <= 'z') ch = (char)(ch - 'a' + 'A');
    if (ch < 32 || ch > '`') ch = '?';
    idx = ch - 32;
    if (idx < 0 || idx >= (int)(sizeof(k_minifont_ascii32_126) /
                                 sizeof(k_minifont_ascii32_126[0]))) {
        idx = '?' - 32;
    }
    glyph = k_minifont_ascii32_126[idx];
    for (gy = 0; gy < 5; ++gy) {
        unsigned char row = glyph[gy];
        for (gx = 0; gx < 3; ++gx) {
            if (row & (1u << (2 - gx))) {
                put_pixel(fb, fbW, fbH, x + gx, y + gy, color);
            }
        }
    }
    return 4; /* glyph width + 1px spacing */
}

static int draw_text_minifont(unsigned char* fb, int fbW, int fbH,
                              int x, int y, const char* text, unsigned char color) {
    int px = x;
    if (!text) return 0;
    while (*text) {
        px += draw_glyph_minifont(fb, fbW, fbH, px, y, *text, color);
        ++text;
    }
    return px - x;
}

void DM1_CombatLog_Render(M11_GameViewState* state,
                          unsigned char* fb,
                          int fbW,
                          int fbH) {
    int lineH;
    int maxLines;
    int displayLines;
    int barH;
    int barY;
    int i;
    int currentDrawIdx;
    int oldestIdx;
    unsigned char fontFg = 15; /* white */
    unsigned char fontBg = 0;  /* black background dither */
    int useOriginalFont = 0;

    if (!fb || fbW <= 0 || fbH <= 0) return;
    if (!M11_QolRuntime_GetCombatLogEnabled()) return;
    if (g_count <= 0) return;

    if (state && state->originalFontAvailable) {
        useOriginalFont = 1;
        lineH = 7; /* DM1 font is roughly 5 wide x 6 tall + spacing */
    } else {
        lineH = 6;
    }

    maxLines = M11_QolRuntime_GetCombatLogMaxLines();
    if (maxLines < 1) maxLines = 1;
    /* Show at most ~5 lines on a 200px FB so we do not clobber the
     * status bar at the very bottom. */
    displayLines = 5;
    if (displayLines > maxLines) displayLines = maxLines;
    if (displayLines > g_count) displayLines = g_count;

    barH = displayLines * lineH + 4;
    if (barH > fbH / 3) barH = fbH / 3;
    barY = fbH - barH - 8; /* leave 8px gutter above bottom */
    if (barY < 0) barY = 0;

    /* Dithered half-transparent background: write color 0 (black) on
     * every other pixel — cheap fake transparency on the indexed buffer. */
    {
        int x, y;
        for (y = barY; y < barY + barH && y < fbH; ++y) {
            for (x = 0; x < fbW; ++x) {
                if (((x + y) & 1) == 0) {
                    fb[y * fbW + x] = 0;
                }
            }
        }
    }

    /* Print the most recent displayLines entries, bottom = newest. */
    oldestIdx = g_writeIdx - displayLines;
    while (oldestIdx < 0) oldestIdx += COMBAT_LOG_RING_CAP;
    currentDrawIdx = oldestIdx;
    for (i = 0; i < displayLines; ++i) {
        int slot = (currentDrawIdx + i) % COMBAT_LOG_RING_CAP;
        const M11_CombatLogEntry* e = &g_ring[slot];
        int ly = barY + 2 + i * lineH;
        unsigned char color = fontFg;
        switch (e->type) {
            case M11_COMBAT_LOG_TYPE_CHAMP_HIT:    color = 14; break; /* yellow */
            case M11_COMBAT_LOG_TYPE_CREATURE_HIT: color = 12; break; /* red */
            case M11_COMBAT_LOG_TYPE_SPELL:        color = 11; break; /* cyan */
            case M11_COMBAT_LOG_TYPE_MISS:         color = 8;  break; /* gray */
            default:                               color = fontFg; break;
        }
        if (useOriginalFont) {
            M11_Font_DrawString(&state->originalFont, fb, fbW, fbH,
                                2, ly, e->text, color, -1, 1);
        } else {
            (void)fontBg;
            draw_text_minifont(fb, fbW, fbH, 2, ly, e->text, color);
        }
    }
}
