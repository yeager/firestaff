#include "dm1_v1_champion_status_bar_redraw_receipt_pc34_compat.h"

#include "dm1_v1_champion_panel_hud_pc34_compat.h"
#include "dm1_v1_champion_status_layout_pc34_compat.h"

#include <string.h>

static int append_op(Dm1V1ChampionStatusBarRedrawReceiptPc34 *receipt,
                     Dm1V1ChampionStatusBarRedrawOperationPc34 operation,
                     Dm1V1ChampionPortraitStatusRedrawRoutePc34 route,
                     int champion, int stat, int zone, int x, int y,
                     int width, int height, int color, int current, int maximum)
{
    Dm1V1ChampionStatusBarRedrawOpPc34 *op;
    if (!receipt || height <= 0 || receipt->operationCount >=
        DM1_V1_CHAMPION_STATUS_BAR_REDRAW_MAX_OPS_PC34) return 0;
    op = &receipt->operations[receipt->operationCount++];
    op->operation = operation;
    op->route = route;
    op->championIndex = champion;
    op->statIndex = stat;
    op->zoneId = zone;
    op->x = x;
    op->y = y;
    op->width = width;
    op->height = height;
    op->color = color;
    op->current = current;
    op->maximum = maximum;
    if (operation == DM1_V1_CHAMPION_STATUS_BAR_CLEAR_PC34) ++receipt->clearCount;
    else ++receipt->repaintCount;
    return 1;
}

static int policy_is_consistent(const struct PartyState_Compat *party,
                                const Dm1V1ChampionPortraitStatusRedrawReceiptPc34 *policy)
{
    int slot;
    if (!party || !policy || !policy->valid || party->championCount < 0 ||
        party->championCount > CHAMPION_MAX_PARTY) return 0;
    for (slot = 0; slot < CHAMPION_MAX_PARTY; ++slot) {
        const Dm1V1ChampionPortraitStatusRedrawEntryPc34 *entry =
            &policy->entries[slot];
        if (entry->championIndex != slot) return 0;
        if (slot >= party->championCount &&
            entry->policy != DM1_V1_CHAMPION_PORTRAIT_STATUS_SKIP_PC34) return 0;
        if (entry->policy == DM1_V1_CHAMPION_PORTRAIT_STATUS_OWNED_PC34 &&
            entry->route == DM1_V1_CHAMPION_PORTRAIT_STATUS_NO_ROUTE_PC34) return 0;
    }
    return 1;
}

static int values_for_stat(const struct ChampionState_Compat *champion, int stat,
                           int *current, int *maximum)
{
    if (!champion || !current || !maximum) return 0;
    if (stat == 0) {
        *current = champion->hp.current;
        *maximum = champion->hp.maximum;
    } else if (stat == 1) {
        *current = champion->stamina.current;
        *maximum = champion->stamina.maximum;
    } else if (stat == 2) {
        *current = champion->mana.current;
        *maximum = champion->mana.maximum;
    } else {
        return 0;
    }
    return *maximum > 0;
}

const char *dm1_v1_champion_status_bar_redraw_receipt_source_evidence_pc34(void)
{
    return "ReDMCSB CHAMDRAW.C F0287:307-346 computes C195..C206 PC34 "
           "bar clear/fill rectangles from original current and maximum "
           "statistics; F0292:771-842 owns the lane and terminates live "
           "bar repaint for dead champions.";
}

int dm1_v1_champion_status_bar_redraw_receipt_pc34(
    const struct PartyState_Compat *party,
    const Dm1V1ChampionPortraitStatusRedrawReceiptPc34 *portraitPolicy,
    Dm1V1ChampionStatusBarRedrawReceiptPc34 *outReceipt)
{
    int slot;
    if (!outReceipt || !policy_is_consistent(party, portraitPolicy)) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));

    for (slot = 0; slot < party->championCount; ++slot) {
        const struct ChampionState_Compat *champion = &party->champions[slot];
        const Dm1V1ChampionPortraitStatusRedrawEntryPc34 *entry =
            &portraitPolicy->entries[slot];
        int stat;
        if (!champion->present) return 0;
        if (entry->policy == DM1_V1_CHAMPION_PORTRAIT_STATUS_SKIP_PC34) continue;

        for (stat = 0; stat < 3; ++stat) {
            DM1_V1_ChampionStatusRectPc34 rect;
            int current;
            int maximum;
            if (!values_for_stat(champion, stat, &current, &maximum) ||
                !dm1_v1_champion_status_bar_rect_pc34(slot, stat, &rect)) return 0;

            if (entry->policy == DM1_V1_CHAMPION_PORTRAIT_STATUS_CLEAR_PC34 ||
                !entry->alive) {
                if (!append_op(outReceipt, DM1_V1_CHAMPION_STATUS_BAR_CLEAR_PC34,
                               entry->route, slot, stat,
                               dm1_v1_champion_status_bar_value_zone_id_pc34(slot, stat),
                               rect.x, rect.y, rect.w, rect.h,
                               DM1_COLOR_DARKEST_GRAY, current, maximum)) return 0;
                continue;
            }

            {
                DM1_ChampionPanel_BarFillModel model;
                if (!DM1_ChampionPanel_BuildPc34BarFillModel(
                        slot, stat, current, maximum, &model)) return 0;
                if (model.emitsBlank && !append_op(outReceipt,
                    DM1_V1_CHAMPION_STATUS_BAR_CLEAR_PC34, entry->route,
                    slot, stat, model.zoneId, model.blankX, model.blankY,
                    model.blankWidth, model.blankHeight, model.blankColor,
                    current, maximum)) return 0;
                if (model.emitsFill && !append_op(outReceipt,
                    DM1_V1_CHAMPION_STATUS_BAR_REPAINT_PC34, entry->route,
                    slot, stat, model.zoneId, model.fillX, model.fillY,
                    model.fillWidth, model.fillHeight, model.fillColor,
                    current, maximum)) return 0;
            }
        }
    }
    outReceipt->dataGateAccepted = 1;
    outReceipt->valid = 1;
    return 1;
}
