#include "dm1_v1_f0346_resurrect_panel_material_pc34_compat.h"

#include <string.h>

int dm1_v1_f0346_resurrect_panel_material_receipt_pc34(
    const DM1_V1_F0355SourceSurfacePc34 *surfaces,
    int surfaceCount,
    DM1_V1_F0346ResurrectPanelMaterialReceiptPc34 *outReceipt)
{
    int i;
    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    outReceipt->sourceAnchor =
        "ReDMCSB PANEL.C F0346:1619-1637; "
        "C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE raw GRAPHICS.DAT";
    for (i = 0; surfaces && i < surfaceCount; ++i) {
        const DM1_V1_F0355SourceSurfacePc34 *surface = &surfaces[i];
        uint32_t hash;
        if (!surface->graphicsDatOwned ||
            surface->graphicIndex != DM1_V1_F0346_C040_RESURRECT_PANEL_PC34 ||
            surface->width != DM1_V1_F0346_PANEL_WIDTH_PC34 ||
            surface->height != DM1_V1_F0346_PANEL_HEIGHT_PC34 ||
            !surface->indexedPixels ||
            surface->indexedPixelCount <
                DM1_V1_F0346_PANEL_WIDTH_PC34 * DM1_V1_F0346_PANEL_HEIGHT_PC34) {
            continue;
        }
        hash = dm1_v1_f0355_inventory_material_fnv1a_pc34(
            surface->indexedPixels, surface->indexedPixelCount);
        if (!hash || hash != surface->pixelsFNV1a) continue;
        outReceipt->valid = 1;
        outReceipt->suppressSyntheticFallback = 1;
        outReceipt->graphicIndex = surface->graphicIndex;
        outReceipt->width = surface->width;
        outReceipt->height = surface->height;
        outReceipt->materialFingerprint = hash;
        return 1;
    }
    return 0;
}
