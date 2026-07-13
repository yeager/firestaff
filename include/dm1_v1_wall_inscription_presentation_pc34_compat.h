#ifndef FIRESTAFF_DM1_V1_WALL_INSCRIPTION_PRESENTATION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_WALL_INSCRIPTION_PRESENTATION_PC34_COMPAT_H

#include "dm1_v1_inscription_host_material_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB DUNVIEW.C F0107:3864-3901 consumes decoded F0168 inscription
 * lines for an unreadable side/depth ornament.  This preserves only the
 * source-owned TextString identity and line count; it does not invent text
 * pixels or a host font. */
typedef struct DM1_V1_WallInscriptionPresentationReceiptPc34 {
    int valid;
    int textStringIndex;
    int lineCount;
} DM1_V1_WallInscriptionPresentationReceiptPc34;

int dm1_v1_wall_inscription_presentation_from_world_pc34(
    const struct DungeonThings_Compat* things,
    int preferredTextIndex,
    unsigned short firstThing,
    DM1_V1_WallInscriptionPresentationReceiptPc34* outReceipt);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_WALL_INSCRIPTION_PRESENTATION_PC34_COMPAT_H */
