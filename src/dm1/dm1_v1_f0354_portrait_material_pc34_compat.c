#include "dm1_v1_f0354_portrait_material_pc34_compat.h"

#include "dm1_v1_champion_panel_hud_pc34_compat.h"

#include <string.h>

static uint32_t f0354_hash(const unsigned char *bytes, int count)
{
    uint32_t hash = 2166136261u;
    int i;
    if (!bytes || count <= 0) return 0u;
    for (i = 0; i < count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

int dm1_v1_f0354_portrait_material_receipt_pc34(
    const struct PartyState_Compat *party,
    const Dm1V1ChampionLeaderOwnershipReceiptPc34 *ownership,
    const Dm1V1ChampionTopRowAssetsReceiptPc34 *assets,
    int championIndex,
    DM1_V1_F0354PortraitMaterialReceiptPc34 *outReceipt)
{
    Dm1V1ChampionPortraitStatusRedrawReceiptPc34 policy;
    const Dm1V1ChampionPortraitStatusRedrawEntryPc34 *entry;
    const struct ChampionState_Compat *champion;
    uint32_t portraitHash;
    uint32_t c028Hash;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    outReceipt->championIndex = championIndex;
    outReceipt->sourceAnchor =
        "ReDMCSB PANEL.C F0354:2195-2242; CHAMDRAW.C F0292:810-812; "
        "M516 champion portrait bytes plus C028 GRAPHICS.DAT";
    if (!party || !ownership || !assets || championIndex < 0 ||
        championIndex >= party->championCount ||
        !dm1_v1_champion_portrait_status_redraw_policy_pc34(
            party, ownership, assets, &policy)) return 0;

    entry = &policy.entries[championIndex];
    champion = &party->champions[championIndex];
    if (entry->policy != DM1_V1_CHAMPION_PORTRAIT_STATUS_OWNED_PC34 ||
        entry->route != DM1_V1_CHAMPION_PORTRAIT_STATUS_INVENTORY_F0292_PC34 ||
        !entry->alive || !champion->portraitBitmapValid ||
        entry->portraitPixels != champion->portraitBitmap ||
        !assets->c028Accepted || !assets->assets.championIcons.pixels ||
        assets->assets.championIcons.graphicIndex != DM1_GFX_CHAMPION_ICONS ||
        assets->assets.championIcons.width != 76 ||
        assets->assets.championIcons.height != 14) return 0;

    portraitHash = f0354_hash(champion->portraitBitmap,
                              CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT);
    c028Hash = f0354_hash(assets->assets.championIcons.pixels, 76 * 14);
    if (!portraitHash || !c028Hash) return 0;
    outReceipt->valid = 1;
    outReceipt->portraitByteCount = CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT;
    outReceipt->statusGraphicIndex = DM1_GFX_CHAMPION_ICONS;
    outReceipt->portraitFingerprint = portraitHash;
    outReceipt->c028Fingerprint = c028Hash;
    outReceipt->materialFingerprint = portraitHash ^ (c028Hash * 16777619u);
    return outReceipt->materialFingerprint != 0u;
}
