#include "dm1_v1_champion_top_row_presentation_pc34_compat.h"

#include "dm1_v1_champion_panel_hud_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static unsigned char pixels[76 * 29];

static int check(const char *label, int value)
{
    if (!value) fprintf(stderr, "FAIL %s\n", label);
    return value;
}

static void surface(Dm1V1ChampionTopRowSurfacePc34 *out, int graphic, int w, int h)
{
    out->graphicIndex = graphic; out->loaded = 1; out->pixels = pixels;
    out->width = w; out->height = h;
}

int main(void)
{
    struct PartyState_Compat party;
    Dm1V1ChampionTopRowAssetsReceiptPc34 assets;
    Dm1V1ChampionTopRowFramePc34 frame;
    Dm1V1ChampionTopRowPresentationReceiptPc34 receipt;
    int foundDeadStatus = 0;
    int ok = 1;

    memset(&party, 0, sizeof(party));
    memset(&assets, 0, sizeof(assets));
    party.championCount = 2; party.activeChampionIndex = 0; party.direction = 0;
    for (int i = 0; i < 2; ++i) {
        party.champions[i].present = 1; party.champions[i].direction = (unsigned char)i;
        party.champions[i].hp.maximum = 100; party.champions[i].stamina.maximum = 100;
        party.champions[i].mana.maximum = 100; party.champions[i].hp.current = 50;
        party.champions[i].stamina.current = 60; party.champions[i].mana.current = 70;
    }
    party.champions[1].hp.current = 0;
    surface(&assets.assets.deadStatusBox, DM1_GFX_DEAD_CHAMPION, 67, 29);
    surface(&assets.assets.championIcons, DM1_GFX_CHAMPION_ICONS, 76, 14);
    surface(&assets.assets.slotNormal, DM1_GFX_SLOT_NORMAL, 18, 18);
    surface(&assets.assets.slotWounded, DM1_GFX_SLOT_WOUNDED, 18, 18);
    surface(&assets.assets.slotActing, DM1_GFX_SLOT_ACTING, 18, 18);
    assets.valid = assets.c008Accepted = assets.c028Accepted = assets.c033Accepted =
        assets.c034Accepted = assets.c035Accepted = 1;

    ok &= check("live frame", dm1_v1_champion_top_row_frame_from_party_pc34(
        &party, 1, 0, &assets.assets, &frame));
    ok &= check("presentation receipt", dm1_v1_champion_top_row_presentation_from_frame_pc34(
        &frame, &assets, &receipt));
    ok &= check("receipt source-bound", receipt.valid && receipt.operationCount == 16);
    ok &= check("first op status fill", receipt.operations[0].kind ==
                DM1_V1_CHAMPION_TOP_ROW_OP_CLEAR_STATUS_PC34 &&
                receipt.operations[0].color == DM1_COLOR_DARKEST_GRAY);
    ok &= check("icon uses C028 bytes", receipt.operations[3].kind ==
                DM1_V1_CHAMPION_TOP_ROW_OP_COMPOSE_ICON_PC34 &&
                receipt.operations[3].graphicIndex == DM1_GFX_CHAMPION_ICONS &&
                receipt.operations[3].sourcePixels == pixels);
    for (int op = 0; op < receipt.operationCount; ++op) {
        if (receipt.operations[op].kind ==
                DM1_V1_CHAMPION_TOP_ROW_OP_BLIT_DEAD_STATUS_PC34 &&
            receipt.operations[op].championSlot == 1 &&
            receipt.operations[op].graphicIndex == DM1_GFX_DEAD_CHAMPION &&
            receipt.operations[op].sourcePixels == pixels) {
            foundDeadStatus = 1;
        }
    }
    ok &= check("dead slot uses C008", foundDeadStatus);
    assets.valid = 0;
    ok &= check("invalid asset receipt fails closed", !dm1_v1_champion_top_row_presentation_from_frame_pc34(
        &frame, &assets, &receipt));
    return ok ? 0 : 1;
}
