#ifndef FIRESTAFF_CSB_V2_HOOKS_STUB_PC34_H
#define FIRESTAFF_CSB_V2_HOOKS_STUB_PC34_H

/**
 * CSB V2 Phase 0 — V1-compatible V2 Hook Stubs
 * =============================================
 *
 * This header provides inline stub implementations for all V2-only
 * hooks that V1 CSB code may call through. When V2 presentation is
 * disabled (the default), all stubs are no-ops / return safe zero
 * values so V1 game logic compiles and runs unchanged.
 *
 * Usage from V1 code:
 *   #include "csb_v2_hooks_stub_pc34.h"
 *   // Then call any csb_v2_hook_*() function — all are safe to call.
 *
 * Phase 0 rule: calling any csb_v2_hook_*() function from V1 code
 * MUST NOT alter V1 game-logic state. All stubs are compile-time
 * no-ops that return zero/false/NULL.
 *
 * These stubs are the "thin ice" layer — any V2-only hook that V1
 * code calls MUST have a stub here to ensure compile-time safety.
 *
 * See also: csb_v2_phase_gate_pc34.h — the runtime decision API.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Forward declarations for V2 types (opaque to V1; stub returns zero) */
struct CSB_V2_ViewportState;
struct CSB_V2_LightMap;
struct CSB_V2_AnimClock;

/* ================================================================
 * V2 Viewport hooks
 * ================================================================ */

/** V2 viewport init. Stub: no-op. */
static inline void csb_v2_hook_viewport_init(void *state, int scale) {
    (void)state; (void)scale;
}

/** V2 tick through the V1 render clock. Stub: no-op. */
static inline void csb_v2_hook_viewport_v1_tick(void *state, uint32_t now_ms) {
    (void)state; (void)now_ms;
}

/** V2 render frame. Stub: no-op. */
static inline void csb_v2_hook_viewport_render_frame(void *state, uint32_t now_ms) {
    (void)state; (void)now_ms;
}

/** V2 sub-tick fraction. Stub: returns 0.0 (no interpolation). */
static inline float csb_v2_hook_viewport_sub_tick(const void *state) {
    (void)state;
    return 0.0f;
}

/** V2 smooth movement active. Stub: returns 0 (not moving). */
static inline int csb_v2_hook_smooth_is_moving(void) {
    return 0;
}

/** V2 smooth interpolated X. Stub: returns 0.0. */
static inline float csb_v2_hook_smooth_get_x(void) {
    return 0.0f;
}

/** V2 smooth interpolated Y. Stub: returns 0.0. */
static inline float csb_v2_hook_smooth_get_y(void) {
    return 0.0f;
}

/** V2 smooth interpolated angle. Stub: returns 0.0. */
static inline float csb_v2_hook_smooth_get_angle(void) {
    return 0.0f;
}

/* ================================================================
 * V2 Dynamic Lighting hooks
 * ================================================================ */

/** V2 light init. Stub: no-op. */
static inline void csb_v2_hook_light_init(void) {
}

/** V2 dungeon light level set. Stub: no-op. */
static inline void csb_v2_hook_light_set_dungeon_level(int level) {
    (void)level;
}

/** V2 dungeon level query. Stub: returns 0. */
static inline int csb_v2_hook_light_get_dungeon_level(void) {
    return 0;
}

/** V2 ambient light set. Stub: no-op. */
static inline void csb_v2_hook_light_set_ambient(float level) {
    (void)level;
}

/** V2 ambient light query. Stub: returns 0.0f. */
static inline float csb_v2_hook_light_get_ambient(void) {
    return 0.0f;
}

/** V2 light source add. Stub: returns -1 (invalid). */
static inline int csb_v2_hook_light_add_source(float x, float y,
                                                float radius,
                                                uint8_t intensity,
                                                uint8_t r, uint8_t g, uint8_t b,
                                                int flicker) {
    (void)x; (void)y; (void)radius;
    (void)intensity; (void)r; (void)g; (void)b; (void)flicker;
    return -1;
}

/** V2 light source remove. Stub: no-op. */
static inline void csb_v2_hook_light_remove_source(int index) {
    (void)index;
}

/** V2 light map compute. Stub: no-op. */
static inline void csb_v2_hook_light_compute_map(void) {
}

/** V2 tile light query. Stub: writes zero values. */
static inline void csb_v2_hook_light_get_tile(int x, int y,
                                              uint8_t *r, uint8_t *g, uint8_t *b) {
    (void)x; (void)y;
    if (r) *r = 0;
    if (g) *g = 0;
    if (b) *b = 0;
}

/** V2 light tick. Stub: no-op. */
static inline void csb_v2_hook_light_tick(float dt) {
    (void)dt;
}

/** V2 light event trigger. Stub: no-op. */
static inline void csb_v2_hook_light_event_trigger(int type,
                                                   float duration,
                                                   float intensity) {
    (void)type; (void)duration; (void)intensity;
}

/** V2 light event tick. Stub: no-op. */
static inline void csb_v2_hook_light_event_tick(float dt) {
    (void)dt;
}

/** V2 light event active. Stub: returns 0. */
static inline int csb_v2_hook_light_event_is_active(void) {
    return 0;
}

/** V2 light event current type. Stub: returns 0. */
static inline int csb_v2_hook_light_event_current_type(void) {
    return 0;
}

/* ================================================================
 * V2 Chaos Enhanced hooks
 * ================================================================ */

/** V2 chaos init. Stub: no-op. */
static inline void csb_v2_hook_chaos_init(void) {
}

/** V2 chaos on DSA script trigger. Stub: no-op. */
static inline void csb_v2_hook_chaos_on_trigger(int script_id, int flag_index) {
    (void)script_id; (void)flag_index;
}

/** V2 chaos tick. Stub: no-op. */
static inline void csb_v2_hook_chaos_tick(float dt) {
    (void)dt;
}

/** V2 chaos active count. Stub: returns 0. */
static inline int csb_v2_hook_chaos_active_count(void) {
    return 0;
}

/** V2 chaos overlay query. Stub: writes zero values. */
static inline void csb_v2_hook_chaos_render_overlay(float *r, float *g,
                                                      float *b, float *alpha) {
    if (r)     *r     = 0.0f;
    if (g)     *g     = 0.0f;
    if (b)     *b     = 0.0f;
    if (alpha) *alpha = 0.0f;
}

/* ================================================================
 * V2 Minimap hooks
 * ================================================================ */

/** V2 minimap square color. Stub: returns 0. */
static inline uint32_t csb_v2_hook_minimap_square_color(int sq_type,
                                                         int has_dsa,
                                                         int explored) {
    (void)sq_type; (void)has_dsa; (void)explored;
    return 0;
}

/* ================================================================
 * V2 Phase Gate query — shortcut for V1 code to check
 * whether V2 presentation is active. Returns 0 (V1-only boot).
 * ================================================================ */

/* ================================================================
 * V2 Asset Pipeline hooks
 * ================================================================ */

/** V2 asset pipeline init. Stub: no-op. */
static inline void csb_v2_hook_asset_pipeline_init(void) {
}

/** V2 asset pipeline configure. Stub: no-op. */
static inline void csb_v2_hook_asset_pipeline_configure(const void *config) {
    (void)config;
}

/** V2 asset get gfx mode. Stub: returns V1 ORIGINAL (0). */
static inline int csb_v2_hook_asset_get_gfx_mode(void) {
    return 0;
}

/** V2 asset set gfx mode. Stub: no-op. */
static inline void csb_v2_hook_asset_set_gfx_mode(int mode) {
    (void)mode;
}

/** V2 asset is V2 mode active. Stub: returns 0. */
static inline int csb_v2_hook_asset_is_v2_mode(void) {
    return 0;
}

/** V2 asset check surface upgrade path.
 *  Stub: returns original surface unchanged.
 *  category: surface category enum (opaque to V1)
 *  src_indexed: input V1 indexed pixels
 *  src_w/src_h: source dimensions
 *  source_palette_level: 0..5
 *  rgba_out: output RGBA buffer (caller-allocated)
 *  out_w/out_h: filled with source dims (pass-through in V1)
 *  Returns: 0=AOK, -1=unsupported category */
static inline int csb_v2_hook_asset_upscale_surface(int category,
                                                     const uint8_t *src_indexed,
                                                     int src_w, int src_h,
                                                     int source_palette_level,
                                                     uint32_t *rgba_out,
                                                     int *out_w, int *out_h) {
    (void)category;
    (void)src_indexed;
    (void)source_palette_level;
    if (out_w) *out_w = src_w;
    if (out_h) *out_h = src_h;
    return -1; /* V1: no V2 upscale available */
}

/** V2 asset set pipeline config from presentation mode index.
 *  Stub: no-op.
 *  mode_index: 0=V1, 1=V2.0, 2=V2.1, 3=V2.2 */
static inline void csb_v2_hook_asset_set_mode_from_index(int mode_index) {
    (void)mode_index;
}

/** V2 asset get best available shape source.
 *  Stub: returns V1_ORIGINAL (0). */
static inline int csb_v2_hook_asset_best_shape_source(int presentation_mode_index) {
    (void)presentation_mode_index;
    return 0;
}

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V2_HOOKS_STUB_PC34_H */
