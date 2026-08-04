/*
 * dm2_v1_mcursor_pc34_compat.c — DM2 mouse cursor graphics.
 *
 * Source: skproject c_mcursor.cpp
 * Reference: c_mcursor.h (s_cursor2, s_mcursor)
 *
 * Functions:
 *   generate_cursor      — convert 4bpp/8bpp source into cursor bitmap
 *   init_basic_cursors   — set up arrow (idx 0) and hand (idx 1)
 */

#include "dm2_v1_mcursor_pc34_compat.h"
#include <string.h>

/* MK_EVEN — round up to even (skproject macro) */
#define MK_EVEN(x) (((x) + 1) & ~1)

DM2_V1_GenerateCursorReceipt dm2_v1_generate_cursor(
    DM2_V1_MCursor cursors[DM2_V1_MCURSOR_MAX],
    const uint8_t *srcmap, int16_t cursor_idx,
    int16_t hx, int16_t hy, int16_t w, int16_t h,
    bool is_4bpp, const uint8_t *palette, uint8_t mask_idx)
{
    DM2_V1_GenerateCursorReceipt receipt;
    memset(&receipt, 0, sizeof(receipt));

    if (cursor_idx < 0 || cursor_idx >= DM2_V1_MCURSOR_MAX) return receipt;
    if (!cursors || !srcmap) return receipt;

    DM2_V1_MCursor *cursor = &cursors[cursor_idx];

    if (is_4bpp) {
        if (!palette) return receipt;
        int i = 0;
        for (int16_t y = 0; y < h; y++) {
            for (int16_t x = 0; x < w; x++, i++) {
                int16_t wis = (int16_t)(MK_EVEN(w) * y + x);
                uint8_t byte = srcmap[wis >> 1];
                uint8_t nibble;
                if ((wis & 1) == 0)
                    nibble = (byte >> 4) & 0x0f;  /* high nibble */
                else
                    nibble = byte & 0x0f;          /* low nibble */
                cursor->pixel[i] = palette[nibble];
            }
        }
        receipt.alphamask = (int8_t)palette[mask_idx];
    } else {
        int count = w * h;
        for (int i = 0; i < count; i++)
            cursor->pixel[i] = srcmap[i];
        receipt.alphamask = (int8_t)mask_idx;
    }

    cursor->hx = (int8_t)hx;
    cursor->hy = (int8_t)hy;
    cursor->w  = (int8_t)w;
    cursor->h  = (int8_t)h;
    cursor->alphamask = receipt.alphamask;

    receipt.generated = true;
    receipt.cursor_idx = (int8_t)cursor_idx;
    receipt.hotspot_x = (int8_t)hx;
    receipt.hotspot_y = (int8_t)hy;
    receipt.width = (int8_t)w;
    receipt.height = (int8_t)h;
    return receipt;
}

DM2_V1_InitBasicCursorsReceipt dm2_v1_init_basic_cursors(
    DM2_V1_MCursor cursors[DM2_V1_MCURSOR_MAX],
    const uint8_t *cursor1_data,
    const DM2_V1_Cursor2 *cursor2_data,
    const uint8_t *palette)
{
    DM2_V1_InitBasicCursorsReceipt receipt;
    memset(&receipt, 0, sizeof(receipt));

    if (!cursors || !cursor1_data || !cursor2_data || !palette) return receipt;

    /* Cursor 0: arrow from mouse1.dat — 12x16, 4bpp, mask index 12 */
    DM2_V1_GenerateCursorReceipt r0 = dm2_v1_generate_cursor(
        cursors, cursor1_data, 0, 0, 0, 12, 16, true, palette, 12);

    /* Cursor 1: hand from mouse2.dat — 16x16, 4bpp, mask index 12 */
    DM2_V1_GenerateCursorReceipt r1 = dm2_v1_generate_cursor(
        cursors, cursor2_data->pixel, 1, 0, 0, 16, 16, true, palette, 12);

    receipt.cursor_count = 0;
    if (r0.generated) receipt.cursor_count++;
    if (r1.generated) receipt.cursor_count++;
    receipt.initialized = (receipt.cursor_count == 2);
    return receipt;
}

DM2_V1_InitMouseCursorsReceipt dm2_v1_init_mouse_cursors(
    uint8_t *cursor1_buf, uint16_t cursor1_size,
    DM2_V1_Cursor2 *cursor2_buf,
    const DM2_V1_InitMouseCursorsCallbacks *cb, void *ctx)
{
    DM2_V1_InitMouseCursorsReceipt receipt;
    memset(&receipt, 0, sizeof(receipt));

    if (!cb || !cb->read_binary || !cursor1_buf || !cursor2_buf)
        return receipt;

    int r1 = cb->read_binary(ctx, "mouse1.dat", cursor1_buf, cursor1_size);
    receipt.file1_loaded = (r1 != 0);

    int r2 = cb->read_binary(ctx, "mouse2.dat", (uint8_t *)cursor2_buf,
                              (uint16_t)sizeof(DM2_V1_Cursor2));
    receipt.file2_loaded = (r2 != 0);

    receipt.initialized = receipt.file1_loaded && receipt.file2_loaded;
    return receipt;
}
