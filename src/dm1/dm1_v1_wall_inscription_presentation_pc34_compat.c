#include "dm1_v1_wall_inscription_presentation_pc34_compat.h"

#include <string.h>

int dm1_v1_wall_inscription_presentation_from_world_pc34(
    const struct DungeonThings_Compat* things,
    int preferredTextIndex,
    unsigned short firstThing,
    DM1_V1_WallInscriptionPresentationReceiptPc34* outReceipt)
{
    DM1_V1_InscriptionHostMaterialReceiptPc34 material;
    DM1_V1_WallInscriptionPresentationReceiptPc34 receipt;
    if (!outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    /* DUNGEON.C F0168 supplies the real TextString decode.  F0107 counts
     * its 0x80/0x81-separated lines before selecting an unreadable box. */
    if (!dm1_v1_inscription_host_material_from_world_pc34(
            things, preferredTextIndex, firstThing, &material) ||
        material.lineCount <= 0) {
        return 0;
    }
    receipt.valid = 1;
    receipt.textStringIndex = material.textStringIndex;
    receipt.lineCount = material.lineCount;
    *outReceipt = receipt;
    return 1;
}

int dm1_v1_viewport_inscription_receipt_from_world_pc34(
    const struct DungeonThings_Compat* things,
    int preferredTextIndex,
    unsigned short firstThing,
    DM1_V1_ViewportInscriptionProjectionPc34 projection,
    int championMirror,
    DM1_V1_ViewportInscriptionReceiptPc34* outReceipt)
{
    DM1_V1_ViewportInscriptionReceiptPc34 receipt;

    if (!outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.clearPreviousMaterial = 1;
    /* ReDMCSB DUNVIEW.C F0128:8318-8616 clears/rebuilds the current viewport
     * tuple before F0107:3590-3706 can select M648. F0107:3913-3928 routes a
     * C127 mirror through C346/C026, never the readable inscription branch.
     * Side/depth projections retain the original unreadable ornament route;
     * only the D1C front-wall tuple may publish readable M648 glyph cells. */
    if (projection != DM1_V1_INSCRIPTION_PROJECTION_D1C_FRONT_PC34 ||
        championMirror || !things) {
        *outReceipt = receipt;
        return 1;
    }
    if (dm1_v1_inscription_host_material_from_world_pc34(
            things, preferredTextIndex, firstThing, &receipt.frontMaterial)) {
        receipt.drawFrontMaterial = 1;
    }
    *outReceipt = receipt;
    return 1;
}
