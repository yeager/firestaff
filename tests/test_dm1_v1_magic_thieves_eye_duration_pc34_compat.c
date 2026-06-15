/*
 * test_dm1_v1_magic_thieves_eye_duration_pc34_compat.c
 *
 * MNU-02 (audit, v2.7.x) regression — F0757 Thieves Eye duration
 * source-lock.
 *
 * Pins the F0757_MAGIC_ProduceOtherEffect_Compat C2_THIEVES_EYE
 * branch (ReDMCSB MENU.C:1945-1963) so that:
 *
 *   - Default build (no opt-in): durationTicks == 0 for every
 *     power ordinal 1..6, matching the original PC 3.4 broken
 *     behaviour where AL1267_ui_Ticks is uninitialised stack
 *     residue and resolves to 0 in a clean process. The followup
 *     event kind and aux0 stay pinned to TIMELINE_EVENT_STATUS_TIMEOUT
 *     and TIMELINE_AUX_THIEVES_EYE so the spell still schedules
 *     its decay correctly (it just fires immediately).
 *
 *   - Build with -DFIRESTAFF_PC34_LEGACY_THIEVES_EYE=1 OR with
 *     env var FIRESTAFF_DM1_THIEVES_EYE_LEGACY=1 at launch:
 *     durationTicks == spellPower * 40 (160..560 ticks, monotonic
 *     in powerOrdinal), the defensive envelope.
 *
 *   - The mode predicate F0757_MAGIC_ThievesEyeLegacyEnvelopeActive_Compat
 *     returns 0 in the source-locked default and 1 in the legacy
 *     opt-in.
 *
 *   - The non-thieves-eye branches (C0_LIGHT, C1_DARKNESS,
 *     C3_INVISIBILITY, C4_PARTY_SHIELD, C5_MAGIC_TORCH) are NOT
 *     affected by the MNU-02 switch.
 *
 * ReDMCSB anchors:
 *   - MENU.C:1945-1963 (F0412 C2_THIEVES_EYE branch with the
 *     uninitialised L1267_ui_Ticks).
 *   - DEFS.H C0..C5_SPELL_TYPE_OTHER_*.
 *
 * Pure data layer (M10 Phase 14). No UI, no IO, no globals.
 * Build linkage: firestaff_m10 only.
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

/* Build a C2_SPELL_TYPE_OTHER_THIEVES_EYE_COMPAT spell descriptor. */
static void make_thieves_eye_spell(struct SpellDefinition_Compat* s) {
    memset(s, 0, sizeof(*s));
    s->kind = C3_SPELL_KIND_OTHER_COMPAT;
    s->type = C2_SPELL_TYPE_OTHER_THIEVES_EYE_COMPAT;
    s->baseRequiredSkillLevel = 1;
    s->disabledTicks = 0;
}

static void make_magic_state(struct MagicState_Compat* m) {
    memset(m, 0, sizeof(*m));
    m->spellShieldDefense = 0;
    m->fireShieldDefense  = 0;
    m->partyShieldDefense = 0;
    m->magicalLightAmount = 0;
    m->event73CountThievesEye = 0;
}

/* ---- Test 1: source-locked default gives 0 ticks for all powers -- */
static void test_source_locked_default_zero_ticks(void) {
    struct SpellDefinition_Compat spell;
    struct MagicState_Compat magic;
    struct SpellEffect_Compat eff;
    int ordinal;
    int legacy;

    legacy = F0757_MAGIC_ThievesEyeLegacyEnvelopeActive_Compat();
    printf("# F0757 thieves-eye legacy envelope active = %d\n", legacy);
    if (legacy) {
        printf("  note: build is in legacy envelope mode; "
               "skipping source-locked default assertions\n");
        return;
    }

    make_thieves_eye_spell(&spell);
    make_magic_state(&magic);

    for (ordinal = 1; ordinal <= 6; ++ordinal) {
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
        CHECK(eff.durationTicks == 0,
              "F0757 source-locked default: durationTicks == 0");
        CHECK(eff.magicStateDelta[5] == 1,
              "F0757 magicStateDelta[5] == 1 (thieves eye set)");
        CHECK(eff.followupEventKind == TIMELINE_EVENT_STATUS_TIMEOUT,
              "F0757 followupEventKind == STATUS_TIMEOUT");
        CHECK(eff.followupEventAux0 == (int)TIMELINE_AUX_THIEVES_EYE,
              "F0757 followupEventAux0 == TIMELINE_AUX_THIEVES_EYE");
    }
    puts("  PASS source_locked_default_zero_ticks (ordinals 1..6)");
}

/* ---- Test 2: legacy envelope (when active) gives spellPower*40 ---- */
static void test_legacy_envelope_monotonic_spellpower_40(void) {
    struct SpellDefinition_Compat spell;
    struct MagicState_Compat magic;
    struct SpellEffect_Compat eff;
    int ordinal;
    int expected;
    int prev = 0;

    if (!F0757_MAGIC_ThievesEyeLegacyEnvelopeActive_Compat()) {
        printf("  skip: build is in source-locked default mode\n");
        return;
    }

    make_thieves_eye_spell(&spell);
    make_magic_state(&magic);

    for (ordinal = 1; ordinal <= 6; ++ordinal) {
        memset(&eff, 0, sizeof(eff));
        CHECK(F0757_MAGIC_ProduceOtherEffect_Compat(
                  &spell, ordinal, &magic, &eff) == 1,
              "F0757 returns 1 for valid input");
        /* spellPower = (ordinal+1) << 2, then >>= 1 = (ordinal+1) << 1 */
        expected = ((ordinal + 1) << 1) * 40;
        CHECK(eff.durationTicks == expected,
              "F0757 legacy envelope: durationTicks == spellPower*40");
        CHECK(eff.durationTicks > prev,
              "F0757 legacy envelope: monotonic in powerOrdinal");
        prev = eff.durationTicks;
    }
    puts("  PASS legacy_envelope_monotonic_spellpower_40 (ordinals 1..6)");
}

/* ---- Test 3: predicate is consistent with build flag / env ------ */
static void test_predicate_matches_build(void) {
#if defined(FIRESTAFF_PC34_LEGACY_THIEVES_EYE) && FIRESTAFF_PC34_LEGACY_THIEVES_EYE
    /* Build flag is unconditional, env cannot turn it off. */
    CHECK(F0757_MAGIC_ThievesEyeLegacyEnvelopeActive_Compat() == 1,
          "predicate: build flag forces legacy envelope");
#else
    /* Without build flag, the predicate is whatever the runtime
     * env var says — assert it returns a stable 0/1. */
    int got = F0757_MAGIC_ThievesEyeLegacyEnvelopeActive_Compat();
    CHECK(got == 0 || got == 1,
          "predicate: 0 or 1 only (no garbage)");
    if (got == 0) {
        /* Two consecutive calls must be deterministic. */
        int got2 = F0757_MAGIC_ThievesEyeLegacyEnvelopeActive_Compat();
        CHECK(got2 == 0,
              "predicate: deterministic across calls when no env");
    }
#endif
    puts("  PASS predicate_matches_build");
}

/* ---- Test 4: invalid inputs still rejected (both modes) --------- */
static void test_invalid_inputs_rejected(void) {
    struct SpellDefinition_Compat spell;
    struct MagicState_Compat magic;
    struct SpellEffect_Compat eff;

    make_thieves_eye_spell(&spell);
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

/* ---- Test 5: other spell kinds are NOT affected by MNU-02 ------ */
static void test_other_spells_unaffected(void) {
    struct SpellDefinition_Compat spell;
    struct MagicState_Compat magic;
    struct SpellEffect_Compat eff;

    make_magic_state(&magic);

    /* C1_DARKNESS: durationTicks must be 98 regardless of MNU-02. */
    memset(&spell, 0, sizeof(spell));
    spell.kind = C3_SPELL_KIND_OTHER_COMPAT;
    spell.type = C1_SPELL_TYPE_OTHER_DARKNESS_COMPAT;
    memset(&eff, 0, sizeof(eff));
    CHECK(F0757_MAGIC_ProduceOtherEffect_Compat(&spell, 3, &magic, &eff) == 1,
          "C1_DARKNESS returns 1");
    CHECK(eff.durationTicks == 98,
          "C1_DARKNESS durationTicks == 98 (MENU.C:1954-1957, unaffected by MNU-02)");

    /* C3_INVISIBILITY: spellPower <<= 3, then *40; ordinal 3 ->
     * (3+1)<<2 = 16, then <<= 3 = 128, then * 40 = 5120.
     * Unaffected by MNU-02. */
    memset(&spell, 0, sizeof(spell));
    spell.kind = C3_SPELL_KIND_OTHER_COMPAT;
    spell.type = C3_SPELL_TYPE_OTHER_INVISIBILITY_COMPAT;
    memset(&eff, 0, sizeof(eff));
    CHECK(F0757_MAGIC_ProduceOtherEffect_Compat(&spell, 3, &magic, &eff) == 1,
          "C3_INVISIBILITY returns 1");
    CHECK(eff.durationTicks == 5120,
          "C3_INVISIBILITY durationTicks == 5120 at powerOrdinal 3 (MENU.C:1970-1982)");

    /* C4_PARTY_SHIELD: spellPower*40; ordinal 3 -> 16 * 40 = 640. */
    memset(&spell, 0, sizeof(spell));
    spell.kind = C3_SPELL_KIND_OTHER_COMPAT;
    spell.type = C4_SPELL_TYPE_OTHER_PARTY_SHIELD_COMPAT;
    memset(&eff, 0, sizeof(eff));
    CHECK(F0757_MAGIC_ProduceOtherEffect_Compat(&spell, 3, &magic, &eff) == 1,
          "C4_PARTY_SHIELD returns 1");
    CHECK(eff.durationTicks == 640,
          "C4_PARTY_SHIELD durationTicks == 640 at powerOrdinal 3 (MENU.C:1988-1996)");

    puts("  PASS other_spells_unaffected");
}

int main(void) {
    printf("# dm1_v1_magic_thieves_eye_duration_pc34_compat (MNU-02)\n");
    test_source_locked_default_zero_ticks();
    test_legacy_envelope_monotonic_spellpower_40();
    test_predicate_matches_build();
    test_invalid_inputs_rejected();
    test_other_spells_unaffected();
    puts("PASS dm1_v1_magic_thieves_eye_duration_source_lock");
    return 0;
}
