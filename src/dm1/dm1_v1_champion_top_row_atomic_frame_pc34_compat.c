#include "dm1_v1_champion_top_row_atomic_frame_pc34_compat.h"

#include "dm1_v1_champion_panel_hud_pc34_compat.h"
#include "dm1_v1_champion_status_layout_pc34_compat.h"

#include <string.h>

static int append_op(Dm1V1ChampionTopRowAtomicFrameReceiptPc34 *receipt,
                     Dm1V1ChampionTopRowAtomicFrameOperationPc34 operation)
{
    if (!receipt || receipt->operationCount >=
        DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34) return 0;
    memset(&receipt->operations[receipt->operationCount], 0,
           sizeof(receipt->operations[receipt->operationCount]));
    receipt->operations[receipt->operationCount++].operation = operation;
    return 1;
}

static int party_and_policy_are_consistent(
    const struct PartyState_Compat *party,
    const Dm1V1ChampionPortraitStatusRedrawReceiptPc34 *policy)
{
    int slot;
    if (!party || !policy || !policy->valid || party->championCount < 0 ||
        party->championCount > CHAMPION_MAX_PARTY) return 0;
    for (slot = 0; slot < CHAMPION_MAX_PARTY; ++slot) {
        if (policy->entries[slot].championIndex != slot ||
            (!!party->champions[slot].present != (slot < party->championCount))) {
            return 0;
        }
    }
    return 1;
}

static int assets_are_exact(const Dm1V1ChampionTopRowAssetsReceiptPc34 *assets)
{
    return assets && assets->c008Accepted && assets->c028Accepted &&
           assets->assets.deadStatusBox.pixels &&
           assets->assets.deadStatusBox.graphicIndex == DM1_GFX_DEAD_CHAMPION &&
           assets->assets.deadStatusBox.width == 67 &&
           assets->assets.deadStatusBox.height == 29 &&
           assets->assets.championIcons.pixels &&
           assets->assets.championIcons.graphicIndex == DM1_GFX_CHAMPION_ICONS &&
           assets->assets.championIcons.width == 76 &&
           assets->assets.championIcons.height == 14;
}

static int all_selected_lanes_are_original(
    const struct PartyState_Compat *party,
    const Dm1V1ChampionPortraitStatusRedrawReceiptPc34 *policy,
    const Dm1V1ChampionTopRowAssetsReceiptPc34 *assets)
{
    int slot;
    for (slot = 0; slot < party->championCount; ++slot) {
        const Dm1V1ChampionPortraitStatusRedrawEntryPc34 *entry = &policy->entries[slot];
        if (entry->policy == DM1_V1_CHAMPION_PORTRAIT_STATUS_SKIP_PC34) continue;
        if (entry->policy != DM1_V1_CHAMPION_PORTRAIT_STATUS_OWNED_PC34) return 0;
        if (entry->alive) {
            if (!entry->portraitPixels || entry->statusPixels !=
                assets->assets.championIcons.pixels) return 0;
        } else if (entry->statusPixels != assets->assets.deadStatusBox.pixels) {
            return 0;
        }
    }
    return 1;
}

static int append_clear_region(Dm1V1ChampionTopRowAtomicFrameReceiptPc34 *receipt,
                               int champion, int zoneId, int x, int y, int width, int height)
{
    Dm1V1ChampionTopRowAtomicFrameOpPc34 *op;
    if (!append_op(receipt, DM1_V1_CHAMPION_TOP_ROW_ATOMIC_CLEAR_STATUS_PC34)) return 0;
    op = &receipt->operations[receipt->operationCount - 1];
    op->championIndex = champion;
    op->zoneId = zoneId;
    op->x = x;
    op->y = y;
    op->width = width;
    op->height = height;
    return 1;
}

static int append_full_clear(Dm1V1ChampionTopRowAtomicFrameReceiptPc34 *receipt,
                             const struct PartyState_Compat *party,
                             const Dm1V1ChampionPortraitStatusRedrawReceiptPc34 *policy)
{
    int slot;
    for (slot = 0; slot < party->championCount; ++slot) {
        DM1_V1_ChampionStatusRectPc34 rect;
        int stat;
        if (policy->entries[slot].policy == DM1_V1_CHAMPION_PORTRAIT_STATUS_SKIP_PC34)
            continue;
        if (!dm1_v1_champion_status_box_rect_pc34(slot, &rect) ||
            !append_clear_region(receipt, slot,
                dm1_v1_champion_status_box_zone_id_pc34(slot),
                rect.x, rect.y, rect.w, rect.h)) return 0;
        for (stat = 0; stat < 3; ++stat) {
            if (!dm1_v1_champion_status_bar_rect_pc34(slot, stat, &rect) ||
                !append_clear_region(receipt, slot,
                    dm1_v1_champion_status_bar_value_zone_id_pc34(slot, stat),
                    rect.x, rect.y, rect.w, rect.h)) return 0;
        }
    }
    return 1;
}

const char *dm1_v1_champion_top_row_atomic_frame_source_evidence_pc34(void)
{
    return "ReDMCSB CHAMDRAW.C F0292:771-905 binds C008 dead status, C028 "
           "champion icon, and F0287 status bars in one top-row redraw pass. "
           "F0680/F0692 present retained indexed material together; missing "
           "material must clear the affected original regions, not mix sources.";
}

int dm1_v1_champion_top_row_atomic_frame_pc34(
    const struct PartyState_Compat *party,
    const Dm1V1ChampionPortraitStatusRedrawReceiptPc34 *portraitPolicy,
    const Dm1V1ChampionTopRowAssetsReceiptPc34 *assets,
    const Dm1V1ChampionStatusBarFramePresentationReceiptPc34 *statusBars,
    Dm1V1ChampionTopRowAtomicFrameReceiptPc34 *outReceipt)
{
    Dm1V1ChampionTopRowAtomicFrameReceiptPc34 pending;
    int publishOriginal;
    int slot;
    if (!outReceipt || !party_and_policy_are_consistent(party, portraitPolicy)) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    memset(&pending, 0, sizeof(pending));

    publishOriginal = assets_are_exact(assets) && statusBars && statusBars->valid &&
        statusBars->atomicPublish && all_selected_lanes_are_original(
            party, portraitPolicy, assets);
    if (!publishOriginal) {
        if (!append_full_clear(&pending, party, portraitPolicy)) return 0;
        pending.clearOnly = 1;
        pending.valid = 1;
        *outReceipt = pending;
        return 1;
    }

    for (slot = 0; slot < party->championCount; ++slot) {
        const Dm1V1ChampionPortraitStatusRedrawEntryPc34 *entry =
            &portraitPolicy->entries[slot];
        Dm1V1ChampionTopRowAtomicFrameOpPc34 *op;
        if (entry->policy == DM1_V1_CHAMPION_PORTRAIT_STATUS_SKIP_PC34) continue;
        if (!append_op(&pending, entry->alive
            ? DM1_V1_CHAMPION_TOP_ROW_ATOMIC_COMPOSE_C028_PC34
            : DM1_V1_CHAMPION_TOP_ROW_ATOMIC_BLIT_C008_PC34)) return 0;
        op = &pending.operations[pending.operationCount - 1];
        op->championIndex = slot;
        op->graphicIndex = entry->alive ? DM1_GFX_CHAMPION_ICONS : DM1_GFX_DEAD_CHAMPION;
        op->sourcePixels = entry->statusPixels;
        op->portraitPixels = entry->portraitPixels;
    }
    for (slot = 0; slot < statusBars->operationCount; ++slot) {
        Dm1V1ChampionTopRowAtomicFrameOpPc34 *op;
        if (!append_op(&pending, DM1_V1_CHAMPION_TOP_ROW_ATOMIC_STATUS_BAR_PC34)) return 0;
        op = &pending.operations[pending.operationCount - 1];
        op->championIndex = statusBars->operations[slot].championIndex;
        op->zoneId = statusBars->operations[slot].zoneId;
        op->statusBar = statusBars->operations[slot];
    }
    pending.originalMaterialsPublished = 1;
    pending.valid = 1;
    *outReceipt = pending;
    return 1;
}
