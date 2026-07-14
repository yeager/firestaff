#ifndef FIRESTAFF_DM2_V1_HAND_ACTION_GDAT_H
#define FIRESTAFF_DM2_V1_HAND_ACTION_GDAT_H

#include "dm2_v1_asset_loader.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* skproject/SKWINSPX/src/v4/skguidrw.cpp::DRAW_HAND_ACTION_ICONS
 * (0x29EE:026C) selects INTERFACE_GENERAL/4/dtImage entries 2..5 and then
 * expands the corresponding rect from the interface layout table. */
typedef struct {
    int possession_index;
    int left_or_right;
    int player_position;
    int party_direction;
} DM2_V1_HandActionInput;

typedef struct {
    uint8_t category;
    uint8_t subcategory;
    uint8_t entry;
    uint8_t rectno;
} DM2_V1_HandActionGdatRoute;

/* Resolve only valid source addresses.  Invalid input produces no route. */
int dm2_v1_hand_action_gdat_route(const DM2_V1_HandActionInput *input,
                                  DM2_V1_HandActionGdatRoute *out_route);

/* Resolve and decode the selected original GDAT image.  This function never
 * manufactures a hand-action bitmap: it returns NULL when the typed source
 * entry cannot be decoded. */
uint8_t *dm2_v1_hand_action_gdat_load_image(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_HandActionInput *input,
    DM2_V1_HandActionGdatRoute *out_route,
    int *out_width,
    int *out_height,
    DM2_ImageFormat *out_format);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_HAND_ACTION_GDAT_H */
