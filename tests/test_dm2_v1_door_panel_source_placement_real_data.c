#include "dm2_v1_asset_loader.h"
#include "dm2_v1_door_mechanics.h"
#include "dm2_v1_gdat_door_overlay_m11_command.h"
#include "dm2_v1_viewport_renderer.h"
#include "firestaff_zip_extract.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
                       int expect_button,
                       int door_record_type)
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
    viewport->squares[view_square].door_record_type =
        (uint8_t)door_record_type;
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

    if (door_record_type == 0 && view_square == DM2_SQ_D0C &&
        render_plan.doors[0].panel_gdat_index !=
            dm2_v1_viewport_door_panel_graphic_index_for_record(
                view_square, viewport->squares[view_square].door_gfx_index,
                viewport->squares[view_square].door_opening_dir)) {
        fprintf(stderr,
                "FAIL: type-0 DB0 door lost its source DOORS record route\n");
        dm2_v1_gdat_door_overlay_m11_command_plan_free(&material_plan);
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
    const char *archive = getenv("FIRESTAFF_DM2_DOS_ARCHIVE");
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    DM2_V1_AssetLoader loader;
    DM2_V1_ViewportState viewport;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    unsigned int door_text_count = 0u;
    int ok = 0;

    if (!archive || !archive[0]) {
        puts("SKIP: FIRESTAFF_DM2_DOS_ARCHIVE is not set");
        return 0;
    }
    if (firestaff_zip_extract_by_suffix(archive, "data/graphics.dat",
                                        &graphics, &graphics_size) != 0 ||
        !graphics || !graphics_size) {
        fputs("FAIL: selected canonical DM2 GRAPHICS.DAT is unreadable\n", stderr);
        return 1;
    }

    memset(&loader, 0, sizeof(loader));
    if (dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0) {
        fputs("FAIL: could not load canonical graphics.dat\n", stderr);
        goto cleanup;
    }
    for (size_t i = 0u; i < loader.entry_count; ++i) {
        if (loader.entries[i].cls1 == DM2_GDAT_CATEGORY_DOORS &&
            loader.entries[i].cls3 == DM2_GDAT_ENTRY_TYPE_TEXT) {
            ++door_text_count;
        }
    }
    if (door_text_count != 0u) {
        fprintf(stderr,
                "FAIL: selected DOORS category contains %u text rows\n",
                door_text_count);
        goto cleanup;
    }
    for (int state = DM2_DOOR_STATE_OPEN;
         state <= DM2_DOOR_STATE_DESTROYED; ++state) {
        if (dm2_door_state_label(state) != NULL) {
            fputs("FAIL: DOORS mechanics exposed synthetic state text\n",
                  stderr);
            goto cleanup;
        }
    }
    for (int type = DM2_DOOR_TYPE_PORTCULLIS;
         type <= DM2_DOOR_TYPE_RA; ++type) {
        if (dm2_door_type_label(type) != NULL) {
            fputs("FAIL: DOORS mechanics exposed synthetic type text\n",
                  stderr);
            goto cleanup;
        }
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
    if (!test_square(&loader, &viewport, DM2_SQ_D0C, 1, 0) ||
        !test_square(&loader, &viewport, DM2_SQ_D1C, 1, 1) ||
        !test_square(&loader, &viewport, DM2_SQ_D2C, 1, 1) ||
        !test_square(&loader, &viewport, DM2_SQ_D3C, 0, 1) ||
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
