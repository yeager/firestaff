#ifndef FIRESTAFF_ASSET_LOADER_M11_H
#define FIRESTAFF_ASSET_LOADER_M11_H

/*
 * asset_loader_m11 — M11 GRAPHICS.DAT asset loader and blitter.
 *
 * Opens GRAPHICS.DAT, loads compressed graphics by index, expands them
 * via the M10 IMG3 pipeline into unpacked 1-byte-per-pixel buffers
 * (each byte holds a 4-bit VGA palette index, values 0-15), and
 * provides blit operations onto the M11 framebuffer.
 *
 * The expanded pixel data is cached in a simple slot array so repeated
 * draws of the same graphic do not re-read the file.
 *
 * When GRAPHICS.DAT is unavailable the loader reports "not loaded" and
 * callers fall back to primitive rendering.
 *
 * IMPORTANT: the M10 IMG3 decompressor uses global state
 * (G2157_, G2159_puc_Bitmap_Source, G2160_puc_Bitmap_Destination).
 * The loader sets these up before each expand call and restores them
 * after, so concurrent use from multiple threads is NOT safe.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum number of cached expanded graphics.
   Slots are allocated on first load. */
#define M11_ASSET_CACHE_SLOTS 64

typedef struct {
    int loaded;              /* 1 if pixels[] is valid */
    unsigned int graphicIndex;
    unsigned short width;    /* pixel width */
    unsigned short height;   /* pixel height */
    unsigned char* pixels;   /* width * height bytes, 1 byte per pixel, 4-bit value */
} M11_AssetSlot;

typedef struct {
    int initialized;
    char graphicsDatPath[512];

    /* M10 GRAPHICS.DAT runtime and file state */
    void* fileState;     /* MemoryGraphicsDatState_Compat* */
    void* runtimeState;  /* MemoryGraphicsDatRuntimeState_Compat* */

    unsigned short graphicCount; /* total graphics in file */

    /* Cache */
    M11_AssetSlot cache[M11_ASSET_CACHE_SLOTS];
    int cacheUsed;
} M11_AssetLoader;

/* Initialize the loader from a GRAPHICS.DAT file path.
   Returns 1 on success, 0 on failure. */
int M11_AssetLoader_Init(M11_AssetLoader* loader, const char* graphicsDatPath);

/* Shut down and free all cached data. */
void M11_AssetLoader_Shutdown(M11_AssetLoader* loader);

/* Check whether the loader is ready. */
int M11_AssetLoader_IsReady(const M11_AssetLoader* loader);

/* Query the width and height of a graphic index without loading it.
   Returns 1 on success. */
int M11_AssetLoader_QuerySize(const M11_AssetLoader* loader,
                              unsigned int graphicIndex,
                              unsigned short* outWidth,
                              unsigned short* outHeight);

/* Load (and cache) a graphic by index. Returns a pointer to the
   cache slot, or NULL on failure. The slot remains valid until
   Shutdown is called. */
const M11_AssetSlot* M11_AssetLoader_Load(M11_AssetLoader* loader,
                                          unsigned int graphicIndex);

/* Blit a loaded asset slot onto the M11 1-byte-per-pixel framebuffer.
   transparentColor: if >= 0, pixels matching this value are skipped.
   Clips to framebuffer bounds. */
void M11_AssetLoader_Blit(const M11_AssetSlot* slot,
                          unsigned char* framebuffer,
                          int fbWidth,
                          int fbHeight,
                          int dstX,
                          int dstY,
                          int transparentColor);

/* Blit a sub-rectangle of a loaded asset slot. */
void M11_AssetLoader_BlitRegion(const M11_AssetSlot* slot,
                                int srcX,
                                int srcY,
                                int srcW,
                                int srcH,
                                unsigned char* framebuffer,
                                int fbWidth,
                                int fbHeight,
                                int dstX,
                                int dstY,
                                int transparentColor);

/* Blit with scaling (nearest-neighbor). */
void M11_AssetLoader_BlitScaled(const M11_AssetSlot* slot,
                                unsigned char* framebuffer,
                                int fbWidth,
                                int fbHeight,
                                int dstX,
                                int dstY,
                                int dstW,
                                int dstH,
                                int transparentColor);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_ASSET_LOADER_M11_H */
