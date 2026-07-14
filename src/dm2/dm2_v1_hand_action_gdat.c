#include "dm2_v1_hand_action_gdat.h"

#include <string.h>

static uint32_t dm2_v1_hand_action_hash_step(uint32_t hash, uint32_t value)
{
    hash ^= value;
    return hash * 16777619u;
}

static uint16_t dm2_v1_hand_action_le16(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static int dm2_v1_hand_action_is_c4_img3(const uint8_t *raw)
{
    int offset_y;

    if (!raw) return 0;
    offset_y = (int)((int16_t)dm2_v1_hand_action_le16(raw + 2u) >> 10);
    /* SKProject IMG3::Getpf reserves -32 for U4/U8 and 31 for C8.  The
     * remaining offset classes, including the hand-action records, are C4. */
    return offset_y != -32 && offset_y != 31;
}

static int dm2_v1_hand_action_image_metadata(
    const DM2_V1_AssetLoader *loader, const DM2_V1_HandActionGdatRoute *route,
    DM2_V1_HandActionImageMetadata *out)
{
    const uint8_t *raw;
    size_t raw_size = 0u;
    uint16_t graphicsset_offset = 0u;
    uint16_t image_offset = 0u;
    uint32_t hash = 2166136261u;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!loader || !route) return 0;
    raw = dm2_v1_asset_load_typed_sized(
        loader, route->category, route->subcategory,
        DM2_GDAT_ENTRY_TYPE_IMAGE, route->entry, &raw_size);
    if (!raw || raw_size < 10u) return 0;
    out->width = (uint16_t)(dm2_v1_hand_action_le16(raw) & 0x03ffu);
    out->height = (uint16_t)(dm2_v1_hand_action_le16(raw + 2u) & 0x03ffu);
    if (out->width == 0u || out->height == 0u ||
        !dm2_v1_hand_action_is_c4_img3(raw)) {
        memset(out, 0, sizeof(*out));
        return 0;
    }
    out->bits_per_pixel = 4u;
    /* QUERY_GDAT_SUMMARY_IMAGE treats missing category/image offsets as the
     * original zero offset; that is not a substitute placement. */
    (void)dm2_v1_asset_load_image_offset(loader, route->category,
                                         route->subcategory, 0xfe,
                                         &graphicsset_offset);
    (void)dm2_v1_asset_load_image_offset(loader, route->category,
                                         route->subcategory, route->entry,
                                         &image_offset);
    out->query_offset_x = (int16_t)((int8_t)(graphicsset_offset >> 8) +
                                    (int8_t)(image_offset >> 8));
    out->query_offset_y = (int16_t)((int8_t)graphicsset_offset +
                                    (int8_t)image_offset);
    hash = dm2_v1_hand_action_hash_step(hash, out->width);
    hash = dm2_v1_hand_action_hash_step(hash, out->height);
    hash = dm2_v1_hand_action_hash_step(hash, out->bits_per_pixel);
    hash = dm2_v1_hand_action_hash_step(hash, graphicsset_offset);
    hash = dm2_v1_hand_action_hash_step(hash, image_offset);
    out->metadata_hash = hash;
    return hash != 0u;
}

static int dm2_v1_hand_action_local_palette(
    const DM2_V1_AssetLoader *loader, const DM2_V1_HandActionGdatRoute *route,
    uint8_t out_palette16[16], uint32_t *out_hash)
{
    const uint8_t *raw;
    size_t raw_size = 0u;
    uint32_t hash = 2166136261u;

    if (out_hash) *out_hash = 0u;
    if (!out_palette16 || !loader || !route) return 0;
    memset(out_palette16, 0, 16u);
    raw = dm2_v1_asset_load_typed_sized(
        loader, route->category, route->subcategory,
        DM2_GDAT_ENTRY_TYPE_IMAGE, route->entry, &raw_size);
    if (!raw || raw_size < 26u || !dm2_v1_hand_action_is_c4_img3(raw)) {
        return 0;
    }
    memcpy(out_palette16, raw + raw_size - 16u, 16u);
    for (int i = 0; i < 16; ++i) {
        hash = dm2_v1_hand_action_hash_step(hash, out_palette16[i]);
    }
    if (hash == 0u) return 0;
    if (out_hash) *out_hash = hash;
    return 1;
}

int dm2_v1_hand_action_gdat_route(const DM2_V1_HandActionInput *input,
                                  DM2_V1_HandActionGdatRoute *out_route)
{
    int rect_base;

    if (out_route) {
        memset(out_route, 0, sizeof(*out_route));
    }
    if (!input || !out_route || input->possession_index < 0 ||
        input->possession_index > 1 || input->left_or_right < 0 ||
        input->left_or_right > 1 || input->player_position < 0 ||
        input->player_position > 3 || input->party_direction < 0 ||
        input->party_direction > 3) {
        return 0;
    }

    rect_base = input->possession_index == 1 ? 0x46 : 0x4a;
    out_route->category = DM2_GDAT_CATEGORY_INTERFACE_GENERAL;
    out_route->subcategory = 4u;
    out_route->entry = (uint8_t)((input->possession_index << 1) +
                                 input->left_or_right + 2);
    out_route->rectno = (uint8_t)(rect_base +
        ((input->player_position + 4 - input->party_direction) & 3));
    return 1;
}

uint8_t *dm2_v1_hand_action_gdat_load_image(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_HandActionInput *input,
    DM2_V1_HandActionGdatRoute *out_route,
    int *out_width,
    int *out_height,
    DM2_ImageFormat *out_format)
{
    DM2_V1_HandActionGdatRoute route;

    if (out_width) *out_width = 0;
    if (out_height) *out_height = 0;
    if (out_format) *out_format = DM2_IMG_FMT_UNKNOWN;
    if (out_route) memset(out_route, 0, sizeof(*out_route));
    if (!dm2_v1_hand_action_gdat_route(input, &route)) {
        return NULL;
    }
    if (out_route) *out_route = route;

    return dm2_v1_asset_load_image_field(loader, route.category,
                                         route.subcategory, route.entry,
                                         out_width, out_height, out_format);
}

int dm2_v1_hand_action_gdat_receipt(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_HandActionInput *input,
    DM2_V1_HandActionGdatReceipt *out_receipt)
{
    uint8_t *pixels;
    int width = 0;
    int height = 0;
    DM2_ImageFormat format = DM2_IMG_FMT_UNKNOWN;
    size_t pixel_count;
    uint32_t hash = 2166136261u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!loader || !dm2_v1_hand_action_gdat_route(input, &out_receipt->route) ||
        !dm2_v1_hand_action_image_metadata(
            loader, &out_receipt->route, &out_receipt->image_metadata) ||
        !dm2_v1_hand_action_local_palette(
            loader, &out_receipt->route, out_receipt->local_palette16,
            &out_receipt->local_palette_hash) ||
        out_receipt->local_palette_hash == 0u) {
        return 0;
    }

    pixels = dm2_v1_asset_load_image_field(
        loader, out_receipt->route.category, out_receipt->route.subcategory,
        out_receipt->route.entry, &width, &height, &format);
    if (!pixels || width <= 0 || height <= 0 ||
        width != (int)out_receipt->image_metadata.width ||
        height != (int)out_receipt->image_metadata.height ||
        (format != DM2_IMG_FMT_IMG3 && format != DM2_IMG_FMT_U4)) {
        dm2_v1_asset_free_pixels(pixels);
        return 0;
    }
    pixel_count = (size_t)width * (size_t)height;
    if (pixel_count == 0u || pixel_count > UINT32_MAX) {
        dm2_v1_asset_free_pixels(pixels);
        return 0;
    }
    for (size_t i = 0u; i < pixel_count; ++i) {
        hash = dm2_v1_hand_action_hash_step(hash, pixels[i]);
    }
    dm2_v1_asset_free_pixels(pixels);
    if (hash == 0u) return 0;

    out_receipt->decoded_width = (uint16_t)width;
    out_receipt->decoded_height = (uint16_t)height;
    out_receipt->decoded_format = format;
    out_receipt->decoded_pixel_count = (uint32_t)pixel_count;
    out_receipt->decoded_pixels_hash = hash;
    hash = dm2_v1_hand_action_hash_step(hash,
                                        out_receipt->route.rectno);
    hash = dm2_v1_hand_action_hash_step(
        hash, out_receipt->image_metadata.metadata_hash);
    hash = dm2_v1_hand_action_hash_step(hash,
                                        out_receipt->local_palette_hash);
    hash = dm2_v1_hand_action_hash_step(hash,
                                        out_receipt->decoded_pixel_count);
    if (hash == 0u) return 0;
    out_receipt->material_hash = hash;
    out_receipt->valid = 1;
    return 1;
}
