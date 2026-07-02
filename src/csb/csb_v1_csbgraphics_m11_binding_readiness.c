#include "csb_v1_csbgraphics_m11_binding_readiness.h"

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
