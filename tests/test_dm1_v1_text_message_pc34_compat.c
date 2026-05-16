/*
 * test_dm1_v1_text_message_pc34_compat.c — CTest source-lock gate for
 * the DM1 V1 message/text display system.
 *
 * Verifies the text message state machine matches ReDMCSB TEXT.C behavior:
 *   - Initialization (F0054)
 *   - Cursor movement (F0042)
 *   - Row clearing (F0043, F0044)
 *   - Row creation / scrolling (F0045)
 *   - Word-wrapping (F0047)
 *   - Character printing (F0048)
 *   - Line feed (F0051)
 *   - Scroll/inscription text
 *   - Damage indicators
 *   - Row expiration timing
 */

#include "dm1_v1_text_message_pc34_compat.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int g_pass = 0;
static int g_fail = 0;

#define ASSERT_EQ(a, b, msg) do { \
    if ((a) == (b)) { g_pass++; } \
    else { g_fail++; fprintf(stderr, "FAIL: %s: expected %d, got %d\n", (msg), (int)(b), (int)(a)); } \
} while(0)

#define ASSERT_STR_EQ(a, b, msg) do { \
    if (strcmp((a), (b)) == 0) { g_pass++; } \
    else { g_fail++; fprintf(stderr, "FAIL: %s: expected '%s', got '%s'\n", (msg), (b), (a)); } \
} while(0)

#define ASSERT_NONNULL(p, msg) do { \
    if ((p) != NULL) { g_pass++; } \
    else { g_fail++; fprintf(stderr, "FAIL: %s: unexpected NULL\n", (msg)); } \
} while(0)

/* ── Test: Initialization (F0054) ───────────────────────────────────── */
static void test_init(void) {
    DM1_V1_TextMessageState state;
    int i, col, row;

    dm1_v1_text_init(&state);

    /* Cursor at (0, ROW_COUNT-1) */
    dm1_v1_text_get_cursor(&state, &col, &row);
    ASSERT_EQ(col, 0, "init cursor column");
    ASSERT_EQ(row, DM1_V1_MESSAGE_AREA_ROW_COUNT - 1, "init cursor row");

    /* All rows empty */
    for (i = 0; i < DM1_V1_MESSAGE_AREA_ROW_COUNT; i++) {
        const DM1_V1_MessageRow* r = dm1_v1_text_get_row(&state, i);
        ASSERT_NONNULL(r, "init row non-null");
        ASSERT_EQ(r->text[0], '\0', "init row empty");
        ASSERT_EQ(r->expirationTime, -1, "init row expiration");
    }

    /* No scroll pending */
    ASSERT_EQ(state.scrollPending, 0, "init no scroll pending");

    /* Font constants */
    ASSERT_EQ(state.fontCharWidth, DM1_V1_TEXT_CHARACTER_WIDTH, "init font width");
    ASSERT_EQ(state.fontLineHeight, DM1_V1_TEXT_LINE_HEIGHT, "init line height");
    ASSERT_EQ(state.messageAreaWidth, DM1_V1_MESSAGE_AREA_WIDTH, "init msg width");

    /* No damage indicators */
    ASSERT_EQ(dm1_v1_text_get_damage_count(&state), 0, "init no damage");

    /* No scroll text */
    ASSERT_EQ(dm1_v1_text_scroll_active(&state), 0, "init no scroll text");
}

/* ── Test: Cursor movement (F0042) ──────────────────────────────────── */
static void test_cursor_movement(void) {
    DM1_V1_TextMessageState state;
    int col, row;

    dm1_v1_text_init(&state);

    /* Normal move */
    dm1_v1_text_move_cursor(&state, 10, 2);
    dm1_v1_text_get_cursor(&state, &col, &row);
    ASSERT_EQ(col, 10 * DM1_V1_TEXT_CHARACTER_WIDTH, "cursor col 10");
    ASSERT_EQ(row, 2, "cursor row 2");

    /* Clamp negative */
    dm1_v1_text_move_cursor(&state, -5, -1);
    dm1_v1_text_get_cursor(&state, &col, &row);
    ASSERT_EQ(col, 0, "cursor col clamped low");
    ASSERT_EQ(row, 0, "cursor row clamped low");

    /* Clamp high: column * 6 > 320 - 6 = 314 → clamped to 314 */
    dm1_v1_text_move_cursor(&state, 100, 10);
    dm1_v1_text_get_cursor(&state, &col, &row);
    ASSERT_EQ(col, DM1_V1_MESSAGE_AREA_WIDTH - DM1_V1_TEXT_CHARACTER_WIDTH,
              "cursor col clamped high");
    ASSERT_EQ(row, DM1_V1_MESSAGE_AREA_ROW_COUNT - 1, "cursor row clamped high");

    /* Move clears scrollPending */
    state.scrollPending = 1;
    dm1_v1_text_move_cursor(&state, 0, 0);
    ASSERT_EQ(state.scrollPending, 0, "move clears scroll pending");
}

/* ── Test: Clear all rows (F0043) ───────────────────────────────────── */
static void test_clear_all_rows(void) {
    DM1_V1_TextMessageState state;
    int col, row;

    dm1_v1_text_init(&state);

    /* Print some messages */
    dm1_v1_text_print_message(&state, DM1_V1_COLOR_WHITE, "Hello world");

    /* Clear */
    dm1_v1_text_clear_all_rows(&state);

    /* Verify cursor at (0, ROW_COUNT-1) */
    dm1_v1_text_get_cursor(&state, &col, &row);
    ASSERT_EQ(col, 0, "clear cursor col");
    ASSERT_EQ(row, DM1_V1_MESSAGE_AREA_ROW_COUNT - 1, "clear cursor row");

    /* All rows empty */
    ASSERT_EQ(dm1_v1_text_get_active_row_count(&state), 0, "clear all empty");
}

/* ── Test: Print message and word wrapping (F0047) ──────────────────── */
static void test_print_message_basic(void) {
    DM1_V1_TextMessageState state;
    const DM1_V1_MessageRow* r;

    dm1_v1_text_init(&state);

    /* Print a simple message — should go to row 3 (bottom) */
    dm1_v1_text_print_message(&state, DM1_V1_COLOR_WHITE, "Hello");

    r = dm1_v1_text_get_row(&state, DM1_V1_MESSAGE_AREA_ROW_COUNT - 1);
    ASSERT_NONNULL(r, "print basic row exists");
    ASSERT_STR_EQ(r->text, "Hello", "print basic text");
    ASSERT_EQ(r->color, DM1_V1_COLOR_WHITE, "print basic color");

    /* Expiration should be set to gameTime + 70 */
    ASSERT_EQ(r->expirationTime, DM1_V1_MESSAGE_ROW_EXPIRATION_TICKS,
              "print basic expiration");
}

static void test_print_message_newline(void) {
    DM1_V1_TextMessageState state;
    const DM1_V1_MessageRow* r;

    dm1_v1_text_init(&state);

    /* Print with newline — should create a new row */
    dm1_v1_text_print_message(&state, DM1_V1_COLOR_CYAN, "Line 1\nLine 2");

    /* After newline processing, we should have text on two rows.
     * Initial cursor is at row 3. After "Line 1", cursor is on row 3.
     * '\n' creates new row: since row 3 is bottom, scroll happens.
     * "Line 2" goes to the new bottom row 3. */
    r = dm1_v1_text_get_row(&state, DM1_V1_MESSAGE_AREA_ROW_COUNT - 2);
    ASSERT_NONNULL(r, "newline upper row exists");
    /* After scroll, "Line 1" should have been shifted up */
    ASSERT_STR_EQ(r->text, "Line 1", "newline upper row text");

    r = dm1_v1_text_get_row(&state, DM1_V1_MESSAGE_AREA_ROW_COUNT - 1);
    ASSERT_NONNULL(r, "newline lower row exists");
    ASSERT_STR_EQ(r->text, "Line 2", "newline lower row text");
}

static void test_print_message_word_wrap(void) {
    DM1_V1_TextMessageState state;

    dm1_v1_text_init(&state);

    /* Print a long message that should wrap.
     * Max chars per line = 53. Build a string > 53 chars. */
    dm1_v1_text_print_message(&state, DM1_V1_COLOR_WHITE,
        "This is a very long message that should definitely wrap around to the next line below");

    /* Should have text on at least 2 rows */
    ASSERT_EQ(dm1_v1_text_get_active_row_count(&state) >= 2, 1,
              "word wrap creates multiple rows");
}

/* ── Test: Character printing (F0048) ───────────────────────────────── */
static void test_print_character(void) {
    DM1_V1_TextMessageState state;
    const DM1_V1_MessageRow* r;

    dm1_v1_text_init(&state);

    dm1_v1_text_print_character(&state, DM1_V1_COLOR_RED, 'X');

    r = dm1_v1_text_get_row(&state, DM1_V1_MESSAGE_AREA_ROW_COUNT - 1);
    ASSERT_NONNULL(r, "print char row");
    ASSERT_STR_EQ(r->text, "X", "print char text");
    ASSERT_EQ(r->color, DM1_V1_COLOR_RED, "print char color");
}

/* ── Test: Linefeed (F0051) ─────────────────────────────────────────── */
static void test_linefeed(void) {
    DM1_V1_TextMessageState state;
    int col, row;

    dm1_v1_text_init(&state);

    dm1_v1_text_print_message(&state, DM1_V1_COLOR_WHITE, "Before");
    dm1_v1_text_print_linefeed(&state);

    /* After linefeed, cursor should be at column 0 on a new row */
    dm1_v1_text_get_cursor(&state, &col, &row);
    ASSERT_EQ(col, 0, "linefeed cursor col");
}

/* ── Test: Row expiration (F0044) ───────────────────────────────────── */
static void test_row_expiration(void) {
    DM1_V1_TextMessageState state;
    const DM1_V1_MessageRow* r;

    dm1_v1_text_init(&state);

    dm1_v1_text_print_message(&state, DM1_V1_COLOR_WHITE, "Temporary");

    /* Row should have expiration = 70 */
    r = dm1_v1_text_get_row(&state, DM1_V1_MESSAGE_AREA_ROW_COUNT - 1);
    ASSERT_EQ(r->expirationTime, DM1_V1_MESSAGE_ROW_EXPIRATION_TICKS,
              "expiration set to 70");

    /* Advance time past expiration */
    dm1_v1_text_set_game_time(&state, 71);
    dm1_v1_text_clear_expired_rows(&state);

    /* Row should be cleared */
    r = dm1_v1_text_get_row(&state, DM1_V1_MESSAGE_AREA_ROW_COUNT - 1);
    ASSERT_EQ(r->text[0], '\0', "expired row cleared");
    ASSERT_EQ(r->expirationTime, -1, "expired row expiration reset");
}

static void test_tick(void) {
    DM1_V1_TextMessageState state;

    dm1_v1_text_init(&state);
    dm1_v1_text_print_message(&state, DM1_V1_COLOR_WHITE, "Test tick");

    ASSERT_EQ(dm1_v1_text_get_active_row_count(&state), 1, "pre-tick active");

    /* Tick past expiration */
    dm1_v1_text_tick(&state, DM1_V1_MESSAGE_ROW_EXPIRATION_TICKS + 1);

    ASSERT_EQ(dm1_v1_text_get_active_row_count(&state), 0, "post-tick cleared");
}

/* ── Test: Scroll text ──────────────────────────────────────────────── */
static void test_scroll_text(void) {
    DM1_V1_TextMessageState state;

    dm1_v1_text_init(&state);

    ASSERT_EQ(dm1_v1_text_scroll_active(&state), 0, "scroll init inactive");

    dm1_v1_text_show_scroll(&state, "BEWARE!", DM1_V1_TEXT_TYPE_INSCRIPTION,
                            DM1_V1_COLOR_WHITE);
    ASSERT_EQ(dm1_v1_text_scroll_active(&state), 1, "scroll active");
    ASSERT_STR_EQ(state.scrollText.text, "BEWARE!", "scroll text content");
    ASSERT_EQ(state.scrollText.textType, DM1_V1_TEXT_TYPE_INSCRIPTION,
              "scroll text type");

    dm1_v1_text_clear_scroll(&state);
    ASSERT_EQ(dm1_v1_text_scroll_active(&state), 0, "scroll cleared");
}


static void test_scroll_layout_source_lock(void) {
    DM1_V1_ScrollLayout layout;
    unsigned char encoded[16];

    ASSERT_EQ(dm1_v1_text_scroll_measure_layout("ONE\nTWO\nTHREE", &layout),
              3, "scroll layout three lines");
    ASSERT_EQ(layout.lineCount, 3, "scroll layout lineCount");
    ASSERT_EQ(layout.storedLineCount, 3, "scroll layout stored count");
    ASSERT_EQ(layout.firstLineY,
              DM1_V1_SCROLL_TEXT_CENTER_Y - ((DM1_V1_TEXT_LINE_HEIGHT * 3) / 2),
              "scroll layout first Y centered");
    ASSERT_STR_EQ(layout.lines[0], "ONE", "scroll layout line 0");
    ASSERT_STR_EQ(layout.lines[1], "TWO", "scroll layout line 1");
    ASSERT_STR_EQ(layout.lines[2], "THREE", "scroll layout line 2");

    /* ReDMCSB F0341 preserves the double-trailing-newline quirk by reducing
     * the logical line count by one. */
    ASSERT_EQ(dm1_v1_text_scroll_measure_layout("ONE\nTWO\n\n", &layout),
              2, "scroll layout double trailing newline quirk");

    /* ReDMCSB F0340 / CSBWin DisplayScrollText_OneLine scroll-glyph remap. */
    ASSERT_EQ(dm1_v1_text_scroll_encode_line("AZ{a", encoded, sizeof(encoded)),
              4, "scroll glyph encoded length");
    ASSERT_EQ(encoded[0], 1, "scroll glyph A maps to 1");
    ASSERT_EQ(encoded[1], 26, "scroll glyph Z maps to 26");
    ASSERT_EQ(encoded[2], 27, "scroll glyph { maps to 27");
    ASSERT_EQ(encoded[3], 'a', "scroll glyph lowercase unchanged");
}

/* ── Test: Damage indicators ────────────────────────────────────────── */
static void test_damage_indicators(void) {
    DM1_V1_TextMessageState state;
    const DM1_V1_DamageIndicator* d;

    dm1_v1_text_init(&state);

    ASSERT_EQ(dm1_v1_text_get_damage_count(&state), 0, "damage init empty");

    dm1_v1_text_show_damage(&state, 0, 25, 0);
    ASSERT_EQ(dm1_v1_text_get_damage_count(&state), 1, "damage count 1");

    d = dm1_v1_text_get_damage(&state, 0);
    ASSERT_NONNULL(d, "damage 0 non-null");
    ASSERT_EQ(d->value, 25, "damage value 25");
    ASSERT_EQ(d->championIndex, 0, "damage champion 0");
    ASSERT_EQ(d->big, 0, "damage small");

    /* Add big damage to champion 2 */
    dm1_v1_text_show_damage(&state, 2, 100, 1);
    ASSERT_EQ(dm1_v1_text_get_damage_count(&state), 2, "damage count 2");

    /* Tick to expire */
    dm1_v1_text_tick_damage(&state, DM1_V1_DAMAGE_DISPLAY_TICKS + 1);
    ASSERT_EQ(dm1_v1_text_get_damage_count(&state), 0, "damage all expired");
}

/* ── Test: Multiple scroll messages fill and scroll ─────────────────── */
static void test_multiple_messages_scroll(void) {
    DM1_V1_TextMessageState state;
    int i;
    char buf[32];

    dm1_v1_text_init(&state);

    /* Print 6 messages to force scrolling (4 rows, so 5th+ forces scroll) */
    for (i = 0; i < 6; i++) {
        snprintf(buf, sizeof(buf), "Message %d", i);
        dm1_v1_text_print_linefeed(&state);
        dm1_v1_text_print_message(&state, DM1_V1_COLOR_WHITE, buf);
    }

    /* Should have exactly 4 active rows */
    ASSERT_EQ(dm1_v1_text_get_active_row_count(&state) <= DM1_V1_MESSAGE_AREA_ROW_COUNT, 1,
              "scroll keeps at most ROW_COUNT rows");

    /* The latest message should be in the bottom row */
    const DM1_V1_MessageRow* lastRow =
        dm1_v1_text_get_row(&state, DM1_V1_MESSAGE_AREA_ROW_COUNT - 1);
    ASSERT_NONNULL(lastRow, "scroll bottom row exists");
    /* It should contain "Message 5" */
    ASSERT_STR_EQ(lastRow->text, "Message 5", "scroll bottom has latest msg");
}

/* ── Test: Constants match ReDMCSB ──────────────────────────────────── */
static void test_constants(void) {
    /* M532_MESSAGE_AREA_ROW_COUNT = 4 for PC */
    ASSERT_EQ(DM1_V1_MESSAGE_AREA_ROW_COUNT, 4, "ROW_COUNT=4");

    /* G2087_C6_TextCharacterWidth = 6 */
    ASSERT_EQ(DM1_V1_TEXT_CHARACTER_WIDTH, 6, "CharWidth=6");

    /* G2088_C7_TextLineHeight = 7 */
    ASSERT_EQ(DM1_V1_TEXT_LINE_HEIGHT, 7, "LineHeight=7");

    /* G2092_MessageAreaWidth = 320 */
    ASSERT_EQ(DM1_V1_MESSAGE_AREA_WIDTH, 320, "MsgAreaWidth=320");

    /* Max chars per row = 53 */
    ASSERT_EQ(DM1_V1_MESSAGE_MAX_CHARS_PER_ROW, 53, "MaxChars=53");

    /* Row expiration = 70 */
    ASSERT_EQ(DM1_V1_MESSAGE_ROW_EXPIRATION_TICKS, 70, "Expiration=70");

    /* Text types */
    ASSERT_EQ(DM1_V1_TEXT_TYPE_INSCRIPTION, 0, "TextType inscription=0");
    ASSERT_EQ(DM1_V1_TEXT_TYPE_MESSAGE, 1, "TextType message=1");
    ASSERT_EQ(DM1_V1_TEXT_TYPE_SCROLL, 2, "TextType scroll=2");

    /* Continuation indent = 12 */
    ASSERT_EQ(DM1_V1_MESSAGE_CONTINUATION_INDENT, 12, "ContIndent=12");
}

/* ── Test: Legacy M11_TextState compatibility ───────────────────────── */
static void test_legacy_compat(void) {
    M11_TextState s;
    const M11_TextMessage* msg;

    m11_text_init(&s);
    ASSERT_EQ(m11_text_get_active_count(&s), 0, "legacy init empty");

    m11_text_show(&s, "Test", 10, 20, 15, 0);
    ASSERT_EQ(m11_text_get_active_count(&s), 1, "legacy count 1");

    msg = m11_text_get_message(&s, 0);
    ASSERT_NONNULL(msg, "legacy msg non-null");
    ASSERT_STR_EQ(msg->text, "Test", "legacy msg text");
    ASSERT_EQ(msg->x, 10, "legacy msg x");
    ASSERT_EQ(msg->y, 20, "legacy msg y");
    ASSERT_EQ(msg->color, 15, "legacy msg color");

    m11_text_show_centered(&s, "Centered", 100, 8);
    ASSERT_EQ(m11_text_get_active_count(&s), 2, "legacy count 2");

    m11_text_tick(&s, M11_TEXT_DISPLAY_TICKS + 1);
    ASSERT_EQ(m11_text_get_active_count(&s), 0, "legacy tick expired");
}

/* ── Main ───────────────────────────────────────────────────────────── */
int main(void) {
    printf("=== DM1 V1 Text/Message Display Source-Lock Gate ===\n");
    printf("ReDMCSB: TEXT.C F0042-F0054, DRAWMSGA.C F0696, DEFS.H\n\n");

    test_constants();
    test_init();
    test_cursor_movement();
    test_clear_all_rows();
    test_print_message_basic();
    test_print_message_newline();
    test_print_message_word_wrap();
    test_print_character();
    test_linefeed();
    test_row_expiration();
    test_tick();
    test_scroll_text();
    test_scroll_layout_source_lock();
    test_damage_indicators();
    test_multiple_messages_scroll();
    test_legacy_compat();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
