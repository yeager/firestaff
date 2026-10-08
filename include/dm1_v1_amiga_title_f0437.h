#ifndef DM1_V1_AMIGA_TITLE_F0437_H
#define DM1_V1_AMIGA_TITLE_F0437_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_AMIGA_TITLE_WIDTH = 320,
    DM1_V1_AMIGA_TITLE_HEIGHT = 200,
    DM1_V1_AMIGA_TITLE_GRAPHIC_C001 = 1,
    DM1_V1_AMIGA_TITLE_ZOOM_STEP_COUNT = 18,
    DM1_V1_AMIGA_TITLE_RGB4_ENTRIES = 16
};

typedef struct DM1_V1_AmigaTitleF0437Receipt {
    int valid;
    char graphics_md5[33];
    char executable_md5[33];
    unsigned int c001_width;
    unsigned int c001_height;
    unsigned int c001_pixel_fingerprint;
    unsigned int presents_nonzero_pixels;
    unsigned int dungeon_nonzero_pixels;
    unsigned int master_nonzero_pixels;
    unsigned int presents_source_y;
    unsigned int presents_source_height;
    unsigned int presents_destination_y;
    unsigned int master_source_y;
    unsigned int master_source_height;
    unsigned int master_destination_y;
    unsigned int dungeon_source_y;
    unsigned int dungeon_source_height;
    unsigned int pre_zoom_vblanks;
    unsigned int source_zoom_steps;
    unsigned int source_delay_ticks;
    unsigned int source_beam_wait_line;
    uint16_t initial_palette[DM1_V1_AMIGA_TITLE_RGB4_ENTRIES];
    uint16_t presents_palette[DM1_V1_AMIGA_TITLE_RGB4_ENTRIES];
    uint16_t zoom_palette[DM1_V1_AMIGA_TITLE_RGB4_ENTRIES];
    uint16_t final_palette[DM1_V1_AMIGA_TITLE_RGB4_ENTRIES];
    const char *source_evidence;
} DM1_V1_AmigaTitleF0437Receipt;

typedef struct DM1_V1_AmigaTitleF0437ZoomStep {
    unsigned int index;
    unsigned int source_x;
    unsigned int source_y;
    unsigned int source_width;
    unsigned int source_height;
    unsigned int destination_x;
    unsigned int destination_y;
    unsigned int destination_width;
    unsigned int destination_height;
    unsigned int wait_vblanks_before;
    unsigned int wait_for_beam_line;
} DM1_V1_AmigaTitleF0437ZoomStep;

/* Validate the selected English A20 game identity and the C001 pixels decoded
 * from its original Amiga GRAPHICS.DAT. Both byte arrays must come from the
 * same selected ADF; this function authenticates the A20 SWSH executable
 * itself rather than accepting a caller-created identity label. */
int dm1_v1_amiga_title_f0437_receipt(
    const uint8_t *graphics_dat,
    size_t graphics_dat_bytes,
    const uint8_t *swsh_executable,
    size_t swsh_executable_bytes,
    DM1_V1_AmigaTitleF0437Receipt *out_receipt);

/* Return one authored A20 C001 zoom frame. Indices are presented in ascending
 * source order, matching TITLE.C's 0..17 Amiga loop. */
int dm1_v1_amiga_title_f0437_zoom_step(
    const DM1_V1_AmigaTitleF0437Receipt *receipt,
    unsigned int index,
    DM1_V1_AmigaTitleF0437ZoomStep *out_step);

#ifdef __cplusplus
}
#endif

#endif
