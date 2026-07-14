#include "redmcsb_f1036_mouse1.h"

#include <string.h>

void redmcsb_f1036_mouse1(uint16_t *sprite_images,
                           const uint8_t *bitmap,
                           int16_t pointer_index)
{
    int16_t pixel_width;
    int16_t pixel_height;
    uint16_t words_per_row;
    uint16_t words_per_plane;
    uint16_t column;
    uint16_t row;
    uint16_t *destination;
    const uint8_t *source;

    memcpy(&pixel_width, bitmap - (2 * sizeof(pixel_width)),
           sizeof(pixel_width));
    memcpy(&pixel_height, bitmap - sizeof(pixel_height), sizeof(pixel_height));

    destination = sprite_images +
                  ((pointer_index + 1) * REDMCSB_F1036_POINTER_BANK_WORDS) +
                  2;
    for (row = 0; row < REDMCSB_F1036_SPRITES_PER_POINTER; row++) {
        memset(destination + (row * REDMCSB_F1036_SPRITE_IMAGE_WORDS), 0,
               REDMCSB_F1036_SPRITE_DATA_WORDS * sizeof(*destination));
    }

    words_per_row = (uint16_t)(((pixel_width + 0x0f) & 0xfff0) >> 4);
    words_per_plane = (uint16_t)(pixel_height * words_per_row);
    source = bitmap;
    for (column = 0; column < words_per_row; column++) {
        uint16_t *plane_0 = destination;
        uint16_t *plane_1 = plane_0 + 1;
        uint16_t *plane_2 = plane_0 + REDMCSB_F1036_SPRITE_IMAGE_WORDS;
        uint16_t *plane_3 = plane_2 + 1;
        const uint8_t *source_0 = source;
        const uint8_t *source_1 = source_0 + (words_per_plane * 2U);
        const uint8_t *source_2 = source_1 + (words_per_plane * 2U);
        const uint8_t *source_3 = source_2 + (words_per_plane * 2U);

        for (row = 0; row < (uint16_t)pixel_height; row++) {
            memcpy(plane_0, source_0, sizeof(*plane_0));
            plane_0 += 2;
            source_0 += words_per_row * 2U;
            memcpy(plane_1, source_1, sizeof(*plane_1));
            plane_1 += 2;
            source_1 += words_per_row * 2U;
            memcpy(plane_2, source_2, sizeof(*plane_2));
            plane_2 += 2;
            source_2 += words_per_row * 2U;
            memcpy(plane_3, source_3, sizeof(*plane_3));
            plane_3 += 2;
            source_3 += words_per_row * 2U;
        }
        destination += REDMCSB_F1036_SPRITE_IMAGE_WORDS * 2;
        source += sizeof(uint16_t);
    }
}

const char *redmcsb_f1036_mouse1_source_evidence(void)
{
    return "ReDMCSB IO.C:1863-2019 defines F1036_Mouse1. IO.C:1956-1961 "
           "selects G3203_ bank P2771_i_ + 1, clears four 18x2-word sprite "
           "data regions, and derives rounded-up 16-pixel words and height "
           "from M100_PIXEL_WIDTH/M101_PIXEL_HEIGHT. IO.C:1962-2017 copies "
           "the four contiguous bitmap planes into interleaved sprite planes "
           "with 40-word records and an 80-word horizontal-group stride. "
           "AMIGA.H:91-97 defines SPRITEIMAGE as two control words, 18x2 data "
           "words, and two reserved words; DEFS.H:3444-3445 defines M100/M101.";
}
