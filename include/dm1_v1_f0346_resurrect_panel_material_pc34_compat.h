#ifndef FIRESTAFF_DM1_V1_F0346_RESURRECT_PANEL_MATERIAL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0346_RESURRECT_PANEL_MATERIAL_PC34_COMPAT_H

#include <stdint.h>

#include "dm1_v1_f0355_inventory_material_pc34_compat.h"

enum {
    DM1_V1_F0346_C040_RESURRECT_PANEL_PC34 = 40,
    DM1_V1_F0346_PANEL_WIDTH_PC34 = 224,
    DM1_V1_F0346_PANEL_HEIGHT_PC34 = 136
};

typedef struct DM1_V1_F0346ResurrectPanelMaterialReceiptPc34 {
    int valid;
    int suppressSyntheticFallback;
    int graphicIndex;
    int width;
    int height;
    uint32_t materialFingerprint;
    const char *sourceAnchor;
} DM1_V1_F0346ResurrectPanelMaterialReceiptPc34;

/* F0346 has no text path: its C040 panel may be published only from an
 * authenticated original GRAPHICS.DAT surface. */
int dm1_v1_f0346_resurrect_panel_material_receipt_pc34(
    const DM1_V1_F0355SourceSurfacePc34 *surfaces,
    int surfaceCount,
    DM1_V1_F0346ResurrectPanelMaterialReceiptPc34 *outReceipt);

#endif
