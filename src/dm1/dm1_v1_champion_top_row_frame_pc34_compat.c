#include "dm1_v1_champion_top_row_frame_pc34_compat.h"

#include "dm1_v1_champion_panel_hud_pc34_compat.h"
#include "dm1_v1_champion_status_layout_pc34_compat.h"

#include <string.h>

static int surface_is_exact(const Dm1V1ChampionTopRowSurfacePc34 *surface,
                            int graphicIndex, int width, int height)
{
    return surface && DM1_ChampionPanel_AssetSurfaceAccepted(
        surface->graphicIndex, graphicIndex, surface->loaded,
        surface->pixels != NULL, surface->width, surface->height, width, height);
}

static const Dm1V1ChampionTopRowSurfacePc34 *surface_for_slot_graphic(
    const Dm1V1ChampionTopRowAssetsPc34 *assets, int graphicIndex)
{
    if (!assets) return NULL;
    if (graphicIndex == DM1_GFX_SLOT_NORMAL) return &assets->slotNormal;
    if (graphicIndex == DM1_GFX_SLOT_WOUNDED) return &assets->slotWounded;
    if (graphicIndex == DM1_GFX_SLOT_ACTING) return &assets->slotActing;
    return NULL;
}

const char *dm1_v1_champion_top_row_frame_source_evidence_pc34(void)
{
    return "ReDMCSB DEFS.H:3779-3803 C151..C218; CHAMDRAW.C "
           "F0287:307-346, F0291:632-651, F0292:771-905, "
           "F0293:1117-1139. C008/C028/C033/C034/C035 must be "
           "retained exact GRAPHICS.DAT surfaces; no substitute surface.";
}

int dm1_v1_champion_top_row_frame_from_party_pc34(
    const struct PartyState_Compat *party,
    int actingChampionOrdinal,
    int invisibilityCount,
    const Dm1V1ChampionTopRowAssetsPc34 *assets,
    Dm1V1ChampionTopRowFramePc34 *outFrame)
{
    int slot;

    if (!party || !assets || !outFrame || party->championCount < 0 ||
        party->championCount > DM1_V1_CHAMPION_TOP_ROW_SLOT_COUNT_PC34 ||
        party->activeChampionIndex < -1 ||
        party->activeChampionIndex >= DM1_V1_CHAMPION_TOP_ROW_SLOT_COUNT_PC34 ||
        actingChampionOrdinal < 0 ||
        actingChampionOrdinal > DM1_V1_CHAMPION_TOP_ROW_SLOT_COUNT_PC34 ||
        invisibilityCount < 0 ||
        !surface_is_exact(&assets->championIcons, DM1_GFX_CHAMPION_ICONS,
                          DM1_CHAMPION_ICON_WIDTH * 4,
                          DM1_CHAMPION_ICON_HEIGHT)) {
        return 0;
    }

    memset(outFrame, 0, sizeof(*outFrame));
    outFrame->partyChampionCount = party->championCount;
    outFrame->activeChampionIndex = party->activeChampionIndex;
    outFrame->actingChampionOrdinal = actingChampionOrdinal;
    outFrame->invisibilityCount = invisibilityCount;

    for (slot = 0; slot < DM1_V1_CHAMPION_TOP_ROW_SLOT_COUNT_PC34; ++slot) {
        const struct ChampionState_Compat *champ = &party->champions[slot];
        Dm1V1ChampionTopRowSlotPc34 *dst = &outFrame->slots[slot];
        DM1_V1_ChampionStatusRectPc34 rect;
        int stat;
        int hand;

        if (!dm1_v1_champion_status_box_rect_pc34(slot, &rect)) return 0;
        dst->statusBoxZoneId = dm1_v1_champion_status_box_zone_id_pc34(slot);
        dst->statusX = rect.x;
        dst->statusY = rect.y;
        dst->statusWidth = rect.w;
        dst->statusHeight = rect.h;
        dst->nameClearZoneId = dm1_v1_champion_status_name_clear_zone_id_pc34(slot);
        dst->nameTextZoneId = dm1_v1_champion_status_name_text_zone_id_pc34(slot);

        if (!champ->present) continue;
        dst->present = 1;
        dst->alive = champ->hp.current > 0;
        dst->nameColor = dm1_v1_champion_status_name_color_pc34(
            1, (int)champ->hp.current, slot == party->activeChampionIndex);

        if (!dst->alive &&
            !surface_is_exact(&assets->deadStatusBox, DM1_GFX_DEAD_CHAMPION,
                              DM1_STATUS_BOX_WIDTH, DM1_STATUS_BOX_HEIGHT)) {
            return 0;
        }

        if (!DM1_ChampionPanel_BuildIconBitmapModel(
                slot, champ->direction & 3, party->direction & 3,
                invisibilityCount, &(DM1_ChampionPanel_IconBitmapModel){0})) {
            return 0;
        }
        dst->iconSourceX = ((champ->direction + 4 - party->direction) & 3) *
                           DM1_CHAMPION_ICON_WIDTH;
        dst->iconFillColor = invisibilityCount > 0 ? DM1_COLOR_DARK_GRAY :
                             DM1_ChampionColor[slot];

        if (!dst->alive) continue;
        for (stat = 0; stat < DM1_V1_CHAMPION_TOP_ROW_STAT_COUNT_PC34; ++stat) {
            Dm1V1ChampionTopRowStatPc34 *bar = &dst->stats[stat];
            DM1_ChampionPanel_BarFillModel model;
            int current = stat == 0 ? champ->hp.current :
                          stat == 1 ? champ->stamina.current : champ->mana.current;
            int maximum = stat == 0 ? champ->hp.maximum :
                          stat == 1 ? champ->stamina.maximum : champ->mana.maximum;
            if (!DM1_ChampionPanel_BuildPc34BarFillModel(
                    slot, stat, current, maximum, &model)) return 0;
            bar->zoneId = model.zoneId;
            bar->x = model.x;
            bar->y = model.y;
            bar->width = model.width;
            bar->height = model.height;
            bar->current = current;
            bar->maximum = maximum;
            bar->blankHeight = model.blankHeight;
            bar->fillHeight = model.fillHeight;
            bar->blankColor = model.blankColor;
            bar->fillColor = model.fillColor;
        }
        for (hand = 0; hand < DM1_V1_CHAMPION_TOP_ROW_HAND_COUNT_PC34; ++hand) {
            Dm1V1ChampionTopRowHandPc34 *handPlan = &dst->hands[hand];
            const Dm1V1ChampionTopRowSurfacePc34 *surface;
            int graphicIndex;
            if (!dm1_v1_champion_status_hand_slot_box_rect_pc34(slot, hand, &rect)) {
                return 0;
            }
            graphicIndex = dm1_v1_champion_status_hand_slot_graphic_pc34(
                hand, champ->wounds,
                hand == DM1_SLOT_ACTION_HAND && actingChampionOrdinal == slot + 1);
            surface = surface_for_slot_graphic(assets, graphicIndex);
            if (!surface || !surface_is_exact(surface, graphicIndex,
                                              DM1_SLOT_BOX_SIZE, DM1_SLOT_BOX_SIZE)) {
                return 0;
            }
            handPlan->zoneId = dm1_v1_champion_status_hand_zone_id_pc34(slot, hand);
            handPlan->x = rect.x;
            handPlan->y = rect.y;
            handPlan->width = rect.w;
            handPlan->height = rect.h;
            handPlan->graphicIndex = graphicIndex;
        }
    }

    outFrame->valid = 1;
    return 1;
}
