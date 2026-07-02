/*
 * test_dm1_v1_mnu03_f0760_apply_state_delta_pc34_compat.c
 *
 * Source-locked to ReDMCSB MENU.C:1969, 1086 ("> 50 -> delta >>= 2"
 * rule for shield/fire/party shield defense) and the per-event
 * kind count bumps in MENU.C:1945-2030.
 *
 * F0760_MAGIC_ApplyStateDelta_Compat is the new-path
 * replacement that mutates a MagicState_Compat from a
 * SpellEffect_Compat.  Pins the contract:
 *  T1  NULL effect returns 0
 *  T2  NULL inOutMagic returns 0
 *  T3  All magicStateDelta[i] = 0: no change to MagicState
 *  T4  Shield delta: spellShieldDefense += delta[0]
 *  T5  Fire delta: fireShieldDefense += delta[1]
 *  T6  Party delta: partyShieldDefense += delta[2]
 *  T7  Light delta: magicalLightAmount += delta[3]
 *  T8  Freeze delta: freezeLifeTicks += delta[4]
 *  T9  "> 50 -> delta >>= 2" rule: shield
 *  T10 "> 50 -> delta >>= 2" rule: fire
 *  T11 "> 50 -> delta >>= 2" rule: party
 *  T12 Thieves Eye count bump (C2): event73CountThievesEye
 *  T13 Invisibility count bump (C3): event71CountInvisibility
 *  T14 Party Shield count bump (C4): event74CountPartyShield
 *  T15 Footprints count bump (C6): event79CountFootprints +
 *      magicFootprintsActive=1, first/last scent window from ScentCount
 *  T16 Returns 1 on success
 *  T17 Out-of-range spellType: no count bump (defensive)
 *  T18 Multiple delta slots at once
 *
 * Source-locked to ReDMCSB MENU.C:1945-2030, especially F0412
 * lines 1979-1986 for footprints first/last scent index handling.
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

int main(void) {
    struct SpellEffect_Compat effect;
    struct MagicState_Compat magic;
    int rc;

    /* T1: NULL effect. */
    memset(&magic, 0, sizeof(magic));
    CHECK(F0760_MAGIC_ApplyStateDelta_Compat(NULL, &magic) == 0,
          "T1: NULL effect returns 0");

    /* T2: NULL inOutMagic. */
    memset(&effect, 0, sizeof(effect));
    CHECK(F0760_MAGIC_ApplyStateDelta_Compat(&effect, NULL) == 0,
          "T2: NULL inOutMagic returns 0");

    /* T3: All zero deltas. */
    memset(&effect, 0, sizeof(effect));
    memset(&magic, 0, sizeof(magic));
    magic.spellShieldDefense = 10;
    magic.fireShieldDefense = 10;
    magic.partyShieldDefense = 10;
    magic.magicalLightAmount = 5;
    magic.freezeLifeTicks = 3;
    F0760_MAGIC_ApplyStateDelta_Compat(&effect, &magic);
    CHECK(magic.spellShieldDefense == 10, "T3a: zero deltas, shield unchanged");
    CHECK(magic.fireShieldDefense == 10, "T3b: zero deltas, fire unchanged");
    CHECK(magic.partyShieldDefense == 10, "T3c: zero deltas, party unchanged");
    CHECK(magic.magicalLightAmount == 5, "T3d: zero deltas, light unchanged");
    CHECK(magic.freezeLifeTicks == 3, "T3e: zero deltas, freeze unchanged");

    /* T4: Shield delta. */
    memset(&effect, 0, sizeof(effect));
    memset(&magic, 0, sizeof(magic));
    effect.magicStateDelta[0] = 25;
    F0760_MAGIC_ApplyStateDelta_Compat(&effect, &magic);
    CHECK(magic.spellShieldDefense == 25, "T4: shield = 0 + 25");

    /* T5: Fire delta. */
    memset(&effect, 0, sizeof(effect));
    memset(&magic, 0, sizeof(magic));
    effect.magicStateDelta[1] = 15;
    F0760_MAGIC_ApplyStateDelta_Compat(&effect, &magic);
    CHECK(magic.fireShieldDefense == 15, "T5: fire = 0 + 15");

    /* T6: Party delta. */
    memset(&effect, 0, sizeof(effect));
    memset(&magic, 0, sizeof(magic));
    effect.magicStateDelta[2] = 20;
    F0760_MAGIC_ApplyStateDelta_Compat(&effect, &magic);
    CHECK(magic.partyShieldDefense == 20, "T6: party = 0 + 20");

    /* T7: Light delta. */
    memset(&effect, 0, sizeof(effect));
    memset(&magic, 0, sizeof(magic));
    effect.magicStateDelta[3] = 50;
    F0760_MAGIC_ApplyStateDelta_Compat(&effect, &magic);
    CHECK(magic.magicalLightAmount == 50, "T7: light = 0 + 50");

    /* T8: Freeze delta. */
    memset(&effect, 0, sizeof(effect));
    memset(&magic, 0, sizeof(magic));
    effect.magicStateDelta[4] = 10;
    F0760_MAGIC_ApplyStateDelta_Compat(&effect, &magic);
    CHECK(magic.freezeLifeTicks == 10, "T8: freeze = 0 + 10");

    /* T9: "> 50 -> delta >>= 2" rule on shield. */
    memset(&effect, 0, sizeof(effect));
    memset(&magic, 0, sizeof(magic));
    magic.spellShieldDefense = 51; /* > 50 */
    effect.magicStateDelta[0] = 16; /* 16 >> 2 = 4 */
    F0760_MAGIC_ApplyStateDelta_Compat(&effect, &magic);
    CHECK(magic.spellShieldDefense == 55, "T9: shield 51 + (16>>2) = 55");

    /* T10: "> 50 -> delta >>= 2" on fire. */
    memset(&effect, 0, sizeof(effect));
    memset(&magic, 0, sizeof(magic));
    magic.fireShieldDefense = 100;
    effect.magicStateDelta[1] = 8; /* 8 >> 2 = 2 */
    F0760_MAGIC_ApplyStateDelta_Compat(&effect, &magic);
    CHECK(magic.fireShieldDefense == 102, "T10: fire 100 + (8>>2) = 102");

    /* T11: "> 50 -> delta >>= 2" on party. */
    memset(&effect, 0, sizeof(effect));
    memset(&magic, 0, sizeof(magic));
    magic.partyShieldDefense = 200;
    effect.magicStateDelta[2] = 12; /* 12 >> 2 = 3 */
    F0760_MAGIC_ApplyStateDelta_Compat(&effect, &magic);
    CHECK(magic.partyShieldDefense == 203, "T11: party 200 + (12>>2) = 203");

    /* T12: Thieves Eye count bump. */
    memset(&effect, 0, sizeof(effect));
    memset(&magic, 0, sizeof(magic));
    effect.spellType = C2_SPELL_TYPE_OTHER_THIEVES_EYE_COMPAT;
    effect.magicStateDelta[5] = 1;
    F0760_MAGIC_ApplyStateDelta_Compat(&effect, &magic);
    CHECK(magic.event73CountThievesEye == 1, "T12: thieves eye count = 1");

    /* T13: Invisibility count bump. */
    memset(&effect, 0, sizeof(effect));
    memset(&magic, 0, sizeof(magic));
    effect.spellType = C3_SPELL_TYPE_OTHER_INVISIBILITY_COMPAT;
    effect.magicStateDelta[5] = 1;
    F0760_MAGIC_ApplyStateDelta_Compat(&effect, &magic);
    CHECK(magic.event71CountInvisibility == 1, "T13: invisibility count = 1");

    /* T14: Party Shield count bump. */
    memset(&effect, 0, sizeof(effect));
    memset(&magic, 0, sizeof(magic));
    effect.spellType = C4_SPELL_TYPE_OTHER_PARTY_SHIELD_COMPAT;
    effect.magicStateDelta[5] = 1;
    F0760_MAGIC_ApplyStateDelta_Compat(&effect, &magic);
    CHECK(magic.event74CountPartyShield == 1, "T14: party shield count = 1");

    /* T15: Footprints count bump + magicFootprintsActive=1. */
    memset(&effect, 0, sizeof(effect));
    memset(&magic, 0, sizeof(magic));
    effect.spellType = C6_SPELL_TYPE_OTHER_FOOTPRINTS_COMPAT;
    effect.powerOrdinal = 2;
    effect.magicStateDelta[5] = 1;
    magic.scentCount = 9;
    F0760_MAGIC_ApplyStateDelta_Compat(&effect, &magic);
    CHECK(magic.event79CountFootprints == 1, "T15a: footprints count = 1");
    CHECK(magic.magicFootprintsActive == 1, "T15b: magicFootprintsActive = 1");
    CHECK(magic.firstScentIndex == 9, "T15c: weak footprints firstScentIndex = ScentCount");
    CHECK(magic.lastScentIndex == 9, "T15d: weak footprints lastScentIndex = first");

    memset(&effect, 0, sizeof(effect));
    memset(&magic, 0, sizeof(magic));
    effect.spellType = C6_SPELL_TYPE_OTHER_FOOTPRINTS_COMPAT;
    effect.powerOrdinal = 3;
    effect.magicStateDelta[5] = 1;
    magic.scentCount = 12;
    F0760_MAGIC_ApplyStateDelta_Compat(&effect, &magic);
    CHECK(magic.firstScentIndex == 12, "T15e: strong footprints firstScentIndex = ScentCount");
    CHECK(magic.lastScentIndex == 0, "T15f: strong footprints lastScentIndex = 0");

    /* T16: Returns 1 on success. */
    memset(&effect, 0, sizeof(effect));
    memset(&magic, 0, sizeof(magic));
    rc = F0760_MAGIC_ApplyStateDelta_Compat(&effect, &magic);
    CHECK(rc == 1, "T16: returns 1 on success");

    /* T17: Out-of-range spellType (no count bump). */
    memset(&effect, 0, sizeof(effect));
    memset(&magic, 0, sizeof(magic));
    effect.spellType = 0xFE; /* out of range */
    effect.magicStateDelta[5] = 1;
    F0760_MAGIC_ApplyStateDelta_Compat(&effect, &magic);
    CHECK(magic.event73CountThievesEye == 0, "T17a: thieves eye unchanged");
    CHECK(magic.event71CountInvisibility == 0, "T17b: invisibility unchanged");
    CHECK(magic.event74CountPartyShield == 0, "T17c: party shield unchanged");
    CHECK(magic.event79CountFootprints == 0, "T17d: footprints unchanged");

    /* T18: Multiple deltas at once. */
    memset(&effect, 0, sizeof(effect));
    memset(&magic, 0, sizeof(magic));
    effect.magicStateDelta[0] = 10;
    effect.magicStateDelta[1] = 20;
    effect.magicStateDelta[2] = 30;
    effect.magicStateDelta[3] = 5;
    effect.magicStateDelta[4] = 2;
    F0760_MAGIC_ApplyStateDelta_Compat(&effect, &magic);
    CHECK(magic.spellShieldDefense == 10, "T18a: shield=10");
    CHECK(magic.fireShieldDefense == 20, "T18b: fire=20");
    CHECK(magic.partyShieldDefense == 30, "T18c: party=30");
    CHECK(magic.magicalLightAmount == 5, "T18d: light=5");
    CHECK(magic.freezeLifeTicks == 2, "T18e: freeze=2");

    printf("PASS: MNU-03 F0760 magic-state-delta pin (18 scenarios)\n");
    return 0;
}
