#ifndef FIRESTAFF_DM2_V1_GRAPHICS_DATA_OPEN_H
#define FIRESTAFF_DM2_V1_GRAPHICS_DATA_OPEN_H

#include "dm2_v1_asset_loader.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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
