#include "dm1_v1_action_spell_presentation_sequence_pc34_compat.h"

#include "firestaff/dm1/v1/box_action_area_pc34_compat.h"
#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { ++failures; fprintf(stderr, "FAIL: %s\n", #c); } } while (0)

static void make_base(
    DM1_V1_ActionSpellHudPresentationReceiptPc34 *presentation,
    DM1_V1_ActionSpellHudMaterialReceiptPc34 *materials,
    int kind)
{
    memset(presentation, 0, sizeof(*presentation));
    memset(materials, 0, sizeof(*materials));
    presentation->valid = 1;
    presentation->drawable = 1;
    presentation->suppressSyntheticFallback = 1;
    presentation->presentationKind = kind;
    materials->accepted = 1;
    materials->drawable = 1;
    materials->presentationKind = kind;
}

int main(void)
{
    DM1_V1_ActionSpellHudPresentationReceiptPc34 presentation;
    DM1_V1_ActionSpellHudMaterialReceiptPc34 materials;
    DM1_V1_ActionSpellPresentationSequenceReceiptPc34 sequence;

    make_base(&presentation, &materials,
              DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34);
    presentation.requiresRealActionMenuLayout = 1;
    materials.sourceSurfaceCount = 2;
    materials.primaryGraphicId = DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34;
    materials.primaryZoneId = DM1_V1_ACTION_AREA_ZONE_ID_PC34;
    materials.fontGraphicId = 695;
    materials.secondaryZoneId = 77;
    CHECK(dm1_v1_action_spell_presentation_sequence_build_pc34(
              &presentation, &materials, 2, &sequence));
    CHECK(sequence.stepCount == 4);
    CHECK(sequence.steps[0].kind == DM1_V1_ACTION_SPELL_SEQUENCE_STEP_BLIT_PC34);
    CHECK(sequence.steps[0].graphicId == 10 && sequence.steps[0].zoneId == 11);
    CHECK(sequence.steps[1].zoneId == 80);
    CHECK(sequence.steps[2].zoneId == 85 && sequence.steps[3].zoneId == 86);
    materials.secondaryZoneId = 11;
    CHECK(!dm1_v1_action_spell_presentation_sequence_build_pc34(
              &presentation, &materials, 2, &sequence));

    make_base(&presentation, &materials,
              DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34);
    presentation.requiresRealSpellAreaLayout = 1;
    materials.sourceSurfaceCount = 3;
    materials.primaryGraphicId = 9;
    materials.secondaryGraphicId = 11;
    materials.primaryZoneId = 13;
    materials.fontGraphicId = 695;
    CHECK(dm1_v1_action_spell_presentation_sequence_build_pc34(
              &presentation, &materials, 0, &sequence));
    CHECK(sequence.stepCount == 4);
    CHECK(sequence.steps[0].graphicId == 9 && sequence.steps[0].zoneId == 13);
    CHECK(sequence.steps[1].graphicId == 11 && sequence.steps[1].zoneId == 245 &&
          sequence.steps[1].zoneCount == 6 && sequence.steps[1].sourceY == 13);
    CHECK(sequence.steps[2].graphicId == 11 && sequence.steps[2].zoneId == 261 &&
          sequence.steps[2].zoneCount == 4 && sequence.steps[2].sourceY == 26);
    CHECK(sequence.steps[3].graphicId == 695 && sequence.steps[3].zoneId == 221);
    materials.fontGraphicId = 123;
    CHECK(!dm1_v1_action_spell_presentation_sequence_build_pc34(
              &presentation, &materials, 0, &sequence));

    make_base(&presentation, &materials,
              DM1_V1_ACTION_HUD_PRESENTATION_DAMAGE_PC34);
    materials.sourceSurfaceCount = 2;
    materials.primaryGraphicId = 14;
    materials.primaryZoneId = 75;
    materials.fontGraphicId = 695;
    CHECK(dm1_v1_action_spell_presentation_sequence_build_pc34(
              &presentation, &materials, 0, &sequence));
    CHECK(sequence.stepCount == 2 && sequence.steps[0].graphicId == 14 &&
          sequence.steps[0].sourceW == 88 && sequence.steps[0].sourceH == 45);

    printf("%s\n", failures ? "failed" : "ok: action/spell source sequence");
    return failures ? 1 : 0;
}
