#include "dm1_v1_screen_framebuffer_pc34_compat.h"
#include <string.h>

/*
 * DM1 V1 Screen/Framebuffer — implementation
 *
 * Source lock: ReDMCSB WIP20210206
 *   PALETTE.C F0093/F0094: palette set/block set
 *   DM.C/INIT.C: initial framebuffer clear
 *   VBLANK.C: present timing
 *
 * Default DM1 palette: the original Atari ST 16-color palette.
 * Values are 3-bit per channel (0-7) scaled to 6-bit (0-63).
 */

/* Default DM1 Atari ST palette (approximation, 6-bit per channel) */
static const DM1_V1_PaletteEntryPc34 s_defaultPalette[DM1_V1_PALETTE_SIZE_PC34] = {
    { 0,  0,  0},  /*  0: black */
    { 4,  4,  4},  /*  1: dark gray */
    { 8,  8,  8},  /*  2: medium gray */
    {16, 16, 16},  /*  3: light gray */
    {24, 16,  8},  /*  4: brown */
    {32, 24, 16},  /*  5: light brown */
    {40, 32, 24},  /*  6: tan */
    {48, 40, 32},  /*  7: cream */
    {16,  8,  0},  /*  8: dark brown */
    {24, 12,  0},  /*  9: medium brown */
    {32, 20,  4},  /* 10: golden brown */
    {40, 28, 12},  /* 11: light golden */
    { 8,  8, 16},  /* 12: dark blue-gray */
    {16, 16, 24},  /* 13: blue-gray */
    {24, 24, 32},  /* 14: light blue-gray */
    {63, 63, 63},  /* 15: white */
};

void DM1_V1_Screen_InitPc34Compat(DM1_V1_ScreenStatePc34 *s)
{
    memset(s, 0, sizeof(*s));
    memcpy(s->palette, s_defaultPalette, sizeof(s_defaultPalette));
}

uint8_t *DM1_V1_Screen_GetBackBufferPc34Compat(DM1_V1_ScreenStatePc34 *s)
{
    return &s->backBuffer[0][0];
}

const uint8_t *DM1_V1_Screen_GetFrontBufferPc34Compat(const DM1_V1_ScreenStatePc34 *s)
{
    return &s->frontBuffer[0][0];
}

void DM1_V1_Screen_SwapBuffersPc34Compat(DM1_V1_ScreenStatePc34 *s)
{
    memcpy(s->frontBuffer, s->backBuffer, sizeof(s->frontBuffer));
}

void DM1_V1_Screen_PresentPc34Compat(DM1_V1_ScreenStatePc34 *s, int32_t nowMs)
{
    DM1_V1_Screen_SwapBuffersPc34Compat(s);
    s->dirty = 0;
    s->lastPresentMs = nowMs;
    s->presentCount++;
}

void DM1_V1_Screen_SetPalettePc34Compat(DM1_V1_ScreenStatePc34 *s, int idx,
                            uint8_t r, uint8_t g, uint8_t b)
{
    if (idx < 0 || idx >= DM1_V1_PALETTE_SIZE_PC34) return;
    s->palette[idx].r = r;
    s->palette[idx].g = g;
    s->palette[idx].b = b;
}

void DM1_V1_Screen_SetPaletteBlockPc34Compat(DM1_V1_ScreenStatePc34 *s, int startIdx,
                                  const DM1_V1_PaletteEntryPc34 *entries, int count)
{
    for (int i = 0; i < count; i++) {
        DM1_V1_Screen_SetPalettePc34Compat(s, startIdx + i,
                               entries[i].r, entries[i].g, entries[i].b);
    }
}

DM1_V1_PaletteEntryPc34 DM1_V1_Screen_GetPalettePc34Compat(const DM1_V1_ScreenStatePc34 *s, int idx)
{
    if (idx < 0 || idx >= DM1_V1_PALETTE_SIZE_PC34) {
        DM1_V1_PaletteEntryPc34 empty = {0, 0, 0};
        return empty;
    }
    return s->palette[idx];
}

void DM1_V1_Screen_ClearBackPc34Compat(DM1_V1_ScreenStatePc34 *s, uint8_t color)
{
    memset(s->backBuffer, color, sizeof(s->backBuffer));
    s->dirty = 1;
}

void DM1_V1_Screen_MarkDirtyPc34Compat(DM1_V1_ScreenStatePc34 *s)
{
    s->dirty = 1;
}

int DM1_V1_Screen_IsDirtyPc34Compat(const DM1_V1_ScreenStatePc34 *s)
{
    return s->dirty;
}

void DM1_V1_Screen_CopyRegionPc34Compat(DM1_V1_ScreenStatePc34 *s,
                            int sx, int sy, int dx, int dy, int w, int h)
{
    /* Clip to buffer bounds */
    if (sx < 0) { w += sx; dx -= sx; sx = 0; }
    if (sy < 0) { h += sy; dy -= sy; sy = 0; }
    if (dx < 0) { w += dx; sx -= dx; dx = 0; }
    if (dy < 0) { h += dy; sy -= dy; dy = 0; }
    if (sx + w > DM1_V1_SCREEN_W_PC34) w = DM1_V1_SCREEN_W_PC34 - sx;
    if (sy + h > DM1_V1_SCREEN_H_PC34) h = DM1_V1_SCREEN_H_PC34 - sy;
    if (dx + w > DM1_V1_SCREEN_W_PC34) w = DM1_V1_SCREEN_W_PC34 - dx;
    if (dy + h > DM1_V1_SCREEN_H_PC34) h = DM1_V1_SCREEN_H_PC34 - dy;
    if (w <= 0 || h <= 0) return;

    /* Use memmove for overlapping regions */
    if (dy < sy || (dy == sy && dx < sx)) {
        for (int row = 0; row < h; row++) {
            memmove(&s->backBuffer[dy + row][dx],
                    &s->backBuffer[sy + row][sx], (size_t)w);
        }
    } else {
        for (int row = h - 1; row >= 0; row--) {
            memmove(&s->backBuffer[dy + row][dx],
                    &s->backBuffer[sy + row][sx], (size_t)w);
        }
    }
    s->dirty = 1;
}

const char *DM1_V1_Screen_SourceEvidencePc34Compat(void)
{
    return
        "ReDMCSB WIP20210206\n"
        "PALETTE.C F0093: set single palette entry (3-bit ST → 6-bit).\n"
        "PALETTE.C F0094: set palette block.\n"
        "DM.C/INIT.C: initial framebuffer allocation and clear.\n"
        "VBLANK.C: present timing synchronized to vertical blank.\n"
        "320x200 8-bit indexed framebuffer, 16-color palette.\n"
        "Double-buffered: draw to back, present copies to front.\n"
        "Default palette: DM1 Atari ST dungeon colors (gray/brown/cream).";
}
