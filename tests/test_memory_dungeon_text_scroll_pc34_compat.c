/*
 * Source-lock gate for scroll TextString decoding.
 *
 * ReDMCSB evidence:
 *   DUNGEON.C F0168 lines 2255-2275: visible/mask gate and text-type separator
 *   DUNGEON.C F0168 lines 2329-2350: separator emission and buffer terminator
 *   PANEL.C F0341 lines 890-895: scroll panel decodes with C2_TEXT_TYPE_SCROLL
 *     | MASK0x8000_DECODE_EVEN_IF_INVISIBLE
 */

#include "memory_dungeon_dat_pc34_compat.h"
#include <stdio.h>
#include <string.h>

static int g_pass = 0;
static int g_fail = 0;

#define ASSERT_EQ(a, b, msg) do { \
    if ((a) == (b)) { g_pass++; } \
    else { g_fail++; fprintf(stderr, "FAIL: %s: expected %d, got %d\n", (msg), (int)(b), (int)(a)); } \
} while (0)

#define ASSERT_STR_EQ(a, b, msg) do { \
    if (strcmp((a), (b)) == 0) { g_pass++; } \
    else { g_fail++; fprintf(stderr, "FAIL: %s: expected '%s', got '%s'\n", (msg), (b), (a)); } \
} while (0)

static unsigned short pack3(int a, int b, int c) {
    return (unsigned short)(((a & 31) << 10) | ((b & 31) << 5) | (c & 31));
}

static void seed_things(struct DungeonThings_Compat* things,
                        struct DungeonTextString_Compat* textStrings,
                        struct DungeonScroll_Compat* scrolls,
                        unsigned short* textData) {
    memset(things, 0, sizeof(*things));

    textData[0] = pack3(14, 13, 4);        /* ONE */
    textData[1] = pack3(28, 19, 22);       /* separator, TW */
    textData[2] = pack3(14, 31, 31);       /* O, end */

    textStrings[0].next = THING_ENDOFLIST;
    textStrings[0].visible = 0;
    textStrings[0].textDataWordOffset = 0;

    scrolls[0].next = THING_ENDOFLIST;
    scrolls[0].textStringThingIndex = 0;
    scrolls[0].closed = 1;

    things->textData = textData;
    things->textDataWordCount = 3;
    things->textStrings = textStrings;
    things->textStringCount = 1;
    things->scrolls = scrolls;
    things->scrollCount = 1;
}

static void test_scroll_decode_uses_newline_and_invisible_mask(void) {
    struct DungeonThings_Compat things;
    struct DungeonTextString_Compat textStrings[1];
    struct DungeonScroll_Compat scrolls[1];
    unsigned short textData[3];
    char decoded[64];

    seed_things(&things, textStrings, scrolls, textData);

    ASSERT_EQ(F0509_DUNGEON_DecodeScrollText_Compat(&things, 0, decoded, sizeof(decoded)),
              7, "scroll decode length");
    ASSERT_STR_EQ(decoded, "ONE\nTWO", "scroll separator is newline");

    ASSERT_EQ(F0507_DUNGEON_DecodeTextAtOffset_Compat(
                  textData, 3, 0, decoded, sizeof(decoded)),
              7, "legacy offset decode length");
    ASSERT_STR_EQ(decoded, "ONE\nTWO", "legacy offset decode remains scroll-like");
}

static void test_f0168_visibility_gate_and_message_separator(void) {
    struct DungeonThings_Compat things;
    struct DungeonTextString_Compat textStrings[1];
    struct DungeonScroll_Compat scrolls[1];
    unsigned short textData[3];
    char decoded[64];

    seed_things(&things, textStrings, scrolls, textData);

    ASSERT_EQ(F0508_DUNGEON_DecodeTextStringThing_Compat(
                  &things, 0, DUNGEON_TEXT_TYPE_SCROLL, decoded, sizeof(decoded)),
              0, "invisible text without mask produces no text");
    ASSERT_STR_EQ(decoded, "", "invisible text without mask output");

    textStrings[0].visible = 1;
    ASSERT_EQ(F0508_DUNGEON_DecodeTextStringThing_Compat(
                  &things, 0, DUNGEON_TEXT_TYPE_MESSAGE, decoded, sizeof(decoded)),
              8, "message decode length includes leading newline");
    ASSERT_STR_EQ(decoded, "\nONE TWO", "message separator is space");
}

static void test_inscription_separator_and_terminator(void) {
    struct DungeonThings_Compat things;
    struct DungeonTextString_Compat textStrings[1];
    struct DungeonScroll_Compat scrolls[1];
    unsigned short textData[3];
    unsigned char decoded[64];

    seed_things(&things, textStrings, scrolls, textData);
    textStrings[0].visible = 1;

    ASSERT_EQ(F0508_DUNGEON_DecodeTextStringThing_Compat(
                  &things, 0, DUNGEON_TEXT_TYPE_INSCRIPTION,
                  (char*)decoded, sizeof(decoded)),
              8, "inscription decode length includes terminator byte");
    ASSERT_EQ(decoded[3], 0x80, "inscription separator is 0x80");
    ASSERT_EQ(decoded[7], 0x81, "inscription final marker is 0x81");
    ASSERT_EQ(decoded[8], 0, "inscription is nul terminated after marker");
}

int main(void) {
    printf("=== Dungeon Text Scroll Source-Lock Gate ===\n");
    printf("ReDMCSB: DUNGEON.C F0168, PANEL.C F0341\n\n");

    test_scroll_decode_uses_newline_and_invisible_mask();
    test_f0168_visibility_gate_and_message_separator();
    test_inscription_separator_and_terminator();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
