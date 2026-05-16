/*
 * dm1_v1_text_message_pc34_compat.c — DM1 V1 Message/Text Display System.
 *
 * Source-locked to ReDMCSB TEXT.C for the PC (F20E) media variant.
 *
 * ReDMCSB source audit:
 *
 *   TEXT.C F0045_TEXT_MESSAGEAREA_CreateNewRow:
 *     if cursorRow == ROW_COUNT-1:
 *       if scrollPending: F0696 (scroll up); clear bitmap
 *       scrollPending = true
 *       shift expirations up
 *     else: cursorRow++; scrollPending = false
 *
 *   TEXT.C F0047 (F20E):
 *     At start: if scrollPending: F0696, clear, scrollPending=false
 *     Word-wrap loop with F0646/PrintString/CreateNewRow
 *     At end: if scrollPending: F0696, clear, scrollPending=false
 *
 * Our text-buffer model maps scrollPending to:
 *   "The current bottom row text is written to the offscreen bitmap
 *    and has NOT yet been committed to the visible row array."
 *   Flushing means: shift all rows up, put the offscreen content
 *   into the new bottom position, clear the offscreen.
 *
 * Q3.6+Opus source-locked implementation.
 */

#include "dm1_v1_text_message_pc34_compat.h"
#include <string.h>
#include <stdio.h>

static int dm1_v1_text_copy_scroll_line(char* dst, int dstSize,
                                        const char* start, int len) {
    if (!dst || dstSize <= 0) return 0;
    if (!start || len < 0) len = 0;
    if (len >= dstSize) len = dstSize - 1;
    if (len > 0) memcpy(dst, start, (size_t)len);
    dst[len] = '\0';
    return len;
}

/* ── Internal: shift visible rows up by one ─────────────────────────── */
static void dm1_v1_text_shift_rows_up(DM1_V1_TextMessageState* state) {
    int i;
    for (i = 0; i < DM1_V1_MESSAGE_AREA_ROW_COUNT - 1; i++) {
        state->rows[i] = state->rows[i + 1];
    }
    memset(state->rows[DM1_V1_MESSAGE_AREA_ROW_COUNT - 1].text, 0,
           DM1_V1_MESSAGE_MAX_LENGTH);
    state->rows[DM1_V1_MESSAGE_AREA_ROW_COUNT - 1].color = 0;
    state->rows[DM1_V1_MESSAGE_AREA_ROW_COUNT - 1].expirationTime = -1;
}

/* ── Internal: F0045 CreateNewRow ───────────────────────────────────── */
/*
 * When at bottom row: make room for new text.
 * If there's already pending content (scrollPending), shift everything
 * up first. Then mark that the current bottom row is "pending" (not yet
 * scrolled to its final position).
 *
 * Effectively: scrollPending means "bottom row has content that needs
 * to be visible but hasn't been shifted into the stable area yet."
 * When CreateNewRow is called again while scrollPending, the previous
 * content gets committed (shifted up) and new space is made.
 */
static void dm1_v1_text_create_new_row(DM1_V1_TextMessageState* state) {
    if (state->cursorRow == DM1_V1_MESSAGE_AREA_ROW_COUNT - 1) {
        /* Always shift rows up when at bottom to make room.
         * This handles both the "scrollPending was set" case (content
         * from previous write) and the fresh case. */
        dm1_v1_text_shift_rows_up(state);

        /* Mark scroll as committed — the row content is now in place */
        state->scrollPending = 0;
    } else {
        state->cursorRow++;
        state->scrollPending = 0;
    }
}

/* ── Internal: F0046 PrintString ────────────────────────────────────── */
static void dm1_v1_text_print_string(DM1_V1_TextMessageState* state,
                                     int color, const char* text) {
    DM1_V1_MessageRow* row;
    size_t existingLen, addLen, maxLen;

    if (!text || !text[0]) return;

    row = &state->rows[state->cursorRow];
    existingLen = strlen(row->text);
    addLen = strlen(text);
    maxLen = DM1_V1_MESSAGE_MAX_LENGTH - 1;

    if (existingLen + addLen < maxLen) {
        memcpy(row->text + existingLen, text, addLen + 1);
    } else if (existingLen < maxLen) {
        size_t space = maxLen - existingLen;
        memcpy(row->text + existingLen, text, space);
        row->text[maxLen] = '\0';
    }

    row->color = color;
    row->expirationTime = state->gameTime + DM1_V1_MESSAGE_ROW_EXPIRATION_TICKS;
    state->cursorColumn += (int)addLen * DM1_V1_TEXT_CHARACTER_WIDTH;
}

/* ── Public API ─────────────────────────────────────────────────────── */

void dm1_v1_text_init(DM1_V1_TextMessageState* state) {
    int i;
    if (!state) return;
    memset(state, 0, sizeof(*state));

    state->fontCharWidth = DM1_V1_TEXT_CHARACTER_WIDTH;
    state->fontCharHeight = 6;
    state->fontLineHeight = DM1_V1_TEXT_LINE_HEIGHT;
    state->messageAreaWidth = DM1_V1_MESSAGE_AREA_WIDTH;

    state->cursorRow = DM1_V1_MESSAGE_AREA_ROW_COUNT - 1;
    state->cursorColumn = 0;
    state->scrollPending = 0;
    state->gameTime = 0;

    for (i = 0; i < DM1_V1_MESSAGE_AREA_ROW_COUNT; i++) {
        state->rows[i].expirationTime = -1;
    }

    state->scrollText.active = 0;

    for (i = 0; i < DM1_V1_MAX_DAMAGE_INDICATORS; i++) {
        state->damageIndicators[i].active = 0;
    }
}

void dm1_v1_text_move_cursor(DM1_V1_TextMessageState* state,
                             int column, int row) {
    int maxCol;
    if (!state) return;

    if (column < 0) column = 0;
    maxCol = state->messageAreaWidth - DM1_V1_TEXT_CHARACTER_WIDTH;
    state->cursorColumn = column * DM1_V1_TEXT_CHARACTER_WIDTH;
    if (state->cursorColumn > maxCol) {
        state->cursorColumn = maxCol;
    }

    if (row < 0) row = 0;
    if (row >= DM1_V1_MESSAGE_AREA_ROW_COUNT) {
        row = DM1_V1_MESSAGE_AREA_ROW_COUNT - 1;
    }
    state->cursorRow = row;
    state->scrollPending = 0;
}

void dm1_v1_text_clear_all_rows(DM1_V1_TextMessageState* state) {
    int i;
    if (!state) return;

    for (i = 0; i < DM1_V1_MESSAGE_AREA_ROW_COUNT; i++) {
        memset(state->rows[i].text, 0, DM1_V1_MESSAGE_MAX_LENGTH);
        state->rows[i].color = 0;
        state->rows[i].expirationTime = -1;
    }

    dm1_v1_text_move_cursor(state, 0, DM1_V1_MESSAGE_AREA_ROW_COUNT - 1);
}

void dm1_v1_text_clear_expired_rows(DM1_V1_TextMessageState* state) {
    int i;
    if (!state) return;

    for (i = 0; i < DM1_V1_MESSAGE_AREA_ROW_COUNT; i++) {
        long exp = state->rows[i].expirationTime;
        if (exp != -1 && exp <= state->gameTime) {
            memset(state->rows[i].text, 0, DM1_V1_MESSAGE_MAX_LENGTH);
            state->rows[i].color = 0;
            state->rows[i].expirationTime = -1;
        }
    }
}

void dm1_v1_text_print_message(DM1_V1_TextMessageState* state,
                               int color, const char* text) {
    char word[DM1_V1_MESSAGE_MAX_LENGTH];
    int wordIdx;
    int maxCol;

    if (!state || !text) return;

    maxCol = DM1_V1_MESSAGE_MAX_CHARS_PER_ROW;

    while (*text) {
        if (*text == '\n') {
            text++;
            if (state->cursorColumn != 0 || state->cursorRow != 0) {
                state->cursorColumn = 0;
                dm1_v1_text_create_new_row(state);
            }
        } else if (*text == ' ') {
            text++;
            if (state->cursorColumn / DM1_V1_TEXT_CHARACTER_WIDTH < maxCol) {
                dm1_v1_text_print_string(state, color, " ");
            }
        } else {
            wordIdx = 0;
            while (*text && *text != ' ' && *text != '\n') {
                if (wordIdx < DM1_V1_MESSAGE_MAX_LENGTH - 1) {
                    word[wordIdx++] = *text;
                }
                text++;
            }
            word[wordIdx] = '\0';

            if (state->cursorColumn / DM1_V1_TEXT_CHARACTER_WIDTH + wordIdx > maxCol) {
                state->cursorColumn = DM1_V1_MESSAGE_CONTINUATION_INDENT;
                dm1_v1_text_create_new_row(state);
            }

            dm1_v1_text_print_string(state, color, word);
        }
    }
}

void dm1_v1_text_print_character(DM1_V1_TextMessageState* state,
                                 int color, char ch) {
    char str[2];
    str[0] = ch;
    str[1] = '\0';
    dm1_v1_text_print_message(state, color, str);
}

void dm1_v1_text_print_linefeed(DM1_V1_TextMessageState* state) {
    dm1_v1_text_print_message(state, DM1_V1_COLOR_BLACK, "\n");
}

void dm1_v1_text_set_game_time(DM1_V1_TextMessageState* state, long gameTime) {
    if (state) state->gameTime = gameTime;
}

void dm1_v1_text_tick(DM1_V1_TextMessageState* state, int deltaTicks) {
    if (!state) return;
    state->gameTime += deltaTicks;
    dm1_v1_text_clear_expired_rows(state);
    dm1_v1_text_tick_damage(state, deltaTicks);
}

/* ── Scroll / Inscription Text ──────────────────────────────────────── */

void dm1_v1_text_show_scroll(DM1_V1_TextMessageState* state,
                             const char* text, int textType, int color) {
    if (!state || !text) return;
    strncpy(state->scrollText.text, text, DM1_V1_SCROLL_TEXT_MAX_LENGTH - 1);
    state->scrollText.text[DM1_V1_SCROLL_TEXT_MAX_LENGTH - 1] = '\0';
    state->scrollText.textType = textType;
    state->scrollText.color = color;
    state->scrollText.active = 1;
}

void dm1_v1_text_clear_scroll(DM1_V1_TextMessageState* state) {
    if (!state) return;
    state->scrollText.active = 0;
    state->scrollText.text[0] = '\0';
}

int dm1_v1_text_scroll_active(const DM1_V1_TextMessageState* state) {
    return state ? state->scrollText.active : 0;
}

int dm1_v1_text_scroll_encode_line(const char* src, unsigned char* dst,
                                   int dstSize) {
    int count = 0;
    if (!src || !dst || dstSize <= 0) return 0;

    /* Source lock: ReDMCSB F0340_INVENTORY_DrawPanel_ScrollTextLine and
     * CSBWin DisplayScrollText_OneLine remap A-Z to scroll glyphs by
     * subtracting 64, and bytes >= '{' by subtracting 96, before drawing. */
    while (*src && count < dstSize - 1) {
        unsigned char ch = (unsigned char)*src++;
        if (ch >= 'A' && ch <= 'Z') ch = (unsigned char)(ch - 64);
        else if (ch >= '{') ch = (unsigned char)(ch - 96);
        dst[count++] = ch;
    }
    dst[count] = 0;
    return count;
}

int dm1_v1_text_scroll_measure_layout(const char* text,
                                      DM1_V1_ScrollLayout* outLayout) {
    const char* p;
    const char* lineStart;
    int lineCount;
    int stored;

    if (!text) text = "";
    if (outLayout) memset(outLayout, 0, sizeof(*outLayout));

    /* Source lock: ReDMCSB F0341_INVENTORY_DrawPanel_Scroll splits the
     * decoded scroll text at newlines, counts one first line plus each later
     * newline, adds a final line when the text does not end in newline, and
     * subtracts one for a double trailing newline. Unlike CSBWin DisplayScroll,
     * the original DM1 path intentionally does not clamp long custom scrolls. */
    p = text;
    while (*p && *p != '\n') p++;
    lineCount = 1;
    if (*p == '\n') {
        const char* tail = p + 1;
        const char* end = tail;
        for (; *end; end++) {
            if (*end == '\n') lineCount++;
        }
        if (end > tail) {
            if (*(end - 1) != '\n') lineCount++;
            else if (end - tail >= 2 && *(end - 2) == '\n') lineCount--;
        }
    }

    if (!outLayout) return lineCount;
    outLayout->lineCount = lineCount;
    outLayout->firstLineY = DM1_V1_SCROLL_TEXT_CENTER_Y -
        ((DM1_V1_TEXT_LINE_HEIGHT * lineCount) / 2);
    outLayout->overflow = lineCount > DM1_V1_SCROLL_LAYOUT_MAX_LINES;

    lineStart = text;
    stored = 0;
    while (stored < DM1_V1_SCROLL_LAYOUT_MAX_LINES && stored < lineCount) {
        const char* lineEnd = lineStart;
        while (*lineEnd && *lineEnd != '\n') lineEnd++;
        dm1_v1_text_copy_scroll_line(outLayout->lines[stored],
                                     DM1_V1_MESSAGE_MAX_LENGTH,
                                     lineStart, (int)(lineEnd - lineStart));
        stored++;
        if (!*lineEnd) break;
        lineStart = lineEnd + 1;
    }
    outLayout->storedLineCount = stored;
    return lineCount;
}


/* ── Damage Indicators ──────────────────────────────────────────────── */

void dm1_v1_text_show_damage(DM1_V1_TextMessageState* state,
                             int championIndex, int damageValue, int big) {
    int i;
    if (!state) return;
    if (championIndex < 0 || championIndex >= DM1_V1_MAX_DAMAGE_INDICATORS) return;

    for (i = 0; i < DM1_V1_MAX_DAMAGE_INDICATORS; i++) {
        if (state->damageIndicators[i].active &&
            state->damageIndicators[i].championIndex == championIndex) {
            state->damageIndicators[i].value = damageValue;
            state->damageIndicators[i].ticksRemaining = DM1_V1_DAMAGE_DISPLAY_TICKS;
            state->damageIndicators[i].big = big;
            return;
        }
    }
    for (i = 0; i < DM1_V1_MAX_DAMAGE_INDICATORS; i++) {
        if (!state->damageIndicators[i].active) {
            state->damageIndicators[i].active = 1;
            state->damageIndicators[i].championIndex = championIndex;
            state->damageIndicators[i].value = damageValue;
            state->damageIndicators[i].ticksRemaining = DM1_V1_DAMAGE_DISPLAY_TICKS;
            state->damageIndicators[i].big = big;
            return;
        }
    }
}

void dm1_v1_text_tick_damage(DM1_V1_TextMessageState* state, int deltaTicks) {
    int i;
    if (!state) return;
    for (i = 0; i < DM1_V1_MAX_DAMAGE_INDICATORS; i++) {
        if (state->damageIndicators[i].active) {
            state->damageIndicators[i].ticksRemaining -= deltaTicks;
            if (state->damageIndicators[i].ticksRemaining <= 0) {
                state->damageIndicators[i].active = 0;
            }
        }
    }
}

int dm1_v1_text_get_damage_count(const DM1_V1_TextMessageState* state) {
    int i, count = 0;
    if (!state) return 0;
    for (i = 0; i < DM1_V1_MAX_DAMAGE_INDICATORS; i++) {
        if (state->damageIndicators[i].active) count++;
    }
    return count;
}

const DM1_V1_DamageIndicator* dm1_v1_text_get_damage(
    const DM1_V1_TextMessageState* state, int index) {
    if (!state || index < 0 || index >= DM1_V1_MAX_DAMAGE_INDICATORS) return NULL;
    if (!state->damageIndicators[index].active) return NULL;
    return &state->damageIndicators[index];
}

/* ── Read-only access ───────────────────────────────────────────────── */

const DM1_V1_MessageRow* dm1_v1_text_get_row(
    const DM1_V1_TextMessageState* state, int rowIndex) {
    if (!state || rowIndex < 0 || rowIndex >= DM1_V1_MESSAGE_AREA_ROW_COUNT) return NULL;
    return &state->rows[rowIndex];
}

void dm1_v1_text_get_cursor(const DM1_V1_TextMessageState* state,
                            int* outColumn, int* outRow) {
    if (!state) return;
    if (outColumn) *outColumn = state->cursorColumn;
    if (outRow) *outRow = state->cursorRow;
}

int dm1_v1_text_get_active_row_count(const DM1_V1_TextMessageState* state) {
    int i, count = 0;
    if (!state) return 0;
    for (i = 0; i < DM1_V1_MESSAGE_AREA_ROW_COUNT; i++) {
        if (state->rows[i].text[0] != '\0') count++;
    }
    return count;
}

/* ══════════════════════════════════════════════════════════════════════
 * Legacy M11_TextState compatibility shim
 * ══════════════════════════════════════════════════════════════════════ */

void m11_text_init(M11_TextState* s) {
    if (!s) return;
    memset(s, 0, sizeof(*s));
    s->fontWidth = 8;
    s->fontHeight = 8;
}

void m11_text_show(M11_TextState* s, const char* text,
                   int x, int y, int color, int priority) {
    int idx = -1;
    if (!s || !text) return;

    if (s->messageCount < M11_TEXT_MAX_MESSAGES) {
        idx = s->messageCount;
    } else {
        int lowest = 0x7FFFFFFF;
        int i;
        for (i = 0; i < M11_TEXT_MAX_MESSAGES; i++) {
            if (s->messages[i].priority < lowest) {
                lowest = s->messages[i].priority;
                idx = i;
            }
        }
    }
    if (idx < 0) return;

    strncpy(s->messages[idx].text, text, M11_TEXT_MAX_LENGTH - 1);
    s->messages[idx].text[M11_TEXT_MAX_LENGTH - 1] = '\0';
    s->messages[idx].ticksRemaining = M11_TEXT_DISPLAY_TICKS;
    s->messages[idx].x = x;
    s->messages[idx].y = y;
    s->messages[idx].color = color;
    s->messages[idx].priority = priority;
    if (s->messageCount < M11_TEXT_MAX_MESSAGES) s->messageCount++;
}

void m11_text_show_centered(M11_TextState* s, const char* text,
                            int y, int color) {
    int len, x;
    if (!s || !text) return;
    len = (int)strlen(text);
    x = (224 - len * s->fontWidth) / 2;
    m11_text_show(s, text, x, y, color, 0);
}

void m11_text_tick(M11_TextState* s, int tickMs) {
    int w = 0, r;
    if (!s) return;
    for (r = 0; r < s->messageCount; r++) {
        s->messages[r].ticksRemaining -= tickMs;
        if (s->messages[r].ticksRemaining > 0) {
            if (w != r) s->messages[w] = s->messages[r];
            w++;
        }
    }
    s->messageCount = w;
}

int m11_text_get_active_count(const M11_TextState* s) {
    return s ? s->messageCount : 0;
}

const M11_TextMessage* m11_text_get_message(const M11_TextState* s, int index) {
    if (!s || index < 0 || index >= s->messageCount) return NULL;
    return &s->messages[index];
}

/* ══════════════════════════════════════════════════════════════════════
 * Pass602b — TEXT.C remaining function citations
 *
 *   TEXT.C:1408 F0553_TEXT_MESSAGEAREA_M
 *   TEXT.C:1919 F0554_TEXT_MESSAGEAREA_D
 *   TEXT.C:2033 F0555_TEXT_A
 *   TEXT.C:2054 F0556_TEXT_D
 *   TEXT.C:1443 F0560_SCROLLER_S
 *   TEXT.C:1378 F0561_SCROLLER_I
 *   TEXT.C:73 F0818_I
 *   TEXT.C:154 F0952_JAPANESE_P
 *   TEXT.C:2002 F1001_JAPANESE_L
 *   TEXT.C:112 F1029_I
 * ══════════════════════════════════════════════════════════════════════ */

