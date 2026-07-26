#include "dm2_v1_asset_loader.h"
#include "dm2_v1_door_mechanics.h"
#include "dm2_v1_gdat_door_overlay_m11_command.h"
#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_file(const char *path, uint8_t **out, size_t *out_size)
{
    FILE *file = fopen(path, "rb");
    long size;
    if (!file || fseek(file, 0, SEEK_END) || (size = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET)) { if (file) fclose(file); return 0; }
    *out = malloc((size_t)size);
    if (!*out || fread(*out, 1u, (size_t)size, file) != (size_t)size) {
        free(*out); *out = NULL; fclose(file); return 0;
    }
    fclose(file); *out_size = (size_t)size; return 1;
}

static int static_fetch(void *user, int index, const uint8_t **pixels,
                        int *width, int *height, int *stride)
{
    int *fallback_fetches = user;
    (void)index;
    (void)pixels;
    (void)width;
    (void)height;
    (void)stride;
    ++*fallback_fetches;
    return -1;
}

static int static_palette(void *user, int index, uint8_t palette[16],
                          uint32_t *hash)
{
    (void)user; (void)index;
    memset(palette, 0, 16); palette[1] = 1; *hash = 1u; return 0;
}

static void bind_scene_control(DM2_V1_ViewportState *viewport)
{
    dm2_v1_viewport_set_gdat_scene_control(
        viewport, 1, DM2_V1_VIEWPORT_GFX_WALL_DEFAULT_GRAPHICSSET,
        0x53434e45u, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

int main(void)
{
    const char *root = getenv("FIRESTAFF_DM2_DATA_DIR");
    const char *home = getenv("HOME");
    char default_root[1024];
    char path[2048];
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    DM2_V1_AssetLoader loader;
    DM2_V1_ViewportState viewport;
    DM2_V1_DoorRenderPlan door_plan;
    DM2_V1_GdatDoorOverlayM11CommandPlan material_plan;
    DM2_V1_GdatDoorOverlayM11CommandPlan moving_material_plan;
    DM2_V1_GdatDoorOverlayM11CommandPlan changed_plan;
    DM2_V1_GdatDoorOverlayM11CommandPlan vertical_plan;
    DM2_V1_GdatDoorOverlayM11CommandPlan moving_vertical_plan;
    DM2_V1_GdatDoorOverlayM11CommandPlan horizontal_plan;
    DM2_V1_GdatDoorOverlayM11CommandPlan d3_plan;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    uint16_t source_color_key = 0u;
    uint16_t source_no_frames = 0u;
    const DM2_V1_GdatDoorOverlayM11Command *d3_panel = NULL;
    int ornate = -1;
    int fallback_fetches = 0;

    if (!root || !root[0]) {
        if (!home || !home[0]) { puts("SKIP: no local canonical DM2 data"); return 0; }
        snprintf(default_root, sizeof(default_root), "%s/.firestaff/data/dm2/data", home);
        root = default_root;
    }
    snprintf(path, sizeof(path), "%s/graphics.dat", root);
    if (!read_file(path, &graphics, &graphics_size)) {
        puts("SKIP: no local canonical DM2 data"); return 0;
    }
    memset(&loader, 0, sizeof(loader));
    memset(&material_plan, 0, sizeof(material_plan));
    memset(&moving_material_plan, 0, sizeof(moving_material_plan));
    memset(&changed_plan, 0, sizeof(changed_plan));
    memset(&vertical_plan, 0, sizeof(vertical_plan));
    memset(&moving_vertical_plan, 0, sizeof(moving_vertical_plan));
    memset(&horizontal_plan, 0, sizeof(horizontal_plan));
    memset(&d3_plan, 0, sizeof(d3_plan));
    if (dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0) goto fail;
    for (int i = 1; i < dm2_v1_asset_category_entry_count(&loader, DM2_GDAT_CATEGORY_DOOR_GFX); ++i) {
        uint8_t palette[16]; uint32_t hash = 0; int w = 0, h = 0;
        uint8_t *pixels = dm2_v1_asset_load_image_field(&loader, DM2_GDAT_CATEGORY_DOOR_GFX, i, 0, &w, &h, NULL);
        if (pixels && w > 0 && h > 0 && dm2_v1_asset_load_image_local_palette(&loader, DM2_GDAT_CATEGORY_DOOR_GFX, i, 0, palette, &hash) && hash) ornate = i;
        dm2_v1_asset_free_pixels(pixels);
        if (ornate >= 0) break;
    }
    if (ornate < 0) { puts("SKIP: canonical GDAT has no decoded ornate door overlay"); goto done; }
    memset(&door_plan, 0, sizeof(door_plan));
    door_plan.door_count = 1;
    door_plan.doors[0].view_square = DM2_SQ_D0C;
    door_plan.doors[0].door_gfx_index = 0;
    door_plan.doors[0].door_gfx_admitted = 1;
    door_plan.doors[0].door_opening_dir = 1;
    /* The only currently admitted source geometry is SKProject DRAW_DOOR's
     * closed-panel QUERY_BLIT_RECT route. Partial split panels fail closed. */
    door_plan.doors[0].door_state = 4;
    door_plan.doors[0].door_open_pct = 50;
    door_plan.doors[0].ornament_index = ornate;
    door_plan.doors[0].door_ornate_gfx_index = ornate;
    door_plan.doors[0].ornate_gdat_index = dm2_v1_viewport_door_ornate_graphic_index(ornate, DM2_SQ_D0C);
    door_plan.doors[0].panel_gdat_index = dm2_v1_viewport_door_panel_graphic_index_for_square(DM2_SQ_D0C);
    door_plan.doors[0].frame_gdat_index = dm2_v1_viewport_door_frame_graphic_index_for_square(DM2_SQ_D0C);
    door_plan.doors[0].graphicsset_index =
        DM2_V1_VIEWPORT_GFX_WALL_DEFAULT_GRAPHICSSET;
    door_plan.doors[0].panel_rect = (DM2_V1_ViewportRect){ 0, 28, 320, 144 };
    door_plan.doors[0].panel_visible_rect = door_plan.doors[0].panel_rect;
    door_plan.doors[0].frame_rect = door_plan.doors[0].panel_rect;
    door_plan.doors[0].button_gdat_index =
        dm2_v1_viewport_door_button_graphic_index_for_state(0);
    door_plan.doors[0].button_source_kind = 1;
    door_plan.doors[0].button_rect = (DM2_V1_ViewportRect){ 150, 80, 16, 16 };
    /* Missing dtWordValue/0x40 is source-defined zero, not a failed material
     * route: skproject's query returns zero for missing word entries. */
    (void)dm2_v1_asset_load_word_value(
        &loader, DM2_GDAT_CATEGORY_DOORS, 0,
        DM2_V1_DOOR_GDAT_NO_FRAMES_FIELD, &source_no_frames);
    if (!dm2_v1_gdat_door_overlay_m11_command_plan_build(&loader, &door_plan, &material_plan) ||
        !material_plan.valid || material_plan.command_count != 4 || !material_plan.command_hash ||
        material_plan.commands[0].kind != DM2_V1_GDAT_DOOR_PANEL ||
        material_plan.commands[1].entry_index != ornate ||
        material_plan.commands[1].kind != DM2_V1_GDAT_DOOR_OVERLAY_ORNATE ||
        material_plan.commands[2].kind != DM2_V1_GDAT_DOOR_FRAME ||
        material_plan.commands[2].entry_index !=
            DM2_V1_VIEWPORT_GFX_WALL_DEFAULT_GRAPHICSSET ||
        material_plan.commands[3].kind != DM2_V1_GDAT_DOOR_BUTTON ||
        material_plan.commands[0].door_opening_dir != 1 ||
        material_plan.commands[0].door_state != 4 ||
        material_plan.commands[0].door_open_pct != 50 ||
        material_plan.commands[0].draw_distance != 0u ||
        material_plan.commands[0].stretch_dual != 0x71u ||
        material_plan.commands[0].light_palette != 0u ||
        !material_plan.commands[0].rect_number ||
        !material_plan.commands[0].rect_width ||
        !material_plan.commands[0].rect_height ||
        !material_plan.commands[0].rect_table_hash ||
        !material_plan.commands[0].rect_row_hash ||
        !material_plan.commands[0].geometry_hash ||
        !material_plan.commands[0].decoded_hash ||
        !material_plan.commands[0].material_source_bytes ||
        !material_plan.commands[0].material_source_byte_count ||
        !material_plan.commands[0].material_receipt_hash ||
        !material_plan.commands[0].selection_hash ||
        !dm2_v1_asset_load_word_value(
            &loader, DM2_GDAT_CATEGORY_DOORS, 0,
            DM2_V1_DOOR_GDAT_COLORKEY_FIELD, &source_color_key) ||
        material_plan.commands[0].color_key != source_color_key ||
        material_plan.commands[0].no_frames != source_no_frames ||
        !material_plan.commands[0].palette_hash || !material_plan.commands[2].palette_hash ||
        !material_plan.commands[3].palette_hash ||
        !dm2_v1_gdat_door_overlay_m11_command_plan_draw_controls_valid(
            &material_plan)) {
        goto fail;
    }
    if (!dm2_v1_gdat_door_overlay_m11_command_plan_build_for_movement(
            &loader, &door_plan, 1, &moving_material_plan) ||
        !moving_material_plan.valid ||
        moving_material_plan.command_count != material_plan.command_count ||
        moving_material_plan.command_hash == 0u ||
        moving_material_plan.command_hash == material_plan.command_hash ||
        moving_material_plan.commands[0].movement_active != 1u ||
        moving_material_plan.commands[0].geometry_hash !=
            material_plan.commands[0].geometry_hash ||
        moving_material_plan.commands[0].decoded_hash !=
            material_plan.commands[0].decoded_hash ||
        !dm2_v1_gdat_door_overlay_m11_command_plan_draw_controls_valid(
            &moving_material_plan)) goto fail;
    door_plan.doors[0].door_open_pct = 75;
    if (!dm2_v1_gdat_door_overlay_m11_command_plan_build(&loader, &door_plan,
                                                          &changed_plan) ||
        changed_plan.command_hash == material_plan.command_hash ||
        changed_plan.commands[0].selection_hash ==
            material_plan.commands[0].selection_hash) goto fail;
    dm2_v1_gdat_door_overlay_m11_command_plan_free(&changed_plan);
    door_plan.doors[0].door_open_pct = 50;
    /* An absent glbMapGraphicsSet receipt may not borrow the former default
     * record.  This is the negative counterpart to the canonical index-one
     * material assertion above. */
    door_plan.doors[0].graphicsset_index = 0x100;
    if (dm2_v1_gdat_door_overlay_m11_command_plan_build(&loader, &door_plan,
                                                         &changed_plan)) goto fail;
    door_plan.doors[0].graphicsset_index =
        DM2_V1_VIEWPORT_GFX_WALL_DEFAULT_GRAPHICSSET;
    /* DRAW_DOOR's vertical intermediate state uses the next source
     * tlbRectnoDoorPosition RAW4 record, not a cropped closed-panel box. */
    door_plan.doors[0].door_state = 1;
    door_plan.doors[0].door_opening_dir = 1;
    door_plan.doors[0].panel_gdat_index =
        dm2_v1_viewport_door_panel_graphic_index_for_record(
            DM2_SQ_D0C, 0, 1);
    door_plan.doors[0].ornament_index = 0;
    door_plan.doors[0].door_ornate_gfx_index = 0;
    door_plan.doors[0].ornate_gdat_index = 0;
    door_plan.doors[0].button_gdat_index = 0;
    door_plan.doors[0].button_source_kind = 0;
    door_plan.doors[0].button_rect = (DM2_V1_ViewportRect){ 0, 0, 0, 0 };
    if (!dm2_v1_gdat_door_overlay_m11_command_plan_build(&loader, &door_plan,
                                                          &vertical_plan) ||
        !vertical_plan.valid || vertical_plan.command_count != 2 ||
        vertical_plan.commands[0].rect_number !=
            (uint16_t)(material_plan.commands[0].rect_number + 1u) ||
        !vertical_plan.commands[0].geometry_hash ||
        vertical_plan.commands[0].geometry_hash ==
            material_plan.commands[0].geometry_hash) goto fail;
    if (!dm2_v1_gdat_door_overlay_m11_command_plan_build_for_movement(
            &loader, &door_plan, 1, &moving_vertical_plan) ||
        !moving_vertical_plan.valid ||
        moving_vertical_plan.command_count != vertical_plan.command_count ||
        moving_vertical_plan.command_hash == 0u ||
        moving_vertical_plan.command_hash == vertical_plan.command_hash ||
        moving_vertical_plan.commands[0].movement_active != 1u ||
        moving_vertical_plan.commands[0].rect_number !=
            vertical_plan.commands[0].rect_number ||
        moving_vertical_plan.commands[0].geometry_hash !=
            vertical_plan.commands[0].geometry_hash ||
        moving_vertical_plan.commands[0].decoded_hash !=
            vertical_plan.commands[0].decoded_hash) {
        goto fail;
    }
    dm2_v1_gdat_door_overlay_m11_command_plan_free(&vertical_plan);
    dm2_v1_gdat_door_overlay_m11_command_plan_free(&moving_vertical_plan);
    /* Source D0 always chooses DOORS image zero through the 0x71 stretch
     * branch. The M11 consumer must reject a stale initial-0x40 receipt. */
    {
        DM2_V1_GdatDoorOverlayM11CommandPlan altered_plan = material_plan;
        altered_plan.commands[0].stretch_dual = 0x40u;
        if (dm2_v1_gdat_door_overlay_m11_command_plan_draw_controls_valid(
                &altered_plan)) goto fail;
        memset(framebuffer, 0, sizeof(framebuffer));
        dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
        viewport.squares[DM2_SQ_D0C].flags = DM2_SQF_HAS_DOOR;
        viewport.squares[DM2_SQ_D0C].door_gfx_admitted = 1;
        dm2_v1_viewport_set_source_materials_required(&viewport, 1);
        bind_scene_control(&viewport);
        dm2_v1_viewport_set_asset_provider(&viewport, static_fetch,
                                           &fallback_fetches);
        dm2_v1_viewport_set_asset_palette_provider(&viewport, static_palette,
                                                    NULL);
        dm2_v1_viewport_set_gdat_door_overlay_material_plan(&viewport,
                                                             &altered_plan);
        dm2_v1_render_doors(&viewport);
        if ((viewport.blocked_material_mask &
             DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR) == 0u) goto fail;
    }
    /* DRAW_DOOR opens horizontally by halving one real DOORS image and
     * submitting right then left RAW4 destinations. Both commands must be
     * present and reach the viewport together. */
    door_plan.doors[0].ornament_index = ornate;
    door_plan.doors[0].door_ornate_gfx_index = ornate;
    door_plan.doors[0].ornate_gdat_index =
        dm2_v1_viewport_door_ornate_graphic_index(ornate, DM2_SQ_D0C);
    door_plan.doors[0].button_source_kind = 1;
    door_plan.doors[0].button_gdat_index =
        dm2_v1_viewport_door_button_graphic_index_for_state(0);
    door_plan.doors[0].button_rect = (DM2_V1_ViewportRect){ 150, 80, 16, 16 };
    door_plan.doors[0].door_state = 1;
    door_plan.doors[0].door_opening_dir = 0;
    door_plan.doors[0].door_record_type = 1;
    door_plan.doors[0].panel_gdat_index =
        dm2_v1_viewport_door_panel_graphic_index_for_record(
            DM2_SQ_D0C, 0, 0);
    if (!dm2_v1_gdat_door_overlay_m11_command_plan_build(&loader, &door_plan,
                                                          &horizontal_plan) ||
        !horizontal_plan.valid || horizontal_plan.command_count != 5 ||
        horizontal_plan.commands[0].kind != DM2_V1_GDAT_DOOR_PANEL ||
        horizontal_plan.commands[1].kind != DM2_V1_GDAT_DOOR_PANEL ||
        horizontal_plan.commands[0].source_width == 0u ||
        horizontal_plan.commands[0].source_width * 2u !=
            horizontal_plan.commands[0].width ||
        horizontal_plan.commands[0].source_x !=
            horizontal_plan.commands[0].source_width ||
        horizontal_plan.commands[1].source_x != 0u ||
        horizontal_plan.commands[1].source_width !=
            horizontal_plan.commands[0].source_width ||
        horizontal_plan.commands[0].rect_number !=
            (uint16_t)(material_plan.commands[0].rect_number + 7u) ||
        horizontal_plan.commands[1].rect_number !=
            (uint16_t)(material_plan.commands[0].rect_number + 4u)) {
        goto fail;
    }
    dm2_v1_gdat_door_overlay_m11_command_plan_free(&horizontal_plan);
    /* The same skproject split transaction is table-driven for D1C/D2C and
     * all three intermediate states. Exercise every zero-light source route
     * against the canonical GDAT rather than treating D0 as a stand-in. */
    {
        static const int distance_squares[] = { DM2_SQ_D1C, DM2_SQ_D2C };
        for (size_t distance_i = 0u;
             distance_i < sizeof(distance_squares) / sizeof(distance_squares[0]);
             ++distance_i) {
            int distance_square = distance_squares[distance_i];
        for (int state = 1; state <= 3; ++state) {
            DM2_V1_GdatDoorOverlayM11CommandPlan split_distance_plan;
            memset(&split_distance_plan, 0, sizeof(split_distance_plan));
            memset(&door_plan, 0, sizeof(door_plan));
            door_plan.door_count = 1;
            door_plan.doors[0].view_square = distance_square;
            door_plan.doors[0].door_record_type = 1;
            door_plan.doors[0].door_gfx_admitted = 1;
            door_plan.doors[0].door_opening_dir = 0;
            door_plan.doors[0].door_state = (uint8_t)state;
            door_plan.doors[0].panel_gdat_index =
                dm2_v1_viewport_door_panel_graphic_index_for_record(
                    distance_square, 0, 0);
            door_plan.doors[0].frame_gdat_index =
                dm2_v1_viewport_door_frame_graphic_index_for_square(
                    distance_square);
            door_plan.doors[0].graphicsset_index =
                DM2_V1_VIEWPORT_GFX_WALL_DEFAULT_GRAPHICSSET;
            door_plan.doors[0].panel_rect =
                (DM2_V1_ViewportRect){ 0, 0, DM2_VP_WIDTH, DM2_VP_HEIGHT };
            door_plan.doors[0].panel_visible_rect = door_plan.doors[0].panel_rect;
            door_plan.doors[0].frame_rect = door_plan.doors[0].panel_rect;
            if (!dm2_v1_gdat_door_overlay_m11_command_plan_build(
                    &loader, &door_plan, &split_distance_plan) ||
                !split_distance_plan.valid ||
                split_distance_plan.command_count != 3 ||
                split_distance_plan.commands[0].kind !=
                    DM2_V1_GDAT_DOOR_PANEL ||
                split_distance_plan.commands[1].kind !=
                    DM2_V1_GDAT_DOOR_PANEL ||
                split_distance_plan.commands[0].light_palette != 0u ||
                split_distance_plan.commands[1].light_palette != 0u ||
                split_distance_plan.commands[0].source_x !=
                    split_distance_plan.commands[0].source_width ||
                split_distance_plan.commands[1].source_x != 0u ||
                split_distance_plan.commands[0].source_width == 0u ||
                split_distance_plan.commands[0].source_width * 2u !=
                    split_distance_plan.commands[0].width ||
                split_distance_plan.commands[0].rect_number !=
                    (uint16_t)(split_distance_plan.commands[1].rect_number + 3u)) {
                dm2_v1_gdat_door_overlay_m11_command_plan_free(
                    &split_distance_plan);
                goto fail;
            }
            memset(framebuffer, 0, sizeof(framebuffer));
            fallback_fetches = 0;
            dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
            viewport.squares[distance_square].flags = DM2_SQF_HAS_DOOR;
            viewport.squares[distance_square].door_gfx_admitted = 1;
            viewport.squares[distance_square].door_record_type = 1;
            viewport.squares[distance_square].door_state = (uint8_t)state;
            viewport.squares[distance_square].door_opening_dir = 0;
            dm2_v1_viewport_set_source_materials_required(&viewport, 1);
            bind_scene_control(&viewport);
            dm2_v1_viewport_set_asset_provider(
                &viewport, static_fetch, &fallback_fetches);
            dm2_v1_viewport_set_asset_palette_provider(
                &viewport, static_palette, NULL);
            dm2_v1_viewport_set_gdat_door_overlay_material_plan(
                &viewport, &split_distance_plan);
            dm2_v1_render_doors(&viewport);
            if (fallback_fetches || viewport.asset_door_panel_drawn_count != 0 ||
                viewport.asset_door_frame_drawn_count != 0 ||
                viewport.gdat_door_overlay_material_plan_consumed_count != 2 ||
                !viewport.last_door_panel_asset_blit_valid ||
                viewport.last_door_panel_asset_blit.src_rect.x != 0 ||
                viewport.last_door_panel_asset_blit.src_rect.w !=
                    split_distance_plan.commands[1].source_width ||
                (viewport.blocked_material_mask &
                 DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR) == 0u) {
                dm2_v1_gdat_door_overlay_m11_command_plan_free(
                    &split_distance_plan);
                goto fail;
            }
            dm2_v1_gdat_door_overlay_m11_command_plan_free(
                &split_distance_plan);
        }
        }
    }
    memset(&door_plan, 0, sizeof(door_plan));
    door_plan.door_count = 1;
    door_plan.doors[0].door_gfx_index = 0;
    door_plan.doors[0].door_gfx_admitted = 1;
    door_plan.doors[0].door_opening_dir = 1;
    door_plan.doors[0].door_state = 4;
    door_plan.doors[0].door_open_pct = 50;
    door_plan.doors[0].graphicsset_index =
        DM2_V1_VIEWPORT_GFX_WALL_DEFAULT_GRAPHICSSET;
    door_plan.doors[0].door_state = 4;
    door_plan.doors[0].door_opening_dir = 1;
    door_plan.doors[0].door_record_type = 0;
    door_plan.doors[0].panel_gdat_index =
        dm2_v1_viewport_door_panel_graphic_index_for_square(DM2_SQ_D0C);
    /* D3 is skproject cell 11 (Y distance 3). It has no admitted frame
     * route, so this source-only M11 transaction must contain panel pixels
     * only. Its panel destination is the actual RAW4 rect transaction. */
    door_plan.doors[0].view_square = DM2_SQ_D3C;
    door_plan.doors[0].panel_gdat_index =
        dm2_v1_viewport_door_panel_graphic_index_for_square(DM2_SQ_D3C);
    door_plan.doors[0].ornate_gdat_index = 0;
    door_plan.doors[0].frame_gdat_index =
        0;
    door_plan.doors[0].button_source_kind = 0;
    door_plan.doors[0].button_gdat_index = 0;
    if (!dm2_v1_gdat_door_overlay_m11_command_plan_build(&loader, &door_plan,
                                                          &d3_plan) ||
        !d3_plan.valid || d3_plan.command_count != 1) goto fail;
    for (int i = 0; i < d3_plan.command_count; ++i) {
        if (d3_plan.commands[i].kind == DM2_V1_GDAT_DOOR_PANEL) {
            d3_panel = &d3_plan.commands[i];
            break;
        }
    }
    if (!d3_panel || d3_panel->draw_distance != 3u ||
        !((d3_panel->field == 2u && d3_panel->stretch_dual == 0x40u &&
           d3_panel->light_palette == 0u) ||
          (d3_panel->field == 0u && d3_panel->stretch_dual == 0x1cu &&
           d3_panel->light_palette == 3u)) || !d3_panel->raw_hash ||
        !d3_panel->decoded_hash || !d3_panel->palette_hash ||
        !d3_panel->rect_number || !d3_panel->rect_width ||
        !d3_panel->rect_height || !d3_panel->geometry_hash) goto fail;
    dm2_v1_gdat_door_overlay_m11_command_plan_free(&d3_plan);
    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    viewport.squares[DM2_SQ_D3C].flags = DM2_SQF_HAS_DOOR;
    viewport.squares[DM2_SQ_D3C].door_gfx_admitted = 1;
    viewport.squares[DM2_SQ_D3C].door_state = 4;
    viewport.squares[DM2_SQ_D3C].door_opening_dir = 1;
    viewport.squares[DM2_SQ_D3C].door_open_pct = 50;
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    bind_scene_control(&viewport);
    dm2_v1_viewport_set_asset_provider(&viewport, static_fetch, &fallback_fetches);
    dm2_v1_viewport_set_asset_palette_provider(&viewport, static_palette, NULL);
    dm2_v1_viewport_set_gdat_door_overlay_material_plan(&viewport, &d3_plan);
    /* Rebuild only after attaching the actual D3 receipt: render_doors may
     * consume its source panel, but runtime promotion for this distant panel
     * remains unavailable until the full placement route is proven. */
    if (!dm2_v1_gdat_door_overlay_m11_command_plan_build(&loader, &door_plan,
                                                          &d3_plan)) goto fail;
    dm2_v1_viewport_set_gdat_door_overlay_material_plan(&viewport, &d3_plan);
    dm2_v1_render_doors(&viewport);
    if (fallback_fetches || viewport.gdat_door_overlay_material_plan_consumed_count != 1 ||
        viewport.asset_door_panel_drawn_count != 0 ||
        viewport.asset_door_frame_drawn_count != 0 ||
        (viewport.blocked_material_mask & DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR) == 0u) {
        goto fail;
    }
    /* DRAW_DOOR's field-zero retry keeps the original distance light palette
     * (three for D3). Its transform is not yet decoded, so the base IMG3
     * palette must not become a substitute draw. */
    d3_plan.commands[0].light_palette = 3u;
    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    viewport.squares[DM2_SQ_D3C].flags = DM2_SQF_HAS_DOOR;
    viewport.squares[DM2_SQ_D3C].door_gfx_admitted = 1;
    viewport.squares[DM2_SQ_D3C].door_state = 4;
    viewport.squares[DM2_SQ_D3C].door_opening_dir = 1;
    viewport.squares[DM2_SQ_D3C].door_open_pct = 50;
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    bind_scene_control(&viewport);
    dm2_v1_viewport_set_asset_provider(&viewport, static_fetch, &fallback_fetches);
    dm2_v1_viewport_set_asset_palette_provider(&viewport, static_palette, NULL);
    dm2_v1_viewport_set_gdat_door_overlay_material_plan(&viewport, &d3_plan);
    dm2_v1_render_doors(&viewport);
    if (viewport.asset_door_panel_drawn_count != 0 ||
        (viewport.blocked_material_mask & DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR) == 0u) goto fail;
    dm2_v1_gdat_door_overlay_m11_command_plan_free(&d3_plan);
    door_plan.doors[0].view_square = DM2_SQ_D0C;
    door_plan.doors[0].ornate_gdat_index =
        dm2_v1_viewport_door_ornate_graphic_index(ornate, DM2_SQ_D0C);
    door_plan.doors[0].frame_gdat_index =
        dm2_v1_viewport_door_frame_graphic_index_for_square(DM2_SQ_D0C);
    door_plan.doors[0].button_source_kind = 1;
    door_plan.doors[0].button_gdat_index =
        dm2_v1_viewport_door_button_graphic_index_for_state(0);
    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    viewport.squares[DM2_SQ_D0C].flags = DM2_SQF_HAS_DOOR;
    viewport.squares[DM2_SQ_D0C].door_gfx_admitted = 1;
    viewport.squares[DM2_SQ_D0C].door_direct_g1_root = 1;
    viewport.squares[DM2_SQ_D0C].door_state = 4;
    viewport.squares[DM2_SQ_D0C].door_opening_dir = 1;
    viewport.squares[DM2_SQ_D0C].door_open_pct = 50;
    viewport.squares[DM2_SQ_D0C].ornament_index = ornate;
    viewport.squares[DM2_SQ_D0C].door_ornate_gfx_index = ornate;
    viewport.squares[DM2_SQ_D0C].door_button = 1;
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    bind_scene_control(&viewport);
    dm2_v1_viewport_set_asset_provider(&viewport, static_fetch, &fallback_fetches);
    dm2_v1_viewport_set_asset_palette_provider(&viewport, static_palette, NULL);
    dm2_v1_viewport_set_gdat_door_overlay_material_plan(&viewport, &material_plan);
    dm2_v1_render_doors(&viewport);
    if (fallback_fetches || viewport.gdat_door_overlay_material_plan_consumed_count != 4 ||
        viewport.asset_door_panel_drawn_count != 0 ||
        viewport.asset_door_overlay_drawn_count != 0 ||
        viewport.asset_door_frame_drawn_count != 0 ||
        viewport.asset_door_button_drawn_count != 0 ||
        viewport.last_door_panel_asset_blit.transparent_color !=
        material_plan.commands[0].color_key ||
        (viewport.blocked_material_mask & DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR) == 0u) {
        goto fail;
    }
    memset(framebuffer, 0, sizeof(framebuffer));
    fallback_fetches = 0;
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    viewport.squares[DM2_SQ_D0C].flags = DM2_SQF_HAS_DOOR;
    viewport.squares[DM2_SQ_D0C].door_gfx_admitted = 1;
    viewport.squares[DM2_SQ_D0C].door_direct_g1_root = 1;
    viewport.squares[DM2_SQ_D0C].door_state = 4;
    viewport.squares[DM2_SQ_D0C].door_opening_dir = 1;
    viewport.squares[DM2_SQ_D0C].door_open_pct = 50;
    viewport.squares[DM2_SQ_D0C].ornament_index = ornate;
    viewport.squares[DM2_SQ_D0C].door_ornate_gfx_index = ornate;
    viewport.squares[DM2_SQ_D0C].door_button = 1;
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    bind_scene_control(&viewport);
    dm2_v1_viewport_set_asset_provider(&viewport, static_fetch, &fallback_fetches);
    dm2_v1_viewport_set_asset_palette_provider(&viewport, static_palette, NULL);
    dm2_v1_viewport_set_gdat_scene_movement_active(&viewport, 1);
    dm2_v1_viewport_set_gdat_door_overlay_material_plan(&viewport, &material_plan);
    dm2_v1_render_doors(&viewport);
    if (fallback_fetches || viewport.asset_door_panel_drawn_count != 0 ||
        (viewport.blocked_material_mask & DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR) == 0u) goto fail;
    memset(framebuffer, 0, sizeof(framebuffer));
    fallback_fetches = 0;
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    viewport.squares[DM2_SQ_D0C].flags = DM2_SQF_HAS_DOOR;
    viewport.squares[DM2_SQ_D0C].door_gfx_admitted = 1;
    viewport.squares[DM2_SQ_D0C].door_direct_g1_root = 1;
    viewport.squares[DM2_SQ_D0C].door_state = 4;
    viewport.squares[DM2_SQ_D0C].door_opening_dir = 1;
    viewport.squares[DM2_SQ_D0C].door_open_pct = 50;
    viewport.squares[DM2_SQ_D0C].ornament_index = ornate;
    viewport.squares[DM2_SQ_D0C].door_ornate_gfx_index = ornate;
    viewport.squares[DM2_SQ_D0C].door_button = 1;
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    bind_scene_control(&viewport);
    dm2_v1_viewport_set_asset_provider(&viewport, static_fetch, &fallback_fetches);
    dm2_v1_viewport_set_asset_palette_provider(&viewport, static_palette, NULL);
    dm2_v1_viewport_set_gdat_scene_movement_active(&viewport, 1);
    dm2_v1_viewport_set_gdat_door_overlay_material_plan(
        &viewport, &moving_material_plan);
    dm2_v1_render_doors(&viewport);
    if (fallback_fetches ||
        viewport.gdat_door_overlay_material_plan_consumed_count != 4 ||
        viewport.asset_door_panel_drawn_count != 0 ||
        viewport.asset_door_overlay_drawn_count != 0 ||
        viewport.asset_door_frame_drawn_count != 0 ||
        viewport.asset_door_button_drawn_count != 0 ||
        (viewport.blocked_material_mask & DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR) == 0u) goto fail;
    /* A receipt has already authenticated each decoded GDAT plane.  Mutating
     * one byte after plan construction must block the full door transaction,
     * rather than presenting a same-sized substitute through M11. */
    material_plan.commands[0].pixels[0] ^= 0x0fu;
    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    viewport.squares[DM2_SQ_D0C].flags = DM2_SQF_HAS_DOOR;
    viewport.squares[DM2_SQ_D0C].door_gfx_admitted = 1;
    viewport.squares[DM2_SQ_D0C].door_direct_g1_root = 1;
    viewport.squares[DM2_SQ_D0C].door_state = 4;
    viewport.squares[DM2_SQ_D0C].door_opening_dir = 1;
    viewport.squares[DM2_SQ_D0C].door_open_pct = 50;
    viewport.squares[DM2_SQ_D0C].ornament_index = ornate;
    viewport.squares[DM2_SQ_D0C].door_ornate_gfx_index = ornate;
    viewport.squares[DM2_SQ_D0C].door_button = 1;
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    bind_scene_control(&viewport);
    dm2_v1_viewport_set_asset_provider(&viewport, static_fetch, &fallback_fetches);
    dm2_v1_viewport_set_asset_palette_provider(&viewport, static_palette, NULL);
    dm2_v1_viewport_set_gdat_door_overlay_material_plan(&viewport, &material_plan);
    dm2_v1_render_doors(&viewport);
    if (viewport.asset_door_panel_drawn_count != 0 ||
        (viewport.blocked_material_mask & DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR) == 0u) goto fail;
    printf("PASS: canonical GDAT door plan hash=%08x commands=%u reaches M11 directly\n",
           material_plan.command_hash, (unsigned)material_plan.command_count);
    dm2_v1_gdat_door_overlay_m11_command_plan_free(&material_plan);
    dm2_v1_gdat_door_overlay_m11_command_plan_free(&moving_material_plan);
done:
    dm2_v1_gdat_door_overlay_m11_command_plan_free(&vertical_plan);
    dm2_v1_gdat_door_overlay_m11_command_plan_free(&moving_vertical_plan);
    dm2_v1_gdat_door_overlay_m11_command_plan_free(&horizontal_plan);
    dm2_v1_gdat_door_overlay_m11_command_plan_free(&d3_plan);
    dm2_v1_gdat_door_overlay_m11_command_plan_free(&changed_plan);
    dm2_v1_gdat_door_overlay_m11_command_plan_free(&moving_material_plan);
    dm2_v1_asset_loader_free(&loader); free(graphics); return 0;
fail:
    fputs("FAIL: canonical GDAT door overlay plan was not source-owned\n", stderr);
    dm2_v1_gdat_door_overlay_m11_command_plan_free(&vertical_plan);
    dm2_v1_gdat_door_overlay_m11_command_plan_free(&moving_vertical_plan);
    dm2_v1_gdat_door_overlay_m11_command_plan_free(&horizontal_plan);
    dm2_v1_gdat_door_overlay_m11_command_plan_free(&d3_plan);
    dm2_v1_gdat_door_overlay_m11_command_plan_free(&changed_plan);
    dm2_v1_gdat_door_overlay_m11_command_plan_free(&material_plan);
    dm2_v1_gdat_door_overlay_m11_command_plan_free(&moving_material_plan);
    dm2_v1_asset_loader_free(&loader); free(graphics); return 1;
}
