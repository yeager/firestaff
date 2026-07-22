#include "dm1_v1_champion_redraw_priority_pc34_compat.h"

#include "dm1_v1_champion_panel_hud_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static unsigned char pixels[96 * 29];

static int check(const char *label, int value)
{
    if (!value) fprintf(stderr, "FAIL %s\n", label);
    return value;
}

static void source(Dm1V1ChampionRedrawSurfacePc34 *out, int graphic, int w, int h)
{
    out->graphicIndex = graphic; out->loaded = 1; out->pixels = pixels;
    out->width = w; out->height = h;
}

int main(void)
{
    Dm1V1ChampionTopRowPresentationReceiptPc34 top;
    Dm1V1ChampionRedrawStatePc34 state;
    Dm1V1ChampionRedrawMaterialsPc34 materials;
    Dm1V1ChampionRedrawPriorityReceiptPc34 receipt;
    int poisonIndex = -1, damageIndex = -1;
    int ok = 1;

    memset(&top, 0, sizeof(top)); memset(&state, 0, sizeof(state));
    memset(&materials, 0, sizeof(materials));
    top.valid = 1; top.operationCount = 3;
    top.operations[0] = (Dm1V1ChampionTopRowPresentationOpPc34){
        .kind = DM1_V1_CHAMPION_TOP_ROW_OP_CLEAR_STATUS_PC34, .championSlot = 0,
        .zoneId = 151, .x = 0, .y = 0, .width = 67, .height = 29 };
    top.operations[1] = (Dm1V1ChampionTopRowPresentationOpPc34){
        .kind = DM1_V1_CHAMPION_TOP_ROW_OP_BLIT_HAND_PC34, .championSlot = 0,
        .zoneId = 212, .graphicIndex = 35, .sourcePixels = pixels,
        .x = 24, .y = 10, .width = 18, .height = 18 };
    top.operations[2] = (Dm1V1ChampionTopRowPresentationOpPc34){
        .kind = DM1_V1_CHAMPION_TOP_ROW_OP_BLIT_DEAD_STATUS_PC34, .championSlot = 1,
        .zoneId = 152, .graphicIndex = 8, .sourcePixels = pixels,
        .x = 69, .y = 0, .width = 67, .height = 29 };
    state.partyChampionCount = 2; state.inventoryChampionIndex = 0;
    state.present[0] = state.present[1] = 1;
    state.currentHealth[0] = 50; state.currentHealth[1] = 0;
    state.poisonDose[0] = state.poisonDose[1] = 1;
    state.pendingDamage[0] = state.pendingDamage[1] = 3;
    source(&materials.poisonLabel, 32, 96, 15);
    source(&materials.damageSmall, 15, 45, 7);
    source(&materials.damageBig, 16, 32, 29);

    ok &= check("source-bound priority receipt", dm1_v1_champion_redraw_priority_from_top_row_pc34(
        &top, &state, &materials, &receipt));
    ok &= check("dead terminal path", receipt.operationCount == 5);
    for (int i = 0; i < receipt.operationCount; ++i) {
        if (receipt.operations[i].kind == DM1_V1_CHAMPION_REDRAW_POISON_PC34) poisonIndex = i;
        if (receipt.operations[i].kind == DM1_V1_CHAMPION_REDRAW_DAMAGE_PC34) damageIndex = i;
    }
    ok &= check("status before poison", poisonIndex == 2 && receipt.operations[poisonIndex].graphicIndex == 32);
    ok &= check("inventory damage after poison", damageIndex == 3 && receipt.operations[damageIndex].graphicIndex == 16);
    ok &= check("dead only C008 status", receipt.operations[4].championSlot == 1 &&
                receipt.operations[4].graphicIndex == 8);
    materials.damageBig.pixels = NULL;
    ok &= check("selected damage material fails closed", !dm1_v1_champion_redraw_priority_from_top_row_pc34(
        &top, &state, &materials, &receipt));
    return ok ? 0 : 1;
}
