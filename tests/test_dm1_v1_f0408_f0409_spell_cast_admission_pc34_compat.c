#include "dm1_v1_f0408_f0409_spell_cast_admission_pc34_compat.h"

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

static int admit_cast(unsigned int tick,
                      DM1_V1_ActionSpellHudMaterialReceiptPc34 *materials,
                      DM1_V1_F0369SpellZoneReceiptPc34 *out)
{
    DM1_V1_F0369SpellZoneRequestPc34 request;
    memset(&request, 0, sizeof(request));
    request.screenX = 234;
    request.screenY = 63;
    request.magicCasterLive = 1;
    request.sourceTick = tick;
    return dm1_v1_f0369_spell_zone_admit_pc34(&request, materials, out);
}

int main(void)
{
    DM1_V1_ActionSpellHudMaterialReceiptPc34 materials;
    DM1_V1_F0369SpellZoneReceiptPc34 zone;
    DM1_V1_F0408F0409SpellCastRequestPc34 request;
    DM1_V1_F0408F0409SpellCastReceiptPc34 receipt;
    DM1_SpellCastingState state;
    DM1_ChampionSpellStats stats;

    set_material(&materials);
    dm1_spell_init(&state);
    memset(&stats, 0, sizeof(stats));
    stats.currentHealth = 100;
    stats.currentMana = 50;
    CHECK(dm1_spell_addSymbol(&state, 0, &stats, 0));
    CHECK(dm1_spell_addSymbol(&state, 0, &stats, 0));
    memset(&request, 0, sizeof(request));
    request.casterIndex = 0;
    request.magicCasterLive = 1;
    request.sourceTick = 101;
    CHECK(strstr(dm1_v1_f0408_f0409_spell_cast_source_evidence_pc34(),
                 "F0408_MENUS_GetClickOnSpellCastResult") != NULL);
    CHECK(admit_cast(request.sourceTick, &materials, &zone));
    CHECK(dm1_v1_f0408_f0409_spell_cast_admit_pc34(
        &request, &zone, &materials, &state, &receipt));
    CHECK(receipt.accepted && receipt.commandId == 108 && receipt.lookupMatched &&
          receipt.spellIndex == 16 && receipt.powerOrdinal == 1 &&
          receipt.f0412DispatchRequired && receipt.suppressSyntheticFallback &&
          state.input[0].symbols[0] == 96 && state.input[0].symbols[1] == 102);

    request.sourceTick = 102;
    CHECK(!dm1_v1_f0408_f0409_spell_cast_admit_pc34(
        &request, &zone, &materials, &state, &receipt));
    CHECK(!receipt.accepted && state.input[0].symbols[0] == 96);

    request.sourceTick = 101;
    state.input[0].symbols[1] = '\0';
    state.input[0].symbolStep = 1;
    CHECK(dm1_v1_f0408_f0409_spell_cast_admit_pc34(
        &request, &zone, &materials, &state, &receipt));
    CHECK(receipt.accepted && !receipt.lookupMatched && receipt.spellIndex == -1 &&
          receipt.f0412DispatchRequired && state.input[0].symbols[0] == 96);

    materials.fontGraphicId = 1;
    CHECK(!dm1_v1_f0408_f0409_spell_cast_admit_pc34(
        &request, &zone, &materials, &state, &receipt));
    CHECK(!receipt.accepted && state.input[0].symbols[0] == 96);

    printf("test_dm1_v1_f0408_f0409_spell_cast_admission_pc34_compat: %d assertions, %d failures\\n",
           assertions, failures);
    return failures == 0 ? 0 : 1;
}
