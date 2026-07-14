#ifndef FIRESTAFF_REDMCSB_F1036_MOUSE1_H
#define FIRESTAFF_REDMCSB_F1036_MOUSE1_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    REDMCSB_F1036_SPRITE_IMAGE_WORDS = 40,
    REDMCSB_F1036_SPRITES_PER_POINTER = 4,
    REDMCSB_F1036_POINTER_BANKS = 5,
    REDMCSB_F1036_POINTER_BANK_WORDS =
        REDMCSB_F1036_SPRITE_IMAGE_WORDS * REDMCSB_F1036_SPRITES_PER_POINTER,
    REDMCSB_F1036_SPRITE_DATA_WORDS = 36
};

/*
 * bitmap points at the planar payload. Its two preceding native-endian
 * int16_t values are the pixel width and height, as in M100/M101. The caller
 * supplies the 5-bank G3203_ storage in uint16_t words.
 */
void redmcsb_f1036_mouse1(uint16_t *sprite_images,
                           const uint8_t *bitmap,
                           int16_t pointer_index);

const char *redmcsb_f1036_mouse1_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1036_MOUSE1_H */
