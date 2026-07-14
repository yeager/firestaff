#include "dm2_v1_asset_loader.h"
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
    static const uint8_t bitmap[4] = { 1, 2, 3, 4 };
    int *overlay_fetches = user;
    if (index <= DM2_V1_VIEWPORT_GFX_DOOR_ORNATE_FIELD_BASE &&
        index > DM2_V1_VIEWPORT_GFX_DOOR_DESTROYED_MASK_FIELD_BASE) {
        ++*overlay_fetches;
        return -1;
    }
    *pixels = bitmap; *width = 2; *height = 2; *stride = 2; return 0;
}

static int static_palette(void *user, int index, uint8_t palette[16],
                          uint32_t *hash)
{
    (void)user; (void)index;
    memset(palette, 0, 16); palette[1] = 1; *hash = 1u; return 0;
}

int main(void)
{
    const char *root = getenv("FIRESTAFF_DM2_DATA_DIR");
    const char *home = getenv("HOME");
    char default_root[1024];
    char path[1024];
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    DM2_V1_AssetLoader loader;
    DM2_V1_ViewportState viewport;
    DM2_V1_DoorRenderPlan door_plan;
    DM2_V1_GdatDoorOverlayM11CommandPlan material_plan;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    int ornate = -1;
    int overlay_fetches = 0;

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
    door_plan.doors[0].ornament_index = ornate;
    door_plan.doors[0].ornate_gdat_index = dm2_v1_viewport_door_ornate_graphic_index(ornate, DM2_SQ_D0C);
    door_plan.doors[0].panel_gdat_index = dm2_v1_viewport_door_panel_graphic_index_for_square(DM2_SQ_D0C);
    door_plan.doors[0].frame_gdat_index = dm2_v1_viewport_door_frame_graphic_index_for_square(DM2_SQ_D0C);
    door_plan.doors[0].panel_rect = (DM2_V1_ViewportRect){ 0, 28, 320, 144 };
    door_plan.doors[0].panel_visible_rect = door_plan.doors[0].panel_rect;
    door_plan.doors[0].frame_rect = door_plan.doors[0].panel_rect;
    if (!dm2_v1_gdat_door_overlay_m11_command_plan_build(&loader, &door_plan, &material_plan) ||
        !material_plan.valid || material_plan.command_count != 1 || !material_plan.command_hash ||
        material_plan.commands[0].entry_index != ornate ||
        material_plan.commands[0].category != DM2_GDAT_CATEGORY_DOOR_GFX ||
        !material_plan.commands[0].palette_hash) goto fail;
    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    viewport.squares[DM2_SQ_D0C].flags = DM2_SQF_HAS_DOOR;
    viewport.squares[DM2_SQ_D0C].ornament_index = ornate;
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_viewport_set_asset_provider(&viewport, static_fetch, &overlay_fetches);
    dm2_v1_viewport_set_asset_palette_provider(&viewport, static_palette, NULL);
    dm2_v1_viewport_set_gdat_door_overlay_material_plan(&viewport, &material_plan);
    dm2_v1_render_doors(&viewport);
    if (overlay_fetches || viewport.gdat_door_overlay_material_plan_consumed_count != 1 ||
        viewport.asset_door_overlay_drawn_count != 1 ||
        (viewport.blocked_material_mask & DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR)) goto fail;
    puts("PASS: canonical GDAT door overlay plan reaches M11 directly");
    dm2_v1_gdat_door_overlay_m11_command_plan_free(&material_plan);
done:
    dm2_v1_asset_loader_free(&loader); free(graphics); return 0;
fail:
    fputs("FAIL: canonical GDAT door overlay plan was not source-owned\n", stderr);
    dm2_v1_gdat_door_overlay_m11_command_plan_free(&material_plan);
    dm2_v1_asset_loader_free(&loader); free(graphics); return 1;
}
