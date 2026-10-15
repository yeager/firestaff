#ifndef DM1_V1_AMIGA_ENTRANCE_F0441_H
#define DM1_V1_AMIGA_ENTRANCE_F0441_H

#include <stddef.h>
#include <stdint.h>
#include "dm1_v1_amiga_title_f0437.h"
#include "asset_loader_m11.h"

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

typedef struct { int x, y, w, h; } DM1_V1_AmigaEntranceRect;

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
    /* A36M C696, resolved with F0635/F0788 native bitmap dimensions. */
    int a36_layout_valid;
    unsigned int a36_step_pixels;
    int a36_fade_valid;
    char a36_driver_md5[33];
    uint8_t a36_fade_increments[15][16];
    DM1_V1_AmigaEntranceRect a36_transfer, a36_working;
    DM1_V1_AmigaEntranceRect a36_left, a36_right, a36_viewport;
    DM1_V1_AmigaEntranceRect a36_enter_button, a36_resume_button;
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
/* Exact four-file A22G startup admission; buffers remain caller-owned. */
int dm1_v1_amiga_entrance_f0441_receipt_for_startup(
    const DM1_V1_AmigaStartupMedia *media,
    DM1_V1_AmigaEntranceF0441Receipt *out_receipt);

int dm1_v1_amiga36_entrance_f0441_receipt(
    const uint8_t *graphics_dat,
    size_t graphics_dat_bytes,
    DM1_V1_AmigaEntranceF0441Receipt *out_receipt);

/* Frame zero is the closed entrance. Frames 1..opening_steps implement
 * F0438; selected_button is -1, 0 (Enter), or 1 (Resume). The borrowed
 * dungeon page is the caller's separately source-owned 320x200 page. */
/* The driver is borrowed from the same selected installation as GRAPHICS.
 * Authenticates VDEO.FTL and reads its native 15x16 DRIA palette increments. */
int dm1_v1_amiga36_entrance_bind_driver(
    DM1_V1_AmigaEntranceF0441Receipt *receipt,
    const uint8_t *driver, size_t driver_bytes);
int dm1_v1_amiga36_entrance_fade_step(
    const DM1_V1_AmigaEntranceF0441Receipt *receipt,
    const uint16_t source[16], const uint16_t target[16],
    unsigned int frame, uint16_t out_palette[16]);

int dm1_v1_amiga36_entrance_render_c255(M11_AssetLoader *loader,
                                        uint8_t *viewport, size_t bytes);

int dm1_v1_amiga36_entrance_compose(
    M11_AssetLoader *loader, const DM1_V1_AmigaEntranceF0441Receipt *receipt,
    const uint8_t *dungeon_page, unsigned int frame, int selected_button,
    uint8_t *out_page, size_t out_page_bytes);

#ifdef __cplusplus
}
#endif

#endif
