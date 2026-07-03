#include "csb_v1_csbgraphics_m11_binding_readiness.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static void check_int(const char *label, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want, anchor);
        return;
    }
    printf("ok %s=%d anchor=%s\n", label, got, anchor);
}

static void check_str(const char *label, const char *got, const char *want)
{
    ++g_assertions;
    if (!got || strcmp(got, want) != 0) {
        ++g_failures;
        printf("FAIL %s got=%s want=%s\n",
               label, got ? got : "(null)", want ? want : "(null)");
        return;
    }
    printf("ok %s=%s\n", label, got);
}

static void check_contains(const char *label, const char *haystack, const char *needle)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        ++g_failures;
        printf("FAIL %s missing=%s\n", label, needle ? needle : "(null)");
        return;
    }
    printf("ok %s contains=%s\n", label, needle);
}

static CSB_V1_CSBGraphicsEntrySpan span_for(uint32_t entry_index)
{
    CSB_V1_CSBGraphicsEntrySpan span;
    memset(&span, 0, sizeof(span));
    span.entry_index = entry_index;
    span.payload_offset = 128u + entry_index;
    span.compressed_size = 12u;
    span.decompressed_size = 64u;
    return span;
}

static CSB_V1_CSBGraphicsDecodedBitmap decoded_for(
    uint32_t entry_index,
    uint16_t width,
    uint16_t height,
    uint8_t max_palette,
    const uint8_t *pixels,
    size_t pixel_count)
{
    CSB_V1_CSBGraphicsDecodedBitmap decoded;
    memset(&decoded, 0, sizeof(decoded));
    decoded.entry_index = entry_index;
    decoded.width = width;
    decoded.height = height;
    decoded.bits_per_pixel = 4u;
    decoded.max_palette_index = max_palette;
    decoded.decoded_ok = 1;
    decoded.trusted = 1;
    decoded.indexed_pixels = pixels;
    decoded.indexed_pixel_count = pixel_count;
    return decoded;
}

static void test_contract_constants_and_evidence(void)
{
    const char *evidence = csb_v1_csbgraphics_m11_source_evidence();

    check_int("source.w", CSB_V1_CSBGRAPHICS_M11_SOURCE_W, 320,
              "M11 320x200 source framebuffer");
    check_int("source.h", CSB_V1_CSBGRAPHICS_M11_SOURCE_H, 200,
              "M11 320x200 source framebuffer");
    check_int("viewport.x", CSB_V1_CSBGRAPHICS_M11_VIEWPORT_X, 0,
              "source viewport x");
    check_int("viewport.y", CSB_V1_CSBGRAPHICS_M11_VIEWPORT_Y, 33,
              "source viewport y");
    check_int("viewport.w", CSB_V1_CSBGRAPHICS_M11_VIEWPORT_W, 224,
              "ReDMCSB DEFS.H C224 viewport width");
    check_int("viewport.h", CSB_V1_CSBGRAPHICS_M11_VIEWPORT_H, 136,
              "ReDMCSB DEFS.H C136 viewport height");
    check_int("c040.x", CSB_V1_CSBGRAPHICS_M11_C040_PANEL_X, 80,
              "PANEL.C:1632 G0032 panel box");
    check_int("c040.y", CSB_V1_CSBGRAPHICS_M11_C040_PANEL_Y, 52,
              "PANEL.C:1632 G0032 panel box");
    check_int("c040.w", CSB_V1_CSBGRAPHICS_M11_C040_PANEL_W, 144,
              "PANEL.C:1632 C072 byte width = 144 pixels");
    check_int("c040.h", CSB_V1_CSBGRAPHICS_M11_C040_PANEL_H, 73,
              "PANEL.C:1632 G0032 panel box");
    check_int("palette.max", (int)CSB_V1_CSBGRAPHICS_M11_PALETTE_MAX, 15,
              "V1 indexed 4bpp palette budget");

    check_contains("evidence.read_index", evidence, "CSBWin/Graphics.cpp:1918");
    check_contains("evidence.locate", evidence, "CSBWin/Graphics.cpp:1643");
    check_contains("evidence.read_graphic", evidence, "CSBWin/Graphics.cpp:1717");
    check_contains("evidence.c017", evidence, "DEFS.H:2178 C017_GRAPHIC_INVENTORY");
    check_contains("evidence.c040", evidence,
                   "DEFS.H:2200 C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE");
    check_contains("evidence.panel", evidence, "PANEL.C:1632");
    check_contains("evidence.inventory", evidence, "PANEL.C:2376");
    check_contains("evidence.viewport", evidence, "C000_DERIVED_BITMAP_VIEWPORT");

    check_str("route.none",
              csb_v1_csbgraphics_m11_route_name(CSB_V1_CSBGRAPHICS_M11_ROUTE_NONE),
              "none");
    check_str("route.viewport",
              csb_v1_csbgraphics_m11_route_name(CSB_V1_CSBGRAPHICS_M11_ROUTE_VIEWPORT_DERIVED),
              "viewport-derived");
    check_str("decision.bind",
              csb_v1_csbgraphics_m11_decision_name(
                  CSB_V1_CSBGRAPHICS_M11_DECISION_BIND_OVERRIDE),
              "bind-override");
    check_str("decision.fallback",
              csb_v1_csbgraphics_m11_decision_name(
                  CSB_V1_CSBGRAPHICS_M11_DECISION_FALLBACK_ORIGINAL),
              "fallback-original");
}

static void test_viewport_override_binding(void)
{
    uint8_t pixels[64 * 40];
    uint8_t framebuffer[CSB_V1_CSBGRAPHICS_M11_SOURCE_W *
                        CSB_V1_CSBGRAPHICS_M11_SOURCE_H];
    CSB_V1_CSBGraphicsEntrySpan span = span_for(73u);
    CSB_V1_CSBGraphicsDecodedBitmap decoded =
        decoded_for(73u, 64u, 40u, 15u, pixels, sizeof(pixels));
    CSB_V1_CSBGraphicsM11Binding binding;

    memset(pixels, 3, sizeof(pixels));
    memset(framebuffer, 0, sizeof(framebuffer));
    check_int("viewport.prepare",
              csb_v1_csbgraphics_m11_prepare_binding(&span, &decoded, &binding),
              1, "trusted decoded viewport graphic");
    check_int("viewport.decision", binding.decision,
              CSB_V1_CSBGRAPHICS_M11_DECISION_BIND_OVERRIDE,
              binding.source_evidence);
    check_int("viewport.route", binding.route,
              CSB_V1_CSBGRAPHICS_M11_ROUTE_VIEWPORT_DERIVED,
              binding.source_evidence);
    check_int("viewport.dest_x", binding.destination_x, 0, binding.source_evidence);
    check_int("viewport.dest_y", binding.destination_y, 33, binding.source_evidence);
    check_int("viewport.dest_w", binding.destination_w, 224, binding.source_evidence);
    check_int("viewport.dest_h", binding.destination_h, 136, binding.source_evidence);
    check_int("viewport.redraw", binding.needs_viewport_redraw, 1,
              binding.source_evidence);
    check_int("viewport.hud_redraw", binding.needs_hud_redraw, 0,
              binding.source_evidence);
    check_int("viewport.palette_preserved", binding.preserves_v1_palette_indices, 1,
              binding.source_evidence);
    check_str("viewport.reason", binding.reason, "trusted-decoded-override");
    check_int("viewport.apply",
              csb_v1_csbgraphics_m11_apply_binding(
                  &binding, &decoded, framebuffer,
                  CSB_V1_CSBGRAPHICS_M11_SOURCE_W,
                  CSB_V1_CSBGRAPHICS_M11_SOURCE_H,
                  CSB_V1_CSBGRAPHICS_M11_SOURCE_W),
              1, binding.source_evidence);
    check_int("viewport.apply.top_left",
              framebuffer[CSB_V1_CSBGRAPHICS_M11_VIEWPORT_Y *
                          CSB_V1_CSBGRAPHICS_M11_SOURCE_W +
                          CSB_V1_CSBGRAPHICS_M11_VIEWPORT_X],
              3, "CSBgraphics viewport override copied into M11 framebuffer");
    check_int("viewport.apply.before",
              framebuffer[(CSB_V1_CSBGRAPHICS_M11_VIEWPORT_Y - 1) *
                          CSB_V1_CSBGRAPHICS_M11_SOURCE_W],
              0, "CSBgraphics viewport override does not write before viewport");
}

static void test_hud_inventory_binding(void)
{
    uint8_t pixels[CSB_V1_CSBGRAPHICS_M11_VIEWPORT_W *
                   CSB_V1_CSBGRAPHICS_M11_VIEWPORT_H];
    CSB_V1_CSBGraphicsEntrySpan span = span_for(17u);
    CSB_V1_CSBGraphicsDecodedBitmap decoded =
        decoded_for(17u,
                    CSB_V1_CSBGRAPHICS_M11_VIEWPORT_W,
                    CSB_V1_CSBGRAPHICS_M11_VIEWPORT_H,
                    12u, pixels, sizeof(pixels));
    CSB_V1_CSBGraphicsM11Binding binding;

    memset(pixels, 4, sizeof(pixels));
    check_int("inventory.prepare",
              csb_v1_csbgraphics_m11_prepare_binding(&span, &decoded, &binding),
              1, "PANEL.C:2376 C017 inventory");
    check_int("inventory.decision", binding.decision,
              CSB_V1_CSBGRAPHICS_M11_DECISION_BIND_OVERRIDE,
              binding.source_evidence);
    check_int("inventory.route", binding.route,
              CSB_V1_CSBGRAPHICS_M11_ROUTE_HUD_INVENTORY,
              binding.source_evidence);
    check_int("inventory.dest_w", binding.destination_w, 224,
              binding.source_evidence);
    check_int("inventory.dest_h", binding.destination_h, 136,
              binding.source_evidence);
    check_int("inventory.viewport_redraw", binding.needs_viewport_redraw, 0,
              binding.source_evidence);
    check_int("inventory.hud_redraw", binding.needs_hud_redraw, 1,
              binding.source_evidence);
}

static void test_hud_c040_panel_binding(void)
{
    uint8_t pixels[CSB_V1_CSBGRAPHICS_M11_C040_PANEL_W *
                   CSB_V1_CSBGRAPHICS_M11_C040_PANEL_H];
    uint8_t framebuffer[CSB_V1_CSBGRAPHICS_M11_SOURCE_W *
                        CSB_V1_CSBGRAPHICS_M11_SOURCE_H];
    CSB_V1_CSBGraphicsEntrySpan span = span_for(40u);
    CSB_V1_CSBGraphicsDecodedBitmap decoded =
        decoded_for(40u,
                    CSB_V1_CSBGRAPHICS_M11_C040_PANEL_W,
                    CSB_V1_CSBGRAPHICS_M11_C040_PANEL_H,
                    6u, pixels, sizeof(pixels));
    CSB_V1_CSBGraphicsM11Binding binding;

    memset(pixels, 6, sizeof(pixels));
    memset(framebuffer, 0, sizeof(framebuffer));
    check_int("c040.prepare",
              csb_v1_csbgraphics_m11_prepare_binding(&span, &decoded, &binding),
              1, "PANEL.C:1632 C040 panel");
    check_int("c040.decision", binding.decision,
              CSB_V1_CSBGRAPHICS_M11_DECISION_BIND_OVERRIDE,
              binding.source_evidence);
    check_int("c040.route", binding.route,
              CSB_V1_CSBGRAPHICS_M11_ROUTE_HUD_RESURRECT_PANEL,
              binding.source_evidence);
    check_int("c040.dest_x", binding.destination_x, 80, binding.source_evidence);
    check_int("c040.dest_y", binding.destination_y, 52, binding.source_evidence);
    check_int("c040.dest_w", binding.destination_w, 144, binding.source_evidence);
    check_int("c040.dest_h", binding.destination_h, 73, binding.source_evidence);
    check_int("c040.hud_redraw", binding.needs_hud_redraw, 1,
              binding.source_evidence);
    check_int("c040.apply",
              csb_v1_csbgraphics_m11_apply_binding(
                  &binding, &decoded, framebuffer,
                  CSB_V1_CSBGRAPHICS_M11_SOURCE_W,
                  CSB_V1_CSBGRAPHICS_M11_SOURCE_H,
                  CSB_V1_CSBGRAPHICS_M11_SOURCE_W),
              1, binding.source_evidence);
    check_int("c040.apply.top_left",
              framebuffer[CSB_V1_CSBGRAPHICS_M11_C040_PANEL_Y *
                          CSB_V1_CSBGRAPHICS_M11_SOURCE_W +
                          CSB_V1_CSBGRAPHICS_M11_C040_PANEL_X],
              6, "CSBgraphics C040 override copied into source panel rect");
    check_int("c040.apply.outside",
              framebuffer[(CSB_V1_CSBGRAPHICS_M11_C040_PANEL_Y - 1) *
                          CSB_V1_CSBGRAPHICS_M11_SOURCE_W +
                          CSB_V1_CSBGRAPHICS_M11_C040_PANEL_X],
              0, "CSBgraphics C040 override preserves pixels outside panel");
}

static void test_fallbacks_are_explicit(void)
{
    uint8_t pixels[32 * 32];
    uint8_t framebuffer[CSB_V1_CSBGRAPHICS_M11_SOURCE_W *
                        CSB_V1_CSBGRAPHICS_M11_SOURCE_H];
    CSB_V1_CSBGraphicsEntrySpan span = span_for(40u);
    CSB_V1_CSBGraphicsDecodedBitmap decoded =
        decoded_for(40u, 32u, 32u, 15u, pixels, sizeof(pixels));
    CSB_V1_CSBGraphicsM11Binding binding;

    memset(pixels, 7, sizeof(pixels));
    memset(framebuffer, 0, sizeof(framebuffer));

    check_int("missing.out",
              csb_v1_csbgraphics_m11_prepare_binding(&span, &decoded, NULL),
              0, "argument guard");

    span.compressed_size = 0u;
    check_int("empty.prepare",
              csb_v1_csbgraphics_m11_prepare_binding(&span, &decoded, &binding),
              1, "empty CSBgraphics.dat span");
    check_int("empty.decision", binding.decision,
              CSB_V1_CSBGRAPHICS_M11_DECISION_FALLBACK_ORIGINAL,
              binding.source_evidence);
    check_str("empty.reason", binding.reason, "empty-span");

    span = span_for(40u);
    decoded.trusted = 0;
    check_int("untrusted.prepare",
              csb_v1_csbgraphics_m11_prepare_binding(&span, &decoded, &binding),
              1, "trusted decode gate");
    check_str("untrusted.reason", binding.reason, "decoded-payload-not-trusted");

    decoded = decoded_for(41u, 32u, 32u, 15u, pixels, sizeof(pixels));
    check_int("mismatch.prepare",
              csb_v1_csbgraphics_m11_prepare_binding(&span, &decoded, &binding),
              1, "entry id guard");
    check_str("mismatch.reason", binding.reason, "entry-mismatch");

    decoded = decoded_for(40u, 144u, 73u, 16u, pixels, sizeof(pixels));
    decoded.indexed_pixel_count = sizeof(pixels);
    check_int("palette.prepare",
              csb_v1_csbgraphics_m11_prepare_binding(&span, &decoded, &binding),
              1, "4bpp palette guard");
    check_str("palette.reason", binding.reason, "palette-budget");

    decoded = decoded_for(40u, 144u, 73u, 15u, pixels, sizeof(pixels));
    check_int("bounds.prepare",
              csb_v1_csbgraphics_m11_prepare_binding(&span, &decoded, &binding),
              1, "decoded pixel count guard");
    check_str("bounds.reason", binding.reason, "decoded-pixel-bounds");

    decoded = decoded_for(40u, 32u, 32u, 15u, pixels, sizeof(pixels));
    check_int("geometry.prepare",
              csb_v1_csbgraphics_m11_prepare_binding(&span, &decoded, &binding),
              1, "C040 geometry guard");
    check_str("geometry.reason", binding.reason, "c040-panel-geometry");

    span = span_for(200u);
    decoded = decoded_for(200u, 32u, 32u, 15u, pixels, sizeof(pixels));
    check_int("unsupported.prepare",
              csb_v1_csbgraphics_m11_prepare_binding(&span, &decoded, &binding),
              1, "unsupported route stays fallback");
    check_str("unsupported.reason", binding.reason, "unsupported-entry-route");
    check_int("fallback.apply.rejected",
              csb_v1_csbgraphics_m11_apply_binding(
                  &binding, &decoded, framebuffer,
                  CSB_V1_CSBGRAPHICS_M11_SOURCE_W,
                  CSB_V1_CSBGRAPHICS_M11_SOURCE_H,
                  CSB_V1_CSBGRAPHICS_M11_SOURCE_W),
              0, "fallback bindings do not mutate M11 framebuffer");
}

int main(void)
{
    test_contract_constants_and_evidence();
    test_viewport_override_binding();
    test_hud_inventory_binding();
    test_hud_c040_panel_binding();
    test_fallbacks_are_explicit();

    printf("csb_v1_csbgraphics_m11_binding_readiness: %d assertions, %d failures\n",
           g_assertions, g_failures);
    return g_failures == 0 ? 0 : 1;
}
