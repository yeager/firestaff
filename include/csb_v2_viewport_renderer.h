
#ifndef FIRESTAFF_CSB_V2_VIEWPORT_RENDERER_H
#define FIRESTAFF_CSB_V2_VIEWPORT_RENDERER_H
#include <stdint.h>
#include "dm1_v2_anim_timing.h"
#include "csb_v2_lighting_dynamic.h"
#include "csb_v2_phase_gate_pc34.h"

/* Phase gate: all functions in this header belong to
 * CSB_V2_PHASE_DOMAIN_RENDER_PRESENTATION and require
 * csb_v2_phase_gate_v2_active() to return true before use.
 * V1 game logic (dungeon loading, combat, movement) is unaffected.
 * See csb_v2_phase_gate_pc34.h Phase 0 rules.
 *
 * CSB V2 Viewport — upscaled CSB rendering with V2.1/V2.2 features.
 * Shares DM1 V2 upscale pipeline (EPX) but adds:
 *   - Custom background upscaling per DSA room
 *   - Prison door intro animation (smooth)
 *   - Extended creature sprite sheets */

typedef struct {
    int scale_factor;
    int epx_enabled;
    int custom_bg_active;
    int prison_door_progress; /* 0-100 */
    V2_AnimClock clock;
} CSB_V2_ViewportState;

void csb_v2_viewport_init(CSB_V2_ViewportState *s, int scale);
void csb_v2_viewport_v1_tick(CSB_V2_ViewportState *s, uint32_t now_ms);
void csb_v2_viewport_render_frame(CSB_V2_ViewportState *s, uint32_t now_ms);
float csb_v2_viewport_sub_tick(const CSB_V2_ViewportState *s);

void csb_v2_viewport_set_dungeon_light(CSB_V2_ViewportState *s, int dungeonLevel);
void csb_v2_viewport_set_ambient_light(float ambient);
int csb_v2_viewport_add_torch(float x,
                              float y,
                              uint8_t intensity,
                              uint8_t r,
                              uint8_t g,
                              uint8_t b,
                              int flicker);
void csb_v2_viewport_remove_torch(int lightIndex);
void csb_v2_viewport_compute_light_map(void);
void csb_v2_viewport_get_tile_light(int tileX,
                                    int tileY,
                                    uint8_t *r,
                                    uint8_t *g,
                                    uint8_t *b);
void csb_v2_viewport_trigger_dsa_light_event(CSB_V2_LightEventType type,
                                             float durationSeconds,
                                             float intensity);
CSB_V2_LightEventType csb_v2_viewport_light_event_type(void);
void csb_v2_viewport_advance_chaos(float dtSeconds);
int csb_v2_viewport_chaos_active_count(void);
void csb_v2_viewport_get_chaos_overlay(float *outR,
                                       float *outG,
                                       float *outB,
                                       float *outAlpha);
const char *csb_v2_viewport_source_evidence(void);
#endif
