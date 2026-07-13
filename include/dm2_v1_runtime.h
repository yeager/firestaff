#ifndef FIRESTAFF_DM2_V1_RUNTIME_H
#define FIRESTAFF_DM2_V1_RUNTIME_H
/*
 * dm2_v1_runtime.h — DM2 V1 Runtime Mechanics Parity API
 *
 * Phase 4: Movement, interactions, shops/NPCs, doors, pressure plates, triggers.
 *
 * Source-locks:
 *   SKULL.ASM T520  — party movement tick, collision, step behavior
 *   SKULL.ASM T560  — dungeon tick, door state machine, actuator processing
 *   SKULL.ASM T048  — input dispatch
 *   SKULL.ASM T800  — outdoor/shop/NPC entry points
 *   docs/dm2_triggers.md      — actuator taxonomy (40+ types)
 *   docs/dm2_actuators.md      — door step-behavior, actuator data
 *   docs/dm2_special_squares.md — teleporter, ladder, door behavior
 *   docs/dm2_sensors.md        — floor/wall sensor types
 *
 * DM2 key differences from DM1:
 *   - Actuator system: generic sensor/actuator separation (DM1: hardwired)
 *   - 40+ actuator types vs ~5 DM1 trigger types
 *   - Cross-map triggers via ACTUATOR_TYPE_CROSS_MAP (0x16)
 *   - Shop interface via ACTUATOR_TYPE_SHOP_PANEL (0x3F)
 *   - Door step-behavior (C00..C06_ACTEFFECT_STEP_*) independent of actuator
 *   - Per-square type in lower 5 bits (same as DM1)
 *   - Companion AI (4 allies, modes: follow/guard/aggressive)
 */

#include <stdint.h>
#include "dm2_v1_boot.h"
#include "dm2_v1_new_game.h"
#include "dm2_v1_startup_menu.h"
#include "dm2_v1_viewport_renderer.h"

/* Runtime-visible proof that the M11-owned frame consumed DM2 GDAT pixels.
 * This is deliberately aggregate: it proves ownership and real consumption
 * without exposing a renderer buffer beyond its frame lifetime. */
typedef struct {
    unsigned int generation;
    int runtime_frame_owned;
    int is_outdoor;
    /* Exact live weather state forwarded into the V1 viewport.  This is
     * state/command provenance only; the viewport remains fail-closed until
     * skproject's source-backed weather material route is decoded. */
    int runtime_weather;
    int runtime_weather_intensity;
    int gdat_provider_bound;
    int startup_title_gdat_blits;
    int startup_menu_gdat_blits;
    int hud_gdat_blits;
    int hud_core_gdat_blits;
    int door_gdat_blits;
    int creature_gdat_blits;
    int floor_ceiling_gdat_blits;
    int outdoor_sky_gdat_blits;
    int outdoor_ground_gdat_blits;
    int wall_gdat_blits;
    int item_gdat_blits;
    int projectile_gdat_blits;
    int total_runtime_gdat_blits;
    int total_runtime_fallback_draws;
    int blocked_material_draws;
    uint32_t blocked_material_mask;
    int full_gdat_frame_valid;
    int outdoor_gdat_frame_valid;
    int real_gdat_evidence_valid;
    int gdat_scene_control_ready;
    int gdat_scene_control_consumed;
    uint32_t gdat_scene_control_hash;
    uint32_t gdat_scene_control_present_mask;
    uint32_t gdat_scene_colorkey;
    uint32_t gdat_scene_flags;
    int gdat_scene_material_index;
    int gdat_scene_material_consumed;
    uint32_t gdat_scene_ambient_light;
    uint32_t gdat_scene_highest_light_level;
    uint32_t gdat_scene_void_random_fall;
    uint32_t gdat_scene_animated_floor;
    uint32_t gdat_scene_rain;
    uint32_t gdat_misty_map;
    uint32_t gdat_thunder_position;
    uint32_t gdat_ambient_darkness;
    int gdat_weather_receipt_ready;
    uint32_t gdat_weather_receipt_hash;
    uint32_t gdat_weather_material_mask;
    int gdat_scene_light_consumed;
    int gdat_scene_weather_consumed;
    int gdat_sprite_palette_consumed;
    int gdat_local_palette_consumed;
    int gdat_interface_palette_ready;
    int gdat_interface_palette_consumed;
    int gdat_interface_font_host_ready;
    int gdat_interface_font_consumed;
    uint32_t gdat_interface_font_hash;
    /* The save/load panel is source-bound for this runtime, but not counted
     * as drawn until the M11 dialogue owner opens it and expands RECT_453. */
    int gdat_save_dialogue_material_bound;
    int gdat_save_dialogue_host_command_ready;
    int gdat_save_dialogue_open_panel_ready;
    uint32_t gdat_save_dialogue_material_hash;
    uint32_t gdat_save_dialogue_host_command_hash;
    uint32_t gdat_save_dialogue_open_panel_hash;
    uint32_t gdat_save_dialogue_rect_index;
    uint32_t gdat_save_dialogue_open_panel_rect_index;
    uint32_t gdat_save_dialogue_open_panel_save_list_rect_index;
    int gdat_save_dialogue_x;
    int gdat_save_dialogue_y;
    int gdat_save_dialogue_w;
    int gdat_save_dialogue_h;
    int gdat_interface_hud_layout_ready;
    uint32_t gdat_interface_hud_layout_hash;
    int gdat_interface_rect14_host_ready;
    int gdat_interface_rect14_consumed;
    uint32_t gdat_interface_rect14_table_hash;
    uint32_t gdat_interface_rect14_placement_hash;
    uint32_t gdat_interface_rect14_placement_count;
    int gdat_material_palette_floor_ceiling_consumed;
    int gdat_material_palette_wall_consumed;
    int gdat_material_palette_door_frame_consumed;
    uint32_t gdat_interface_palette_hash;
    uint8_t gdat_interface_palette16[16];
    int viewport_raw_gdat_asset_count;
    int viewport_decoded_gdat_asset_count;
    uint32_t viewport_raw_gdat_hash;
    uint32_t viewport_raw_gdat_byte_count;
    uint32_t viewport_decoded_gdat_hash;
    uint32_t viewport_decoded_gdat_pixel_count;
    int valid;
} DM2_V1_RuntimeFrameOwnershipReceipt;

/* Atomic identity for the exact source-required viewport frame M11 is about
 * to present. The values come from the live GDAT scene and interface palette
 * receipts, never from a fallback frame. */
typedef struct {
    int valid;
    int m11_consume_frame;
    int source_materials_required;
    uint32_t map_load_token;
    uint32_t scene_control_hash;
    uint32_t palette_hash;
} DM2_V1_ViewportM11FrameReceipt;

typedef struct {
    int ready;
    int map_graphics_style;
    int scene_material_index;
    uint32_t hash;
    uint32_t present_mask;
    uint32_t query_count;
    uint32_t scene_colorkey;
    uint32_t scene_flags;
    uint32_t ambient_light;
    uint32_t highest_light_level;
    uint32_t void_random_fall;
    uint32_t animated_floor;
    uint32_t scene_rain;
    uint32_t misty_map;
    uint32_t thunder_position;
    uint32_t ambient_darkness;
    int interface_palette_ready;
    uint32_t interface_palette_hash;
    uint8_t interface_palette16[16];
} DM2_V1_RuntimeGraphicsSetSceneReceipt;
#include "dm2_v1_weather.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Core movement ──────────────────────────────────────────────── */

void dm2_v1_runtime_init(DM2_V1_BootProfile *boot_profile);
int  dm2_v1_runtime_bind_boot_profile(DM2_V1_BootProfile *boot_profile);
/* Returns the map-0 bounded receipt when a canonical G1 partial world was
 * consumed. Only source-defined direct DB1 teleporter fields are available;
 * no generic object or link traversal is exposed through this API. */
int dm2_v1_runtime_g1_first_map_receipt(
    DM2_V1_G1FirstMapRuntimeReceipt *out_receipt);
/* Returns the DB1 teleporter receipt for the current map-0 party pose. It
 * applies a transition only when the source and complete-world gates pass;
 * otherwise the receipt is a strict no-transition result. GenericRecord::w0
 * and blocked roots remain unavailable. */
int dm2_v1_runtime_g1_map0_teleporter_transition_receipt(
    DM2_V1_G1TeleporterTransitionReceipt *out_receipt);
/* Returns the map-5 direct DB2 Text::w2 field receipt consumed at boot.
 * It exposes no text bytes, GenericRecord::w0 links, or non-DB2 records. */
int dm2_v1_runtime_g1_map5_text_receipt(
    DM2_V1_G1Map5TextRuntimeReceipt *out_receipt);
/* Visible DB2 TextMode==0 strings decoded from the original G1 text table.
 * Mode-one GDAT messages and unknown phrase-bank escapes are not fabricated. */
int dm2_v1_runtime_g1_map5_text_message_receipt(
    DM2_V1_G1TextMessageRuntimeReceipt *out_receipt);
int dm2_v1_runtime_g1_map5_text_wall_gfx_receipt(
    DM2_V1_G1TextWallGfxRuntimeReceipt *out_receipt);
int dm2_v1_runtime_g1_actuator_wall_gfx_receipt(
    DM2_V1_G1ActuatorWallGfxRuntimeReceipt *out_receipt);
int dm2_v1_runtime_g1_creature_map_chip_receipt(
    DM2_V1_G1CreatureMapChipRuntimeReceipt *out_receipt);
int  dm2_v1_runtime_bind_boot_profile_with_receipt(
    DM2_V1_BootProfile *boot_profile,
    DM2_V1_StartupHostReceipt *out_receipt);
int  dm2_v1_runtime_apply_session(const DM2_V1_SessionState *session);
void dm2_v1_runtime_tick(void);
int  dm2_v1_runtime_get_tick_count(void);
int  dm2_v1_runtime_can_move(void);
int  dm2_v1_runtime_move(int direction);        /* 0=N 1=E 2=S 3=W, returns 0=ok -1=blocked */
int  dm2_v1_runtime_turn(int delta);            /* -1=left, +1=right, returns 0=ok -1=unbooted */
void dm2_v1_runtime_set_outdoor(int is_outdoor);/* 1=outdoor 0=dungeon */
void dm2_v1_runtime_set_position(int level, int x, int y, int dir);

/* ── Projectile drain (Phase 5) ──────────────────────────────────────
 * dm2_v1_runtime_get_projectile_drain — returns the per-tick drain cache
 * populated by dm2_v1_runtime_tick().  M11 game view calls this each
 * render frame to draw DM2 projectiles (fireballs / lightning / arrows)
 * in the V1 viewport.
 *
 * Source: skproject/SKULLWIN/c_render.cpp — projectile draw dispatch */
#include "dm2_v1_projectile_pc34_compat.h"  /* DM2_V1_DrainedProjectile */
int  dm2_v1_runtime_get_projectile_drain(DM2_V1_DrainedProjectile **out_list);

/* ── Viewport rendering ─────────────────────────────────────────────── */

/* dm2_v1_runtime_render_frame — render DM2 V1 viewport frame.
 * This is the base V1 discrete renderer (no smooth interpolation).
 * Used by dm2_v2_runtime_render_frame as the base render target.
 *
 * Source: SKULL.ASM T560 — viewport frame rendering
 *         SKULL.ASM T600 — outdoor rendering
 */
int dm2_v1_runtime_render_frame(int party_dir, int party_x, int party_y,
                                  uint8_t *framebuffer, int fb_stride,
                                  int view_w, int view_h);

void dm2_v1_runtime_note_startup_frame_consumption(
    int title_gdat_blits, int menu_gdat_blits);
int dm2_v1_runtime_last_frame_ownership(
    DM2_V1_RuntimeFrameOwnershipReceipt *out_receipt);
int dm2_v1_runtime_last_m11_frame_receipt(
    DM2_V1_ViewportM11FrameReceipt *out_receipt);
int dm2_v1_runtime_graphicsset_scene_receipt(
    DM2_V1_RuntimeGraphicsSetSceneReceipt *out_receipt);
void dm2_v1_runtime_set_viewport_asset_provider(
    DM2_V1_ViewportAssetFetch fetch,
    void *user);
int dm2_v1_runtime_set_map_wall_gfx_list(const uint8_t *wall_gfx_list,
                                         int wall_gfx_count);
int dm2_v1_runtime_last_asset_floor_ceiling_count(void);
int dm2_v1_runtime_last_fallback_floor_ceiling_count(void);
int dm2_v1_runtime_last_asset_wall_count(void);
int dm2_v1_runtime_last_fallback_wall_count(void);
int dm2_v1_runtime_last_asset_door_panel_count(void);
int dm2_v1_runtime_last_asset_door_overlay_count(void);
int dm2_v1_runtime_last_asset_door_frame_count(void);
int dm2_v1_runtime_last_asset_door_button_count(void);
int dm2_v1_runtime_last_fallback_door_count(void);
typedef struct DM2_V1_RuntimeDoorRenderReceipt {
    int valid;
    int view_square;
    int skproject_cell;
    int door_record_type;
    int door_gfx_index;
    int door_opening_dir;
    int ornament_index;
    int door_button;
    int door_button_state;
    int door_state;
    int door_open_pct;
    int panel_gdat_index;
    int ornate_gdat_index;
    int destroyed_mask_gdat_index;
    int frame_gdat_index;
    int button_gdat_index;
    int button_source_kind; /* 1=default door button, 2=wall-gfx button */
    int button_clickable;
    int button_rectno;
    int wall_button_index;
    int wall_button_field;
    int panel_blit_ready;
    int ornate_blit_ready;
    int destroyed_mask_blit_ready;
    int frame_blit_ready;
    int button_blit_ready;
    int skproject_material_expected_count;
    int skproject_material_ready_count;
    int skproject_material_drawn_count;
    int skproject_material_chain_ready;
    int skproject_material_chain_drawn;
    uint32_t skproject_material_chain_hash;
    int panel_asset_drawn;
    int ornate_asset_drawn;
    int destroyed_mask_asset_drawn;
    int frame_asset_drawn;
    int button_asset_drawn;
    int panel_asset_src_w;
    int panel_asset_src_h;
    int panel_asset_src_stride;
    int ornate_asset_src_w;
    int ornate_asset_src_h;
    int ornate_asset_src_stride;
    int destroyed_mask_asset_src_w;
    int destroyed_mask_asset_src_h;
    int destroyed_mask_asset_src_stride;
    int frame_asset_src_w;
    int frame_asset_src_h;
    int frame_asset_src_stride;
    int button_asset_src_w;
    int button_asset_src_h;
    int button_asset_src_stride;
    DM2_V1_ViewportRect panel_rect;
    DM2_V1_ViewportRect panel_visible_rect;
    DM2_V1_ViewportRect overlay_rect;
    DM2_V1_ViewportRect frame_rect;
    DM2_V1_ViewportRect button_rect;
    DM2_V1_ViewportRect panel_asset_dst_rect;
    DM2_V1_ViewportRect ornate_asset_dst_rect;
    DM2_V1_ViewportRect destroyed_mask_asset_dst_rect;
    DM2_V1_ViewportRect frame_asset_dst_rect;
    DM2_V1_ViewportRect button_asset_dst_rect;
} DM2_V1_RuntimeDoorRenderReceipt;
int dm2_v1_runtime_last_door_render_receipt(
    DM2_V1_RuntimeDoorRenderReceipt *out_receipt);
int dm2_v1_runtime_last_asset_item_count(void);
int dm2_v1_runtime_last_fallback_item_count(void);
int dm2_v1_runtime_last_asset_carried_item_count(void);
int dm2_v1_runtime_last_fallback_carried_item_count(void);
typedef struct DM2_V1_RuntimeItemRenderReceipt {
    int valid;
    int source_kind;       /* 1=floor item, 2=creature possession, 3=carried */
    int item_index;
    int item_category;
    int item_type;
    int frame_index;
    int render_frame;
    int direction;
    int depth;
    int center_x;
    int center_y;
    int gdat_index;
    int draw_order;
    int flip_mirror;
    int asset_blit_ready;
    int fallback_drawn;
    int asset_src_w;
    int asset_src_h;
    int asset_src_stride;
    int asset_frame_count;
    int atlas_frame_x;
    int atlas_frame_y;
    int atlas_frame_w;
    int atlas_frame_h;
    DM2_V1_ViewportRect asset_dst_rect;
    int fallback_radius;
} DM2_V1_RuntimeItemRenderReceipt;
int dm2_v1_runtime_last_item_render_receipt(
    DM2_V1_RuntimeItemRenderReceipt *out_receipt);
typedef struct DM2_V1_RuntimeCreatureRenderReceipt {
    int valid;
    int instance_id;
    int thing_handle;
    int source_kind;       /* 1=live CCM instance, 2=DB4 dungeon record */
    int creature_type;
    int frame_index;
    int direction;
    int hp_pct;
    int ccm_primary_state;
    int ccm_secondary_state;
    int attack_cooldown;
    int frame_source;      /* 0=walk/tick, 1=cooldown, 2=attack state */
    uint32_t animation_tick;
    uint32_t render_revision;
    int map_x;
    int map_y;
    int screen_x;
    int screen_y;
    int depth;
    int gdat_index;
    int draw_order;
    int asset_blit_ready;
    int fallback_drawn;
    int asset_src_w;
    int asset_src_h;
    int asset_src_stride;
    int asset_frame_count;
    int requested_frame_index;
    int party_direction;
    int relative_direction;
    int atlas_frame_index;
    int atlas_frame_x;
    int atlas_frame_y;
    int atlas_frame_w;
    int atlas_frame_h;
    int render_frame;
    DM2_V1_ViewportRect asset_dst_rect;
    DM2_V1_ViewportRect fallback_rect;
} DM2_V1_RuntimeCreatureRenderReceipt;
int dm2_v1_runtime_last_creature_render_receipt(
    DM2_V1_RuntimeCreatureRenderReceipt *out_receipt);
int dm2_v1_runtime_last_asset_creature_count(void);
int dm2_v1_runtime_last_fallback_creature_count(void);
int dm2_v1_runtime_last_asset_creature_possession_item_count(void);
int dm2_v1_runtime_last_fallback_creature_possession_item_count(void);
int dm2_v1_runtime_last_asset_projectile_count(void);
int dm2_v1_runtime_last_fallback_projectile_count(void);
typedef struct DM2_V1_RuntimeProjectileRenderReceipt {
    int valid;
    int projectile_index;
    int projectile_category;
    int projectile_type;
    int frame_index;
    int render_frame;
    int direction;
    int object_direction;
    int frame_class;
    int render_kind;
    int depth;
    int center_x;
    int center_y;
    int gdat_index;
    int draw_order;
    int flip_mirror;
    int cloud_flip_from_seed;
    int asset_blit_ready;
    int fallback_drawn;
    int asset_src_w;
    int asset_src_h;
    int asset_src_stride;
    int asset_frame_count;
    int atlas_frame_x;
    int atlas_frame_y;
    int atlas_frame_w;
    int atlas_frame_h;
    DM2_V1_ViewportRect asset_dst_rect;
    int fallback_dx;
    int fallback_dy;
    int fallback_len;
    uint32_t random_seed_before;
    uint32_t random_seed_after;
} DM2_V1_RuntimeProjectileRenderReceipt;
int dm2_v1_runtime_last_projectile_render_receipt(
    DM2_V1_RuntimeProjectileRenderReceipt *out_receipt);
int dm2_v1_runtime_last_asset_hud_portrait_count(void);
int dm2_v1_runtime_last_fallback_hud_portrait_count(void);

/* ── Party position accessors ─────────────────────────────────────── */

/* dm2_v1_runtime_get_party_x / _y / _dir — read V1-snapped party state.
 * These return the instant V1 game state (no interpolation).
 * For interpolated V2 state, use dm2_v2_runtime_smooth_query().
 *
 * Source: SKULL.ASM T520 — party position fields
 */
int dm2_v1_runtime_get_party_x(void);
int dm2_v1_runtime_get_party_y(void);
int dm2_v1_runtime_get_party_dir(void);
int dm2_v1_runtime_get_weather(void);
int dm2_v1_runtime_get_weather_intensity(void);
uint32_t dm2_v1_runtime_get_leader_hand_object(void);
void dm2_v1_runtime_set_leader_hand_object(uint32_t object);
uint32_t dm2_v1_runtime_get_champion_inventory_object(uint8_t champion,
                                                      uint8_t slot);
int dm2_v1_runtime_set_champion_inventory_object(uint8_t champion,
                                                 uint8_t slot,
                                                 uint32_t object);
int dm2_v1_runtime_export_inventory_to_session(DM2_V1_SessionState *session);
int dm2_v1_runtime_export_session(DM2_V1_SessionState *session);

/* Runtime sidecar persistence supplements the interoperable SKSave session
 * with live DM2 state that is not representable in its startup envelope:
 * CCM instances, animation/revision writeback, mutable dungeon bytes (door
 * and DB runtime), and the GDAT profile binding used to render them. */
size_t dm2_v1_runtime_live_save_size(void);
int dm2_v1_runtime_serialize_live_save(uint8_t *out, size_t out_size);
int dm2_v1_runtime_restore_live_save(const uint8_t *data, size_t data_size);

/* Restore a decoded SKSave payload into the active DM2 runtime. Original raw
 * candidates must carry a dungeon prefix exactly matching the booted dungeon;
 * malformed or incompatible inputs leave the live runtime untouched. */
int dm2_v1_runtime_restore_save_candidate(const uint8_t *data,
                                          size_t data_size);
int dm2_v1_runtime_load_save_slot(const char *save_base, uint8_t slot);
int dm2_v1_runtime_load_last_session(const char *save_base);

typedef enum DM2_V1_QuicksaveResult {
    DM2_V1_QUICKSAVE_OK = 0,
    DM2_V1_QUICKSAVE_PROFILE_MISSING,
    DM2_V1_QUICKSAVE_SAVE_DIR_FAILED,
    DM2_V1_QUICKSAVE_EXPORT_FAILED,
    DM2_V1_QUICKSAVE_WRITE_FAILED
} DM2_V1_QuicksaveResult;

typedef struct DM2_V1_QuicksaveReceipt {
    DM2_V1_QuicksaveResult result;
    const char *status_scope;
    const char *status;
    char save_root[512];
    char save_path[512];
    DM2_V1_SessionState session;
    DM2_V1_RuntimeGraphicsSetSceneReceipt graphicsset_scene;
    int session_valid;
} DM2_V1_QuicksaveReceipt;

int dm2_v1_runtime_quicksave_boot_profile_with_receipt(
    DM2_V1_BootProfile *profile,
    DM2_V1_QuicksaveReceipt *out_receipt);

uint8_t dm2_v1_runtime_get_minion_count(void);
int dm2_v1_runtime_get_minion_assoc(uint8_t index, DM2_MinionAssoc *out_assoc);
uint32_t dm2_v1_runtime_get_weather_seed(void);
void dm2_v1_runtime_set_weather_seed(uint32_t seed);

/* dm2_v1_runtime_has_dungeon_data — returns 1 if dungeon state is available.
 * Used by dm2_v2_runtime_render_frame to detect headless mode.
 * Source: Phase 5 runtime binding */
int dm2_v1_runtime_has_dungeon_data(void);

/* ── V2 Smooth Movement Callbacks ───────────────────────────────── */

/* DM2_V2_MoveCallback — called when party successfully moves.
 * Called from dm2_v1_runtime_move() on a successful move.
 * Parameters: from_x, from_y, to_x, to_y (V1 grid positions).
 * Context: called during V1 tick, before movement cooldown is set.
 *
 * Source: SKULL.ASM T520 — party/movement tick
 */
typedef void (*DM2_V2_MoveCallback)(int from_x, int from_y, int to_x, int to_y);

/* DM2_V2_TurnCallback — called when party turns.
 * Parameters: from_dir, to_dir (0=N 1=E 2=S 3=W).
 *
 * Source: SKULL.ASM T520 — party/movement tick
 */
typedef void (*DM2_V2_TurnCallback)(int from_dir, int to_dir);

/* DM2_V2_StairsCallback — called when party uses stairs.
 * Parameters: from_x, from_y, to_x, to_y (grid positions)
 *             vert_offset (camera vertical displacement).
 *
 * Source: SKULL.ASM T520 — stairs movement
 */
typedef void (*DM2_V2_StairsCallback)(int from_x, int from_y,
                                      int to_x, int to_y,
                                      float vert_offset);

/* dm2_v1_runtime_set_move_callback — register V2 smooth move callback.
 * Only one callback can be registered at a time (replaces previous).
 * Pass NULL to deregister.
 *
 * Source: Phase 5 runtime binding
 */
void dm2_v1_runtime_set_move_callback(DM2_V2_MoveCallback cb);

/* dm2_v1_runtime_set_turn_callback — register V2 smooth turn callback. */
void dm2_v1_runtime_set_turn_callback(DM2_V2_TurnCallback cb);

/* dm2_v1_runtime_set_stairs_callback — register V2 stairs callback. */
void dm2_v1_runtime_set_stairs_callback(DM2_V2_StairsCallback cb);

/* ── Doors ─────────────────────────────────────────────────────────── */

/*
 * Door step-behavior (C00..C06_ACTEFFECT_STEP_*).
 * These are per-door record fields, not actuator types.
 * Source: docs/dm2_actuators.md "Step-behavior modes"
 *         docs/dm2_special_squares.md "DM2 has a second clan door type (0x0A)" */
typedef enum {
    DM2_DOOR_STEP_NONE          = 0,  /* nothing on step */
    DM2_DOOR_STEP_OPEN          = 1,  /* open on step */
    DM2_DOOR_STEP_CLOSE         = 2,  /* close on step away */
    DM2_DOOR_STEP_TOGGLE        = 3,  /* toggle on step */
    DM2_DOOR_STEP_OPEN_AUTOCLOSE = 4, /* open on step, auto-close */
    DM2_DOOR_STEP_FORCE_OPEN    = 5,  /* force open (from actuator) */
    DM2_DOOR_STEP_FORCE_CLOSE   = 6,  /* force close */
} DM2_DoorStepBehavior;

/* Door state machine (DoorBit09, DoorBit10).
 * Source: docs/dm2_actuators.md "Door Actuator Control"
 *         SKULL.ASM T560 door state transitions */
typedef enum {
    DM2_RUNTIME_DOOR_CLOSED  = 0,
    DM2_RUNTIME_DOOR_OPENING = 1,   /* DoorBit09=1, animates open */
    DM2_RUNTIME_DOOR_OPEN    = 2,
    DM2_RUNTIME_DOOR_CLOSING = 3,   /* DoorBit09=0, animates closed */
} DM2_RuntimeDoorMotionState;

/* Attempt to open or close the door at (level, x, y, facing_dir).
 * Returns 0 on success, -1 if no door at that location. */
int dm2_v1_runtime_door_action(int level, int x, int y, int facing_dir, int action);
/*  action: 0=open, 1=close, 2=toggle */

/* Get door state at a position. Returns -1 if no door. */
int dm2_v1_runtime_get_door_state(int level, int x, int y);

/* ── Teleporters ──────────────────────────────────────────────────── */

/* Teleport party to (target_level, tx, ty).
 * Source: docs/dm2_special_squares.md "SDFSM_CMD_X_TELEPORTER (4), SDFSM_CMD_X_ANCHOR (5)" */
int dm2_v1_runtime_teleport(int target_level, int tx, int ty);

/* Teleport scope — controls what can use the teleporter.
 * Source: DME.h:384 "Teleporter scope enum" */
typedef enum {
    DM2_TELEPORT_SCOPE_PARTY    = 0,  /* party only */
    DM2_TELEPORT_SCOPE_CREATURE = 1,  /* creatures only */
    DM2_TELEPORT_SCOPE_ALL      = 2,  /* everything */
    DM2_TELEPORT_SCOPE_OBJECT   = 3,  /* objects/items */
} DM2_TeleportScope;

/* ── Pressure plates / floor triggers ─────────────────────────────── */

/* Floor trigger types (sensor side).
 * Source: docs/dm2_sensors.md "Floor Sensors" table
 *         ACTUATOR_FLOOR_TYPE__* in defines.h */
typedef enum {
    DM2_FLOOR_EVERYTHING       = 0x01,  /* any entity steps */
    DM2_FLOOR_PARTY             = 0x03,  /* party member steps */
    DM2_FLOOR_ITEM              = 0x04,  /* item placed/dropped */
    DM2_FLOOR_CREATURE          = 0x07,  /* creature steps */
    DM2_FLOOR_ITEM_POSSESSION   = 0x08,  /* party member carrying item steps */
    DM2_FLOOR_TELEPORTER        = 0x2E,  /* party teleporter */
    DM2_FLOOR_SHOP              = 0x30,  /* party enters shop */
} DM2_FloorTriggerType;

/* Activate the floor trigger at (level, x, y). Returns 0 on success. */
int dm2_v1_runtime_floor_trigger(int level, int x, int y);

/* ── Actuators / triggers ──────────────────────────────────────────── */

/* Actuator type taxonomy (effect side).
 * Source: docs/dm2_triggers.md "Actuator Type Taxonomy"
 *         ACTUATOR_TYPE_* in defines.h */
typedef enum {
    DM2_ACTUATOR_DM1_WALL_SWITCH       = 0x01,
    DM2_ACTUATOR_ITEM_WATCHER          = 0x03,
    DM2_ACTUATOR_MISSILE_SHOOTER       = 0x08,
    DM2_ACTUATOR_WEAPON_SHOOTER        = 0x09,
    DM2_ACTUATOR_ITEM_SHOOTER           = 0x0E,
    DM2_ACTUATOR_CROSS_MAP             = 0x16,
    DM2_ACTUATOR_2_STATE_WALL_SWITCH   = 0x17,
    DM2_ACTUATOR_WALL_SWITCH           = 0x18,
    DM2_ACTUATOR_KEY_HOLE              = 0x1A,
    DM2_ACTUATOR_COUNTER               = 0x1D,
    DM2_ACTUATOR_TICK_GENERATOR        = 0x1E,
    DM2_ACTUATOR_RELAY_1               = 0x20,
    DM2_ACTUATOR_ARRIVAL_DEPARTURE     = 0x21,
    DM2_ACTUATOR_FLYING_ITEM_CATCHER   = 0x22,
    DM2_ACTUATOR_FLYING_ITEM_TELEPORTER = 0x23,
    DM2_ACTUATOR_SWITCH_SIGN_FOR_CREATURE = 0x26,
    DM2_ACTUATOR_CREATURE_GENERATOR    = 0x2E,
    DM2_ACTUATOR_WORK_TIMER            = 0x31,
    DM2_ACTUATOR_ITEM_GENERATOR        = 0x3C,
    DM2_ACTUATOR_RELAY_2               = 0x3D,
    DM2_ACTUATOR_SHOP_PANEL            = 0x3F,
    DM2_ACTUATOR_ITEM_RECYCLER         = 0x40,
    DM2_ACTUATOR_PUSH_BUTTON_WALL_SWITCH = 0x46,
    DM2_ACTUATOR_ITEM_CAPTURE          = 0x47,
    DM2_ACTUATOR_RESURECTOR            = 0x7E,
} DM2_ActuatorType;

/* Activate an actuator by type+position. Returns 0 on success. */
int dm2_v1_runtime_invoke_actuator(int level, int x, int y,
                                     DM2_ActuatorType type, uint16_t flag);
int dm2_v1_runtime_invoke_square_actuators(int level, int x, int y);

/* ── Shops / NPCs ──────────────────────────────────────────────────── */

/* Enter shop mode. DM2 shops are accessed via ACTUATOR_TYPE_SHOP_PANEL (0x3F).
 * Source: SKULL.ASM T800 outdoor/shop entry
 *         docs/dm2_interaction.md "SHOP_PANEL opens shop interface" */
int dm2_v1_runtime_enter_shop(int level, int x, int y);

/* Interact with merchant NPC (AI index 33).
 * CCM_MERCHANT_BEHAVIOR (0x0A) handles the shop interaction flow.
 * Source: dm2_v1_creature.h DM2_AI_MERCHANT=33
 *         DM2_CCM_MERCHANT_BEHAVIOR=0x0a */
int dm2_v1_runtime_npc_interact(int level, int x, int y);
int dm2_v1_runtime_get_last_npc_id(void);
int dm2_v1_runtime_get_last_npc_dialog_line(void);
int dm2_v1_runtime_signal_item_used(int item_id);
int dm2_v1_runtime_signal_combat_ended(int victory);
const char *dm2_v1_runtime_get_last_target_message(void);
int dm2_v1_runtime_get_last_spawn_instance_id(void);
int dm2_v1_runtime_get_last_spawn_ai(void);
int dm2_v1_runtime_get_last_spawn_x(void);
int dm2_v1_runtime_get_last_spawn_y(void);
int dm2_v1_runtime_get_last_spawn_level(void);
int dm2_v1_runtime_get_spawn_count(void);
int dm2_v1_runtime_get_last_actuator_type(void);
int dm2_v1_runtime_get_last_actuator_x(void);
int dm2_v1_runtime_get_last_actuator_y(void);
int dm2_v1_runtime_get_last_actuator_level(void);
int dm2_v1_runtime_get_actuator_count(void);
uint32_t dm2_v1_runtime_get_last_generated_object(void);
int dm2_v1_runtime_get_last_projectile_slot(void);
int dm2_v1_runtime_get_projectile_actuator_count(void);

/* ── Special squares ──────────────────────────────────────────────── */

/* Check if a square is passable (wall, pit, lava block movement).
 * Uses dm2_world_is_walkable internally. */
int dm2_v1_runtime_is_passable(int level, int x, int y);

/* Get the square type at a position (0..15, or -1 on error).
 * Lower 5 bits of tile word — same encoding as DM1. */
int dm2_v1_runtime_get_square_type(int level, int x, int y);

/* ── Source evidence ──────────────────────────────────────────────── */
const char *dm2_v1_runtime_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_RUNTIME_H */
