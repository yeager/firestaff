/* skproject/SKWIN/SkWinCore.cpp: map-load GRAPHICSSET selection 44929-45030,
 * DRAW_DOOR_FRAMES 46311-46334, and floor/ceiling DRAW_DUNGEON_GRAPHIC
 * 48008-48027. Data-free address/palette regression only. */
#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>
#include <string.h>

static int failures;
static int reject_frame_fetch;
static int reject_hud_top_bar_fetch;
static int reject_hud_status_panel_fetch;
static int reject_hud_hand_action_fetch;
static int reject_creature_fetch;
static int reject_item_fetch;
static int invalid_u4_pixels;
static const uint8_t frame_pixels[4] = { 6u, 5u, 5u, 5u };
static const uint8_t invalid_frame_pixels[4] = { 6u, 0x10u, 5u, 5u };

static const uint8_t *packed_frame_pixels(void)
{
    return invalid_u4_pixels ? invalid_frame_pixels : frame_pixels;
}

static void check(int condition, const char *label)
{
    if (condition) {
        fprintf(stderr, "PASS: %s\n", label);
    } else {
        fprintf(stderr, "FAIL: %s\n", label);
        ++failures;
    }
}

static int packed_asset_fetch(void *user, int gdat_index,
                              const uint8_t **out_pixels, int *out_w,
                              int *out_h, int *out_stride)
{
    static const uint8_t floor_pixels[4] = { 3u, 3u, 3u, 3u };
    static const uint8_t ceiling_pixels[4] = { 4u, 4u, 4u, 4u };
    static const uint8_t wall_pixels[4] = { 6u, 5u, 5u, 5u };
    static const uint8_t creature_pixels[4] = { 10u, 9u, 9u, 9u };
    static const uint8_t item_pixels[4] = { 10u, 6u, 6u, 6u };
    static const uint8_t hud_top_bar_pixels[4] = { 10u, 7u, 7u, 7u };
    static const uint8_t hud_status_panel_pixels[4] = { 10u, 8u, 8u, 8u };
    static const uint8_t hud_hand_action_pixels[4] = { 10u, 9u, 9u, 9u };
    int set = 0;
    int field = 0;

    (void)user;
    if (out_pixels) *out_pixels = NULL;
    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    if (out_stride) *out_stride = 0;

    if (dm2_v1_viewport_scene_material_graphic_address(gdat_index, &set,
                                                        &field)) {
        if (set != 3) return -1;
        if (field == DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_FLOOR) {
            if (out_pixels) *out_pixels = floor_pixels;
        } else if (field == DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_CEILING) {
            if (out_pixels) *out_pixels = ceiling_pixels;
        } else {
            return -1;
        }
    } else if (dm2_v1_viewport_wall_graphic_address(gdat_index, &set,
                                                     &field)) {
        if (set != 3 || field !=
            dm2_v1_viewport_wall_field_for_square(DM2_SQ_D0L)) {
            return -1;
        }
        if (out_pixels) *out_pixels = wall_pixels;
    } else if (gdat_index == dm2_v1_viewport_creature_graphic_index(7, 0)) {
        if (reject_creature_fetch) return -1;
        if (out_pixels) *out_pixels = creature_pixels;
    } else if (gdat_index == dm2_v1_viewport_item_graphic_index(
                   DM2_GDAT_CATEGORY_WEAPONS, 4, 0)) {
        if (reject_item_fetch) return -1;
        if (out_pixels) *out_pixels = item_pixels;
    } else if (gdat_index == dm2_v1_viewport_hud_core_graphic_index(
                   DM2_V1_VIEWPORT_GFX_HUD_CORE_TOP_BAR)) {
        if (reject_hud_top_bar_fetch) return -1;
        if (out_pixels) *out_pixels = hud_top_bar_pixels;
    } else if (gdat_index == dm2_v1_viewport_hud_core_graphic_index(
                   DM2_V1_VIEWPORT_GFX_HUD_CORE_PORTRAIT_PANEL)) {
        if (reject_hud_status_panel_fetch) return -1;
        if (out_pixels) *out_pixels = hud_status_panel_pixels;
    } else if (dm2_v1_viewport_hud_hand_action_graphic_address(
                   gdat_index, NULL, NULL, NULL)) {
        if (reject_hud_hand_action_fetch) return -1;
        if (out_pixels) *out_pixels = hud_hand_action_pixels;
    } else if (gdat_index ==
               dm2_v1_viewport_door_frame_graphic_index_for_graphicsset(
                   3, DM2_SQ_D0C)) {
        if (reject_frame_fetch) return -1;
        if (out_pixels) *out_pixels = packed_frame_pixels();
    } else {
        return -1;
    }
    if (out_w) *out_w = 2;
    if (out_h) *out_h = 2;
    if (out_stride) *out_stride = 2;
    return 0;
}

static int packed_door_surface_view(void *user, int gdat_index,
                                    DM2_V1_BootViewportSurfaceView *out_view)
{
    (void)user;
    if (!out_view || gdat_index !=
        dm2_v1_viewport_door_frame_graphic_index_for_graphicsset(
            3, DM2_SQ_D0C)) {
        return 0;
    }
    memset(out_view, 0, sizeof(*out_view));
    out_view->pixels = packed_frame_pixels();
    out_view->width = 2;
    out_view->height = 2;
    out_view->stride = 2;
    out_view->format = DM2_IMG_FMT_U4;
    return 1;
}

int main(void)
{
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    uint8_t before_doors[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    uint8_t blocked_framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    uint8_t palette16[16];
    DM2_V1_ViewportState viewport;
    DM2_V1_OriginalMaterialGateReceipt gate;
    DM2_V1_OriginalDoorSurfaceRequest surface_request;
    DM2_V1_OriginalDoorSurfaceBinding surface_binding;
    DM2_V1_OriginalDoorOpeningFrameRequest opening_request;
    DM2_V1_ViewportDoorPresentationCommand presentation_command;
    DM2_V1_ViewportDungeonMaterialCommand ceiling_command;
    DM2_V1_ViewportDungeonMaterialCommand floor_command;
    DM2_V1_ViewportDungeonMaterialCommand wall_command;
    DM2_V1_ViewportSceneControlCommand scene_control_command;
    DM2_V1_ViewportCreatureMaterialCommand creature_command;
    DM2_V1_ViewportItemMaterialCommand item_command;
    DM2_V1_ViewportM11FrameReceipt m11_frame;
    DM2_V1_ViewportHudPresentationCommand hand_action_command;
    DM2_V1_ViewportFrameCompositionReceipt composition;
    DM2_V1_HudHandActionSource hand_action_source;
    DM2_V1_HudPartyState hand_action_party;
    const DM2_WallFrame *door_frame =
        dm2_v1_get_wall_frame(DM2_SQ_D0C);
    int set = -1;
    int field = -1;
    int hand_entry = -1;
    int wall_field = dm2_v1_viewport_wall_field_for_square(DM2_SQ_D0L);
    int floor_index = dm2_v1_viewport_scene_material_graphic_index(3,
        DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_FLOOR);
    int ceiling_index = dm2_v1_viewport_scene_material_graphic_index(3,
        DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_CEILING);
    int wall_index = dm2_v1_viewport_wall_graphic_index_for_graphicsset(
        3, DM2_SQ_D0L);
    int creature_index = dm2_v1_viewport_creature_graphic_index(7, 0);
    int item_index = dm2_v1_viewport_item_graphic_index(
        DM2_GDAT_CATEGORY_WEAPONS, 4, 0);

    check(dm2_v1_viewport_scene_material_graphic_address(
              floor_index, &set, &field) == 1 && set == 3 && field == 0,
          "active GRAPHICSSET packs the source floor address");
    check(dm2_v1_viewport_scene_material_graphic_address(
              ceiling_index, &set, &field) == 1 && set == 3 && field == 1,
          "active GRAPHICSSET packs the source ceiling address");
    check(dm2_v1_viewport_door_frame_graphic_index_for_graphicsset(
              3, DM2_SQ_D0C) ==
              DM2_V1_VIEWPORT_GFX_DOOR_FRAME_GRAPHICSSET_BASE -
                  (3 << 8) - DM2_V1_VIEWPORT_GFX_DOOR_FRAME_FRONT &&
              dm2_v1_viewport_door_frame_graphic_index_for_graphicsset(
                  -1, DM2_SQ_D0C) == 0 &&
              dm2_v1_viewport_door_frame_graphic_index_for_graphicsset(
                  256, DM2_SQ_D0C) == 0,
          "door-frame packing rejects malformed GRAPHICSSET ranges");
    check(dm2_v1_viewport_scene_material_graphic_address(
              DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_BASE - 0x10000,
              &set, &field) == 0,
          "scene-material decoder rejects malformed packed range");
    check(wall_field >= DM2_V1_VIEWPORT_GFX_WALL_FIELD_FIRST &&
              dm2_v1_viewport_wall_graphic_address(wall_index, &set, &field) &&
              set == 3 && field == wall_field,
          "active GRAPHICSSET packs the source D0L wall address");
    check(dm2_v1_viewport_hud_hand_action_graphic_index(1, 1) ==
              DM2_V1_VIEWPORT_GFX_HUD_HAND_ACTION_BASE - 5 &&
              dm2_v1_viewport_hud_hand_action_graphic_address(
                  DM2_V1_VIEWPORT_GFX_HUD_HAND_ACTION_BASE - 5,
                  &set, &field, &hand_entry) &&
              set == 1 && field == 1 && hand_entry == 5 &&
              dm2_v1_viewport_hud_hand_action_graphic_index(2, 0) == 0 &&
              !dm2_v1_viewport_hud_hand_action_graphic_address(
                  DM2_V1_VIEWPORT_GFX_HUD_HAND_ACTION_BASE - 1,
                  NULL, NULL, NULL),
          "hand-action GDAT packing keeps the skproject entry route bounded");

    for (int i = 0; i < 16; ++i) palette16[i] = (uint8_t)(0x80 + i);
    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_asset_provider(&viewport, packed_asset_fetch, NULL);
    dm2_v1_viewport_set_door_surface_view_provider(
        &viewport, packed_door_surface_view, NULL);
    dm2_v1_viewport_set_scene_map_load_token(&viewport, 42u);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, 3, 0x53434e45u, 6u, 0u, 0u, 0u, 0u, 0u,
        0u, 0u, 0u, 0u);
    dm2_v1_viewport_set_gdat_interface_palette(
        &viewport, 1, 0x50414c31u, palette16);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    memset(&hand_action_party, 0, sizeof(hand_action_party));
    hand_action_party.champion_count = 1;
    hand_action_party.leader_index = 0;
    hand_action_party.champions[0].occupied = 1;
    dm2_v1_viewport_set_hud_party(&viewport, &hand_action_party);
    hand_action_source = (DM2_V1_HudHandActionSource){
        .valid = 1,
        .player_index = 0u,
        .possession_index = 1u,
        .left_or_right = 1u,
        .player_position = 2u,
        .party_direction = 0u,
        .gdat_category = DM2_GDAT_CATEGORY_INTERFACE_GENERAL,
        .gdat_subcategory = 4u,
        .gdat_entry = 5u,
        .rectno = 0x48u,
        .map_load_token = 42u,
        .scene_control_hash = 0x53434e45u,
        .palette_hash = 0x50414c31u,
        .raw4_hash = 0x52415734u,
        .source_rect = { 0, 0, 2, 2 },
        .destination_rect = { 24, 170, 2, 2 }
    };
    dm2_v1_viewport_set_hud_hand_action_source(&viewport, &hand_action_source);
    check(dm2_v1_viewport_bind_static_graphicsset_scene_record(
              &viewport, 42u, 0x53434e45u) == 1,
          "active GRAPHICSSET frame keeps its source scene record");
    check(dm2_v1_viewport_bind_static_scene_light_control(
              &viewport, 42u, 0x53434e45u) == 1 &&
              dm2_v1_viewport_bind_static_scene_floor_material(
                  &viewport, 42u, 0x53434e45u) == 1 &&
              dm2_v1_viewport_bind_static_scene_ceiling_material(
                  &viewport, 42u, 0x53434e45u) == 1,
          "floor and ceiling passes require matching scene light ownership");
    check(dm2_v1_viewport_bind_static_scene_wall_material(
              &viewport, 42u, 0x53434e45u, DM2_SQ_D0L) == 1,
          "wall pass requires a matching source D0L panel ownership");
    viewport.squares[DM2_SQ_D0C].flags |= DM2_SQF_HAS_DOOR;
    viewport.squares[DM2_SQ_D0C].door_open_pct = 50u;
    dm2_v1_render_floor_ceiling(&viewport);
    dm2_v1_render_walls(&viewport);
    memcpy(before_doors, framebuffer, sizeof(framebuffer));
    dm2_v1_render_doors(&viewport);
    check(viewport.asset_floor_ceiling_drawn_count == 2 &&
              viewport.asset_door_frame_drawn_count == 1 &&
              viewport.gdat_material_palette_floor_ceiling_consumed_count > 0 &&
              viewport.gdat_material_palette_door_frame_consumed_count > 0 &&
              viewport.last_door_frame_asset_blit_valid &&
              framebuffer[0] == palette16[4],
          "active packed GDAT materials reach palette-bound dungeon surfaces");
    check(dm2_v1_viewport_last_dungeon_ceiling_presentation_command(
              &viewport, &ceiling_command) == 1 && ceiling_command.valid &&
              ceiling_command.gdat_category == DM2_GDAT_CATEGORY_GRAPHICSSET &&
              ceiling_command.graphicsset == 3u &&
              ceiling_command.field == DM2_GDAT_GFXSET_CEIL &&
              ceiling_command.gdat_index == ceiling_index &&
              ceiling_command.map_load_token == 42u &&
              ceiling_command.scene_control_hash == 0x53434e45u &&
              ceiling_command.palette_hash == 0x50414c31u &&
              ceiling_command.palette16 == viewport.gdat_interface_palette16 &&
              ceiling_command.palette_stride == 16 &&
              ceiling_command.transparent_color == -1 &&
              ceiling_command.source_rect.w == 2 &&
              ceiling_command.source_rect.h == 2 &&
              ceiling_command.destination_rect.x == 0 &&
              ceiling_command.destination_rect.y == 0 &&
              ceiling_command.destination_rect.w == DM2_VP_WIDTH &&
              ceiling_command.destination_rect.h == 29,
          "indoor ceiling publishes its owned GDAT palette command");
    check(dm2_v1_viewport_last_dungeon_floor_presentation_command(
              &viewport, &floor_command) == 1 && floor_command.valid &&
              floor_command.gdat_category == DM2_GDAT_CATEGORY_GRAPHICSSET &&
              floor_command.graphicsset == 3u &&
              floor_command.field == DM2_GDAT_GFXSET_FLOOR &&
              floor_command.gdat_index == floor_index &&
              floor_command.map_load_token == 42u &&
              floor_command.scene_control_hash == 0x53434e45u &&
              floor_command.palette_hash == 0x50414c31u &&
              floor_command.palette16 == viewport.gdat_interface_palette16 &&
              floor_command.palette_stride == 16 &&
              floor_command.source_rect.w == 2 &&
              floor_command.source_rect.h == 2 &&
              floor_command.destination_rect.x == 0 &&
              floor_command.destination_rect.y == 66 &&
              floor_command.destination_rect.w == DM2_VP_WIDTH &&
              floor_command.destination_rect.h == 70,
          "indoor floor publishes its owned GDAT palette command");
    check(dm2_v1_viewport_last_dungeon_wall_presentation_command(
              &viewport, &wall_command) == 1 && wall_command.valid &&
              wall_command.gdat_category == DM2_GDAT_CATEGORY_GRAPHICSSET &&
              wall_command.graphicsset == 3u && wall_command.field == wall_field &&
              wall_command.gdat_index == wall_index &&
              wall_command.map_load_token == 42u &&
              wall_command.scene_control_hash == 0x53434e45u &&
              wall_command.palette_hash == 0x50414c31u &&
              wall_command.palette16 == viewport.gdat_interface_palette16 &&
              wall_command.palette_stride == 16 &&
              wall_command.transparent_color == 6 &&
              wall_command.colorkey_palette_index == palette16[6] &&
              wall_command.source_rect.w == 2 && wall_command.source_rect.h == 2 &&
              wall_command.destination_rect.w > 0 &&
              wall_command.destination_rect.h > 0,
          "indoor wall publishes its owned GDAT palette command");
    check(viewport.last_door_frame_asset_blit.transparent_color == 6 &&
              framebuffer[viewport.last_door_frame_asset_blit.dst_rect.y *
                              DM2_VP_WIDTH +
                          viewport.last_door_frame_asset_blit.dst_rect.x] ==
                  before_doors[viewport.last_door_frame_asset_blit.dst_rect.y *
                                   DM2_VP_WIDTH +
                               viewport.last_door_frame_asset_blit.dst_rect.x] &&
              framebuffer[(viewport.last_door_frame_asset_blit.dst_rect.y +
                           viewport.last_door_frame_asset_blit.dst_rect.h / 2) *
                              DM2_VP_WIDTH +
                          viewport.last_door_frame_asset_blit.dst_rect.x +
                          viewport.last_door_frame_asset_blit.dst_rect.w / 2] ==
                  palette16[5],
          "door frame preserves source colorkey and palette-maps opaque pixels");
    check(dm2_v1_viewport_last_original_material_gate_receipt(
              &viewport, &gate) == 1 && gate.valid &&
              gate.original_material_required &&
              gate.active_graphicsset_frame &&
              gate.decoded_format_gate_ready && gate.scene_record_ready &&
              gate.palette_ready && gate.colorkey_ready && gate.accepted &&
              gate.scene_colorkey == 6u,
          "door material receipt exposes proven decode, scene, palette and colorkey");
    check(dm2_v1_viewport_last_original_door_surface_request(
              &viewport, &surface_request) == 1 && surface_request.valid &&
              surface_request.gdat_category == DM2_GDAT_CATEGORY_GRAPHICSSET &&
              surface_request.graphicsset == 3u &&
              surface_request.field == DM2_V1_VIEWPORT_GFX_DOOR_FRAME_FRONT &&
              surface_request.gdat_index ==
                  dm2_v1_viewport_door_frame_graphic_index_for_graphicsset(
                      3, DM2_SQ_D0C) &&
              surface_request.map_load_token == 42u &&
              surface_request.scene_control_hash == 0x53434e45u &&
              surface_request.palette_hash == 0x50414c31u &&
              surface_request.scene_colorkey == 6u &&
              surface_request.decoded_pixels == frame_pixels &&
              surface_request.palette16 == viewport.gdat_interface_palette16 &&
              surface_request.width == 2 && surface_request.height == 2 &&
              surface_request.stride == 2 &&
              surface_request.format == DM2_IMG_FMT_U4,
          "accepted door material publishes a typed original surface request");
    check(dm2_v1_viewport_last_original_door_surface_binding(
              &viewport, &surface_binding) == 1 && surface_binding.valid &&
              surface_binding.request.gdat_index == surface_request.gdat_index &&
              surface_binding.blit.gdat_index == surface_request.gdat_index &&
              surface_binding.blit.src_rect.x == 0 &&
              surface_binding.blit.src_rect.y == 0 &&
              surface_binding.blit.src_rect.w == surface_request.width &&
              surface_binding.blit.src_rect.h == surface_request.height &&
              surface_binding.blit.src_stride == surface_request.stride &&
              surface_binding.blit.transparent_color ==
                  (int)surface_request.scene_colorkey &&
              surface_binding.palette_stride == 16 &&
              surface_binding.colorkey_palette_index == palette16[6],
          "door binding preserves GDAT geometry, palette stride and colorkey");
    check(dm2_v1_viewport_last_original_door_opening_frame_request(
              &viewport, &opening_request) == 1 && opening_request.valid &&
              opening_request.view_square == DM2_SQ_D0C &&
              opening_request.skproject_cell ==
                  dm2_v1_viewport_skproject_cell_for_square(DM2_SQ_D0C) &&
              opening_request.door_open_pct == 50u &&
              opening_request.opening_visible_rect.w > 0 &&
              opening_request.opening_visible_rect.h > 0 &&
              opening_request.frame_rect.x == surface_binding.blit.dst_rect.x &&
              opening_request.frame_rect.y == surface_binding.blit.dst_rect.y &&
              opening_request.frame_rect.w == surface_binding.blit.dst_rect.w &&
              opening_request.frame_rect.h == surface_binding.blit.dst_rect.h &&
              opening_request.source_rect.w == surface_request.width &&
              opening_request.source_rect.h == surface_request.height &&
              opening_request.destination_rect.x == surface_binding.blit.dst_rect.x &&
              opening_request.destination_rect.y == surface_binding.blit.dst_rect.y &&
              opening_request.destination_rect.w == surface_binding.blit.dst_rect.w &&
              opening_request.destination_rect.h == surface_binding.blit.dst_rect.h,
          "half-open door publishes source frame geometry and destination");
    check(dm2_v1_viewport_last_original_door_presentation_command(
              &viewport, &presentation_command) == 1 &&
              presentation_command.valid &&
              presentation_command.opening_frame.valid &&
              presentation_command.u4_pixels == frame_pixels &&
              presentation_command.palette16 == viewport.gdat_interface_palette16 &&
              presentation_command.palette_stride == 16 &&
              presentation_command.scene_colorkey == 6u &&
              presentation_command.colorkey_palette_index == palette16[6] &&
              presentation_command.format == DM2_IMG_FMT_U4 &&
              presentation_command.source_rect.x == 0 &&
              presentation_command.source_rect.y == 0 &&
              presentation_command.source_rect.w == 2 &&
              presentation_command.source_rect.h == 2 &&
              presentation_command.destination_rect.x ==
                  surface_binding.blit.dst_rect.x &&
              presentation_command.destination_rect.y ==
                  surface_binding.blit.dst_rect.y &&
              presentation_command.destination_rect.w ==
                  surface_binding.blit.dst_rect.w &&
              presentation_command.destination_rect.h ==
                  surface_binding.blit.dst_rect.h,
          "viewport presentation consumes the typed U4 door command");

    memset(blocked_framebuffer, 0x3au, sizeof(blocked_framebuffer));
    dm2_v1_viewport_init(&viewport, blocked_framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_asset_provider(&viewport, packed_asset_fetch, NULL);
    dm2_v1_viewport_set_scene_map_load_token(&viewport, 42u);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, 3, 0x53434e45u, 6u, 0u, 0u, 0u, 0u, 0u,
        0u, 0u, 0u, 0u);
    dm2_v1_viewport_set_gdat_interface_palette(
        &viewport, 1, 0x50414c31u, palette16);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    (void)dm2_v1_viewport_bind_static_graphicsset_scene_record(
        &viewport, 42u, 0x53434e45u);
    (void)dm2_v1_viewport_bind_static_scene_light_control(
        &viewport, 42u, 0x53434e45u);
    dm2_v1_render_floor_ceiling(&viewport);
    check((viewport.blocked_material_mask &
           DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING) != 0u &&
              dm2_v1_viewport_last_dungeon_floor_presentation_command(
                  &viewport, &floor_command) == 0 && !floor_command.valid &&
              blocked_framebuffer[66 * DM2_VP_WIDTH] == 0x3au,
          "source-required floor pass fails closed without FLOOR ownership");

    memset(blocked_framebuffer, 0x3au, sizeof(blocked_framebuffer));
    dm2_v1_viewport_init(&viewport, blocked_framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_asset_provider(&viewport, packed_asset_fetch, NULL);
    dm2_v1_viewport_set_scene_map_load_token(&viewport, 42u);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, 3, 0x53434e45u, 6u, 0u, 0u, 0u, 0u, 0u,
        0u, 0u, 0u, 0u);
    dm2_v1_viewport_set_gdat_interface_palette(
        &viewport, 1, 0x50414c31u, palette16);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    (void)dm2_v1_viewport_bind_static_graphicsset_scene_record(
        &viewport, 42u, 0x53434e45u);
    (void)dm2_v1_viewport_bind_static_scene_light_control(
        &viewport, 42u, 0x53434e45u);
    (void)dm2_v1_viewport_bind_static_scene_floor_material(
        &viewport, 42u, 0x53434e45u);
    dm2_v1_render_floor_ceiling(&viewport);
    check((viewport.blocked_material_mask &
           DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING) != 0u &&
              dm2_v1_viewport_last_dungeon_ceiling_presentation_command(
                  &viewport, &ceiling_command) == 0 && !ceiling_command.valid &&
              blocked_framebuffer[0] == 0x3au,
          "source-required ceiling pass fails closed without CEIL ownership");

    memset(blocked_framebuffer, 0x3au, sizeof(blocked_framebuffer));
    dm2_v1_viewport_init(&viewport, blocked_framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_asset_provider(&viewport, packed_asset_fetch, NULL);
    dm2_v1_viewport_set_scene_map_load_token(&viewport, 42u);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, 3, 0x53434e45u, 6u, 0u, 0u, 0u, 0u, 0u,
        0u, 0u, 0u, 0u);
    dm2_v1_viewport_set_gdat_interface_palette(
        &viewport, 1, 0x50414c31u, palette16);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    (void)dm2_v1_viewport_bind_static_graphicsset_scene_record(
        &viewport, 42u, 0x53434e45u);
    (void)dm2_v1_viewport_bind_static_scene_light_control(
        &viewport, 42u, 0x53434e45u);
    dm2_v1_render_walls(&viewport);
    check((viewport.blocked_material_mask &
           DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL) != 0u &&
              dm2_v1_viewport_last_dungeon_wall_presentation_command(
                  &viewport, &wall_command) == 0 && !wall_command.valid &&
              door_frame &&
              blocked_framebuffer[door_frame->top_y * DM2_VP_WIDTH +
                                  door_frame->left_x] == 0x3au,
          "source-required wall pass fails closed without WALL ownership");

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_asset_provider(&viewport, packed_asset_fetch, NULL);
    dm2_v1_viewport_set_door_surface_view_provider(
        &viewport, packed_door_surface_view, NULL);
    dm2_v1_viewport_set_scene_map_load_token(&viewport, 42u);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, 3, 0x53434e45u, 6u, 0u, 0u, 0u, 0u, 0u,
        0u, 0u, 0u, 0u);
    dm2_v1_viewport_set_gdat_interface_palette(
        &viewport, 1, 0x50414c31u, palette16);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    memset(&hand_action_party, 0, sizeof(hand_action_party));
    hand_action_party.champion_count = 1;
    hand_action_party.leader_index = 0;
    hand_action_party.champions[0].occupied = 1;
    dm2_v1_viewport_set_hud_party(&viewport, &hand_action_party);
    hand_action_source = (DM2_V1_HudHandActionSource){
        .valid = 1,
        .player_index = 0u,
        .possession_index = 1u,
        .left_or_right = 1u,
        .player_position = 2u,
        .party_direction = 0u,
        .gdat_category = DM2_GDAT_CATEGORY_INTERFACE_GENERAL,
        .gdat_subcategory = 4u,
        .gdat_entry = 5u,
        .rectno = 0x48u,
        .map_load_token = 42u,
        .scene_control_hash = 0x53434e45u,
        .palette_hash = 0x50414c31u,
        .raw4_hash = 0x52415734u,
        .source_rect = { 0, 0, 2, 2 },
        .destination_rect = { 24, 170, 2, 2 }
    };
    dm2_v1_viewport_set_hud_hand_action_source(&viewport, &hand_action_source);
    (void)dm2_v1_viewport_bind_static_graphicsset_scene_record(
        &viewport, 42u, 0x53434e45u);
    (void)dm2_v1_viewport_bind_static_scene_light_control(
        &viewport, 42u, 0x53434e45u);
    (void)dm2_v1_viewport_bind_static_scene_floor_material(
        &viewport, 42u, 0x53434e45u);
    (void)dm2_v1_viewport_bind_static_scene_ceiling_material(
        &viewport, 42u, 0x53434e45u);
    (void)dm2_v1_viewport_bind_static_scene_wall_material(
        &viewport, 42u, 0x53434e45u, DM2_SQ_D0L);
    (void)dm2_v1_viewport_bind_static_scene_ambient_light_control(
        &viewport, 42u, 0x53434e45u);
    (void)dm2_v1_viewport_bind_static_scene_ambient_darkness_control(
        &viewport, 42u, 0x53434e45u);
    (void)dm2_v1_viewport_bind_static_scene_flags_control(
        &viewport, 42u, 0x53434e45u);
    (void)dm2_v1_viewport_bind_static_scene_colorkey_control(
        &viewport, 42u, 0x53434e45u);
    viewport.creature_count = 1;
    viewport.creatures[0] = (DM2_CreatureSprite){
        .creature_type = 7u,
        .frame_index = 0u,
        .depth = 0,
        .screen_x = 120,
        .screen_y = 100,
        .health_pct = 100u,
        .direction = 0u,
        .source_kind = 1u
    };
    viewport.item_count = 1;
    viewport.items[0] = (DM2_ItemSprite){
        .item_category = DM2_GDAT_CATEGORY_WEAPONS,
        .item_type = 4u,
        .frame_index = 0u,
        .depth = 0,
        .screen_x = 140,
        .screen_y = 110,
        .direction = 0u
    };
    viewport.squares[DM2_SQ_D0C].flags |= DM2_SQF_HAS_DOOR;
    dm2_v1_viewport_render(&viewport);
    check(dm2_v1_viewport_last_frame_composition_receipt(
              &viewport, &composition) == 1 && composition.valid &&
              composition.indoor_viewport && composition.scene_record_owned &&
              composition.scene_light_owned && composition.palette_owned &&
              composition.map_load_token == 42u &&
              composition.scene_control_hash == 0x53434e45u &&
              composition.palette_hash == 0x50414c31u &&
              composition.door_command_consumed &&
              composition.door_presentation_stage == 4 &&
              composition.dungeon_ceiling_presentation_stage == 1 &&
              composition.dungeon_floor_presentation_stage == 2 &&
              composition.dungeon_wall_presentation_stage == 3 &&
              composition.scene_control_presentation_stage == 5 &&
              composition.hud_presentation_stage == 11 &&
              composition.dungeon_floor_presentation_stage <
                  composition.hud_presentation_stage &&
              composition.dungeon_ceiling_presentation_stage <
                  composition.dungeon_floor_presentation_stage &&
              composition.dungeon_ceiling_command_consumed &&
              composition.dungeon_ceiling_command.valid &&
              composition.dungeon_ceiling_command.gdat_category ==
                  DM2_GDAT_CATEGORY_GRAPHICSSET &&
              composition.dungeon_ceiling_command.graphicsset == 3u &&
              composition.dungeon_ceiling_command.field == DM2_GDAT_GFXSET_CEIL &&
              composition.dungeon_ceiling_command.gdat_index == ceiling_index &&
              composition.dungeon_ceiling_command.map_load_token == 42u &&
              composition.dungeon_ceiling_command.scene_control_hash ==
                  0x53434e45u &&
              composition.dungeon_ceiling_command.palette_hash == 0x50414c31u &&
              composition.dungeon_ceiling_command.transparent_color == -1 &&
              composition.dungeon_wall_presentation_stage <
                  composition.hud_presentation_stage &&
              composition.scene_control_presentation_stage <
                  composition.hud_presentation_stage &&
              composition.dungeon_floor_command_consumed &&
              composition.dungeon_floor_command.valid &&
              composition.dungeon_floor_command.gdat_category ==
                  DM2_GDAT_CATEGORY_GRAPHICSSET &&
              composition.dungeon_floor_command.graphicsset == 3u &&
              composition.dungeon_floor_command.field == DM2_GDAT_GFXSET_FLOOR &&
              composition.dungeon_floor_command.map_load_token == 42u &&
              composition.dungeon_floor_command.scene_control_hash ==
                  0x53434e45u &&
              composition.dungeon_floor_command.palette_hash == 0x50414c31u &&
              composition.dungeon_wall_command_consumed &&
              composition.dungeon_wall_command.valid &&
              composition.dungeon_wall_command.gdat_category ==
                  DM2_GDAT_CATEGORY_GRAPHICSSET &&
              composition.dungeon_wall_command.graphicsset == 3u &&
              composition.dungeon_wall_command.field == wall_field &&
              composition.dungeon_wall_command.gdat_index == wall_index &&
              composition.dungeon_wall_command.map_load_token == 42u &&
              composition.dungeon_wall_command.scene_control_hash ==
                  0x53434e45u &&
              composition.dungeon_wall_command.palette_hash == 0x50414c31u &&
              composition.dungeon_wall_command.transparent_color == 6 &&
              composition.dungeon_wall_command.colorkey_palette_index ==
                  palette16[6] &&
              composition.scene_control_command_consumed &&
              composition.scene_control_command.valid &&
              composition.scene_control_command.gdat_category ==
                  DM2_GDAT_CATEGORY_GRAPHICSSET &&
              composition.scene_control_command.graphicsset == 3u &&
              composition.scene_control_command.field ==
                  DM2_GDAT_GFXSET_AMBIANT_DARKNESS &&
              composition.scene_control_command.map_load_token == 42u &&
              composition.scene_control_command.scene_control_hash ==
                  0x53434e45u &&
              composition.scene_control_command.palette_hash == 0x50414c31u &&
              composition.scene_control_command.scene_colorkey == 6u &&
              composition.scene_control_command.colorkey_palette_index ==
                  palette16[6] &&
              composition.scene_control_command.ambient_darkness == 0u &&
              !composition.scene_control_command.light_check_enabled &&
              composition.creature_presentation_stage == 6 &&
              composition.creature_command_consumed &&
              composition.creature_command.valid &&
              composition.creature_command.gdat_category ==
                  DM2_GDAT_CATEGORY_CREATURES &&
              composition.creature_command.creature_type == 7u &&
              composition.creature_command.field == DM2_GDAT_IMG_MAP_CHIP &&
              composition.creature_command.gdat_index == creature_index &&
              composition.creature_command.map_load_token == 42u &&
              composition.creature_command.scene_control_hash ==
                  0x53434e45u &&
              composition.creature_command.palette_hash == 0x50414c31u &&
              composition.creature_command.palette16 ==
                  viewport.gdat_interface_palette16 &&
              composition.creature_command.palette_stride == 16 &&
              composition.creature_command.transparent_color == 10 &&
              composition.creature_command.colorkey_palette_index ==
                  palette16[10] &&
              composition.creature_command.source_rect.w == 2 &&
              composition.creature_command.source_rect.h == 2 &&
              composition.creature_command.destination_rect.w > 0 &&
              composition.creature_command.destination_rect.h > 0 &&
              composition.scene_control_presentation_stage <
                  composition.creature_presentation_stage &&
              composition.creature_presentation_stage <
                  composition.item_presentation_stage &&
              composition.item_presentation_stage == 7 &&
              composition.item_command_consumed &&
              composition.item_command.valid &&
              composition.item_command.gdat_category ==
                  DM2_GDAT_CATEGORY_WEAPONS &&
              composition.item_command.item_type == 4u &&
              composition.item_command.field == DM2_GDAT_IMG_MAP_CHIP &&
              composition.item_command.gdat_index == item_index &&
              composition.item_command.map_load_token == 42u &&
              composition.item_command.scene_control_hash == 0x53434e45u &&
              composition.item_command.palette_hash == 0x50414c31u &&
              composition.item_command.palette16 ==
                  viewport.gdat_interface_palette16 &&
              composition.item_command.palette_stride == 16 &&
              composition.item_command.transparent_color == 10 &&
              composition.item_command.colorkey_palette_index ==
                  palette16[10] &&
              composition.item_command.source_rect.w == 2 &&
              composition.item_command.source_rect.h == 2 &&
              composition.item_command.destination_rect.w > 0 &&
              composition.item_command.destination_rect.h > 0 &&
              composition.item_presentation_stage <
                  composition.hud_presentation_stage &&
              composition.door_presentation_stage <
                  composition.hud_presentation_stage &&
              composition.door_command.valid &&
              composition.door_command.format == DM2_IMG_FMT_U4 &&
              composition.hud_top_bar_material_consumed &&
              composition.hud_top_bar_request.valid &&
              composition.hud_top_bar_request.gdat_index ==
                  dm2_v1_viewport_hud_core_graphic_index(
                      DM2_V1_VIEWPORT_GFX_HUD_CORE_TOP_BAR) &&
              composition.hud_top_bar_request.field ==
                  DM2_V1_VIEWPORT_GFX_HUD_CORE_TOP_BAR &&
              composition.hud_top_bar_request.palette16 ==
                  viewport.gdat_interface_palette16 &&
              composition.hud_top_bar_request.palette_hash == 0x50414c31u &&
              composition.hud_top_bar_request.palette_entry_count == 16 &&
              composition.hud_top_bar_request.transparent_color == 10 &&
              composition.hud_top_bar_request.colorkey_palette_index ==
                  palette16[10] &&
              composition.hud_top_bar_command_consumed &&
              composition.hud_top_bar_command.valid &&
              composition.hud_top_bar_command.indexed_pixels ==
                  composition.hud_top_bar_request.indexed_pixels &&
              composition.hud_top_bar_command.palette16 ==
                  composition.hud_top_bar_request.palette16 &&
              composition.hud_top_bar_command.transparent_color == 10 &&
              composition.hud_top_bar_command.source_rect.w == 2 &&
              composition.hud_top_bar_command.source_rect.h == 2 &&
              composition.hud_top_bar_command.destination_rect.w == DM2_VP_WIDTH &&
              composition.hud_top_bar_command.destination_rect.h ==
                  DM2_VP_CHROME_TOP &&
              composition.hud_status_panel_material_consumed &&
              composition.hud_status_panel_command_consumed &&
              composition.hud_status_panel_request.valid &&
              composition.hud_status_panel_request.field ==
                  DM2_V1_VIEWPORT_GFX_HUD_CORE_PORTRAIT_PANEL &&
              composition.hud_status_panel_request.palette16 ==
                  viewport.gdat_interface_palette16 &&
              composition.hud_status_panel_request.transparent_color == 10 &&
              composition.hud_status_panel_command.valid &&
              composition.hud_status_panel_command.material.field ==
                  DM2_V1_VIEWPORT_GFX_HUD_CORE_PORTRAIT_PANEL &&
              composition.hud_hand_action_command_consumed &&
              composition.hud_hand_action_request.valid &&
              composition.hud_hand_action_request.gdat_category ==
                  DM2_GDAT_CATEGORY_INTERFACE_GENERAL &&
              composition.hud_hand_action_request.gdat_subcategory == 4u &&
              composition.hud_hand_action_request.gdat_entry == 5u &&
              composition.hud_hand_action_request.field == 5u &&
              composition.hud_hand_action_request.palette16 ==
                  viewport.gdat_interface_palette16 &&
              composition.hud_hand_action_request.transparent_color == 10 &&
              composition.hud_hand_action_request.colorkey_palette_index ==
                  palette16[10] &&
              composition.hud_hand_action_command.valid &&
              composition.hud_hand_action_command.destination_rect.x == 24 &&
              composition.hud_hand_action_command.destination_rect.y == 170 &&
              composition.hud_hand_action_command.destination_rect.w == 2 &&
              composition.hud_hand_action_command.destination_rect.h == 2 &&
              composition.hud_top_bar_order == 1 &&
              composition.hud_status_panel_order == 2 &&
              composition.hud_hand_action_order == 3 &&
              framebuffer[170 * DM2_VP_WIDTH + 25] == palette16[9] &&
              framebuffer[0] != palette16[10] &&
              framebuffer[DM2_VP_WIDTH / 2] == palette16[7],
          "stage-11 hand action consumes its GDAT entry, palette, colorkey and rect");
    check(dm2_v1_viewport_last_hud_hand_action_presentation_command(
              &viewport, &hand_action_command) == 1 && hand_action_command.valid &&
              hand_action_command.material.gdat_entry == 5u &&
              hand_action_command.destination_rect.x == 24 &&
              hand_action_command.destination_rect.y == 170,
          "hand action publishes a typed stage-11 presentation command");
    check(dm2_v1_viewport_last_scene_control_presentation_command(
              &viewport, &scene_control_command) == 1 &&
              scene_control_command.valid &&
              scene_control_command.field == DM2_GDAT_GFXSET_AMBIANT_DARKNESS &&
              scene_control_command.scene_colorkey == 6u &&
              scene_control_command.ambient_light == 0u &&
              scene_control_command.highest_light_level == 0u &&
              scene_control_command.ambient_darkness == 0u &&
              scene_control_command.scene_flags == 0u &&
              !scene_control_command.outdoor_scene &&
              scene_control_command.colorkey_palette_index == palette16[6],
          "scene command carries verified GRAPHICSSET light and scene controls");
    check(dm2_v1_viewport_last_creature_presentation_command(
              &viewport, &creature_command) == 1 && creature_command.valid &&
              creature_command.gdat_index == creature_index &&
              creature_command.destination_rect.w > 0 &&
              creature_command.destination_rect.h > 0,
          "creature map-chip publishes a typed pre-HUD GDAT command");
    check(dm2_v1_viewport_last_item_presentation_command(
              &viewport, &item_command) == 1 && item_command.valid &&
              item_command.gdat_index == item_index &&
              item_command.gdat_category == DM2_GDAT_CATEGORY_WEAPONS &&
              item_command.item_type == 4u &&
              item_command.destination_rect.w > 0 &&
              item_command.destination_rect.h > 0,
          "floor item map-chip publishes a typed post-creature GDAT command");
    check(dm2_v1_viewport_last_m11_frame_receipt(&viewport, &m11_frame) == 1 &&
              m11_frame.valid && m11_frame.m11_consume_frame &&
              m11_frame.source_materials_required &&
              m11_frame.map_load_token == 42u &&
              m11_frame.scene_control_hash == 0x53434e45u &&
              m11_frame.palette_hash == 0x50414c31u &&
              m11_frame.composition.dungeon_ceiling_command_consumed &&
              m11_frame.composition.dungeon_floor_command_consumed &&
              m11_frame.composition.dungeon_wall_command_consumed &&
              m11_frame.composition.door_command_consumed &&
              m11_frame.composition.scene_control_command_consumed &&
              m11_frame.composition.creature_command_consumed &&
              m11_frame.composition.item_command_consumed &&
              m11_frame.composition.hud_top_bar_command_consumed &&
              m11_frame.composition.hud_status_panel_command_consumed &&
              m11_frame.composition.hud_hand_action_command_consumed,
          "M11 receives one atomic source-required GDAT frame receipt");
    viewport.gdat_static_ambient_light_control_owned = 0;
    dm2_v1_viewport_render(&viewport);
    check((viewport.blocked_material_mask &
           DM2_V1_VIEWPORT_BLOCKED_MATERIAL_SCENE_CONTROL) != 0u &&
              dm2_v1_viewport_last_scene_control_presentation_command(
                  &viewport, &scene_control_command) == 0 &&
              !scene_control_command.valid &&
              dm2_v1_viewport_last_m11_frame_receipt(
                  &viewport, &m11_frame) == 0 && !m11_frame.valid,
          "scene command fails closed without AMBIANT_LIGHT ownership");
    (void)dm2_v1_viewport_bind_static_scene_ambient_light_control(
        &viewport, 42u, 0x53434e45u);
    viewport.gdat_static_ambient_darkness_control_owned = 0;
    dm2_v1_viewport_render(&viewport);
    check((viewport.blocked_material_mask &
           DM2_V1_VIEWPORT_BLOCKED_MATERIAL_SCENE_CONTROL) != 0u &&
              dm2_v1_viewport_last_scene_control_presentation_command(
                  &viewport, &scene_control_command) == 0 &&
              !scene_control_command.valid &&
              dm2_v1_viewport_last_frame_composition_receipt(
                  &viewport, &composition) == 0 && !composition.valid,
          "scene light control fails closed without darkness ownership");
    (void)dm2_v1_viewport_bind_static_scene_ambient_darkness_control(
        &viewport, 42u, 0x53434e45u);
    reject_creature_fetch = 1;
    dm2_v1_viewport_render(&viewport);
    reject_creature_fetch = 0;
    check((viewport.blocked_material_mask &
           DM2_V1_VIEWPORT_BLOCKED_MATERIAL_CREATURE) != 0u &&
              dm2_v1_viewport_last_creature_presentation_command(
                  &viewport, &creature_command) == 0 && !creature_command.valid &&
              dm2_v1_viewport_last_frame_composition_receipt(
                  &viewport, &composition) == 0 && !composition.valid,
          "creature stage fails closed without its real GDAT payload");
    reject_item_fetch = 1;
    dm2_v1_viewport_render(&viewport);
    reject_item_fetch = 0;
    check((viewport.blocked_material_mask &
           DM2_V1_VIEWPORT_BLOCKED_MATERIAL_ITEM) != 0u &&
              dm2_v1_viewport_last_item_presentation_command(
                  &viewport, &item_command) == 0 && !item_command.valid &&
              dm2_v1_viewport_last_m11_frame_receipt(
                  &viewport, &m11_frame) == 0 && !m11_frame.valid &&
              dm2_v1_viewport_last_frame_composition_receipt(
                  &viewport, &composition) == 0 && !composition.valid,
          "floor item stage fails closed without its real GDAT payload");
    reject_hud_hand_action_fetch = 1;
    dm2_v1_viewport_render(&viewport);
    reject_hud_hand_action_fetch = 0;
    check((viewport.blocked_material_mask &
           DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_CORE) != 0u &&
              dm2_v1_viewport_last_hud_hand_action_presentation_command(
                  &viewport, &hand_action_command) == 0 &&
              !hand_action_command.valid &&
              dm2_v1_viewport_last_frame_composition_receipt(
                  &viewport, &composition) == 0 && !composition.valid,
          "stage-11 hand action fails closed when its GDAT entry is absent");
    hand_action_source.map_load_token = 43u;
    dm2_v1_viewport_set_hud_hand_action_source(&viewport, &hand_action_source);
    dm2_v1_viewport_render(&viewport);
    check(dm2_v1_viewport_last_hud_hand_action_presentation_command(
              &viewport, &hand_action_command) == 0 &&
              !hand_action_command.valid &&
              dm2_v1_viewport_last_frame_composition_receipt(
                  &viewport, &composition) == 0 && !composition.valid,
          "stale hand-action map token cannot enter indoor stage 11");
    hand_action_source.map_load_token = 42u;
    hand_action_source.scene_control_hash = 0x53434e46u;
    dm2_v1_viewport_set_hud_hand_action_source(&viewport, &hand_action_source);
    dm2_v1_viewport_render(&viewport);
    check(dm2_v1_viewport_last_hud_hand_action_presentation_command(
              &viewport, &hand_action_command) == 0 &&
              !hand_action_command.valid &&
              dm2_v1_viewport_last_frame_composition_receipt(
                  &viewport, &composition) == 0 && !composition.valid,
          "stale hand-action scene hash cannot enter indoor stage 11");
    hand_action_source.scene_control_hash = 0x53434e45u;
    hand_action_source.palette_hash = 0x50414c32u;
    dm2_v1_viewport_set_hud_hand_action_source(&viewport, &hand_action_source);
    dm2_v1_viewport_render(&viewport);
    check(dm2_v1_viewport_last_hud_hand_action_presentation_command(
              &viewport, &hand_action_command) == 0 &&
              !hand_action_command.valid &&
              dm2_v1_viewport_last_frame_composition_receipt(
                  &viewport, &composition) == 0 && !composition.valid,
          "stale hand-action palette cannot enter indoor stage 11");

    memset(blocked_framebuffer, 0, sizeof(blocked_framebuffer));
    dm2_v1_viewport_init(&viewport, blocked_framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_asset_provider(&viewport, packed_asset_fetch, NULL);
    dm2_v1_viewport_set_door_surface_view_provider(
        &viewport, packed_door_surface_view, NULL);
    dm2_v1_viewport_set_scene_map_load_token(&viewport, 42u);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, 3, 0x53434e45u, 6u, 0u, 0u, 0u, 0u, 0u,
        0u, 0u, 0u, 0u);
    dm2_v1_viewport_set_gdat_interface_palette(
        &viewport, 1, 0x50414c31u, palette16);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    (void)dm2_v1_viewport_bind_static_graphicsset_scene_record(
        &viewport, 42u, 0x53434e45u);
    (void)dm2_v1_viewport_bind_static_scene_light_control(
        &viewport, 42u, 0x53434e45u);
    viewport.squares[DM2_SQ_D0C].flags |= DM2_SQF_HAS_DOOR;
    reject_hud_top_bar_fetch = 1;
    dm2_v1_viewport_render(&viewport);
    reject_hud_top_bar_fetch = 0;
    check((viewport.blocked_material_mask &
           DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_CORE) != 0u &&
              dm2_v1_viewport_last_hud_top_bar_material_request(
                  &viewport, &composition.hud_top_bar_request) == 0 &&
              !composition.hud_top_bar_request.valid &&
              dm2_v1_viewport_last_hud_top_bar_presentation_command(
                  &viewport, &composition.hud_top_bar_command) == 0 &&
              !composition.hud_top_bar_command.valid &&
              dm2_v1_viewport_last_frame_composition_receipt(
                  &viewport, &composition) == 0 && !composition.valid,
          "frame composition fails closed without proven HUD top-bar material");

    memset(blocked_framebuffer, 0, sizeof(blocked_framebuffer));
    dm2_v1_viewport_init(&viewport, blocked_framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_asset_provider(&viewport, packed_asset_fetch, NULL);
    dm2_v1_viewport_set_door_surface_view_provider(
        &viewport, packed_door_surface_view, NULL);
    dm2_v1_viewport_set_scene_map_load_token(&viewport, 42u);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, 3, 0x53434e45u, 6u, 0u, 0u, 0u, 0u, 0u,
        0u, 0u, 0u, 0u);
    dm2_v1_viewport_set_gdat_interface_palette(
        &viewport, 1, 0x50414c31u, palette16);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    (void)dm2_v1_viewport_bind_static_graphicsset_scene_record(
        &viewport, 42u, 0x53434e45u);
    (void)dm2_v1_viewport_bind_static_scene_light_control(
        &viewport, 42u, 0x53434e45u);
    viewport.squares[DM2_SQ_D0C].flags |= DM2_SQF_HAS_DOOR;
    reject_hud_status_panel_fetch = 1;
    dm2_v1_viewport_render(&viewport);
    reject_hud_status_panel_fetch = 0;
    check((viewport.blocked_material_mask &
           DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_CORE) != 0u &&
              dm2_v1_viewport_last_hud_status_panel_material_request(
                  &viewport, &composition.hud_status_panel_request) == 0 &&
              !composition.hud_status_panel_request.valid &&
              dm2_v1_viewport_last_hud_status_panel_presentation_command(
                  &viewport, &composition.hud_status_panel_command) == 0 &&
              !composition.hud_status_panel_command.valid &&
              dm2_v1_viewport_last_frame_composition_receipt(
                  &viewport, &composition) == 0 && !composition.valid,
          "frame composition fails closed without proven HUD status panel");

    memset(blocked_framebuffer, 0, sizeof(blocked_framebuffer));
    dm2_v1_viewport_init(&viewport, blocked_framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_asset_provider(&viewport, packed_asset_fetch, NULL);
    dm2_v1_viewport_set_door_surface_view_provider(
        &viewport, packed_door_surface_view, NULL);
    dm2_v1_viewport_set_scene_map_load_token(&viewport, 42u);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, 3, 0x53434e45u, 6u, 0u, 0u, 0u, 0u, 0u,
        0u, 0u, 0u, 0u);
    dm2_v1_viewport_set_gdat_interface_palette(
        &viewport, 1, 0x50414c31u, palette16);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    (void)dm2_v1_viewport_bind_static_graphicsset_scene_record(
        &viewport, 42u, 0x53434e45u);
    viewport.squares[DM2_SQ_D0C].flags |= DM2_SQF_HAS_DOOR;
    dm2_v1_viewport_render(&viewport);
    check(viewport.asset_door_frame_drawn_count == 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR) != 0u &&
              dm2_v1_viewport_last_m11_frame_receipt(
                  &viewport, &m11_frame) == 0 && !m11_frame.valid &&
              dm2_v1_viewport_last_frame_composition_receipt(
                  &viewport, &composition) == 0 && !composition.valid,
          "M11 frame receipt blocks a required door without source light ownership");

    memset(blocked_framebuffer, 0, sizeof(blocked_framebuffer));
    dm2_v1_viewport_init(&viewport, blocked_framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_asset_provider(&viewport, packed_asset_fetch, NULL);
    dm2_v1_viewport_set_scene_map_load_token(&viewport, 42u);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, 3, 0x53434e45u, 6u, 0u, 0u, 0u, 0u, 0u,
        0u, 0u, 0u, 0u);
    dm2_v1_viewport_set_gdat_interface_palette(
        &viewport, 1, 0x50414c31u, palette16);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    viewport.squares[DM2_SQ_D0C].flags |= DM2_SQF_HAS_DOOR;
    dm2_v1_render_doors(&viewport);
    check(viewport.asset_door_frame_drawn_count == 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR) != 0u,
          "active door frame fails closed without its source scene record");
    check(dm2_v1_viewport_last_original_material_gate_receipt(
              &viewport, &gate) == 1 && gate.valid &&
              gate.original_material_required &&
              !gate.scene_record_ready && gate.palette_ready &&
              gate.colorkey_ready && !gate.decoded_format_gate_ready &&
              !gate.accepted,
          "door material receipt identifies the missing scene-record gate");
    check(dm2_v1_viewport_last_original_door_surface_request(
              &viewport, &surface_request) == 0 && !surface_request.valid,
          "blocked door material publishes no renderer surface request");
    check(dm2_v1_viewport_last_original_door_surface_binding(
              &viewport, &surface_binding) == 0 && !surface_binding.valid,
          "blocked door material publishes no renderer surface binding");
    check(dm2_v1_viewport_last_original_door_opening_frame_request(
              &viewport, &opening_request) == 0 && !opening_request.valid,
          "blocked door material publishes no opening-frame request");
    check(dm2_v1_viewport_last_original_door_presentation_command(
              &viewport, &presentation_command) == 0 &&
              !presentation_command.valid,
          "blocked door material publishes no presentation command");

    memset(blocked_framebuffer, 0x3au, sizeof(blocked_framebuffer));
    dm2_v1_viewport_init(&viewport, blocked_framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_asset_provider(&viewport, packed_asset_fetch, NULL);
    dm2_v1_viewport_set_door_surface_view_provider(
        &viewport, packed_door_surface_view, NULL);
    dm2_v1_viewport_set_scene_map_load_token(&viewport, 42u);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, 3, 0x53434e45u, 6u, 0u, 0u, 0u, 0u, 0u,
        0u, 0u, 0u, 0u);
    dm2_v1_viewport_set_gdat_interface_palette(
        &viewport, 1, 0x50414c31u, palette16);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    (void)dm2_v1_viewport_bind_static_graphicsset_scene_record(
        &viewport, 42u, 0x53434e45u);
    viewport.squares[DM2_SQ_D0C].flags |= DM2_SQF_HAS_DOOR;
    invalid_u4_pixels = 1;
    dm2_v1_render_doors(&viewport);
    invalid_u4_pixels = 0;
    check(viewport.asset_door_frame_drawn_count == 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR) != 0u &&
              door_frame &&
              blocked_framebuffer[door_frame->top_y * DM2_VP_WIDTH +
                                  door_frame->left_x] ==
                  0x3au,
          "out-of-range U4 index blocks raster before destination writes");
    check(dm2_v1_viewport_last_original_door_presentation_command(
              &viewport, &presentation_command) == 0 &&
              !presentation_command.valid,
          "invalid indexed source publishes no presentation command");

    memset(blocked_framebuffer, 0, sizeof(blocked_framebuffer));
    dm2_v1_viewport_init(&viewport, blocked_framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_asset_provider(&viewport, packed_asset_fetch, NULL);
    dm2_v1_viewport_set_scene_map_load_token(&viewport, 42u);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, 3, 0x53434e45u, 6u, 0u, 0u, 0u, 0u, 0u,
        0u, 0u, 0u, 0u);
    dm2_v1_viewport_set_gdat_interface_palette(
        &viewport, 1, 0x50414c31u, palette16);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    (void)dm2_v1_viewport_bind_static_graphicsset_scene_record(
        &viewport, 42u, 0x53434e45u);
    viewport.squares[DM2_SQ_D0C].flags |= DM2_SQF_HAS_DOOR;
    reject_frame_fetch = 1;
    dm2_v1_render_doors(&viewport);
    reject_frame_fetch = 0;
    check(dm2_v1_viewport_last_original_material_gate_receipt(
              &viewport, &gate) == 1 && gate.scene_record_ready &&
              gate.palette_ready && gate.colorkey_ready &&
              !gate.decoded_format_gate_ready && !gate.accepted,
          "door material receipt identifies a rejected decoded-format fetch");

    memset(blocked_framebuffer, 0, sizeof(blocked_framebuffer));
    dm2_v1_viewport_init(&viewport, blocked_framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_asset_provider(&viewport, packed_asset_fetch, NULL);
    dm2_v1_viewport_set_scene_map_load_token(&viewport, 42u);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, 3, 0x53434e45u, 6u, 0u, 0u, 0u, 0u, 0u,
        0u, 0u, 0u, 0u);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    (void)dm2_v1_viewport_bind_static_graphicsset_scene_record(
        &viewport, 42u, 0x53434e45u);
    viewport.squares[DM2_SQ_D0C].flags |= DM2_SQF_HAS_DOOR;
    dm2_v1_render_doors(&viewport);
    check(dm2_v1_viewport_last_original_material_gate_receipt(
              &viewport, &gate) == 1 && gate.scene_record_ready &&
              !gate.palette_ready && gate.colorkey_ready && !gate.accepted,
          "door material receipt identifies the absent palette gate");

    memset(blocked_framebuffer, 0, sizeof(blocked_framebuffer));
    dm2_v1_viewport_init(&viewport, blocked_framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_asset_provider(&viewport, packed_asset_fetch, NULL);
    dm2_v1_viewport_set_scene_map_load_token(&viewport, 42u);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, 3, 0x53434e45u, 16u, 0u, 0u, 0u, 0u, 0u,
        0u, 0u, 0u, 0u);
    dm2_v1_viewport_set_gdat_interface_palette(
        &viewport, 1, 0x50414c31u, palette16);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    (void)dm2_v1_viewport_bind_static_graphicsset_scene_record(
        &viewport, 42u, 0x53434e45u);
    viewport.squares[DM2_SQ_D0C].flags |= DM2_SQF_HAS_DOOR;
    dm2_v1_render_doors(&viewport);
    check(dm2_v1_viewport_last_original_material_gate_receipt(
              &viewport, &gate) == 1 && gate.scene_record_ready &&
              gate.palette_ready && !gate.colorkey_ready && !gate.accepted,
          "door material receipt identifies the malformed source colorkey");

    return failures == 0 ? 0 : 1;
}
