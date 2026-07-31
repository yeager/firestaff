/*
 * dm2_v2_hud_widget_bitmap_blit.h — DM2 V2 HUD Widget bounded bitmap blit
 *
 * Fixture-only PNG reader retained for HUD-manifest diagnostics. It must
 * never own runtime pixels: original DM2 HUD rendering is GDAT-only.
 *
 * The blit path is intentionally bounded to the synthetic-test
 * fixture format already shipped under examples/dm2_hud_widget_synthetic/:
 *   - 1x1 pixel
 *   - 8-bit depth
 *   - Color type 6 (RGBA, non-interlaced)
 *   - Single IDAT chunk (typical for tiny fixtures)
 *
 * Anything outside that envelope returns 0. The renderer-facing APIs below
 * deliberately return 0 for every input, including a valid fixture, so a
 * manifest cannot promote synthetic or operator-provided art into DM2.
 *
 * Source-owner boundary:
 *   - Manifest classification is diagnostics only; it cannot grant a PNG
 *     ownership of a DM2 HUD surface.
 *   - The ordinary DM2 HUD path uses authenticated
 *     INTERFACE_GENERAL/CHAMPIONS GDAT records and their paired palettes.
 *   - Any future material path must carry that original-data provenance;
 *     it must not repurpose this fixture decoder as a renderer.
 *
 * Source:
 *   - SKULL.ASM T560 (DM2 HUD rendering pipeline)
 *   - skproject/SKULLWIN/c_gui_vp.cpp (DM2 UI chrome layout)
 *   - ReDMCSB PANEL.C F0354 (champion status-box drawing)
 *   - examples/dm2_hud_widget_synthetic/ (synthetic 1x1 RGBA fixtures)
 *   - PNG specification (W3C / ISO 15948): IHDR + IDAT + IEND chunks,
 *     filter byte 0 (None) for 1x1 RGBA, zlib-wrapped deflate stream.
 *   - include/dm2_v2_hud_widget_assets.h (slot gate this module reads)
 *   - include/dm2_v2_hud_runtime.h (runtime hook this module extends)
 */

#ifndef FIRESTAFF_DM2_V2_HUD_WIDGET_BITMAP_BLIT_H
#define FIRESTAFF_DM2_V2_HUD_WIDGET_BITMAP_BLIT_H

#include <stddef.h>
#include <stdint.h>

#include "dm2_v2_hud_widget_assets.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Decoded-pixel view ───────────────────────────────────────────
 *
 * Returned by dm2_v2_hud_widget_bitmap_blit_read_pixel() for the
 * synthetic 1x1 RGBA PNGs that the existing
 * examples/dm2_hud_widget_synthetic/ fixtures ship. Fields are
 * populated only when the function returns 1; otherwise the struct
 * is zeroed.
 *
 * For larger PNGs (width > DM2_V2_HUD_WIDGET_BLIT_MAX_WIDTH or
 * height > DM2_V2_HUD_WIDGET_BLIT_MAX_HEIGHT) or any non-RGBA
 * format, the function returns 0 unconditionally. That contract is
 * the bounded-blit guarantee: the runtime never silently falls into
 * a multi-pixel decode path or claims finished PBR art decoding. */
typedef struct {
    int     width;        /* always 1 in the supported envelope */
    int     height;       /* always 1 in the supported envelope */
    int     bit_depth;    /* always 8 in the supported envelope */
    int     color_type;   /* always 6 (RGBA) in the supported envelope */
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} DM2_V2_HudWidgetBlitPixel;

/* Synthetic blit envelope. Multi-pixel PNGs are intentionally
 * rejected so the bounded-blit path cannot accidentally decode an
 * operator-installed real-art asset and claim real art is on
 * screen. Bumping these is an explicit API change. */
#define DM2_V2_HUD_WIDGET_BLIT_MAX_WIDTH  1
#define DM2_V2_HUD_WIDGET_BLIT_MAX_HEIGHT 1

/* Hard cap on the size of the PNG file we'll read. The synthetic
 * fixtures are ~167 bytes; this is an order-of-magnitude safety net
 * against accidental expansion to multi-kilobyte real-art assets. */
#define DM2_V2_HUD_WIDGET_BLIT_MAX_FILE_BYTES (64U * 1024U)

/* Read the single pixel from a 1x1 8-bit RGBA PNG.
 *
 * Returns 1 on success and fills *out_pixel. Returns 0 if:
 *   - path is NULL or empty
 *   - file is missing, unreadable, or larger than
 *     DM2_V2_HUD_WIDGET_BLIT_MAX_FILE_BYTES
 *   - first 8 bytes are not the PNG signature
 *   - IHDR chunk is missing, malformed, or describes a width/height
 *     outside [1..DM2_V2_HUD_WIDGET_BLIT_MAX_WIDTH/HEIGHT]
 *   - color type is not 6 (RGBA) or bit depth is not 8
 *   - IDAT chunks are missing, truncated, or fail zlib inflate
 *   - decompressed payload is not exactly 5 bytes (filter byte + RGBA)
 *
 * Out-parameter is always written — zeroed on failure so callers can
 * log/diff pixel values without uninitialised-read UB. */
int dm2_v2_hud_widget_bitmap_blit_read_pixel(
    const char* path,
    DM2_V2_HudWidgetBlitPixel* out_pixel);

/* Retired single-pixel blit compatibility entry point.
 *
 * Always returns 0 without changing fb. Kept only for API compatibility;
 * use the mounted GDAT renderer for any real HUD surface. */
int dm2_v2_hud_widget_bitmap_blit_pixel_rgba(
    uint8_t* fb, int w, int h_res,
    int dst_x, int dst_y,
    uint8_t r, uint8_t g, uint8_t b, uint8_t a);

/* Retired high-level PNG compatibility entry point.
 *
 * Always returns 0 without reading or writing a runtime framebuffer. */
int dm2_v2_hud_widget_bitmap_blit_render_slot(
    const DM2_V2_HudWidgetSlotInfo* info,
    uint8_t* fb, int w, int h_res,
    int dst_x, int dst_y);

/* Source-evidence citation for source-lock tests. */
const char* dm2_v2_hud_widget_bitmap_blit_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V2_HUD_WIDGET_BITMAP_BLIT_H */
