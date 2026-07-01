/*
 * dm2_v2_hud_widget_bitmap_blit.h — DM2 V2 HUD Widget bounded bitmap blit
 *
 * Phase 3 follow-up: bounded real-bitmap blit path for the DM2 V2 HUD
 * widget runtime hook. Sits between dm2_v2_hud_widget_assets (the
 * manifest gate that classifies each slot) and
 * dm2_v2_hud_runtime_render_with_assets (the runtime entry that
 * currently emits a single-pixel anchor stamp for REAL slots).
 *
 * The blit path is intentionally bounded to the synthetic-test
 * fixture format already shipped under examples/dm2_hud_widget_synthetic/:
 *   - 1x1 pixel
 *   - 8-bit depth
 *   - Color type 6 (RGBA, non-interlaced)
 *   - Single IDAT chunk (typical for tiny fixtures)
 *
 * Anything outside that envelope (different dimensions, indexed color,
 * different bit depth, interlaced, missing IDAT, missing signature,
 * file missing, path outside the asset manifest's resolved_path,
 * decompression failure, ...) returns 0 and the caller is expected to
 * fall back to the legacy 1-pixel anchor stamp — never to crash, never
 * to write out of bounds, never to claim a finished bitmap decode for
 * anything beyond a synthetic-test fixture.
 *
 * Honest boundary (mirrors examples/dm2_hud_widget_synthetic/README.md):
 *   - This module enables the manifest gate's REAL classification to
 *     actually substitute the procedural fallback for a small bounded
 *     blit, using the synthetic 1x1 RGBA PNG fixtures already on disk.
 *   - It does NOT decode multi-pixel PNGs, does NOT blit to arbitrary
 *     destinations, does NOT claim operator-installable finished art.
 *   - Real-art promotion requires a multi-pixel decode path (OPEN-
 *     BOUNDED next step) and a sibling gap-list update. This module
 *     is the seam that future real-art code lands behind — it proves
 *     the runtime hook is wired end-to-end with a synthetic-only blit.
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

/* Bounded single-pixel blit into a framebuffer.
 *
 * Writes the (r,g,b,a) pixel at (dst_x, dst_y) into fb, with the
 * following bounds guarantees:
 *   - If dst_x is outside [0, w), returns 0 without writing.
 *   - If dst_y is outside [0, h_res), returns 0 without writing.
 *   - If a == 255, overwrites fb[y*w+x] = r (red channel as the
 *     palette-index byte, matching the existing
 *     dm2_v2_hud_overlay drawing convention).
 *   - If a < 255, alpha-blends over fb[y*w+x] using the standard
 *     "src over dst" integer blend: out = (a*src + (255-a)*dst) / 255.
 *   - Any other fb parameter (NULL, w<=0, h_res<=0) returns 0.
 *
 * The red-channel-as-index convention keeps the synthetic blit
 * testable: each fixture has a distinct R value (see
 * examples/dm2_hud_widget_synthetic/) so a probe can read back the
 * blitted byte and assert it matches the expected R for that slot.
 *
 * Returns 1 on a successful in-bounds write, 0 on no-op (out of
 * bounds or invalid arguments). */
int dm2_v2_hud_widget_bitmap_blit_pixel_rgba(
    uint8_t* fb, int w, int h_res,
    int dst_x, int dst_y,
    uint8_t r, uint8_t g, uint8_t b, uint8_t a);

/* High-level: read a slot's PNG and bounded-blit its pixel.
 *
 * Reads info->resolved_path through the synthetic envelope. Returns 1
 * on a successful in-bounds blit, 0 on any failure (file missing,
 * unsupported format, decompression error, destination outside
 * framebuffer). The runtime uses 0 as the signal to fall back to the
 * legacy 1-pixel anchor stamp, preserving the no-gate baseline byte
 * pattern for any slot whose blit cannot run.
 *
 * Defensive: refuses to run when info->resolved_path is empty (a
 * REAL-classified slot whose source_file did not resolve) — that is
 * a PARTIAL slot, not a REAL one, and the gate should never have
 * routed it here. */
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
