#include "dm1_v1_champion_top_row_assets_pc34_compat.h"

#include "dm1_v1_champion_panel_hud_pc34_compat.h"

#include <string.h>

static Dm1V1ChampionTopRowSurfacePc34 surface_from_slot(const M11_AssetSlot *slot)
{
    Dm1V1ChampionTopRowSurfacePc34 surface;
    memset(&surface, 0, sizeof(surface));
    if (slot) {
        surface.graphicIndex = (int)slot->graphicIndex;
        surface.loaded = slot->loaded;
        surface.pixels = slot->pixels;
        surface.width = slot->width;
        surface.height = slot->height;
    }
    return surface;
}

static int surface_is_exact(const Dm1V1ChampionTopRowSurfacePc34 *surface,
                            int graphicIndex, int width, int height)
{
    return surface && DM1_ChampionPanel_AssetSurfaceAccepted(
        surface->graphicIndex, graphicIndex, surface->loaded,
        surface->pixels != NULL, surface->width, surface->height, width, height);
}

const char *dm1_v1_champion_top_row_assets_source_evidence_pc34(void)
{
    return "ReDMCSB CHAMDRAW.C F0291:632-651 uses C033/C034/C035; "
           "F0292:816-838 uses C008; F0622 uses C028 as four 19x14 "
           "frames. GRAPHICS.DAT loader slots remain valid until shutdown.";
}

int dm1_v1_champion_top_row_assets_from_slots_pc34(
    const M11_AssetSlot *c008,
    const M11_AssetSlot *c028,
    const M11_AssetSlot *c033,
    const M11_AssetSlot *c034,
    const M11_AssetSlot *c035,
    Dm1V1ChampionTopRowAssetsReceiptPc34 *outReceipt)
{
    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    outReceipt->assets.deadStatusBox = surface_from_slot(c008);
    outReceipt->assets.championIcons = surface_from_slot(c028);
    outReceipt->assets.slotNormal = surface_from_slot(c033);
    outReceipt->assets.slotWounded = surface_from_slot(c034);
    outReceipt->assets.slotActing = surface_from_slot(c035);

    outReceipt->c008Accepted = surface_is_exact(&outReceipt->assets.deadStatusBox,
                                                 DM1_GFX_DEAD_CHAMPION, 67, 29);
    outReceipt->c028Accepted = surface_is_exact(&outReceipt->assets.championIcons,
                                                 DM1_GFX_CHAMPION_ICONS, 76, 14);
    outReceipt->c033Accepted = surface_is_exact(&outReceipt->assets.slotNormal,
                                                 DM1_GFX_SLOT_NORMAL, 18, 18);
    outReceipt->c034Accepted = surface_is_exact(&outReceipt->assets.slotWounded,
                                                 DM1_GFX_SLOT_WOUNDED, 18, 18);
    outReceipt->c035Accepted = surface_is_exact(&outReceipt->assets.slotActing,
                                                 DM1_GFX_SLOT_ACTING, 18, 18);
    outReceipt->valid = outReceipt->c008Accepted && outReceipt->c028Accepted &&
                        outReceipt->c033Accepted && outReceipt->c034Accepted &&
                        outReceipt->c035Accepted;
    return outReceipt->valid;
}

int dm1_v1_champion_top_row_assets_from_m11_loader_pc34(
    M11_AssetLoader *loader,
    Dm1V1ChampionTopRowAssetsReceiptPc34 *outReceipt)
{
    if (!loader || !M11_AssetLoader_IsReady(loader)) {
        if (outReceipt) memset(outReceipt, 0, sizeof(*outReceipt));
        return 0;
    }
    return dm1_v1_champion_top_row_assets_from_slots_pc34(
        M11_AssetLoader_Load(loader, DM1_GFX_DEAD_CHAMPION),
        M11_AssetLoader_Load(loader, DM1_GFX_CHAMPION_ICONS),
        M11_AssetLoader_Load(loader, DM1_GFX_SLOT_NORMAL),
        M11_AssetLoader_Load(loader, DM1_GFX_SLOT_WOUNDED),
        M11_AssetLoader_Load(loader, DM1_GFX_SLOT_ACTING), outReceipt);
}
