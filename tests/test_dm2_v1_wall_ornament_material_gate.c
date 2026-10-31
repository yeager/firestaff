/* DM2 V1 wall-ornament material-class gate.
 * skproject/SKWIN/SkWinCore.cpp DRAW_WALL_ORNATE (^32CB:15B8) consumes a
 * WALL_GFX image through QUERY_TEMP_PICST/DRAW_TEMP_PICST.  Firestaff treats
 * the wall ornament as a distinct dungeon material class: missing source
 * material blocks the frame instead of leaving a blank wall or procedural
 * substitute.  Exact destination rectangles are owned by the runtime plan.
 */
#include "dm2_v1_viewport_renderer.h"
#include "dm2_v1_wall_ornament.h"
#include "dm2_v1_world_model.h"
#include "dm2_v1_runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int withhold_material;
static int plan_valid;

static int read_graphics_file(const char *path, uint8_t **out_data,
                              size_t *out_size)
{
    FILE *file;
    long size;
    uint8_t *data;
    if (out_data) *out_data = NULL;
    if (out_size) *out_size = 0u;
    if (!path || !out_data || !out_size || !(file = fopen(path, "rb")))
        return 0;
    if (fseek(file, 0, SEEK_END) != 0 || (size = ftell(file)) <= 0) {
        fclose(file);
        return 0;
    }
    rewind(file);
    data = (uint8_t *)malloc((size_t)size);
    if (!data || fread(data, 1u, (size_t)size, file) != (size_t)size) {
        free(data);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out_data = data;
    *out_size = (size_t)size;
    return 1;
}

static int render_real_ornament(const DM2_V1_AssetLoader *loader,
                               uint8_t wall_gfx, uint8_t base_field,
                               int with_overlay, uint8_t *framebuffer)
{
    DM2_V1_ViewportState viewport;
    DM2_V1_WallOrnamentRenderPlan plan;
    DM2_V1_WallOrnamentReceipt base;
    DM2_V1_WallOrnamentReceipt overlay;
    if (!dm2_v1_wall_ornament_material_receipt(
            loader, wall_gfx, base_field, &base) ||
        (with_overlay && !dm2_v1_wall_ornament_material_receipt(
            loader, wall_gfx, DM2_V1_WALL_ORNATE_OVERLAY_IMAGE_FIELD,
            &overlay))) return 0;
    memset(framebuffer, 0, DM2_VP_WIDTH * DM2_VP_HEIGHT);
    memset(&plan, 0, sizeof(plan));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_viewport_set_asset_loader(&viewport, loader);
    viewport.squares[DM2_SQ_D1C].square_type = DM2_SQUARE_WALL;
    viewport.squares[DM2_SQ_D1C].wall_ornate_gfx_index = wall_gfx;
    plan.valid = 1;
    plan.ornament_count = 1;
    plan.ornaments[0].view_square = DM2_SQ_D1C;
    plan.ornaments[0].gdat_index =
        dm2_v1_viewport_wall_gfx_map_chip_graphic_index(wall_gfx);
    plan.ornaments[0].image_field = base_field;
    plan.ornaments[0].dst_rect =
        (DM2_V1_ViewportRect){ 16, 16, 48, 48 };
    plan.ornaments[0].src_rect = (DM2_V1_ViewportRect){
        0, 0, base.decoded_width, base.decoded_height };
    if (with_overlay) {
        plan.ornaments[0].overlay_valid = 1u;
        plan.ornaments[0].overlay_image_field =
            DM2_V1_WALL_ORNATE_OVERLAY_IMAGE_FIELD;
        plan.ornaments[0].overlay_colorkey = overlay.colorkey;
        plan.ornaments[0].overlay_dst_rect =
            (DM2_V1_ViewportRect){ 16, 16, 48, 48 };
        plan.ornaments[0].overlay_src_rect = (DM2_V1_ViewportRect){
            0, 0, overlay.decoded_width, overlay.decoded_height };
    }
    dm2_v1_viewport_set_gdat_wall_ornament_material_plan(&viewport, &plan);
    dm2_v1_render_wall_ornaments(&viewport);
    return viewport.blocked_material_mask == 0u &&
        viewport.asset_wall_ornament_drawn_count == 1;
}

static int test_real_generic_overlay(void)
{
    const char *graphics_path = getenv("FIRESTAFF_DM2_GRAPHICS_DAT");
    const char *data_root = getenv("FIRESTAFF_DATA");
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    DM2_V1_AssetLoader loader;
    uint8_t *base_frame = NULL;
    uint8_t *overlay_frame = NULL;
    int found = 0;
    unsigned int shop_overlay_count = 0u;
    char path[2048];

    if (!graphics_path && data_root && data_root[0]) {
        snprintf(path, sizeof(path), "%s/dm2/GRAPHICS.DAT", data_root);
        graphics_path = path;
    }
    if (!graphics_path && getenv("HOME")) {
        snprintf(path, sizeof(path), "%s/.firestaff/data/dm2/GRAPHICS.DAT",
                 getenv("HOME"));
        graphics_path = path;
    }
    if (!graphics_path || !read_graphics_file(
            graphics_path, &graphics, &graphics_size)) {
        puts("SKIP: original DM2 graphics are unavailable for overlay pixels");
        return 0;
    }
    memset(&loader, 0, sizeof(loader));
    if (dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0) {
        free(graphics);
        puts("SKIP: DM2 graphics could not initialize the GDAT loader");
        return 0;
    }
    base_frame = (uint8_t *)malloc(DM2_VP_WIDTH * DM2_VP_HEIGHT);
    overlay_frame = (uint8_t *)malloc(DM2_VP_WIDTH * DM2_VP_HEIGHT);
    if (!base_frame || !overlay_frame) goto done;

    for (unsigned int index = 0u; index <= 255u && !found; ++index) {
        DM2_V1_WallOrnateAlcoveTypeReceipt alcove;
        DM2_V1_WallOrnamentReceipt overlay;
        if (!dm2_v1_GET_WALL_ORNATE_ALCOVE_TYPE(
                &loader, (uint8_t)index, &alcove) || !alcove.valid ||
            !dm2_v1_wall_ornament_material_receipt(
                &loader, (uint8_t)index,
                DM2_V1_WALL_ORNATE_OVERLAY_IMAGE_FIELD, &overlay)) continue;
        if (alcove.alcove_type == 2u) {
            ++shop_overlay_count;
            continue;
        }
        for (unsigned int field = 1u; field <= 3u; ++field) {
            DM2_V1_WallOrnamentReceipt base;
            if (field == DM2_V1_WALL_ORNATE_OVERLAY_IMAGE_FIELD ||
                !dm2_v1_wall_ornament_material_receipt(
                    &loader, (uint8_t)index, (uint8_t)field, &base)) continue;
            if (!render_real_ornament(&loader, (uint8_t)index,
                                      (uint8_t)field, 0, base_frame) ||
                !render_real_ornament(&loader, (uint8_t)index,
                                      (uint8_t)field, 1, overlay_frame))
                continue;
            if (memcmp(base_frame, overlay_frame,
                       DM2_VP_WIDTH * DM2_VP_HEIGHT) != 0) {
                found = 1;
                break;
            }
        }
    }
    if (found) puts("PASS: real non-shop WALL_GFX overlay changes rendered pixels");
    else printf("SKIP: no generic overlay in this data set (%u shop overlays)\n",
                shop_overlay_count);

done:
    free(base_frame);
    free(overlay_frame);
    dm2_v1_asset_loader_free(&loader);
    free(graphics);
    return 0;
}

static void write_le16(uint8_t *out, uint16_t value)
{
    out[0] = (uint8_t)value;
    out[1] = (uint8_t)(value >> 8);
}

static int fetch_asset(void *user, int gdat_index, const uint8_t **pixels,
                       int *width, int *height, int *stride)
{
    static const uint8_t image[16] = {
        1, 1, 2, 2, 1, 1, 2, 2,
        1, 1, 2, 2, 1, 1, 2, 2
    };
    int expected;
    (void)user;

    expected = dm2_v1_viewport_wall_gfx_map_chip_graphic_index(7);
    if (withhold_material || gdat_index != expected) {
        return -1;
    }
    *pixels = image;
    *width = 4;
    *height = 4;
    *stride = 4;
    return 0;
}

static int fetch_palette(void *user, int gdat_index, uint8_t palette[16],
                         uint32_t *hash)
{
    int expected = dm2_v1_viewport_wall_gfx_map_chip_graphic_index(7);
    (void)user;
    if (gdat_index != expected) {
        return -1;
    }
    memset(palette, 0, 16u);
    palette[1] = 0x31u;
    palette[2] = 0x32u;
    *hash = 0x574f524eu;
    return 0;
}

int main(void)
{
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    DM2_V1_ViewportState viewport;
    DM2_V1_WallOrnamentRenderPlan plan;

    {
        static const uint8_t raw_rects[] = {
            0x0du, 0xfcu, 0x02u, 0x00u,
            0x00u, 0x10u, 0x01u, 0x10u,
            0x04u, 0x10u, 0x04u, 0x10u,
            10u, 0u, 20u, 0u, 30u, 0u, 40u, 0u,
            11u, 0u, 21u, 0u, 31u, 0u, 41u, 0u,
            14u, 0u, 24u, 0u, 34u, 0u, 44u, 0u
        };
        DM2_V1_RectTableNode node;
        if (!dm2_v1_wall_ornament_rect_table_query(
                raw_rects, sizeof(raw_rects), 0x1001u, &node) ||
            node.rect_number != 0x1001u || node.x != 11 || node.y != 21 ||
            node.width != 31 || node.height != 41 ||
            dm2_v1_wall_ornament_rect_table_query(
                raw_rects, sizeof(raw_rects) - 1u, 0x1004u, &node) ||
            dm2_v1_wall_ornament_rect_table_query(
                raw_rects, sizeof(raw_rects), 0x2000u, &node)) {
            fputs("FAIL: skproject GDAT rectangle table decoding\n", stderr);
            return 1;
        }
        puts("PASS: skproject GDAT rectangle table decoding");
    }
    {
        /* A 2x2 child anchored at the upper-left of a 3x3 containment parent
         * draws from (0,0), clipped to the parent's 3x3 bounds. */
        static const uint8_t raw_rects[] = {
            0x0du, 0xfcu, 0x01u, 0x00u,
            0x00u, 0x10u, 0x01u, 0x10u,
            9u, 0u, 0u, 0u, 3u, 0u, 3u, 0u,
            0u, 0u, 0x00u, 0x10u, 2u, 0u, 2u, 0u
        };
        DM2_V1_WallOrnamentBlitRect blit;
        if (!dm2_v1_wall_ornament_query_blit_rect(
                raw_rects, sizeof(raw_rects), 0x1001u, 0, -1, 4, 4, &blit) ||
            blit.destination_x != 0 || blit.destination_y != 0 ||
            blit.destination_width != 3 || blit.destination_height != 3 ||
            blit.source_x != 0 || blit.source_y != 0 ||
            blit.source_width != 3 || blit.source_height != 3) {
            fputs("FAIL: skproject QUERY_BLIT_RECT parent clipping\n", stderr);
            return 1;
        }
        {
            static const uint8_t big_endian_rects[] = {
                0xfcu, 0x0du, 0x00u, 0x01u,
                0x10u, 0x00u, 0x10u, 0x01u,
                0x00u, 0x09u, 0x00u, 0x00u, 0x00u, 0x03u, 0x00u, 0x03u,
                0x00u, 0x00u, 0x10u, 0x00u, 0x00u, 0x02u, 0x00u, 0x02u
            };
            if (!dm2_v1_wall_ornament_query_blit_rect(
                    big_endian_rects, sizeof(big_endian_rects), 0x1001u,
                    1, -1, 4, 4, &blit) ||
                blit.destination_x != 0 || blit.destination_y != 0 ||
                blit.destination_width != 3 || blit.destination_height != 3 ||
                blit.source_width != 3 || blit.source_height != 3) {
                fputs("FAIL: big-endian QUERY_BLIT_RECT parity\n", stderr);
                return 1;
            }
        }
        {
            uint8_t anchor_table[4u + 4u + 9u * 8u] = { 0 };
            static const int16_t expected_x[9] = { 98, 100, 97, 97, 100, 98, 97, 98, 100 };
            static const int16_t expected_y[9] = { 77, 80, 80, 75, 75, 80, 77, 75, 77 };
            DM2_V1_WallOrnamentBlitRect anchored;
            anchor_table[0] = 0x0du;
            anchor_table[1] = 0xfcu;
            write_le16(anchor_table + 2u, 1u);
            write_le16(anchor_table + 4u, 0x1000u);
            write_le16(anchor_table + 6u, 0x1008u);
            for (uint16_t mode = 0u; mode < 9u; ++mode) {
                uint8_t *row = anchor_table + 8u + (size_t)mode * 8u;
                write_le16(row, mode);
                write_le16(row + 2u, 0u);
                write_le16(row + 4u, 100u);
                write_le16(row + 6u, 80u);
            }
            for (uint16_t mode = 0u; mode < 9u; ++mode) {
                if (!dm2_v1_wall_ornament_query_blit_rect(
                        anchor_table, sizeof(anchor_table),
                        (uint16_t)(0x1000u + mode), 0, -1, 4, 6,
                        &anchored) ||
                    anchored.destination_x != expected_x[mode] ||
                    anchored.destination_y != expected_y[mode] ||
                    anchored.destination_width != 4 ||
                    anchored.destination_height != 6) {
                    fputs("FAIL: skproject QUERY_BLIT_RECT anchor mode\n", stderr);
                    return 1;
                }
            }
        }
        {
            /* A deferred containment chain using anchor 6 inherits the
             * parent height, matching skproject QUERY_BLIT_RECT case 6. */
            uint8_t containment_table[4u + 4u + 3u * 8u] = { 0 };
            DM2_V1_WallOrnamentBlitRect contained;
            containment_table[0] = 0x0du;
            containment_table[1] = 0xfcu;
            write_le16(containment_table + 2u, 1u);
            write_le16(containment_table + 4u, 0x1000u);
            write_le16(containment_table + 6u, 0x1002u);
            write_le16(containment_table + 8u, 1u);
            write_le16(containment_table + 10u, 0x1001u);
            write_le16(containment_table + 12u, 4u);
            write_le16(containment_table + 14u, 6u);
            write_le16(containment_table + 16u, 6u);
            write_le16(containment_table + 18u, 0x1002u);
            write_le16(containment_table + 20u, 4u);
            write_le16(containment_table + 22u, 6u);
            write_le16(containment_table + 24u, 9u);
            write_le16(containment_table + 26u, 0u);
            write_le16(containment_table + 28u, 100u);
            write_le16(containment_table + 30u, 80u);
            if (!dm2_v1_wall_ornament_query_blit_rect(
                    containment_table, sizeof(containment_table),
                    0x1000u, 0, -1, 4, 6, &contained) ||
                contained.destination_x != -91 ||
                contained.destination_y != 86 ||
                contained.destination_width != 4 ||
                contained.destination_height != 6) {
                fputs("FAIL: QUERY_BLIT_RECT mode-6 containment anchor\n", stderr);
                return 1;
            }
        }
        {
            static const uint8_t nested_table[] = {
                0x0du, 0xfcu, 0x01u, 0x00u,
                0x00u, 0x10u, 0x02u, 0x10u,
                10u, 0u, 1u, 0x10u, 1u, 0u, 1u, 0u,
                1u, 0u, 2u, 0x10u, 0u, 0u, 0u, 0u,
                9u, 0u, 0u, 0u, 4u, 0u, 4u, 0u
            };
            DM2_V1_WallOrnamentBlitRect nested;
            if (!dm2_v1_wall_ornament_query_blit_rect(
                    nested_table, sizeof(nested_table), 0x1000u,
                    0, -1, 4, 4, &nested) ||
                nested.destination_x != 1 || nested.destination_y != 1 ||
                nested.destination_width != 3 ||
                nested.destination_height != 3 || nested.source_x != 0 ||
                nested.source_y != 0) {
                fputs("FAIL: skproject QUERY_BLIT_RECT nested anchor mode\n", stderr);
                return 1;
            }
        }
        puts("PASS: skproject QUERY_BLIT_RECT placement and clipping");
    }
    {
        uint16_t rect_number = 0u;
        if (!dm2_v1_wall_ornament_rect_number(3, 0, 12u, &rect_number) ||
            rect_number != 3187u ||
            !dm2_v1_wall_ornament_rect_number(1, -1, 12u, &rect_number) ||
            rect_number != 0x1155u ||
            !dm2_v1_wall_ornament_rect_number(2, 1, 24u, &rect_number) ||
            rect_number != 0x117au ||
            dm2_v1_wall_ornament_rect_number(3, 1, 12u, &rect_number) ||
            dm2_v1_wall_ornament_rect_number(1, 0, 25u, &rect_number)) {
            fputs("FAIL: skproject wall-ornament rectangle selection\n", stderr);
            return 1;
        }
        puts("PASS: skproject wall-ornament rectangle selection");
    }
    {
        DM2_V1_SourceTimerQueue effect_timers;
        memset(&effect_timers, 0, sizeof(effect_timers));
        effect_timers.count = 1u;
        effect_timers.timers[0].type = 0x48u;
        if (dm2_v1_runtime_see_thru_walls_active_for_queue(&effect_timers) != 0) {
            fputs("FAIL: unrelated source timer enabled SeeThruWalls\n", stderr);
            return 1;
        }
        effect_timers.timers[0].type = 0x49u;
        if (dm2_v1_runtime_see_thru_walls_active_for_queue(&effect_timers) != 1) {
            fputs("FAIL: source ttySeeThruWalls timer was not detected\n", stderr);
            return 1;
        }
        puts("PASS: active source timer type 0x49 enables SeeThruWalls");
    }
    if (dm2_v1_viewport_wall_ornament_visible(3, 1) != 0 ||
        dm2_v1_viewport_wall_ornament_visible(3, 0) != 1 ||
        dm2_v1_viewport_wall_ornament_visible(2, 1) != 1) {
        fputs("FAIL: active SeeThruWalls should hide only source cell 3 ornament\n",
              stderr);
        return 1;
    }
    puts("PASS: active SeeThruWalls suppresses only the source D1-front ornament");

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_viewport_set_asset_provider(&viewport, fetch_asset, NULL);
    dm2_v1_viewport_set_asset_palette_provider(&viewport, fetch_palette, NULL);

    /* Visible D1C wall with a WALL_GFX ornament index. */
    viewport.squares[DM2_SQ_D1C].square_type = DM2_SQUARE_WALL;
    viewport.squares[DM2_SQ_D1C].wall_ornate_gfx_index = 7u;

    /* Without a source-owned placement plan the ornament is required but
     * not drawable, so the frame must block rather than invent a destination. */
    withhold_material = 0;
    plan_valid = 0;
    dm2_v1_render_wall_ornaments(&viewport);
    if ((viewport.blocked_material_mask &
         DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL_ORNAMENT) == 0u ||
        viewport.asset_wall_ornament_drawn_count != 0) {
        fputs("FAIL: wall ornament did not block without source placement plan\n",
              stderr);
        return 1;
    }

    /* Provide a runtime-bound placement plan.  The renderer may now consume
     * the fetched material at the source-owned destination. */
    memset(&plan, 0, sizeof(plan));
    plan.valid = 1;
    plan.ornament_count = 1;
    plan.ornaments[0].view_square = DM2_SQ_D1C;
    plan.ornaments[0].gdat_index =
        dm2_v1_viewport_wall_gfx_map_chip_graphic_index(7);
    plan.ornaments[0].dst_rect = (DM2_V1_ViewportRect){ 100, 60, 16, 16 };
    plan.ornaments[0].src_rect = (DM2_V1_ViewportRect){ 0, 0, 4, 4 };
    plan.ornaments[0].material_hash = 0u; /* not validated in this gate */
    dm2_v1_viewport_set_gdat_wall_ornament_material_plan(&viewport, &plan);
    memset(framebuffer, 0, sizeof(framebuffer));
    viewport.blocked_material_mask = 0u;
    viewport.asset_wall_ornament_drawn_count = 0;
    dm2_v1_render_wall_ornaments(&viewport);
    if ((viewport.blocked_material_mask &
         DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL_ORNAMENT) != 0u ||
        viewport.asset_wall_ornament_drawn_count != 1 ||
        framebuffer[68 * DM2_VP_WIDTH + 108] != 0x32u) {
        fputs("FAIL: wall ornament did not consume source material with plan\n",
              stderr);
        return 1;
    }

    /* Generic DRAW_WALL_ORNATE overlays require the authenticated WALL_GFX
     * loader. A plan cannot turn field 0x0f into a synthetic overlay. */
    plan.ornaments[0].overlay_valid = 1u;
    plan.ornaments[0].overlay_image_field =
        DM2_V1_WALL_ORNATE_OVERLAY_IMAGE_FIELD;
    plan.ornaments[0].overlay_dst_rect =
        (DM2_V1_ViewportRect){ 100, 60, 16, 16 };
    plan.ornaments[0].overlay_src_rect =
        (DM2_V1_ViewportRect){ 0, 0, 4, 4 };
    memset(framebuffer, 0, sizeof(framebuffer));
    viewport.blocked_material_mask = 0u;
    viewport.asset_wall_ornament_drawn_count = 0;
    dm2_v1_render_wall_ornaments(&viewport);
    if ((viewport.blocked_material_mask &
         DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL_ORNAMENT) == 0u ||
        viewport.asset_wall_ornament_drawn_count != 0) {
        fputs("FAIL: wall overlay drew without authenticated WALL_GFX data\n",
              stderr);
        return 1;
    }
    plan.ornaments[0].overlay_valid = 0u;

    /* A placement receipt for another WALL_GFX row must not borrow this
     * square's derived pixels. It is an unowned source-material mismatch,
     * not permission to draw at the supplied destination. */
    plan.ornaments[0].gdat_index =
        dm2_v1_viewport_wall_gfx_map_chip_graphic_index(8);
    memset(framebuffer, 0, sizeof(framebuffer));
    viewport.blocked_material_mask = 0u;
    viewport.asset_wall_ornament_drawn_count = 0;
    dm2_v1_render_wall_ornaments(&viewport);
    if ((viewport.blocked_material_mask &
         DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL_ORNAMENT) == 0u ||
        viewport.asset_wall_ornament_drawn_count != 0 ||
        framebuffer[68 * DM2_VP_WIDTH + 108] != 0u) {
        fputs("FAIL: mismatched wall ornament plan borrowed source pixels\n",
              stderr);
        return 1;
    }
    plan.ornaments[0].gdat_index =
        dm2_v1_viewport_wall_gfx_map_chip_graphic_index(7);

    /* Withholding the asset must block even when the plan is present. */
    withhold_material = 1;
    memset(framebuffer, 0, sizeof(framebuffer));
    viewport.blocked_material_mask = 0u;
    viewport.asset_wall_ornament_drawn_count = 0;
    dm2_v1_render_wall_ornaments(&viewport);
    if ((viewport.blocked_material_mask &
         DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL_ORNAMENT) == 0u ||
        viewport.asset_wall_ornament_drawn_count != 0) {
        fputs("FAIL: missing wall ornament material did not block frame\n",
              stderr);
        return 1;
    }

    puts("PASS: wall ornaments consume only source-owned WALL_GFX material");
    if (test_real_generic_overlay() != 0) {
        fputs("FAIL: real generic wall overlay rendering\n", stderr);
        return 1;
    }
    return 0;
}
