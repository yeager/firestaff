#include "dm2_v1_hand_action_gdat.h"

#include <string.h>

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
