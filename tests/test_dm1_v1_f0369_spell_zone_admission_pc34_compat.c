#include "dm1_v1_f0369_spell_zone_admission_pc34_compat.h"

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

static void set_source_material(DM1_V1_ActionSpellHudMaterialReceiptPc34 *m)
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

int main(void)
{
    DM1_V1_F0369SpellZoneRequestPc34 request;
    DM1_V1_ActionSpellHudMaterialReceiptPc34 materials;
    DM1_V1_F0369SpellZoneReceiptPc34 receipt;

    memset(&request, 0, sizeof(request));
    set_source_material(&materials);
    request.magicCasterLive = 1;
    request.sourceTick = 81;
    request.screenX = 235;
    request.screenY = 51;
    CHECK(strstr(dm1_v1_f0369_spell_zone_source_evidence_pc34(),
                 "F0369_COMMAND_ProcessTypes101To108") != NULL);
    CHECK(dm1_v1_f0369_spell_zone_admit_pc34(&request, &materials, &receipt));
    CHECK(receipt.commandId == 101 && receipt.zoneIndex == 245 &&
          receipt.runeIndex == 0 && !receipt.recant && !receipt.cast &&
          receipt.parentGraphicId == 9 && receipt.linesGraphicId == 11 &&
          receipt.fontGraphicId == 695 && receipt.suppressSyntheticFallback);

    request.screenX = 305;
    request.screenY = 63;
    CHECK(dm1_v1_f0369_spell_zone_admit_pc34(&request, &materials, &receipt));
    CHECK(receipt.commandId == 107 && receipt.zoneIndex == 254 &&
          receipt.runeIndex == -1 && receipt.recant && !receipt.cast);

    request.screenX = 234;
    request.screenY = 63;
    CHECK(dm1_v1_f0369_spell_zone_admit_pc34(&request, &materials, &receipt));
    CHECK(receipt.commandId == 108 && receipt.zoneIndex == 252 &&
          receipt.runeIndex == -1 && !receipt.recant && receipt.cast);

    request.candidatePanelActive = 1;
    CHECK(!dm1_v1_f0369_spell_zone_admit_pc34(&request, &materials, &receipt));
    request.candidatePanelActive = 0;
    materials.fontGraphicId = 1;
    CHECK(!dm1_v1_f0369_spell_zone_admit_pc34(&request, &materials, &receipt));
    materials.fontGraphicId = 695;
    request.screenX = 232;
    request.screenY = 51;
    CHECK(!dm1_v1_f0369_spell_zone_admit_pc34(&request, &materials, &receipt));
    CHECK(!receipt.accepted && !receipt.suppressSyntheticFallback);

    printf("test_dm1_v1_f0369_spell_zone_admission_pc34_compat: %d assertions, %d failures\\n",
           assertions, failures);
    return failures == 0 ? 0 : 1;
}
