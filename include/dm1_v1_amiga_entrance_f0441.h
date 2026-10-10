#ifndef DM1_V1_AMIGA_ENTRANCE_F0441_H
#define DM1_V1_AMIGA_ENTRANCE_F0441_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_AMIGA_ENTRANCE_F0441_GRAPHIC_LEFT_DOOR = 2,
    DM1_V1_AMIGA_ENTRANCE_F0441_GRAPHIC_RIGHT_DOOR = 3,
    DM1_V1_AMIGA_ENTRANCE_F0441_GRAPHIC_SCREEN = 4,
    DM1_V1_AMIGA_ENTRANCE_F0441_DOOR_FRAMES = 8,
    DM1_V1_AMIGA_ENTRANCE_F0441_OPENING_STEPS = 31,
    DM1_V1_AMIGA_ENTRANCE_F0441_SWITCH_DELAY_TICKS = 20,
    DM1_V1_AMIGA_ENTRANCE_F0441_PALETTE_ENTRIES = 16
};

typedef struct DM1_V1_AmigaEntranceF0441Receipt {
    int valid;
    char graphics_md5[33];
    char executable_md5[33];
    unsigned int screen_width;
    unsigned int screen_height;
    unsigned int left_door_width;
    unsigned int left_door_height;
    unsigned int right_door_width;
    unsigned int right_door_height;
    unsigned int screen_nonzero_pixels;
    unsigned int left_door_nonzero_pixels;
    unsigned int right_door_nonzero_pixels;
    unsigned int door_frame_count;
    unsigned int opening_steps;
    unsigned int switch_delay_ticks;
    int mouse_input_only;
    int has_credits_graphic;
    int has_entrance_buttons_graphic;
    uint16_t entrance_palette_rgb4[DM1_V1_AMIGA_ENTRANCE_F0441_PALETTE_ENTRIES];
    const char *source_evidence;
} DM1_V1_AmigaEntranceF0441Receipt;

/* Authenticate A20 English v2.0 C002/C003/C004 pixels and their source
 * contract from the selected ADF's GRAPHICS.DAT and SWSH executable. */
int dm1_v1_amiga_entrance_f0441_receipt(
    const uint8_t *graphics_dat,
    size_t graphics_dat_bytes,
    const uint8_t *swsh_executable,
    size_t swsh_executable_bytes,
    DM1_V1_AmigaEntranceF0441Receipt *out_receipt);

/* Authenticate the A36M F0441 source path from its original GRAPHICS.DAT.
 * A36 has no F0437/SWSH title pair: it runs F0441 after F1051_ directly.
 * This receipt also proves C005 credits and C011 entrance-button assets,
 * which F0441 loads before waiting for C200. */
int dm1_v1_amiga36_entrance_f0441_receipt(
    const uint8_t *graphics_dat,
    size_t graphics_dat_bytes,
    DM1_V1_AmigaEntranceF0441Receipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif
