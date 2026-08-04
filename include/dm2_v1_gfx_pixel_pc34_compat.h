/* skproject: c_gfx_pixel.h */
#ifndef FIRESTAFF_DM2_V1_GFX_PIXEL_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_GFX_PIXEL_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* --- e_color enum: 16-color palette indices --- */
typedef enum {
    DM2_V1_E_COL00 = 0x00,
    DM2_V1_E_COL01 = 0x01,
    DM2_V1_E_COL02 = 0x02,
    DM2_V1_E_COL03 = 0x03,
    DM2_V1_E_COL04 = 0x04,
    DM2_V1_E_COL05 = 0x05,
    DM2_V1_E_COL06 = 0x06,
    DM2_V1_E_COL07 = 0x07,
    DM2_V1_E_COL08 = 0x08,
    DM2_V1_E_COL09 = 0x09,
    DM2_V1_E_COL10 = 0x0A,
    DM2_V1_E_COL11 = 0x0B,
    DM2_V1_E_COL12 = 0x0C,
    DM2_V1_E_COL13 = 0x0D,
    DM2_V1_E_COL14 = 0x0E,
    DM2_V1_E_COL15 = 0x0F,
    DM2_V1_E_COLX90 = 0x90
} DM2_V1_EColor;

/* --- t_resolution: bits per pixel --- */
typedef uint8_t DM2_V1_Resolution;
#define DM2_V1_BPP_4 4
#define DM2_V1_BPP_8 8

/* --- c_pixel: generic pixel --- */
typedef struct {
    uint8_t p;
} DM2_V1_Pixel;

/* --- c_pixel16: 4bpp packed pixel pair --- */
typedef struct {
    uint8_t p;
} DM2_V1_Pixel16;

/* --- c_pixel256: 8bpp pixel --- */
typedef struct {
    uint8_t p;
} DM2_V1_Pixel256;

/* --- Conversion --- */

static inline uint8_t dm2_v1_pixel_to_ui8(DM2_V1_Pixel px) {
    return px.p;
}

static inline DM2_V1_Pixel dm2_v1_ui8_to_pixel(uint8_t v) {
    DM2_V1_Pixel px;
    px.p = v;
    return px;
}

/* --- Pixel comparison --- */

static inline int dm2_v1_pixel_eq(DM2_V1_Pixel a, DM2_V1_Pixel b) {
    return a.p == b.p;
}

static inline int dm2_v1_pixel_ne(DM2_V1_Pixel a, DM2_V1_Pixel b) {
    return a.p != b.p;
}

static inline int dm2_v1_pixel_eq_color(DM2_V1_Pixel px, DM2_V1_EColor col) {
    return px.p == (uint8_t)col;
}

static inline int dm2_v1_pixel_ne_color(DM2_V1_Pixel px, DM2_V1_EColor col) {
    return px.p != (uint8_t)col;
}

/* --- Pixel index/identity --- */

static inline int16_t dm2_v1_pixel_mkidx(DM2_V1_Pixel px) {
    return (int16_t)px.p;
}

static inline int dm2_v1_pixel_is(DM2_V1_Pixel px, DM2_V1_EColor col) {
    return px.p == (uint8_t)col;
}

/* --- Pixel16 (4bpp packed pair) nibble operations --- */

/* Left pixel shifted to high nibble position (left-to-right order) */
static inline DM2_V1_Pixel16 dm2_v1_pixel16_ltor(DM2_V1_Pixel px) {
    DM2_V1_Pixel16 p16;
    p16.p = (uint8_t)(px.p << 4);
    return p16;
}

/* Right pixel in low nibble position (right-to-left order) */
static inline DM2_V1_Pixel16 dm2_v1_pixel16_rtol(DM2_V1_Pixel px) {
    DM2_V1_Pixel16 p16;
    p16.p = (uint8_t)(px.p & 0x0F);
    return p16;
}

/* Get left (high) nibble as pixel */
static inline DM2_V1_Pixel dm2_v1_pixel16_getl(DM2_V1_Pixel16 p16) {
    DM2_V1_Pixel px;
    px.p = (uint8_t)(p16.p >> 4);
    return px;
}

/* Get right (low) nibble as pixel */
static inline DM2_V1_Pixel dm2_v1_pixel16_getr(DM2_V1_Pixel16 p16) {
    DM2_V1_Pixel px;
    px.p = (uint8_t)(p16.p & 0x0F);
    return px;
}

/* Set both nibbles from two pixels */
static inline DM2_V1_Pixel16 dm2_v1_pixel16_set(DM2_V1_Pixel left, DM2_V1_Pixel right) {
    DM2_V1_Pixel16 p16;
    p16.p = (uint8_t)((left.p << 4) | (right.p & 0x0F));
    return p16;
}

/* Build a packed pixel pair from two color values */
static inline DM2_V1_Pixel16 dm2_v1_build_pixels16(uint8_t left, uint8_t right) {
    DM2_V1_Pixel16 p16;
    p16.p = (uint8_t)((left << 4) | (right & 0x0F));
    return p16;
}

/* Build a masked packed pixel pair: use src nibbles where mask is nonzero, else keep dst */
static inline DM2_V1_Pixel16 dm2_v1_build_pixels_masked16(DM2_V1_Pixel16 src, DM2_V1_Pixel16 dst, DM2_V1_Pixel16 mask) {
    DM2_V1_Pixel16 result;
    uint8_t left  = (mask.p & 0xF0) ? (src.p & 0xF0) : (dst.p & 0xF0);
    uint8_t right = (mask.p & 0x0F) ? (src.p & 0x0F) : (dst.p & 0x0F);
    result.p = (uint8_t)(left | right);
    return result;
}

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_GFX_PIXEL_PC34_COMPAT_H */
