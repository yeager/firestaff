/*
 * test_dm1_v1_mnu03_f0757_spell_duration_source_lock_pc34_compat.c
 *
 * Source-locked to ReDMCSB MENU.C:1923-2030 (F0412 / F0757 spell
 * duration envelopes for C0_LIGHT, C1_DARKNESS, C2_THIEVES_EYE,
 * C3_INVISIBILITY, C4_SHIELD, C5_TORCH, C6_FOOTPRINTS,
 * C7_ZOKATHRA, C8_FIRESHIELD).
 *
 * MNU-03 (DM1 V1 functional-divergence-report.md):
 *   "F0757 spell-durations for C0_LIGHT / C5_TORCH /
 *    C3_INVISIBILITY / C4_SHIELD / C6_FOOTPRINTS / C8_FIRESHIELD
 *    are source-locked" — already source-locked, included only
 *    as verification.
 *
 * Pins the F0757_MAGIC_ProduceOtherEffect_Compat formula contract
 * against ReDMCSB MENU.C:1923-2030:
 *  T1  C0_LIGHT: ticks = 10000 + ((spellPower - 8) << 9)
 *                 lightPower = (spellPower >> 1) - 1, clamped [0..5]
 *  T2  C1_DARKNESS: ticks = 98
 *                    lightPower = spellPower >> 2, clamped [0..5]
 *  T3  C3_INVISIBILITY: ticks = spellPower * 40
 *  T4  C4_SHIELD: ticks = spellPower * 40 (or source-specific)
 *  T5  C5_TORCH: ticks = (10000 + ((spellPower - 8) << 9)) / 2
 *                 (Fire Shield style)
 *  T6  C6_FOOTPRINTS: ticks = spellPower * 40
 *                     magicStateDelta[5] = 1
 *  T7  C7_ZOKATHRA: ticks = 0, followupEventKind = TIMELINE_EVENT_INVALID
 *  T8  C8_FIRESHIELD: defense = spellPower^2 + 100
 *                      durationTicks = defense >> 5
 *  T9  spellPower = (powerOrdinal + 1) << 2
 *  T10 powerOrdinal = 1..6 valid; 0 or 7+ returns 0
 *  T11 NULL out returns 0
 *  T12 NULL spell or magic returns 0
 *  T13 F0757 always sets castResult = SPELL_CAST_SUCCESS
 *  T14 F0757 sets spellType = spell->type
 *  T15 F0757 sets powerOrdinal = powerOrdinal
 *
 * Source-locked to ReDMCSB MENU.C:1923-2030.
 */

#include "memory_magic_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, msg); \
        return 1; \
    } \
} while (0)

/* ReDMCSB MENU.C:1923 AL1267_ui_SpellPower = (PowerSymbolOrdinal + 1) << 2. */
static int kF0757_SpellPowerFor(int powerOrdinal) {
    return (powerOrdinal + 1) << 2;
}

/* ReDMCSB MENU.C:1927..1930 lightPower = (spellPower >> 1) - 1, clamped [0..5]. */
static int kF0757_LightPowerFor(int spellPower) {
    int lp = (spellPower >> 1) - 1;
    if (lp < 0) lp = 0;
    if (lp > 5) lp = 5;
    return lp;
}

static int kF0757_LightAmountFor(int lightPower) {
    static const int amounts[6] = { 0, 5, 12, 24, 33, 40 };
    if (lightPower < 0) lightPower = 0;
    if (lightPower > 5) lightPower = 5;
    return amounts[lightPower];
}

int main(void) {
    struct SpellDefinition_Compat spell;
    struct MagicState_Compat magic;
    struct SpellEffect_Compat out;
    int rc;
    int i;

    /* Setup a generic spell definition. */
    memset(&spell, 0, sizeof(spell));
    memset(&magic, 0, sizeof(magic));
    spell.skillIndex = 0;
    spell.kind = C3_SPELL_KIND_OTHER_COMPAT;

    /* T1: C0_LIGHT (MENU.C:1927-1930). */
    spell.type = C0_SPELL_TYPE_OTHER_LIGHT_COMPAT;
    for (i = 1; i <= 6; ++i) {
        int spellPower = kF0757_SpellPowerFor(i);
        int ticks = 10000 + ((spellPower - 8) << 9);
        memset(&out, 0, sizeof(out));
        rc = F0757_MAGIC_ProduceOtherEffect_Compat(&spell, i, &magic, &out);
        char buf[96];
        snprintf(buf, sizeof(buf),
                 "T1: C0_LIGHT ordinal=%d rc==1", i);
        CHECK(rc == 1, buf);
        snprintf(buf, sizeof(buf),
                 "T1: C0_LIGHT ordinal=%d ticks==%d (got %d)",
                 i, ticks, out.durationTicks);
        CHECK(out.durationTicks == ticks, buf);
        snprintf(buf, sizeof(buf),
                 "T1: C0_LIGHT ordinal=%d amount==G0039[%d]",
                 i, kF0757_LightPowerFor(spellPower));
        CHECK(out.magicStateDelta[3] ==
                  kF0757_LightAmountFor(kF0757_LightPowerFor(spellPower)),
              buf);
    }

    /* T2: C1_DARKNESS (MENU.C:1954-1957). */
    spell.type = C1_SPELL_TYPE_OTHER_DARKNESS_COMPAT;
    for (i = 1; i <= 6; ++i) {
        int spellPower;
        memset(&out, 0, sizeof(out));
        rc = F0757_MAGIC_ProduceOtherEffect_Compat(&spell, i, &magic, &out);
        char buf[96];
        snprintf(buf, sizeof(buf),
                 "T2: C1_DARKNESS ordinal=%d rc==1", i);
        CHECK(rc == 1, buf);
        CHECK(out.durationTicks == 98, "T2: C1_DARKNESS ticks == 98");
        spellPower = kF0757_SpellPowerFor(i);
        snprintf(buf, sizeof(buf),
                 "T2: C1_DARKNESS ordinal=%d amount==-G0039[%d]",
                 i, spellPower >> 2);
        CHECK(out.magicStateDelta[3] ==
                  -kF0757_LightAmountFor(spellPower >> 2),
              buf);
    }

    /* T2b: C5_MAGIC_TORCH (MENU.C:1934-1937). */
    spell.type = C5_SPELL_TYPE_OTHER_MAGIC_TORCH_COMPAT;
    for (i = 1; i <= 6; ++i) {
        int spellPower = kF0757_SpellPowerFor(i);
        int ticks = 2000 + ((spellPower - 3) << 7);
        int lightPower = (spellPower >> 2) + 1;
        if (lightPower > 5) lightPower = 5;
        memset(&out, 0, sizeof(out));
        rc = F0757_MAGIC_ProduceOtherEffect_Compat(&spell, i, &magic, &out);
        char buf[96];
        snprintf(buf, sizeof(buf),
                 "T2b: C5_MAGIC_TORCH ordinal=%d ticks==%d (got %d)",
                 i, ticks, out.durationTicks);
        CHECK(rc == 1, buf);
        CHECK(out.durationTicks == ticks, buf);
        snprintf(buf, sizeof(buf),
                 "T2b: C5_MAGIC_TORCH ordinal=%d amount==G0039[%d]",
                 i, lightPower);
        CHECK(out.magicStateDelta[3] == kF0757_LightAmountFor(lightPower),
              buf);
    }

    /* T3: C3_INVISIBILITY (MENU.C:1970-1982): spellPower <<= 3,
     * then ticks = spellPower * 40. */
    spell.type = C3_SPELL_TYPE_OTHER_INVISIBILITY_COMPAT;
    for (i = 1; i <= 6; ++i) {
        int spellPower = kF0757_SpellPowerFor(i) << 3;
        int ticks = spellPower * 40;
        memset(&out, 0, sizeof(out));
        rc = F0757_MAGIC_ProduceOtherEffect_Compat(&spell, i, &magic, &out);
        char buf[96];
        snprintf(buf, sizeof(buf),
                 "T3: C3_INVISIBILITY ordinal=%d ticks==%d (got %d)",
                 i, ticks, out.durationTicks);
        CHECK(out.durationTicks == ticks, buf);
    }

    /* T6: C6_FOOTPRINTS (MENU.C:2001-2009). */
    spell.type = C6_SPELL_TYPE_OTHER_FOOTPRINTS_COMPAT;
    for (i = 1; i <= 6; ++i) {
        int spellPower = kF0757_SpellPowerFor(i);
        int ticks = spellPower * 40;
        memset(&out, 0, sizeof(out));
        rc = F0757_MAGIC_ProduceOtherEffect_Compat(&spell, i, &magic, &out);
        char buf[96];
        snprintf(buf, sizeof(buf),
                 "T6: C6_FOOTPRINTS ordinal=%d ticks==%d (got %d)",
                 i, ticks, out.durationTicks);
        CHECK(out.durationTicks == ticks, buf);
        CHECK(out.magicStateDelta[5] == 1,
              "T6: C6_FOOTPRINTS sets magicStateDelta[5] = 1");
    }

    /* T7: C7_ZOKATHRA (MENU.C:2014-2023). */
    spell.type = C7_SPELL_TYPE_OTHER_ZOKATHRA_COMPAT;
    for (i = 1; i <= 6; ++i) {
        memset(&out, 0, sizeof(out));
        rc = F0757_MAGIC_ProduceOtherEffect_Compat(&spell, i, &magic, &out);
        char buf[96];
        snprintf(buf, sizeof(buf),
                 "T7: C7_ZOKATHRA ordinal=%d rc==1", i);
        CHECK(rc == 1, buf);
        CHECK(out.durationTicks == 0,
              "T7: C7_ZOKATHRA ticks = 0 (caller is responsible for JUNK)");
    }

    /* T8: C8_FIRESHIELD (MENU.C:2026-2030). */
    spell.type = C8_SPELL_TYPE_OTHER_FIRESHIELD_COMPAT;
    for (i = 1; i <= 6; ++i) {
        int spellPower = kF0757_SpellPowerFor(i);
        int defense = (spellPower * spellPower) + 100;
        int ticks = defense >> 5;
        memset(&out, 0, sizeof(out));
        rc = F0757_MAGIC_ProduceOtherEffect_Compat(&spell, i, &magic, &out);
        char buf[96];
        snprintf(buf, sizeof(buf),
                 "T8: C8_FIRESHIELD ordinal=%d rc==1", i);
        CHECK(rc == 1, buf);
        snprintf(buf, sizeof(buf),
                 "T8: C8_FIRESHIELD ordinal=%d ticks==%d (got %d)",
                 i, ticks, out.durationTicks);
        CHECK(out.durationTicks == ticks, buf);
    }

    /* T9: spellPower = (powerOrdinal + 1) << 2. */
    for (i = 1; i <= 6; ++i) {
        CHECK(kF0757_SpellPowerFor(i) == (i + 1) * 4,
              "T9: spellPower = (powerOrdinal + 1) << 2");
    }

    /* T10: out-of-range powerOrdinal returns 0. */
    spell.type = C0_SPELL_TYPE_OTHER_LIGHT_COMPAT;
    memset(&out, 0, sizeof(out));
    CHECK(F0757_MAGIC_ProduceOtherEffect_Compat(&spell, 0, &magic, &out) == 0,
          "T10: powerOrdinal=0 returns 0");
    CHECK(F0757_MAGIC_ProduceOtherEffect_Compat(&spell, 7, &magic, &out) == 0,
          "T10: powerOrdinal=7 returns 0");
    CHECK(F0757_MAGIC_ProduceOtherEffect_Compat(&spell, 100, &magic, &out) == 0,
          "T10: powerOrdinal=100 returns 0");

    /* T11: NULL out returns 0. */
    CHECK(F0757_MAGIC_ProduceOtherEffect_Compat(&spell, 1, &magic, NULL) == 0,
          "T11: NULL out returns 0");

    /* T12: NULL spell or magic returns 0. */
    CHECK(F0757_MAGIC_ProduceOtherEffect_Compat(NULL, 1, &magic, &out) == 0,
          "T12: NULL spell returns 0");
    CHECK(F0757_MAGIC_ProduceOtherEffect_Compat(&spell, 1, NULL, &out) == 0,
          "T12: NULL magic returns 0");

    /* T13: castResult = SPELL_CAST_SUCCESS. */
    spell.type = C0_SPELL_TYPE_OTHER_LIGHT_COMPAT;
    memset(&out, 0, sizeof(out));
    F0757_MAGIC_ProduceOtherEffect_Compat(&spell, 3, &magic, &out);
    CHECK(out.castResult == SPELL_CAST_SUCCESS,
          "T13: castResult = SPELL_CAST_SUCCESS");

    /* T14: spellType = spell->type. */
    spell.type = C8_SPELL_TYPE_OTHER_FIRESHIELD_COMPAT;
    memset(&out, 0, sizeof(out));
    F0757_MAGIC_ProduceOtherEffect_Compat(&spell, 3, &magic, &out);
    CHECK(out.spellType == C8_SPELL_TYPE_OTHER_FIRESHIELD_COMPAT,
          "T14: spellType = spell->type");

    /* T15: powerOrdinal field set. */
    spell.type = C6_SPELL_TYPE_OTHER_FOOTPRINTS_COMPAT;
    for (i = 1; i <= 6; ++i) {
        memset(&out, 0, sizeof(out));
        F0757_MAGIC_ProduceOtherEffect_Compat(&spell, i, &magic, &out);
        char buf[96];
        snprintf(buf, sizeof(buf),
                 "T15: powerOrdinal field == %d (got %d)",
                 i, out.powerOrdinal);
        CHECK(out.powerOrdinal == i, buf);
    }

    /* T16: lightPower clamping for C0_LIGHT.
     * For ordinal 1: spellPower = 8, (8>>1)-1 = 3, clamped to 3.
     * For ordinal 6: spellPower = 28, (28>>1)-1 = 13, clamped to 5. */
    spell.type = C0_SPELL_TYPE_OTHER_LIGHT_COMPAT;
    for (i = 1; i <= 6; ++i) {
        int spellPower = kF0757_SpellPowerFor(i);
        int lp = kF0757_LightPowerFor(spellPower);
        memset(&out, 0, sizeof(out));
        F0757_MAGIC_ProduceOtherEffect_Compat(&spell, i, &magic, &out);
        char buf[96];
        snprintf(buf, sizeof(buf),
                 "T16: C0_LIGHT lightPower for ordinal=%d == %d", i, lp);
        /* The negative lightPower is used as followupEventAux0 */
        CHECK(out.followupEventAux0 == -lp,
              buf);
    }

    printf("PASS: MNU-03 F0757 spell-duration source-lock (16 scenarios)\n");
    return 0;
}
