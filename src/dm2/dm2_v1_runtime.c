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
#include "dm2_v1_pressure_plate.h"
#include "dm2_v1_runtime.h"
#include "dm2_v1_projectile_pc34_compat.h"
#include "dm2_v1_projectile_step_pc34_compat.h"
#include "dm2_v1_viewport_renderer.h"
#include "dm2_v1_shop.h"
#include "dm2_v1_trigger.h"
#include "dm2_v1_world_model.h"
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
    DM2_V1_WeatherState weather;
    int time_of_day_minutes;  /* 0-1439 */
    /* Dungeon state */
    int dungeon_level;
    int view_dir;
    int last_npc_level;
    int last_npc_x;
    int last_npc_y;
    int last_npc_id;
    int last_npc_dialog_line;
    /* V2 smooth movement callbacks — registered by dm2_v2_runtime */
    DM2_V2_MoveCallback  move_callback;
    DM2_V2_TurnCallback  turn_callback;
    DM2_V2_StairsCallback stairs_callback;
    /* Startup/render asset boundary owned by the runtime handoff. */
    DM2_V1_ViewportAssetFetch viewport_asset_fetch;
    void *viewport_asset_user;
} DM2_V1_RuntimeState;

static DM2_V1_RuntimeState g_dm2_runtime;
static int g_dm2_last_asset_floor_ceiling_count = 0;
static int g_dm2_last_fallback_floor_ceiling_count = 0;
static int g_dm2_last_asset_wall_count = 0;
static int g_dm2_last_fallback_wall_count = 0;
static int g_dm2_last_asset_door_panel_count = 0;
static int g_dm2_last_asset_door_frame_count = 0;
static int g_dm2_last_fallback_door_count = 0;

static int dm2_runtime_door_state(uint16_t square_raw) {
    return (int)(square_raw & 0x0007u);
}

static uint16_t dm2_runtime_door_set_state(uint16_t square_raw, int state) {
    return (uint16_t)((square_raw & ~0x0007u) | (uint16_t)(state & 0x0007));
}

static int dm2_runtime_door_step(int current_state, int action) {
    if (current_state == 5) return 5;
    if (action == 1) {
        if (current_state >= 4) return 4;
        return current_state + 1;
    }
    if (current_state <= 0) return 0;
    return current_state - 1;
}

static void dm2_runtime_populate_front_square(DM2_V1_RuntimeState *rt,
                                              DM2_V1_ViewportState *viewport,
                                              int party_dir,
                                              int party_x,
                                              int party_y) {
    static const int dx[4] = { 0, 1, 0, -1 };
    static const int dy[4] = { -1, 0, 1, 0 };
    static const struct {
        int square;
        int forward;
    } center_doors[] = {
        { DM2_SQ_D0C, 1 },
        { DM2_SQ_D1C, 2 },
        { DM2_SQ_D2C, 3 },
    };
    DM2_V1_DungeonData *dd;
    int dir;

    if (!rt || !viewport || rt->outdoor || !rt->boot ||
        !rt->boot->dungeon_data) {
        return;
    }
    dir = party_dir & 3;
    dd = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    for (size_t i = 0; i < sizeof(center_doors) / sizeof(center_doors[0]); ++i) {
        int raw = dm2_v1_dungeon_get_tile_raw(
            dd,
            rt->dungeon_level,
            party_x + dx[dir] * center_doors[i].forward,
            party_y + dy[dir] * center_doors[i].forward);
        int type;
        if (raw < 0) continue;
        type = raw & DM2_SQUARE_TYPE_MASK;
        if (type == DM2_SQUARE_DOOR) {
            DM2_ViewSquare *door = &viewport->squares[center_doors[i].square];
            door->square_type = DM2_SQUARE_DOOR;
            door->flags |= DM2_SQF_HAS_DOOR | DM2_SQF_HAS_WALL;
            door->door_open_pct =
                (uint8_t)(dm2_runtime_door_state((uint16_t)raw) * 25);
        }
    }
}

static int dm2_runtime_set_target_door_state(DM2_V1_RuntimeState *rt,
                                             int level,
                                             int x,
                                             int y,
                                             int state) {
    DM2_V1_DungeonData *dd;
    int raw;

    if (!rt || !rt->boot || !rt->boot->dungeon_data) return -1;
    dd = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    raw = dm2_v1_dungeon_get_tile_raw(dd, level, x, y);
    if (raw < 0) return -1;
    return dm2_v1_dungeon_set_tile_raw(
        dd, level, x, y,
        dm2_runtime_door_set_state((uint16_t)raw, state));
}

static void dm2_runtime_apply_trigger_target(DM2_V1_RuntimeState *rt,
                                             const DM2_V1_Trigger *trigger) {
    DM2_V1_GameState *gs;
    int raw;
    int state;
    int next_state;

    if (!rt || !trigger) return;
    if (!rt->boot || !rt->boot->dm2_state) return;
    gs = (DM2_V1_GameState *)rt->boot->dm2_state;

    switch (trigger->target) {
        case DM2_TRIGGER_TARGET_DOOR_OPEN:
            dm2_runtime_set_target_door_state(rt, trigger->target_level,
                                              trigger->target_x,
                                              trigger->target_y, 0);
            break;
        case DM2_TRIGGER_TARGET_DOOR_CLOSE:
            dm2_runtime_set_target_door_state(rt, trigger->target_level,
                                              trigger->target_x,
                                              trigger->target_y, 4);
            break;
        case DM2_TRIGGER_TARGET_DOOR_TOGGLE:
            if (rt->boot->dungeon_data) {
                DM2_V1_DungeonData *dd =
                    (DM2_V1_DungeonData *)rt->boot->dungeon_data;
                raw = dm2_v1_dungeon_get_tile_raw(dd, trigger->target_level,
                                                  trigger->target_x,
                                                  trigger->target_y);
                if (raw >= 0) {
                    state = dm2_runtime_door_state((uint16_t)raw);
                    next_state = state == 0 ? 4 : 0;
                    dm2_runtime_set_target_door_state(rt,
                                                      trigger->target_level,
                                                      trigger->target_x,
                                                      trigger->target_y,
                                                      next_state);
                }
            }
            break;
        case DM2_TRIGGER_TARGET_TELEPORT_PARTY:
            gs->current_level = trigger->target_level;
            gs->party_x = trigger->target_x;
            gs->party_y = trigger->target_y;
            rt->dungeon_level = trigger->target_level;
            break;
        default:
            break;
    }
}

static void dm2_runtime_apply_plate_target(DM2_V1_RuntimeState *rt,
                                           const DM2_V1_PressurePlate *plate) {
    int raw;
    int state;
    int next_state;

    if (!rt || !plate) return;
    switch (plate->target_kind) {
        case DM2_PLATE_TARGET_DOOR_OPEN:
            dm2_runtime_set_target_door_state(rt, plate->target_level,
                                              plate->target_x,
                                              plate->target_y, 0);
            break;
        case DM2_PLATE_TARGET_DOOR_CLOSE:
            dm2_runtime_set_target_door_state(rt, plate->target_level,
                                              plate->target_x,
                                              plate->target_y, 4);
            break;
        case DM2_PLATE_TARGET_DOOR_TOGGLE:
            if (rt->boot && rt->boot->dungeon_data) {
                DM2_V1_DungeonData *dd =
                    (DM2_V1_DungeonData *)rt->boot->dungeon_data;
                raw = dm2_v1_dungeon_get_tile_raw(dd, plate->target_level,
                                                  plate->target_x,
                                                  plate->target_y);
                if (raw >= 0) {
                    state = dm2_runtime_door_state((uint16_t)raw);
                    next_state = state == 0 ? 4 : 0;
                    dm2_runtime_set_target_door_state(rt,
                                                      plate->target_level,
                                                      plate->target_x,
                                                      plate->target_y,
                                                      next_state);
                }
            }
            break;
        case DM2_PLATE_TARGET_PIT_TOGGLE:
            dm2_runtime_set_target_door_state(rt, plate->target_level,
                                              plate->target_x,
                                              plate->target_y, 0);
            break;
        default:
            break;
    }
}

/* ── Runtime init ──────────────────────────────────────────────────── */

void dm2_v1_runtime_init(DM2_V1_BootProfile *boot_profile) {
    if (!boot_profile) return;
    memset(&g_dm2_runtime, 0, sizeof(g_dm2_runtime));
    g_dm2_runtime.boot = boot_profile;
    g_dm2_runtime.outdoor = 0;
    g_dm2_runtime.tick_count = 0;
    g_dm2_runtime.move_cooldown_ticks = 0;
    dm2_v1_weather_init(&g_dm2_runtime.weather);
    g_dm2_runtime.time_of_day_minutes = 720;  /* noon */
    g_dm2_runtime.dungeon_level = 0;
    g_dm2_runtime.view_dir = 0;  /* North */
    g_dm2_runtime.last_npc_level = -1;
    g_dm2_runtime.last_npc_x = -1;
    g_dm2_runtime.last_npc_y = -1;
    g_dm2_runtime.last_npc_id = DM2_NPC_MERCHANT_FRIENDLY;
    g_dm2_runtime.last_npc_dialog_line = -1;
    g_dm2_runtime.move_callback  = NULL;
    g_dm2_runtime.turn_callback  = NULL;
    g_dm2_runtime.stairs_callback = NULL;
    g_dm2_runtime.viewport_asset_fetch = NULL;
    g_dm2_runtime.viewport_asset_user = NULL;
}

int dm2_v1_runtime_apply_session(const DM2_V1_SessionState *session) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    DM2_V1_GameState *gs;

    if (!session || !rt->boot || !rt->boot->dm2_state) {
        return -1;
    }
    if (!dm2_v1_session_validate(session)) {
        return -1;
    }
    gs = (DM2_V1_GameState *)rt->boot->dm2_state;

    /* skproject SKWINSPX/src/v4/skgame.cpp SELECT_LOAD_GAME and
     * skfileop.cpp READ_SAVEGAMES_FILENAMES route startup resume through a
     * chosen SKSAVE digit after validating the 0xBEEF/0xDEAD slot header.
     * Firestaff's bounded session importer applies the startup-owned fields
     * already modeled by DM2_V1_GameState/RuntimeState; broader dungeon DB
     * pools stay owned by the later full SKSave importer. */
    gs->party_x = (int)session->party_x;
    gs->party_y = (int)session->party_y;
    gs->party_dir = (int)(session->party_dir & 3u);
    gs->current_level = (int)session->party_level;
    gs->outdoor = session->outdoor_mode ? 1 : 0;
    gs->gold = (int)session->gold;
    gs->reputation = (int)session->reputation;
    gs->time_of_day = (int)session->time_of_day_minutes;

    rt->tick_count = (int)session->game_tick;
    rt->outdoor = gs->outdoor;
    rt->time_of_day_minutes = gs->time_of_day;
    rt->dungeon_level = gs->current_level;
    rt->view_dir = gs->party_dir;
    dm2_v1_weather_set(&rt->weather, session->rain_intensity > 0
                                      ? DM2_WEATHER_RAIN
                                      : DM2_WEATHER_CLEAR);
    rt->weather.weather_intensity = (int)session->rain_intensity;
    return 0;
}

/* ── V1 Game Tick ──────────────────────────────────────────────────── */

/* Module-static projectile drain cache (refreshed each tick).
 * M11 game view can read this to draw fireballs/lightning/arrows. */
static DM2_V1_DrainedProjectile g_dm2_projectile_drain[DM2_DRAIN_MAX_PROJECTILES];
static int g_dm2_projectile_drain_count = 0;

/*
 * dm2_v1_runtime_tick — advance DM2 game state by one V1 tick.
 *
 * Called at 18.2 Hz (every ~55ms) from the Firestaff game loop.
 * Advances: time-of-day, movement cooldown, weather, timers,
 * and refreshes the projectile drain cache for M11 viewport rendering.
 *
 * Movement is gated by move_cooldown_ticks — each successful move
 * consumes 1 tick; failing a move (wall) may incur penalty.
 *
 * Source: SKULL.ASM T048 — input dispatch / tick update
 *         SKULL.ASM T560 — dungeon tick
 *         SKULL.ASM T600 — outdoor tick
 *         skproject/SKULLWIN/c_render.cpp — projectile draw dispatch
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
        dm2_v1_weather_next_state(&rt->weather);
    }

    /* Phase 5+ extension: step then drain DM2 projectile list into
     * M11-ready cache.  The step path applies the STEP_MISSILE
     * energy-decay + despawn boundary (skproject/SKULLWIN/c_tim_proc.cpp
     * m_7CE0/m_7D2A), so the drain reflects only post-step survivors.
     * Without this step the cache would grow without bound and the
     * M11 viewport would draw stale projectiles forever.
     * Source: skproject/SKULLWIN/c_tim_proc.cpp:442-563   (DM2_STEP_MISSILE)
     *         skproject/SKULLWIN/c_render.cpp              (projectile draw)
     *         ReDMCSB DUNGEON.C:2362-2387                  (F0209 visible)
     */
    g_dm2_projectile_drain_count = dm2_v1_projectile_step_and_drain(
        g_dm2_projectile_drain, DM2_DRAIN_MAX_PROJECTILES, NULL);
}

/*
 * dm2_v1_runtime_get_tick_count — narrow probe accessor for the V1 tick
 * boundary.  This exposes only the deterministic tick counter advanced by
 * dm2_v1_runtime_tick(); it does not claim broader DM2 runtime parity.
 *
 * Source: SKULL.ASM T048/T560 tick/update boundary; ReDMCSB GAMELOOP.C
 * lines 55-70 show the V1 loop advancing timeline work once per loop.
 */
int dm2_v1_runtime_get_tick_count(void) {
    return g_dm2_runtime.tick_count;
}

/*
 * dm2_v1_runtime_get_projectile_drain — read-only access to the
 * per-tick projectile drain cache.  M11 game view calls this each
 * render frame to draw DM2 projectiles in the V1 viewport.
 *
 * Returns the count (0..DM2_DRAIN_MAX_PROJECTILES).
 * *out_list is set to the module-static array (do not free).
 */
int dm2_v1_runtime_get_projectile_drain(DM2_V1_DrainedProjectile **out_list) {
    if (out_list) *out_list = g_dm2_projectile_drain;
    return g_dm2_projectile_drain_count;
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
    DM2_V1_ViewportState viewport;

    if (!framebuffer || fb_stride <= 0 ||
        view_w < DM2_VP_WIDTH || view_h < DM2_VP_HEIGHT) {
        return -1;
    }

    dm2_v1_viewport_init(&viewport, framebuffer, fb_stride);
    dm2_v1_viewport_set_party(&viewport, party_dir, party_x, party_y);
    dm2_v1_viewport_set_level(&viewport, rt->dungeon_level);
    dm2_v1_viewport_set_outdoor(&viewport, rt->outdoor);
    dm2_v1_viewport_set_weather(&viewport,
                                rt->outdoor ? 1 : 0,
                                rt->weather.weather_intensity);
    dm2_v1_viewport_set_time(
        &viewport,
        (float)(rt->time_of_day_minutes % 1440) / 1440.0f);
    dm2_runtime_populate_front_square(rt, &viewport, party_dir, party_x, party_y);
    dm2_v1_viewport_set_asset_provider(&viewport,
                                       rt->viewport_asset_fetch,
                                       rt->viewport_asset_user);
    viewport.tick_count = rt->tick_count;
    dm2_v1_viewport_render(&viewport);
    g_dm2_last_asset_floor_ceiling_count =
        viewport.asset_floor_ceiling_drawn_count;
    g_dm2_last_fallback_floor_ceiling_count =
        viewport.fallback_floor_ceiling_drawn_count;
    g_dm2_last_asset_wall_count = viewport.asset_wall_drawn_count;
    g_dm2_last_fallback_wall_count = viewport.fallback_wall_drawn_count;
    g_dm2_last_asset_door_panel_count =
        viewport.asset_door_panel_drawn_count;
    g_dm2_last_asset_door_frame_count =
        viewport.asset_door_frame_drawn_count;
    g_dm2_last_fallback_door_count = viewport.fallback_door_drawn_count;

    return 0;
}

void dm2_v1_runtime_set_viewport_asset_provider(
    DM2_V1_ViewportAssetFetch fetch,
    void *user) {
    g_dm2_runtime.viewport_asset_fetch = fetch;
    g_dm2_runtime.viewport_asset_user = user;
}

int dm2_v1_runtime_last_asset_floor_ceiling_count(void) {
    return g_dm2_last_asset_floor_ceiling_count;
}

int dm2_v1_runtime_last_fallback_floor_ceiling_count(void) {
    return g_dm2_last_fallback_floor_ceiling_count;
}

int dm2_v1_runtime_last_asset_wall_count(void) {
    return g_dm2_last_asset_wall_count;
}

int dm2_v1_runtime_last_fallback_wall_count(void) {
    return g_dm2_last_fallback_wall_count;
}

int dm2_v1_runtime_last_asset_door_panel_count(void) {
    return g_dm2_last_asset_door_panel_count;
}

int dm2_v1_runtime_last_asset_door_frame_count(void) {
    return g_dm2_last_asset_door_frame_count;
}

int dm2_v1_runtime_last_fallback_door_count(void) {
    return g_dm2_last_fallback_door_count;
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

    /* Save pre-move position for smooth animation trigger */
    int old_x = gs->party_x;
    int old_y = gs->party_y;
    int old_dir = gs->party_dir;

    /* Detect turn-only (facing change, no movement) */
    int is_turn_only = (dir != old_dir);
    (void)is_turn_only;  /* turn-only detection reserved for future smooth-move path */

    nx = gs->party_x + dx[dir & 3];
    ny = gs->party_y + dy[dir & 3];

    /* Check dungeon collision if in dungeon mode.
     * Tile type is in lower 5 bits (0x1F) of raw tile.
     * For door tiles (type 4), door state is in lower 3 bits (0x07):
     *   state 0 = open (passable), state 4 = closed (impassable).
     * Other non-walkable: type 0 (wall), type 5 (pit), 11 (lava), 13 (inaccessible).
     * Source: SKULL.ASM T520 — movement collision and door state check.
     *         dm2_special_squares.md — door tile type and state encoding. */
    if (!rt->outdoor && rt->boot->dungeon_data) {
        DM2_V1_DungeonData *dd = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
        int raw = dm2_v1_dungeon_get_tile_raw(dd, rt->dungeon_level, nx, ny);
        if (raw < 0) {
            blocked = 1;
        } else {
            int tile_type = raw & 0x001F;
            /* Impassable tile types: wall (0), pit (5), lava (11), inaccessible (13) */
            if (tile_type == 0 || tile_type == 5 || tile_type == 11 || tile_type == 13) {
                blocked = 1;
            } else if (tile_type == 4) {
                /* Door tile: door state in lower 3 bits.
                 * DM2_DOOR_STATE_OPEN=0 (passable), DM2_DOOR_STATE_CLOSED=4 (impassable).
                 * Source: dm2_v1_object_model.h DM2_DoorState enum.
                 *         SKULL.ASM T520 movement tile access. */
                int door_state = raw & 0x0007;
                if (door_state != 0) {  /* not open */
                    blocked = 1;
                }
            }
            /* All other tile types (1=floor, 3=floor_ornate, 4=door when open,
             * 8=teleporter, 10=water, etc.) are passable. */
        }
    }

    if (!blocked) {
        /* Fire smooth movement callback before updating state.
         * This gives the V2 layer the from/to positions for interpolation.
         * Source: Phase 5 runtime binding */
        if (rt->move_callback) {
            rt->move_callback(old_x, old_y, nx, ny);
        }
        gs->party_x = nx;
        gs->party_y = ny;
        for (int i = 1; i <= dm2_v1_trigger_get_builtin_count(); ++i) {
            const DM2_V1_Trigger *trigger =
                dm2_v1_trigger_get_builtin(i);
            if (trigger &&
                trigger->kind == DM2_TRIGGER_KIND_SQUARE_ENTERED &&
                trigger->arg_map_x == nx &&
                trigger->arg_map_y == ny &&
                trigger->arg_map_level == rt->dungeon_level &&
                dm2_v1_trigger_fire(trigger->trigger_id) ==
                    (int)DM2_TRIGGER_RESULT_OK) {
                dm2_runtime_apply_trigger_target(rt, trigger);
            }
        }
        dm2_v1_plate_set_party_position(nx, ny, rt->dungeon_level);
        for (int i = 1; i <= dm2_v1_plate_get_builtin_count(); ++i) {
            const DM2_V1_PressurePlate *plate =
                dm2_v1_plate_get_builtin(i);
            if (plate && plate->map_x == nx && plate->map_y == ny &&
                plate->map_level == rt->dungeon_level &&
                dm2_v1_plate_check(i, rt->tick_count) ==
                    (int)DM2_PLATE_RESULT_OK) {
                dm2_runtime_apply_plate_target(rt, plate);
            }
        }
    }

    /* Fire smooth turn callback when facing changes.
     * Turn triggers even on blocked moves (party still turns).
     * Source: Phase 5 runtime binding */
    if (dir != old_dir && rt->turn_callback) {
        rt->turn_callback(old_dir, dir);
    }

    gs->party_dir = dir;
    rt->view_dir = dir;

    /* Set movement cooldown: dungeon=1 tick, outdoor=0.5 tick */
    rt->move_cooldown_ticks = rt->outdoor ? 0 : 1;

    return blocked ? -1 : 0;
}

/*
 * dm2_v1_runtime_turn — rotate the party in place without attempting a
 * movement step. M11 keyboard input maps Q/E/Home/End to this boundary so
 * DM2 does not treat turning as a sidestep through dm2_v1_runtime_move().
 *
 * Source: SKULL.ASM T048 dispatches separate turn commands before the
 * T520 party-position update. The existing V2 turn callback is kept as the
 * single animation hand-off, matching the callback used by move().
 */
int dm2_v1_runtime_turn(int delta) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    DM2_V1_GameState *gs;
    int old_dir;
    int new_dir;

    if (!rt->boot || !rt->boot->dm2_state) return -1;
    gs = (DM2_V1_GameState *)rt->boot->dm2_state;
    old_dir = gs->party_dir & 3;
    new_dir = (old_dir + (delta < 0 ? 3 : 1)) & 3;

    if (new_dir != old_dir && rt->turn_callback) {
        rt->turn_callback(old_dir, new_dir);
    }
    gs->party_dir = new_dir;
    rt->view_dir = new_dir;
    return 0;
}

/* ── Outdoor/Dungeon mode ─────────────────────────────────────────── */

/*
 * dm2_v1_runtime_set_outdoor — switch between outdoor and dungeon view.
 *
 * Source: SKULL.ASM T600 — outdoor mode entry
 *         SKULL.ASM T560 — dungeon mode entry
 */
void dm2_v1_runtime_set_outdoor(int is_outdoor) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    g_dm2_runtime.outdoor = is_outdoor ? 1 : 0;
    if (rt->boot && rt->boot->dm2_state) {
        DM2_V1_GameState *gs = (DM2_V1_GameState *)rt->boot->dm2_state;
        gs->outdoor = g_dm2_runtime.outdoor;
    }
}

void dm2_v1_runtime_set_position(int level, int x, int y, int dir) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    DM2_V1_GameState *gs;

    if (!rt->boot || !rt->boot->dm2_state) return;
    gs = (DM2_V1_GameState *)rt->boot->dm2_state;
    gs->current_level = level;
    gs->party_x = x;
    gs->party_y = y;
    gs->party_dir = dir & 3;
    rt->dungeon_level = level;
    rt->view_dir = gs->party_dir;
}

/* ── Party position accessors ─────────────────────────────────────── */

/* dm2_v1_runtime_get_party_x / _y / _dir — read V1-snapped party state.
 * Returns the instant (non-interpolated) V1 game state.
 *
 * Source: SKULL.ASM T520 — party position fields
 */
int dm2_v1_runtime_get_party_x(void) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    if (!rt->boot || !rt->boot->dm2_state) return 0;
    DM2_V1_GameState *gs = (DM2_V1_GameState *)rt->boot->dm2_state;
    return gs->party_x;
}

int dm2_v1_runtime_get_party_y(void) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    if (!rt->boot || !rt->boot->dm2_state) return 0;
    DM2_V1_GameState *gs = (DM2_V1_GameState *)rt->boot->dm2_state;
    return gs->party_y;
}

int dm2_v1_runtime_get_party_dir(void) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    if (!rt->boot || !rt->boot->dm2_state) return 0;
    DM2_V1_GameState *gs = (DM2_V1_GameState *)rt->boot->dm2_state;
    return gs->party_dir;
}

int dm2_v1_runtime_get_weather(void) {
    return g_dm2_runtime.weather.weather;
}

int dm2_v1_runtime_get_weather_intensity(void) {
    return g_dm2_runtime.weather.weather_intensity;
}

uint32_t dm2_v1_runtime_get_weather_seed(void) {
    return g_dm2_runtime.weather.weather_seed;
}

void dm2_v1_runtime_set_weather_seed(uint32_t seed) {
    dm2_v1_weather_set_seed(&g_dm2_runtime.weather, seed);
}

/* dm2_v1_runtime_has_dungeon_data — returns 1 if dungeon state is available.
 * Used by dm2_v2_runtime_render_frame to detect headless (no dungeon) mode.
 * Source: Phase 5 runtime binding */
int dm2_v1_runtime_has_dungeon_data(void) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    return (rt->boot && rt->boot->dm2_state) ? 1 : 0;
}

/* ── V2 Smooth Movement Callbacks ───────────────────────────────── */

void dm2_v1_runtime_set_move_callback(DM2_V2_MoveCallback cb) {
    g_dm2_runtime.move_callback = cb;
}

void dm2_v1_runtime_set_turn_callback(DM2_V2_TurnCallback cb) {
    g_dm2_runtime.turn_callback = cb;
}

void dm2_v1_runtime_set_stairs_callback(DM2_V2_StairsCallback cb) {
    g_dm2_runtime.stairs_callback = cb;
}

/* ── Interaction / square helpers ─────────────────────────────────── */

int dm2_v1_runtime_get_square_type(int level, int x, int y) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    DM2_V1_DungeonData *dd;

    if (!rt->boot || !rt->boot->dungeon_data) return -1;
    dd = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    return dm2_v1_dungeon_get_square_type(dd, level, x, y);
}

int dm2_v1_runtime_is_passable(int level, int x, int y) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    DM2_V1_DungeonData *dd;
    int raw;
    int tile_type;

    if (!rt->boot || !rt->boot->dungeon_data) return 0;
    dd = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    raw = dm2_v1_dungeon_get_tile_raw(dd, level, x, y);
    if (raw < 0) return 0;

    tile_type = raw & 0x001F;
    if (tile_type == 0 || tile_type == 5 ||
        tile_type == 11 || tile_type == 13) {
        return 0;
    }
    if (tile_type == 4 && (raw & 0x0007) != 0) {
        return 0;
    }
    return 1;
}

int dm2_v1_runtime_door_action(int level,
                               int x,
                               int y,
                               int facing_dir,
                               int action) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    DM2_V1_DungeonData *dd;
    int raw;
    int tile_type;
    int state;
    int next_state;

    (void)facing_dir;
    if (!rt->boot || !rt->boot->dungeon_data) return -1;
    dd = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    raw = dm2_v1_dungeon_get_tile_raw(dd, level, x, y);
    if (raw < 0) return -1;
    tile_type = raw & 0x001F;
    if (tile_type != 4) return -1;

    state = dm2_runtime_door_state((uint16_t)raw);
    next_state = dm2_runtime_door_step(state, action);
    if (next_state == state) return 0;

    return dm2_v1_dungeon_set_tile_raw(
        dd, level, x, y,
        dm2_runtime_door_set_state((uint16_t)raw, next_state));
}

int dm2_v1_runtime_get_door_state(int level, int x, int y) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    DM2_V1_DungeonData *dd;
    int raw;

    if (!rt->boot || !rt->boot->dungeon_data) return -1;
    dd = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    raw = dm2_v1_dungeon_get_tile_raw(dd, level, x, y);
    if (raw < 0 || (raw & 0x001F) != 4) return -1;
    return dm2_runtime_door_state((uint16_t)raw);
}

int dm2_v1_runtime_enter_shop(int level, int x, int y) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    DM2_V1_GameState *gs;
    int shop_id = DM2_SHOP_ID_NONE;

    if (!rt->boot || !rt->boot->dm2_state) return -1;
    gs = (DM2_V1_GameState *)rt->boot->dm2_state;
    if (!rt->outdoor && !gs->outdoor) return -1;
    for (int i = 1; i <= DM2_NUM_BUILTIN_SHOPS; i++) {
        const DM2_V1_ShopDescriptor *shop = dm2_v1_shop_get_builtin(i);
        if (shop && shop->map_level == level &&
            shop->map_x == x && shop->map_y == y) {
            shop_id = shop->shop_id;
            break;
        }
    }
    if (shop_id == DM2_SHOP_ID_NONE) return -1;
    dm2_v1_shop_set_party_gold((uint32_t)(gs->gold < 0 ? 0 : gs->gold));
    if (!dm2_v1_shop_enter(shop_id)) return -1;
    gs->time_of_day = rt->time_of_day_minutes;
    return 0;
}

int dm2_v1_runtime_npc_interact(int level, int x, int y) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    DM2_V1_GameState *gs;
    int same_npc_square;

    if (!rt->boot || !rt->boot->dm2_state) return -1;
    if (dm2_v1_runtime_get_square_type(level, x, y) < 0) return -1;
    gs = (DM2_V1_GameState *)rt->boot->dm2_state;
    if (!rt->outdoor && !gs->outdoor) return -1;

    same_npc_square = rt->last_npc_level == level &&
                      rt->last_npc_x == x &&
                      rt->last_npc_y == y;
    rt->last_npc_id = DM2_NPC_MERCHANT_FRIENDLY;
    rt->last_npc_level = level;
    rt->last_npc_x = x;
    rt->last_npc_y = y;
    if (same_npc_square && rt->last_npc_dialog_line >= 0) {
        rt->last_npc_dialog_line =
            (rt->last_npc_dialog_line + 1) % DM2_NPC_DIALOG_LINES;
    } else {
        rt->last_npc_dialog_line = 0;
    }
    if (gs->reputation < 9999) {
        gs->reputation++;
    }
    return 0;
}

int dm2_v1_runtime_get_last_npc_id(void) {
    return g_dm2_runtime.last_npc_id;
}

int dm2_v1_runtime_get_last_npc_dialog_line(void) {
    return g_dm2_runtime.last_npc_dialog_line;
}

int dm2_v1_runtime_invoke_actuator(int level, int x, int y,
                                   DM2_ActuatorType type, uint16_t flag) {
    (void)flag;
    if (dm2_v1_runtime_get_square_type(level, x, y) < 0) return -1;
    if (type == DM2_ACTUATOR_SHOP_PANEL) {
        return dm2_v1_runtime_enter_shop(level, x, y);
    }
    if (type == DM2_ACTUATOR_PUSH_BUTTON_WALL_SWITCH ||
        type == DM2_ACTUATOR_WALL_SWITCH ||
        type == DM2_ACTUATOR_2_STATE_WALL_SWITCH ||
        type == DM2_ACTUATOR_DM1_WALL_SWITCH) {
        return 0;
    }
    return 0;
}

/* ── Source evidence ──────────────────────────────────────────────── */

const char *dm2_v1_runtime_source_evidence(void) {
    return
        "DM2 V1 Runtime Stub — Phase 1\n"
        "Source: SKULL.ASM T048  — input dispatch / tick update\n"
        "Source: SKULL.ASM T520  — movement speed and party placement\n"
        "Source: SKULL.ASM T560  — dungeon tick and viewport rendering\n"
        "Source: SKULL.ASM T600  — outdoor tick and weather rendering\n"
        "Weather transition seed: ReDMCSB BASE.C F0027/F0029 (LCG 0xBB40E62D, +11)\n"
        "Reference: CSB path in firestaff_game_loop.c (FS_GAME_CSB → csb_v1_viewport_render_frame)\n";
}
