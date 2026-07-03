/*
 * csb_v1_csbgraphics_m11_binding_readiness.h
 *
 * Data-free readiness contract for the future CSBgraphics.dat
 * decoded-payload -> M11 render handoff. This does not decode
 * CSBgraphics.dat LZW payloads and does not install a runtime
 * override. It only defines the narrow, bounds-checked shape a
 * trusted decoded override must satisfy before the M11 viewport or
 * HUD lanes may consume it. The companion apply helper copies an
 * already trusted indexed bitmap into the M11 320x200 framebuffer
 * according to that binding decision; it still does not discover or
 * interpret CSBgraphics.dat by itself.
 */

#ifndef FIRESTAFF_CSB_V1_CSBGRAPHICS_M11_BINDING_READINESS_H
#define FIRESTAFF_CSB_V1_CSBGRAPHICS_M11_BINDING_READINESS_H

#include "csb_v1_csbgraphics_dat_classify.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_V1_CSBGRAPHICS_M11_SOURCE_W 320
#define CSB_V1_CSBGRAPHICS_M11_SOURCE_H 200
#define CSB_V1_CSBGRAPHICS_M11_VIEWPORT_X 0
#define CSB_V1_CSBGRAPHICS_M11_VIEWPORT_Y 33
#define CSB_V1_CSBGRAPHICS_M11_VIEWPORT_W 224
#define CSB_V1_CSBGRAPHICS_M11_VIEWPORT_H 136
#define CSB_V1_CSBGRAPHICS_M11_C040_PANEL_X 80
#define CSB_V1_CSBGRAPHICS_M11_C040_PANEL_Y 52
#define CSB_V1_CSBGRAPHICS_M11_C040_PANEL_W 144
#define CSB_V1_CSBGRAPHICS_M11_C040_PANEL_H 73
#define CSB_V1_CSBGRAPHICS_M11_PALETTE_MAX 15u

typedef enum {
    CSB_V1_CSBGRAPHICS_M11_ROUTE_NONE = 0,
    CSB_V1_CSBGRAPHICS_M11_ROUTE_VIEWPORT_DERIVED = 1,
    CSB_V1_CSBGRAPHICS_M11_ROUTE_HUD_INVENTORY = 2,
    CSB_V1_CSBGRAPHICS_M11_ROUTE_HUD_RESURRECT_PANEL = 3
} CSB_V1_CSBGraphicsM11Route;

typedef enum {
    CSB_V1_CSBGRAPHICS_M11_DECISION_FALLBACK_ORIGINAL = 0,
    CSB_V1_CSBGRAPHICS_M11_DECISION_BIND_OVERRIDE = 1
} CSB_V1_CSBGraphicsM11Decision;

typedef struct {
    uint32_t entry_index;
    uint16_t width;
    uint16_t height;
    uint8_t bits_per_pixel;
    uint8_t max_palette_index;
    int decoded_ok;
    int trusted;
    const uint8_t *indexed_pixels;
    size_t indexed_pixel_count;
} CSB_V1_CSBGraphicsDecodedBitmap;

typedef struct {
    CSB_V1_CSBGraphicsM11Decision decision;
    CSB_V1_CSBGraphicsM11Route route;
    uint32_t entry_index;
    int destination_x;
    int destination_y;
    int destination_w;
    int destination_h;
    int decoded_w;
    int decoded_h;
    int needs_viewport_redraw;
    int needs_hud_redraw;
    int preserves_v1_palette_indices;
    const char *reason;
    const char *source_evidence;
} CSB_V1_CSBGraphicsM11Binding;

int csb_v1_csbgraphics_m11_prepare_binding(
    const CSB_V1_CSBGraphicsEntrySpan *span,
    const CSB_V1_CSBGraphicsDecodedBitmap *decoded,
    CSB_V1_CSBGraphicsM11Binding *out_binding);

int csb_v1_csbgraphics_m11_apply_binding(
    const CSB_V1_CSBGraphicsM11Binding *binding,
    const CSB_V1_CSBGraphicsDecodedBitmap *decoded,
    uint8_t *framebuffer,
    int framebuffer_width,
    int framebuffer_height,
    int framebuffer_stride);

int csb_v1_csbgraphics_m11_prepare_and_apply(
    const CSB_V1_CSBGraphicsEntrySpan *span,
    const CSB_V1_CSBGraphicsDecodedBitmap *decoded,
    uint8_t *framebuffer,
    int framebuffer_width,
    int framebuffer_height,
    int framebuffer_stride,
    CSB_V1_CSBGraphicsM11Binding *out_binding);

const char *csb_v1_csbgraphics_m11_route_name(
    CSB_V1_CSBGraphicsM11Route route);

const char *csb_v1_csbgraphics_m11_decision_name(
    CSB_V1_CSBGraphicsM11Decision decision);

const char *csb_v1_csbgraphics_m11_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_CSBGRAPHICS_M11_BINDING_READINESS_H */
