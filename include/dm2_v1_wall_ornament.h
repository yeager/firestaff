#ifndef DM2_V1_WALL_ORNAMENT_H
#define DM2_V1_WALL_ORNAMENT_H

#include <stdint.h>
#include "dm2_v1_asset_loader.h"

typedef struct {
    int valid;
    uint8_t wall_gfx_index;
    uint16_t colorkey;
    uint16_t position;
    uint16_t do_not_flip;
    uint16_t alcove_type;
    uint16_t item_inside_displacement;
} DM2_V1_WallOrnamentReceipt;

/* Source: skproject DRAW_WALL_ORNATE. This binds only the original GDAT
 * scalar inputs; it deliberately performs no image lookup or rendering. */
int dm2_v1_wall_ornament_receipt(const DM2_V1_AssetLoader *loader,
                                 uint8_t wall_gfx_index,
                                 DM2_V1_WallOrnamentReceipt *out_receipt);
#endif
