#include "dm1_v1_champion_portrait_status_redraw_policy_pc34_compat.h"

#include "dm1_v1_champion_panel_hud_pc34_compat.h"

#include <string.h>

static int ownership_is_consistent(
    const struct PartyState_Compat *party,
    const Dm1V1ChampionLeaderOwnershipReceiptPc34 *ownership)
{
    int slot;
    if (!party || !ownership || !ownership->valid ||
        party->championCount < 0 || party->championCount > CHAMPION_MAX_PARTY ||
        ownership->partyChampionCount != party->championCount) return 0;
    for (slot = 0; slot < CHAMPION_MAX_PARTY; ++slot) {
        if (!!party->champions[slot].present != (slot < party->championCount)) return 0;
    }
    return ownership->inventoryChampionIndex >= -1 &&
           ownership->inventoryChampionIndex < party->championCount;
}

static int live_material_is_original(const struct ChampionState_Compat *champion,
                                     const Dm1V1ChampionTopRowAssetsReceiptPc34 *assets)
{
    return champion && champion->portraitBitmapValid && assets &&
           assets->c028Accepted && assets->assets.championIcons.pixels &&
           assets->assets.championIcons.graphicIndex == DM1_GFX_CHAMPION_ICONS &&
           assets->assets.championIcons.width == 76 &&
           assets->assets.championIcons.height == 14;
}

static int dead_material_is_original(const Dm1V1ChampionTopRowAssetsReceiptPc34 *assets)
{
    return assets && assets->c008Accepted && assets->assets.deadStatusBox.pixels &&
           assets->assets.deadStatusBox.graphicIndex == DM1_GFX_DEAD_CHAMPION &&
           assets->assets.deadStatusBox.width == 67 &&
           assets->assets.deadStatusBox.height == 29;
}

const char *dm1_v1_champion_portrait_status_redraw_policy_source_evidence_pc34(void)
{
    return "ReDMCSB CHAMDRAW.C F0292:771-815 selects a champion status "
           "lane, F0292:816-842 uses C008 for dead champions, and F0622 "
           "uses the four C028 champion-icon frames. CHAMPION.C Champion "
           "portrait data remains the original source for a live champion.";
}

int dm1_v1_champion_portrait_status_redraw_policy_pc34(
    const struct PartyState_Compat *party,
    const Dm1V1ChampionLeaderOwnershipReceiptPc34 *ownership,
    const Dm1V1ChampionTopRowAssetsReceiptPc34 *assets,
    Dm1V1ChampionPortraitStatusRedrawReceiptPc34 *outReceipt)
{
    int slot;
    if (!outReceipt || !ownership_is_consistent(party, ownership)) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));

    for (slot = 0; slot < CHAMPION_MAX_PARTY; ++slot) {
        const struct ChampionState_Compat *champion = &party->champions[slot];
        Dm1V1ChampionPortraitStatusRedrawEntryPc34 *entry = &outReceipt->entries[slot];
        int isInventoryOwner = slot == ownership->inventoryChampionIndex;
        int ownsPrimary = !!(ownership->redrawOwnerMask & (1U << slot));
        int ownsLane = !ownership->candidateBlocksF0296 &&
            !!(ownership->topRowChampionMask & (1U << slot)) &&
            (ownsPrimary || isInventoryOwner);

        entry->championIndex = slot;
        if (!ownsLane) {
            entry->policy = DM1_V1_CHAMPION_PORTRAIT_STATUS_SKIP_PC34;
            entry->route = DM1_V1_CHAMPION_PORTRAIT_STATUS_NO_ROUTE_PC34;
            ++outReceipt->skipCount;
            continue;
        }

        entry->route = isInventoryOwner
            ? DM1_V1_CHAMPION_PORTRAIT_STATUS_INVENTORY_F0292_PC34
            : DM1_V1_CHAMPION_PORTRAIT_STATUS_PRIMARY_F0296_PC34;
        entry->alive = champion->hp.current > 0;
        if (entry->alive) {
            entry->statusGraphicIndex = DM1_GFX_CHAMPION_ICONS;
            if (live_material_is_original(champion, assets)) {
                entry->policy = DM1_V1_CHAMPION_PORTRAIT_STATUS_OWNED_PC34;
                entry->portraitPixels = champion->portraitBitmap;
                entry->statusPixels = assets->assets.championIcons.pixels;
                ++outReceipt->ownedCount;
            } else {
                entry->policy = DM1_V1_CHAMPION_PORTRAIT_STATUS_CLEAR_PC34;
                ++outReceipt->clearCount;
            }
        } else {
            entry->statusGraphicIndex = DM1_GFX_DEAD_CHAMPION;
            if (dead_material_is_original(assets)) {
                entry->policy = DM1_V1_CHAMPION_PORTRAIT_STATUS_OWNED_PC34;
                entry->statusPixels = assets->assets.deadStatusBox.pixels;
                ++outReceipt->ownedCount;
            } else {
                entry->policy = DM1_V1_CHAMPION_PORTRAIT_STATUS_CLEAR_PC34;
                ++outReceipt->clearCount;
            }
        }
    }
    outReceipt->valid = 1;
    return 1;
}
