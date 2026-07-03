/*
 * csb_v1_csbgraphics_m11_runtime_plan.h
 *
 * Startup/runtime manifest for CSBWin CSBgraphics.dat entries that can
 * feed the M11 indexed framebuffer handoff.
 *
 * This layer deliberately does not discover files by name and does not
 * guess bitmap geometry from compressed payload size. It consumes the
 * hash-owned real-scan cache plus the existing CSBgraphics.dat index and
 * records only entries whose M11 geometry is known or supplied by the
 * caller. The actual decode/copy remains in
 * csb_v1_csbgraphics_m11_binding_readiness.
 *
 * Source references:
 *   - CSBWin/Graphics.cpp:1838 OpenCSBgraphicsFile
 *   - CSBWin/Graphics.cpp:1918 ReadGraphicsIndex
 *   - CSBWin/Graphics.cpp:1643 LocateNthGraphic
 *   - CSBWin/Graphics.cpp:1717 ReadGraphic
 *   - ReDMCSB DEFS.H C017/C040/C000_DERIVED_BITMAP_VIEWPORT
 *   - ReDMCSB PANEL.C F0346/F0370 viewport and panel blit lanes
 */

#ifndef FIRESTAFF_CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_H
#define FIRESTAFF_CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_H

#include "csb_v1_csbgraphics_dat_real_scan.h"
#include "csb_v1_csbgraphics_m11_binding_readiness.h"
#include "csb_v1_viewport_pc34_compat.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_MAX_ENTRIES 16

typedef enum {
    CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK = 0,
    CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_ARGUMENT = -1,
    CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_CACHE = -2,
    CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_SUPPORTED_ENTRIES = -3,
    CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_FULL = -4,
    CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_GEOMETRY = -5,
    CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_APPLY = -6,
    CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_DEFERRED_COMPOSITE = -7
} CSB_V1_CSBGraphicsM11RuntimePlanResult;

typedef enum {
    CSB_V1_CSBGRAPHICS_M11_CUSTOM_BACKGROUND_LAYER_LARGE = 0,
    CSB_V1_CSBGRAPHICS_M11_CUSTOM_BACKGROUND_LAYER_MIDDLE = 1,
    CSB_V1_CSBGRAPHICS_M11_CUSTOM_BACKGROUND_LAYER_NEAR = 2
} CSB_V1_CSBGraphicsM11CustomBackgroundLayer;

typedef struct {
    uint32_t entry_index;
    uint32_t mask_entry_index;
    uint16_t expected_width;
    uint16_t expected_height;
    uint16_t decompressed_size;
    uint16_t mask_decompressed_size;
    CSB_V1_CSBGraphicsM11Route route;
    int explicit_dimensions;
    int needs_viewport_redraw;
    int needs_hud_redraw;
    int needs_custom_background_composite;
    int deferred_masked_composite;
    CSB_V1_CSBGraphicsM11CustomBackgroundLayer custom_background_layer;
} CSB_V1_CSBGraphicsM11RuntimePlanEntry;

typedef struct {
    int ready;
    int cache_loaded;
    uint32_t source_entry_count;
    uint32_t supported_present_count;
    uint32_t planned_count;
    uint32_t skipped_unknown_geometry_count;
    uint32_t custom_background_pair_count;
    char source_path[CSB_V1_CSBGRAPHICS_DAT_REAL_PATH_CAP];
    char source_md5[CSB_V1_CSBGRAPHICS_DAT_REAL_MD5_CAP];
    char source_label[CSB_V1_CSBGRAPHICS_DAT_REAL_LABEL_CAP];
    CSB_V1_CSBGraphicsM11RuntimePlanEntry
        entries[CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_MAX_ENTRIES];
} CSB_V1_CSBGraphicsM11RuntimePlan;

void csb_v1_csbgraphics_m11_runtime_plan_init(
    CSB_V1_CSBGraphicsM11RuntimePlan *plan);

int csb_v1_csbgraphics_m11_runtime_plan_build_from_cache(
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    CSB_V1_CSBGraphicsM11RuntimePlan *plan);

int csb_v1_csbgraphics_m11_runtime_plan_add_explicit_entry(
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    uint32_t entry_index,
    uint16_t expected_width,
    uint16_t expected_height,
    CSB_V1_CSBGraphicsM11RuntimePlan *plan);

int csb_v1_csbgraphics_m11_runtime_plan_add_custom_background_skin_def(
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    const uint16_t *skin_def_words,
    size_t skin_def_word_count,
    CSB_V1_CSBGraphicsM11RuntimePlan *plan);

int csb_v1_csbgraphics_m11_runtime_plan_apply_entry(
    const CSB_V1_CSBGraphicsM11RuntimePlan *plan,
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    uint32_t entry_index,
    uint8_t *framebuffer,
    int framebuffer_width,
    int framebuffer_height,
    int framebuffer_stride,
    CSB_V1_CSBGraphicsM11Binding *out_binding);

int csb_v1_csbgraphics_m11_runtime_plan_apply_custom_background_entry(
    const CSB_V1_CSBGraphicsM11RuntimePlan *plan,
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    uint32_t entry_index,
    const CSB_V1_ViewportCustomBackgroundMask *mask_geometry,
    uint32_t *viewport_words,
    size_t viewport_word_count,
    int viewport_width_pixels);

int csb_v1_csbgraphics_m11_runtime_plan_apply_custom_background_room_layer(
    const CSB_V1_CSBGraphicsM11RuntimePlan *plan,
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    int room_num,
    CSB_V1_CSBGraphicsM11CustomBackgroundLayer layer,
    const uint16_t *skin_def_words,
    size_t skin_def_word_count,
    const CSB_V1_ViewportCustomBackgroundMask *mask_geometry,
    uint32_t *viewport_words,
    size_t viewport_word_count,
    int viewport_width_pixels);

const CSB_V1_CSBGraphicsM11RuntimePlanEntry *
csb_v1_csbgraphics_m11_runtime_plan_find_entry(
    const CSB_V1_CSBGraphicsM11RuntimePlan *plan,
    uint32_t entry_index);

const char *csb_v1_csbgraphics_m11_runtime_plan_result_name(int result);
const char *csb_v1_csbgraphics_m11_runtime_plan_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_H */
