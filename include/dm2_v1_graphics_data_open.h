#ifndef FIRESTAFF_DM2_V1_GRAPHICS_DATA_OPEN_H
#define FIRESTAFF_DM2_V1_GRAPHICS_DATA_OPEN_H

#include "dm2_v1_asset_loader.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int valid;
    uint16_t gdat_version;
    uint16_t raw_data_count;
    uint16_t entry_count;
    uint32_t container_byte_count;
    uint32_t typed_graph_hash;
    uint32_t interface_palette_hash;
    uint32_t title_menu_pixel_count;
    uint32_t title_menu_hash;
    uint32_t hud_hand_action_image_mask;
    uint32_t hud_hand_action_palette_hash;
    uint32_t hud_hand_action_pixel_hash;
    uint32_t environment_text_count;
    uint32_t environment_text_hash;
    uint32_t admission_hash;
} DM2_V1_GraphicsDataOpenReceipt;

/* Source-named DM2 GRAPHICS_DATA_OPEN admission boundary.
 * This does not open host files or fabricate fallback material.  The caller
 * supplies the candidate GRAPHICS.DAT bytes; Firestaff admits them only after
 * the skproject GDAT graph and required HUD/dungeon material anchors resolve
 * through typed GDAT queries. */
int dm2_v1_GRAPHICS_DATA_OPEN_receipt(
    const uint8_t *graphics_dat,
    size_t graphics_dat_size,
    DM2_V1_GraphicsDataOpenReceipt *out_receipt);

const char *dm2_v1_GRAPHICS_DATA_OPEN_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_GRAPHICS_DATA_OPEN_H */
