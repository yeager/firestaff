#ifndef FIRESTAFF_DM1_V2_HUD_OVERLAY_PC34_H
#define FIRESTAFF_DM1_V2_HUD_OVERLAY_PC34_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int direction;
    float needle_angle;
    bool animated;
} M11_V2_HudCompass;

typedef struct {
    int current_level;
    int max_level;
} M11_V2_HudDepth;

typedef struct {
    M11_V2_HudCompass compass;
    M11_V2_HudDepth depth;
    bool visible;
    uint8_t opacity;
    bool stats_bar_visible;
    /* pass601a: movement/turn complete signals for V2 interpolation consumers.
     * Source-lock: ReDMCSB GAMELOOP.C:90 viewport redraw cadence. */
    int moveCompletePending;
    int turnCompletePending;
} M11_V2_HudOverlay;

void v2_hud_init(void);
void v2_hud_set_direction(int dir);
void v2_hud_set_level(int cur, int max);
void v2_hud_render(uint8_t* fb, int w, int h);
void v2_hud_toggle(void);
void v2_hud_set_opacity(uint8_t val);

/* V2.2 health-pulse alpha — V1 tick-synchronous ping-pong, 2 Hz.
 * Source: v22_hud_pulse_v1_sync marker; ReDMCSB TIMELINE.C F0260. */
void v22_hud_pulse_v1_tick(void);
void v22_hud_start_health_pulse(void);
float v22_hud_health_pulse_alpha(void);

/* pass601a: move/turn complete signals for the V2 interpolation layer.
 * Raised when a camera move/turn interpolation finishes — consumers can
 * poll without directly poking the camera struct.
 * Source-lock: ReDMCSB GAMELOOP.C:90 viewport redraw cadence. */
void v22_hud_notify_move_complete(void);
void v22_hud_clear_move_complete(void);
int v22_hud_is_move_complete_pending(void);
void v22_hud_notify_turn_complete(void);
void v22_hud_clear_turn_complete(void);
int v22_hud_is_turn_complete_pending(void);

/* V2.1 champion panel renderer — pure presentation overlay.
 * Draws the 4-champion status panel (portrait/name/HP/Stamina/Mana/hand)
 * into the bottom region of the framebuffer using V1 coordinates.
 * ReDMCSB: PANEL.C F0395-F0404; CHAMDRAW.C ~180; STATS.C F0090-F0092.
 * Does not mutate game state or command queues. */
void v22_hud_render_champion_panel(uint8_t* fb, int w, int h,
                                    int champion_hp_pct[4],
                                    int champion_stam_pct[4],
                                    int champion_mana_pct[4]);

/* V2 champion select — renders focused champion slot into framebuffer.
 * Pure presentation; does not mutate game state or command queues.
 * Source: CLIKCHAM.C:24-35; COMMAND.C:484-497; CHAMDRAW.C ~180. */
void v2_champion_select_render_fb(uint8_t* fb, int w, int h);

#ifdef __cplusplus
}
#endif

#endif
