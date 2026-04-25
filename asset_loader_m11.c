/*
 * asset_loader_m11.c — GRAPHICS.DAT asset loader and blitter for M11.
 *
 * Uses the M10 IMG3 pipeline to decompress graphic entries from
 * GRAPHICS.DAT into unpacked 1-byte-per-pixel buffers.
 *
 * The IMG3 expand path works on packed nibbles (4-bit, two pixels per
 * byte, high nibble first). After expansion, we unpack to 1 byte per
 * pixel for trivial framebuffer blitting.
 */

#include "asset_loader_m11.h"
#include "memory_graphics_dat_pc34_compat.h"
#include "memory_graphics_dat_state_pc34_compat.h"
#include "memory_graphics_dat_select_pc34_compat.h"
#include "memory_graphics_dat_header_pc34_compat.h"
#include "memory_frontend_pc34_compat.h"
#include "graphics_dat_entry_classify_pc34_compat.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* IMG3 global state used by the decompressor */
extern unsigned short G2157_;
extern unsigned char* G2159_puc_Bitmap_Source;
extern unsigned char* G2160_puc_Bitmap_Destination;

/* --- lifecycle --- */

int M11_AssetLoader_Init(M11_AssetLoader* loader, const char* graphicsDatPath) {
    struct MemoryGraphicsDatState_Compat* fileState;
    struct MemoryGraphicsDatRuntimeState_Compat* runtimeState;

    if (!loader || !graphicsDatPath || graphicsDatPath[0] == '\0') {
        return 0;
    }
    memset(loader, 0, sizeof(*loader));
    snprintf(loader->graphicsDatPath, sizeof(loader->graphicsDatPath), "%s", graphicsDatPath);

    fileState = (struct MemoryGraphicsDatState_Compat*)calloc(
        1, sizeof(struct MemoryGraphicsDatState_Compat));
    runtimeState = (struct MemoryGraphicsDatRuntimeState_Compat*)calloc(
        1, sizeof(struct MemoryGraphicsDatRuntimeState_Compat));
    if (!fileState || !runtimeState) {
        free(fileState);
        free(runtimeState);
        return 0;
    }

    if (!F0479_MEMORY_InitializeGraphicsDatState_Compat(
            graphicsDatPath, fileState, runtimeState)) {
        free(fileState);
        free(runtimeState);
        return 0;
    }

    /* Reopen the file for subsequent graphic loads — the header loading
     * path closes the file handle after reading the header. */
    if (!F0477_MEMORY_OpenGraphicsDat_CPSDF_Compat(graphicsDatPath, fileState)) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(runtimeState);
        free(fileState);
        free(runtimeState);
        return 0;
    }

    loader->fileState = fileState;
    loader->runtimeState = runtimeState;
    loader->graphicCount = runtimeState->graphicCount;
    loader->initialized = 1;
    return 1;
}

void M11_AssetLoader_Shutdown(M11_AssetLoader* loader) {
    int i;
    if (!loader) {
        return;
    }
    for (i = 0; i < M11_ASSET_CACHE_SLOTS; ++i) {
        if (loader->cache[i].loaded && loader->cache[i].pixels) {
            free(loader->cache[i].pixels);
        }
    }
    if (loader->runtimeState) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(
            (struct MemoryGraphicsDatRuntimeState_Compat*)loader->runtimeState);
        free(loader->runtimeState);
    }
    if (loader->fileState) {
        struct MemoryGraphicsDatState_Compat* fs =
            (struct MemoryGraphicsDatState_Compat*)loader->fileState;
        F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(fs);
        free(fs);
    }
    memset(loader, 0, sizeof(*loader));
}

int M11_AssetLoader_IsReady(const M11_AssetLoader* loader) {
    return loader && loader->initialized;
}

int M11_AssetLoader_QuerySize(const M11_AssetLoader* loader,
                              unsigned int graphicIndex,
                              unsigned short* outWidth,
                              unsigned short* outHeight) {
    const struct MemoryGraphicsDatRuntimeState_Compat* rt;
    if (!loader || !loader->initialized) {
        return 0;
    }
    rt = (const struct MemoryGraphicsDatRuntimeState_Compat*)loader->runtimeState;
    if (graphicIndex >= rt->graphicCount) {
        return 0;
    }
    if (outWidth) {
        *outWidth = rt->widthHeight[graphicIndex].Width;
    }
    if (outHeight) {
        *outHeight = rt->widthHeight[graphicIndex].Height;
    }
    return 1;
}

/* --- loading --- */

static M11_AssetSlot* m11_find_cached(M11_AssetLoader* loader,
                                      unsigned int graphicIndex) {
    int i;
    for (i = 0; i < loader->cacheUsed; ++i) {
        if (loader->cache[i].loaded &&
            loader->cache[i].graphicIndex == graphicIndex) {
            return &loader->cache[i];
        }
    }
    return NULL;
}

static M11_AssetSlot* m11_alloc_slot(M11_AssetLoader* loader) {
    if (loader->cacheUsed < M11_ASSET_CACHE_SLOTS) {
        return &loader->cache[loader->cacheUsed++];
    }
    /* Load() returns slot pointers that callers may keep until Shutdown.
     * Evicting here creates dangling pointers inside a single complex draw
     * once enough GRAPHICS.DAT entries have been touched. Treat a full cache
     * as a soft load miss instead. */
    return NULL;
}

const M11_AssetSlot* M11_AssetLoader_Load(M11_AssetLoader* loader,
                                          unsigned int graphicIndex) {
    const struct MemoryGraphicsDatRuntimeState_Compat* rt;
    struct MemoryGraphicsDatState_Compat* fs;
    struct GraphicsDatEntryClassificationResult_Compat classResult;
    struct MemoryGraphicsDatSelection_Compat selection;
    struct MemoryGraphicsDatHeader_Compat tempHeader;
    unsigned short w, h;
    unsigned long packedSize;
    unsigned long expandedCapacity;
    unsigned long unpackedSize;
    unsigned char* compressedBuf = NULL;
    unsigned char* packedBitmap = NULL;
    unsigned char* unpackedPixels = NULL;
    M11_AssetSlot* slot;
    long offset;
    unsigned long ix, iy;
    unsigned long packedStride;
    unsigned long overrunFirst;
    unsigned long overrunLast;
    int traceAssets;

    if (!loader || !loader->initialized) {
        return NULL;
    }
    traceAssets = (getenv("FIRESTAFF_ASSET_TRACE") != NULL);

    /* Check cache first */
    slot = m11_find_cached(loader, graphicIndex);
    if (slot) {
        return slot;
    }

    rt = (const struct MemoryGraphicsDatRuntimeState_Compat*)loader->runtimeState;
    fs = (struct MemoryGraphicsDatState_Compat*)loader->fileState;

    if (graphicIndex >= rt->graphicCount) {
        return NULL;
    }

    /* Classify the entry */
    memset(&classResult, 0, sizeof(classResult));
    if (!F9012_RUNTIME_ClassifyGraphicsDatEntry_Compat(rt, graphicIndex, &classResult)) {
        return NULL;
    }
    if (!classResult.shouldUseBitmapPath) {
        return NULL; /* not a bitmap-safe entry */
    }

    w = rt->widthHeight[graphicIndex].Width;
    h = rt->widthHeight[graphicIndex].Height;
    if (w == 0 || h == 0) {
        return NULL;
    }
    if (traceAssets) {
        fprintf(stderr, "asset-load %u w=%u h=%u comp=%u dec=%u\n",
                graphicIndex, (unsigned)w, (unsigned)h,
                (unsigned)rt->compressedByteCounts[graphicIndex],
                (unsigned)rt->decompressedByteCounts[graphicIndex]);
    }

    /* Calculate buffer for the "fake header" approach:
     * We need to build a temporary header to select the graphic,
     * then read the compressed data from the file and expand it.
     *
     * Approach: use F0490_MEMORY_SelectGraphicFromHeader_Compat to
     * get the file offset and byte count, then read into a buffer,
     * then expand via the IMG3 pipeline.
     */
    memset(&tempHeader, 0, sizeof(tempHeader));
    tempHeader.format = rt->format;
    tempHeader.graphicCount = rt->graphicCount;
    tempHeader.compressedByteCounts = rt->compressedByteCounts;
    tempHeader.decompressedByteCounts = rt->decompressedByteCounts;
    tempHeader.widthHeight = rt->widthHeight;
    tempHeader.fileSize = rt->fileSize;

    memset(&selection, 0, sizeof(selection));
    if (!F0490_MEMORY_SelectGraphicFromHeader_Compat(
            &tempHeader, graphicIndex, &selection)) {
        return NULL;
    }

    offset = selection.offset;

    /* Allocate buffer for compressed data.  Even format-1 GRAPHICS.DAT keeps
     * the legacy 4-byte width/height prefix in each graphic's compressed
     * stream; the global header duplicates those dimensions for indexing. */
    compressedBuf = (unsigned char*)calloc(
        selection.compressedByteCount + 16, 1);
    if (!compressedBuf) {
        return NULL;
    }

    /* Read compressed data from GRAPHICS.DAT */
    if (!F0474_MEMORY_LoadGraphic_CPSDF_Compat(
            offset,
            selection.compressedByteCount,
            fs,
            compressedBuf)) {
        free(compressedBuf);
        return NULL;
    }

    /* Packed bitmap: IMG3 writes one 4-bit pixel per nibble and pads each
     * scanline to an even pixel stride. Odd-width graphics (notably C010,
     * the 87x45 action/PASS strip) therefore use ceil(evenStride/2) bytes
     * per row, not a continuous width*height nibble stream. */
    packedStride = (unsigned long)(((unsigned int)w + 1u) & ~1u) / 2u;
    packedSize = packedStride * (unsigned long)h;
    /* 4 bytes header prefix + padded data */
    unpackedSize = (unsigned long)w * (unsigned long)h;
    expandedCapacity = packedSize;
    if (expandedCapacity < unpackedSize) {
        expandedCapacity = unpackedSize;
    }
    packedBitmap = (unsigned char*)calloc(expandedCapacity + 64, 1);
    if (!packedBitmap) {
        free(compressedBuf);
        return NULL;
    }

    memset(packedBitmap + 4 + expandedCapacity, 0xCD, 60);
    overrunFirst = 60;
    overrunLast = 0;

    /* The M10 expand writes width/height at negative offsets from the
     * bitmap pointer: bitmap[-4..-3] = width, bitmap[-2..-1] = height.
     * So we pass packedBitmap+4 as the destination. */
    {
        unsigned char* bitmapBase = packedBitmap + 4;
        struct GraphicWidthHeight_Compat sizeInfo;
        sizeInfo.Width = w;
        sizeInfo.Height = h;
        F0488_MEMORY_ExpandGraphicToBitmap_Compat(compressedBuf, bitmapBase, &sizeInfo);
        {
            unsigned long gi;
            for (gi = 0; gi < 60; ++gi) {
                if (packedBitmap[4 + expandedCapacity + gi] != 0xCD) {
                    if (overrunFirst == 60) overrunFirst = gi;
                    overrunLast = gi;
                }
            }
            if (traceAssets && overrunFirst != 60) {
                fprintf(stderr, "asset-overrun %u first=+%lu last=+%lu\n",
                        graphicIndex, overrunFirst, overrunLast);
            }
        }
    }
    free(compressedBuf);

    /* Unpack from nibble-packed to 1-byte-per-pixel */
    unpackedPixels = (unsigned char*)calloc(unpackedSize, 1);
    if (!unpackedPixels) {
        free(packedBitmap);
        return NULL;
    }

    {
        unsigned char* bitmapBase = packedBitmap + 4;
        /* The packed format: each byte holds two pixels.
         * High nibble = left pixel (even X), low nibble = right pixel (odd X).
         * Each row is padded to an even pixel count; without respecting that
         * row stride, every row after the first in an odd-width graphic is
         * shifted by half a byte, visibly distorting text like PASS. */
        for (iy = 0; iy < (unsigned long)h; ++iy) {
            for (ix = 0; ix < (unsigned long)w; ++ix) {
                unsigned long byteIndex = iy * packedStride + (ix / 2u);
                unsigned char packed = bitmapBase[byteIndex];
                unsigned char pixel;
                if ((ix & 1u) == 0) {
                    pixel = (packed >> 4) & 0x0F;
                } else {
                    pixel = packed & 0x0F;
                }
                unpackedPixels[iy * (unsigned long)w + ix] = pixel;
            }
        }
    }
    free(packedBitmap);

    /* Store in cache */
    slot = m11_alloc_slot(loader);
    if (!slot) {
        free(unpackedPixels);
        return NULL;
    }
    slot->loaded = 1;
    slot->graphicIndex = graphicIndex;
    slot->width = w;
    slot->height = h;
    slot->pixels = unpackedPixels;
    if (traceAssets) {
        fprintf(stderr, "asset-done %u\n", graphicIndex);
    }
    return slot;
}

/* --- blitting --- */

void M11_AssetLoader_Blit(const M11_AssetSlot* slot,
                          unsigned char* framebuffer,
                          int fbWidth,
                          int fbHeight,
                          int dstX,
                          int dstY,
                          int transparentColor) {
    int srcX, srcY;
    int copyW, copyH;
    int startSrcX = 0, startSrcY = 0;

    if (!slot || !slot->loaded || !slot->pixels || !framebuffer) {
        return;
    }

    copyW = (int)slot->width;
    copyH = (int)slot->height;

    /* Clip left */
    if (dstX < 0) {
        startSrcX = -dstX;
        copyW -= startSrcX;
        dstX = 0;
    }
    /* Clip top */
    if (dstY < 0) {
        startSrcY = -dstY;
        copyH -= startSrcY;
        dstY = 0;
    }
    /* Clip right */
    if (dstX + copyW > fbWidth) {
        copyW = fbWidth - dstX;
    }
    /* Clip bottom */
    if (dstY + copyH > fbHeight) {
        copyH = fbHeight - dstY;
    }
    if (copyW <= 0 || copyH <= 0) {
        return;
    }

    for (srcY = 0; srcY < copyH; ++srcY) {
        const unsigned char* srcRow =
            slot->pixels + (startSrcY + srcY) * (int)slot->width + startSrcX;
        unsigned char* dstRow = framebuffer + (dstY + srcY) * fbWidth + dstX;
        if (transparentColor >= 0) {
            for (srcX = 0; srcX < copyW; ++srcX) {
                if (srcRow[srcX] != (unsigned char)transparentColor) {
                    dstRow[srcX] = srcRow[srcX];
                }
            }
        } else {
            memcpy(dstRow, srcRow, (size_t)copyW);
        }
    }
}

void M11_AssetLoader_BlitRegion(const M11_AssetSlot* slot,
                                int srcRX,
                                int srcRY,
                                int srcRW,
                                int srcRH,
                                unsigned char* framebuffer,
                                int fbWidth,
                                int fbHeight,
                                int dstX,
                                int dstY,
                                int transparentColor) {
    int y;
    int copyW, copyH;
    int startSrcX, startSrcY;

    if (!slot || !slot->loaded || !slot->pixels || !framebuffer) {
        return;
    }

    /* Clip source region to asset bounds */
    if (srcRX < 0) { srcRW += srcRX; srcRX = 0; }
    if (srcRY < 0) { srcRH += srcRY; srcRY = 0; }
    if (srcRX + srcRW > (int)slot->width) { srcRW = (int)slot->width - srcRX; }
    if (srcRY + srcRH > (int)slot->height) { srcRH = (int)slot->height - srcRY; }
    if (srcRW <= 0 || srcRH <= 0) {
        return;
    }

    startSrcX = srcRX;
    startSrcY = srcRY;
    copyW = srcRW;
    copyH = srcRH;

    /* Clip to destination */
    if (dstX < 0) { startSrcX -= dstX; copyW += dstX; dstX = 0; }
    if (dstY < 0) { startSrcY -= dstY; copyH += dstY; dstY = 0; }
    if (dstX + copyW > fbWidth) { copyW = fbWidth - dstX; }
    if (dstY + copyH > fbHeight) { copyH = fbHeight - dstY; }
    if (copyW <= 0 || copyH <= 0) {
        return;
    }

    for (y = 0; y < copyH; ++y) {
        const unsigned char* srcRow =
            slot->pixels + (startSrcY + y) * (int)slot->width + startSrcX;
        unsigned char* dstRow = framebuffer + (dstY + y) * fbWidth + dstX;
        if (transparentColor >= 0) {
            int x;
            for (x = 0; x < copyW; ++x) {
                if (srcRow[x] != (unsigned char)transparentColor) {
                    dstRow[x] = srcRow[x];
                }
            }
        } else {
            memcpy(dstRow, srcRow, (size_t)copyW);
        }
    }
}

void M11_AssetLoader_BlitScaled(const M11_AssetSlot* slot,
                                unsigned char* framebuffer,
                                int fbWidth,
                                int fbHeight,
                                int dstX,
                                int dstY,
                                int dstW,
                                int dstH,
                                int transparentColor) {
    int dy, dx;
    if (!slot || !slot->loaded || !slot->pixels || !framebuffer ||
        dstW <= 0 || dstH <= 0) {
        return;
    }
    for (dy = 0; dy < dstH; ++dy) {
        int sy = dy * (int)slot->height / dstH;
        int fbY = dstY + dy;
        if (fbY < 0 || fbY >= fbHeight) continue;
        for (dx = 0; dx < dstW; ++dx) {
            int sx = dx * (int)slot->width / dstW;
            int fbX = dstX + dx;
            unsigned char pixel;
            if (fbX < 0 || fbX >= fbWidth) continue;
            pixel = slot->pixels[sy * (int)slot->width + sx];
            if (transparentColor >= 0 && pixel == (unsigned char)transparentColor) {
                continue;
            }
            framebuffer[fbY * fbWidth + fbX] = pixel;
        }
    }
}

void M11_AssetLoader_BlitScaledMirror(const M11_AssetSlot* slot,
                                      unsigned char* framebuffer,
                                      int fbWidth,
                                      int fbHeight,
                                      int dstX,
                                      int dstY,
                                      int dstW,
                                      int dstH,
                                      int transparentColor) {
    int dy, dx;
    if (!slot || !slot->loaded || !slot->pixels || !framebuffer ||
        dstW <= 0 || dstH <= 0) {
        return;
    }
    for (dy = 0; dy < dstH; ++dy) {
        int sy = dy * (int)slot->height / dstH;
        int fbY = dstY + dy;
        if (fbY < 0 || fbY >= fbHeight) continue;
        for (dx = 0; dx < dstW; ++dx) {
            /* Mirror: sample from (width - 1 - sx) instead of sx */
            int sx = dx * (int)slot->width / dstW;
            int mirrorSx = (int)slot->width - 1 - sx;
            int fbX = dstX + dx;
            unsigned char pixel;
            if (fbX < 0 || fbX >= fbWidth) continue;
            if (mirrorSx < 0) mirrorSx = 0;
            pixel = slot->pixels[sy * (int)slot->width + mirrorSx];
            if (transparentColor >= 0 && pixel == (unsigned char)transparentColor) {
                continue;
            }
            framebuffer[fbY * fbWidth + fbX] = pixel;
        }
    }
}

/* ── Replacement-color blit variants ── */

static unsigned char m11_apply_replacement(unsigned char pixel,
                                           int replSrc9, int replDst9,
                                           int replSrc10, int replDst10) {
    if (replSrc9 >= 0 && pixel == (unsigned char)replSrc9)
        return (unsigned char)replDst9;
    if (replSrc10 >= 0 && pixel == (unsigned char)replSrc10)
        return (unsigned char)replDst10;
    return pixel;
}

void M11_AssetLoader_BlitScaledReplace(const M11_AssetSlot* slot,
                                      unsigned char* framebuffer,
                                      int fbWidth,
                                      int fbHeight,
                                      int dstX,
                                      int dstY,
                                      int dstW,
                                      int dstH,
                                      int transparentColor,
                                      int replSrc9,
                                      int replDst9,
                                      int replSrc10,
                                      int replDst10) {
    int dy, dx;
    if (!slot || !slot->loaded || !slot->pixels || !framebuffer ||
        dstW <= 0 || dstH <= 0) {
        return;
    }
    for (dy = 0; dy < dstH; ++dy) {
        int sy = dy * (int)slot->height / dstH;
        int fbY = dstY + dy;
        if (fbY < 0 || fbY >= fbHeight) continue;
        for (dx = 0; dx < dstW; ++dx) {
            int sx = dx * (int)slot->width / dstW;
            int fbX = dstX + dx;
            unsigned char pixel;
            if (fbX < 0 || fbX >= fbWidth) continue;
            pixel = slot->pixels[sy * (int)slot->width + sx];
            if (transparentColor >= 0 && pixel == (unsigned char)transparentColor)
                continue;
            pixel = m11_apply_replacement(pixel, replSrc9, replDst9,
                                          replSrc10, replDst10);
            framebuffer[fbY * fbWidth + fbX] = pixel;
        }
    }
}

void M11_AssetLoader_BlitScaledMirrorReplace(const M11_AssetSlot* slot,
                                             unsigned char* framebuffer,
                                             int fbWidth,
                                             int fbHeight,
                                             int dstX,
                                             int dstY,
                                             int dstW,
                                             int dstH,
                                             int transparentColor,
                                             int replSrc9,
                                             int replDst9,
                                             int replSrc10,
                                             int replDst10) {
    int dy, dx;
    if (!slot || !slot->loaded || !slot->pixels || !framebuffer ||
        dstW <= 0 || dstH <= 0) {
        return;
    }
    for (dy = 0; dy < dstH; ++dy) {
        int sy = dy * (int)slot->height / dstH;
        int fbY = dstY + dy;
        if (fbY < 0 || fbY >= fbHeight) continue;
        for (dx = 0; dx < dstW; ++dx) {
            int sx = dx * (int)slot->width / dstW;
            int mirrorSx = (int)slot->width - 1 - sx;
            int fbX = dstX + dx;
            unsigned char pixel;
            if (fbX < 0 || fbX >= fbWidth) continue;
            if (mirrorSx < 0) mirrorSx = 0;
            pixel = slot->pixels[sy * (int)slot->width + mirrorSx];
            if (transparentColor >= 0 && pixel == (unsigned char)transparentColor)
                continue;
            pixel = m11_apply_replacement(pixel, replSrc9, replDst9,
                                          replSrc10, replDst10);
            framebuffer[fbY * fbWidth + fbX] = pixel;
        }
    }
}
