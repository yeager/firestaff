#include "dm2_v1_gdat_door_overlay_m11_command.h"

#include "dm2_v1_door_mechanics.h"
#include "dm2_v1_viewport_renderer.h"

#include <string.h>

static uint32_t hash_bytes(uint32_t hash, const uint8_t *bytes, size_t size)
{
    for (size_t i = 0; i < size; ++i) { hash ^= bytes[i]; hash *= 16777619u; }
    return hash;
}

static uint32_t hash_u32(uint32_t hash, uint32_t value)
{
    return hash_bytes(hash, (const uint8_t *)&value, sizeof(value));
}

/* SKWINSPX v0/skglobal.cpp glbTabYAxisDistance and
 * v4/SkWinCore.cpp DRAW_DOOR: D0/D1/D2/D3 center cells are 0/3/6/11.
 * DRAW_DOOR starts with stretch 0x40 and light palette 0, selecting
 * image y-distance-1 except D0, which explicitly selects image zero. */
static int resolve_draw_door_distance(int view_square,
                                      uint8_t *out_distance,
                                      uint8_t *out_stretch,
                                      uint8_t *out_light)
{
    uint8_t distance;
    if (!out_distance || !out_stretch || !out_light) return 0;
    switch (view_square) {
    case DM2_SQ_D0C: distance = 0u; break;
    case DM2_SQ_D1C: distance = 1u; break;
    case DM2_SQ_D2C: distance = 2u; break;
    case DM2_SQ_D3C: distance = 3u; break;
    default: return 0;
    }
    *out_distance = distance;
    *out_stretch = 0x40u;
    *out_light = 0u;
    return 1;
}

static uint8_t draw_door_distance_stretch(uint8_t distance)
{
    /* kskval1.h tlbDistanceStretch = { 0x60, 0x40, 0x2b, 0x1c, 0x13 }. */
    static const uint8_t stretch[] = { 0x60u, 0x40u, 0x2bu, 0x1cu, 0x13u };
    return distance < sizeof(stretch) ? stretch[distance] : 0u;
}

void dm2_v1_gdat_door_overlay_m11_command_plan_free(
    DM2_V1_GdatDoorOverlayM11CommandPlan *plan)
{
    if (!plan) return;
    for (int i = 0; i < DM2_V1_GDAT_DOOR_OVERLAY_M11_COMMAND_MAX; ++i)
        dm2_v1_asset_free_pixels(plan->commands[i].pixels);
    memset(plan, 0, sizeof(*plan));
}

/* SKProject SkWinCore.cpp DM2_DRAW_DOOR/DRAW_DOOR_FRAMES resolves these
 * category/index/field triples before the first door blit. */
static int resolve_material_address(const DM2_V1_DoorRender *door, int kind,
                                    int *out_gdat_index, int *out_category,
                                    int *out_index, int *out_field)
{
    int gdat_index;
    int packed;
    if (!door || !out_gdat_index || !out_category || !out_index || !out_field) return 0;
    switch (kind) {
    case DM2_V1_GDAT_DOOR_PANEL:
        gdat_index = door->panel_gdat_index;
        *out_category = DM2_GDAT_CATEGORY_DOORS;
        if (gdat_index <= DM2_V1_VIEWPORT_GFX_DOOR_RECORD_PANEL_FIELD_BASE &&
            gdat_index > DM2_V1_VIEWPORT_GFX_DOOR_ORNATE_FIELD_BASE) {
            packed = DM2_V1_VIEWPORT_GFX_DOOR_RECORD_PANEL_FIELD_BASE - gdat_index;
            *out_index = (packed >> DM2_V1_VIEWPORT_GFX_DOOR_PANEL_INDEX_SHIFT) & 0xff;
            *out_field = packed & DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FIELD_MASK;
        } else {
            *out_index = 0;
            *out_field = DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FIELD_BASE - gdat_index;
        }
        break;
    case DM2_V1_GDAT_DOOR_OVERLAY_ORNATE:
        gdat_index = door->ornate_gdat_index;
        *out_category = DM2_GDAT_CATEGORY_DOOR_GFX;
        *out_index = door->ornament_index;
        *out_field = dm2_v1_viewport_door_panel_field_for_square(door->view_square);
        break;
    case DM2_V1_GDAT_DOOR_OVERLAY_DESTROYED_MASK:
        gdat_index = door->destroyed_mask_gdat_index;
        *out_category = DM2_GDAT_CATEGORY_DOORS;
        *out_index = door->door_gfx_index;
        *out_field = dm2_v1_viewport_door_panel_field_for_square(door->view_square);
        break;
    case DM2_V1_GDAT_DOOR_FRAME:
        gdat_index = door->frame_gdat_index;
        *out_category = DM2_GDAT_CATEGORY_GRAPHICSSET;
        *out_index = DM2_V1_VIEWPORT_GFX_WALL_DEFAULT_GRAPHICSSET;
        *out_field = dm2_v1_viewport_door_frame_field_for_square(door->view_square);
        break;
    case DM2_V1_GDAT_DOOR_BUTTON:
        if (door->button_source_kind != 1) return 1;
        gdat_index = door->button_gdat_index;
        *out_category = DM2_GDAT_CATEGORY_DOOR_BUTTONS;
        *out_index = 0;
        *out_field = DM2_V1_VIEWPORT_GFX_DOOR_BUTTON_FIELD_BASE - gdat_index;
        break;
    default: return 0;
    }
    if (gdat_index == 0 || *out_index < 0 || *out_index > 0xff ||
        *out_field < 0 || *out_field > 0xff) return 0;
    *out_gdat_index = gdat_index;
    return 1;
}

static int add_material(const DM2_V1_AssetLoader *loader,
                        DM2_V1_GdatDoorOverlayM11CommandPlan *plan,
                        const DM2_V1_DoorRender *door, int kind)
{
    DM2_V1_GdatDoorOverlayM11Command *command;
    int gdat_index, category, index, field;
    const uint8_t *raw;
    size_t raw_size = 0u;
    int width = 0, height = 0;

    if (kind == DM2_V1_GDAT_DOOR_BUTTON && door->button_source_kind != 1) return 1;
    if ((kind == DM2_V1_GDAT_DOOR_OVERLAY_ORNATE && !door->ornate_gdat_index) ||
        (kind == DM2_V1_GDAT_DOOR_OVERLAY_DESTROYED_MASK &&
         !door->destroyed_mask_gdat_index)) return 1;
    if (!resolve_material_address(door, kind, &gdat_index, &category, &index, &field)) return 0;
    if (plan->command_count >= DM2_V1_GDAT_DOOR_OVERLAY_M11_COMMAND_MAX ||
        index < 0 || index > 0xff || field < 0) return 0;
    command = &plan->commands[plan->command_count];
    if (kind == DM2_V1_GDAT_DOOR_PANEL &&
        (!resolve_draw_door_distance(door->view_square,
                                     &command->draw_distance,
                                     &command->stretch_dual,
                                     &command->light_palette) ||
         field != (command->draw_distance == 0u
                       ? 0 : (int)command->draw_distance - 1))) {
        return 0;
    }
    raw = dm2_v1_asset_load_sized(loader, category, index, field, &raw_size);
    command->pixels = dm2_v1_asset_load_image_field(loader, category, index,
                                                      field, &width, &height, NULL);
    /* DRAW_DOOR retries image zero when the distance-selected image cannot
     * be loaded. Its retry is a real DOORS GDAT image, with the table-owned
     * distance stretch and light palette, rather than substitute artwork. */
    if (kind == DM2_V1_GDAT_DOOR_PANEL &&
        (!raw || !raw_size || !command->pixels || width <= 0 || height <= 0) &&
        field != 0 && command->draw_distance != 0u) {
        dm2_v1_asset_free_pixels(command->pixels);
        command->pixels = NULL;
        raw_size = 0u;
        field = 0;
        raw = dm2_v1_asset_load_sized(loader, category, index, field, &raw_size);
        command->pixels = dm2_v1_asset_load_image_field(loader, category, index,
                                                          field, &width, &height, NULL);
        command->stretch_dual = draw_door_distance_stretch(command->draw_distance);
        command->light_palette = command->draw_distance;
    }
    if (!raw || !raw_size || !command->pixels || width <= 0 || height <= 0 ||
        !dm2_v1_asset_load_image_local_palette(loader, category, index, field,
                                                command->palette16,
                                                &command->palette_hash) ||
        !command->palette_hash) return 0;
    command->gdat_index = gdat_index;
    command->view_square = (uint8_t)door->view_square;
    command->kind = (uint8_t)kind;
    command->category = (uint8_t)category;
    command->entry_index = (uint8_t)index;
    command->field = (uint8_t)field;
    command->width = (uint16_t)width;
    command->height = (uint16_t)height;
    command->door_opening_dir = door->door_opening_dir;
    command->door_state = door->door_state;
    command->door_open_pct = door->door_open_pct;
    command->raw_hash = hash_bytes(2166136261u, raw, raw_size);
    command->decoded_hash = hash_bytes(2166136261u, command->pixels,
                                       (size_t)width * (size_t)height);
    command->selection_hash = 2166136261u;
    command->selection_hash = hash_u32(command->selection_hash,
                                       (uint32_t)door->view_square);
    command->selection_hash = hash_u32(command->selection_hash,
                                       (uint32_t)door->door_gfx_index);
    command->selection_hash = hash_u32(command->selection_hash,
                                       (uint32_t)door->door_opening_dir);
    command->selection_hash = hash_u32(command->selection_hash,
                                       (uint32_t)door->door_state);
    command->selection_hash = hash_u32(command->selection_hash,
                                       (uint32_t)door->door_open_pct);
    command->selection_hash = hash_u32(command->selection_hash,
                                       (uint32_t)field);
    if (kind == DM2_V1_GDAT_DOOR_PANEL) {
        if (!dm2_v1_asset_load_word_value(
                loader, DM2_GDAT_CATEGORY_DOORS, index,
                DM2_V1_DOOR_GDAT_COLORKEY_FIELD, &command->color_key)) {
            return 0;
        }
        if (command->color_key > 0xffu) return 0;
        /* DM2_QUERY_GDAT_ENTRY_DATA_INDEX returns zero for a missing
         * dtWordValue entry (skgdtqdb.cpp:105-116), exactly the value that
         * makes DRAW_DOOR_FRAMES retain frames. This is query semantics, not
         * a visual fallback. */
        (void)dm2_v1_asset_load_word_value(
            loader, DM2_GDAT_CATEGORY_DOORS, index,
            DM2_V1_DOOR_GDAT_NO_FRAMES_FIELD, &command->no_frames);
        command->selection_hash = hash_u32(command->selection_hash,
                                           command->color_key);
        command->selection_hash = hash_u32(command->selection_hash,
                                           command->no_frames);
        command->selection_hash = hash_u32(command->selection_hash,
                                           command->draw_distance);
        command->selection_hash = hash_u32(command->selection_hash,
                                           command->stretch_dual);
        command->selection_hash = hash_u32(command->selection_hash,
                                           command->light_palette);
    }
    return command->raw_hash != 0u && command->decoded_hash != 0u &&
           command->selection_hash != 0u && ++plan->command_count;
}

int dm2_v1_gdat_door_overlay_m11_command_plan_build(
    const DM2_V1_AssetLoader *loader, const DM2_V1_DoorRenderPlan *door_plan,
    DM2_V1_GdatDoorOverlayM11CommandPlan *out_plan)
{
    DM2_V1_GdatDoorOverlayM11CommandPlan candidate;
    uint32_t hash = 2166136261u;
    if (!out_plan) return 0;
    memset(out_plan, 0, sizeof(*out_plan));
    memset(&candidate, 0, sizeof(candidate));
    if (!loader || !door_plan || !dm2_v1_asset_loader_verify(loader)) return 0;
    for (int i = 0; i < door_plan->door_count; ++i) {
        int no_frames;
        if (!add_material(loader, &candidate, &door_plan->doors[i], DM2_V1_GDAT_DOOR_PANEL)) goto fail;
        no_frames = candidate.commands[candidate.command_count - 1].no_frames != 0u;
        if (
            !add_material(loader, &candidate, &door_plan->doors[i], DM2_V1_GDAT_DOOR_OVERLAY_ORNATE) ||
            !add_material(loader, &candidate, &door_plan->doors[i], DM2_V1_GDAT_DOOR_OVERLAY_DESTROYED_MASK)) goto fail;
        if ((!no_frames &&
             !add_material(loader, &candidate, &door_plan->doors[i], DM2_V1_GDAT_DOOR_FRAME)) ||
            !add_material(loader, &candidate, &door_plan->doors[i], DM2_V1_GDAT_DOOR_BUTTON)) goto fail;
    }
    if (!candidate.command_count) goto fail;
    for (int i = 0; i < candidate.command_count; ++i) {
        hash = hash_bytes(hash, (const uint8_t *)&candidate.commands[i].raw_hash,
                          sizeof(candidate.commands[i].raw_hash));
        hash = hash_bytes(hash, (const uint8_t *)&candidate.commands[i].decoded_hash,
                          sizeof(candidate.commands[i].decoded_hash));
        hash = hash_bytes(hash, (const uint8_t *)&candidate.commands[i].palette_hash,
                          sizeof(candidate.commands[i].palette_hash));
        hash = hash_bytes(hash, (const uint8_t *)&candidate.commands[i].selection_hash,
                          sizeof(candidate.commands[i].selection_hash));
    }
    candidate.command_hash = hash ? hash : 1u;
    candidate.valid = 1;
    *out_plan = candidate;
    return 1;
fail:
    dm2_v1_gdat_door_overlay_m11_command_plan_free(&candidate);
    return 0;
}
