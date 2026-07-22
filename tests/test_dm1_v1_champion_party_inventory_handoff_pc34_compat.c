#include "dm1_v1_champion_party_inventory_handoff_pc34_compat.h"

#include "dm1_v1_champion_panel_hud_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static unsigned char pixels[96 * 29];

static int check(const char *label, int value)
{
    if (!value) fprintf(stderr, "FAIL %s\n", label);
    return value;
}

static void top_surface(Dm1V1ChampionTopRowSurfacePc34 *out, int id, int w, int h)
{
    out->graphicIndex = id; out->loaded = 1; out->pixels = pixels; out->width = w; out->height = h;
}

static void redraw_surface(Dm1V1ChampionRedrawSurfacePc34 *out, int id, int w, int h)
{
    out->graphicIndex = id; out->loaded = 1; out->pixels = pixels; out->width = w; out->height = h;
}

int main(void)
{
    Dm1V1ChampionTopRowPresentationReceiptPc34 top;
    Dm1V1ChampionRedrawPriorityReceiptPc34 redraw;
    Dm1V1ChampionRedrawMaterialsPc34 materials;
    Dm1V1ChampionPartyInventorySwitchPc34 transition = { 2, 0, 1 };
    Dm1V1ChampionPartyInventoryHandoffReceiptPc34 receipt;
    int ok = 1;

    memset(&top, 0, sizeof(top)); memset(&redraw, 0, sizeof(redraw));
    memset(&materials, 0, sizeof(materials));
    top.valid = top.assetReceipt.valid = 1; top.assetReceipt.c008Accepted = 1;
    top.assetReceipt.c028Accepted = top.assetReceipt.c033Accepted = 1;
    top.assetReceipt.c034Accepted = top.assetReceipt.c035Accepted = 1;
    top_surface(&top.assetReceipt.assets.deadStatusBox, 8, 67, 29);
    top_surface(&top.assetReceipt.assets.championIcons, 28, 76, 14);
    top_surface(&top.assetReceipt.assets.slotNormal, 33, 18, 18);
    top_surface(&top.assetReceipt.assets.slotWounded, 34, 18, 18);
    top_surface(&top.assetReceipt.assets.slotActing, 35, 18, 18);
    top.operationCount = 2;
    top.operations[0] = (Dm1V1ChampionTopRowPresentationOpPc34){
        .kind = DM1_V1_CHAMPION_TOP_ROW_OP_BLIT_DEAD_STATUS_PC34,
        .championSlot = 0, .zoneId = 151, .graphicIndex = 8, .sourcePixels = pixels };
    top.operations[1] = (Dm1V1ChampionTopRowPresentationOpPc34){
        .kind = DM1_V1_CHAMPION_TOP_ROW_OP_COMPOSE_ICON_PC34,
        .championSlot = 1, .zoneId = 114, .graphicIndex = 28, .sourcePixels = pixels };
    redraw.valid = 1; redraw.operationCount = 2;
    redraw.operations[0] = (Dm1V1ChampionRedrawPriorityOpPc34){
        .kind = DM1_V1_CHAMPION_REDRAW_POISON_PC34, .championSlot = 0,
        .graphicIndex = 32, .sourcePixels = pixels };
    redraw.operations[1] = (Dm1V1ChampionRedrawPriorityOpPc34){
        .kind = DM1_V1_CHAMPION_REDRAW_DAMAGE_PC34, .championSlot = 1,
        .graphicIndex = 16, .sourcePixels = pixels };
    redraw_surface(&materials.poisonLabel, 32, 96, 15);
    redraw_surface(&materials.damageSmall, 15, 45, 7);
    redraw_surface(&materials.damageBig, 16, 32, 29);

    ok &= check("full handoff", dm1_v1_champion_party_inventory_handoff_pc34(
        &top, &redraw, &transition, &materials, &receipt));
    ok &= check("all originals retained", receipt.valid && receipt.operationCount == 4 &&
                receipt.topRowAssets.assets.deadStatusBox.pixels == pixels &&
                receipt.topRowAssets.assets.championIcons.pixels == pixels &&
                receipt.redrawMaterials.poisonLabel.pixels == pixels &&
                receipt.redrawMaterials.damageSmall.pixels == pixels &&
                receipt.redrawMaterials.damageBig.pixels == pixels);
    ok &= check("top row precedes redraw", receipt.operations[0].kind ==
                DM1_V1_CHAMPION_PARTY_INVENTORY_HANDOFF_TOP_ROW_PC34 &&
                receipt.operations[2].kind == DM1_V1_CHAMPION_PARTY_INVENTORY_HANDOFF_REDRAW_PC34);
    materials.damageSmall.loaded = 0;
    ok &= check("missing retained C015 fails closed", !dm1_v1_champion_party_inventory_handoff_pc34(
        &top, &redraw, &transition, &materials, &receipt));
    return ok ? 0 : 1;
}
