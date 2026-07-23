#ifndef FIRESTAFF_DM1_V1_F0354_PORTRAIT_MATERIAL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0354_PORTRAIT_MATERIAL_PC34_COMPAT_H

#include <stdint.h>

#include "dm1_v1_champion_portrait_status_redraw_policy_pc34_compat.h"

typedef struct DM1_V1_F0354PortraitMaterialReceiptPc34 {
    int valid;
    int championIndex;
    int portraitByteCount;
    int statusGraphicIndex;
    uint32_t portraitFingerprint;
    uint32_t c028Fingerprint;
    uint32_t materialFingerprint;
    const char *sourceAnchor;
} DM1_V1_F0354PortraitMaterialReceiptPc34;

/* PANEL.C F0354 is only reached for the live inventory champion.  The
 * admission binds its raw PC34 portrait bytes to the accepted C028 source
 * surface before any status-box blit can be published. */
int dm1_v1_f0354_portrait_material_receipt_pc34(
    const struct PartyState_Compat *party,
    const Dm1V1ChampionLeaderOwnershipReceiptPc34 *ownership,
    const Dm1V1ChampionTopRowAssetsReceiptPc34 *assets,
    int championIndex,
    DM1_V1_F0354PortraitMaterialReceiptPc34 *outReceipt);

#endif
