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

static int rect_equal(const DM2_V1_ViewportRect *a,
                      const DM2_V1_ViewportRect *b)
{
    return a->x == b->x && a->y == b->y &&
           a->w == b->w && a->h == b->h;
}

static int build_material_plan(const DM2_V1_AssetLoader *loader,
                               const DM2_V1_DoorRenderPlan *render_plan,
                               DM2_V1_GdatDoorOverlayM11CommandPlan *out_plan)
{
    memset(out_plan, 0, sizeof(*out_plan));
    if (!dm2_v1_gdat_door_overlay_m11_command_plan_build(loader, render_plan,
                                                         out_plan)) {
        return 0;
    }
    return out_plan->valid && out_plan->command_count > 0;
}

static const DM2_V1_GdatDoorOverlayM11Command *find_command(
    const DM2_V1_GdatDoorOverlayM11CommandPlan *plan,
    int view_square,
    int kind)
{
    for (int i = 0; i < plan->command_count; ++i) {
        const DM2_V1_GdatDoorOverlayM11Command *cmd = &plan->commands[i];
        if (cmd->view_square == view_square && cmd->kind == kind) {
            return cmd;
        }
    }
    return NULL;
}

static int expected_button_rect(const DM2_V1_AssetLoader *loader,
                                int view_square,
                                DM2_V1_ViewportRect *out_rect)
{
    uint16_t rect_number;
    int width = 0, height = 0;
    uint8_t *pixels;
    int result;

    if (!out_rect) return 0;
    if (!dm2_v1_gdat_door_overlay_button_rect_number(view_square, &rect_number)) {
        return 0;
    }
    pixels = dm2_v1_asset_load_image_field(
        loader, DM2_GDAT_CATEGORY_DOOR_BUTTONS, 0, 0,
        &width, &height, NULL);
    if (!pixels || width <= 0 || height <= 0) {
        dm2_v1_asset_free_pixels(pixels);
        return 0;
    }
    result = dm2_v1_gdat_door_overlay_query_raw4_destination_rect(
        loader, rect_number, width, height, out_rect);
    dm2_v1_asset_free_pixels(pixels);
    return result;
}

static int test_square(const DM2_V1_AssetLoader *loader,
                       DM2_V1_ViewportState *viewport,
                       int view_square,
                       int expect_button)
{
    DM2_V1_DoorRenderPlan render_plan;
    DM2_V1_GdatDoorOverlayM11CommandPlan material_plan;
    const DM2_V1_GdatDoorOverlayM11Command *panel_cmd;
    DM2_V1_ViewportRect expected_button;
    DM2_V1_ViewportRect fallback_panel;
    DM2_V1_ViewportRect fallback_button;
    int has_fallback_button;

    memset(&render_plan, 0, sizeof(render_plan));
    memset(&material_plan, 0, sizeof(material_plan));
    memset(&expected_button, 0, sizeof(expected_button));
    memset(viewport->squares, 0, sizeof(viewport->squares));

    viewport->squares[view_square].flags = DM2_SQF_HAS_DOOR;
    viewport->squares[view_square].door_gfx_admitted = 1;
    viewport->squares[view_square].door_record_type = 1;
    viewport->squares[view_square].door_gfx_index = 0;
    viewport->squares[view_square].door_state = 4;
    viewport->squares[view_square].door_opening_dir = 1;
    viewport->squares[view_square].door_open_pct = 0;
    viewport->squares[view_square].ornament_index = 0;
    viewport->squares[view_square].door_ornate_gfx_index = 0;
    viewport->squares[view_square].door_button = expect_button ? 1 : 0;
    viewport->squares[view_square].door_button_state = 0;
    if (view_square == DM2_SQ_D0C) {
        viewport->squares[view_square].door_direct_g1_root = 1;
    }

    if (!dm2_v1_viewport_build_door_render_plan(viewport, &render_plan) ||
        render_plan.door_count != 1) {
        fprintf(stderr, "FAIL: no render plan for square %d\n", view_square);
        return 0;
    }

    if (!build_material_plan(loader, &render_plan, &material_plan)) {
        fprintf(stderr, "FAIL: no material plan for square %d\n", view_square);
        return 0;
    }

    panel_cmd = find_command(&material_plan, view_square,
                             DM2_V1_GDAT_DOOR_PANEL);
    if (!panel_cmd || panel_cmd->rect_width == 0u ||
        panel_cmd->rect_height == 0u) {
        fprintf(stderr, "FAIL: missing source panel rect for square %d\n",
                view_square);
        dm2_v1_gdat_door_overlay_m11_command_plan_free(&material_plan);
        return 0;
    }

    if (render_plan.doors[0].panel_rect.x != panel_cmd->rect_x ||
        render_plan.doors[0].panel_rect.y != panel_cmd->rect_y ||
        render_plan.doors[0].panel_rect.w != (int)panel_cmd->rect_width ||
        render_plan.doors[0].panel_rect.h != (int)panel_cmd->rect_height) {
        fprintf(stderr,
                "FAIL: square %d panel rect mismatch: render (%d,%d,%d,%d) "
                "vs source (%d,%d,%u,%u)\n",
                view_square,
                render_plan.doors[0].panel_rect.x,
                render_plan.doors[0].panel_rect.y,
                render_plan.doors[0].panel_rect.w,
                render_plan.doors[0].panel_rect.h,
                panel_cmd->rect_x, panel_cmd->rect_y,
                panel_cmd->rect_width, panel_cmd->rect_height);
        dm2_v1_gdat_door_overlay_m11_command_plan_free(&material_plan);
        return 0;
    }

    /* The source panel rect must differ from the hard-coded fallback so the
     * source-lock is actually exercising the RAW4 table. */
    if (!dm2_v1_viewport_door_panel_rect_for_square(view_square,
                                                    &fallback_panel) ||
        rect_equal(&render_plan.doors[0].panel_rect, &fallback_panel)) {
        fprintf(stderr,
                "FAIL: square %d source panel rect equals fallback\n",
                view_square);
        dm2_v1_gdat_door_overlay_m11_command_plan_free(&material_plan);
        return 0;
    }

    has_fallback_button = dm2_v1_viewport_door_button_rect_for_square(
        view_square, &fallback_button);

    if (expect_button) {
        if (!expected_button_rect(loader, view_square, &expected_button)) {
            fprintf(stderr,
                    "FAIL: could not resolve expected button rect for square %d\n",
                    view_square);
            dm2_v1_gdat_door_overlay_m11_command_plan_free(&material_plan);
            return 0;
        }
        if (!rect_equal(&render_plan.doors[0].button_rect, &expected_button)) {
            fprintf(stderr,
                    "FAIL: square %d button rect mismatch: render (%d,%d,%d,%d) "
                    "vs source (%d,%d,%d,%d)\n",
                    view_square,
                    render_plan.doors[0].button_rect.x,
                    render_plan.doors[0].button_rect.y,
                    render_plan.doors[0].button_rect.w,
                    render_plan.doors[0].button_rect.h,
                    expected_button.x, expected_button.y,
                    expected_button.w, expected_button.h);
            dm2_v1_gdat_door_overlay_m11_command_plan_free(&material_plan);
            return 0;
        }
        if (has_fallback_button &&
            rect_equal(&render_plan.doors[0].button_rect, &fallback_button)) {
            fprintf(stderr,
                    "FAIL: square %d source button rect equals fallback\n",
                    view_square);
            dm2_v1_gdat_door_overlay_m11_command_plan_free(&material_plan);
            return 0;
        }
    }

    dm2_v1_gdat_door_overlay_m11_command_plan_free(&material_plan);
    return 1;
}

static int test_fallback_without_source(DM2_V1_ViewportState *viewport)
{
    DM2_V1_DoorRenderPlan render_plan;
    DM2_V1_ViewportRect fallback_panel;

    memset(&render_plan, 0, sizeof(render_plan));
    memset(viewport->squares, 0, sizeof(viewport->squares));
    viewport->squares[DM2_SQ_D0C].flags = DM2_SQF_HAS_DOOR;
    viewport->squares[DM2_SQ_D0C].door_gfx_admitted = 1;
    viewport->squares[DM2_SQ_D0C].door_record_type = 1;
    viewport->squares[DM2_SQ_D0C].door_gfx_index = 0;
    viewport->squares[DM2_SQ_D0C].door_state = 4;
    viewport->squares[DM2_SQ_D0C].door_opening_dir = 1;
    viewport->squares[DM2_SQ_D0C].door_open_pct = 0;
    viewport->squares[DM2_SQ_D0C].door_button = 1;
    viewport->squares[DM2_SQ_D0C].door_button_state = 0;

    viewport->source_materials_required = 0;
    viewport->asset_loader = NULL;

    if (!dm2_v1_viewport_build_door_render_plan(viewport, &render_plan) ||
        render_plan.door_count != 1) {
        fputs("FAIL: no fallback render plan\n", stderr);
        return 0;
    }

    if (!dm2_v1_viewport_door_panel_rect_for_square(DM2_SQ_D0C,
                                                    &fallback_panel)) {
        fputs("FAIL: could not resolve fallback panel rect\n", stderr);
        return 0;
    }

    if (!rect_equal(&render_plan.doors[0].panel_rect, &fallback_panel)) {
        fprintf(stderr,
                "FAIL: non-source render plan panel rect (%d,%d,%d,%d) "
                "does not match fallback (%d,%d,%d,%d)\n",
                render_plan.doors[0].panel_rect.x,
                render_plan.doors[0].panel_rect.y,
                render_plan.doors[0].panel_rect.w,
                render_plan.doors[0].panel_rect.h,
                fallback_panel.x, fallback_panel.y,
                fallback_panel.w, fallback_panel.h);
        return 0;
    }

    return 1;
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
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    int ok = 0;

    if (!root || !root[0]) {
        if (!home || !home[0]) {
            puts("SKIP: no local canonical DM2 data");
            return 0;
        }
        snprintf(default_root, sizeof(default_root),
                 "%s/.firestaff/data/dm2/data", home);
        root = default_root;
    }
    snprintf(path, sizeof(path), "%s/graphics.dat", root);
    if (!read_file(path, &graphics, &graphics_size)) {
        puts("SKIP: no local canonical DM2 data");
        return 0;
    }

    memset(&loader, 0, sizeof(loader));
    if (dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0) {
        fputs("FAIL: could not load canonical graphics.dat\n", stderr);
        goto cleanup;
    }

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_asset_loader(&viewport, &loader);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, DM2_V1_VIEWPORT_GFX_WALL_DEFAULT_GRAPHICSSET,
        0x53434e45u, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    /* D0C/D1C/D2C have both closed panels and default buttons. D3C has a panel
     * but no default button route. */
    if (!test_square(&loader, &viewport, DM2_SQ_D0C, 1) ||
        !test_square(&loader, &viewport, DM2_SQ_D1C, 1) ||
        !test_square(&loader, &viewport, DM2_SQ_D2C, 1) ||
        !test_square(&loader, &viewport, DM2_SQ_D3C, 0) ||
        !test_fallback_without_source(&viewport)) {
        goto cleanup;
    }

    printf("PASS: DM2 V1 door panel/button placement is source-locked "
           "(panel/button rects match RAW4-derived M11 commands)\n");
    ok = 1;

cleanup:
    dm2_v1_asset_loader_free(&loader);
    free(graphics);
    return ok ? 0 : 1;
}
