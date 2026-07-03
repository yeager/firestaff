#include "csb_v1_csbgraphics_m11_binding_readiness.h"

#include <stdlib.h>
#include <string.h>

enum {
    CSB_GRAPHIC_INVENTORY = 17,
    CSB_GRAPHIC_PANEL_RESURRECT_REINCARNATE = 40,
    CSB_GRAPHIC_FIELD_MIN = 73,
    CSB_GRAPHIC_FIELD_MAX = 74,
    CSB_GRAPHIC_EXPLOSION_MIN = 351,
    CSB_GRAPHIC_EXPLOSION_MAX = 359
};

static const char s_source_evidence[] =
    "CSBgraphics.dat handoff: CSBWin/Graphics.cpp:1918 ReadGraphicsIndex; "
    "CSBWin/Graphics.cpp:1643 LocateNthGraphic; CSBWin/Graphics.cpp:1717 "
    "ReadGraphic. M11 lanes: ReDMCSB DEFS.H:2178 C017_GRAPHIC_INVENTORY; "
    "DEFS.H:2200 C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE; "
    "PANEL.C:1632 F0346 blits C040 to G0032_ai_Graphic562_Box_Panel "
    "(80,223,52,124) with C072 byte width; PANEL.C:2376 expands C017 "
    "inventory into G0296_puc_Bitmap_Viewport; DEFS.H:2407 "
    "C000_DERIVED_BITMAP_VIEWPORT covers fields graphics #73..#74 and "
    "explosions #351..#359 over the 224x136 viewport; touch/layout source "
    "contract keeps the M11 source framebuffer at 320x200 and dungeon "
    "viewport at x=0 y=33 w=224 h=136.";

static void fallback(
    CSB_V1_CSBGraphicsM11Binding *out_binding,
    uint32_t entry_index,
    const char *reason)
{
    memset(out_binding, 0, sizeof(*out_binding));
    out_binding->decision = CSB_V1_CSBGRAPHICS_M11_DECISION_FALLBACK_ORIGINAL;
    out_binding->route = CSB_V1_CSBGRAPHICS_M11_ROUTE_NONE;
    out_binding->entry_index = entry_index;
    out_binding->preserves_v1_palette_indices = 1;
    out_binding->reason = reason;
    out_binding->source_evidence = s_source_evidence;
}

static int is_viewport_entry(uint32_t entry_index)
{
    return (entry_index >= CSB_GRAPHIC_FIELD_MIN &&
            entry_index <= CSB_GRAPHIC_FIELD_MAX) ||
           (entry_index >= CSB_GRAPHIC_EXPLOSION_MIN &&
            entry_index <= CSB_GRAPHIC_EXPLOSION_MAX);
}

static int decoded_pixel_count_fits(
    const CSB_V1_CSBGraphicsDecodedBitmap *decoded)
{
    size_t required;

    if (!decoded || decoded->width == 0u || decoded->height == 0u) {
        return 0;
    }
    required = (size_t)decoded->width * (size_t)decoded->height;
    return decoded->indexed_pixels != NULL &&
           decoded->indexed_pixel_count >= required;
}

int csb_v1_csbgraphics_m11_prepare_binding(
    const CSB_V1_CSBGraphicsEntrySpan *span,
    const CSB_V1_CSBGraphicsDecodedBitmap *decoded,
    CSB_V1_CSBGraphicsM11Binding *out_binding)
{
    if (!out_binding) {
        return 0;
    }

    if (!span) {
        fallback(out_binding, 0u, "missing-span");
        return 1;
    }

    if (span->compressed_size == 0u || span->decompressed_size == 0u) {
        fallback(out_binding, span->entry_index, "empty-span");
        return 1;
    }

    if (!decoded || !decoded->decoded_ok || !decoded->trusted) {
        fallback(out_binding, span->entry_index, "decoded-payload-not-trusted");
        return 1;
    }

    if (decoded->entry_index != span->entry_index) {
        fallback(out_binding, span->entry_index, "entry-mismatch");
        return 1;
    }

    if (decoded->bits_per_pixel != 4u ||
        decoded->max_palette_index > CSB_V1_CSBGRAPHICS_M11_PALETTE_MAX) {
        fallback(out_binding, span->entry_index, "palette-budget");
        return 1;
    }

    if (!decoded_pixel_count_fits(decoded)) {
        fallback(out_binding, span->entry_index, "decoded-pixel-bounds");
        return 1;
    }

    memset(out_binding, 0, sizeof(*out_binding));
    out_binding->decision = CSB_V1_CSBGRAPHICS_M11_DECISION_BIND_OVERRIDE;
    out_binding->entry_index = span->entry_index;
    out_binding->decoded_w = decoded->width;
    out_binding->decoded_h = decoded->height;
    out_binding->preserves_v1_palette_indices = 1;
    out_binding->reason = "trusted-decoded-override";
    out_binding->source_evidence = s_source_evidence;

    if (is_viewport_entry(span->entry_index)) {
        if (decoded->width > CSB_V1_CSBGRAPHICS_M11_VIEWPORT_W ||
            decoded->height > CSB_V1_CSBGRAPHICS_M11_VIEWPORT_H) {
            fallback(out_binding, span->entry_index, "viewport-geometry");
            return 1;
        }
        out_binding->route = CSB_V1_CSBGRAPHICS_M11_ROUTE_VIEWPORT_DERIVED;
        out_binding->destination_x = CSB_V1_CSBGRAPHICS_M11_VIEWPORT_X;
        out_binding->destination_y = CSB_V1_CSBGRAPHICS_M11_VIEWPORT_Y;
        out_binding->destination_w = CSB_V1_CSBGRAPHICS_M11_VIEWPORT_W;
        out_binding->destination_h = CSB_V1_CSBGRAPHICS_M11_VIEWPORT_H;
        out_binding->needs_viewport_redraw = 1;
        return 1;
    }

    if (span->entry_index == CSB_GRAPHIC_INVENTORY) {
        if (decoded->width != CSB_V1_CSBGRAPHICS_M11_VIEWPORT_W ||
            decoded->height != CSB_V1_CSBGRAPHICS_M11_VIEWPORT_H) {
            fallback(out_binding, span->entry_index, "inventory-geometry");
            return 1;
        }
        out_binding->route = CSB_V1_CSBGRAPHICS_M11_ROUTE_HUD_INVENTORY;
        out_binding->destination_x = CSB_V1_CSBGRAPHICS_M11_VIEWPORT_X;
        out_binding->destination_y = CSB_V1_CSBGRAPHICS_M11_VIEWPORT_Y;
        out_binding->destination_w = CSB_V1_CSBGRAPHICS_M11_VIEWPORT_W;
        out_binding->destination_h = CSB_V1_CSBGRAPHICS_M11_VIEWPORT_H;
        out_binding->needs_hud_redraw = 1;
        return 1;
    }

    if (span->entry_index == CSB_GRAPHIC_PANEL_RESURRECT_REINCARNATE) {
        if (decoded->width != CSB_V1_CSBGRAPHICS_M11_C040_PANEL_W ||
            decoded->height != CSB_V1_CSBGRAPHICS_M11_C040_PANEL_H) {
            fallback(out_binding, span->entry_index, "c040-panel-geometry");
            return 1;
        }
        out_binding->route = CSB_V1_CSBGRAPHICS_M11_ROUTE_HUD_RESURRECT_PANEL;
        out_binding->destination_x = CSB_V1_CSBGRAPHICS_M11_C040_PANEL_X;
        out_binding->destination_y = CSB_V1_CSBGRAPHICS_M11_C040_PANEL_Y;
        out_binding->destination_w = CSB_V1_CSBGRAPHICS_M11_C040_PANEL_W;
        out_binding->destination_h = CSB_V1_CSBGRAPHICS_M11_C040_PANEL_H;
        out_binding->needs_hud_redraw = 1;
        return 1;
    }

    fallback(out_binding, span->entry_index, "unsupported-entry-route");
    return 1;
}

int csb_v1_csbgraphics_m11_apply_binding(
    const CSB_V1_CSBGraphicsM11Binding *binding,
    const CSB_V1_CSBGraphicsDecodedBitmap *decoded,
    uint8_t *framebuffer,
    int framebuffer_width,
    int framebuffer_height,
    int framebuffer_stride)
{
    int x;
    int y;
    int copy_w;
    int copy_h;

    if (!binding || !decoded || !framebuffer) {
        return 0;
    }
    if (binding->decision != CSB_V1_CSBGRAPHICS_M11_DECISION_BIND_OVERRIDE ||
        binding->route == CSB_V1_CSBGRAPHICS_M11_ROUTE_NONE) {
        return 0;
    }
    if (!decoded->decoded_ok || !decoded->trusted ||
        decoded->entry_index != binding->entry_index ||
        decoded->bits_per_pixel != 4u ||
        decoded->max_palette_index > CSB_V1_CSBGRAPHICS_M11_PALETTE_MAX ||
        !decoded_pixel_count_fits(decoded)) {
        return 0;
    }
    if (framebuffer_width <= 0 || framebuffer_height <= 0 ||
        framebuffer_stride < framebuffer_width) {
        return 0;
    }
    if (binding->destination_x < 0 || binding->destination_y < 0 ||
        binding->destination_w <= 0 || binding->destination_h <= 0) {
        return 0;
    }

    copy_w = (int)decoded->width;
    copy_h = (int)decoded->height;
    if (copy_w <= 0 || copy_h <= 0 ||
        copy_w > binding->destination_w ||
        copy_h > binding->destination_h) {
        return 0;
    }
    if (binding->destination_x + copy_w > framebuffer_width ||
        binding->destination_y + copy_h > framebuffer_height) {
        return 0;
    }

    /* Source-lock boundary: CSBWin Graphics.cpp:1717 ReadGraphic delivers
     * indexed bitmap payloads for graphics overrides. M11 keeps V1's
     * 320x200 indexed framebuffer, so the runtime handoff is a direct
     * palette-index copy into the prepared route rectangle. */
    for (y = 0; y < copy_h; ++y) {
        uint8_t *dst = framebuffer +
            (binding->destination_y + y) * framebuffer_stride +
            binding->destination_x;
        const uint8_t *src = decoded->indexed_pixels +
            (size_t)y * (size_t)decoded->width;
        for (x = 0; x < copy_w; ++x) {
            dst[x] = src[x];
        }
    }
    return 1;
}

int csb_v1_csbgraphics_m11_prepare_and_apply(
    const CSB_V1_CSBGraphicsEntrySpan *span,
    const CSB_V1_CSBGraphicsDecodedBitmap *decoded,
    uint8_t *framebuffer,
    int framebuffer_width,
    int framebuffer_height,
    int framebuffer_stride,
    CSB_V1_CSBGraphicsM11Binding *out_binding)
{
    CSB_V1_CSBGraphicsM11Binding local_binding;
    CSB_V1_CSBGraphicsM11Binding *binding =
        out_binding ? out_binding : &local_binding;

    if (!csb_v1_csbgraphics_m11_prepare_binding(span, decoded, binding)) {
        return 0;
    }
    return csb_v1_csbgraphics_m11_apply_binding(binding,
                                                decoded,
                                                framebuffer,
                                                framebuffer_width,
                                                framebuffer_height,
                                                framebuffer_stride);
}

int csb_v1_csbgraphics_m11_decode_entry_and_apply(
    const uint8_t *csbgraphics_bytes,
    size_t csbgraphics_size,
    uint32_t entry_index,
    uint16_t decoded_width,
    uint16_t decoded_height,
    uint8_t *framebuffer,
    int framebuffer_width,
    int framebuffer_height,
    int framebuffer_stride,
    CSB_V1_CSBGraphicsM11Binding *out_binding)
{
    CSB_V1_CSBGraphicsEntrySpan span;
    CSB_V1_CSBGraphicsDecodedBitmap decoded;
    uint8_t *decoded_bytes = NULL;
    size_t expected_pixels;
    size_t written = 0u;
    size_t i;
    uint8_t max_palette = 0u;
    int rc;
    int applied;

    if (!csbgraphics_bytes || decoded_width == 0u || decoded_height == 0u ||
        !framebuffer) {
        return 0;
    }

    expected_pixels = (size_t)decoded_width * (size_t)decoded_height;
    if (expected_pixels == 0u || expected_pixels > 65535u) {
        return 0;
    }

    rc = csb_v1_csbgraphics_dat_entry_span(csbgraphics_bytes,
                                           csbgraphics_size,
                                           entry_index,
                                           &span);
    if (rc != CSB_V1_CSBGRAPHICS_CLASSIFY_OK ||
        (size_t)span.decompressed_size != expected_pixels) {
        return 0;
    }

    decoded_bytes = (uint8_t *)malloc(expected_pixels);
    if (!decoded_bytes) {
        return 0;
    }

    rc = csb_v1_csbgraphics_dat_decode_entry(csbgraphics_bytes,
                                             csbgraphics_size,
                                             entry_index,
                                             decoded_bytes,
                                             expected_pixels,
                                             &written);
    if (rc != CSB_V1_CSBGRAPHICS_CLASSIFY_OK || written != expected_pixels) {
        free(decoded_bytes);
        return 0;
    }

    for (i = 0u; i < expected_pixels; ++i) {
        if (decoded_bytes[i] > max_palette) {
            max_palette = decoded_bytes[i];
        }
    }

    memset(&decoded, 0, sizeof(decoded));
    decoded.entry_index = entry_index;
    decoded.width = decoded_width;
    decoded.height = decoded_height;
    decoded.bits_per_pixel = 4u;
    decoded.max_palette_index = max_palette;
    decoded.decoded_ok = 1;
    decoded.trusted = 1;
    decoded.indexed_pixels = decoded_bytes;
    decoded.indexed_pixel_count = expected_pixels;

    applied = csb_v1_csbgraphics_m11_prepare_and_apply(&span,
                                                       &decoded,
                                                       framebuffer,
                                                       framebuffer_width,
                                                       framebuffer_height,
                                                       framebuffer_stride,
                                                       out_binding);
    free(decoded_bytes);
    return applied;
}

const char *csb_v1_csbgraphics_m11_route_name(
    CSB_V1_CSBGraphicsM11Route route)
{
    switch (route) {
    case CSB_V1_CSBGRAPHICS_M11_ROUTE_NONE:
        return "none";
    case CSB_V1_CSBGRAPHICS_M11_ROUTE_VIEWPORT_DERIVED:
        return "viewport-derived";
    case CSB_V1_CSBGRAPHICS_M11_ROUTE_HUD_INVENTORY:
        return "hud-inventory";
    case CSB_V1_CSBGRAPHICS_M11_ROUTE_HUD_RESURRECT_PANEL:
        return "hud-resurrect-panel";
    default:
        return "unknown";
    }
}

const char *csb_v1_csbgraphics_m11_decision_name(
    CSB_V1_CSBGraphicsM11Decision decision)
{
    switch (decision) {
    case CSB_V1_CSBGRAPHICS_M11_DECISION_FALLBACK_ORIGINAL:
        return "fallback-original";
    case CSB_V1_CSBGRAPHICS_M11_DECISION_BIND_OVERRIDE:
        return "bind-override";
    default:
        return "unknown";
    }
}

const char *csb_v1_csbgraphics_m11_source_evidence(void)
{
    return s_source_evidence;
}
