#ifndef FIRESTAFF_REDMCSB_F0952_JAPANESE_PRINT_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0952_JAPANESE_PRINT_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REDMCSB_F0952_JAPANESE_PRINT_ANK_PATTERN_BYTE_COUNT 16U
#define REDMCSB_F0952_JAPANESE_PRINT_KANJI_PATTERN_BYTE_COUNT 32U

typedef enum {
    REDMCSB_F0952_JAPANESE_PRINT_P20JA = 0,
    REDMCSB_F0952_JAPANESE_PRINT_P20JB,
    REDMCSB_F0952_JAPANESE_PRINT_P31J_GAME,
    REDMCSB_F0952_JAPANESE_PRINT_P31J_CEDT
} redmcsb_f0952_japanese_print_profile_pc34_compat;

typedef void (*redmcsb_f0952_japanese_pc98_pattern_pc34_compat)(
    void *context,
    int16_t character_code,
    uint8_t pattern[REDMCSB_F0952_JAPANESE_PRINT_KANJI_PATTERN_BYTE_COUNT]);

typedef void (*redmcsb_f0952_japanese_ank_pattern_pc34_compat)(
    void *context,
    int16_t character_code,
    uint8_t pattern[REDMCSB_F0952_JAPANESE_PRINT_ANK_PATTERN_BYTE_COUNT]);

typedef struct {
    redmcsb_f0952_japanese_pc98_pattern_pc34_compat pc98_pattern;
    redmcsb_f0952_japanese_ank_pattern_pc34_compat ank_pattern;
} redmcsb_f0952_japanese_print_io_pc34_compat;

/*
 * Host representation of the source bitmap. pixel_width is M100_PIXEL_WIDTH
 * from JAPANESE.C; pixels contain its packed, two 4-bit pixels per byte rows.
 */
typedef struct {
    uint8_t *pixels;
    uint16_t pixel_width;
} redmcsb_f0952_japanese_print_bitmap_pc34_compat;

/*
 * ReDMCSB JAPANESE.C F0952_JAPANESE_Print for MEDIA459_P20JA_P20JB_P31J.
 * When bitmap is NULL, glyph lookup and rasterization are skipped exactly as
 * in the source. Otherwise bitmap, io, and the required callback are valid.
 */
int16_t redmcsb_f0952_japanese_print_pc34_compat(
    const uint8_t *string,
    int16_t byte_index,
    const redmcsb_f0952_japanese_print_bitmap_pc34_compat *bitmap,
    int16_t *width,
    int16_t *height,
    int16_t text_color,
    int16_t background_color,
    redmcsb_f0952_japanese_print_profile_pc34_compat profile,
    const redmcsb_f0952_japanese_print_io_pc34_compat *io,
    void *context);

const char *redmcsb_f0952_japanese_print_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0952_JAPANESE_PRINT_PC34_COMPAT_H */
