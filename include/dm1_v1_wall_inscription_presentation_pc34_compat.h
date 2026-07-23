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
    int textDataWordOffset;
    int textDataWordCount;
    uint32_t textDataFNV1a;
    uint32_t glyphBytesFNV1a;
    unsigned int visibleLineMask;
} DM1_V1_WallInscriptionPresentationReceiptPc34;

/* One F0128 viewport tuple's readable-M648 decision. F0128 rebuilds the
 * viewport before F0107 selects its ornament/text branch, so each tuple first
 * clears the prior M648/C10 presentation and may then carry one current D1C
 * material receipt. A C127 mirror or non-front wall intentionally receives
 * clear-only; no host text or substitute glyph bitmap is permitted. */
typedef struct DM1_V1_ViewportInscriptionReceiptPc34 {
    int valid;
    int clearPreviousMaterial;
    int drawFrontMaterial;
    DM1_V1_InscriptionHostMaterialReceiptPc34 frontMaterial;
} DM1_V1_ViewportInscriptionReceiptPc34;

typedef enum DM1_V1_ViewportInscriptionProjectionPc34 {
    DM1_V1_INSCRIPTION_PROJECTION_CLEAR_ONLY_PC34 = 0,
    DM1_V1_INSCRIPTION_PROJECTION_D1C_FRONT_PC34 = 1,
    DM1_V1_INSCRIPTION_PROJECTION_SIDE_OR_DEPTH_PC34 = 2
} DM1_V1_ViewportInscriptionProjectionPc34;

int dm1_v1_wall_inscription_presentation_from_world_pc34(
    const struct DungeonThings_Compat* things,
    int preferredTextIndex,
    unsigned short firstThing,
    DM1_V1_WallInscriptionPresentationReceiptPc34* outReceipt);
int dm1_v1_wall_inscription_presentation_from_selected_wall_pc34(
    const struct DungeonThings_Compat* things,
    int selectedTextIndex,
    DM1_V1_WallInscriptionPresentationReceiptPc34* outReceipt);
int dm1_v1_viewport_inscription_receipt_from_world_pc34(
    const struct DungeonThings_Compat* things,
    int preferredTextIndex,
    unsigned short firstThing,
    DM1_V1_ViewportInscriptionProjectionPc34 projection,
    int championMirror,
    DM1_V1_ViewportInscriptionReceiptPc34* outReceipt);
int dm1_v1_viewport_inscription_receipt_from_selected_wall_pc34(
    const struct DungeonThings_Compat* things,
    int selectedTextIndex,
    DM1_V1_ViewportInscriptionProjectionPc34 projection,
    int championMirror,
    DM1_V1_ViewportInscriptionReceiptPc34* outReceipt);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_WALL_INSCRIPTION_PRESENTATION_PC34_COMPAT_H */
