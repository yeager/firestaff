#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0952_japanese_print_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

typedef struct {
    unsigned int pc98_calls;
    unsigned int ank_calls;
    int16_t pc98_code;
    int16_t ank_code;
} glyph_log;

static void load_pc98(void *context, int16_t character_code,
                      uint8_t pattern[
                          REDMCSB_F0952_JAPANESE_PRINT_KANJI_PATTERN_BYTE_COUNT])
{
    glyph_log *log = context;

    ++log->pc98_calls;
    log->pc98_code = character_code;
    memset(pattern, 0, REDMCSB_F0952_JAPANESE_PRINT_KANJI_PATTERN_BYTE_COUNT);
    pattern[0] = UINT8_C(0xc0);
    pattern[1] = UINT8_C(0x03);
}

static void load_ank(void *context, int16_t character_code,
                     uint8_t pattern[
                         REDMCSB_F0952_JAPANESE_PRINT_ANK_PATTERN_BYTE_COUNT])
{
    glyph_log *log = context;

    ++log->ank_calls;
    log->ank_code = character_code;
    memset(pattern, 0, REDMCSB_F0952_JAPANESE_PRINT_ANK_PATTERN_BYTE_COUNT);
    pattern[0] = UINT8_C(0x90);
}

int main(void)
{
    static const redmcsb_f0952_japanese_print_io_pc34_compat io = {
        load_pc98,
        load_ank
    };
    (void)io;
    static const uint8_t control[] = { UINT8_C(0x1b) };
    (void)control;
    static const uint8_t vertical_bar[] = { UINT8_C(0x7c) };
    (void)vertical_bar;
    static const uint8_t ank[] = { UINT8_C(0x41) };
    (void)ank;
    static const uint8_t kanji[] = { UINT8_C(0x81), UINT8_C(0x40) };
    (void)kanji;
    static const uint8_t special_kanji[] = { UINT8_C(0xea), UINT8_C(0xa1) };
    (void)special_kanji;
    uint8_t pixels[16U * 32U];
    redmcsb_f0952_japanese_print_bitmap_pc34_compat bitmap = {
        pixels,
        32U
    };
    glyph_log log = { 0U, 0U, 0, 0 };
    (void)log;
    int16_t width = -1;
    (void)width;
    int16_t height = -1;
    (void)height;
    const char *evidence;
    (void)evidence;

    assert(redmcsb_f0952_japanese_print_pc34_compat(
               control, 0, NULL, &width, &height, 2, 5,
               REDMCSB_F0952_JAPANESE_PRINT_P20JA, &io, &log) == 1);
    assert(width == 0 && height == 0 && log.pc98_calls == 0U &&
           log.ank_calls == 0U);
    assert(redmcsb_f0952_japanese_print_pc34_compat(
               vertical_bar, 0, NULL, &width, &height, 2, 5,
               REDMCSB_F0952_JAPANESE_PRINT_P20JA, &io, &log) == 1);
    assert(width == 0 && height == 0);

    assert(redmcsb_f0952_japanese_print_pc34_compat(
               kanji, 0, NULL, &width, &height, 2, 5,
               REDMCSB_F0952_JAPANESE_PRINT_P20JA, &io, &log) == 2);
    assert(width == 16 && height == 16 && log.pc98_calls == 0U);

    memset(pixels, UINT8_C(0xaa), sizeof(pixels));
    assert(redmcsb_f0952_japanese_print_pc34_compat(
               ank, 0, &bitmap, &width, &height, 2, 5,
               REDMCSB_F0952_JAPANESE_PRINT_P20JA, &io, &log) == 1);
    assert(width == 8 && height == 16 && log.ank_calls == 1U &&
           log.ank_code == 0x41);
    assert(pixels[0] == UINT8_C(0x25));
    assert(pixels[1] == UINT8_C(0x52));
    assert(pixels[2] == UINT8_C(0x55));
    assert(pixels[3] == UINT8_C(0x55));
    assert(pixels[16] == UINT8_C(0x55));

    memset(pixels, UINT8_C(0xaa), sizeof(pixels));
    assert(redmcsb_f0952_japanese_print_pc34_compat(
               kanji, 0, &bitmap, &width, &height, 2, 5,
               REDMCSB_F0952_JAPANESE_PRINT_P20JA, &io, &log) == 2);
    assert(width == 16 && height == 16 && log.pc98_calls == 1U &&
           (uint16_t)log.pc98_code == UINT16_C(0x2121));
    assert(pixels[0] == UINT8_C(0x22));
    assert(pixels[1] == UINT8_C(0x55));
    assert(pixels[2] == UINT8_C(0x55));
    assert(pixels[3] == UINT8_C(0x55));
    assert(pixels[4] == UINT8_C(0x55));
    assert(pixels[5] == UINT8_C(0x55));
    assert(pixels[6] == UINT8_C(0x55));
    assert(pixels[7] == UINT8_C(0x22));

    memset(pixels, UINT8_C(0xaa), sizeof(pixels));
    assert(redmcsb_f0952_japanese_print_pc34_compat(
               special_kanji, 0, &bitmap, &width, &height, 2, 5,
               REDMCSB_F0952_JAPANESE_PRINT_P20JB, &io, &log) == 2);
    assert(log.pc98_calls == 1U);
    assert(pixels[0] == UINT8_C(0x52));
    assert(pixels[1] == UINT8_C(0x55));
    assert(pixels[2] == UINT8_C(0x55));
    assert(pixels[3] == UINT8_C(0x55));

    bitmap.pixel_width = 64U;
    memset(pixels, UINT8_C(0xaa), sizeof(pixels));
    assert(redmcsb_f0952_japanese_print_pc34_compat(
               ank, 0, &bitmap, &width, &height, 2, 5,
               REDMCSB_F0952_JAPANESE_PRINT_P31J_CEDT, &io, &log) == 1);
    assert(pixels[32] == UINT8_C(0x55));
    assert(pixels[16] == UINT8_C(0xaa));

    evidence = redmcsb_f0952_japanese_print_source_evidence_pc34();
    assert(strstr(evidence, "JAPANESE.C:270-381") != NULL);
    assert(strstr(evidence, "0x7423") != NULL);
    puts("ok: ReDMCSB F0952 Japanese PC-98 print");
    return 0;
}
