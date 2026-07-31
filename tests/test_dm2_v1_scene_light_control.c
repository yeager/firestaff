/* skproject/SKWIN/SkWinCore.cpp:
 * RECALC_LIGHT_LEVEL 5093-5131 and CHECK_RECOMPUTE_LIGHT 30408-30439. */
#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *label)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", label);
        ++failures;
    } else {
        fprintf(stderr, "PASS: %s\n", label);
    }
}

int main(void)
{
    struct {
        uint16_t highest_light_level;
        uint16_t ambient_darkness;
        uint8_t expected_floor;
        uint8_t expected_depth;
        int expected_recompute;
    } cases[] = {
        {0u, 0u, 0u, 0u, 0},
        {3u, 3u, 3u, 3u, 1},
        {5u, 8u, 5u, 8u, 1},
        {9u, 12u, 5u, 8u, 1}
    };
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    uint8_t record_framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    DM2_V1_ViewportState viewport;
    DM2_V1_ViewportState record_only_viewport;
    DM2_V1_FloorGfxViewportOwnershipReceipt ownership;
    DM2_V1_ViewportFloorGfxRenderPlanReceipt floor_plan;

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        uint8_t floor = 0xffu;
        uint8_t depth = 0xffu;
        int recompute = -1;
        dm2_v1_viewport_scene_light_control(
            cases[i].highest_light_level, cases[i].ambient_darkness,
            &floor, &depth, &recompute);
        check(floor == cases[i].expected_floor &&
                  depth == cases[i].expected_depth &&
                  recompute == cases[i].expected_recompute,
              "scene-light bounds match skproject control flow");
    }

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, 3, 0x53434e45u, 0u, 0u, 4u, 9u, 0u, 0u,
        0u, 0u, 0u, 12u);
    dm2_v1_viewport_render(&viewport);
    check(viewport.gdat_scene_light_floor == 5u &&
              viewport.gdat_scene_light_search_depth == 8u &&
              viewport.gdat_scene_light_recompute_enabled == 1 &&
              viewport.gdat_scene_light_consumed_count == 1,
          "active dungeon frame consumes the bounded source light plan");

    /* skproject/SKWIN/SkWinCore.cpp UPDATE_GFXSET 14493-14525 owns one
     * GRAPHICSSET command receipt; an invalid selector cannot inherit the
     * previous scene's materials or recalculated light plan. */
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, -1, 0x53434e45u, 0u, 0u, 4u, 9u, 0u, 0u,
        0u, 0u, 0u, 12u);
    check(!viewport.gdat_scene_control_ready &&
              viewport.gdat_scene_material_index == 0 &&
              viewport.gdat_scene_control_hash == 0u &&
              viewport.gdat_scene_light_floor == 0u &&
              viewport.gdat_scene_light_search_depth == 0u &&
              viewport.gdat_scene_light_recompute_enabled == 0,
          "invalid GRAPHICSSET selector clears rather than falling back to set zero");

    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 0, 3, 0x53434e45u, 0u, 0u, 4u, 9u, 0u, 0u,
        0u, 0u, 0u, 12u);
    check(viewport.gdat_scene_light_floor == 0u &&
              viewport.gdat_scene_light_search_depth == 0u &&
              viewport.gdat_scene_light_recompute_enabled == 0,
          "missing scene receipt clears the live light plan");

    memset(&ownership, 0, sizeof(ownership));
    ownership.valid = 1;
    ownership.viewport_owned = 1;
    ownership.map_load_token = 33u;
    ownership.gdat_category = DM2_GDAT_CATEGORY_FLOOR_GFX;
    ownership.floor_ornate_source_index = 0x28u;
    ownership.animated_frame_route = 1;
    memset(record_framebuffer, 0, sizeof(record_framebuffer));
    dm2_v1_viewport_init(&record_only_viewport, record_framebuffer,
                         DM2_VP_WIDTH);
    dm2_v1_viewport_set_scene_map_load_token(&record_only_viewport, 33u);
    dm2_v1_viewport_set_gdat_scene_control(
        &record_only_viewport, 1, 3, 0x53434e45u, 0x0042u,
        DM2_V1_GDAT_SCENE_FLAG_OUTDOOR, 4u, 5u, 0u, 0u,
        0u, 0u, 0u, 8u);
    check(dm2_v1_viewport_set_floor_gfx_viewport_ownership(
              &record_only_viewport, &ownership) == 1 &&
              dm2_v1_viewport_bind_static_graphicsset_scene_record(
                  &record_only_viewport, 33u, 0x53434e45u) == 1 &&
              dm2_v1_viewport_floor_gfx_render_plan_receipt(
                  &record_only_viewport, &floor_plan) == 1 &&
              floor_plan.valid,
          "coherent GRAPHICSSET record alone owns the static scene plan");
    check(dm2_v1_viewport_door_frame_graphic_index_for_graphicsset(
              3, DM2_SQ_D0C) ==
              DM2_V1_VIEWPORT_GFX_DOOR_FRAME_GRAPHICSSET_BASE -
                  (3 << 8) - DM2_V1_VIEWPORT_GFX_DOOR_FRAME_FRONT &&
              dm2_v1_viewport_door_frame_graphic_index_for_graphicsset(
                  DM2_V1_VIEWPORT_GFX_WALL_DEFAULT_GRAPHICSSET,
                  DM2_SQ_D0C) ==
              dm2_v1_viewport_door_frame_graphic_index_for_square(DM2_SQ_D0C),
          "active GRAPHICSSET packs a distinct door-frame material address");
    dm2_v1_viewport_set_scene_map_load_token(&viewport, 33u);
    check(dm2_v1_viewport_set_floor_gfx_viewport_ownership(
              &viewport, &ownership) == 1 &&
              dm2_v1_viewport_floor_gfx_render_plan_receipt(
                  &viewport, &floor_plan) == 0 && !floor_plan.valid,
          "floor render plan fails closed without static scene control");
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, 3, 0x53434e45u, 0x0042u,
        DM2_V1_GDAT_SCENE_FLAG_OUTDOOR, 4u, 5u, 0u, 0u,
        0u, 0u, 0u, 8u);
    check(dm2_v1_viewport_floor_gfx_render_plan_receipt(
              &viewport, &floor_plan) == 0 && !floor_plan.valid &&
              dm2_v1_viewport_bind_static_scene_light_control(
                  &viewport, 32u, 0x53434e45u) == 0 &&
              dm2_v1_viewport_bind_static_scene_light_control(
                  &viewport, 33u, 0x11111111u) == 0 &&
              dm2_v1_viewport_bind_static_scene_light_control(
                  &viewport, 33u, 0x53434e45u) == 1 &&
              dm2_v1_viewport_floor_gfx_render_plan_receipt(
                  &viewport, &floor_plan) == 0 && !floor_plan.valid &&
              dm2_v1_viewport_bind_static_scene_ambient_light_control(
                  &viewport, 32u, 0x53434e45u) == 0 &&
              dm2_v1_viewport_bind_static_scene_ambient_light_control(
                  &viewport, 33u, 0x11111111u) == 0 &&
              dm2_v1_viewport_bind_static_scene_ambient_light_control(
                  &viewport, 33u, 0x53434e45u) == 1 &&
              dm2_v1_viewport_floor_gfx_render_plan_receipt(
                  &viewport, &floor_plan) == 0 && !floor_plan.valid &&
              dm2_v1_viewport_bind_static_scene_ambient_darkness_control(
                  &viewport, 32u, 0x53434e45u) == 0 &&
              dm2_v1_viewport_bind_static_scene_ambient_darkness_control(
                  &viewport, 33u, 0x11111111u) == 0 &&
              dm2_v1_viewport_bind_static_scene_ambient_darkness_control(
                  &viewport, 33u, 0x53434e45u) == 1 &&
              dm2_v1_viewport_floor_gfx_render_plan_receipt(
                  &viewport, &floor_plan) == 0 && !floor_plan.valid &&
              dm2_v1_viewport_bind_static_scene_flags_control(
                  &viewport, 32u, 0x53434e45u) == 0 &&
              dm2_v1_viewport_bind_static_scene_flags_control(
                  &viewport, 33u, 0x11111111u) == 0 &&
              dm2_v1_viewport_bind_static_scene_flags_control(
                  &viewport, 33u, 0x53434e45u) == 1 &&
              dm2_v1_viewport_floor_gfx_render_plan_receipt(
                  &viewport, &floor_plan) == 0 && !floor_plan.valid &&
              dm2_v1_viewport_bind_static_scene_colorkey_control(
                  &viewport, 32u, 0x53434e45u) == 0 &&
              dm2_v1_viewport_bind_static_scene_colorkey_control(
                  &viewport, 33u, 0x11111111u) == 0 &&
              dm2_v1_viewport_bind_static_scene_colorkey_control(
                  &viewport, 33u, 0x53434e45u) == 1 &&
              dm2_v1_viewport_floor_gfx_render_plan_receipt(
                  &viewport, &floor_plan) == 0 && !floor_plan.valid &&
              dm2_v1_viewport_bind_static_scene_floor_material(
                  &viewport, 32u, 0x53434e45u) == 0 &&
              dm2_v1_viewport_bind_static_scene_floor_material(
                  &viewport, 33u, 0x11111111u) == 0 &&
              dm2_v1_viewport_bind_static_scene_floor_material(
                  &viewport, 33u, 0x53434e45u) == 1 &&
              dm2_v1_viewport_floor_gfx_render_plan_receipt(
                  &viewport, &floor_plan) == 0 && !floor_plan.valid &&
              dm2_v1_viewport_bind_static_scene_ceiling_material(
                  &viewport, 32u, 0x53434e45u) == 0 &&
              dm2_v1_viewport_bind_static_scene_ceiling_material(
                  &viewport, 33u, 0x11111111u) == 0 &&
              dm2_v1_viewport_bind_static_scene_ceiling_material(
                  &viewport, 33u, 0x53434e45u) == 1 &&
              dm2_v1_viewport_floor_gfx_render_plan_receipt(
                  &viewport, &floor_plan) == 0 && !floor_plan.valid &&
              dm2_v1_viewport_bind_static_scene_door_frame_material(
                  &viewport, 32u, 0x53434e45u) == 0 &&
              dm2_v1_viewport_bind_static_scene_door_frame_material(
                  &viewport, 33u, 0x11111111u) == 0 &&
              dm2_v1_viewport_bind_static_scene_door_frame_material(
                  &viewport, 33u, 0x53434e45u) == 1 &&
              dm2_v1_viewport_floor_gfx_render_plan_receipt(
                  &viewport, &floor_plan) == 0 && !floor_plan.valid &&
              dm2_v1_viewport_bind_static_scene_door_frame_d1c_material(
                  &viewport, 32u, 0x53434e45u) == 0 &&
              dm2_v1_viewport_bind_static_scene_door_frame_d1c_material(
                  &viewport, 33u, 0x11111111u) == 0 &&
              dm2_v1_viewport_bind_static_scene_door_frame_d1c_material(
                  &viewport, 33u, 0x53434e45u) == 1 &&
              dm2_v1_viewport_floor_gfx_render_plan_receipt(
                  &viewport, &floor_plan) == 0 && !floor_plan.valid &&
              dm2_v1_viewport_bind_static_scene_door_frame_d2c_material(
                  &viewport, 32u, 0x53434e45u) == 0 &&
              dm2_v1_viewport_bind_static_scene_door_frame_d2c_material(
                  &viewport, 33u, 0x11111111u) == 0 &&
              dm2_v1_viewport_bind_static_scene_door_frame_d2c_material(
                  &viewport, 33u, 0x53434e45u) == 1 &&
              dm2_v1_viewport_bind_static_graphicsset_scene_record(
                  &viewport, 32u, 0x53434e45u) == 0 &&
              dm2_v1_viewport_bind_static_graphicsset_scene_record(
                  &viewport, 33u, 0x11111111u) == 0 &&
              dm2_v1_viewport_bind_static_graphicsset_scene_record(
                  &viewport, 33u, 0x53434e45u) == 1,
          "static scene record requires matching active map token and hash");
    check(dm2_v1_viewport_set_floor_gfx_viewport_ownership(
              &viewport, &ownership) == 1 &&
              dm2_v1_viewport_floor_gfx_render_plan_receipt(
                  &viewport, &floor_plan) == 1 && floor_plan.valid &&
              viewport.gdat_static_scene_record.valid &&
              viewport.gdat_static_scene_record.map_load_token == 33u &&
              viewport.gdat_static_scene_record.scene_control_hash ==
                  0x53434e45u &&
              viewport.gdat_static_scene_record.graphicsset == 3u &&
              floor_plan.map_load_token == 33u &&
              floor_plan.static_scene_control_owned &&
              floor_plan.static_light_control_owned &&
              floor_plan.static_ambient_light_control_owned &&
              floor_plan.static_ambient_darkness_control_owned &&
              floor_plan.static_scene_flags_control_owned &&
              floor_plan.static_scene_colorkey_control_owned &&
              floor_plan.static_scene_floor_material_owned &&
              floor_plan.static_scene_ceiling_material_owned &&
              floor_plan.static_scene_door_frame_material_owned &&
              floor_plan.static_scene_door_frame_d1c_material_owned &&
              floor_plan.static_scene_door_frame_d2c_material_owned &&
              floor_plan.scene_control_hash == 0x53434e45u &&
              floor_plan.scene_colorkey == 0x0042u &&
              floor_plan.scene_floor_material_category ==
                  DM2_GDAT_CATEGORY_GRAPHICSSET &&
              floor_plan.scene_floor_material_graphicsset == 3u &&
              floor_plan.scene_floor_material_field == DM2_GDAT_GFXSET_FLOOR &&
              floor_plan.scene_ceiling_material_category ==
                  DM2_GDAT_CATEGORY_GRAPHICSSET &&
              floor_plan.scene_ceiling_material_graphicsset == 3u &&
              floor_plan.scene_ceiling_material_field == DM2_GDAT_GFXSET_CEIL &&
              floor_plan.scene_door_frame_material_category ==
                  DM2_GDAT_CATEGORY_GRAPHICSSET &&
              floor_plan.scene_door_frame_material_graphicsset == 3u &&
              floor_plan.scene_door_frame_material_field ==
                  DM2_GDAT_GFXSET_DOOR_FRAME_FRONT_D1 &&
              floor_plan.scene_door_frame_d1c_material_category ==
                  DM2_GDAT_CATEGORY_GRAPHICSSET &&
              floor_plan.scene_door_frame_d1c_material_graphicsset == 3u &&
              floor_plan.scene_door_frame_d1c_material_field ==
                  DM2_GDAT_GFXSET_DOOR_FRAME_D1C &&
              floor_plan.scene_door_frame_d2c_material_category ==
                  DM2_GDAT_CATEGORY_GRAPHICSSET &&
              floor_plan.scene_door_frame_d2c_material_graphicsset == 3u &&
              floor_plan.scene_door_frame_d2c_material_field ==
                  DM2_GDAT_GFXSET_DOOR_FRAME_D2C &&
              floor_plan.scene_flags == DM2_V1_GDAT_SCENE_FLAG_OUTDOOR &&
              floor_plan.outdoor_scene &&
              floor_plan.ambient_light == 4u &&
              floor_plan.highest_light_level == 5u &&
              floor_plan.ambient_darkness == 8u &&
              floor_plan.gdat_category == DM2_GDAT_CATEGORY_FLOOR_GFX &&
              floor_plan.floor_ornate_source_index == 0x28u &&
              floor_plan.animated_frame_route,
          "matching FLOOR_GFX provenance reaches dungeon viewport render plan");
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, 3, 0x4e455748u, 0u, 0u, 4u, 5u, 0u, 0u,
        0u, 0u, 0u, 8u);
    check(dm2_v1_viewport_floor_gfx_render_plan_receipt(
              &viewport, &floor_plan) == 0 && !floor_plan.valid,
          "changed static scene hash releases prior light ownership");
    dm2_v1_viewport_set_scene_map_load_token(&viewport, 34u);
    check(dm2_v1_viewport_floor_gfx_render_plan_receipt(
              &viewport, &floor_plan) == 0 && !floor_plan.valid &&
              dm2_v1_viewport_set_floor_gfx_viewport_ownership(
                  &viewport, &ownership) == 0,
          "stale FLOOR_GFX provenance cannot reach a later viewport token");

    return failures == 0 ? 0 : 1;
}
