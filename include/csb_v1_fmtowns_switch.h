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

#ifdef __cplusplus
}
#endif

#endif /* CSB_V1_FMTOWNS_SWITCH_H */
