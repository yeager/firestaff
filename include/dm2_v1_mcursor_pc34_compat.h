#ifndef FIRESTAFF_DM2_V1_MCURSOR_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_MCURSOR_PC34_COMPAT_H

/*
 * dm2_v1_mcursor_pc34_compat.h — DM2 mouse cursor graphics.
 *
 * Source: skproject c_mcursor.cpp (3 functions).
 * Converts 4bpp/8bpp source pixel data into cursor bitmaps for
 * the DM2 mouse cursor system.
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * Constants
 * ======================================================================== */

#define DM2_V1_MOUSECURSOR_FILESIZE1  0x60
#define DM2_V1_MOUSECURSOR_FILESIZE2  0x90
#define DM2_V1_MCURSOR_PIXEL_COUNT    0x240
#define DM2_V1_MCURSOR_MAX            4

/* ========================================================================
 * Data structures
 * ======================================================================== */

typedef struct { int16_t x, y, w, h; } DM2_V1_MCursorRect;

/* s_cursor2 — secondary cursor data from mouse2.dat (size 0x90) */
typedef struct {
    uint8_t pixel[0x80];     /* 4bpp nibble pairs */
    DM2_V1_MCursorRect rect1;
    DM2_V1_MCursorRect rect2;
} DM2_V1_Cursor2;

/* s_mcursor — generated cursor bitmap (size 0x246) */
typedef struct {
    int8_t  hx;              /* @00 hotspot x */
    int8_t  hy;              /* @01 hotspot y */
    int8_t  w;               /* @02 width */
    int8_t  h;               /* @03 height */
    int8_t  alphamask;       /* @04 transparency mask */
    int8_t  dummy;           /* @05 alignment padding */
    uint8_t pixel[DM2_V1_MCURSOR_PIXEL_COUNT]; /* @06 pixel data */
} DM2_V1_MCursor;

/* ========================================================================
 * Receipts
 * ======================================================================== */

typedef struct {
    bool   generated;
    int8_t cursor_idx;
    int8_t hotspot_x, hotspot_y;
    int8_t width, height;
    int8_t alphamask;
} DM2_V1_GenerateCursorReceipt;

typedef struct {
    bool initialized;
    int  cursor_count;
} DM2_V1_InitBasicCursorsReceipt;

/* ---- init_mousecursors (c_mcursor.cpp:19) ----
 * Read mouse cursor data from binary files into buffers.
 * In the reference this reads mouse1.dat (0x60 bytes) and mouse2.dat. */
typedef struct {
    int (*read_binary)(void *ctx, const char *filename, uint8_t *buffer, uint16_t size);
} DM2_V1_InitMouseCursorsCallbacks;

typedef struct {
    bool file1_loaded;
    bool file2_loaded;
    bool initialized;
} DM2_V1_InitMouseCursorsReceipt;

/* ========================================================================
 * Functions
 * ======================================================================== */

/*
 * generate_cursor — convert source pixel data into a cursor bitmap.
 *
 * is_4bpp: true for 4-bit nibble source, false for 8-bit direct.
 * palette: color lookup table (used for 4bpp mode).
 * mask_idx: palette index for transparency (4bpp) or direct value (8bpp).
 */
DM2_V1_GenerateCursorReceipt dm2_v1_generate_cursor(
    DM2_V1_MCursor cursors[DM2_V1_MCURSOR_MAX],
    const uint8_t *srcmap, int16_t cursor_idx,
    int16_t hx, int16_t hy, int16_t w, int16_t h,
    bool is_4bpp, const uint8_t *palette, uint8_t mask_idx);

/*
 * init_basic_cursors — set up arrow (cursor 0) and hand (cursor 1).
 */
DM2_V1_InitBasicCursorsReceipt dm2_v1_init_basic_cursors(
    DM2_V1_MCursor cursors[DM2_V1_MCURSOR_MAX],
    const uint8_t *cursor1_data,
    const DM2_V1_Cursor2 *cursor2_data,
    const uint8_t *palette);

/*
 * init_mouse_cursors — load mouse cursor data from binary files via callback.
 */
DM2_V1_InitMouseCursorsReceipt dm2_v1_init_mouse_cursors(
    uint8_t *cursor1_buf, uint16_t cursor1_size,
    DM2_V1_Cursor2 *cursor2_buf,
    const DM2_V1_InitMouseCursorsCallbacks *cb, void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_MCURSOR_PC34_COMPAT_H */
