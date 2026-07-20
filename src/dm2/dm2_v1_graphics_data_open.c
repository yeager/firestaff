#include "dm2_v1_graphics_data_open.h"

#include "dm2_v1_weather_gdat.h"

#include <limits.h>
#include <string.h>

static uint32_t dm2_graphics_data_open_hash_step(uint32_t hash,
                                                 uint32_t value)
{
    hash ^= value;
    return hash * 16777619u;
}

static uint32_t dm2_graphics_data_open_hash_bytes(const uint8_t *bytes,
                                                  size_t size)
{
    uint32_t hash = 2166136261u;
    size_t i;

    for (i = 0u; i < size; ++i) {
        hash = dm2_graphics_data_open_hash_step(hash, bytes[i]);
    }
    return hash;
}

static int dm2_graphics_data_open_hash_image(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    uint32_t *io_palette_hash,
    uint32_t *io_pixel_hash)
{
    uint8_t palette16[16];
    uint32_t palette_hash = 0u;
    uint8_t *pixels;
    int width = 0;
    int height = 0;
    DM2_ImageFormat format = DM2_IMG_FMT_UNKNOWN;
    size_t pixel_count;

    if (!io_palette_hash || !io_pixel_hash) return 0;
    if (!dm2_v1_asset_load_image_local_palette(loader, category, index, field,
                                               palette16, &palette_hash) ||
        palette_hash == 0u) {
        return 0;
    }
    pixels = dm2_v1_asset_load_image_field(loader, category, index, field,
                                           &width, &height, &format);
    if (!pixels || width <= 0 || height <= 0 ||
        (format != DM2_IMG_FMT_IMG3 && format != DM2_IMG_FMT_U4)) {
        dm2_v1_asset_free_pixels(pixels);
        return 0;
    }
    pixel_count = (size_t)width * (size_t)height;
    if (pixel_count == 0u || pixel_count > UINT32_MAX) {
        dm2_v1_asset_free_pixels(pixels);
        return 0;
    }
    *io_palette_hash = dm2_graphics_data_open_hash_step(*io_palette_hash,
                                                        palette_hash);
    *io_pixel_hash = dm2_graphics_data_open_hash_step(
        *io_pixel_hash, dm2_graphics_data_open_hash_bytes(pixels, pixel_count));
    dm2_v1_asset_free_pixels(pixels);
    return 1;
}

int dm2_v1_GRAPHICS_DATA_OPEN_receipt(
    const uint8_t *graphics_dat,
    size_t graphics_dat_size,
    DM2_V1_GraphicsDataOpenReceipt *out_receipt)
{
    DM2_V1_AssetLoader loader;
    DM2_V1_InterfacePalette interface_palette;
    uint8_t *title_menu_pixels;
    int title_menu_width = 0;
    int title_menu_height = 0;
    DM2_ImageFormat title_menu_format = DM2_IMG_FMT_UNKNOWN;
    size_t title_menu_pixel_count;
    uint32_t hash = 2166136261u;
    uint32_t hand_palette_hash = 2166136261u;
    uint32_t hand_pixel_hash = 2166136261u;
    uint32_t environment_text_hash = 2166136261u;
    uint32_t environment_text_count = 0u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&loader, 0, sizeof(loader));
    if (!graphics_dat || graphics_dat_size == 0u ||
        graphics_dat_size > UINT32_MAX ||
        dm2_v1_asset_loader_init(&loader, graphics_dat, graphics_dat_size) != 0) {
        return 0;
    }

    if (!dm2_v1_asset_loader_validate_typed_graph(&loader) ||
        !dm2_v1_asset_load_interface_palette(
            &loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
            DM2_GDAT_INTERFACE_PALETTE_FIELD, &interface_palette) ||
        interface_palette.hash == 0u) {
        dm2_v1_asset_loader_free(&loader);
        return 0;
    }

    /* skproject SHOW_MENU_SCREEN consumes TITLE/0 field 4 as the static menu
     * surface.  The verified PC corpus stores that field as dtImage, so admit
     * only a real decoded source image here rather than a synthetic menu or a
     * raw-shape substitute. */
    title_menu_pixels = dm2_v1_asset_load_image_field(
        &loader, DM2_GDAT_CATEGORY_TITLE, 0, 4, &title_menu_width,
        &title_menu_height, &title_menu_format);
    if (!title_menu_pixels || title_menu_width <= 0 || title_menu_height <= 0 ||
        title_menu_format == DM2_IMG_FMT_UNKNOWN) {
        dm2_v1_asset_free_pixels(title_menu_pixels);
        dm2_v1_asset_loader_free(&loader);
        return 0;
    }
    title_menu_pixel_count =
        (size_t)title_menu_width * (size_t)title_menu_height;
    if (title_menu_pixel_count == 0u || title_menu_pixel_count > UINT32_MAX) {
        dm2_v1_asset_free_pixels(title_menu_pixels);
        dm2_v1_asset_loader_free(&loader);
        return 0;
    }

    for (int entry = 2; entry <= 5; ++entry) {
        if (!dm2_graphics_data_open_hash_image(
                &loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 4, entry,
                &hand_palette_hash, &hand_pixel_hash)) {
            dm2_v1_asset_free_pixels(title_menu_pixels);
            dm2_v1_asset_loader_free(&loader);
            return 0;
        }
        out_receipt->hud_hand_action_image_mask |=
            (uint32_t)(1u << (unsigned int)entry);
    }

    for (int graphicsset = 1; graphicsset <= 5; ++graphicsset) {
        for (unsigned int command = DM2_V1_WEATHER_BOLT_CMD_BASE;
             command <= DM2_V1_WEATHER_RAIN_STORM_CMD;
             ++command) {
            size_t text_size = 0u;
            const uint8_t *text = dm2_v1_asset_load_text_sized(
                &loader, DM2_GDAT_CATEGORY_ENVIRONMENT, graphicsset,
                (int)command, &text_size);
            if (text && text_size > 0u && text_size <= UINT32_MAX) {
                ++environment_text_count;
                environment_text_hash = dm2_graphics_data_open_hash_step(
                    environment_text_hash,
                    dm2_graphics_data_open_hash_bytes(text, text_size));
            }
        }
    }
    if (environment_text_count == 0u) {
        dm2_v1_asset_free_pixels(title_menu_pixels);
        dm2_v1_asset_loader_free(&loader);
        return 0;
    }

    out_receipt->gdat_version = loader.gdat_version;
    out_receipt->raw_data_count = loader.raw_data_count;
    out_receipt->entry_count = loader.entry_count;
    out_receipt->container_byte_count = (uint32_t)graphics_dat_size;
    out_receipt->typed_graph_hash = dm2_graphics_data_open_hash_step(
        dm2_graphics_data_open_hash_step(hash, loader.raw_data_count),
        loader.entry_count);
    out_receipt->interface_palette_hash = interface_palette.hash;
    out_receipt->title_menu_pixel_count = (uint32_t)title_menu_pixel_count;
    out_receipt->title_menu_hash =
        dm2_graphics_data_open_hash_bytes(title_menu_pixels,
                                          title_menu_pixel_count);
    out_receipt->hud_hand_action_palette_hash = hand_palette_hash;
    out_receipt->hud_hand_action_pixel_hash = hand_pixel_hash;
    out_receipt->environment_text_count = environment_text_count;
    out_receipt->environment_text_hash = environment_text_hash;

    hash = dm2_graphics_data_open_hash_step(hash,
                                            out_receipt->container_byte_count);
    hash = dm2_graphics_data_open_hash_step(hash,
                                            out_receipt->typed_graph_hash);
    hash = dm2_graphics_data_open_hash_step(hash,
                                            out_receipt->interface_palette_hash);
    hash = dm2_graphics_data_open_hash_step(hash,
                                            out_receipt->title_menu_hash);
    hash = dm2_graphics_data_open_hash_step(
        hash, out_receipt->hud_hand_action_palette_hash);
    hash = dm2_graphics_data_open_hash_step(
        hash, out_receipt->hud_hand_action_pixel_hash);
    hash = dm2_graphics_data_open_hash_step(hash,
                                            out_receipt->environment_text_hash);
    if (hash == 0u) {
        dm2_v1_asset_free_pixels(title_menu_pixels);
        dm2_v1_asset_loader_free(&loader);
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    out_receipt->admission_hash = hash;
    out_receipt->valid = 1;

    dm2_v1_asset_free_pixels(title_menu_pixels);
    dm2_v1_asset_loader_free(&loader);
    return 1;
}

const char *dm2_v1_GRAPHICS_DATA_OPEN_source_evidence(void)
{
    return "skproject/SKULLWIN/c_gdatfile.cpp GRAPHICS_DATA_OPEN and "
           "SKWIN/SkWinCore.cpp INIT/SHOW_MENU_SCREEN/QUERY_GDAT_TEXT: "
           "admit GRAPHICS.DAT only after typed GDAT graph, interface palette, "
           "TITLE/0 field-4 source image, INTERFACE_GENERAL/4 IMG3 palettes, and "
           "ENVIRONMENT dtText command bytes resolve from the same source.";
}
