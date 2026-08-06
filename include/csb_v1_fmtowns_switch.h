#ifndef CSB_V1_FMTOWNS_SWITCH_H
#define CSB_V1_FMTOWNS_SWITCH_H

#include <stddef.h>
#include <stdint.h>

#include "csb_v1_fmtowns_graphics_dat.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * FM Towns CSB Switch-menu resources reside in the original SWITCHTW.EXP
 * executable, not in a host-made menu sheet. ReDMCSB SWITCH.C F2279/F2280
 * registers the four compressed button images and expands the Japanese or
 * English 320x200 background with IMAGE2.C F0689.
 */
#define CSB_FMTOWNS_SWITCH_WIDTH 320u
#define CSB_FMTOWNS_SWITCH_HEIGHT 200u
#define CSB_FMTOWNS_SWITCH_PIXELS \
    (CSB_FMTOWNS_SWITCH_WIDTH * CSB_FMTOWNS_SWITCH_HEIGHT)
#define CSB_FMTOWNS_SWITCH_BUTTON_COUNT 4u

typedef enum {
    CSB_FMTOWNS_SWITCH_JAPANESE = 0,
    CSB_FMTOWNS_SWITCH_ENGLISH = 1
} CSB_V1_FmtownsSwitchLanguage;

typedef enum {
    CSB_FMTOWNS_SWITCH_ACTION_NONE = 0,
    CSB_FMTOWNS_SWITCH_ACTION_STORY = 1,
    CSB_FMTOWNS_SWITCH_ACTION_UTILITY = 2,
    CSB_FMTOWNS_SWITCH_ACTION_GAME = 3,
    CSB_FMTOWNS_SWITCH_ACTION_TOGGLE_LANGUAGE = 4
} CSB_V1_FmtownsSwitchAction;

typedef struct {
    int valid;
    uint8_t button_index;
    uint8_t source_exit_status;
    CSB_V1_FmtownsSwitchAction action;
} CSB_V1_FmtownsSwitchInputReceipt;

typedef struct {
    uint8_t red6;
    uint8_t green6;
    uint8_t blue6;
} CSB_V1_FmtownsSwitchColor;

typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    size_t source_offset;
    size_t source_byte_count;
    CSB_V1_FmtownsItemDecodeReceipt image;
} CSB_V1_FmtownsSwitchButton;

typedef struct {
    int valid;
    uint32_t executable_fnv1a;
    size_t palette_offset;
    size_t palette_byte_count;
    CSB_V1_FmtownsSwitchColor palette[16];
    size_t japanese_page_offset;
    size_t japanese_page_byte_count;
    size_t english_page_offset;
    size_t english_page_byte_count;
    CSB_V1_FmtownsItemDecodeReceipt japanese_page;
    CSB_V1_FmtownsItemDecodeReceipt english_page;
    CSB_V1_FmtownsSwitchButton buttons[CSB_FMTOWNS_SWITCH_BUTTON_COUNT];
    /* F2279 registers G4172 for Japanese and G4173 for English in the
     * fourth rectangle. The common hit rectangle remains buttons[3]. */
    CSB_V1_FmtownsSwitchButton language_buttons[2];
} CSB_V1_FmtownsSwitchReceipt;

/* Locates only the contiguous F31E/F31J Switch resource sequence from the
 * original executable. A loose 320x200 header is deliberately insufficient. */
int csb_v1_fmtowns_switch_parse(const uint8_t *executable,
                                size_t executable_size,
                                CSB_V1_FmtownsSwitchReceipt *out);

/* Decodes one source-owned page. out_pixels must contain 320*200 bytes. */
int csb_v1_fmtowns_switch_decode_page(const uint8_t *executable,
                                      size_t executable_size,
                                      const CSB_V1_FmtownsSwitchReceipt *receipt,
                                      CSB_V1_FmtownsSwitchLanguage language,
                                      uint8_t *out_pixels,
                                      size_t out_pixel_capacity,
                                      CSB_V1_FmtownsItemDecodeReceipt *out);

/* Expands the complete source-owned switch page: the language page followed
 * by its four F2279 button bitmaps. No host-drawn text or replacement art is
 * accepted here. */
int csb_v1_fmtowns_switch_compose_page(const uint8_t *executable,
                                       size_t executable_size,
                                       const CSB_V1_FmtownsSwitchReceipt *receipt,
                                       CSB_V1_FmtownsSwitchLanguage language,
                                       uint8_t *out_pixels,
                                       size_t out_pixel_capacity);

/* ReDMCSB SWITCH.C main() sends buttons 1..3 through exit(), while button
 * four restarts the loop in the other language. AUTOEXEC.BAT maps exits
 * 1/4 to ANIMTW STORY.ANM, 2/5 to UTILJ/UTILE and 3/6 to CHTWJ/CHTWE. */
int csb_v1_fmtowns_switch_route_click(const CSB_V1_FmtownsSwitchReceipt *receipt,
                                      CSB_V1_FmtownsSwitchLanguage language,
                                      int16_t x, int16_t y,
                                      int left_button_down,
                                      CSB_V1_FmtownsSwitchInputReceipt *out);

#ifdef __cplusplus
}
#endif

#endif /* CSB_V1_FMTOWNS_SWITCH_H */
