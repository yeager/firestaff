#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "csb_v1_csbwin_layout_0232.h"
#include "csb_v1_csbwin_planar_bitmap.h"
#include "csb_v1_csbwin_viewport_graphics_map.h"
#include "csb_v1_boot.h"

static int failures;

typedef struct {
    uint8_t pixels[33][128 * 128];
    int widths[33];
    int heights[33];
    int deny_graphic;
} TestHudSource0232;

typedef struct {
    const char *graphics_dat_path;
    uint8_t *pixels[33];
    int widths[33];
    int heights[33];
} RealHudSource0232;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        ++failures; \
    } \
} while (0)

static void put_be16(uint8_t *bytes, size_t offset, int value)
{
    bytes[offset] = (uint8_t)((unsigned)value >> 8);
    bytes[offset + 1u] = (uint8_t)value;
}

static void put_rect(uint8_t *bytes, size_t offset,
                     int x1, int x2, int y1, int y2)
{
    put_be16(bytes, offset, x1);
    put_be16(bytes, offset + 2u, x2);
    put_be16(bytes, offset + 4u, y1);
    put_be16(bytes, offset + 6u, y2);
}

static int write_standard_graphics_dat(const char *path, const uint8_t *graphic)
{
    FILE *file = fopen(path, "wb");
    uint8_t word[2];
    int index;

    if (!file) return 0;
    put_be16(word, 0u, 563);
    if (fwrite(word, 1, sizeof(word), file) != sizeof(word)) goto failed;
    for (index = 0; index < 563; ++index) {
        put_be16(word, 0u, index == 0x232 ?
                 CSB_V1_CSBWIN_LAYOUT_0232_DECODED_SIZE : 0);
        if (fwrite(word, 1, sizeof(word), file) != sizeof(word)) goto failed;
    }
    for (index = 0; index < 563; ++index) {
        put_be16(word, 0u, index == 0x232 ?
                 CSB_V1_CSBWIN_LAYOUT_0232_DECODED_SIZE : 0);
        if (fwrite(word, 1, sizeof(word), file) != sizeof(word)) goto failed;
    }
    if (fwrite(graphic, 1, CSB_V1_CSBWIN_LAYOUT_0232_DECODED_SIZE, file) !=
        CSB_V1_CSBWIN_LAYOUT_0232_DECODED_SIZE) goto failed;
    fclose(file);
    return 1;
failed:
    fclose(file);
    return 0;
}

/* PC3.4 and Atari/CSBWin packages both commonly call this file
 * GRAPHICS.DAT.  The optional real-data lane is specifically for CSBWin's
 * big-endian 563-entry catalog; silently treating a PC package as that
 * catalog gives misleading failures when a broad data-root environment
 * variable is exported. */
static int is_csbwin_graphics_dat(const char *path)
{
    FILE *file;
    unsigned char count_bytes[2];

    if (!path || !path[0]) return 0;
    file = fopen(path, "rb");
    if (!file) return 0;
    if (fread(count_bytes, 1, sizeof(count_bytes), file) !=
        sizeof(count_bytes)) {
        fclose(file);
        return 0;
    }
    fclose(file);
    return (((unsigned int)count_bytes[0] << 8) | count_bytes[1]) == 563u;
}

static void check_real_layout(const char *path)
{
    CSB_V1_CSBWinLayout0232 layout;
    size_t index;

    if (!path || !path[0]) return;
    CHECK(csb_v1_csbwin_layout_0232_read_graphics_dat(path, &layout));
    if (!layout.valid) return;
    for (index = 0; index < 4u; ++index) {
        CHECK(csb_v1_csbwin_layout_0232_rect_is_screen_valid(
            &layout.party_direction[index]));
    }
    CHECK(csb_v1_csbwin_layout_0232_rect_is_screen_valid(&layout.eye_box));
    CHECK(csb_v1_csbwin_layout_0232_rect_is_screen_valid(&layout.mouth_box));
    CHECK(csb_v1_csbwin_layout_0232_rect_is_screen_valid(&layout.poison_box));
    CHECK(csb_v1_csbwin_layout_0232_rect_is_screen_valid(
        &layout.food_water_box));
    CHECK(csb_v1_csbwin_layout_0232_rect_is_screen_valid(
        &layout.movement_box));
    CHECK(csb_v1_csbwin_layout_0232_rect_is_screen_valid(&layout.magic_box));
    CHECK(layout.default_graphic_list[0] != 0u);
    CHECK(layout.default_graphic_list[
        CSB_V1_CSBWIN_LAYOUT_0232_DEFAULT_GRAPHIC_COUNT - 1u] != 0u);
}

static int resolve_test_hud_source(void *user_data, uint16_t graphic_index,
                                   const uint8_t **out_pixels,
                                   int *out_width, int *out_height)
{
    TestHudSource0232 *source = (TestHudSource0232 *)user_data;

    if (!source || graphic_index >= 33u ||
        source->deny_graphic == (int)graphic_index ||
        source->widths[graphic_index] <= 0 ||
        source->heights[graphic_index] <= 0) return 0;
    *out_pixels = source->pixels[graphic_index];
    *out_width = source->widths[graphic_index];
    *out_height = source->heights[graphic_index];
    return 1;
}

static void check_hud_composition(const CSB_V1_CSBWinHudMaterialPlan0232 *plan)
{
    TestHudSource0232 source;
    CSB_V1_CSBWinHudCompositionReceipt0232 receipt;
    uint8_t frame[320 * 200];
    uint8_t unchanged[320 * 200];
    size_t index;

    memset(&source, 0, sizeof(source));
    memset(frame, 0xa5, sizeof(frame));
    for (index = 0; index < plan->count; ++index) {
        const CSB_V1_CSBWinHudMaterial0232 *entry = &plan->entries[index];
        size_t pixel_count;
        source.widths[entry->graphic_index] = 128;
        source.heights[entry->graphic_index] = 128;
        pixel_count = (size_t)source.widths[entry->graphic_index] *
            source.heights[entry->graphic_index];
        memset(source.pixels[entry->graphic_index],
               (int)(0x20u + entry->graphic_index), pixel_count);
    }
    CHECK(csb_v1_csbwin_layout_0232_compose_hud(
        plan, resolve_test_hud_source, &source, frame, sizeof(frame), &receipt));
    CHECK(receipt.valid && receipt.material_count == plan->count &&
          receipt.source_hash != 0u && receipt.composed_hash != 0u);
    CHECK(frame[30u * 320u + 10u] == 28u + 0x20u);
    CHECK(frame[160u * 320u + 216u] == 9u + 0x20u);
    CHECK(frame[0] == 0xa5u);

    memcpy(unchanged, frame, sizeof(frame));
    source.deny_graphic = 13;
    memset(&receipt, 0, sizeof(receipt));
    CHECK(!csb_v1_csbwin_layout_0232_compose_hud(
        plan, resolve_test_hud_source, &source, frame, sizeof(frame), &receipt));
    CHECK(!receipt.valid && !memcmp(frame, unchanged, sizeof(frame)));
}

static int resolve_real_hud_source(void *user_data, uint16_t graphic_index,
                                   const uint8_t **out_pixels,
                                   int *out_width, int *out_height)
{
    RealHudSource0232 *source = (RealHudSource0232 *)user_data;
    CSB_V1_StartupGraphicDecodeReceipt_PC34 receipt;

    if (!source || !source->graphics_dat_path || graphic_index >= 33u) return 0;
    if (!source->pixels[graphic_index]) {
        memset(&receipt, 0, sizeof(receipt));
        if (!csb_v1_boot_decode_atari_st_graphics_dat_asset_pc34(
                source->graphics_dat_path, graphic_index,
                &source->pixels[graphic_index], &source->widths[graphic_index],
                &source->heights[graphic_index], &receipt) || !receipt.valid) {
            free(source->pixels[graphic_index]);
            source->pixels[graphic_index] = NULL;
            source->widths[graphic_index] = source->heights[graphic_index] = 0;
            return 0;
        }
    }
    *out_pixels = source->pixels[graphic_index];
    *out_width = source->widths[graphic_index];
    *out_height = source->heights[graphic_index];
    return 1;
}

static void check_real_hud_composition(const char *path)
{
    CSB_V1_CSBWinLayout0232 layout;
    CSB_V1_CSBWinHudMaterialPlan0232 plan;
    CSB_V1_CSBWinHudCompositionReceipt0232 receipt;
    RealHudSource0232 source;
    uint8_t frame[320 * 200];
    size_t index;

    if (!path || !path[0]) return;
    memset(&source, 0, sizeof(source));
    source.graphics_dat_path = path;
    CHECK(csb_v1_csbwin_layout_0232_read_graphics_dat(path, &layout));
    CHECK(csb_v1_csbwin_layout_0232_build_hud_material_plan(&layout, &plan));
    CHECK(csb_v1_csbwin_layout_0232_compose_hud(
        &plan, resolve_real_hud_source, &source, frame, sizeof(frame), &receipt));
    CHECK(receipt.valid && receipt.material_count == plan.count &&
          receipt.source_hash != 0u && receipt.composed_hash != 0u);
    CHECK(source.pixels[28] &&
          frame[(size_t)plan.entries[0].destination.y1 * 320u +
                plan.entries[0].destination.x1] == source.pixels[28][0]);
    CHECK(source.pixels[9] &&
          frame[(size_t)plan.entries[9].destination.y1 * 320u +
                plan.entries[9].destination.x1] == source.pixels[9][0]);
    for (index = 0; index < 33u; ++index) free(source.pixels[index]);
}

static void check_real_viewport_wall_catalog(const char *path)
{
    uint16_t graphic_index;
    unsigned char *pixels;
    CSB_V1_StartupGraphicDecodeReceipt_PC34 receipt;
    int width;
    int height;
    unsigned int wall_set;
    unsigned int slot;

    if (!path || !path[0]) return;
    for (wall_set = 0u; wall_set < 4u; ++wall_set) {
        for (slot = 0u; slot < CSB_V1_CSBWIN_WALLSET_GRAPHIC_COUNT; ++slot) {
            pixels = NULL;
            width = height = 0;
            memset(&receipt, 0, sizeof(receipt));
            CHECK(csb_v1_csbwin_viewport_graphic_index((uint16_t)wall_set,
                (uint16_t)slot, &graphic_index));
            CHECK(csb_v1_boot_decode_atari_st_graphics_dat_asset_pc34(path,
                graphic_index, &pixels, &width, &height, &receipt));
            CHECK(receipt.valid && pixels && width > 0 && height > 0);
            free(pixels);
        }
    }
    for (wall_set = 0u; wall_set < 4u; ++wall_set) {
        int ceiling;
        for (ceiling = 0; ceiling <= 1; ++ceiling) {
            pixels = NULL;
            width = height = 0;
            memset(&receipt, 0, sizeof(receipt));
            CHECK(csb_v1_csbwin_floor_ceiling_graphic_index(
                (uint16_t)wall_set, ceiling, &graphic_index));
            CHECK(csb_v1_boot_decode_atari_st_graphics_dat_asset_pc34(path,
                graphic_index, &pixels, &width, &height, &receipt));
            CHECK(receipt.valid && pixels && width > 0 && height > 0);
            free(pixels);
        }
    }
}

static void check_real_viewport_projection_layout(const char *path)
{
    CSB_V1_CSBWinViewportLayout022e layout;
    unsigned int wall;
    unsigned int family;
    unsigned int state;

    if (!path || !path[0]) return;
    CHECK(csb_v1_csbwin_viewport_layout_022e_read_graphics_dat(path, &layout));
    if (!layout.valid) return;
    /* Current original Atari-CSBWin 0x22e: the far-centre wall and F0 local
     * cell prove both the packed source coordinates and the no-source case. */
    CHECK(layout.rectangles[0].x1 == 74u && layout.rectangles[0].x2 == 149u &&
          layout.rectangles[0].source_stride == 64u &&
          layout.rectangles[0].source_x == 18u);
    CHECK(layout.rectangles[9].x1 == 0u && layout.rectangles[9].x2 == 223u &&
          layout.rectangles[9].source_stride == 0u &&
          layout.rectangles[9].source_height == 0u);
    /* CSBWin Data.h puts DoorRectsF1R1..DoorRectsF3L1, then track/frame
     * RectPos records directly before wallRectangles.  Reading them from the
     * original 0x22e record prevents later render work from inventing door
     * projection geometry. */
    for (family = 0u;
         family < CSB_V1_CSBWIN_LAYOUT_022E_DOOR_RECTANGLE_FAMILY_COUNT;
         ++family) {
        for (state = 0u;
             state < CSB_V1_CSBWIN_LAYOUT_022E_DOOR_RECTANGLE_STATE_COUNT;
             ++state) {
            CHECK(csb_v1_csbwin_viewport_projection_rectangle_is_valid(
                &layout.door_rectangles[family][state]));
        }
    }
    for (wall = 0u; wall < CSB_V1_CSBWIN_LAYOUT_022E_DOOR_TRACK_RECTANGLE_COUNT;
         ++wall) {
        CHECK(csb_v1_csbwin_viewport_projection_rectangle_is_valid(
            &layout.door_track_rectangles[wall]));
    }
    for (wall = 0u; wall < CSB_V1_CSBWIN_LAYOUT_022E_DOOR_FRAME_RECTANGLE_COUNT;
         ++wall) {
        CHECK(csb_v1_csbwin_viewport_projection_rectangle_is_valid(
            &layout.door_frame_rectangles[wall]));
    }
    for (wall = 0u; wall < CSB_V1_CSBWIN_VIEWPORT_WALL_COUNT; ++wall) {
        uint8_t rectangle_index = UINT8_MAX;
        CHECK(csb_v1_csbwin_viewport_wall_projection_rectangle(
            (CSB_V1_CSBWinViewportWall)wall, &rectangle_index));
        CHECK(rectangle_index < CSB_V1_CSBWIN_VIEWPORT_WALL_COUNT);
        CHECK(csb_v1_csbwin_viewport_projection_rectangle_is_valid(
            &layout.rectangles[rectangle_index]));
    }
}

static void check_viewport_wall_plan(
    const CSB_V1_CSBWinViewportLayout022e *layout, uint16_t wall_set)
{
    CSB_V1_CSBWinViewportWallPlan plan;
    size_t draw;

    CHECK(csb_v1_csbwin_viewport_build_wall_plan(wall_set, layout, &plan));
    CHECK(plan.valid && plan.count == CSB_V1_CSBWIN_VIEWPORT_WALL_DRAW_COUNT);
    for (draw = 0u; draw < plan.count; ++draw) {
        const CSB_V1_CSBWinViewportWallDraw *entry = &plan.draws[draw];
        uint8_t rectangle_index = UINT8_MAX;
        uint16_t graphic_index = 0u;
        int mirrored = 0;

        CHECK(entry->wall != CSB_V1_CSBWIN_VIEWPORT_WALL_F0);
        CHECK(csb_v1_csbwin_viewport_wall_projection_rectangle(
            entry->wall, &rectangle_index));
        CHECK(csb_v1_csbwin_viewport_wall_source(
            wall_set, entry->wall, &graphic_index, &mirrored));
        CHECK(entry->graphic_index == graphic_index &&
              entry->mirrored == mirrored &&
              !memcmp(&entry->projection, &layout->rectangles[rectangle_index],
                      sizeof(entry->projection)));
    }
    CHECK(plan.draws[0].wall == CSB_V1_CSBWIN_VIEWPORT_WALL_F3L2 &&
          plan.draws[0].graphic_index ==
              CSB_V1_CSBWIN_WALLSET_FIRST_GRAPHIC + wall_set * 13u + 12u);
    CHECK(plan.draws[4].wall == CSB_V1_CSBWIN_VIEWPORT_WALL_F3R2 &&
          plan.draws[4].mirrored);
    CHECK(plan.draws[plan.count - 1u].wall ==
          CSB_V1_CSBWIN_VIEWPORT_WALL_F0R1);
}

static void check_viewport_door_plans(
    const CSB_V1_CSBWinViewportLayout022e *layout)
{
    CSB_V1_CSBWinDoorFramePlan frame_plan;
    const CSB_V1_CSBWinViewportProjectionRectangle *first;
    const CSB_V1_CSBWinViewportProjectionRectangle *second;

    CHECK(csb_v1_csbwin_viewport_door_panel_projections(
        layout, CSB_V1_CSBWIN_DOOR_PANEL_F2, 0u, 0, &first, &second));
    CHECK(!first && !second);
    CHECK(csb_v1_csbwin_viewport_door_panel_projections(
        layout, CSB_V1_CSBWIN_DOOR_PANEL_F2, 2u, 1, &first, &second));
    CHECK(first == &layout->door_rectangles[CSB_V1_CSBWIN_DOOR_PANEL_F2][2] &&
          !second);
    CHECK(csb_v1_csbwin_viewport_door_panel_projections(
        layout, CSB_V1_CSBWIN_DOOR_PANEL_F2, 2u, 0, &first, &second));
    CHECK(first == &layout->door_rectangles[CSB_V1_CSBWIN_DOOR_PANEL_F2][5] &&
          second == &layout->door_rectangles[CSB_V1_CSBWIN_DOOR_PANEL_F2][8]);
    CHECK(csb_v1_csbwin_viewport_door_panel_projections(
        layout, CSB_V1_CSBWIN_DOOR_PANEL_F3, 5u, 0, &first, &second));
    CHECK(first == &layout->door_rectangles[CSB_V1_CSBWIN_DOOR_PANEL_F3][0] &&
          !second);
    CHECK(!csb_v1_csbwin_viewport_door_panel_projections(
        layout, CSB_V1_CSBWIN_DOOR_PANEL_COUNT, 1u, 0, &first, &second));

    CHECK(csb_v1_csbwin_viewport_build_door_frame_plan(
        layout, CSB_V1_CSBWIN_DOOR_PANEL_F1, &frame_plan));
    CHECK(frame_plan.valid && frame_plan.count == 3u &&
          frame_plan.draws[0].bitmap_slot == 6u &&
          frame_plan.draws[1].bitmap_slot == 2u &&
          frame_plan.draws[2].bitmap_slot == 0u);
    CHECK(csb_v1_csbwin_viewport_build_door_frame_plan(
        layout, CSB_V1_CSBWIN_DOOR_PANEL_F2, &frame_plan));
    CHECK(frame_plan.count == 3u && frame_plan.draws[0].bitmap_slot == 7u &&
          frame_plan.draws[2].bitmap_slot == 3u && frame_plan.draws[2].mirrored);
    CHECK(csb_v1_csbwin_viewport_build_door_frame_plan(
        layout, CSB_V1_CSBWIN_DOOR_PANEL_F3R1, &frame_plan));
    CHECK(frame_plan.count == 1u && frame_plan.draws[0].bitmap_slot == 5u &&
          frame_plan.draws[0].mirrored);
}

static void check_wall_projection_blit(
    const CSB_V1_CSBWinViewportLayout022e *layout)
{
    uint8_t source_pixels[48 * 64];
    uint8_t destination[224 * 136];
    uint8_t *packed = NULL;
    size_t packed_count = 0u;
    CSB_V1_CSBWinPlanarBitmap source;
    CSB_V1_CSBWinViewportProjectionRectangle f0;
    const CSB_V1_CSBWinViewportProjectionRectangle *projection;
    int x;
    int y;

    memset(source_pixels, 0, sizeof(source_pixels));
    for (y = 0; y < 64; ++y) {
        for (x = 0; x < 48; ++x) {
            source_pixels[y * 48 + x] = (uint8_t)((x + y) & 15);
        }
    }
    projection = &layout->rectangles[13];
    source_pixels[(size_t)projection->source_y * 48u + projection->source_x] =
        10u;
    CHECK(csb_v1_csbwin_planar_bitmap_pack_indexed(
        source_pixels, 48u, 64u, &packed, &packed_count));
    memset(&source, 0, sizeof(source));
    source.bytes = packed;
    source.width = 48u;
    source.height = 64u;
    source.byte_stride = 24u;
    CHECK(packed_count == 24u * 64u);
    memset(destination, 0xa5, sizeof(destination));
    CHECK(csb_v1_csbwin_planar_bitmap_blit_wall_projection(
        &source, projection, 0, destination, 224, 136, 224));
    CHECK(destination[(size_t)projection->y1 * 224u + projection->x1] ==
          0xa5u);
    CHECK(destination[(size_t)projection->y1 * 224u + projection->x1 + 1u] ==
          source_pixels[(size_t)projection->source_y * 48u +
                        projection->source_x + 1u]);
    memset(destination, 0xa5, sizeof(destination));
    CHECK(csb_v1_csbwin_planar_bitmap_blit_wall_projection(
        &source, projection, 1, destination, 224, 136, 224));
    CHECK(destination[(size_t)projection->y1 * 224u + projection->x1] ==
          source_pixels[(size_t)projection->source_y * 48u +
                        projection->source_x +
                        (projection->x2 - projection->x1)]);
    f0 = layout->rectangles[9];
    f0.source_stride = f0.source_height = f0.source_x = f0.source_y = 0u;
    CHECK(!csb_v1_csbwin_planar_bitmap_blit_wall_projection(
        &source, &f0, 0, destination, 224, 136, 224));
    free(packed);
}

static void check_real_viewport_wall_plan(const char *path)
{
    CSB_V1_CSBWinViewportLayout022e layout;
    unsigned int wall_set;

    if (!path || !path[0]) return;
    CHECK(csb_v1_csbwin_viewport_layout_022e_read_graphics_dat(path, &layout));
    if (!layout.valid) return;
    for (wall_set = 0u; wall_set < 4u; ++wall_set) {
        check_viewport_wall_plan(&layout, (uint16_t)wall_set);
    }
}

/* The DMCSB1 decoder exposes a convenient expanded indexed raster, while
 * CSBWin's viewport blitter addresses the same source through packed bytes.
 * Lock the reversible boundary against every standard wall/floor source, not
 * just a synthetic odd-width sample. */
static void check_real_viewport_planar_roundtrip(const char *path)
{
    unsigned int group;

    if (!path || !path[0]) return;
    for (group = 0u; group < 60u; ++group) {
        uint16_t graphic_index = 0u;
        unsigned char *pixels = NULL;
        uint8_t *packed = NULL;
        size_t packed_count = 0u;
        CSB_V1_CSBWinPlanarBitmap bitmap;
        CSB_V1_StartupGraphicDecodeReceipt_PC34 receipt;
        int width = 0;
        int height = 0;
        size_t pixel;

        if (group < 52u) {
            CHECK(csb_v1_csbwin_viewport_graphic_index(
                (uint16_t)(group / CSB_V1_CSBWIN_WALLSET_GRAPHIC_COUNT),
                (uint16_t)(group % CSB_V1_CSBWIN_WALLSET_GRAPHIC_COUNT),
                &graphic_index));
        } else {
            CHECK(csb_v1_csbwin_floor_ceiling_graphic_index(
                (uint16_t)((group - 52u) / 2u), (int)(group & 1u),
                &graphic_index));
        }
        memset(&receipt, 0, sizeof(receipt));
        CHECK(csb_v1_boot_decode_atari_st_graphics_dat_asset_pc34(path,
            graphic_index, &pixels, &width, &height, &receipt));
        CHECK(pixels && receipt.valid && width > 0 && height > 0);
        if (!pixels || width <= 0 || height <= 0) goto next;
        CHECK(csb_v1_csbwin_planar_bitmap_pack_indexed(pixels,
            (uint16_t)width, (uint16_t)height, &packed, &packed_count));
        memset(&bitmap, 0, sizeof(bitmap));
        bitmap.bytes = packed;
        bitmap.width = (uint16_t)width;
        bitmap.height = (uint16_t)height;
        bitmap.byte_stride = (uint16_t)(((width + 15) / 16) * 8);
        CHECK(packed_count == (size_t)bitmap.byte_stride * bitmap.height);
        for (pixel = 0u; packed && pixel < (size_t)width * height; ++pixel) {
            uint8_t color = 0u;
            CHECK(csb_v1_csbwin_planar_bitmap_pixel_at(&bitmap,
                (uint16_t)(pixel % (size_t)width),
                (uint16_t)(pixel / (size_t)width), &color));
            CHECK(color == pixels[pixel]);
        }
next:
        free(packed);
        free(pixels);
    }
}

int main(void)
{
    uint8_t graphic[CSB_V1_CSBWIN_LAYOUT_0232_DECODED_SIZE];
    CSB_V1_CSBWinLayout0232 layout;
    CSB_V1_CSBWinHudMaterialPlan0232 plan;
    char path[512];
    const char *tmpdir;
    const char *real_graphics_dat;
    int index;

    {
        uint16_t panel_graphic = 0u;
        CHECK(csb_v1_csbwin_door_panel_graphic_index(0u, 0u, 2u,
                                                      &panel_graphic) &&
              panel_graphic == 108u);
        CHECK(csb_v1_csbwin_door_panel_graphic_index(0u, 0u, 1u,
                                                      &panel_graphic) &&
              panel_graphic == 109u);
        CHECK(csb_v1_csbwin_door_panel_graphic_index(0u, 0u, 0u,
                                                      &panel_graphic) &&
              panel_graphic == 110u);
        CHECK(csb_v1_csbwin_door_panel_graphic_index(15u, 1u, 2u,
                                                      &panel_graphic) &&
              panel_graphic == 153u);
        CHECK(csb_v1_csbwin_door_panel_graphic_index(15u, 1u, 0u,
                                                      &panel_graphic) &&
              panel_graphic == 155u);
        CHECK(!csb_v1_csbwin_door_panel_graphic_index(16u, 0u, 0u,
                                                       &panel_graphic));
        CHECK(!csb_v1_csbwin_door_panel_graphic_index(0u, 2u, 0u,
                                                       &panel_graphic));
        CHECK(!csb_v1_csbwin_door_panel_graphic_index(0u, 0u, 3u,
                                                       &panel_graphic));
    }

    {
        uint8_t viewport_layout[CSB_V1_CSBWIN_LAYOUT_022E_DECODED_SIZE];
        CSB_V1_CSBWinViewportLayout022e decoded_layout;
        memset(viewport_layout, 0, sizeof(viewport_layout));
        for (index = 0; index < CSB_V1_CSBWIN_VIEWPORT_WALL_COUNT; ++index) {
            uint8_t *rect = viewport_layout +
                CSB_V1_CSBWIN_LAYOUT_022E_WALL_RECTANGLE_OFFSET +
                (size_t)index * 8u;
            rect[0] = (uint8_t)(10 + index);
            rect[1] = (uint8_t)(20 + index);
            rect[2] = 30u;
            rect[3] = 40u;
            rect[4] = 24u;
            rect[5] = 64u;
            rect[6] = 4u;
            rect[7] = 2u;
        }
        CHECK(csb_v1_csbwin_viewport_layout_022e_decode(
            viewport_layout, sizeof(viewport_layout), &decoded_layout));
        CHECK(decoded_layout.valid && decoded_layout.rectangles[0].x1 == 10u &&
              decoded_layout.rectangles[13].x2 == 33u &&
              decoded_layout.rectangles[6].source_stride == 24u);
        check_viewport_wall_plan(&decoded_layout, 2u);
        check_viewport_door_plans(&decoded_layout);
        check_wall_projection_blit(&decoded_layout);
        viewport_layout[CSB_V1_CSBWIN_LAYOUT_022E_WALL_RECTANGLE_OFFSET + 4u] = 0u;
        CHECK(!csb_v1_csbwin_viewport_layout_022e_decode(
            viewport_layout, sizeof(viewport_layout), &decoded_layout));
        CHECK(!csb_v1_csbwin_viewport_layout_022e_decode(
            viewport_layout, sizeof(viewport_layout) - 1u, &decoded_layout));
    }

    memset(graphic, 0, sizeof(graphic));
    for (index = 0; index < 4; ++index) {
        put_rect(graphic, 376u + (size_t)index * 8u,
                 10 + index * 20, 29 + index * 20, 30, 45);
    }
    put_rect(graphic, 424u, 100, 115, 48, 59);
    put_rect(graphic, 432u, 100, 115, 64, 75);
    put_rect(graphic, 864u, 110, 157, 80, 91);
    put_rect(graphic, 872u, 110, 181, 92, 103);
    put_rect(graphic, 880u, 110, 181, 69, 80);
    put_rect(graphic, 904u, 110, 181, 69, 100);
    put_rect(graphic, 1802u, 216, 319, 88, 159);
    put_rect(graphic, 1818u, 216, 319, 160, 199);
    put_be16(graphic, 914u, 112);
    put_be16(graphic, 916u, 71);
    put_be16(graphic, 918u, 44);
    put_be16(graphic, 914u + 45u * 6u, 224);
    put_be16(graphic, 914u + 45u * 6u + 2u, 96);
    put_be16(graphic, 914u + 45u * 6u + 4u, 179);
    put_be16(graphic, 1218u, 0);
    put_be16(graphic, 1218u + 12u, 96);
    put_be16(graphic, 1280u, 0x0123);
    put_be16(graphic, 1280u + 2u * 16u + 30u, 0x0765);
    put_be16(graphic, 1534u, 1);
    put_be16(graphic, 1534u + 138u, 562);

    CHECK(csb_v1_csbwin_layout_0232_decode(graphic, sizeof(graphic), &layout));
    {
        uint16_t graphic_index = 0;
        uint8_t rectangle_index = 0u;
        int mirrored = 0;
        CHECK(csb_v1_csbwin_viewport_graphic_index(0u, 0u, &graphic_index) &&
              graphic_index == 77u);
        CHECK(csb_v1_csbwin_viewport_graphic_index(3u, 12u, &graphic_index) &&
              graphic_index == 128u);
        CHECK(!csb_v1_csbwin_viewport_graphic_index(16u, 0u, &graphic_index));
        CHECK(!csb_v1_csbwin_viewport_graphic_index(0u, 13u, &graphic_index));
        CHECK(csb_v1_csbwin_floor_ceiling_graphic_index(3u, 1, &graphic_index) &&
              graphic_index == 82u);
        CHECK(csb_v1_csbwin_packed_byte_width(224u, &graphic_index) &&
              graphic_index == 112u);
        CHECK(csb_v1_csbwin_packed_byte_width(45u, &graphic_index) &&
              graphic_index == 23u);
        CHECK(!csb_v1_csbwin_packed_byte_width(0u, &graphic_index));
        CHECK(csb_v1_csbwin_viewport_wall_source(0u,
            CSB_V1_CSBWIN_VIEWPORT_WALL_F3L2, &graphic_index, &mirrored) &&
              graphic_index == 89u && !mirrored);
        CHECK(csb_v1_csbwin_viewport_wall_source(0u,
            CSB_V1_CSBWIN_VIEWPORT_WALL_F3R2, &graphic_index, &mirrored) &&
              graphic_index == 89u && mirrored);
        CHECK(csb_v1_csbwin_viewport_wall_source(3u,
            CSB_V1_CSBWIN_VIEWPORT_WALL_F0R1, &graphic_index, &mirrored) &&
              graphic_index == 123u && !mirrored);
        {
            static const uint8_t expected_rectangles[
                CSB_V1_CSBWIN_VIEWPORT_WALL_COUNT] = {
                13u, 1u, 0u, 2u, 12u, 4u, 3u, 5u, 7u, 6u, 8u, 10u, 9u,
                11u
            };
            unsigned int wall;
            for (wall = 0u; wall < CSB_V1_CSBWIN_VIEWPORT_WALL_COUNT; ++wall) {
                uint8_t rectangle_index = UINT8_MAX;
                CHECK(csb_v1_csbwin_viewport_wall_projection_rectangle(
                    (CSB_V1_CSBWinViewportWall)wall, &rectangle_index));
                CHECK(rectangle_index == expected_rectangles[wall]);
            }
        }
        CHECK(!csb_v1_csbwin_viewport_wall_projection_rectangle(
            CSB_V1_CSBWIN_VIEWPORT_WALL_COUNT, &rectangle_index));
        CHECK(!csb_v1_csbwin_viewport_wall_source(0u,
            CSB_V1_CSBWIN_VIEWPORT_WALL_F0, &graphic_index, &mirrored));
        CHECK(!csb_v1_csbwin_viewport_wall_source(0u,
            CSB_V1_CSBWIN_VIEWPORT_WALL_COUNT, &graphic_index, &mirrored));
    }
    CHECK(layout.valid);
    CHECK(layout.party_direction[3].x1 == 70 && layout.party_direction[3].x2 == 89);
    CHECK(layout.eye_box.y1 == 48 && layout.mouth_box.y1 == 64);
    CHECK(layout.food_water_box.x2 == 181 && layout.poison_box.y2 == 91);
    CHECK(layout.food_label_box.y1 == 69 && layout.water_label_box.y1 == 92);
    CHECK(layout.movement_box.x1 == 216 && layout.movement_box.y2 == 159);
    CHECK(layout.magic_box.x2 == 319 && layout.magic_box.y1 == 160);
    CHECK(layout.icon_display[0].pixel_x == 112 &&
          layout.icon_display[0].pixel_y == 71 &&
          layout.icon_display[0].object_type == 44);
    CHECK(layout.icon_display[45].pixel_x == 224 &&
          layout.icon_display[45].object_type == 179);
    CHECK(layout.object_graphic_first[0] == 0 &&
          layout.object_graphic_first[6] == 96);
    CHECK(layout.default_graphic_list[0] == 1 &&
          layout.default_graphic_list[69] == 562);
    CHECK(layout.viewport_palettes[0][0] == 0x0123u &&
          layout.viewport_palettes[1][15] == 0x0765u);
    CHECK(csb_v1_csbwin_layout_0232_rect_is_screen_valid(&layout.movement_box));
    CHECK(!csb_v1_csbwin_layout_0232_rect_is_screen_valid(NULL));
    CHECK(csb_v1_csbwin_layout_0232_build_hud_material_plan(&layout, &plan));
    CHECK(plan.valid && plan.count == CSB_V1_CSBWIN_LAYOUT_0232_HUD_MATERIAL_COUNT);
    CHECK(plan.entries[0].graphic_index == 28u && plan.entries[0].source_x == 0u);
    CHECK(plan.entries[3].graphic_index == 28u && plan.entries[3].source_x == 57u);
    CHECK(plan.entries[4].graphic_index == 20u &&
          plan.entries[4].destination.y1 == 69);
    CHECK(plan.entries[9].graphic_index == 9u &&
          plan.entries[9].destination.y1 == 160);
    check_hud_composition(&plan);
    layout.magic_box.y2 = 200;
    CHECK(!csb_v1_csbwin_layout_0232_rect_is_screen_valid(&layout.magic_box));
    CHECK(!csb_v1_csbwin_layout_0232_decode(graphic, sizeof(graphic) - 1u, &layout));
    CHECK(!csb_v1_csbwin_layout_0232_decode(NULL, sizeof(graphic), &layout));
    CHECK(!csb_v1_csbwin_layout_0232_decode(graphic, sizeof(graphic), NULL));

    tmpdir = getenv("TMPDIR");
    if (!tmpdir || !tmpdir[0]) tmpdir = ".";
    snprintf(path, sizeof(path), "%s/firestaff-csbwin-layout-0232-%lu.dat",
             tmpdir, (unsigned long)clock());
    CHECK(write_standard_graphics_dat(path, graphic));
    CHECK(csb_v1_csbwin_layout_0232_read_graphics_dat(path, &layout));
    CHECK(layout.valid && layout.party_direction[0].x1 == 10);
    remove(path);

    /* The fixture proves format mechanics.  CI/users may additionally point
     * this at a legally supplied Atari/CSBWin GRAPHICS.DAT to prove that the
     * fixed offsets survive real source bytes. */
    real_graphics_dat = getenv("FIRESTAFF_CSBWIN_GRAPHICS_DAT");
    if (real_graphics_dat && real_graphics_dat[0] &&
        !is_csbwin_graphics_dat(real_graphics_dat)) {
        fprintf(stderr,
                "SKIP: FIRESTAFF_CSBWIN_GRAPHICS_DAT is not a CSBWin "
                "563-entry Atari GRAPHICS.DAT: %s\n",
                real_graphics_dat);
    } else {
        check_real_layout(real_graphics_dat);
        check_real_hud_composition(real_graphics_dat);
        check_real_viewport_wall_catalog(real_graphics_dat);
        check_real_viewport_projection_layout(real_graphics_dat);
        check_real_viewport_wall_plan(real_graphics_dat);
        check_real_viewport_planar_roundtrip(real_graphics_dat);
    }

    if (failures) return 1;
    puts("PASS: CSBWin GRAPHICS.DAT 0x232/0x22e layout decode");
    return 0;
}
