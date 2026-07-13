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
