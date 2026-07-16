#include "dm2_v1_champion_portrait_gdat.h"

#include <string.h>

static uint32_t dm2_v1_champion_portrait_hash_step(uint32_t hash,
                                                    uint32_t value)
{
    hash ^= value;
    return hash * 16777619u;
}

int dm2_v1_champion_portrait_gdat_route(
    const DM2_V1_ChampionPortraitInput *input,
    DM2_V1_ChampionPortraitGdatRoute *out_route)
{
    if (out_route) memset(out_route, 0, sizeof(*out_route));
    if (!input || !out_route || !input->hero_type_source_bound ||
        input->hero_type !=
            (uint8_t)DM2_V1_CHAMPION_PORTRAIT_ORIGINAL_SKSAVE_HEROTYPE ||
        input->player_index >= DM2_V1_CHAMPION_PORTRAIT_PLAYER_COUNT) {
        return 0;
    }

    out_route->category = DM2_GDAT_CATEGORY_CHAMPIONS;
    out_route->hero_type = input->hero_type;
    out_route->field = DM2_V1_CHAMPION_PORTRAIT_GDAT_FIELD;
    out_route->rectno = (uint16_t)(DM2_V1_CHAMPION_PORTRAIT_RECT_BASE +
                                   input->player_index);
    return 1;
}

int dm2_v1_champion_portrait_gdat_receipt(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_ChampionPortraitInput *input,
    DM2_V1_ChampionPortraitGdatReceipt *out_receipt)
{
    DM2_V1_ChampionPortraitGdatRoute route;
    const uint8_t *raw;
    uint8_t *pixels;
    size_t raw_size = 0u;
    int width = 0;
    int height = 0;
    DM2_ImageFormat format = DM2_IMG_FMT_UNKNOWN;
    uint32_t palette_hash = 2166136261u;
    uint32_t pixels_hash = 2166136261u;
    uint32_t material_hash = 2166136261u;
    size_t pixel_count;
    size_t i;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!loader || !out_receipt ||
        !dm2_v1_champion_portrait_gdat_route(input, &route)) {
        return 0;
    }

    raw = dm2_v1_asset_load_typed_sized(loader, route.category,
                                         route.hero_type,
                                         DM2_GDAT_ENTRY_TYPE_IMAGE,
                                         route.field, &raw_size);
    /* QUERY_GDAT_IMAGE_LOCALPAL returns the IMG3 pointer only when its
     * four-bit local palette is present. */
    if (!raw || raw_size < 26u) return 0;

    pixels = dm2_v1_asset_load_image_field(loader, route.category,
                                            route.hero_type, route.field,
                                            &width, &height, &format);
    if (!pixels || width <= 0 || height <= 0 ||
        format == DM2_IMG_FMT_UNKNOWN) {
        dm2_v1_asset_free_pixels(pixels);
        return 0;
    }
    pixel_count = (size_t)width * (size_t)height;
    if (pixel_count > UINT32_MAX) {
        dm2_v1_asset_free_pixels(pixels);
        return 0;
    }

    memcpy(out_receipt->local_palette16, raw + raw_size - 16u, 16u);
    for (i = 0u; i < 16u; ++i) {
        palette_hash = dm2_v1_champion_portrait_hash_step(
            palette_hash, out_receipt->local_palette16[i]);
    }
    for (i = 0u; i < pixel_count; ++i) {
        pixels_hash = dm2_v1_champion_portrait_hash_step(pixels_hash,
                                                          pixels[i]);
    }
    dm2_v1_asset_free_pixels(pixels);

    material_hash = dm2_v1_champion_portrait_hash_step(material_hash,
                                                        route.category);
    material_hash = dm2_v1_champion_portrait_hash_step(material_hash,
                                                        route.hero_type);
    material_hash = dm2_v1_champion_portrait_hash_step(material_hash,
                                                        route.field);
    material_hash = dm2_v1_champion_portrait_hash_step(material_hash,
                                                        route.rectno);
    material_hash = dm2_v1_champion_portrait_hash_step(material_hash,
                                                        (uint32_t)width);
    material_hash = dm2_v1_champion_portrait_hash_step(material_hash,
                                                        (uint32_t)height);
    material_hash = dm2_v1_champion_portrait_hash_step(material_hash,
                                                        (uint32_t)format);
    material_hash = dm2_v1_champion_portrait_hash_step(material_hash,
                                                        palette_hash);
    material_hash = dm2_v1_champion_portrait_hash_step(material_hash,
                                                        pixels_hash);

    out_receipt->valid = 1;
    out_receipt->route = route;
    out_receipt->decoded_width = (uint16_t)width;
    out_receipt->decoded_height = (uint16_t)height;
    out_receipt->decoded_format = format;
    out_receipt->local_palette_hash = palette_hash;
    out_receipt->decoded_pixel_count = (uint32_t)pixel_count;
    out_receipt->decoded_pixels_hash = pixels_hash;
    out_receipt->material_hash = material_hash;
    return 1;
}
