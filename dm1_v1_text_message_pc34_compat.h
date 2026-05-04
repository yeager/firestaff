/*
 * dm1_v1_text_message_pc34_compat.h — DM1 V1 Message/Text Display System.
 *
 * Source-locked to ReDMCSB TEXT.C (F0040–F0054), DRAWMSGA.C (F0696),
 * FONT.C (F0642–F0643), and DEFS.H constants for the PC (F20E) media.
 *
 * Key ReDMCSB references:
 *   TEXT.C:
 *     F0042_TEXT_MESSAGEAREA_MoveCursor      — cursor position
 *     F0043_TEXT_MESSAGEAREA_ClearAllRows    — clear all + reset
 *     F0044_TEXT_MESSAGEAREA_ClearExpiredRows — time-based row removal
 *     F0045_TEXT_MESSAGEAREA_CreateNewRow     — scroll or advance
 *     F0046_TEXT_MESSAGEAREA_PrintString      — render one string segment
 *     F0047_TEXT_MESSAGEAREA_PrintMessage     — word-wrapped message
 *     F0048_TEXT_MESSAGEAREA_PrintCharacter   — single char to message area
 *     F0051_TEXT_MESSAGEAREA_PrintLineFeed    — newline in message area
 *     F0054_TEXT_Initialize                   — startup init
 *   DRAWMSGA.C:
 *     F0696_UpdateMessageArea                 — scroll pixel rows up
 *   DEFS.H:
 *     M532_MESSAGE_AREA_ROW_COUNT = 4 (PC)
 *     C015_ZONE_MESSAGE_AREA = zone 15
 *     C0_TEXT_TYPE_INSCRIPTION = 0
 *     C1_TEXT_TYPE_MESSAGE = 1
 *     C2_TEXT_TYPE_SCROLL = 2
 *     G2087_C6_TextCharacterWidth = 6
 *     G2088_C7_TextLineHeight = 7
 *     G2092_MessageAreaWidth = 320 (PC)
 *   COORD.C:
 *     Row expiration: G0313_ul_GameTime + 70 ticks
 */

#ifndef FIRESTAFF_DM1_V1_TEXT_MESSAGE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_TEXT_MESSAGE_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── ReDMCSB DEFS.H constants (PC F20E media) ──────────────────────── */

#define DM1_V1_MESSAGE_AREA_ROW_COUNT       4
#define DM1_V1_TEXT_CHARACTER_WIDTH          6
#define DM1_V1_TEXT_LINE_HEIGHT             7
#define DM1_V1_MESSAGE_AREA_WIDTH           320
#define DM1_V1_MESSAGE_MAX_CHARS_PER_ROW    53
#define DM1_V1_MESSAGE_ROW_EXPIRATION_TICKS 70
#define DM1_V1_TEXT_TYPE_INSCRIPTION        0
#define DM1_V1_TEXT_TYPE_MESSAGE            1
#define DM1_V1_TEXT_TYPE_SCROLL             2
#define DM1_V1_MESSAGE_CONTINUATION_INDENT  12
#define DM1_V1_STRING_BUILD_BUFFER_SIZE     150
#define DM1_V1_MESSAGE_MAX_LENGTH           128
#define DM1_V1_SCROLL_TEXT_MAX_LENGTH       512

#define DM1_V1_COLOR_BLACK                  0
#define DM1_V1_COLOR_WHITE                  15
#define DM1_V1_COLOR_RED                    8
#define DM1_V1_COLOR_CYAN                   4
#define DM1_V1_COLOR_DARKEST_GRAY           12

/* ── Message area row ───────────────────────────────────────────────── */

typedef struct {
    char text[DM1_V1_MESSAGE_MAX_LENGTH];
    int  color;
    long expirationTime;  /* -1 = empty/no-expire */
} DM1_V1_MessageRow;

typedef struct {
    char text[DM1_V1_SCROLL_TEXT_MAX_LENGTH];
    int  textType;
    int  active;
    int  color;
} DM1_V1_ScrollTextState;

typedef struct {
    int  active;
    int  value;
    int  championIndex;
    int  ticksRemaining;
    int  big;
} DM1_V1_DamageIndicator;

#define DM1_V1_MAX_DAMAGE_INDICATORS 4
#define DM1_V1_DAMAGE_DISPLAY_TICKS  30

typedef struct {
    DM1_V1_MessageRow rows[DM1_V1_MESSAGE_AREA_ROW_COUNT];
    int cursorRow;
    int cursorColumn;
    int scrollPending;
    long gameTime;
    DM1_V1_ScrollTextState scrollText;
    DM1_V1_DamageIndicator damageIndicators[DM1_V1_MAX_DAMAGE_INDICATORS];
    int fontCharWidth;
    int fontCharHeight;
    int fontLineHeight;
    int messageAreaWidth;
} DM1_V1_TextMessageState;

/* ── API ────────────────────────────────────────────────────────────── */

void dm1_v1_text_init(DM1_V1_TextMessageState* state);

void dm1_v1_text_move_cursor(DM1_V1_TextMessageState* state,
                             int column, int row);
void dm1_v1_text_clear_all_rows(DM1_V1_TextMessageState* state);
void dm1_v1_text_clear_expired_rows(DM1_V1_TextMessageState* state);

void dm1_v1_text_print_message(DM1_V1_TextMessageState* state,
                               int color, const char* text);
void dm1_v1_text_print_character(DM1_V1_TextMessageState* state,
                                 int color, char ch);
void dm1_v1_text_print_linefeed(DM1_V1_TextMessageState* state);

void dm1_v1_text_set_game_time(DM1_V1_TextMessageState* state, long gameTime);
void dm1_v1_text_tick(DM1_V1_TextMessageState* state, int deltaTicks);

void dm1_v1_text_show_scroll(DM1_V1_TextMessageState* state,
                             const char* text, int textType, int color);
void dm1_v1_text_clear_scroll(DM1_V1_TextMessageState* state);
int  dm1_v1_text_scroll_active(const DM1_V1_TextMessageState* state);

void dm1_v1_text_show_damage(DM1_V1_TextMessageState* state,
                             int championIndex, int damageValue, int big);
void dm1_v1_text_tick_damage(DM1_V1_TextMessageState* state, int deltaTicks);
int  dm1_v1_text_get_damage_count(const DM1_V1_TextMessageState* state);
const DM1_V1_DamageIndicator* dm1_v1_text_get_damage(
    const DM1_V1_TextMessageState* state, int index);

const DM1_V1_MessageRow* dm1_v1_text_get_row(
    const DM1_V1_TextMessageState* state, int rowIndex);
void dm1_v1_text_get_cursor(const DM1_V1_TextMessageState* state,
                            int* outColumn, int* outRow);
int  dm1_v1_text_get_active_row_count(const DM1_V1_TextMessageState* state);

/* ── Legacy compat shim (old M11_TextState API) ─────────────────────── */

#define M11_TEXT_MAX_MESSAGES 8
#define M11_TEXT_MAX_LENGTH   64
#define M11_TEXT_DISPLAY_TICKS 150

typedef struct {
    char text[M11_TEXT_MAX_LENGTH];
    int  ticksRemaining;
    int  priority;
    int  x;
    int  y;
    int  color;
} M11_TextMessage;

typedef struct {
    M11_TextMessage messages[M11_TEXT_MAX_MESSAGES];
    int messageCount;
    int fontWidth;
    int fontHeight;
} M11_TextState;

void m11_text_init(M11_TextState* s);
void m11_text_show(M11_TextState* s, const char* text,
                   int x, int y, int color, int priority);
void m11_text_show_centered(M11_TextState* s, const char* text,
                            int y, int color);
void m11_text_tick(M11_TextState* s, int tickMs);
int  m11_text_get_active_count(const M11_TextState* s);
const M11_TextMessage* m11_text_get_message(const M11_TextState* s, int index);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_TEXT_MESSAGE_PC34_COMPAT_H */
