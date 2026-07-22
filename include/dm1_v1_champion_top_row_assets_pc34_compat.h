#ifndef FIRESTAFF_DM1_V1_CHAMPION_TOP_ROW_ASSETS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_TOP_ROW_ASSETS_PC34_COMPAT_H

#include "asset_loader_m11.h"
#include "dm1_v1_champion_top_row_frame_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* DM1-owned bridge from the retained GRAPHICS.DAT loader slots to the
 * F0287/F0291/F0292 top-row frame contract. It never creates pixels or
 * substitutes a surface when the source bank is incomplete. */
typedef struct Dm1V1ChampionTopRowAssetsReceiptPc34 {
    int valid;
    int c008Accepted;
    int c028Accepted;
    int c033Accepted;
    int c034Accepted;
    int c035Accepted;
    Dm1V1ChampionTopRowAssetsPc34 assets;
} Dm1V1ChampionTopRowAssetsReceiptPc34;

int dm1_v1_champion_top_row_assets_from_m11_loader_pc34(
    M11_AssetLoader *loader,
    Dm1V1ChampionTopRowAssetsReceiptPc34 *outReceipt);

/* Slot-level form retained for direct testing and for host adapters that
 * already own loader-slot lifetime. It applies the same exact admission
 * checks as the loader-facing entry point. */
int dm1_v1_champion_top_row_assets_from_slots_pc34(
    const M11_AssetSlot *c008,
    const M11_AssetSlot *c028,
    const M11_AssetSlot *c033,
    const M11_AssetSlot *c034,
    const M11_AssetSlot *c035,
    Dm1V1ChampionTopRowAssetsReceiptPc34 *outReceipt);

const char *dm1_v1_champion_top_row_assets_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
