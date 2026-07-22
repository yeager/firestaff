#ifndef FIRESTAFF_DM2_V1_RUNTIME_H
#define FIRESTAFF_DM2_V1_RUNTIME_H

#include "dm2_v1_surface_snapshot.h"
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
#include "dm2_v1_proceed_timers_pc34_compat.h"
#include "dm2_v1_think_creature_pc34_compat.h"
#include "dm2_v1_creature_schedule_pc34_compat.h"
#include "dm2_v1_caii_alloc_pc34_compat.h"
#include "dm2_v1_delete_creature_full_pc34_compat.h"
#include "dm2_v1_new_game.h"
#include "dm2_v1_perform_move.h"
#include "dm2_v1_startup_menu.h"
#include "dm2_v1_viewport_renderer.h"
#include "dm2_v1_g1_scene_runtime_bridge.h"
#include "dm2_v1_gdat_door_overlay_m11_command.h"
#include "dm2_v1_gdat_wall_m11_command.h"
#include "dm2_v1_weather_gdat.h"
#include "dm2_v1_weather.h"

/* Runtime-visible proof that the M11-owned frame consumed DM2 GDAT pixels.
 * This is deliberately aggregate: it proves ownership and real consumption
 * without exposing a renderer buffer beyond its frame lifetime. */
typedef struct {
    unsigned int generation;
    int runtime_frame_owned;
    int is_outdoor;
    int gdat_provider_bound;
    int startup_title_gdat_blits;
    int startup_menu_gdat_blits;
    int hud_gdat_blits;
    int hud_core_gdat_blits;
    int door_gdat_blits;
    int creature_gdat_blits;
    int creature_gdat_material_evidence_count;
    uint32_t creature_gdat_material_evidence_hash;
    int floor_ceiling_gdat_blits;
    int outdoor_sky_gdat_blits;
    int outdoor_ground_gdat_blits;
    int wall_gdat_blits;
    int wall_gdat_material_evidence_count;
    uint32_t wall_gdat_material_evidence_hash;
    int gdat_wall_material_plan_consumed;
    int item_gdat_blits;
    int item_gdat_material_evidence_count;
    uint32_t item_gdat_material_evidence_hash;
    int projectile_gdat_blits;
    int projectile_gdat_material_evidence_count;
    uint32_t projectile_gdat_material_evidence_hash;
    int total_runtime_gdat_blits;
    int total_runtime_fallback_draws;
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
    int gdat_c_light_receipt_ready;
    uint8_t gdat_c_light_level;
    uint32_t gdat_c_light_receipt_hash;
    uint32_t gdat_c_light_source_state_hash;
    int gdat_c_light_consumed;
    int gdat_weather_receipt_ready;
    uint32_t gdat_weather_receipt_hash;
    uint32_t gdat_weather_material_mask;
    int gdat_weather_destination_ready;
    uint32_t gdat_weather_destination_hash;
    uint32_t gdat_weather_destination_mask;
    int gdat_weather_renderer_ready;
    uint32_t gdat_weather_renderer_hash;
    uint32_t gdat_weather_renderer_command_count;
    int gdat_dialogue_shell_receipt_ready;
    uint32_t gdat_dialogue_shell_receipt_hash;
    int gdat_scene_light_consumed;
    int gdat_scene_weather_consumed;
    int gdat_sprite_palette_consumed;
    int gdat_interface_palette_ready;
    int gdat_interface_palette_consumed;
    uint32_t gdat_interface_action_palette_hash;
    int gdat_interface_action_palette_consumed;
    /* INTERFACE_GENERAL dt07/0x0A Rect14 placement table consumed by the
     * runtime viewport. skproject/SKWIN/SkWinCore.cpp LOAD_GDAT_INTERFACE_00_0A
     * supplies the 14-byte rows used by QUERY_CREATURE_BLIT_RECTI and
     * DM2_DRAW_ITEM placement. */
    int gdat_interface_rect14_ready;
    uint32_t gdat_interface_rect14_table_hash;
    uint32_t gdat_interface_rect14_placement_hash;
    uint32_t gdat_interface_rect14_row_count;
    int gdat_material_palette_floor_ceiling_consumed;
    int gdat_material_palette_wall_consumed;
    int gdat_material_palette_door_frame_consumed;
    uint8_t floor_ceiling_material_required_mask;
    uint8_t floor_ceiling_material_consumed_mask;
    int floor_ceiling_materials_complete;
    int blocked_material_draws;
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

/* Exact post-load ownership facts reconstructed from the original ten-byte
 * timer table.  skproject/SKWIN/SkWinCore.cpp::_3a15_020f only reconnects
 * tty0C to Champion::timerIndex directly; tty1D/tty1E instead require a
 * RecordE DB address.  Keep that latter dependency explicit until the saved
 * DB graph has a source-owned runtime representation. */
typedef struct {
    int valid;
    uint8_t timer_count;
    /* skproject/SKULLWIN/c_timer.cpp::DM2_SORT_TIMERS reconstructs a
     * min-heap of timer-table indices after SKSave load.  Keep the exact
     * source ordering as a receipt; this is not permission to dispatch an
     * otherwise unimplemented timer type. */
    uint8_t timer_heap_count;
    uint8_t timer_heap_index[DM2_MAX_TIMERS];
    uint8_t next_timer_index;
    uint32_t next_timer_tick;
    uint32_t timer_heap_hash;
    uint8_t champion_timer_bound_mask;
    uint8_t champion_timer_index[4];
    uint8_t unresolved_record_timer_count;
    uint8_t other_timer_count;
} DM2_V1_RuntimeTimerPostLoadReceipt;
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
int dm2_v1_runtime_g1_map5_gdat_text_message_receipt(
    DM2_V1_G1GdatTextMessageRuntimeReceipt *out_receipt);
int dm2_v1_runtime_g1_map5_text_wall_gfx_receipt(
    DM2_V1_G1TextWallGfxRuntimeReceipt *out_receipt);
int dm2_v1_runtime_g1_actuator_wall_gfx_receipt(
    DM2_V1_G1ActuatorWallGfxRuntimeReceipt *out_receipt);
int dm2_v1_runtime_g1_creature_map_chip_receipt(
    DM2_V1_G1CreatureMapChipRuntimeReceipt *out_receipt);
int dm2_v1_runtime_g1_weapon_map_chip_receipt(
    DM2_V1_G1WeaponMapChipRuntimeReceipt *out_receipt);
/* Source-classified G1 tile/root handoff used by the live dungeon frame.
 * A direct class without a source material is reported blocked, never drawn
 * through a generic material. */
int dm2_v1_runtime_g1_scene_handoff_receipt(
    DM2_V1_G1SceneRuntimeHandoffReceipt *out_receipt);
int  dm2_v1_runtime_bind_boot_profile_with_receipt(
    DM2_V1_BootProfile *boot_profile,
    DM2_V1_StartupHostReceipt *out_receipt);
int  dm2_v1_runtime_apply_session(const DM2_V1_SessionState *session);
void dm2_v1_runtime_tick(void);
int  dm2_v1_runtime_get_tick_count(void);
int  dm2_v1_runtime_can_move(void);
int  dm2_v1_runtime_move(int direction);        /* 0=N 1=E 2=S 3=W, returns 0=ok -1=blocked */
int  dm2_v1_runtime_last_perform_move_receipt(
    DM2_V1_PerformMoveReceipt *out_receipt);
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
uint32_t dm2_v1_runtime_frame_presentation_state_hash(
    uint32_t scene_light_hash, uint16_t ambient_light,
    uint32_t c_light_receipt_hash, uint32_t c_light_source_state_hash,
    uint8_t c_light_level, int c_light_consumed,
    int weather_graphicsset_bound, uint8_t weather_graphicsset,
    uint32_t weather_source_receipt_hash,
    uint32_t weather_destination_receipt_hash,
    uint32_t weather_material_hash);
uint32_t dm2_v1_runtime_g1_scene_map_token(int level, int graphicsset,
                                            int outdoor);
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
    int door_gfx_admitted;
    int door_opening_dir;
    int ornament_index;
    int door_ornate_gfx_index;
    int door_button;
    int door_button_state;
    int door_state;
    int door_open_pct;
    int panel_gdat_index;
    int ornate_gdat_index;
    int destroyed_mask_gdat_index;
    int frame_gdat_index;
    int side_frame_gdat_index[2];
    int side_frame_graphicsset_field[2];
    int side_frame_rect_number[2];
    int side_frame_mirror_flip[2];
    int side_frame_offset_x[2];
    int side_frame_offset_y[2];
    int button_gdat_index;
    int button_source_kind; /* 1=default door button, 2=wall-gfx button */
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

typedef enum { DM2_V1_RUNTIME_CORPUS_IMPORT_NONE = 0,
               DM2_V1_RUNTIME_CORPUS_IMPORT_OK,
               DM2_V1_RUNTIME_CORPUS_IMPORT_UNAVAILABLE,
               DM2_V1_RUNTIME_CORPUS_IMPORT_REJECTED } DM2_V1_RuntimeCorpusImportResult;
typedef struct {
    DM2_V1_RuntimeCorpusImportResult result;
    int restored;
    int candidate_kind;
    size_t selected_payload_size;
    uint32_t selected_payload_hash;
    /* Complete SKSave-file identity from the scanner receipt. Runtime restore
     * rechecks this before it reads any payload bytes. */
    uint32_t selected_source_file_hash;
    int rejected_original_candidate;
    char selected_path[256];
} DM2_V1_RuntimeCorpusImportReceipt;

/* Original-save-only GAME_LOAD handoff.  The caller must select a row from
 * dm2_v1_original_save_state_corpus_probe(); this receipt rescans the corpus
 * and admits that exact decoded row only.  It deliberately has no
 * first-importable or Firestaff-session fallback. */
typedef struct {
    DM2_V1_RuntimeCorpusImportReceipt runtime_import;
    int corpus_complete;
    int selected_state_admitted;
    uint16_t original_candidate_count;
    uint16_t parsed_candidate_count;
    uint32_t corpus_hash;
    uint32_t selected_state_hash;
    int selected_raw_dungeon_layout_valid;
    uint8_t selected_raw_dungeon_map_count;
    uint16_t selected_raw_db_record_counts[DM2_ORIGINAL_SAVE_RAW_DB_POOL_COUNT];
    uint32_t selected_raw_dungeon_prefix_hash;
    uint32_t selected_raw_map_data_hash;
} DM2_V1_RuntimeOriginalCorpusImportReceipt;

/* Source-owned raw-SKSave dungeon handoff carried from GAME_LOAD into the
 * first runtime frame. It exposes layout identity only, never record fields
 * or GenericRecord links. */
typedef struct {
    int valid;
    int first_frame_consumed;
    uint8_t map_count;
    uint16_t db_record_counts[DM2_RAW_SKSAVE_DB_POOL_COUNT];
    size_t dungeon_byte_count;
    uint32_t prefix_hash;
    uint32_t map_data_hash;
    uint16_t party_level;
    uint16_t party_x;
    uint16_t party_y;
    uint8_t party_dir;
} DM2_V1_RuntimeRawSaveHandoffReceipt;
int dm2_v1_runtime_last_raw_sksave_handoff_receipt(
    DM2_V1_RuntimeRawSaveHandoffReceipt *out_receipt);
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

typedef struct {
    int valid;
    int no_draw;
    DM2_V1_G1FlyingItemSourceReceipt source;
    uint32_t raw_gfx256_hash;
    uint32_t raw_gfx256_receipt_hash;
    uint32_t palette_hash;
    uint32_t raw4_hash;
    uint32_t raw4_receipt_hash;
    uint32_t timer_receipt_hash;
    uint32_t identity_hash;
} DM2_V1_RuntimeFlyingItemReceipt;

/* Source-owned DB14 viewport admission. This enriches an already complete
 * timer/GDAT receipt with the SKWIN selector and table geometry, but remains
 * explicitly incapable of supplying an image field or pixels. */
typedef struct {
    int valid;
    int no_draw;
    uint32_t session_identity;
    uint32_t map_load_token;
    uint32_t timer_receipt_hash;
    uint32_t gdat_identity_hash;
    uint32_t selector_identity_hash;
    uint32_t geometry_identity_hash;
    uint32_t identity_hash;
} DM2_V1_RuntimeFlyingItemViewportEvidence;

/* The decoded DB14 material cannot become a frame by itself. This plan joins
 * it to the live timer/session/map and selector/table identities, while
 * remaining explicitly no-draw until an original frame consumer is proven. */
typedef struct {
    int valid;
    int no_draw;
    uint32_t session_identity;
    uint32_t map_load_token;
    uint32_t timer_receipt_hash;
    uint32_t selector_identity_hash;
    uint32_t vb30_identity_hash;
    uint32_t geometry_identity_hash;
    uint32_t material_identity_hash;
    uint32_t raw_gfx256_hash;
    uint32_t raw_gfx256_receipt_hash;
    uint32_t decoded_pixels_hash;
    uint32_t palette_hash;
    uint32_t identity_hash;
} DM2_V1_RuntimeFlyingItemDecodedMaterialPlan;

/* c_gui_vp.cpp:4320-4417 orders PUT_DOWN_ITEM, creature summary, then
 * DRAW_FLYING_ITEM. This DM2-owned M11-boundary receipt rechecks the actual
 * indexed IMG3 bytes and local palette, but never exposes a framebuffer or
 * invents the still-unproven destination transform. */
typedef struct {
    int valid;
    int no_draw;
    uint8_t source_order;
    uint8_t follows_static_and_creature;
    uint8_t indexed_bytes_consumed;
    uint8_t orientation_unapplied;
    uint16_t clip_rect_id;
    uint8_t flip_flags;
    uint16_t width;
    uint16_t height;
    DM2_ImageFormat format;
    int16_t source_offset_x;
    int16_t source_offset_y;
    uint32_t offset_receipt_hash;
    uint32_t delivery_identity_hash;
    uint32_t decoded_material_identity_hash;
    uint32_t indexed_pixels_hash;
    uint32_t palette_hash;
    uint32_t identity_hash;
} DM2_V1_Dm2FlyingItemM11MaterialConsumerReceipt;

typedef struct {
    int valid;
    int no_draw;
    uint16_t clip_rect_id;
    DM2_V1_InterfaceRect clip_rect;
    int16_t source_offset_x;
    int16_t source_offset_y;
    uint8_t flip_flags;
    uint8_t orientation_unapplied;
    uint32_t raw4_hash;
    uint32_t raw4_receipt_hash;
    uint32_t offset_receipt_hash;
    uint32_t consumer_identity_hash;
    uint32_t identity_hash;
} DM2_V1_Dm2FlyingItemDestinationReceipt;

typedef struct {
    int valid;
    int no_draw;
    uint16_t scale_x;
    uint16_t scale_y;
    uint8_t blitmode;
    DM2_V1_InterfaceRect clip_rect;
    int16_t source_offset_x;
    int16_t source_offset_y;
    uint32_t destination_identity_hash;
    uint32_t material_identity_hash;
    uint32_t identity_hash;
} DM2_V1_Dm2FlyingItemPicstTransformReceipt;

/* Separate M11 delivery plan for SKWIN DRAW_FLYING_ITEM. It transports source
 * identities only; a later original pixel decoder must explicitly unlock any
 * raster work. It never aliases the older projectile/map-chip plan. */
typedef struct {
    int valid;
    int no_draw;
    int pixel_decoder_ready;
    int m11_delivery_ready;
    uint16_t missile_object_id;
    uint8_t category;
    uint8_t item_type;
    uint8_t image_field;
    uint8_t flip_flags;
    uint8_t cell_pos;
    uint8_t position_5x5;
    uint16_t clip_rect_id;
    uint16_t stretch_factor64;
    uint32_t raw_gfx256_hash;
    uint32_t raw_gfx256_receipt_hash;
    uint32_t palette_hash;
    uint32_t raw4_hash;
    uint32_t raw4_receipt_hash;
    uint32_t timer_receipt_hash;
    uint32_t viewport_evidence_hash;
    uint32_t viewport_session_identity;
    uint32_t viewport_map_load_token;
    uint32_t identity_hash;
} DM2_V1_FlyingItemM11DeliveryPlan;

/* Separate DB5/DB9 DRAW_STATIC_OBJECT delivery plan. It is intentionally a
 * source receipt, not a decoded sprite or a map-chip/F9 compatibility path. */
typedef struct {
    int valid;
    int no_draw;
    int pixel_decoder_ready;
    int m11_delivery_ready;
    uint32_t session_identity;
    uint16_t object_id;
    uint8_t category;
    uint8_t item_type;
    uint8_t image_field;
    uint8_t direction;
    uint8_t container_open;
    uint16_t image_offset;
    int source_cell;
    int source_pass;
    int position_5x5;
    uint16_t clip_rect_id;
    int stretch_factor64;
    int flip_mirror;
    uint32_t selector_identity_hash;
    uint32_t raw_gfx256_hash;
    uint32_t raw_gfx256_receipt_hash;
    uint32_t palette_hash;
    uint32_t raw4_hash;
    uint32_t raw4_receipt_hash;
    uint32_t rect14_row_hash;
    uint32_t rect14_placement_hash;
    uint32_t identity_hash;
} DM2_V1_StaticObjectM11DeliveryPlan;

/* DM2-owned M11 composition gate.  This binds already-admitted GDAT plans
 * into one source order, but deliberately cannot authorize pixel decoding or
 * a framebuffer blit.  The caller supplies the active session/data epoch;
 * every member receipt is folded into the immutable composition identity. */
typedef struct {
    int valid;
    int no_draw;
    int pixel_decoder_ready;
    int m11_delivery_ready;
    uint32_t session_identity;
    uint32_t data_epoch;
    uint32_t scene_command_hash;
    uint32_t scene_light_receipt_hash;
    uint32_t c_light_receipt_hash;
    uint32_t wall_command_hash;
    uint32_t door_command_hash;
    uint32_t weather_receipt_hash;
    uint32_t static_object_identity_hash;
    uint32_t flying_item_identity_hash;
    uint32_t ordered_member_hash;
    uint32_t identity_hash;
    DM2_V1_ViewportSurfaceSnapshot surface_before;
    DM2_V1_ViewportSurfaceSnapshot surface_after;
    uint32_t surface_generation_hash;
} DM2_V1_Dm2ViewportM11CompositionReceipt;

typedef struct {
    int valid;
    int no_draw;
    DM2_V1_Dm2ViewportM11CompositionReceipt composition;
    DM2_V1_Dm2FlyingItemM11MaterialConsumerReceipt flying_item_material;
    uint32_t identity_hash;
} DM2_V1_Dm2ViewportM11MaterialCompositionReceipt;

/* Live-session owner for the source plans admitted by the DM2 composition
 * gate.  These pointers borrow runtime-owned G1/GDAT receipts only for the
 * call; no copy becomes a drawable surface. */
typedef struct {
    const DM2_V1_GdatSceneM11CommandPlan *scene;
    const DM2_V1_GdatSceneLightM11Receipt *scene_light;
    const DM2_V1_CLightM11Receipt *c_light;
    const DM2_V1_GdatWallM11CommandPlan *wall;
    const DM2_V1_GdatDoorOverlayM11CommandPlan *door;
    const DM2_V1_OutdoorWeatherM11Receipt *weather;
    const DM2_V1_StaticObjectM11DeliveryPlan *static_object;
    const DM2_V1_FlyingItemM11DeliveryPlan *flying_item;
} DM2_V1_Dm2ViewportM11LivePlanSet;

typedef struct {
    int valid;
    int no_draw;
    uint8_t graphicsset;
    uint8_t outdoor;
    uint16_t level;
    uint32_t session_identity;
    uint32_t map_load_token;
    DM2_V1_Dm2ViewportM11CompositionReceipt composition;
} DM2_V1_Dm2ViewportM11LiveCompositionReceipt;

uint32_t dm2_v1_runtime_dm2_viewport_session_identity(
    const DM2_V1_SessionState *session);
int dm2_v1_runtime_enumerate_dm2_viewport_m11_live_plans(
    const DM2_V1_SessionState *session, uint8_t graphicsset,
    const DM2_V1_Dm2ViewportM11LivePlanSet *plans,
    DM2_V1_Dm2ViewportM11LiveCompositionReceipt *out_receipt,
    const DM2_V1_ViewportState *owner);
int dm2_v1_runtime_dm2_viewport_m11_live_composition_matches(
    const DM2_V1_Dm2ViewportM11LiveCompositionReceipt *receipt,
    const DM2_V1_SessionState *session, uint8_t graphicsset,
    const DM2_V1_Dm2ViewportM11LivePlanSet *plans,
    const DM2_V1_ViewportState *owner);

int dm2_v1_runtime_build_dm2_viewport_m11_composition(
    const DM2_V1_GdatSceneM11CommandPlan *scene,
    const DM2_V1_GdatSceneLightM11Receipt *scene_light,
    const DM2_V1_CLightM11Receipt *c_light,
    const DM2_V1_GdatWallM11CommandPlan *wall,
    const DM2_V1_GdatDoorOverlayM11CommandPlan *door,
    const DM2_V1_OutdoorWeatherM11Receipt *weather,
    const DM2_V1_StaticObjectM11DeliveryPlan *static_object,
    const DM2_V1_FlyingItemM11DeliveryPlan *flying_item,
    uint32_t session_identity, uint32_t data_epoch,
    DM2_V1_Dm2ViewportM11CompositionReceipt *out_receipt);
int dm2_v1_runtime_dm2_viewport_m11_composition_matches(
    const DM2_V1_Dm2ViewportM11CompositionReceipt *receipt,
    const DM2_V1_GdatSceneM11CommandPlan *scene,
    const DM2_V1_GdatSceneLightM11Receipt *scene_light,
    const DM2_V1_CLightM11Receipt *c_light,
    const DM2_V1_GdatWallM11CommandPlan *wall,
    const DM2_V1_GdatDoorOverlayM11CommandPlan *door,
    const DM2_V1_OutdoorWeatherM11Receipt *weather,
    const DM2_V1_StaticObjectM11DeliveryPlan *static_object,
    const DM2_V1_FlyingItemM11DeliveryPlan *flying_item,
    uint32_t session_identity, uint32_t data_epoch);

int dm2_v1_runtime_last_flying_item_receipt(
    DM2_V1_RuntimeFlyingItemReceipt *out_receipt);
int dm2_v1_runtime_flying_item_viewport_evidence(
    const DM2_V1_RuntimeFlyingItemReceipt *material,
    const DM2_V1_G1FlyingItemSelectorReceipt *selector,
    const DM2_V1_G1FlyingItemGeometryReceipt *geometry,
    uint32_t session_identity, uint32_t map_load_token,
    DM2_V1_RuntimeFlyingItemViewportEvidence *out_evidence);
int dm2_v1_runtime_flying_item_decoded_material_plan(
    const DM2_V1_RuntimeFlyingItemReceipt *timer_receipt,
    const DM2_V1_G1FlyingItemSelectorReceipt *selector,
    const DM2_V1_G1FlyingItemVb30Receipt *vb30,
    const DM2_V1_G1FlyingItemGeometryReceipt *geometry,
    const DM2_V1_G1FlyingItemDecodedMaterialReceipt *material,
    uint32_t session_identity, uint32_t map_load_token,
    DM2_V1_RuntimeFlyingItemDecodedMaterialPlan *out_plan);
int dm2_v1_runtime_flying_item_decoded_material_plan_matches(
    const DM2_V1_RuntimeFlyingItemDecodedMaterialPlan *plan,
    const DM2_V1_RuntimeFlyingItemReceipt *timer_receipt,
    const DM2_V1_G1FlyingItemSelectorReceipt *selector,
    const DM2_V1_G1FlyingItemVb30Receipt *vb30,
    const DM2_V1_G1FlyingItemGeometryReceipt *geometry,
    const DM2_V1_G1FlyingItemDecodedMaterialReceipt *material,
    uint32_t session_identity, uint32_t map_load_token);
int dm2_v1_runtime_consume_flying_item_decoded_material_for_m11(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_FlyingItemM11DeliveryPlan *delivery,
    const DM2_V1_RuntimeFlyingItemDecodedMaterialPlan *plan,
    const DM2_V1_G1FlyingItemDecodedMaterialReceipt *material,
    const DM2_V1_G1FlyingItemSourceReceipt *source,
    DM2_V1_Dm2FlyingItemM11MaterialConsumerReceipt *out_receipt);
int dm2_v1_runtime_flying_item_destination_receipt(
    const DM2_V1_Dm2FlyingItemM11MaterialConsumerReceipt *consumer,
    const DM2_V1_G1FlyingItemSourceReceipt *source,
    const DM2_V1_BootExpandedRectReceipt *clip,
    DM2_V1_Dm2FlyingItemDestinationReceipt *out_receipt);
int dm2_v1_runtime_flying_item_picst_transform_receipt(
    const DM2_V1_Dm2FlyingItemDestinationReceipt *destination,
    const DM2_V1_G1FlyingItemDecodedMaterialReceipt *material,
    const DM2_V1_G1FlyingItemSourceReceipt *source,
    DM2_V1_Dm2FlyingItemPicstTransformReceipt *out_receipt);
int dm2_v1_runtime_blit_flying_item_normal_scale_indexed(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_Dm2FlyingItemM11MaterialConsumerReceipt *consumer,
    const DM2_V1_Dm2FlyingItemDestinationReceipt *destination,
    const DM2_V1_Dm2FlyingItemPicstTransformReceipt *transform,
    const DM2_V1_G1FlyingItemDecodedMaterialReceipt *material,
    uint8_t *framebuffer, int framebuffer_width, int framebuffer_height,
    int framebuffer_stride);
int dm2_v1_runtime_build_dm2_viewport_m11_material_composition(
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_Dm2FlyingItemM11MaterialConsumerReceipt *flying_item,
    DM2_V1_Dm2ViewportM11MaterialCompositionReceipt *out_receipt);
int dm2_v1_runtime_dm2_viewport_m11_material_composition_matches(
    const DM2_V1_Dm2ViewportM11MaterialCompositionReceipt *receipt,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_Dm2FlyingItemM11MaterialConsumerReceipt *flying_item);
int dm2_v1_runtime_last_static_object_m11_delivery_plans(
    DM2_V1_StaticObjectM11DeliveryPlan *out_plans, int max_plans,
    int *out_count);
int dm2_v1_viewport_build_flying_item_m11_delivery_plan(
    const DM2_V1_RuntimeFlyingItemReceipt *receipt,
    DM2_V1_FlyingItemM11DeliveryPlan *out_plan);
int dm2_v1_viewport_build_flying_item_m11_delivery_plan_from_viewport_evidence(
    const DM2_V1_RuntimeFlyingItemReceipt *receipt,
    const DM2_V1_RuntimeFlyingItemViewportEvidence *evidence,
    DM2_V1_FlyingItemM11DeliveryPlan *out_plan);
int dm2_v1_viewport_build_static_object_m11_delivery_plan(
    const DM2_V1_G1StaticObjectMaterialReceipt *material,
    const DM2_V1_StaticObjectSourcePlan *source_plan,
    uint32_t session_identity,
    DM2_V1_StaticObjectM11DeliveryPlan *out_plan);
int dm2_v1_viewport_static_object_m11_delivery_plan_matches(
    const DM2_V1_StaticObjectM11DeliveryPlan *plan,
    const DM2_V1_G1StaticObjectMaterialReceipt *material,
    const DM2_V1_StaticObjectSourcePlan *source_plan,
    uint32_t session_identity);
int dm2_v1_runtime_flying_item_timer_receipt(
    const DM2_V1_G1DirectMissileReceipt *missile,
    const DM2_V1_G1FlyingItemSourceReceipt *source,
    const DM2_V1_G1MissileTimerReceipt *timer,
    DM2_V1_RuntimeFlyingItemReceipt *out_receipt);
int dm2_v1_runtime_flying_item_timer_from_session(
    const DM2_V1_SessionState *session,
    const DM2_V1_G1DirectMissileReceipt *missile,
    const DM2_V1_G1FlyingItemSourceReceipt *source,
    DM2_V1_RuntimeFlyingItemReceipt *out_receipt);
int dm2_v1_runtime_admit_flying_item_material(
    const DM2_V1_SessionState *session,
    const DM2_V1_G1DirectMissileReceipt *missile,
    const DM2_V1_G1FlyingItemSourceReceipt *source,
    const DM2_V1_G1FlyingItemMaterialReceipt *material);
int dm2_v1_runtime_flying_item_material_receipt(
    const DM2_V1_RuntimeFlyingItemReceipt *timer_receipt,
    const DM2_V1_G1FlyingItemMaterialReceipt *material,
    DM2_V1_RuntimeFlyingItemReceipt *out_receipt);
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
int dm2_v1_runtime_last_timer_post_load_receipt(
    DM2_V1_RuntimeTimerPostLoadReceipt *out_receipt);
/* DM2-003: DM2-owned source-order timer route.  Every DM2 timer enters
 * the runtime queue here and is dispatched by dm2_v1_proceed_timers
 * (skproject/SKULLWIN/c_tim_proc.cpp DM2_PROCEED_TIMERS) during
 * dm2_v1_runtime_tick; no host-side behavioural substitution. */
DM2_V1_SourceTimerResult dm2_v1_runtime_enqueue_source_timer(
    const DM2_V1_SourceTimer *timer, uint16_t source_index);
int dm2_v1_runtime_last_proceed_timers_receipt(
    DM2_V1_ProceedTimersReceipt *out_receipt);
/* DM2-003 follow-up: 1 while a producer-bound type-0x54 weather timer
 * (skproject/SKULLWIN/c_weather.cpp:20-30 DM2_SET_TIMER_WEATHER) is
 * pending in the runtime source queue. */
int dm2_v1_runtime_weather_source_timer_pending(void);
/* DM2-003 follow-up: the source 0x54 weather chain
 * (skproject/SKULLWIN/c_weather.cpp DM2_weather_3df7_0037 +
 * DM2_UPDATE_WEATHER(1)) is session-owned; 1 while the chain runs. */
int dm2_v1_runtime_weather_chain_started(void);
/* Copies the session-owned v1e14xx weather chain state into `out`;
 * returns 0 when the chain is not running or `out` is NULL. */
struct DM2_V1_UpdateWeatherState;
int dm2_v1_runtime_weather_chain_snapshot(
    struct DM2_V1_UpdateWeatherState *out);
/* DM2-003/005 follow-up: 1 once the session-owned DM2-002 record pools
 * validated from the boot dungeon data (lazy, on the first tick). */
int dm2_v1_runtime_record_pools_valid(void);
/* Copies the per-cell DM2_THINK_CREATURE binding receipt into `out`;
 * returns 0 when the binding is not ready or `out` is NULL. */
int dm2_v1_runtime_think_creature_receipt(
    DM2_V1_ThinkCreatureReceipt *out);
/* DM2-003 follow-up: DM2-owned creature-scheduling producer boundary
 * (skproject/SKULLWIN/c_1c9a.cpp:5695-5728 DM2_1c9a_0cf7).  Schedules one
 * 0x21/0x22 source timer for the creature record at cell (x, y) into the
 * runtime source queue; the timer is then dispatched by
 * dm2_v1_proceed_timers to the per-cell DM2_THINK_CREATURE binding on the
 * next tick — the end-to-end producer/consumer chain.  `map_id` stands in
 * for ddat.v1d3248 (caller-owned until DM2_CHANGE_CURRENT_MAP_TO is
 * proven); the due tick is the session tick + 1 exactly like the source's
 * setmticks(gametick + 1).  Returns 1 when a timer was enqueued;
 * fail-closed 0 otherwise.  When `out` is non-NULL it always receives the
 * audit receipt. */
int dm2_v1_runtime_schedule_creature_at(
    int map_id, int x, int y,
    DM2_V1_CreatureScheduleReceipt *out);
/* DM2-003/005 follow-up: session-owned CAII (creature-array) setup.
 * `capacity` stands in for ddat.v1e08a0 (computed at DM2_INIT by
 * DM2_1c9a_3c30, startend.cpp:467-494 — the savegame word@0x14 owner is
 * unproven, so the caller owns the capacity).  Returns 1 when the array
 * is ready; re-initializing frees the previous array. */
int dm2_v1_runtime_caii_init(int capacity);
/* DM2-003/005 follow-up: DM2-owned lazy creature-activation boundary —
 * the bounded slice of DM2_ALLOC_CAII_TO_CREATURE
 * (skproject/SKULLWIN/c_1c9a.cpp:5772-5894) reached the way
 * DM2_ATTACK_CREATURE reaches it (record resolved via DM2_GET_CREATURE_AT
 * at the activation cell, c_creature.cpp:347-352).  Allocates one CAII
 * slot for the creature at (x, y) and queues its first 0x21/0x22 timer
 * through the bound scheduling producer.  Returns 1 when a slot was
 * allocated; 0 (fail-closed) when the cell holds no creature, the record
 * already owns a slot, no slot is free, or the session is not ready.
 * When `out` is non-NULL it always receives the audit receipt. */
int dm2_v1_runtime_alloc_caii_at(
    int x, int y,
    DM2_V1_CaiiAllocReceipt *out);
/* DM2-003/005 follow-up: the COMPLETE DM2_1c9a_0cf7 replacement slice
 * over the session CAII array (c_1c9a.cpp:5695-5728) — the way the
 * source's direct callers reach the producer (c_creature.cpp:648,
 * c_move.cpp:700).  Deletes the creature's previously queued timer
 * through the bound DM2_1c9a_0db0 path (receipt.replaced_existing == 1)
 * before queueing the new 0x21/0x22 timer and storing the issued ticket
 * in the CAII slot timer word.  Returns 1 when a timer was enqueued;
 * 0 (fail-closed) when the cell holds no creature, the record owns no
 * CAII slot (receipt.no_caii_slot == 1), or the session is not ready. */
int dm2_v1_runtime_reschedule_creature_at(
    int x, int y,
    DM2_V1_CreatureScheduleReceipt *out);
/* DM2-003/005 follow-up: free one session CAII slot — the bounded
 * DM2_1c9a_0fcb slice (c_1c9a.cpp:5896-5944), exposed for the source's
 * despawn/cleanup callers (c_ai.cpp:5775, c_moverec.cpp:684 + 997,
 * c_savegame.cpp:2049).  Deletes the slot's pending timer through the
 * bound DM2_1c9a_0db0 path, clears record byte@5 and marks the slot
 * free; the DM2_DELETE_CREATURE_RECORD branch stays unbound (receipted
 * record_delete_unbound).  Returns 1 when the slot was freed;
 * 0 (fail-closed) otherwise. */
int dm2_v1_runtime_free_caii_slot(
    int slot_index,
    DM2_V1_CaiiFreeReceipt *out);
/* 1 once dm2_v1_runtime_caii_init succeeded; the alloc counter stands
 * in for ddat.v1d4020. */
int dm2_v1_runtime_caii_ready(void);
int dm2_v1_runtime_caii_alloc_count(void);
/* Session/test support: write one CAII slot's mode byte (byte@1a),
 * mirroring the source's slot-mode writers (e.g. the 0x13 dying mode
 * that gates the 0fcb record-delete branch, c_1c9a.cpp:5921-5929).
 * Returns 1 on success, 0 (fail-closed) when the CAII session is
 * unready or the index is out of range. */
int dm2_v1_runtime_caii_set_slot_mode_byte(int slot_index, int value);
/* Read-only access to the last full DELETE_CREATURE_RECORD composition
 * receipt produced through the production-wired 0fcb branch hook
 * (c_1c9a.cpp:5956-5957).  Returns 1 and copies the receipt when the
 * branch ran since the last runtime init; 0 otherwise. */
int dm2_v1_runtime_last_delete_full_receipt(
    DM2_V1_DeleteCreatureFullReceipt *out);

/* 2026-07-21 (round 23): session receipt for the floor-mecha CAII
 * activation wiring (0x04 timer, square class 1 ->
 * DM2_ACTUATE_FLOOR_MECHA chain walk -> DB3 record type 0x3a ->
 * DM2_ANIMATE_CREATURE, c_tim_proc.cpp:3009-3532 + 4297-4299). */
typedef struct {
    int valid;
    int timers;         /* class-1 0x04 timers consumed */
    int records_0x3a;   /* DB3 type-0x3a records visited */
    int activations;    /* animate activation slices evaluated valid */
    int allocs;         /* bound CAII allocations performed */
    int db_break;       /* DB > 3 chain link: source return */
    int walk_failed;    /* chain walk failed closed */
} DM2_V1_RuntimeFloorMechaReceipt;
/* Copies the session floor-mecha receipt.  Returns 1 when the think
 * binding is ready (the handler is registered); 0 otherwise. */
int dm2_v1_runtime_floor_mecha_receipt(DM2_V1_RuntimeFloorMechaReceipt *out);

/* 2026-07-22 (round 24): session receipt for the door-step timer wiring
 * (0x01 timer -> DM2_STEP_DOOR -> one source-ordered door state mutation
 * per tick, re-queued until OPEN/CLOSED). */
typedef struct {
    int valid;
    int timers;     /* type-0x01 timers consumed */
    int mutations;  /* successful square state writes */
    int requeues;   /* next-step timers queued */
} DM2_V1_RuntimeDoorStepReceipt;
/* Copies the session door-step receipt.  Always returns 1; the counters
 * are zero until the first tick runs. */
int dm2_v1_runtime_door_step_receipt(DM2_V1_RuntimeDoorStepReceipt *out);

int dm2_v1_runtime_import_sksave_corpus(
    const char *save_root, DM2_V1_RuntimeCorpusImportReceipt *out);
/* Source: skproject/SKULLWIN/c_savegame.cpp::DM2_SELECT_LOAD_GAME and
 * DM2_GAME_LOAD. The caller selects one scanner-issued SKSave receipt; this
 * function revalidates its complete original file before any runtime state
 * can change. It never substitutes another corpus candidate. */
int dm2_v1_runtime_import_sksave_receipted_candidate(
    const DM2_SKSaveCandidateReceipt *candidate_receipt,
    DM2_V1_RuntimeCorpusImportReceipt *out);
/* Source: skproject/SKULLWIN/c_savegame.cpp::DM2_SELECT_LOAD_GAME followed
 * by DM2_GAME_LOAD. Revalidate a selected original-save census row against a
 * fresh full corpus probe, then restore only that same original candidate.
 * A missing, stale, non-original, or incompletely parsed row is rejected;
 * this path never selects another candidate as a substitute. */
int dm2_v1_runtime_import_original_sksave_state_entry(
    const char *save_root,
    const DM2_OriginalSaveStateCorpusEntry *selected_entry,
    DM2_V1_RuntimeOriginalCorpusImportReceipt *out);
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
int dm2_v1_runtime_last_set_timer_weather_receipt(
    DM2_V1_SetTimerWeatherReceipt *out_receipt);
int dm2_v1_runtime_last_weather_3df7_0037_receipt(
    DM2_V1_Weather3df70037Receipt *out_receipt);
int dm2_v1_runtime_last_weather_timer_receipt(
    DM2_V1_WeatherTimerReceipt *out_receipt);
/* Accepts only c_weather DistantEnvironment receipts that can be rebuilt
 * against the current source-owned MapGraphicsStyle/GDAT weather receipt.
 * A level or scene refresh clears them; no bound slot is no-draw. */
int dm2_v1_runtime_bind_weather_distant_environment(
    const DM2_V1_DistantEnvironmentReceipt *slots, unsigned int slot_count);

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

/* Leave the active shop and write the final shop gold back to the game state.
 * Returns 0 on success, -1 if no shop is active or the runtime is unbound. */
int dm2_v1_runtime_leave_shop(void);

/* Buy the stock item at `stock_idx` from the active shop.  On success the
 * purchase is applied and the updated gold is committed to the game state.
 * Returns 1 on success, or a DM2_ShopResult error code otherwise. */
int dm2_v1_runtime_buy_from_shop(int stock_idx);

/* Sell the inventory item at `inv_idx` to the active shop.  On success the
 * sale is applied and the updated gold is committed to the game state.
 * Returns 1 on success, or a DM2_ShopResult error code otherwise. */
int dm2_v1_runtime_sell_to_shop(int inv_idx);

/* Interact with merchant NPC (AI index 33).
 * The source merchant CCM states (0x17 PLACE_MERCHANDISE /
 * 0x18 TAKE_MERCHANDISE) handle the shop interaction flow.
 * Source: dm2_v1_creature.h DM2_AI_MERCHANT=33
 *         skproject/SKULLWIN/c_creature.cpp:2930-3212 (b_1a 0x17/0x18) */
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
