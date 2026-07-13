#ifndef FIRESTAFF_DM1_V1_INSCRIPTION_HOST_MATERIAL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_INSCRIPTION_HOST_MATERIAL_PC34_COMPAT_H

#include "dm1_v1_inscription_font_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_INSCRIPTION_HOST_MATERIAL_MAX_GLYPHS_PC34 128

/* ReDMCSB DUNGEON.C F0168 + DUNVIEW.C F0107 material hand-off. The
 * decoded bytes are original TextString codes, never host text. */
typedef struct DM1_V1_InscriptionHostMaterialReceiptPc34 {
    int valid;
    int textStringIndex;
    int fontGraphicIndex;
    int transparentColor;
    int glyphByteCount;
    int lineCount;
    unsigned char glyphBytes[DM1_V1_INSCRIPTION_HOST_MATERIAL_MAX_GLYPHS_PC34];
    DM1_V1_InscriptionFrontWallLineDrawPlanPc34
        lines[DM1_V1_INSCRIPTION_MAX_LINES];
} DM1_V1_InscriptionHostMaterialReceiptPc34;

int dm1_v1_inscription_host_material_from_world_pc34(
    const struct DungeonThings_Compat* things,
    int preferredTextIndex,
    unsigned short firstThing,
    DM1_V1_InscriptionHostMaterialReceiptPc34* outReceipt);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_INSCRIPTION_HOST_MATERIAL_PC34_COMPAT_H */
