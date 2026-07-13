/* Source-lock: skproject/SKWIN/SkWinCore.cpp QUERY_MESSAGE_TEXT, 0CEE:159B.
 * DB2 TextMode==0 indexes the little-endian U16 dunTextData table, with
 * three 5-bit glyph codes per word. */
#include "dm2_v1_dungeon_loader.h"

#include <stdio.h>
#include <string.h>

static int checks;
static int failures;

static void expect_true(int condition, const char *label)
{
    ++checks;
    if (!condition) {
        ++failures;
        fprintf(stderr, "FAIL: %s\n", label);
    }
}

static void put16le(uint8_t *p, uint16_t value)
{
    p[0] = (uint8_t)(value & 0xffu);
    p[1] = (uint8_t)(value >> 8);
}

int main(void)
{
    uint8_t raw[8] = { 0 };
    DM2_V1_DungeonData dungeon;
    DM2_V1_G1Map5TextRuntimeReceipt texts;
    DM2_V1_G1TextMessageRuntimeReceipt receipt;

    /* H I . / terminator: ((7 << 10) | (8 << 5) | 27), (31 << 10). */
    put16le(raw + 0, (uint16_t)((7u << 10) | (8u << 5) | 27u));
    put16le(raw + 2, (uint16_t)(31u << 10));
    /* Phrase-bank escape 29 must not be replaced by guessed output. */
    put16le(raw + 4, (uint16_t)((0u << 10) | (29u << 5) | 31u));

    memset(&dungeon, 0, sizeof(dungeon));
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    dungeon.text_data_base = 0;
    dungeon.text_word_count = 4;

    memset(&texts, 0, sizeof(texts));
    texts.committed = 1;
    texts.incomplete_world = 1;
    texts.map = 5;
    texts.text_root_count = 3;
    texts.texts[0].x = 4;
    texts.texts[0].y = 9;
    texts.texts[0].object_id = 0x8800u;
    texts.texts[0].visible = 1u;
    texts.texts[0].mode = 0u;
    texts.texts[0].text_index = 0u;
    texts.texts[1].visible = 1u;
    texts.texts[1].mode = 0u;
    texts.texts[1].text_index = 2u;
    texts.texts[2].visible = 1u;
    texts.texts[2].mode = 1u; /* GDAT MESSAGE branch, outside dunTextData. */
    texts.texts[2].text_index = 0x0e01u;

    expect_true(dm2_v1_dungeon_materialize_g1_map5_text_messages(
                    &dungeon, &texts, &receipt) == 1 && receipt.valid,
                "committed DB2 receipt reaches source text decoder");
    expect_true(receipt.decoded_message_count == 1 &&
                    receipt.messages[0].x == 4 && receipt.messages[0].y == 9 &&
                    receipt.messages[0].object_id == 0x8800u &&
                    strcmp(receipt.messages[0].text, "HI.") == 0 &&
                    receipt.messages[0].source_word_count == 2u,
                "literal 5-bit dungeon text preserves source placement and bytes");
    expect_true(receipt.blocked_phrase_message_count == 1 &&
                    receipt.skipped_non_dungeon_message_count == 1,
                "phrase-bank and GDAT-only branches stay unavailable, never synthetic");

    texts.blocked_record_reads = 1;
    expect_true(!dm2_v1_dungeon_materialize_g1_map5_text_messages(
                    &dungeon, &texts, &receipt),
                "untrusted partial map receipt cannot publish text");

    printf("DM2 G1 DB2 text messages: %d/%d checks passed\n",
           checks - failures, checks);
    return failures ? 1 : 0;
}
