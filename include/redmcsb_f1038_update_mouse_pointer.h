#ifndef FIRESTAFF_REDMCSB_F1038_UPDATE_MOUSE_POINTER_H
#define FIRESTAFF_REDMCSB_F1038_UPDATE_MOUSE_POINTER_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    REDMCSB_F1038_SPRITE_COUNT = 4,
    REDMCSB_F1038_SPRITE_IMAGE_WORDS = 40,
    REDMCSB_F1038_POINTER_BANK_WORDS =
        REDMCSB_F1038_SPRITE_COUNT * REDMCSB_F1038_SPRITE_IMAGE_WORDS,
    REDMCSB_F1038_POINTER_NONE = -1
};

typedef int16_t (*redmcsb_f1038_get_pointer_type)(void *context,
                                                    int16_t x,
                                                    int16_t y);
typedef void (*redmcsb_f1038_change_sprite)(void *context,
                                             uint16_t sprite_index,
                                             const uint16_t *image);
typedef void (*redmcsb_f1038_move_sprite)(void *context,
                                           uint16_t sprite_index,
                                           long x,
                                           long y);

typedef struct {
    bool active;
    int16_t mouse_x;
    int16_t mouse_y;
    int16_t pointer_type;
    int16_t last_mouse_x;
    int16_t last_mouse_y;
    /* Rows are addressed by pointer_type + 1; [0] is the no-pointer row. */
    const int16_t (*hotspots)[4];
    const uint16_t *sprite_images;
    redmcsb_f1038_get_pointer_type get_pointer_type;
    redmcsb_f1038_change_sprite change_sprite;
    redmcsb_f1038_move_sprite move_sprite;
    void *context;
} redmcsb_f1038_update_mouse_pointer_state;

/*
 * Implements the portable Amiga F1038 pointer-selection, image-bank, and
 * four-sprite positioning sequence. Callbacks and image storage are host
 * boundaries. The resolver must return -1 or an index represented by both
 * hotspots and sprite-image banks.
 */
void redmcsb_f1038_update_mouse_pointer(
    redmcsb_f1038_update_mouse_pointer_state *state);

const char *redmcsb_f1038_update_mouse_pointer_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1038_UPDATE_MOUSE_POINTER_H */
