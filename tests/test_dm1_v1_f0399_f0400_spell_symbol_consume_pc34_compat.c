#include "dm1_v1_f0399_f0400_spell_symbol_consume_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int assertions;
static int failures;

#define CHECK(expression) do { \
    ++assertions; \
    if (!(expression)) { \
        ++failures; \
        fprintf(stderr, "%s:%d: %s\\n", __FILE__, __LINE__, #expression); \
    } \
} while (0)

static void set_material(DM1_V1_ActionSpellHudMaterialReceiptPc34 *m)
{
    memset(m, 0, sizeof(*m));
    m->accepted = 1;
    m->drawable = 1;
    m->sourceSurfaceCount = 3;
    m->primaryGraphicId = 9;
    m->secondaryGraphicId = 11;
    m->primaryZoneId = 13;
    m->fontGraphicId = 695;
}

static int admit_zone(int x, int y, unsigned int tick,
                      DM1_V1_ActionSpellHudMaterialReceiptPc34 *materials,
                      DM1_V1_F0369SpellZoneReceiptPc34 *out)
{
    DM1_V1_F0369SpellZoneRequestPc34 request;
    memset(&request, 0, sizeof(request));
    request.screenX = x;
    request.screenY = y;
    request.magicCasterLive = 1;
    request.sourceTick = tick;
    return dm1_v1_f0369_spell_zone_admit_pc34(&request, materials, out);
}

int main(void)
{
    DM1_V1_ActionSpellHudMaterialReceiptPc34 materials;
    DM1_V1_F0369SpellZoneReceiptPc34 zone;
    DM1_V1_F0399F0400SpellSymbolRequestPc34 request;
    DM1_V1_F0399F0400SpellSymbolReceiptPc34 receipt;
    DM1_SpellCastingState state;
    DM1_ChampionSpellStats stats;

    set_material(&materials);
    dm1_spell_init(&state);
    memset(&stats, 0, sizeof(stats));
    stats.currentHealth = 100;
    stats.currentMana = 50;
    memset(&request, 0, sizeof(request));
    request.casterIndex = 0;
    request.magicCasterLive = 1;
    request.sourceTick = 91;
    CHECK(strstr(dm1_v1_f0399_f0400_spell_symbol_source_evidence_pc34(),
                 "F0399_MENUS_AddChampionSymbol") != NULL);
    CHECK(admit_zone(235, 51, request.sourceTick, &materials, &zone));
    CHECK(dm1_v1_f0399_f0400_spell_symbol_consume_pc34(
        &request, &zone, &materials, &state, &stats, &receipt));
    CHECK(receipt.accepted && receipt.commandId == 101 && receipt.symbolAdded &&
          !receipt.symbolDeleted && receipt.symbolStepBefore == 0 &&
          receipt.symbolStepAfter == 1 && receipt.manaBefore == 50 &&
          receipt.manaAfter == 49 && state.input[0].symbols[0] == 96 &&
          receipt.suppressSyntheticFallback);

    CHECK(admit_zone(305, 63, request.sourceTick, &materials, &zone));
    CHECK(dm1_v1_f0399_f0400_spell_symbol_consume_pc34(
        &request, &zone, &materials, &state, &stats, &receipt));
    CHECK(receipt.commandId == 107 && !receipt.symbolAdded &&
          receipt.symbolDeleted && receipt.symbolStepBefore == 1 &&
          receipt.symbolStepAfter == 0 && receipt.manaBefore == 49 &&
          receipt.manaAfter == 49 && state.input[0].symbols[0] == '\0');

    CHECK(admit_zone(235, 51, request.sourceTick, &materials, &zone));
    request.sourceTick = 92;
    CHECK(!dm1_v1_f0399_f0400_spell_symbol_consume_pc34(
        &request, &zone, &materials, &state, &stats, &receipt));
    CHECK(!receipt.accepted && state.input[0].symbols[0] == '\0' &&
          stats.currentMana == 49);

    request.sourceTick = 91;
    materials.fontGraphicId = 1;
    CHECK(!dm1_v1_f0399_f0400_spell_symbol_consume_pc34(
        &request, &zone, &materials, &state, &stats, &receipt));
    CHECK(!receipt.accepted && state.input[0].symbols[0] == '\0');

    printf("test_dm1_v1_f0399_f0400_spell_symbol_consume_pc34_compat: %d assertions, %d failures\\n",
           assertions, failures);
    return failures == 0 ? 0 : 1;
}
