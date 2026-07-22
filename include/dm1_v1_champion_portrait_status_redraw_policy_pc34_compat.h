#ifndef FIRESTAFF_DM1_V1_CHAMPION_PORTRAIT_STATUS_REDRAW_POLICY_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PORTRAIT_STATUS_REDRAW_POLICY_PC34_COMPAT_H

#include "dm1_v1_champion_leader_ownership_handoff_pc34_compat.h"
#include "dm1_v1_champion_top_row_assets_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum Dm1V1ChampionPortraitStatusRedrawPolicyPc34 {
    DM1_V1_CHAMPION_PORTRAIT_STATUS_SKIP_PC34 = 0,
    DM1_V1_CHAMPION_PORTRAIT_STATUS_OWNED_PC34,
    DM1_V1_CHAMPION_PORTRAIT_STATUS_CLEAR_PC34
} Dm1V1ChampionPortraitStatusRedrawPolicyPc34;

typedef enum Dm1V1ChampionPortraitStatusRedrawRoutePc34 {
    DM1_V1_CHAMPION_PORTRAIT_STATUS_NO_ROUTE_PC34 = 0,
    DM1_V1_CHAMPION_PORTRAIT_STATUS_PRIMARY_F0296_PC34,
    DM1_V1_CHAMPION_PORTRAIT_STATUS_INVENTORY_F0292_PC34
} Dm1V1ChampionPortraitStatusRedrawRoutePc34;

typedef struct Dm1V1ChampionPortraitStatusRedrawEntryPc34 {
    Dm1V1ChampionPortraitStatusRedrawPolicyPc34 policy;
    Dm1V1ChampionPortraitStatusRedrawRoutePc34 route;
    int championIndex;
    int alive;
    int statusGraphicIndex;
    const uint8_t *portraitPixels;
    const uint8_t *statusPixels;
} Dm1V1ChampionPortraitStatusRedrawEntryPc34;

typedef struct Dm1V1ChampionPortraitStatusRedrawReceiptPc34 {
    int valid;
    int ownedCount;
    int skipCount;
    int clearCount;
    Dm1V1ChampionPortraitStatusRedrawEntryPc34 entries[CHAMPION_MAX_PARTY];
} Dm1V1ChampionPortraitStatusRedrawReceiptPc34;

/* Derives portrait/status ownership after leader and inventory selection.
 * C028 plus a validated portrait source own a live status lane; C008 owns a
 * dead lane. Missing original source material yields an explicit clear, never
 * a replacement portrait, generated icon, or substitute status surface. */
int dm1_v1_champion_portrait_status_redraw_policy_pc34(
    const struct PartyState_Compat *party,
    const Dm1V1ChampionLeaderOwnershipReceiptPc34 *ownership,
    const Dm1V1ChampionTopRowAssetsReceiptPc34 *assets,
    Dm1V1ChampionPortraitStatusRedrawReceiptPc34 *outReceipt);

const char *dm1_v1_champion_portrait_status_redraw_policy_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
