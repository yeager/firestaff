/*
 * csb_hint_oracle_graphical_overlay.h
 *
 * Minimal graphical overlay boundary for the CSB Utility Disk
 * HCSB.HTC Hint Oracle.
 *
 * Scope:
 *   - Draws a framed 320x200-style oracle panel into a caller-owned
 *     8-bit framebuffer.
 *   - Uses decoded HCSB.HTC first-page text when a RealCache is staged.
 *   - Stays variant-agnostic; csb_hint_oracle_htc_variant remains the
 *     only place that names R1/R2/R3/FR/GE catalog variants.
 *
 * Source references:
 *   - ReDMCSB HINTHTC.C:177-358 defines the hint/page table parsed by
 *     csb_hint_oracle_htc.
 *   - ReDMCSB HINTLZW.C:122-212 defines the on-demand page decode used
 *     by csb_hint_oracle_htc_real_decompress_first_page().
 *   - dmweb Hint Oracle Files page describes the same HCSB.HTC layout.
 *
 * Non-goals:
 *   - No M11/M12 event-loop integration.
 *   - No parity claim against original Utility Disk screen pixels.
 *   - No game-data discovery beyond the already staged RealCache.
 */

#ifndef FIRESTAFF_CSB_HINT_ORACLE_GRAPHICAL_OVERLAY_H
#define FIRESTAFF_CSB_HINT_ORACLE_GRAPHICAL_OVERLAY_H

#include <stddef.h>
#include <stdint.h>

#include "csb_hint_oracle_htc_real_scan.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_HINT_ORACLE_OVERLAY_DEFAULT_FB_W 320
#define CSB_HINT_ORACLE_OVERLAY_DEFAULT_FB_H 200
#define CSB_HINT_ORACLE_OVERLAY_TEXT_CAP 2048u

typedef enum {
    CSB_HINT_ORACLE_OVERLAY_OK = 0,
    CSB_HINT_ORACLE_OVERLAY_ERR_ARGUMENT = -1,
    CSB_HINT_ORACLE_OVERLAY_ERR_NOT_LOADED = -2,
    CSB_HINT_ORACLE_OVERLAY_ERR_HINT_OUT_OF_RANGE = -3,
    CSB_HINT_ORACLE_OVERLAY_ERR_DECODE = -4,
    CSB_HINT_ORACLE_OVERLAY_ERR_GEOMETRY = -5
} CSB_HintOracleOverlay_Result;

typedef struct {
    int x;
    int y;
    int w;
    int h;
    uint8_t background;
    uint8_t border;
    uint8_t title;
    uint8_t text;
    uint8_t shadow;
} CSB_HintOracleOverlay_Config;

typedef struct {
    size_t background_pixels;
    size_t border_pixels;
    size_t glyph_pixels;
    size_t chars_drawn;
    size_t lines_drawn;
    int clipped;
} CSB_HintOracleOverlay_Stats;

void csb_hint_oracle_overlay_default_config(
    CSB_HintOracleOverlay_Config *cfg);

int csb_hint_oracle_overlay_render_text(
    const char *title,
    const char *decoded_text,
    uint8_t *framebuffer,
    int framebuffer_w,
    int framebuffer_h,
    const CSB_HintOracleOverlay_Config *cfg,
    CSB_HintOracleOverlay_Stats *out_stats);

int csb_hint_oracle_overlay_render_hint(
    const CSB_HintOracleHTC_RealCache *cache,
    size_t hint_index,
    uint8_t *framebuffer,
    int framebuffer_w,
    int framebuffer_h,
    const CSB_HintOracleOverlay_Config *cfg,
    CSB_HintOracleOverlay_Stats *out_stats);

const char *csb_hint_oracle_overlay_result_name(int result);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_HINT_ORACLE_GRAPHICAL_OVERLAY_H */
