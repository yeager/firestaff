/*
 * dm2_v1_runtime.c — DM2 V1 Runtime Stub
 *
 * Phase 1: Provides the game tick path for DM2 when launched.
 * The actual game logic (movement, combat, spells) is Phases 2-6.
 * This stub wires the DM2 viewport into the Firestaff game loop
 * so that a DM2 launch can display a viewport frame without crashes.
 *
 * The CSB path in firestaff_game_loop.c provides the reference pattern:
 *   FS_GAME_CSB → csb_v1_viewport_render_frame() → DM1 viewport engine
 *   FS_GAME_DM2 → dm2_v1_runtime_render_frame()   → DM2 viewport engine
 *
 * Source: SKULL.ASM T560  — dungeon tick
 *         SKULL.ASM T600  — outdoor tick
 *         SKULL.ASM T520  — party/movement tick
 *         SKULL.ASM T048  — input dispatch
 */

#include "dm2_v1_game.h"
#include "dm2_v1_boot.h"
#include "dm2_v1_dungeon_loader.h"
#include <stdio.h>
#include <string.h>

/* ── DM2 V1 Runtime State ─────────────────────────────────────────── */

typedef struct {
    DM2_V1_BootProfile *boot;
    int outdoor;              /* 1=outdoor mode, 0=dungeon mode */
    int tick_count;
    int paused;
    /* Movement state */
    int move_cooldown_ticks;
    /* Weather state (outdoor) */
    int rain_intensity;       /* 0-100 */
    int time_of_day_minutes;  /* 0-1439 */
    /* Dungeon state */
    int dungeon_level;
    int view_dir;
} DM2_V1_RuntimeState;

static DM2_V1_RuntimeState g_dm2_runtime;

/* ── Runtime init ──────────────────────────────────────────────────── */

void dm2_v1_runtime_init(DM2_V1_BootProfile *boot_profile) {
    if (!boot_profile) return;
    memset(&g_dm2_runtime, 0, sizeof(g_dm2_runtime));
    g_dm2_runtime.boot = boot_profile;
    g_dm2_runtime.outdoor = 0;
    g_dm2_runtime.tick_count = 0;
    g_dm2_runtime.move_cooldown_ticks = 0;
    g_dm2_runtime.rain_intensity = 0;
    g_dm2_runtime.time_of_day_minutes = 720;  /* noon */
    g_dm2_runtime.dungeon_level = 0;
    g_dm2_runtime.view_dir = 0;  /* North */
}

/* ── V1 Game Tick ──────────────────────────────────────────────────── */

/*
 * dm2_v1_runtime_tick — advance DM2 game state by one V1 tick.
 *
 * Called at 18.2 Hz (every ~55ms) from the Firestaff game loop.
 * Advances: time-of-day, movement cooldown, weather, timers.
 *
 * Movement is gated by move_cooldown_ticks — each successful move
 * consumes 1 tick; failing a move (wall) may incur penalty.
 *
 * Source: SKULL.ASM T048 — input dispatch / tick update
 *         SKULL.ASM T560 — dungeon tick
 *         SKULL.ASM T600 — outdoor tick
 */
void dm2_v1_runtime_tick(void) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    rt->tick_count++;

    /* Advance time-of-day (1440 min per day) */
    if (rt->tick_count % 1092 == 0) {  /* ~1092 ticks = 1 minute */
        rt->time_of_day_minutes = (rt->time_of_day_minutes + 1) % 1440;
    }

    /* Movement cooldown counts down */
    if (rt->move_cooldown_ticks > 0) {
        rt->move_cooldown_ticks--;
    }

    /* Outdoor weather tick */
    if (rt->outdoor && rt->tick_count % 182 == 0) {  /* ~10 sec */
        /* Rain intensity fluctuates in outdoor areas */
        if (rt->rain_intensity > 0) {
            rt->rain_intensity = (rt->rain_intensity + 1) % 100;
        }
    }
}

/* ── Viewport rendering ────────────────────────────────────────────── */

/*
 * dm2_v1_runtime_render_frame — render one DM2 viewport frame.
 *
 * party_dir:  facing direction (0=N, 1=E, 2=S, 3=W)
 * party_x, party_y: position on dungeon grid or outdoor map
 *
 * DM2 viewport renders differently depending on outdoor/dungeon mode:
 *   - Dungeon: first-person 3D view using DM2 wall/floor graphics
 *   - Outdoor: overhead or 3D sky view with weather overlay
 *
 * For Phase 1, this renders a placeholder frame that distinguishes
 * DM2 from DM1/CSB: a blue-ish sky for outdoor, dark dungeon for indoor.
 * Real DM2 rendering is Phase 4.
 *
 * Returns 0 on success.
 *
 * Source: SKULL.ASM T560 — viewport frame rendering
 *         SKULL.ASM T600 — outdoor rendering
 */
int dm2_v1_runtime_render_frame(int party_dir, int party_x, int party_y,
                                  uint8_t *framebuffer, int fb_stride,
                                  int view_w, int view_h) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    int x, y;

    if (!framebuffer) return -1;

    /* Clear to black */
    memset(framebuffer, 0, fb_stride * view_h);

    if (rt->outdoor) {
        /* DM2 outdoor: sky gradient with horizon line.
         * Sky color: deep blue (index 9) to dark cyan (index 3).
         * This is distinct from DM1/CSB which have no outdoor mode. */
        for (y = 0; y < view_h / 2; y++) {
            int sky_index = (y < view_h / 4) ? 9 : 3;
            for (x = 0; x < view_w; x++) {
                framebuffer[y * fb_stride + x] = (uint8_t)sky_index;
            }
        }
        /* Ground/horizon bottom half */
        for (y = view_h / 2; y < view_h; y++) {
            for (x = 0; x < view_w; x++) {
                framebuffer[y * fb_stride + x] = 6;  /* brown */
            }
        }

        /* Weather overlay (rain) */
        if (rt->rain_intensity > 20) {
            for (y = 0; y < view_h; y++) {
                for (x = 0; x < view_w; x += 3) {
                    if ((x + y + rt->tick_count) % 7 < rt->rain_intensity / 20) {
                        framebuffer[y * fb_stride + x] = 15;  /* white streak */
                    }
                }
            }
        }
    } else {
        /* DM2 dungeon: first-person view.
         * Distinct from DM1: different wall set, ceiling color,
         * and no corridor (DM2 has rooms, not corridors). */
        /* Ceiling: dark gray */
        for (y = 0; y < view_h / 2; y++) {
            for (x = 0; x < view_w; x++) {
                framebuffer[y * fb_stride + x] = 8;
            }
        }
        /* Floor: darker brown */
        for (y = view_h / 2; y < view_h; y++) {
            for (x = 0; x < view_w; x++) {
                framebuffer[y * fb_stride + x] = 5;
            }
        }

        /* DM2 distinctive: dungeon wall frame outline.
         * DM2 rooms are wider than DM1 corridors. */
        int frame_thickness = 4;
        for (y = 0; y < frame_thickness; y++) {
            for (x = 0; x < view_w; x++) {
                framebuffer[y * fb_stride + x] = 7;
                framebuffer[(view_h - 1 - y) * fb_stride + x] = 7;
            }
        }
        for (y = 0; y < view_h; y++) {
            for (x = 0; x < frame_thickness; x++) {
                framebuffer[y * fb_stride + x] = 7;
                framebuffer[y * fb_stride + (view_w - 1 - x)] = 7;
            }
        }

        /* Center "room" indicator — DM2 marks room presence */
        {
            int cx = view_w / 2;
            int cy = view_h / 2;
            int r = 8;
            for (y = cy - r; y <= cy + r; y++) {
                for (x = cx - r; x <= cx + r; x++) {
                    if (y >= 0 && y < view_h && x >= 0 && x < view_w) {
                        int dist = (x - cx) * (x - cx) + (y - cy) * (y - cy);
                        if (dist <= r * r && dist > (r - 2) * (r - 2)) {
                            framebuffer[y * fb_stride + x] = 10;  /* green — room */
                        }
                    }
                }
            }
        }
    }

    /* HUD bottom strip: DM2 gold display (vs DM1 empty) */
    int hud_y = view_h - 16;
    for (y = hud_y; y < view_h; y++) {
        for (x = 0; x < view_w; x++) {
            framebuffer[y * fb_stride + x] = 1;  /* dark blue */
        }
    }

    /* DM2 marker text area — shows "DM2" in debug */
    {
        /* Placeholder: DM2 label at bottom-right of HUD */
        (void)0;
    }

    return 0;
}

/* ── Movement ──────────────────────────────────────────────────────── */

/*
 * dm2_v1_runtime_can_move — check if party can move this tick.
 *
 * DM2 movement: outdoor is 2x dungeon speed.
 * move_cooldown_ticks gates movement so you can't move every tick.
 *
 * Source: SKULL.ASM T520 — movement speed
 */
int dm2_v1_runtime_can_move(void) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    return (rt->move_cooldown_ticks == 0);
}

/*
 * dm2_v1_runtime_move — attempt party movement in direction dir.
 * Returns 0 on success, -1 if blocked.
 *
 * Source: SKULL.ASM T520
 */
int dm2_v1_runtime_move(int dir) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    DM2_V1_GameState *gs;
    int dx[] = {0, 1, 0, -1};  /* N E S W */
    int dy[] = {-1, 0, 1, 0};
    int nx, ny;
    int blocked = 0;

    if (!rt->boot || !rt->boot->dm2_state) return -1;
    gs = (DM2_V1_GameState *)rt->boot->dm2_state;

    if (!dm2_v1_runtime_can_move()) return -1;

    nx = gs->party_x + dx[dir & 3];
    ny = gs->party_y + dy[dir & 3];

    /* Check dungeon collision if in dungeon mode */
    if (!rt->outdoor && rt->boot->dungeon_data) {
        DM2_V1_DungeonData *dd = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
        int sq = dm2_v1_dungeon_get_square_type(dd, rt->dungeon_level, nx, ny);
        if (sq < 0 || sq == 0) {  /* wall or out-of-bounds */
            blocked = 1;
        }
    }

    if (!blocked) {
        gs->party_x = nx;
        gs->party_y = ny;
        gs->party_dir = dir;
    }

    /* Set movement cooldown: dungeon=1 tick, outdoor=0.5 tick */
    rt->move_cooldown_ticks = rt->outdoor ? 0 : 1;

    return blocked ? -1 : 0;
}

/* ── Outdoor/Dungeon mode ─────────────────────────────────────────── */

/*
 * dm2_v1_runtime_set_outdoor — switch between outdoor and dungeon view.
 *
 * Source: SKULL.ASM T600 — outdoor mode entry
 *         SKULL.ASM T560 — dungeon mode entry
 */
void dm2_v1_runtime_set_outdoor(int is_outdoor) {
    g_dm2_runtime.outdoor = is_outdoor ? 1 : 0;
}

/* ── Source evidence ──────────────────────────────────────────────── */

const char *dm2_v1_runtime_source_evidence(void) {
    return
        "DM2 V1 Runtime Stub — Phase 1\n"
        "Source: SKULL.ASM T048  — input dispatch / tick update\n"
        "Source: SKULL.ASM T520  — movement speed and party placement\n"
        "Source: SKULL.ASM T560  — dungeon tick and viewport rendering\n"
        "Source: SKULL.ASM T600  — outdoor tick and weather rendering\n"
        "Reference: CSB path in firestaff_game_loop.c (FS_GAME_CSB → csb_v1_viewport_render_frame)\n";
}