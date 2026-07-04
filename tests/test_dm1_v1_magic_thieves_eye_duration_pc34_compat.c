/*
 * test_dm1_v1_magic_thieves_eye_duration_pc34_compat.c
 *
 * Pins the F0757_MAGIC_ProduceOtherEffect_Compat C2_THIEVES_EYE
 * branch against ReDMCSB MENU.C F0412 lines 1945-1963.
 *
 * In the decompiled source AL1267_ui_Ticks, AL1267_ui_SpellPower and
 * AL1267_ui_Multiple all alias L1267. The C2 branch shifts spellPower
 * right once, then T0412032 multiplies L1267 by itself before T0412033
 * schedules the C73 status timeout.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_magic_pc34_compat.h"

#define CHECK(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("FAIL %s (line %d): %s\n", msg, __LINE__, #cond); \
            exit(1); \
        } \
    } while (0)

static void make_other_spell(struct SpellDefinition_Compat* s, int type) {
    memset(s, 0, sizeof(*s));
    s->kind = C3_SPELL_KIND_OTHER_COMPAT;
    s->type = type;
    s->baseRequiredSkillLevel = 1;
    s->disabledTicks = 0;
}

static void make_magic_map_spell(struct SpellDefinition_Compat* s, int type) {
    memset(s, 0, sizeof(*s));
    s->kind = C4_SPELL_KIND_MAGIC_MAP_COMPAT;
    s->type = type;
    s->baseRequiredSkillLevel = 1;
    s->disabledTicks = 0;
}

static void make_magic_state(struct MagicState_Compat* m) {
    memset(m, 0, sizeof(*m));
}

static int spell_power_for(int ordinal) {
    return (ordinal + 1) << 2;
}

static void test_thieves_eye_squares_shifted_spellpower(void) {
    struct SpellDefinition_Compat spell;
    struct MagicState_Compat magic;
    struct SpellEffect_Compat eff;
    int ordinal;
    int prev = 0;

    make_other_spell(&spell, C2_SPELL_TYPE_OTHER_THIEVES_EYE_COMPAT);
    make_magic_state(&magic);

    for (ordinal = 1; ordinal <= 6; ++ordinal) {
        int shiftedPower = spell_power_for(ordinal) >> 1;
        int expectedTicks = shiftedPower * shiftedPower;
        memset(&eff, 0, sizeof(eff));
        CHECK(F0757_MAGIC_ProduceOtherEffect_Compat(
                  &spell, ordinal, &magic, &eff) == 1,
              "F0757 returns 1 for valid input");
        CHECK(eff.spellKind == C3_SPELL_KIND_OTHER_COMPAT,
              "F0757 spellKind == C3 (OTHER)");
        CHECK(eff.spellType == C2_SPELL_TYPE_OTHER_THIEVES_EYE_COMPAT,
              "F0757 spellType == C2 (THIEVES_EYE)");
        CHECK(eff.powerOrdinal == ordinal,
              "F0757 powerOrdinal echoed back");
        CHECK(eff.durationTicks == expectedTicks,
              "F0757 Thieves Eye duration = shifted spellPower^2");
        CHECK(eff.durationTicks > prev,
              "F0757 Thieves Eye duration is monotonic");
        CHECK(eff.magicStateDelta[5] == 1,
              "F0757 magicStateDelta[5] == 1 (thieves eye set)");
        CHECK(eff.followupEventKind == TIMELINE_EVENT_STATUS_TIMEOUT,
              "F0757 followupEventKind == STATUS_TIMEOUT");
        CHECK(eff.followupEventAux0 == (int)TIMELINE_AUX_THIEVES_EYE,
              "F0757 followupEventAux0 == TIMELINE_AUX_THIEVES_EYE");
        prev = eff.durationTicks;
    }
    puts("  PASS thieves_eye_squares_shifted_spellpower");
}

static void test_invalid_inputs_rejected(void) {
    struct SpellDefinition_Compat spell;
    struct MagicState_Compat magic;
    struct SpellEffect_Compat eff;

    make_other_spell(&spell, C2_SPELL_TYPE_OTHER_THIEVES_EYE_COMPAT);
    make_magic_state(&magic);

    CHECK(F0757_MAGIC_ProduceOtherEffect_Compat(0, 3, &magic, &eff) == 0,
          "F0757 rejects NULL spell");
    CHECK(F0757_MAGIC_ProduceOtherEffect_Compat(&spell, 3, 0, &eff) == 0,
          "F0757 rejects NULL magic");
    CHECK(F0757_MAGIC_ProduceOtherEffect_Compat(&spell, 3, &magic, 0) == 0,
          "F0757 rejects NULL out");
    CHECK(F0757_MAGIC_ProduceOtherEffect_Compat(&spell, 0, &magic, &eff) == 0,
          "F0757 rejects powerOrdinal 0");
    CHECK(F0757_MAGIC_ProduceOtherEffect_Compat(&spell, 7, &magic, &eff) == 0,
          "F0757 rejects powerOrdinal 7");
    puts("  PASS invalid_inputs_rejected");
}

static void test_related_status_duration_branches(void) {
    struct SpellDefinition_Compat spell;
    struct MagicState_Compat magic;
    struct SpellEffect_Compat eff;

    make_magic_state(&magic);

    make_other_spell(&spell, C1_SPELL_TYPE_OTHER_DARKNESS_COMPAT);
    memset(&eff, 0, sizeof(eff));
    CHECK(F0757_MAGIC_ProduceOtherEffect_Compat(&spell, 3, &magic, &eff) == 1,
          "C1_DARKNESS returns 1");
    CHECK(eff.durationTicks == 98,
          "C1_DARKNESS durationTicks == 98");

    make_other_spell(&spell, C3_SPELL_TYPE_OTHER_INVISIBILITY_COMPAT);
    memset(&eff, 0, sizeof(eff));
    CHECK(F0757_MAGIC_ProduceOtherEffect_Compat(&spell, 3, &magic, &eff) == 1,
          "C3_INVISIBILITY returns 1");
    CHECK(eff.durationTicks == 128,
          "C3_INVISIBILITY durationTicks == shifted spellPower");

    make_other_spell(&spell, C4_SPELL_TYPE_OTHER_PARTY_SHIELD_COMPAT);
    memset(&eff, 0, sizeof(eff));
    CHECK(F0757_MAGIC_ProduceOtherEffect_Compat(&spell, 3, &magic, &eff) == 1,
          "C4_PARTY_SHIELD returns 1");
    CHECK(eff.durationTicks == 256,
          "C4_PARTY_SHIELD durationTicks == spellPower^2");

    make_other_spell(&spell, C6_SPELL_TYPE_OTHER_FOOTPRINTS_COMPAT);
    memset(&eff, 0, sizeof(eff));
    CHECK(F0757_MAGIC_ProduceOtherEffect_Compat(&spell, 3, &magic, &eff) == 1,
          "C6_FOOTPRINTS returns 1");
    CHECK(eff.durationTicks == 256,
          "C6_FOOTPRINTS durationTicks == spellPower^2");

    puts("  PASS related_status_duration_branches");
}

static void test_magic_map_duration_branches(void) {
    struct SpellDefinition_Compat spell;
    struct SpellEffect_Compat eff;
    int power = spell_power_for(3);

    make_magic_map_spell(&spell, C0_SPELL_TYPE_MAGIC_MAP_WIZARD_COMPAT);
    CHECK(F0759_MAGIC_ProduceMagicMapEffect_Compat(
              &spell, 3, 1, 2, 1, &eff) == 1,
          "magic map wizard returns 1");
    CHECK(eff.castResult == SPELL_CAST_SUCCESS,
          "magic map wizard succeeds with map");
    CHECK(eff.durationTicks == power * power * 2,
          "magic map wizard duration = power^2*2");
    CHECK(eff.followupEventAux0 == TIMELINE_AUX_MAGIC_MAP_WIZARD,
          "magic map wizard aux0 C80");
    CHECK(eff.followupEventAux1 == 2,
          "magic map wizard aux1 champion");

    make_magic_map_spell(&spell, C1_SPELL_TYPE_MAGIC_MAP_NINJA_COMPAT);
    CHECK(F0759_MAGIC_ProduceMagicMapEffect_Compat(
              &spell, 3, 1, 1, 0, &eff) == 1,
          "magic map ninja returns 1");
    CHECK(eff.durationTicks == (power - 2) * (power - 2) * 2,
          "magic map ninja duration = adjusted power^2*2");
    CHECK(eff.followupEventAux0 == TIMELINE_AUX_MAGIC_MAP_NINJA,
          "magic map ninja aux0 C81");

    make_magic_map_spell(&spell, C2_SPELL_TYPE_MAGIC_MAP_FIGHTER_COMPAT);
    CHECK(F0759_MAGIC_ProduceMagicMapEffect_Compat(
              &spell, 3, 1, 0, 0, &eff) == 1,
          "magic map fighter returns 1");
    CHECK(eff.durationTicks ==
              (power - ((power >> 2) - 1)) *
              (power - ((power >> 2) - 1)) * 2,
          "magic map fighter duration = adjusted power^2*2");
    CHECK(eff.followupEventAux0 == TIMELINE_AUX_MAGIC_MAP_FIGHTER,
          "magic map fighter aux0 C82");

    make_magic_map_spell(&spell, C3_SPELL_TYPE_MAGIC_MAP_PRIEST_COMPAT);
    CHECK(F0759_MAGIC_ProduceMagicMapEffect_Compat(
              &spell, 3, 1, 3, 2, &eff) == 1,
          "magic map priest returns 1");
    CHECK(eff.durationTicks == power * power * 2,
          "magic map priest duration = power^2*2");
    CHECK(eff.followupEventAux0 == TIMELINE_AUX_MAGIC_MAP_PRIEST,
          "magic map priest aux0 C83");
    CHECK(eff.followupEventAux1 == 3,
          "magic map priest aux1 champion");
    CHECK(eff.magicStateDelta[0] == 2,
          "magic map priest direction is carried in timeline aux2 delta slot");

    CHECK(F0759_MAGIC_ProduceMagicMapEffect_Compat(
              &spell, 3, 0, 0, 0, &eff) == 1,
          "magic map missing-map returns handled failure");
    CHECK(eff.castResult == SPELL_CAST_FAILURE_NEEDS_FLASK,
          "magic map missing-map uses source failure result 3");
    CHECK(eff.failureReason == SPELL_FAILURE_NEEDS_MAGIC_MAP,
          "magic map missing-map failure reason C11");

    puts("  PASS magic_map_duration_branches");
}

int main(void) {
    printf("# dm1_v1_magic_thieves_eye_duration_pc34_compat\n");
    test_thieves_eye_squares_shifted_spellpower();
    test_invalid_inputs_rejected();
    test_related_status_duration_branches();
    test_magic_map_duration_branches();
    puts("PASS dm1_v1_magic_thieves_eye_duration_source_lock");
    return 0;
}
