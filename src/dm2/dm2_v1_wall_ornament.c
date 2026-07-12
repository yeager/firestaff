#include "dm2_v1_wall_ornament.h"
#include <string.h>

int dm2_v1_wall_ornament_receipt(const DM2_V1_AssetLoader *loader,
                                 uint8_t index,
                                 DM2_V1_WallOrnamentReceipt *out)
{
    uint16_t colorkey, position, no_flip, alcove, displacement;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    /* skproject SKWIN/SkWinCore.cpp DRAW_WALL_ORNATE: dtWordValue 04/05/07/0A
     * and dtImageOffset FD are all required before QUERY_TEMP_PICST. */
    if (!loader ||
        !dm2_v1_asset_load_word_value(loader, DM2_GDAT_CATEGORY_WALL_GFX, index, 0x04, &colorkey) ||
        !dm2_v1_asset_load_word_value(loader, DM2_GDAT_CATEGORY_WALL_GFX, index, 0x05, &position) ||
        !dm2_v1_asset_load_word_value(loader, DM2_GDAT_CATEGORY_WALL_GFX, index, 0x07, &no_flip) ||
        !dm2_v1_asset_load_word_value(loader, DM2_GDAT_CATEGORY_WALL_GFX, index, 0x0a, &alcove) ||
        !dm2_v1_asset_load_image_offset(loader, DM2_GDAT_CATEGORY_WALL_GFX, index, 0xfd, &displacement)) return 0;
    if (position > 24u || alcove > 3u) return 0;
    out->valid = 1; out->wall_gfx_index = index; out->colorkey = colorkey;
    out->position = position; out->do_not_flip = no_flip; out->alcove_type = alcove;
    out->item_inside_displacement = displacement; return 1;
}
