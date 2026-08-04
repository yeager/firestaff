/*
 * dm2_v1_gfx_blit_pc34_compat.c — DM2 graphics blitting engine.
 *
 * Source parity: skproject c_gfx_blit.cpp (37 functions, ~1260 lines).
 *
 * The c_blitter C++ class is represented as DM2_V1_BlitterState.
 * All pixel manipulation is done on raw uint8_t buffers.
 *
 * 4bpp pixel16 encoding: each byte holds two pixels.
 *   High nibble = left pixel, low nibble = right pixel.
 *   getl() = byte & 0xF0        (left pixel in high nibble position)
 *   getr() = byte & 0x0F        (right pixel in low nibble position)
 *   ltor() = (byte >> 4) & 0x0F (left pixel moved to low nibble)
 *   rtol() = (byte << 4) & 0xF0 (right pixel moved to high nibble)
 *   mkidx() for a nibble = the 4-bit value (0..15)
 *
 * 8bpp pixel256: straight uint8_t index.
 */

#include "dm2_v1_gfx_blit_pc34_compat.h"
#include <string.h>

/* ========================================================================
 * Helpers — nibble manipulation (matching c_pixel16 methods)
 * ======================================================================== */

/* IS_ODD(x): true if x is odd */
#define IS_ODD(x) (((x) & 1) != 0)

/* MK_EVEN(x): round up to even */
#define MK_EVEN(x) (((x) + 1) & ~1)

/* Get left nibble (high) as full byte with low nibble zeroed */
static inline uint8_t pix16_getl(uint8_t p) { return p & 0xF0; }

/* Get right nibble (low) as full byte with high nibble zeroed */
static inline uint8_t pix16_getr(uint8_t p) { return p & 0x0F; }

/* Move left nibble to right nibble position */
static inline uint8_t pix16_ltor(uint8_t p) { return (p >> 4) & 0x0F; }

/* Move right nibble to left nibble position */
static inline uint8_t pix16_rtol(uint8_t p) { return (p << 4) & 0xF0; }

/* Build a pixel16 from left nibble value and right nibble value */
static inline uint8_t build_pixels16(uint8_t left, uint8_t right)
{
    return (left & 0xF0) | (right & 0x0F);
}

/*
 * build_pixels_masked16: apply mask from xblitb table.
 * masktable[pix16.mkidx()] gives a mask byte.
 * Bits set in mask select source nibble, bits clear select dest nibble.
 * skproject: build_pixels_masked16(src, dest, mask)
 *   mask byte has bit patterns per nibble pair:
 *   0x00 = keep both dest, 0x0F = keep left dest / use right src,
 *   0xF0 = use left src / keep right dest, 0xFF = use both src.
 */
static inline uint8_t build_pixels_masked16(uint8_t src, uint8_t dest,
                                            uint8_t mask)
{
    return (src & mask) | (dest & ~mask);
}

/* pix16.is(val): compare nibble-in-position against val */
static inline bool pix16_is(uint8_t pix, uint8_t val)
{
    return pix == val;
}

/* ========================================================================
 * Init
 * ======================================================================== */

void dm2_v1_blit_init(DM2_V1_BlitterState *st,
                      const uint8_t *xblitb_data,
                      DM2_V1_BlitCallbacks cb)
{
    if (xblitb_data != NULL)
        memcpy(st->xblitb, xblitb_data, DM2_V1_BLIT_XBLITB_SIZE);
    else
        memset(st->xblitb, 0, DM2_V1_BLIT_XBLITB_SIZE);

    st->bmpdata_src16  = NULL;
    st->bmpdata_dest16 = NULL;
    st->bmpdata_src256  = NULL;
    st->bmpdata_dest256 = NULL;
    st->bmpdata_src  = NULL;
    st->bmpdata_dest = NULL;
    st->cb = cb;
    st->initialized = true;
}

/* ========================================================================
 * 4bpp-to-4bpp line blitters
 * ======================================================================== */

/* was DM2_blit_15B5 — plain 4-to-4 line blit */
void dm2_v1_blit_blitline_44_plain(DM2_V1_BlitterState *st,
                                   uint16_t srcofs, uint16_t destofs,
                                   uint16_t pixels)
{
    uint8_t *src16  = st->bmpdata_src16;
    uint8_t *dest16 = st->bmpdata_dest16;

    if (IS_ODD(srcofs) != IS_ODD(destofs)) {
        /* Misaligned case */
        if (IS_ODD(destofs)) {
            dest16[destofs / 2] = build_pixels16(
                pix16_getl(dest16[destofs / 2]),
                pix16_ltor(src16[srcofs / 2]));
            srcofs++; destofs++; pixels--;
        }
        uint16_t si = srcofs / 2;
        uint16_t di = destofs / 2;
        while (pixels > 1) {
            dest16[di++] = build_pixels16(
                pix16_rtol(src16[si]),
                pix16_ltor(src16[si + 1]));
            si++;
            pixels -= 2;
        }
        if (pixels > 0)
            dest16[di] = build_pixels16(
                pix16_rtol(src16[si]),
                pix16_getr(dest16[di]));
    } else {
        /* Aligned case */
        if (IS_ODD(destofs)) {
            dest16[destofs / 2] = build_pixels16(
                pix16_getl(dest16[destofs / 2]),
                pix16_getr(src16[srcofs / 2]));
            srcofs++; destofs++; pixels--;
        }
        uint16_t si = srcofs / 2;
        uint16_t di = destofs / 2;
        while (pixels > 1) {
            dest16[di++] = src16[si++];
            pixels -= 2;
        }
        if (pixels > 0)
            dest16[di] = build_pixels16(
                pix16_getl(src16[si]),
                pix16_getr(dest16[di]));
    }
}

/* was R_190F — masked 4-to-4 line blit */
void dm2_v1_blit_blitline_44_masked(DM2_V1_BlitterState *st,
                                    uint16_t srcofs, uint16_t destofs,
                                    uint16_t pixels, uint8_t alpha)
{
    uint8_t *masktable = &st->xblitb[((uint16_t)alpha) << 8];
    uint8_t *src16  = st->bmpdata_src16;
    uint8_t *dest16 = st->bmpdata_dest16;

    if (IS_ODD(srcofs) == IS_ODD(destofs)) {
        if (IS_ODD(destofs)) {
            uint8_t pix = pix16_getr(src16[srcofs / 2]);
            srcofs++;
            if (!pix16_is(pix, alpha))
                dest16[destofs / 2] = build_pixels16(
                    pix16_getl(dest16[destofs / 2]), pix);
            destofs++;
            pixels--;
        }
        uint16_t si = srcofs / 2;
        uint16_t di = destofs / 2;
        while (pixels > 1) {
            uint8_t pix = src16[si++];
            dest16[di] = build_pixels_masked16(pix, dest16[di], masktable[pix]);
            di++;
            pixels -= 2;
        }
        if (pixels > 0) {
            uint8_t pix = pix16_getl(src16[si]);
            if (!pix16_is(pix, (uint8_t)(alpha << 4)))
                dest16[di] = build_pixels16(pix, pix16_getr(dest16[di]));
        }
    } else {
        if (IS_ODD(destofs)) {
            uint8_t pix = pix16_ltor(src16[srcofs / 2]);
            srcofs++;
            if (!pix16_is(pix, alpha))
                dest16[destofs / 2] = build_pixels16(
                    pix16_getl(dest16[destofs / 2]), pix);
            destofs++;
            pixels--;
        }
        uint16_t si = srcofs / 2;
        uint16_t di = destofs / 2;
        while (pixels > 1) {
            uint8_t pix = build_pixels16(
                pix16_rtol(src16[si]),
                pix16_ltor(src16[si + 1]));
            dest16[di] = build_pixels_masked16(pix, dest16[di], masktable[pix]);
            si++;
            di++;
            pixels -= 2;
        }
        if (pixels > 0) {
            uint8_t pix = pix16_getr(src16[si]);
            if (!pix16_is(pix, alpha))
                dest16[di] = build_pixels16(
                    pix16_rtol(pix),
                    pix16_getr(dest16[di]));
        }
    }
}

/* was R_1761 — mirrored 4-to-4 line blit */
void dm2_v1_blit_blitline_44_mirror(DM2_V1_BlitterState *st,
                                    uint16_t srcofs, uint16_t destofs,
                                    uint16_t pixels)
{
    uint8_t *src16  = st->bmpdata_src16;
    uint8_t *dest16 = st->bmpdata_dest16;

    srcofs += pixels - 1;

    if (IS_ODD(srcofs) != IS_ODD(destofs)) {
        if (IS_ODD(destofs)) {
            dest16[destofs / 2] = build_pixels16(
                pix16_getl(dest16[destofs / 2]),
                pix16_ltor(src16[srcofs / 2]));
            pixels--; destofs++; srcofs--;
        }
        uint16_t si = srcofs / 2;
        uint16_t di = destofs / 2;
        while (pixels > 1) {
            uint8_t pix = src16[si];
            dest16[di++] = build_pixels16(
                pix16_rtol(pix), pix16_ltor(pix));
            si--;
            pixels -= 2;
        }
        if (pixels > 0)
            dest16[di] = build_pixels16(
                pix16_rtol(src16[si]),
                pix16_getr(dest16[di]));
    } else {
        if (IS_ODD(destofs)) {
            dest16[destofs / 2] = build_pixels16(
                pix16_getl(dest16[destofs / 2]),
                pix16_getr(src16[srcofs / 2]));
            pixels--; destofs++; srcofs--;
        }
        uint16_t si = srcofs / 2;
        uint16_t di = destofs / 2;
        while (pixels > 1) {
            dest16[di++] = build_pixels16(
                pix16_getl(src16[si]),
                pix16_getr(src16[si - 1]));
            si--;
            pixels -= 2;
        }
        if (pixels > 0)
            dest16[di] = build_pixels16(
                pix16_getl(src16[si]),
                pix16_getr(dest16[di]));
    }
}

/* was SKW_FIRE_BLIT_TO_MEMORY_ROW_4TO4BPP — mirrored+masked 4-to-4 */
void dm2_v1_blit_blitline_44_mirror_masked(DM2_V1_BlitterState *st,
                                           uint16_t srcofs, uint16_t destofs,
                                           uint16_t pixels, uint8_t alpha)
{
    uint8_t *masktable = &st->xblitb[((uint16_t)alpha) << 8];
    uint8_t *src16  = st->bmpdata_src16;
    uint8_t *dest16 = st->bmpdata_dest16;

    srcofs += pixels - 1;

    if (IS_ODD(srcofs) != IS_ODD(destofs)) {
        if (IS_ODD(destofs)) {
            uint8_t pix = pix16_ltor(src16[srcofs / 2]);
            if (!pix16_is(pix, alpha))
                dest16[destofs / 2] = build_pixels16(
                    pix16_getl(dest16[destofs / 2]), pix);
            destofs++; srcofs--; pixels--;
        }
        uint16_t si = srcofs / 2;
        uint16_t di = destofs / 2;
        while (pixels > 1) {
            uint8_t spix = src16[si--];
            uint8_t dpix = build_pixels16(
                pix16_rtol(spix), pix16_ltor(spix));
            uint8_t olddpix = dest16[di];
            dest16[di++] = build_pixels_masked16(dpix, olddpix,
                                                 masktable[dpix]);
            pixels -= 2;
        }
        if (pixels > 0) {
            uint8_t pix = pix16_getr(src16[si]);
            if (!pix16_is(pix, alpha))
                dest16[di] = build_pixels16(
                    pix16_rtol(pix), pix16_getr(dest16[di]));
        }
    } else {
        if (IS_ODD(destofs)) {
            uint8_t pix = pix16_getr(src16[srcofs / 2]);
            if (!pix16_is(pix, alpha))
                dest16[destofs / 2] = build_pixels16(
                    pix16_getl(dest16[destofs / 2]), pix);
            destofs++; srcofs--; pixels--;
        }
        uint16_t si = srcofs / 2;
        uint16_t di = destofs / 2;
        while (pixels > 1) {
            uint8_t pix = build_pixels16(
                pix16_getl(src16[si]),
                pix16_getr(src16[si - 1]));
            uint8_t olddpix = dest16[di++];
            dest16[di] = build_pixels_masked16(pix, olddpix,
                                               masktable[pix]);
            si--;
            pixels -= 2;
        }
        if (pixels > 0) {
            uint8_t pix = pix16_getl(src16[si]);
            if (pix16_is(pix, (uint8_t)(alpha << 4)))
                dest16[di] = build_pixels16(pix, pix16_getr(dest16[di]));
        }
    }
}

/* was SKW_FIRE_BLIT_TO_MEMORY_4TO4BPP — top-level 4-to-4 dispatcher */
void dm2_v1_blit_blitline_44(DM2_V1_BlitterState *st,
                             uint8_t *srcgfx, uint8_t *destgfx,
                             DM2_V1_BlitRect *blitrect,
                             int16_t srcx, int16_t srcy,
                             uint16_t srcw, uint16_t destw,
                             int16_t alphamask, int blitmode)
{
    if (blitrect->w == 0 || blitrect->h == 0)
        return;

    st->bmpdata_src16  = srcgfx;
    st->bmpdata_dest16 = destgfx;

    srcw  = MK_EVEN(srcw);
    destw = MK_EVEN(destw);

    uint16_t srcofs  = (uint16_t)srcx;
    uint16_t destofs = (uint16_t)(blitrect->y * destw + blitrect->x);
    uint16_t pixels  = (uint16_t)blitrect->w;
    uint8_t  alpha   = (uint8_t)alphamask;

    int16_t i;

    switch (blitmode) {
    case DM2_V1_BLITMODE_HMIRROR:
        srcofs += (uint16_t)(srcy * srcw);
        for (i = 0; i < blitrect->h; i++, srcofs += srcw, destofs += destw)
            if (alphamask >= 0)
                dm2_v1_blit_blitline_44_mirror_masked(st, srcofs, destofs, pixels, alpha);
            else
                dm2_v1_blit_blitline_44_mirror(st, srcofs, destofs, pixels);
        break;

    case DM2_V1_BLITMODE_VMIRROR:
        srcofs += (uint16_t)((srcy + blitrect->h - 1) * srcw);
        for (i = 0; i < blitrect->h; i++, srcofs -= srcw, destofs += destw)
            if (alphamask >= 0)
                dm2_v1_blit_blitline_44_masked(st, srcofs, destofs, pixels, alpha);
            else
                dm2_v1_blit_blitline_44_plain(st, srcofs, destofs, pixels);
        break;

    case DM2_V1_BLITMODE_HVMIRROR:
        srcofs += (uint16_t)((srcy + blitrect->h - 1) * srcw);
        for (i = 0; i < blitrect->h; i++, srcofs -= srcw, destofs += destw)
            if (alphamask >= 0)
                dm2_v1_blit_blitline_44_mirror_masked(st, srcofs, destofs, pixels, alpha);
            else
                dm2_v1_blit_blitline_44_mirror(st, srcofs, destofs, pixels);
        break;

    default: /* BLITMODE_NORMAL */
        srcofs += (uint16_t)(srcy * srcw);
        for (i = 0; i < blitrect->h; i++, srcofs += srcw, destofs += destw)
            if (alphamask >= 0)
                dm2_v1_blit_blitline_44_masked(st, srcofs, destofs, pixels, alpha);
            else
                dm2_v1_blit_blitline_44_plain(st, srcofs, destofs, pixels);
        break;
    }
}

/* ========================================================================
 * 4bpp-to-8bpp line blitters
 * ======================================================================== */

/* was DM2_BLIT_TO_MEMORY_ROW_4TO8BPP_NOKEY */
void dm2_v1_blit_blitline_48_plain(DM2_V1_BlitterState *st,
                                   uint16_t srcofs, uint16_t destofs,
                                   uint16_t pixels)
{
    uint8_t *src16  = st->bmpdata_src16;
    uint8_t *dest256 = st->bmpdata_dest256;
    const uint8_t *pal = st->cb.get_pal16to256(st->cb.ctx);

    if (IS_ODD(srcofs)) {
        dest256[destofs++] = pal[pix16_ltor(src16[srcofs / 2])];
        srcofs++;
        pixels--;
    }

    uint16_t si = srcofs / 2;

    while (pixels >= 2) {
        uint8_t pix = src16[si++];
        dest256[destofs++] = pal[pix16_ltor(pix)];
        dest256[destofs++] = pal[pix16_getr(pix)];
        pixels -= 2;
    }

    if (pixels > 0)
        dest256[destofs] = pal[pix16_ltor(src16[si])];
}

/* was DM2_blit_44c8_08ae — masked 4-to-8 */
void dm2_v1_blit_blitline_48_masked(DM2_V1_BlitterState *st,
                                    uint16_t srcofs, uint16_t destofs,
                                    uint16_t pixels, uint8_t alpha)
{
    uint8_t *src16  = st->bmpdata_src16;
    uint8_t *dest256 = st->bmpdata_dest256;
    const uint8_t *pal = st->cb.get_pal16to256(st->cb.ctx);

    alpha &= 0x0F;

    if (IS_ODD(srcofs)) {
        uint8_t pix = pix16_getr(src16[srcofs / 2]);
        srcofs++;
        if (pix != alpha)
            dest256[destofs] = pal[pix];
        destofs++;
        pixels--;
    }

    uint16_t si = srcofs / 2;
    int16_t n = pixels / 2;
    uint16_t ofsstep = (uint16_t)n;

    if (n != 0) {
        uint8_t *sp = src16 + si;
        uint8_t *dp = dest256 + destofs;
        uint8_t twoalpha = (uint8_t)((alpha << 4) | alpha);

        while (n--) {
            uint8_t pix = *sp++;
            if (pix != twoalpha) {
                uint8_t lpix = pix16_ltor(pix);
                if (lpix != alpha)
                    *dp = pal[lpix];
                dp++;

                uint8_t rpix = pix16_getr(pix);
                if (rpix != alpha)
                    *dp = pal[rpix];
                dp++;
            } else {
                dp += 2;
            }
        }

        si += ofsstep;
        destofs += 2 * ofsstep;
    }

    if (IS_ODD(pixels)) {
        uint8_t pix = pix16_ltor(src16[si]);
        if (pix != alpha)
            dest256[destofs] = pal[pix];
    }
}

/* was R_2035 — mirrored 4-to-8 */
void dm2_v1_blit_blitline_48_mirror(DM2_V1_BlitterState *st,
                                    uint16_t srcofs, uint16_t destofs,
                                    uint16_t pixels)
{
    uint8_t *src16   = st->bmpdata_src16;
    uint8_t *dest256 = st->bmpdata_dest256;
    const uint8_t *pal = st->cb.get_pal16to256(st->cb.ctx);

    destofs += pixels - 1;

    if (IS_ODD(srcofs)) {
        dest256[destofs--] = pal[pix16_ltor(src16[srcofs / 2])];
        srcofs++;
        pixels--;
    }

    uint16_t si = srcofs / 2;

    while (pixels >= 2) {
        uint8_t pix = src16[si++];
        dest256[destofs--] = pal[pix16_ltor(pix)];
        dest256[destofs--] = pal[pix16_getr(pix)];
        pixels -= 2;
    }

    if (pixels > 0)
        dest256[destofs] = pal[pix16_ltor(src16[si])];
}

/* was SKW_FIRE_BLIT_TO_MEMORY_ROW_4TO8BPP — mirrored+masked 4-to-8 */
void dm2_v1_blit_blitline_48_mirror_masked(DM2_V1_BlitterState *st,
                                           uint16_t srcofs, uint16_t destofs,
                                           uint16_t pixels, uint8_t alpha)
{
    uint8_t *src16   = st->bmpdata_src16;
    uint8_t *dest256 = st->bmpdata_dest256;
    const uint8_t *pal = st->cb.get_pal16to256(st->cb.ctx);

    destofs += pixels - 1;

    if (IS_ODD(srcofs)) {
        uint8_t pix = pix16_getr(src16[srcofs / 2]);
        srcofs++;
        if (pix != alpha)
            dest256[destofs] = pal[pix];
        destofs--;
        pixels--;
    }

    uint16_t si = srcofs / 2;

    while (pixels >= 2) {
        uint8_t pix = src16[si++];
        uint8_t lpix = pix16_ltor(pix);
        if (lpix != alpha)
            dest256[destofs] = pal[lpix];
        destofs--;

        uint8_t rpix = pix16_getr(pix);
        if (rpix != alpha)
            dest256[destofs] = pal[rpix];
        destofs--;

        pixels -= 2;
    }

    if (pixels > 0) {
        uint8_t pix = pix16_ltor(src16[si]);
        if (pix != alpha)
            dest256[destofs] = pal[pix];
    }
}

/* was SKW_FIRE_BLIT_TO_MEMORY_4TO8BPP — top-level 4-to-8 dispatcher */
void dm2_v1_blit_blitline_48(DM2_V1_BlitterState *st,
                             uint8_t *srcgfx, uint8_t *destgfx,
                             DM2_V1_BlitRect *blitrect,
                             int16_t srcofs_in, int16_t srcy,
                             uint16_t srcw, uint16_t destw,
                             int16_t alphamask, int blitmode,
                             const uint8_t *palette)
{
    if (blitrect->w == 0 || blitrect->h == 0)
        return;

    if (st->cb.update_blit_palette)
        st->cb.update_blit_palette(st->cb.ctx, palette);

    st->bmpdata_src16  = srcgfx;
    st->bmpdata_dest256 = destgfx;

    srcw = MK_EVEN(srcw);
    uint16_t srcofs  = (uint16_t)srcofs_in;
    uint16_t destofs = (uint16_t)(blitrect->y * destw + blitrect->x);
    uint16_t pixels  = (uint16_t)blitrect->w;
    uint8_t  alpha   = (uint8_t)alphamask;
    int16_t i;

    switch (blitmode) {
    case DM2_V1_BLITMODE_HMIRROR:
        srcofs += (uint16_t)(srcy * srcw);
        for (i = 0; i < blitrect->h; i++, srcofs += srcw, destofs += destw)
            if (alphamask >= 0)
                dm2_v1_blit_blitline_48_mirror_masked(st, srcofs, destofs, pixels, alpha);
            else
                dm2_v1_blit_blitline_48_mirror(st, srcofs, destofs, pixels);
        break;

    case DM2_V1_BLITMODE_VMIRROR:
        srcofs += (uint16_t)((srcy + blitrect->h - 1) * srcw);
        for (i = 0; i < blitrect->h; i++, srcofs -= srcw, destofs += destw)
            if (alphamask >= 0)
                dm2_v1_blit_blitline_48_masked(st, srcofs, destofs, pixels, alpha);
            else
                dm2_v1_blit_blitline_48_plain(st, srcofs, destofs, pixels);
        break;

    case DM2_V1_BLITMODE_HVMIRROR:
        srcofs += (uint16_t)((srcy + blitrect->h - 1) * srcw);
        for (i = 0; i < blitrect->h; i++, srcofs -= srcw, destofs += destw)
            if (alphamask >= 0)
                dm2_v1_blit_blitline_48_mirror_masked(st, srcofs, destofs, pixels, alpha);
            else
                dm2_v1_blit_blitline_48_mirror(st, srcofs, destofs, pixels);
        break;

    default:
        srcofs += (uint16_t)(srcy * srcw);
        for (i = 0; i < blitrect->h; i++, srcofs += srcw, destofs += destw)
            if (alphamask >= 0)
                dm2_v1_blit_blitline_48_masked(st, srcofs, destofs, pixels, alpha);
            else
                dm2_v1_blit_blitline_48_plain(st, srcofs, destofs, pixels);
        break;
    }
}

/* ========================================================================
 * 8bpp-to-8bpp line blitters
 * ======================================================================== */

/* was SKW_44c8_0b8d, DM2_blit8pppixel */
void dm2_v1_blit_blitline_88_plain(DM2_V1_BlitterState *st,
                                   uint16_t srcofs, uint16_t destofs,
                                   uint16_t pixels)
{
    uint8_t *src  = st->bmpdata_src256 + srcofs;
    uint8_t *dest = st->bmpdata_dest256 + destofs;
    while (pixels--)
        *dest++ = *src++;
}

/* was SKW_44c8_0bc5, DM2_blit8pppixelmasked */
void dm2_v1_blit_blitline_88_masked(DM2_V1_BlitterState *st,
                                    uint16_t srcofs, uint16_t destofs,
                                    uint16_t pixels, uint8_t alpha)
{
    uint8_t *src  = st->bmpdata_src256 + srcofs;
    uint8_t *dest = st->bmpdata_dest256 + destofs;
    while (pixels--) {
        uint8_t pix = *src++;
        if (pix != alpha)
            *dest = pix;
        dest++;
    }
}

/* was SKW_44c8_0bf8, DM2_blit8pppixelmirrored */
void dm2_v1_blit_blitline_88_mirror(DM2_V1_BlitterState *st,
                                    uint16_t srcofs, uint16_t destofs,
                                    uint16_t pixels)
{
    uint8_t *src  = st->bmpdata_src256 + srcofs + pixels - 1;
    uint8_t *dest = st->bmpdata_dest256 + destofs;
    while (pixels--)
        *dest++ = *src--;
}

/* was SKW_44c8_0c3c, DM2_blit8pppixelmirroredmasked */
void dm2_v1_blit_blitline_88_mirror_masked(DM2_V1_BlitterState *st,
                                           uint16_t srcofs, uint16_t destofs,
                                           uint16_t pixels, uint8_t alpha)
{
    uint8_t *src  = st->bmpdata_src256 + srcofs + pixels - 1;
    uint8_t *dest = st->bmpdata_dest256 + destofs;
    while (pixels--) {
        uint8_t pix = *src--;
        if (pix != alpha)
            *dest = pix;
        dest++;
    }
}

/* was SKW_FIRE_BLIT_TO_MEMORY_8TO8BPP — top-level 8-to-8 dispatcher */
void dm2_v1_blit_blitline_88(DM2_V1_BlitterState *st,
                             uint8_t *srcgfx, uint8_t *destgfx,
                             DM2_V1_BlitRect *blitrect,
                             int16_t srcx, int16_t srcy,
                             uint16_t srcw, uint16_t destw,
                             int16_t alphamask, int blitmode)
{
    if (blitrect->w <= 0 || blitrect->h <= 0)
        return;

    st->bmpdata_src256  = srcgfx;
    st->bmpdata_dest256 = destgfx;

    uint16_t srcofs  = (uint16_t)srcx;
    uint16_t destofs = (uint16_t)(blitrect->y * destw + blitrect->x);
    uint16_t pixels  = (uint16_t)blitrect->w;
    uint8_t  alpha   = (uint8_t)alphamask;
    int16_t i;

    switch (blitmode) {
    case DM2_V1_BLITMODE_HMIRROR:
        srcofs += (uint16_t)(srcy * srcw);
        for (i = 0; i < blitrect->h; i++, srcofs += srcw, destofs += destw)
            if (alphamask >= 0)
                dm2_v1_blit_blitline_88_mirror_masked(st, srcofs, destofs, pixels, alpha);
            else
                dm2_v1_blit_blitline_88_mirror(st, srcofs, destofs, pixels);
        break;

    case DM2_V1_BLITMODE_VMIRROR:
        srcofs += (uint16_t)((srcy + blitrect->h - 1) * srcw);
        for (i = 0; i < blitrect->h; i++, srcofs -= srcw, destofs += destw)
            if (alphamask >= 0)
                dm2_v1_blit_blitline_88_masked(st, srcofs, destofs, pixels, alpha);
            else
                dm2_v1_blit_blitline_88_plain(st, srcofs, destofs, pixels);
        break;

    case DM2_V1_BLITMODE_HVMIRROR:
        srcofs += (uint16_t)((srcy + blitrect->h - 1) * srcw);
        for (i = 0; i < blitrect->h; i++, srcofs -= srcw, destofs += destw)
            if (alphamask >= 0)
                dm2_v1_blit_blitline_88_mirror_masked(st, srcofs, destofs, pixels, alpha);
            else
                dm2_v1_blit_blitline_88_mirror(st, srcofs, destofs, pixels);
        break;

    default:
        srcofs += (uint16_t)(srcy * srcw);
        for (i = 0; i < blitrect->h; i++, srcofs += srcw, destofs += destw)
            if (alphamask >= 0)
                dm2_v1_blit_blitline_88_masked(st, srcofs, destofs, pixels, alpha);
            else
                dm2_v1_blit_blitline_88_plain(st, srcofs, destofs, pixels);
        break;
    }
}

/* ========================================================================
 * 8bpp-to-8bpp with palette translation
 * ======================================================================== */

/* was R_2871 */
void dm2_v1_blit_blitline_88xlat_plain(DM2_V1_BlitterState *st,
                                       uint16_t srcofs, uint16_t destofs,
                                       uint16_t pixels,
                                       const uint8_t *palette)
{
    uint8_t *src  = st->bmpdata_src256 + srcofs;
    uint8_t *dest = st->bmpdata_dest256 + destofs;
    while (pixels--)
        *dest++ = palette[*src++];
}

/* was R_28A2 */
void dm2_v1_blit_blitline_88xlat_masked(DM2_V1_BlitterState *st,
                                        uint16_t srcofs, uint16_t destofs,
                                        uint16_t pixels, uint8_t alpha,
                                        const uint8_t *palette)
{
    uint8_t *src  = st->bmpdata_src256 + srcofs;
    uint8_t *dest = st->bmpdata_dest256 + destofs;
    while (pixels--) {
        uint8_t pix = *src++;
        if (pix != alpha)
            *dest = palette[pix];
        dest++;
    }
}

/* was R_28DF */
void dm2_v1_blit_blitline_88xlat_mirror(DM2_V1_BlitterState *st,
                                        uint16_t srcofs, uint16_t destofs,
                                        uint16_t pixels,
                                        const uint8_t *palette)
{
    uint8_t *src  = st->bmpdata_src256 + srcofs + pixels - 1;
    uint8_t *dest = st->bmpdata_dest256 + destofs;
    while (pixels--)
        *dest++ = palette[*src--];
}

/* was R_291B */
void dm2_v1_blit_blitline_88xlat_mirror_masked(DM2_V1_BlitterState *st,
                                               uint16_t srcofs,
                                               uint16_t destofs,
                                               uint16_t pixels, uint8_t alpha,
                                               const uint8_t *palette)
{
    uint8_t *src  = st->bmpdata_src256 + srcofs + pixels - 1;
    uint8_t *dest = st->bmpdata_dest256 + destofs;
    while (pixels--) {
        uint8_t pix = *src--;
        if (pix != alpha)
            *dest = palette[pix];
        dest++;
    }
}

/* was SKW_FIRE_BLIT_TO_MEMORY_8TO8BPP_TRANSLATED */
void dm2_v1_blit_blitline_88xlat(DM2_V1_BlitterState *st,
                                 uint8_t *srcgfx, uint8_t *destgfx,
                                 DM2_V1_BlitRect *blitrect,
                                 int16_t srcx, int16_t srcy,
                                 uint16_t srcw, uint16_t destw,
                                 int16_t alphamask, int blitmode,
                                 const uint8_t *palette)
{
    if (blitrect->w == 0 || blitrect->h == 0)
        return;

    st->bmpdata_src256  = srcgfx;
    st->bmpdata_dest256 = destgfx;

    uint16_t srcofs  = (uint16_t)srcx;
    uint16_t destofs = (uint16_t)(blitrect->y * destw + blitrect->x);
    uint16_t pixels  = (uint16_t)blitrect->w;
    uint8_t  alpha   = (uint8_t)alphamask;
    int16_t i;

    switch (blitmode) {
    case DM2_V1_BLITMODE_HMIRROR:
        srcofs += (uint16_t)(srcy * srcw);
        for (i = 0; i < blitrect->h; i++, srcofs += srcw, destofs += destw)
            if (alphamask >= 0)
                dm2_v1_blit_blitline_88xlat_mirror_masked(st, srcofs, destofs, pixels, alpha, palette);
            else
                dm2_v1_blit_blitline_88xlat_mirror(st, srcofs, destofs, pixels, palette);
        break;

    case DM2_V1_BLITMODE_VMIRROR:
        srcofs += (uint16_t)((srcy + blitrect->h - 1) * srcw);
        for (i = 0; i < blitrect->h; i++, srcofs -= srcw, destofs += destw)
            if (alphamask >= 0)
                dm2_v1_blit_blitline_88xlat_masked(st, srcofs, destofs, pixels, alpha, palette);
            else
                dm2_v1_blit_blitline_88xlat_plain(st, srcofs, destofs, pixels, palette);
        break;

    case DM2_V1_BLITMODE_HVMIRROR:
        srcofs += (uint16_t)((srcy + blitrect->h - 1) * srcw);
        for (i = 0; i < blitrect->h; i++, srcofs -= srcw, destofs += destw)
            if (alphamask >= 0)
                dm2_v1_blit_blitline_88xlat_mirror_masked(st, srcofs, destofs, pixels, alpha, palette);
            else
                dm2_v1_blit_blitline_88xlat_mirror(st, srcofs, destofs, pixels, palette);
        break;

    default:
        srcofs += (uint16_t)(srcy * srcw);
        for (i = 0; i < blitrect->h; i++, srcofs += srcw, destofs += destw)
            if (alphamask >= 0)
                dm2_v1_blit_blitline_88xlat_masked(st, srcofs, destofs, pixels, alpha, palette);
            else
                dm2_v1_blit_blitline_88xlat_plain(st, srcofs, destofs, pixels, palette);
        break;
    }
}

/* ========================================================================
 * Unified blit dispatcher (was DM2_BLIT_PICTURE)
 * ======================================================================== */

void dm2_v1_blit_picture(DM2_V1_BlitterState *st,
                         uint8_t *srcgfx, uint8_t *destgfx,
                         DM2_V1_BlitRect *blitrect,
                         int16_t srcx, int16_t srcy,
                         uint16_t srcw, uint16_t destw,
                         int16_t alphamask, int blitmode,
                         int src_bpp, int dest_bpp,
                         const uint8_t *palette)
{
    if (blitrect == NULL)
        return;

    if (src_bpp == DM2_V1_BPP_4) {
        if (dest_bpp == DM2_V1_BPP_4)
            dm2_v1_blit_blitline_44(st, srcgfx, destgfx, blitrect,
                                    srcx, srcy, srcw, destw,
                                    alphamask, blitmode);
        else
            dm2_v1_blit_blitline_48(st, srcgfx, destgfx, blitrect,
                                    srcx, srcy, srcw, destw,
                                    alphamask, blitmode, palette);
    } else {
        if (palette == NULL)
            dm2_v1_blit_blitline_88(st, srcgfx, destgfx, blitrect,
                                    srcx, srcy, srcw, destw,
                                    alphamask, blitmode);
        else
            dm2_v1_blit_blitline_88xlat(st, srcgfx, destgfx, blitrect,
                                        srcx, srcy, srcw, destw,
                                        alphamask, blitmode, palette);
    }
}

/* ========================================================================
 * Fill operations
 * ======================================================================== */

/* was DM2_FILL_4BPP_PICT_LINE */
void dm2_v1_blit_fill_line_4(DM2_V1_BlitterState *st,
                             uint16_t ofs, uint16_t pixels, uint8_t pix16)
{
    uint8_t *dest = st->bmpdata_dest16;
    bool flag = IS_ODD(ofs);
    ofs /= 2;

    if (flag) {
        dest[ofs] = build_pixels16(pix16_getl(dest[ofs]), pix16_getr(pix16));
        ofs++;
        pixels--;
    }

    uint8_t dpix = build_pixels16(pix16_rtol(pix16), pix16_getr(pix16));
    while (pixels >= 2) {
        dest[ofs++] = dpix;
        pixels -= 2;
    }

    if (pixels > 0)
        dest[ofs] = build_pixels16(pix16_rtol(pix16), pix16_getr(dest[ofs]));
}

/* was DM2_FILL_8BPP_PICT_LINE */
void dm2_v1_blit_fill_line_8(DM2_V1_BlitterState *st,
                             uint16_t ofs, uint16_t pixels, uint8_t pixel256)
{
    uint8_t *dest = st->bmpdata_dest256 + ofs;

    /* Fill all pixels byte-by-byte (the original uses word-aligned writes,
     * but the result is identical) */
    while (pixels--)
        *dest++ = pixel256;
}

/* was DM2_FILL_RECT_4BPP_PICT */
void dm2_v1_blit_fill_4(DM2_V1_BlitterState *st,
                        uint8_t *gfxdata, uint8_t pix,
                        uint16_t stride_pixels, DM2_V1_BlitRect *blitrect)
{
    st->bmpdata_dest16 = gfxdata;
    uint16_t width = MK_EVEN(stride_pixels + 1);
    uint16_t ofs = (uint16_t)(blitrect->y * width + blitrect->x);

    for (int16_t j = 0; j < blitrect->h; j++, ofs += width)
        dm2_v1_blit_fill_line_4(st, ofs, (uint16_t)blitrect->w, pix);
}

/* was DM2_FILL_RECT_8BPP_PICT */
void dm2_v1_blit_fill_8(DM2_V1_BlitterState *st,
                        uint8_t *gfxdata, uint8_t pixel256,
                        uint16_t stride_pixels, DM2_V1_BlitRect *blitrect)
{
    st->bmpdata_dest256 = gfxdata;
    uint16_t ofs = (uint16_t)(blitrect->y * stride_pixels + blitrect->x);

    for (int16_t lines = 0; lines < blitrect->h; lines++, ofs += stride_pixels)
        dm2_v1_blit_fill_line_8(st, ofs, (uint16_t)blitrect->w, pixel256);
}

/* was DM2_FILL_RECT_ANY */
void dm2_v1_blit_fill(DM2_V1_BlitterState *st,
                      uint8_t *gfxdata, uint8_t pix,
                      uint16_t stride_pixels, DM2_V1_BlitRect *blitrect,
                      int bpp)
{
    if (bpp == DM2_V1_BPP_4)
        dm2_v1_blit_fill_4(st, gfxdata, pix, stride_pixels, blitrect);
    else
        dm2_v1_blit_fill_8(st, gfxdata, pix, stride_pixels, blitrect);
}

/* ========================================================================
 * Stretch blit
 * ======================================================================== */

/* was DM2_CALC_STRETCHED_SIZE */
int32_t dm2_v1_blit_calc_stretched_size(int16_t eaxw, int16_t edxw)
{
    int32_t longrg2 = (int32_t)eaxw;
    int32_t longrg1 = (int32_t)edxw;
    longrg2 *= longrg1;
    longrg1 >>= 1;
    longrg1 += longrg2;
    longrg1 >>= 6;
    return longrg1;
}

/* was SKW_44c8_20e5 — stretch sub2 (no stretchptr) */
static void stretch16_sub2(DM2_V1_BlitterState *st,
                           uint16_t srcofs, uint16_t destofs,
                           uint16_t srcfrac, int16_t pixels)
{
    uint8_t *src16  = st->bmpdata_src16;
    uint8_t *dest16 = st->bmpdata_dest16;

    uint16_t srcbase = srcfrac / 2;
    uint16_t dest = destofs / 2;
    pixels = (pixels + 1) / 2;

    while (--pixels != -1) {
        uint16_t src = (srcbase >> 7) + srcofs;
        uint8_t umask;
        if (IS_ODD(src))
            umask = pix16_rtol(src16[src / 2]);
        else
            umask = pix16_getl(src16[src / 2]);
        srcbase += srcfrac;

        src = (srcbase >> 7) + srcofs;
        if (IS_ODD(src))
            dest16[dest] = pix16_getr(src16[src / 2]) | umask;
        else
            dest16[dest] = pix16_ltor(src16[src / 2]) | umask;
        srcbase += srcfrac;
        dest++;
    }
}

/* was SKW_44c8_2143 — stretch sub1 (with stretchptr, never called in practice) */
static void stretch16_sub1(DM2_V1_BlitterState *st,
                           uint8_t *stretchptr,
                           uint16_t srcofs, uint16_t destofs,
                           uint16_t srcfrac, int16_t pixels)
{
    uint8_t *src16  = st->bmpdata_src16;
    uint8_t *dest16 = st->bmpdata_dest16;

    uint16_t srcplus = srcfrac / 2;
    uint16_t dest = destofs / 2;
    uint16_t destlim = (destofs + pixels + 1) / 2;

    while (dest < destlim) {
        uint16_t src = srcofs + (srcplus >> 7);
        uint8_t umask;
        if (IS_ODD(src))
            umask = stretchptr[pix16_getr(src16[src / 2])];
        else
            umask = stretchptr[pix16_ltor(src16[src / 2])];
        srcplus += srcfrac;
        umask <<= 4;

        src = srcofs + (srcplus >> 7);
        if (IS_ODD(src))
            dest16[dest] = stretchptr[pix16_getr(src16[src / 2])] | umask;
        else
            dest16[dest] = stretchptr[pix16_ltor(src16[src / 2])] | umask;
        srcplus += srcfrac;
        dest++;
    }
}

/* was DM2_STRETCH_BLIT_TO_MEMORY_4TO4BPP */
void dm2_v1_blit_stretch16(DM2_V1_BlitterState *st,
                           uint8_t *srcgfx, uint8_t *destgfx,
                           int16_t width, int16_t height,
                           int16_t xpixels, int16_t totalpixels,
                           uint8_t *stretchptr)
{
    int16_t olddeltax = 0;

    st->bmpdata_src16  = srcgfx;
    st->bmpdata_dest16 = destgfx;
    int16_t evenx = MK_EVEN(xpixels);
    uint16_t srcfrac = (uint16_t)((width << 7) / xpixels);
    uint16_t deltay = (uint16_t)((height << 7) / totalpixels);
    uint16_t srcbase = deltay / 2;
    width = MK_EVEN(width);

    for (int16_t n = 0; n < totalpixels; n++) {
        uint16_t deltax = srcbase >> 7;
        if (deltax != (uint16_t)olddeltax || n == 0) {
            if (stretchptr != NULL)
                stretch16_sub1(st, stretchptr,
                               (uint16_t)(deltax * width),
                               (uint16_t)(n * evenx), srcfrac, xpixels);
            else
                stretch16_sub2(st,
                               (uint16_t)(deltax * width),
                               (uint16_t)(n * evenx), srcfrac, xpixels);
        } else {
            st->bmpdata_src16 = destgfx;
            dm2_v1_blit_blitline_44_plain(st,
                (uint16_t)((n - 1) * evenx),
                (uint16_t)(n * evenx),
                (uint16_t)xpixels);
            st->bmpdata_src16 = srcgfx;
        }
        olddeltax = (int16_t)deltax;
        srcbase += deltay;
    }
}

/* was DM2_image_44c8_2351 — stretch 8bpp */
void dm2_v1_blit_stretch256(DM2_V1_BlitterState *st,
                            uint8_t *srcgfx, uint8_t *destgfx,
                            int16_t width, int16_t height,
                            int16_t argw0, int16_t argw1)
{
    int16_t rg51 = 0;
    uint8_t *gfx1 = destgfx;
    st->bmpdata_src256  = srcgfx;
    st->bmpdata_dest256 = destgfx;

    int16_t vw_04 = (int16_t)(((int32_t)width << 7) / (int32_t)argw0);
    int16_t vw_10 = (int16_t)(((int32_t)height << 7) / (int32_t)argw1);
    int16_t vw_08 = vw_10 / 2;

    for (int16_t vw_0c = 0; vw_0c < argw1; vw_0c++) {
        int16_t vw_18 = (int16_t)((uint16_t)vw_08 >> 7);
        if (rg51 != vw_18 || vw_0c == 0) {
            /* Fractional stepping */
            uint16_t rg1w = (uint16_t)vw_04;
            int16_t count = argw0;
            uint8_t *gfx1x = gfx1 + (int32_t)argw0 * vw_0c;
            int16_t base = vw_18 * width;
            uint8_t *rg52p = srcgfx + (uint16_t)base + (rg1w >> 8);
            uint8_t rg4bhi = (uint8_t)(rg1w & 0xFF);
            rg1w *= 2;
            uint8_t rg4blo = (uint8_t)(rg1w & 0xFF);
            uint8_t rg2blo = (uint8_t)((rg1w >> 8) & 0xFF);

            for (;;) {
                *gfx1x++ = *rg52p;
                uint16_t utmp = (uint16_t)rg4bhi + (uint16_t)rg4blo;
                rg4bhi += rg4blo;
                bool carry = (utmp & 0xFF00) != 0;
                rg52p += (uint16_t)rg2blo + (carry ? 1 : 0);
                if (--count == 0)
                    break;
            }
        } else {
            st->bmpdata_src256 = gfx1;
            dm2_v1_blit_blitline_88_plain(st,
                (uint16_t)((vw_0c - 1) * argw0),
                (uint16_t)(vw_0c * argw0),
                (uint16_t)argw0);
            st->bmpdata_src256 = srcgfx;
        }
        rg51 = vw_18;
        vw_08 += vw_10;
    }
}

/* ========================================================================
 * Within-screen blit (was sub_25AF)
 * ======================================================================== */

void dm2_v1_blit_within_screen(DM2_V1_BlitterState *st,
                               uint8_t *screen,
                               DM2_V1_BlitRect *rectp,
                               int16_t yofs)
{
    (void)st;
    for (int32_t wi = rectp->y + yofs;
         wi <= rectp->y + rectp->h - 1; wi++)
    {
        memcpy(screen + wi * DM2_V1_ORIG_SWIDTH,
               screen + (wi - yofs) * DM2_V1_ORIG_SWIDTH,
               (size_t)rectp->w);
    }
}

/* ========================================================================
 * Special effects blit
 * ======================================================================== */

static const int8_t table1d255a[2] = { (int8_t)0xF0, 0x0F };

void dm2_v1_blit_sub_specialeffects(DM2_V1_BlitterState *st,
                                    uint8_t *srcgfx, uint8_t *destgfx,
                                    uint8_t *gfx,
                                    DM2_V1_BlitRect *ecxrp,
                                    int16_t xend, int16_t srcofs_in,
                                    int16_t argw2, int16_t argw3,
                                    int16_t argw4, int16_t pixperline,
                                    int16_t argw6,
                                    const uint8_t *palette)
{
    if (st->cb.update_blit_palette)
        st->cb.update_blit_palette(st->cb.ctx, palette);

    int16_t width = ecxrp->w;
    int16_t height = ecxrp->h;
    int32_t pixofs = ecxrp->x + ecxrp->y * pixperline;
    int16_t pixels = xend - srcofs_in;
    int16_t srcofs = srcofs_in;

    st->bmpdata_src16 = srcgfx;
    st->bmpdata_dest256 = destgfx;

    if (gfx == NULL) {
        for (int16_t h = 0; h < height; h++) {
            int16_t w = width;
            int16_t vw_1c = 0;
            int16_t wp;
            for (;;) {
                int16_t destofs = (int16_t)(pixofs + vw_1c);
                wp = w;
                if (pixels > wp)
                    break;
                if (argw6 >= 0)
                    dm2_v1_blit_blitline_48_masked(st, (uint16_t)srcofs,
                        (uint16_t)destofs, (uint16_t)pixels,
                        (uint8_t)argw6);
                else
                    dm2_v1_blit_blitline_48_plain(st, (uint16_t)srcofs,
                        (uint16_t)destofs, (uint16_t)pixels);
                w -= pixels;
                vw_1c += pixels;
                pixels = xend;
                srcofs = 0;
            }
            if (wp != 0) {
                int16_t destofs = (int16_t)(pixofs + vw_1c);
                if (argw6 >= 0)
                    dm2_v1_blit_blitline_48_masked(st, (uint16_t)srcofs,
                        (uint16_t)destofs, (uint16_t)wp,
                        (uint8_t)argw6);
                else
                    dm2_v1_blit_blitline_48_plain(st, (uint16_t)srcofs,
                        (uint16_t)destofs, (uint16_t)wp);
                pixels -= w;
                srcofs += w;
            }
            if (IS_ODD(width)) {
                if (--pixels == 0) {
                    pixels = xend;
                    srcofs = 0;
                } else {
                    srcofs++;
                }
            }
            pixofs += pixperline;
        }
    } else {
        argw4 = MK_EVEN(argw4 + 1);
        int16_t vw_08 = argw2 + argw3 * argw4;

        for (int16_t h = 0; h < height; h++) {
            int16_t destofs_w = width;
            int16_t vw_1c = vw_08;

            for (;;) {
                if (destofs_w == 0) break;
                uint8_t rg2l = gfx[vw_1c / 2];
                if (((uint8_t)table1d255a[vw_1c & 1] & rg2l) != 0) break;
                destofs_w--;
                vw_1c++;
                srcofs++;
                if (--pixels == 0) {
                    srcofs = 0;
                    pixels = xend;
                }
            }

            vw_1c -= vw_08;
            int16_t w = destofs_w;
            destofs_w = vw_08 + width;

            for (;;) {
                destofs_w--;
                if (w == 0) break;
                uint8_t rg2l = gfx[destofs_w / 2];
                if (((uint8_t)table1d255a[destofs_w & 1] & rg2l) != 0) break;
                w--;
            }

            if (w > 0) {
                for (;;) {
                    int16_t dofs = (int16_t)(pixofs + vw_1c);
                    if (pixels > w) break;
                    if (argw6 >= 0)
                        dm2_v1_blit_blitline_48_masked(st, (uint16_t)srcofs,
                            (uint16_t)dofs, (uint16_t)pixels,
                            (uint8_t)argw6);
                    else
                        dm2_v1_blit_blitline_48_plain(st, (uint16_t)srcofs,
                            (uint16_t)dofs, (uint16_t)pixels);
                    w -= pixels;
                    vw_1c += pixels;
                    pixels = xend;
                    srcofs = 0;
                }
                if (w != 0) {
                    int16_t dofs = (int16_t)(pixofs + vw_1c);
                    if (argw6 >= 0)
                        dm2_v1_blit_blitline_48_masked(st, (uint16_t)srcofs,
                            (uint16_t)dofs, (uint16_t)w,
                            (uint8_t)argw6);
                    else
                        dm2_v1_blit_blitline_48_plain(st, (uint16_t)srcofs,
                            (uint16_t)dofs, (uint16_t)w);
                    pixels -= w;
                    srcofs += w;
                }
            }

            if (IS_ODD(width)) {
                if (--pixels == 0) {
                    pixels = xend;
                    srcofs = 0;
                } else {
                    srcofs++;
                }
            }
            vw_08 += argw4;
            pixofs += pixperline;
        }
    }
}

/* was DM2_blit_44c8_20a4 */
void dm2_v1_blit_specialeffects(DM2_V1_BlitterState *st,
                                uint8_t *srcgfx, uint8_t *destgfx,
                                uint8_t *gfx,
                                DM2_V1_BlitRect *ecxrp,
                                int16_t argw0, int16_t argw1,
                                int16_t argw2, int16_t argw3,
                                const uint8_t *palette)
{
    dm2_v1_blit_sub_specialeffects(st, srcgfx, destgfx, gfx, ecxrp,
                                   argw0, argw1, 0, 0, ecxrp->w,
                                   argw2, argw3, palette);
}

/* ========================================================================
 * Dither fill (stretch_4to8, checkerboard pattern)
 * ======================================================================== */

/* was DM2_guidraw_44c8_1aca — checkerboard dither fill */
void dm2_v1_blit_stretch_4to8(DM2_V1_BlitterState *st,
                              uint8_t *destgfx,
                              DM2_V1_BlitRect *rect,
                              uint8_t mask, int16_t width)
{
    uint8_t vca_00[160];
    uint8_t vca_a0[160];

    if (rect == NULL)
        return;

    if (st->cb.update_blit_palette && st->cb.get_default_palette)
        st->cb.update_blit_palette(st->cb.ctx,
                                   st->cb.get_default_palette(st->cb.ctx));

    uint8_t alpha = mask ^ 0x0F;
    uint8_t xmask = alpha | (uint8_t)(mask << 4);

    st->bmpdata_src16   = vca_00;
    st->bmpdata_dest256 = destgfx;

    int16_t pixels = rect->w;
    if (pixels <= 0) return;
    if (rect->h == 0) return;

    for (int16_t i = 0; i < (pixels + 1) / 2; i++) {
        vca_00[i] = xmask;
        vca_a0[i] = (uint8_t)~xmask;
    }

    int16_t destofs = rect->x + rect->y * width;
    bool flag = (((destofs / width) ^ destofs) & 1) != 0;

    for (int16_t i = 0; i < rect->h; i++, destofs += width) {
        if (flag)
            st->bmpdata_src16 = vca_a0;
        else
            st->bmpdata_src16 = vca_00;
        flag = !flag;
        dm2_v1_blit_blitline_48_masked(st, 0, (uint16_t)destofs,
                                       (uint16_t)pixels, alpha);
    }
}

/* ========================================================================
 * Parity evidence
 * ======================================================================== */

const char *dm2_v1_gfx_blit_source_evidence(void)
{
    return "skproject c_gfx_blit.cpp -> dm2_v1_gfx_blit_pc34_compat.c "
           "(37 functions: init, blitline_44 family [4], blitline_48 family [4], "
           "blitline_88 family [4], blitline_88xlat family [4], blit_picture, "
           "fill_line_4, fill_line_8, fill_4, fill_8, fill, "
           "calc_stretched_size, stretch16, stretch256, "
           "blit_within_screen, blit_specialeffects, blit_sub_specialeffects, "
           "stretch_4to8)";
}
