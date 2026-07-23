#include "dm1_v1_f0410_f0411_spell_cast_continuation_pc34_compat.h"

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
                      DM1_V1_F0369SpellZoneReceiptPc34 *zone,
                      DM1_V1_F0408F0409SpellCastReceiptPc34 *cast,
                      DM1_SpellCastingState *state)
{
    DM1_V1_F0369SpellZoneRequestPc34 zoneRequest;
    DM1_V1_F0408F0409SpellCastRequestPc34 castRequest;
    memset(&zoneRequest, 0, sizeof(zoneRequest));
    memset(&castRequest, 0, sizeof(castRequest));
    zoneRequest.screenX = 234;
    zoneRequest.screenY = 63;
    zoneRequest.magicCasterLive = 1;
    zoneRequest.sourceTick = tick;
    castRequest.casterIndex = 0;
    castRequest.magicCasterLive = 1;
    castRequest.sourceTick = tick;
    return dm1_v1_f0369_spell_zone_admit_pc34(&zoneRequest, materials, zone) &&
           dm1_v1_f0408_f0409_spell_cast_admit_pc34(
               &castRequest, zone, materials, state, cast);
}

int main(void)
{
    DM1_V1_ActionSpellHudMaterialReceiptPc34 materials;
    DM1_V1_F0369SpellZoneReceiptPc34 zone;
    DM1_V1_F0408F0409SpellCastReceiptPc34 cast;
    DM1_V1_F0410F0411SpellCastRequestPc34 request;
    DM1_V1_F0410F0411SpellCastReceiptPc34 receipt;
    DM1_SpellCastingState state;
    DM1_SpellPotionInventory inventory;
    DM1_SpellF0412RuntimeReceipt runtime;

    set_material(&materials);
    dm1_spell_init(&state);
    memset(&inventory, 0, sizeof(inventory));
    inventory.slots[DM1_SPELL_SLOT_ACTION_HAND] = 0x8401u;
    inventory.slots[DM1_SPELL_SLOT_READY_HAND] = 0x8402u;
    inventory.potionCount = 2;
    inventory.potions[0].thing = 0x8401u;
    inventory.potions[0].iconIndex = DM1_SPELL_ICON_EMPTY_FLASK_PC34;
    inventory.potions[0].type = DM1_SPELL_POTION_EMPTY_FLASK_PC34;
    inventory.potions[1].thing = 0x8402u;
    inventory.potions[1].iconIndex = DM1_SPELL_ICON_EMPTY_FLASK_PC34;
    inventory.potions[1].type = DM1_SPELL_POTION_EMPTY_FLASK_PC34;
    memset(&request, 0, sizeof(request));
    request.casterIndex = 0;
    request.magicCasterLive = 1;
    request.sourceTick = 111;
    CHECK(strstr(dm1_v1_f0410_f0411_spell_cast_source_evidence_pc34(),
                 "SPELFAIL.C") != NULL);
    CHECK(admit_cast(request.sourceTick, &materials, &zone, &cast, &state));

    memset(&runtime, 0, sizeof(runtime));
    runtime.castResult = DM1_SPELL_CAST_FAILURE;
    runtime.failureType = DM1_FAILURE_MEANINGLESS_SPELL;
    runtime.spellKind = DM1_SPELL_KIND_POTION;
    CHECK(dm1_v1_f0410_f0411_spell_cast_continue_pc34(
        &request, &zone, &cast, &materials, &runtime, &inventory, &receipt));
    CHECK(receipt.accepted && receipt.failureFeedbackAdmitted &&
          receipt.failureType == DM1_FAILURE_MEANINGLESS_SPELL &&
          receipt.messageColor == 4 && receipt.printsChampionName &&
          !receipt.appendsBaseSkillName && receipt.flaskFound &&
          receipt.flaskSlotIndex == DM1_SPELL_SLOT_ACTION_HAND &&
          receipt.flaskThing == 0x8401u && receipt.suppressSyntheticFallback);

    request.sourceTick = 112;
    CHECK(!dm1_v1_f0410_f0411_spell_cast_continue_pc34(
        &request, &zone, &cast, &materials, &runtime, &inventory, &receipt));
    CHECK(!receipt.accepted && receipt.flaskSlotIndex == -1);

    request.sourceTick = 111;
    runtime.failureType = 99;
    CHECK(!dm1_v1_f0410_f0411_spell_cast_continue_pc34(
        &request, &zone, &cast, &materials, &runtime, &inventory, &receipt));
    CHECK(!receipt.accepted && receipt.flaskSlotIndex == -1);

    runtime.castResult = DM1_SPELL_CAST_SUCCESS;
    runtime.failureType = -1;
    runtime.spellKind = DM1_SPELL_KIND_POTION;
    materials.fontGraphicId = 1;
    CHECK(!dm1_v1_f0410_f0411_spell_cast_continue_pc34(
        &request, &zone, &cast, &materials, &runtime, &inventory, &receipt));
    CHECK(!receipt.accepted && receipt.flaskSlotIndex == -1);

    printf("test_dm1_v1_f0410_f0411_spell_cast_continuation_pc34_compat: %d assertions, %d failures\\n",
           assertions, failures);
    return failures == 0 ? 0 : 1;
}
