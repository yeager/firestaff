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

static int read_gdat_text(void *userdata, int category, int index, int field,
                          const uint8_t **out_data, uint32_t *out_byte_count)
{
    static const uint8_t source_text[] = { 0x91u, 0x02u, 0x44u, 0x00u };
    (void)userdata;
    if (!out_data || !out_byte_count || category != DM2_GDAT_CATEGORY_MESSAGES ||
        index != 0 || field != 1) {
        return 0;
    }
    *out_data = source_text;
    *out_byte_count = (uint32_t)sizeof(source_text);
    return 1;
}

int main(void)
{
    uint8_t raw[8] = { 0 };
    DM2_V1_DungeonData dungeon;
    DM2_V1_G1Map5TextRuntimeReceipt texts;
    DM2_V1_G1TextMessageRuntimeReceipt receipt;
    DM2_V1_G1GdatTextMessageRuntimeReceipt gdat_receipt;

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
    texts.texts[2].object_id = 0x8802u;

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
                "phrase-bank remains unavailable and GDAT text is separate");

    expect_true(dm2_v1_dungeon_materialize_g1_map5_gdat_text_messages(
                    &texts, read_gdat_text, NULL, &gdat_receipt) == 1 &&
                    gdat_receipt.valid && gdat_receipt.material_count == 1 &&
                    gdat_receipt.messages[0].object_id == 0x8800u + 2u &&
                    gdat_receipt.messages[0].gdat_field == 1u &&
                    gdat_receipt.messages[0].raw_byte_count == 4u &&
                    gdat_receipt.messages[0].raw_hash != 0u,
                "mode-one extension 14 retains exact MESSAGES dtText bytes");

    texts.texts[2].text_index = 0x0e02u;
    expect_true(!dm2_v1_dungeon_materialize_g1_map5_gdat_text_messages(
                    &texts, read_gdat_text, NULL, &gdat_receipt),
                "missing mode-one GDAT payload stays unavailable");
    texts.texts[2].text_index = 0x0e01u;

    texts.blocked_record_reads = 1;
    expect_true(!dm2_v1_dungeon_materialize_g1_map5_text_messages(
                    &dungeon, &texts, &receipt),
                "untrusted partial map receipt cannot publish text");

    printf("DM2 G1 DB2 text messages: %d/%d checks passed\n",
           checks - failures, checks);
    return failures ? 1 : 0;
}
