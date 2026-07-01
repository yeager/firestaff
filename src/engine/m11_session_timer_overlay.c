/*
 * m11_session_timer_overlay.c
 *
 * Runtime follow-up to the launcher session timer setting. See
 * include/m11_session_timer_overlay.h for the full contract.
 *
 * Source-lock (ReDMCSB):
 *   - Per-second cadence: derived from ReDMCSB COMMAND.C F0610_TICK_Process
 *     which advances the in-game clock once per accepted game tick; we
 *     drive it from M11_GameView_AdvanceIdleTick rather than the V1
 *     game-tick pipeline because the launcher session timer is a
 *     wall-clock safety feature (not a gameplay clock) and must keep
 *     ticking even when the player is paused.
 *   - Forced-pause boundary mirrors ReDMCSB CLIKMENU.C F0387 (menu
 *     mode blocks gameplay input) without copying any copyrighted
 *     source verbatim. The dismiss/return path is documented in
 *     ReDMCSB MENU.C F0388 (clear-acting-champion); we use the same
 *     "explicit user gesture required" semantics for dismissing the
 *     forced-pause overlay.
 *
 * Determinism:
 *   - Integer arithmetic only; no float.
 *   - State transitions are driven entirely by `elapsedSeconds`, so a
 *     test that ticks 60 times in a row gets the same result as a
 *     single Tick(overlay, 60) call.
 *
 * No game data required.
 */

#include "m11_session_timer_overlay.h"

#include <stdio.h>
#include <string.h>

/* Default grace window between the reminder banner and the forced
 * pause. Matches the 60-second cap used by the M12 settings row so the
 * pause boundary aligns with the "60 min" UI hint. */
#define M11_SESSION_TIMER_DEFAULT_GRACE_SECONDS 60

enum {
    M11_SESSION_TIMER_MIN_GRACE_SECONDS = 0,
    M11_SESSION_TIMER_MAX_GRACE_SECONDS = 3600
};

static int m11_session_timer_clamp_grace(int graceSeconds) {
    if (graceSeconds < M11_SESSION_TIMER_MIN_GRACE_SECONDS) {
        return M11_SESSION_TIMER_DEFAULT_GRACE_SECONDS;
    }
    if (graceSeconds > M11_SESSION_TIMER_MAX_GRACE_SECONDS) {
        return M11_SESSION_TIMER_MAX_GRACE_SECONDS;
    }
    return graceSeconds;
}

void M11_SessionTimerOverlay_Init(M11_SessionTimerOverlay* overlay) {
    if (!overlay) {
        return;
    }
    memset(overlay, 0, sizeof(*overlay));
    overlay->state = M11_SESSION_TIMER_STATE_DISABLED;
}

void M11_SessionTimerOverlay_Configure(M11_SessionTimerOverlay* overlay,
                                       int limitMinutes,
                                       int graceSeconds) {
    if (!overlay) {
        return;
    }
    overlay->elapsedSeconds = 0;
    overlay->forcedPauseCount = 0;
    overlay->dismissCount = 0;
    if (limitMinutes <= 0) {
        overlay->limitSeconds = 0;
        overlay->graceSeconds = 0;
        overlay->forcedPauseAtSeconds = 0;
        overlay->state = M11_SESSION_TIMER_STATE_DISABLED;
        return;
    }
    overlay->limitSeconds = limitMinutes * 60;
    overlay->graceSeconds = m11_session_timer_clamp_grace(graceSeconds);
    overlay->forcedPauseAtSeconds = overlay->limitSeconds + overlay->graceSeconds;
    overlay->state = M11_SESSION_TIMER_STATE_RUNNING;
}

void M11_SessionTimerOverlay_Tick(M11_SessionTimerOverlay* overlay,
                                  int seconds) {
    if (!overlay || seconds <= 0) {
        return;
    }
    overlay->elapsedSeconds += seconds;
    /* Cap to a sane upper bound so a runaway probe cannot overflow
     * the elapsed counter. 24 hours is well past any reasonable
     * 120-min limit + 60-sec grace. */
    if (overlay->elapsedSeconds > 24 * 60 * 60) {
        overlay->elapsedSeconds = 24 * 60 * 60;
    }
    if (overlay->state == M11_SESSION_TIMER_STATE_DISABLED) {
        /* Even with a disabled overlay we record elapsed time so a
         * probe that calls Tick before Configure can still assert
         * the elapsed counter shape. Limit is 0 so the state stays
         * DISABLED forever. */
        return;
    }
    switch (overlay->state) {
    case M11_SESSION_TIMER_STATE_RUNNING:
        if (overlay->elapsedSeconds >= overlay->limitSeconds) {
            overlay->state = M11_SESSION_TIMER_STATE_REMINDER;
        }
        /* fallthrough intentional: state can also escalate from
         * RUNNING straight to FORCED_PAUSE when a large tick
         * jumps over both thresholds in one step. */
        /* FALLTHROUGH */
    case M11_SESSION_TIMER_STATE_REMINDER:
    case M11_SESSION_TIMER_STATE_DISMISSED:
        if (overlay->elapsedSeconds >= overlay->forcedPauseAtSeconds) {
            overlay->state = M11_SESSION_TIMER_STATE_FORCED_PAUSE;
            overlay->forcedPauseCount += 1;
        }
        break;
    case M11_SESSION_TIMER_STATE_FORCED_PAUSE:
        /* Tick keeps counting but state stays FORCED_PAUSE until
         * the player dismisses. */
        break;
    default:
        break;
    }
}

int M11_SessionTimerOverlay_Dismiss(M11_SessionTimerOverlay* overlay) {
    if (!overlay) {
        return 0;
    }
    if (overlay->state != M11_SESSION_TIMER_STATE_FORCED_PAUSE) {
        return 0;
    }
    overlay->state = M11_SESSION_TIMER_STATE_DISMISSED;
    overlay->dismissCount += 1;
    return 1;
}

/* ── Accessors ─────────────────────────────────────────────────────── */

M11_SessionTimerState M11_SessionTimerOverlay_GetState(
    const M11_SessionTimerOverlay* overlay) {
    return overlay ? overlay->state : M11_SESSION_TIMER_STATE_DISABLED;
}

int M11_SessionTimerOverlay_IsActive(const M11_SessionTimerOverlay* overlay) {
    if (!overlay) {
        return 0;
    }
    return overlay->state == M11_SESSION_TIMER_STATE_REMINDER ||
           overlay->state == M11_SESSION_TIMER_STATE_FORCED_PAUSE ||
           overlay->state == M11_SESSION_TIMER_STATE_DISMISSED;
}

int M11_SessionTimerOverlay_BlocksGameplayInput(
    const M11_SessionTimerOverlay* overlay) {
    if (!overlay) {
        return 0;
    }
    return overlay->state == M11_SESSION_TIMER_STATE_FORCED_PAUSE;
}

int M11_SessionTimerOverlay_GetRemainingSeconds(
    const M11_SessionTimerOverlay* overlay) {
    int remaining;
    if (!overlay || overlay->limitSeconds <= 0) {
        return -1;
    }
    remaining = overlay->limitSeconds - overlay->elapsedSeconds;
    if (remaining < 0) {
        return 0;
    }
    return remaining;
}

int M11_SessionTimerOverlay_GetLimitSeconds(
    const M11_SessionTimerOverlay* overlay) {
    return overlay ? overlay->limitSeconds : 0;
}

int M11_SessionTimerOverlay_GetElapsedSeconds(
    const M11_SessionTimerOverlay* overlay) {
    return overlay ? overlay->elapsedSeconds : 0;
}

int M11_SessionTimerOverlay_GetForcedPauseCount(
    const M11_SessionTimerOverlay* overlay) {
    return overlay ? overlay->forcedPauseCount : 0;
}

int M11_SessionTimerOverlay_GetDismissCount(
    const M11_SessionTimerOverlay* overlay) {
    return overlay ? overlay->dismissCount : 0;
}

/* ── Banner text ───────────────────────────────────────────────────── */

static void m11_session_timer_format_hhmmss(int seconds, char* out, int outSize) {
    int h, m, s;
    if (!out || outSize <= 0) {
        return;
    }
    if (seconds < 0) {
        seconds = 0;
    }
    h = seconds / 3600;
    m = (seconds / 60) % 60;
    s = seconds % 60;
    snprintf(out, (size_t)outSize, "%02d:%02d:%02d", h, m, s);
}

int M11_SessionTimerOverlay_Format(const M11_SessionTimerOverlay* overlay,
                                   char* out,
                                   int outSize) {
    int kind = M11_SESSION_TIMER_BANNER_NONE;
    if (!out || outSize <= 0) {
        return M11_SESSION_TIMER_BANNER_NONE;
    }
    out[0] = '\0';
    if (!overlay) {
        return M11_SESSION_TIMER_BANNER_NONE;
    }
    switch (overlay->state) {
    case M11_SESSION_TIMER_STATE_REMINDER:
    case M11_SESSION_TIMER_STATE_DISMISSED: {
        char buf[16];
        /* Banner shows time until the forced-pause boundary, which
         * is the player's actual deadline. This is
         *   (limit + grace) - elapsed
         * so a player who just hit REMINDER at T+limit still sees
         * the grace window in the banner (e.g. 60s grace -> "00:01:00
         * LEFT"), not 0. */
        int toGo = overlay->forcedPauseAtSeconds - overlay->elapsedSeconds;
        if (toGo < 0) {
            toGo = 0;
        }
        m11_session_timer_format_hhmmss(toGo, buf, (int)sizeof(buf));
        snprintf(out, (size_t)outSize,
                 "SESSION TIMER %s LEFT", buf);
        kind = M11_SESSION_TIMER_BANNER_REMINDER;
        break;
    }
    case M11_SESSION_TIMER_STATE_FORCED_PAUSE:
        snprintf(out, (size_t)outSize,
                 "SESSION EXPIRED - PRESS ESC TO EXIT");
        kind = M11_SESSION_TIMER_BANNER_FORCED_PAUSE;
        break;
    case M11_SESSION_TIMER_STATE_DISABLED:
    case M11_SESSION_TIMER_STATE_RUNNING:
    default:
        kind = M11_SESSION_TIMER_BANNER_NONE;
        out[0] = '\0';
        break;
    }
    return kind;
}

/* ── Runtime draw ──────────────────────────────────────────────────── */

/* Local minimal text draw so the overlay module can render its banner
 * without pulling in the full m11_draw_text helper (which requires the
 * V1 font loader to be active). Each glyph is 5 wide + 1 tracking
 * pixel = 6 px advance.
 *
 * Framebuffer layout: indexed 8-bit, one byte per pixel. This matches
 * the M11 game view framebuffer and the firestaff_accessibility
 * framebuffer used by the existing in-game UI panels.
 *
 * The draw is intentionally minimal: it paints a single-row banner at
 * y = 4 (near the top of the 200-row V1 canvas) for REMINDER and a
 * centered two-line message for FORCED_PAUSE. This avoids overlapping
 * the viewport and the bottom panel where the existing chrome lives.
 */

#define M11_SESSION_TIMER_GLYPH_W 5
#define M11_SESSION_TIMER_GLYPH_H 7
#define M11_SESSION_TIMER_GLYPH_ADVANCE 6
#define M11_SESSION_TIMER_GLYPH_ROWS 7

/* Minimal 5x7 glyph table covering the characters we need for the
 * banner text: A-Z, 0-9, space, dash, colon, and the few punctuation
 * marks used in the forced-pause message. Characters outside this set
 * render as a blank (space) so the banner never overflows the buffer.
 *
 * Each glyph is encoded as 7 bytes; each byte is the column bitmask
 * for that row, MSB on the left. This matches the V1 chrome text
 * orientation used by m11_draw_text_original / M11_Font.
 */
static const unsigned char m11_session_timer_font[26 + 10][7] = {
    /* A-Z (26 glyphs) */
    {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}, /* A */
    {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E}, /* B */
    {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E}, /* C */
    {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E}, /* D */
    {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F}, /* E */
    {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10}, /* F */
    {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0E}, /* G */
    {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}, /* H */
    {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E}, /* I */
    {0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C}, /* J */
    {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11}, /* K */
    {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F}, /* L */
    {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11}, /* M */
    {0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11}, /* N */
    {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}, /* O */
    {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10}, /* P */
    {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D}, /* Q */
    {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11}, /* R */
    {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E}, /* S */
    {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}, /* T */
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}, /* U */
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04}, /* V */
    {0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A}, /* W */
    {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11}, /* X */
    {0x11, 0x11, 0x11, 0x0A, 0x04, 0x04, 0x04}, /* Y */
    {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F}, /* Z */
    /* 0-9 (10 glyphs) */
    {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E}, /* 0 */
    {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E}, /* 1 */
    {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F}, /* 2 */
    {0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E}, /* 3 */
    {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}, /* 4 */
    {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E}, /* 5 */
    {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E}, /* 6 */
    {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08}, /* 7 */
    {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}, /* 8 */
    {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C}, /* 9 */
};

/* Look up a glyph row for an ASCII character. Returns 0 (blank row)
 * for unknown characters so unknown text never overflows. */
static unsigned char m11_session_timer_glyph_row(char ch, int row) {
    unsigned char out = 0;
    if (row < 0 || row >= M11_SESSION_TIMER_GLYPH_ROWS) {
        return 0;
    }
    if (ch >= 'A' && ch <= 'Z') {
        out = m11_session_timer_font[ch - 'A'][row];
    } else if (ch >= '0' && ch <= '9') {
        out = m11_session_timer_font[26 + (ch - '0')][row];
    } else if (ch == '-') {
        /* Centered dash for SESSION-EXPIRED */
        static const unsigned char dash[7] = {0, 0, 0, 0x1F, 0, 0, 0};
        out = dash[row];
    } else if (ch == ':') {
        static const unsigned char colon[7] = {0, 0x0E, 0x0E, 0, 0x0E, 0x0E, 0};
        out = colon[row];
    } else if (ch == ' ') {
        out = 0;
    }
    return out;
}

static void m11_session_timer_paint_glyph(unsigned char* fb, int fbW, int fbH,
                                          int x, int y, char ch,
                                          unsigned char color) {
    int row;
    if (!fb || fbW <= 0 || fbH <= 0) {
        return;
    }
    if (x < 0 || y < 0) {
        return;
    }
    for (row = 0; row < M11_SESSION_TIMER_GLYPH_H; ++row) {
        unsigned char bits = m11_session_timer_glyph_row(ch, row);
        int col;
        for (col = 0; col < M11_SESSION_TIMER_GLYPH_W; ++col) {
            int px = x + col;
            int py = y + row;
            if (px < 0 || px >= fbW || py < 0 || py >= fbH) {
                continue;
            }
            if ((bits >> (M11_SESSION_TIMER_GLYPH_W - 1 - col)) & 0x1) {
                fb[py * fbW + px] = color;
            }
        }
    }
}

static int m11_session_timer_paint_string(unsigned char* fb, int fbW, int fbH,
                                          int x, int y,
                                          const char* text,
                                          unsigned char color) {
    int drawn = 0;
    if (!text) {
        return 0;
    }
    while (*text) {
        char ch = *text;
        /* Uppercase to keep the glyph table compact. */
        if (ch >= 'a' && ch <= 'z') {
            ch = (char)(ch - 'a' + 'A');
        }
        m11_session_timer_paint_glyph(fb, fbW, fbH, x, y, ch, color);
        x += M11_SESSION_TIMER_GLYPH_ADVANCE;
        ++text;
        ++drawn;
    }
    return drawn;
}

static void m11_session_timer_paint_box(unsigned char* fb, int fbW, int fbH,
                                       int x, int y, int w, int h,
                                       unsigned char color) {
    int i;
    if (!fb || fbW <= 0 || fbH <= 0 || w <= 0 || h <= 0) {
        return;
    }
    if (x < 0) {
        w += x;
        x = 0;
    }
    if (y < 0) {
        h += y;
        y = 0;
    }
    if (x + w > fbW) {
        w = fbW - x;
    }
    if (y + h > fbH) {
        h = fbH - y;
    }
    if (w <= 0 || h <= 0) {
        return;
    }
    /* Top + bottom border. */
    for (i = 0; i < w; ++i) {
        if (y >= 0 && y < fbH) {
            fb[y * fbW + (x + i)] = color;
        }
        if (y + h - 1 >= 0 && y + h - 1 < fbH) {
            fb[(y + h - 1) * fbW + (x + i)] = color;
        }
    }
    /* Left + right border. */
    for (i = 0; i < h; ++i) {
        if (x >= 0 && x < fbW) {
            fb[(y + i) * fbW + x] = color;
        }
        if (x + w - 1 >= 0 && x + w - 1 < fbW) {
            fb[(y + i) * fbW + (x + w - 1)] = color;
        }
    }
}

int M11_SessionTimerOverlay_Draw(const void* state,
                                 const M11_SessionTimerOverlay* overlay,
                                 unsigned char* framebuffer,
                                 int framebufferWidth,
                                 int framebufferHeight) {
    char banner[M11_SESSION_TIMER_OVERLAY_TEXT_CAPACITY];
    int kind;
    int drawn = 0;
    (void)state;
    if (!overlay || !framebuffer ||
        framebufferWidth <= 0 || framebufferHeight <= 0) {
        return 0;
    }
    if (overlay->state == M11_SESSION_TIMER_STATE_DISABLED ||
        overlay->state == M11_SESSION_TIMER_STATE_RUNNING) {
        return 0;
    }
    kind = M11_SessionTimerOverlay_Format(overlay, banner, (int)sizeof(banner));
    if (kind == M11_SESSION_TIMER_BANNER_NONE || banner[0] == '\0') {
        return 0;
    }
    if (kind == M11_SESSION_TIMER_BANNER_REMINDER) {
        /* Single-row banner at the top of the canvas. Background
         * stripe so the text stays readable over arbitrary viewport
         * content. */
        int y = 4;
        int textLen = (int)strlen(banner);
        int textW = textLen * M11_SESSION_TIMER_GLYPH_ADVANCE;
        int x = (framebufferWidth - textW) / 2;
        if (x < 1) x = 1;
        /* 1px-tall border stripe to anchor the banner visually. */
        m11_session_timer_paint_box(framebuffer, framebufferWidth, framebufferHeight,
                                    x - 4, y - 2,
                                    textW + 8,
                                    M11_SESSION_TIMER_GLYPH_H + 4,
                                    /* border */ 0);
        /* Re-paint the text on top so the box outline does not eat
         * the glyph pixels. */
        drawn = m11_session_timer_paint_string(framebuffer, framebufferWidth,
                                               framebufferHeight,
                                               x, y, banner, /* color */ 11);
    } else if (kind == M11_SESSION_TIMER_BANNER_FORCED_PAUSE) {
        /* Centered two-line overlay so the player cannot miss it.
         * Use a thick border box to dim the underlying viewport. */
        int boxW = framebufferWidth - 40;
        int boxH = 36;
        int boxX = 20;
        int boxY = (framebufferHeight - boxH) / 2;
        int textW;
        int textX;
        m11_session_timer_paint_box(framebuffer, framebufferWidth, framebufferHeight,
                                    boxX, boxY, boxW, boxH, /* border */ 0);
        /* Title line */
        textW = (int)strlen("SESSION EXPIRED") * M11_SESSION_TIMER_GLYPH_ADVANCE;
        textX = boxX + (boxW - textW) / 2;
        if (textX < boxX + 2) textX = boxX + 2;
        m11_session_timer_paint_string(framebuffer, framebufferWidth,
                                       framebufferHeight,
                                       textX, boxY + 6,
                                       "SESSION EXPIRED",
                                       /* color */ 11);
        /* Body line */
        textW = (int)strlen("PRESS ESC TO EXIT") * M11_SESSION_TIMER_GLYPH_ADVANCE;
        textX = boxX + (boxW - textW) / 2;
        if (textX < boxX + 2) textX = boxX + 2;
        drawn = m11_session_timer_paint_string(framebuffer, framebufferWidth,
                                               framebufferHeight,
                                               textX, boxY + 22,
                                               "PRESS ESC TO EXIT",
                                               /* color */ 15);
    }
    return drawn;
}
