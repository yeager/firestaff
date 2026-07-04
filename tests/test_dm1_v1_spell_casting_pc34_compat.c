/**
 * test_dm1_v1_spell_casting_pc34_compat.c — CTest gate for DM1 V1 spell casting
 *
 * Verifies source-locked spell system against ReDMCSB constants:
 * - Symbol encoding and step cycling
 * - Mana cost calculation per SYMBOL.C F0399
 * - Spell lookup per MENU.C F0409
 * - Spell cast result per MENU.C F0412
 * - Projectile energy per CHAMPION.C F0327
 * - Experience per MENU.C F0412
 */
#include "dm1_v1_spell_casting_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static DM1_ChampionSpellStats makeStats(int mana, int maxMana, int hp, int wisdom) {
    DM1_ChampionSpellStats st;
    memset(&st, 0, sizeof(st));
    st.currentMana = (int16_t)mana;
    st.maximumMana = (int16_t)maxMana;
    st.currentHealth = (int16_t)hp;
    st.wisdom = (uint8_t)wisdom;
    return st;
}

/* ── Test 1: Symbol encoding ─────────────────────────────────────── */
static void test_symbol_encoding(void) {
    printf("  [1] Symbol encoding...\n");

    /* Step 0, index 0 → char 96 (SYMBOL.C:36: 96 + 0*6 + 0) */
    assert(dm1_encodeSymbol(0, 0) == 96);
    assert(dm1_encodeSymbol(0, 5) == 101);
    assert(dm1_encodeSymbol(1, 0) == 102);
    assert(dm1_encodeSymbol(2, 3) == 111);  /* Ir */
    assert(dm1_encodeSymbol(3, 5) == 119);  /* Sar */

    /* Decode back */
    assert(dm1_symbolCharStep(96) == 0);
    assert(dm1_symbolCharIndex(96) == 0);
    assert(dm1_symbolCharStep(101) == 0);
    assert(dm1_symbolCharIndex(101) == 5);
    assert(dm1_symbolCharStep(111) == 2);
    assert(dm1_symbolCharIndex(111) == 3);

    printf("    PASS\n");
}

/* ── Test 2: Symbol names (verified from spell table hex) ────────── */
static void test_symbol_names(void) {
    printf("  [2] Symbol names...\n");

    /* Power (step 0) */
    assert(strcmp(dm1_spell_symbolName(96), "Lo") == 0);
    assert(strcmp(dm1_spell_symbolName(97), "Um") == 0);
    assert(strcmp(dm1_spell_symbolName(101), "Mon") == 0);

    /* Element (step 1): Ya=102, Vi=103, Oh=104, Ful=105, Des=106, Zo=107 */
    assert(strcmp(dm1_spell_symbolName(102), "Ya") == 0);    /* 0x66 */
    assert(strcmp(dm1_spell_symbolName(103), "Vi") == 0);    /* 0x67 */
    assert(strcmp(dm1_spell_symbolName(104), "Oh") == 0);    /* 0x68 */
    assert(strcmp(dm1_spell_symbolName(105), "Ful") == 0);   /* 0x69 */
    assert(strcmp(dm1_spell_symbolName(106), "Des") == 0);   /* 0x6A */
    assert(strcmp(dm1_spell_symbolName(107), "Zo") == 0);    /* 0x6B */

    /* Class (step 2): Ven=108, Ew=109, Kath=110, Ir=111, Bro=112, Gor=113 */
    assert(strcmp(dm1_spell_symbolName(108), "Ven") == 0);   /* 0x6C */
    assert(strcmp(dm1_spell_symbolName(109), "Ew") == 0);    /* 0x6D */
    assert(strcmp(dm1_spell_symbolName(110), "Kath") == 0);  /* 0x6E */
    assert(strcmp(dm1_spell_symbolName(111), "Ir") == 0);    /* 0x6F */
    assert(strcmp(dm1_spell_symbolName(112), "Bro") == 0);   /* 0x70 */
    assert(strcmp(dm1_spell_symbolName(113), "Gor") == 0);   /* 0x71 */

    /* Alignment (step 3): Ku=114, Ros=115, Dain=116, Neta=117, Ra=118, Sar=119 */
    assert(strcmp(dm1_spell_symbolName(114), "Ku") == 0);    /* 0x72 */
    assert(strcmp(dm1_spell_symbolName(115), "Ros") == 0);   /* 0x73 */
    assert(strcmp(dm1_spell_symbolName(116), "Dain") == 0);  /* 0x74 */
    assert(strcmp(dm1_spell_symbolName(117), "Neta") == 0);  /* 0x75 */
    assert(strcmp(dm1_spell_symbolName(118), "Ra") == 0);    /* 0x76 */
    assert(strcmp(dm1_spell_symbolName(119), "Sar") == 0);   /* 0x77 */

    printf("    PASS\n");
}

/* ── Test 3: Init state ──────────────────────────────────────────── */
static void test_init(void) {
    printf("  [3] Init state...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);

    assert(s.magicCasterIndex == -1);
    for (int i = 0; i < 4; i++) {
        assert(s.input[i].symbolStep == 0);
        assert(s.input[i].symbols[0] == '\0');
    }

    printf("    PASS\n");
}

/* ── Test 4: Add symbol — mana cost and step cycling ─────────────── */
static void test_add_symbol(void) {
    printf("  [4] Add symbol — mana cost & step cycling...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(100, 100, 50, 40);

    /* Add power symbol "On" (step 0, index 2).
     * BaseCost = G0485[0][2] = 3 (MENU.C:45). Step 0, no multiplier. Cost = 3. */
    int ok __attribute__((unused)) = dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_ON);
    assert(ok == 1);
    assert(stats.currentMana == 97);
    assert(s.input[0].symbolStep == 1);
    assert(s.input[0].symbols[0] == dm1_encodeSymbol(0, 2));  /* char 98 */
    assert(s.input[0].symbols[1] == '\0');

    /* Add element "Ful" (step 1, index 3 = DM1_ELEM_FUL).
     * BaseCost = G0485[1][3] = 5 (MENU.C:46)
     * Step > 0: cost = (5 * G0486[symbols[0]-96]) >> 3
     *   symbols[0] = 98, 98 - 96 = 2
     *   G0486[2] = 16 (MENU.C:49)
     *   cost = (5 * 16) >> 3 = 80 >> 3 = 10 */
    ok = dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_FUL);
    assert(ok == 1);
    assert(stats.currentMana == 87);  /* 97 - 10 */
    assert(s.input[0].symbolStep == 2);
    assert(s.input[0].symbols[1] == dm1_encodeSymbol(1, 3));  /* char 105 = Ful */

    /* Step cycles 0→1→2→3→0 (wrap at 4) */
    ok = dm1_spell_addSymbol(&s, 0, &stats, DM1_CLASS_IR);  /* step 2, idx 3 */
    assert(ok == 1);
    assert(s.input[0].symbolStep == 3);

    ok = dm1_spell_addSymbol(&s, 0, &stats, DM1_ALIGN_RA);  /* step 3, idx 4 */
    assert(ok == 1);
    assert(s.input[0].symbolStep == 0);  /* Wrapped! */
    assert(s.input[0].symbols[3] == dm1_encodeSymbol(3, 4));  /* char 118 = Ra */
    assert(s.input[0].symbols[4] == '\0');

    printf("    PASS\n");
}

/* ── Test 5: Insufficient mana preserves selected rune chain ─────── */
static void test_insufficient_mana(void) {
    printf("  [5] Insufficient mana preserves selected rune chain...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(14, 100, 50, 40);
    int ok __attribute__((unused)) = 0;

    /* ReDMCSB SYMBOL.C F0399: the mana gate sits before the rune write,
     * so an insufficient add must leave the active rune chain untouched. */
    ok = dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_ON);
    assert(ok == 1);
    ok = dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_YA);
    assert(ok == 1);
    assert(stats.currentMana == 7);
    assert(s.input[0].symbolStep == 2);
    assert(s.input[0].symbols[0] == dm1_encodeSymbol(0, 2));
    assert(s.input[0].symbols[1] == dm1_encodeSymbol(1, 0));
    assert(s.input[0].symbols[2] == '\0');

    /* Third rune costs 12 at step 2, but only 7 mana remain. */
    ok = dm1_spell_addSymbol(&s, 0, &stats, DM1_CLASS_KATH);
    assert(ok == 0);
    assert(s.input[0].symbolStep == 2);
    assert(s.input[0].symbols[0] == dm1_encodeSymbol(0, 2));
    assert(s.input[0].symbols[1] == dm1_encodeSymbol(1, 0));
    assert(s.input[0].symbols[2] == '\0');
    assert(stats.currentMana == 7);
    assert(stats.maximumMana == 100);
    assert(stats.currentHealth == 50);
    assert(stats.wisdom == 40);

    printf("    PASS\n");
}

static void test_insufficient_mana_preserves_partial_rune_chain(void) {
    printf("  [6] Insufficient mana preserves partial rune chain...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    s.magicCasterIndex = 0;
    DM1_ChampionSpellStats stats = makeStats(200, 222, 77, 44);
    stats.skillLevels[DM1_SKILL_FIRE] = 9;

    int ok __attribute__((unused)) = dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_ON);
    assert(ok == 1);
    ok = dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_FUL);
    assert(ok == 1);

    DM1_ChampionSpellInput beforeInput = s.input[0];
    DM1_ChampionSpellStats beforeStats = stats;
    int beforeCaster = s.magicCasterIndex;

    /* ReDMCSB SYMBOL.C F0399 lines 18-30 computes cost, then only mutates
     * CurrentMana/Symbols/SymbolStep inside the line 26 mana-success branch. */
    assert(dm1_spell_symbolManaCost(&s, 0, DM1_CLASS_IR) == 14);
    stats.currentMana = 13;
    beforeStats.currentMana = 13;

    ok = dm1_spell_addSymbol(&s, 0, &stats, DM1_CLASS_IR);
    assert(ok == 0);
    assert(s.magicCasterIndex == beforeCaster);
    assert(memcmp(&s.input[0], &beforeInput, sizeof(beforeInput)) == 0);
    assert(memcmp(&stats, &beforeStats, sizeof(beforeStats)) == 0);

    dm1_spell_init(&s);
    stats = makeStats(12, 100, 50, 40);
    stats.skillLevels[DM1_SKILL_FIRE] = 7;
    stats.skillLevels[DM1_SKILL_WIZARD] = 3;
    s.magicCasterIndex = 0;
    s.input[1].symbolStep = 1;
    s.input[1].symbols[0] = dm1_encodeSymbol(0, DM1_POWER_MON);
    s.input[1].symbols[1] = '\0';

    assert(dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_LO) == 1);
    assert(dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_FUL) == 1);
    assert(stats.currentMana == 6);
    assert(s.input[0].symbolStep == 2);
    assert(s.input[0].symbols[0] == dm1_encodeSymbol(0, DM1_POWER_LO));
    assert(s.input[0].symbols[1] == dm1_encodeSymbol(1, DM1_ELEM_FUL));
    assert(s.input[0].symbols[2] == '\0');

    {
        DM1_ChampionSpellInput beforeInput = s.input[0];
        DM1_ChampionSpellInput beforeOtherInput = s.input[1];
        DM1_ChampionSpellStats beforeStats = stats;
        int beforeCaster = s.magicCasterIndex;
        int expectedClassCost = dm1_spell_symbolManaCost(&s, 0, DM1_CLASS_IR);
        const DM1_Spell* beforeSpell = dm1_spell_lookup(&s, 0);

        assert(expectedClassCost == 7);
        assert(beforeSpell == &dm1_spells[7]);  /* Lo Ful / Torch */
        /* ReDMCSB SYMBOL.C F0399 lines 20-39 mutates mana, Symbols[], and
         * SymbolStep only inside `if (ManaCost <= CurrentMana)`. */
        assert(dm1_spell_addSymbol(&s, 0, &stats, DM1_CLASS_IR) == 0);
        assert(memcmp(&s.input[0], &beforeInput, sizeof(beforeInput)) == 0);
        assert(memcmp(&s.input[1], &beforeOtherInput, sizeof(beforeOtherInput)) == 0);
        assert(memcmp(&stats, &beforeStats, sizeof(beforeStats)) == 0);
        assert(s.magicCasterIndex == beforeCaster);
        assert(dm1_spell_lookup(&s, 0) == beforeSpell);
    }

    printf("    PASS\n");
}

/* ── Test 6: Delete symbol (recant) ──────────────────────────────── */
static void test_delete_symbol(void) {
    printf("  [7] Delete symbol (recant)...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(100, 100, 50, 40);

    dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_ON);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_FUL);
    int manaBefore __attribute__((unused)) = stats.currentMana;

    /* Recant: step 2→1, Symbols[1] = '\0' */
    dm1_spell_deleteSymbol(&s, 0);
    assert(s.input[0].symbolStep == 1);
    assert(s.input[0].symbols[1] == '\0');
    assert(s.input[0].symbols[0] != '\0');
    assert(stats.currentMana == manaBefore);  /* No refund */

    /* Recant: step 1→0, Symbols[0] = '\0' */
    dm1_spell_deleteSymbol(&s, 0);
    assert(s.input[0].symbolStep == 0);
    assert(s.input[0].symbols[0] == '\0');

    /* Empty recant: no-op */
    dm1_spell_deleteSymbol(&s, 0);
    assert(s.input[0].symbolStep == 0);
    assert(s.input[0].symbols[0] == '\0');

    printf("    PASS\n");
}

/* ── Test 7: Spell lookup — Fireball ─────────────────────────────── */
static void test_spell_lookup_fireball(void) {
    printf("  [8] Spell lookup — Fireball...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(200, 200, 50, 40);

    /* Fireball = Ful Ir = 0x00696F00
     * Power "On" (step 0 idx 2) + Ful (step 1 idx 3) + Ir (step 2 idx 3) */
    dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_ON);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_FUL);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_CLASS_IR);

    const DM1_Spell* spell __attribute__((unused)) = dm1_spell_lookup(&s, 0);
    assert(spell != NULL);
    assert(DM1_SPELL_KIND(spell) == DM1_SPELL_KIND_PROJECTILE);
    assert(spell->skillIndex == DM1_SKILL_FIRE);
    assert((int)(spell - dm1_spells) == 8);
    assert(strcmp(dm1_spell_name(8), "FIREBALL") == 0);

    printf("    PASS\n");
}

/* ── Test 8: Spell lookup — Open Door ────────────────────────────── */
static void test_spell_lookup_open_door(void) {
    printf("  [9] Spell lookup — Open Door...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(200, 200, 50, 40);

    /* Open Door = Zo = 0x006B0000
     * Lo (power) + Zo (step 1 idx 5) */
    dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_LO);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_ZO);

    const DM1_Spell* spell __attribute__((unused)) = dm1_spell_lookup(&s, 0);
    assert(spell != NULL);
    assert((int)(spell - dm1_spells) == 14);
    assert(DM1_SPELL_KIND(spell) == DM1_SPELL_KIND_PROJECTILE);
    assert(DM1_SPELL_TYPE(spell) == DM1_SPELL_TYPE_PROJ_OPEN_DOOR);

    printf("    PASS\n");
}

/* ── Test 9: Spell lookup — meaningless sequence ─────────────────── */
static void test_spell_lookup_meaningless(void) {
    printf("  [10] Spell lookup — meaningless...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(200, 200, 50, 40);

    dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_MON);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_YA);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_CLASS_VEN);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_ALIGN_KU);

    assert(dm1_spell_lookup(&s, 0) == NULL);

    printf("    PASS\n");
}

/* ── Test 10: Mana cost calculation ──────────────────────────────── */
static void test_mana_cost(void) {
    printf("  [11] Mana cost calculation...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(200, 200, 50, 40);

    /* Step 0: cost = base[0][idx] */
    assert(dm1_spell_symbolManaCost(&s, 0, 0) == 1);  /* Lo */
    assert(dm1_spell_symbolManaCost(&s, 0, 3) == 4);  /* Ee */
    assert(dm1_spell_symbolManaCost(&s, 0, 5) == 6);  /* Mon */

    /* Add power "Ee" (idx 3) → char 99, 99-96=3, multiplier[3]=20 */
    dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_EE);

    /* Step 1: cost = (base[1][idx] * 20) >> 3 */
    assert(dm1_spell_symbolManaCost(&s, 0, 0) == 5);   /* (2*20)>>3 = 5 */
    assert(dm1_spell_symbolManaCost(&s, 0, 2) == 10);  /* (4*20)>>3 = 10 */

    printf("    PASS\n");
}

/* ── Test 11: Malformed rune-step boundary gate ─────────────────── */
static void test_malformed_symbol_step_rejected_without_mutation(void) {
    printf("  [12] Malformed SymbolStep is rejected without mutation...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(77, 100, 50, 40);
    DM1_ChampionSpellInput beforeInput;
    DM1_ChampionSpellStats beforeStats;

    /* ReDMCSB SYMBOL.C F0399:17-32 indexes G0485[SymbolStep][SymbolIndex]
     * under the invariant that SymbolStep is kept in 0..3 by F0399/F0400.
     * The standalone compatibility API must fail deterministically if a
     * caller hands it corrupted state instead of indexing outside G0485. */
    s.input[0].symbolStep = 4;
    s.input[0].symbols[0] = dm1_encodeSymbol(0, DM1_POWER_ON);
    s.input[0].symbols[1] = '\0';
    beforeInput = s.input[0];
    beforeStats = stats;

    assert(dm1_spell_symbolManaCost(&s, 0, DM1_ELEM_FUL) == 0);
    assert(dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_FUL) == 0);
    assert(memcmp(&s.input[0], &beforeInput, sizeof(beforeInput)) == 0);
    assert(memcmp(&stats, &beforeStats, sizeof(beforeStats)) == 0);

    /* Step > 0 also depends on Symbols[0] being a power rune byte 96..101
     * before indexing G0486 (SYMBOL.C F0399:19-24). Reject a stale/null
     * power slot without touching mana or the partial rune buffer. */
    s.input[0].symbolStep = 1;
    s.input[0].symbols[0] = '\0';
    beforeInput = s.input[0];
    beforeStats = stats;

    assert(dm1_spell_symbolManaCost(&s, 0, DM1_ELEM_YA) == 0);
    assert(dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_YA) == 0);
    assert(memcmp(&s.input[0], &beforeInput, sizeof(beforeInput)) == 0);
    assert(memcmp(&stats, &beforeStats, sizeof(beforeStats)) == 0);

    printf("    PASS\n");
}

static void test_malformed_power_rune_cannot_dispatch_spell(void) {
    printf("  [12b] Malformed power rune cannot dispatch spell...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(77, 100, 50, 40);
    int failure = -1;
    int powerOrdinal = -1;
    const DM1_Spell* outSpell = NULL;

    stats.skillLevels[DM1_SKILL_FIRE] = 10;

    /* ReDMCSB SYMBOL.C F0399:29 can only write power bytes 96..101.
     * MENU.C F0409:1704-1706 ignores the power byte for ordinary DM1
     * spell-table matches, and MENU.C F0412 later uses Symbols[0] - '_'
     * as the power ordinal.  A standalone caller with corrupted state must
     * not turn an invalid power byte plus a valid spell tail into a cast. */
    s.input[0].symbolStep = 3;
    s.input[0].symbols[0] = '!';
    s.input[0].symbols[1] = dm1_encodeSymbol(1, DM1_ELEM_FUL);
    s.input[0].symbols[2] = dm1_encodeSymbol(2, DM1_CLASS_IR);
    s.input[0].symbols[3] = '\0';

    assert(dm1_spell_lookup(&s, 0) == NULL);
    assert(dm1_spell_cast(&s, 0, &stats, 0x0000,
                          &outSpell, &powerOrdinal, &failure) ==
           DM1_SPELL_CAST_FAILURE);
    assert(outSpell == NULL);
    assert(powerOrdinal == -1);
    assert(failure == DM1_FAILURE_MEANINGLESS_SPELL);
    assert(s.input[0].symbols[0] == '\0');
    assert(s.input[0].symbolStep == 0);
    assert(stats.currentMana == 77);

    printf("    PASS\n");
}

/* ── Test 12: Spell cast — success ───────────────────────────────── */
static void test_spell_cast_success(void) {
    printf("  [13] Spell cast — success...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(200, 200, 50, 60);
    stats.skillLevels[DM1_SKILL_FIRE] = 10;

    /* Fireball: On Ful Ir */
    dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_ON);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_FUL);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_CLASS_IR);

    const DM1_Spell* outSpell = NULL;
    int powerOrd = 0, failure = -1;

    int result __attribute__((unused)) = dm1_spell_cast(&s, 0, &stats, 0x1234, &outSpell, &powerOrd, &failure);
    assert(result == DM1_SPELL_CAST_SUCCESS);
    assert(outSpell != NULL);
    assert(powerOrd == 3);  /* On = char 98, ordinal = 98 - 95 = 3 */
    assert(DM1_SPELL_KIND(outSpell) == DM1_SPELL_KIND_PROJECTILE);

    /* Symbols cleared after success */
    assert(s.input[0].symbols[0] == '\0');
    assert(s.input[0].symbolStep == 0);

    printf("    PASS\n");
}

/* ── Test 13: Spell cast — dead champion ─────────────────────────── */
static void test_spell_cast_dead(void) {
    printf("  [14] Spell cast — dead champion...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(200, 200, 0, 60);
    stats.skillLevels[DM1_SKILL_FIRE] = 10;

    dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_ON);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_FUL);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_CLASS_IR);

    assert(dm1_spell_cast(&s, 0, &stats, 0, NULL, NULL, NULL) == DM1_SPELL_CAST_FAILURE);
    assert(s.input[0].symbols[0] == 0);
    assert(s.input[0].symbolStep == 0);

    printf("    PASS\n");
}

/* ── Test 14: Spell cast — meaningless ───────────────────────────── */
static void test_spell_cast_meaningless(void) {
    printf("  [15] Spell cast — meaningless...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(200, 200, 50, 60);

    dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_MON);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_YA);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_CLASS_VEN);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_ALIGN_KU);

    int failure __attribute__((unused)) = -1;
    assert(dm1_spell_cast(&s, 0, &stats, 0, NULL, NULL, &failure) == DM1_SPELL_CAST_FAILURE);
    assert(failure == DM1_FAILURE_MEANINGLESS_SPELL);
    assert(s.input[0].symbols[0] == 0);
    assert(s.input[0].symbolStep == 0);

    printf("    PASS\n");
}

/* ── Test 15: Spell cast — potion needs flask ────────────────────── */
static void test_spell_cast_potion(void) {
    printf("  [16] Spell cast — potion needs flask...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(200, 200, 50, 60);
    stats.skillLevels[DM1_SKILL_HEAL] = 10;

    /* Health Potion = Vi (step1 idx1 = char 103 = 0x67)
     * Spell: { 0x00670000, 1, HEAL, 0x80E1 } kind=1=potion */
    dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_LO);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_VI);

    const DM1_Spell* outSpell = NULL;
    int failure = -1;
    int result __attribute__((unused)) = dm1_spell_cast(&s, 0, &stats, 0, &outSpell, NULL, &failure);
    assert(result == DM1_SPELL_CAST_FAILURE_NEEDS_FLASK);
    assert(failure == DM1_FAILURE_NEEDS_FLASK_IN_HAND);
    assert(outSpell != NULL);
    assert(DM1_SPELL_KIND(outSpell) == DM1_SPELL_KIND_POTION);
    assert(s.input[0].symbols[0] != 0);
    assert(s.input[0].symbolStep == 2);

    printf("    PASS\n");
}

static void test_spell_cast_potion_flask_inventory_mutation(void) {
    printf("  [17] Spell cast — potion flask inventory mutation...\n");

    DM1_SpellCastingState s;
    DM1_ChampionSpellStats stats;
    DM1_SpellPotionInventory inventory;
    DM1_SpellPotionCastResult result;
    DM1_SpellPotionInventory beforeInventory;
    DM1_ChampionSpellInput beforeInput;
    DM1_ChampionSpellStats beforeStats;

    dm1_spell_init(&s);
    stats = makeStats(200, 200, 50, 60);
    stats.skillLevels[DM1_SKILL_HEAL] = 10;
    memset(&inventory, 0, sizeof(inventory));
    inventory.slots[DM1_SPELL_SLOT_READY_HAND] = DM1_SPELL_THING_NONE_PC34;
    inventory.slots[DM1_SPELL_SLOT_ACTION_HAND] = DM1_SPELL_THING_NONE_PC34;
    inventory.load = 42;
    inventory.potionCount = 1;
    inventory.potions[0].thing = 0x2011u;
    inventory.potions[0].iconIndex = DM1_SPELL_ICON_EMPTY_FLASK_PC34;
    inventory.potions[0].type = DM1_SPELL_POTION_EMPTY_FLASK_PC34;
    inventory.potions[0].power = 7;
    inventory.potions[0].weight = 2;

    /* ReDMCSB MENU.C:71 defines Vi as health-potion type 14; F0411 only
     * finds C195 empty flasks in C01 action or C00 ready hand. */
    assert(dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_LO) == 1);
    assert(dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_VI) == 1);

    beforeInventory = inventory;
    beforeInput = s.input[0];
    beforeStats = stats;
    assert(dm1_spell_castPotionWithInventory(&s, 0, &stats, 0x000B,
                                             &inventory, &result) ==
           DM1_SPELL_CAST_FAILURE_NEEDS_FLASK);
    assert(result.castResult == DM1_SPELL_CAST_FAILURE_NEEDS_FLASK);
    assert(result.failureType == DM1_FAILURE_NEEDS_FLASK_IN_HAND);
    assert(result.spellIndex == 19);
    assert(result.powerOrdinal == 1);
    assert(result.flaskSlotIndex == -1);
    assert(result.symbolsCleared == 0);
    assert(memcmp(&inventory, &beforeInventory, sizeof(inventory)) == 0);
    assert(memcmp(&s.input[0], &beforeInput, sizeof(beforeInput)) == 0);
    assert(memcmp(&stats, &beforeStats, sizeof(beforeStats)) == 0);

    inventory.slots[DM1_SPELL_SLOT_READY_HAND] = inventory.potions[0].thing;
    assert(dm1_spell_castPotionWithInventory(&s, 0, &stats, 0x000B,
                                             &inventory, &result) ==
           DM1_SPELL_CAST_SUCCESS);
    assert(result.castResult == DM1_SPELL_CAST_SUCCESS);
    assert(result.failureType == -1);
    assert(result.spellIndex == 19);
    assert(result.powerOrdinal == 1);
    assert(result.flaskSlotIndex == DM1_SPELL_SLOT_READY_HAND);
    assert(result.flaskThing == 0x2011u);
    assert(result.potionTypeBefore == DM1_SPELL_POTION_EMPTY_FLASK_PC34);
    assert(result.potionTypeAfter == 14);
    assert(result.potionPowerBefore == 7);
    assert(result.potionPowerAfter == 51);
    assert(inventory.potions[0].type == 14);
    assert(inventory.potions[0].power == 51);
    assert(result.loadBefore == 42);
    assert(result.loadAfter == 42);
    assert(inventory.load == 42);
    assert(s.input[0].symbols[0] == '\0');
    assert(s.input[0].symbolStep == 0);
    assert(result.symbolsCleared == 1);

    dm1_spell_init(&s);
    stats = makeStats(200, 200, 50, 60);
    stats.skillLevels[DM1_SKILL_HEAL] = 10;
    inventory.slots[DM1_SPELL_SLOT_READY_HAND] = 0x3010u;
    inventory.slots[DM1_SPELL_SLOT_ACTION_HAND] = 0x3011u;
    inventory.potionCount = 2;
    inventory.potions[0].thing = 0x3010u;
    inventory.potions[0].iconIndex = DM1_SPELL_ICON_EMPTY_FLASK_PC34;
    inventory.potions[0].type = DM1_SPELL_POTION_EMPTY_FLASK_PC34;
    inventory.potions[0].power = 1;
    inventory.potions[0].weight = 2;
    inventory.potions[1].thing = 0x3011u;
    inventory.potions[1].iconIndex = DM1_SPELL_ICON_EMPTY_FLASK_PC34;
    inventory.potions[1].type = DM1_SPELL_POTION_EMPTY_FLASK_PC34;
    inventory.potions[1].power = 2;
    inventory.potions[1].weight = 2;

    assert(dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_LO) == 1);
    assert(dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_VI) == 1);
    assert(dm1_spell_castPotionWithInventory(&s, 0, &stats, 0x0000,
                                             &inventory, &result) ==
           DM1_SPELL_CAST_SUCCESS);
    assert(result.flaskSlotIndex == DM1_SPELL_SLOT_ACTION_HAND);
    assert(result.flaskThing == 0x3011u);
    assert(inventory.potions[0].type == DM1_SPELL_POTION_EMPTY_FLASK_PC34);
    assert(inventory.potions[0].power == 1);
    assert(inventory.potions[1].type == 14);
    assert(inventory.potions[1].power == 40);

    printf("    PASS\n");
}

/* ── Test 16: Projectile kinetic energy ──────────────────────────── */
static void test_projectile_kinetic_energy(void) {
    printf("  [18] Projectile kinetic energy...\n");

    /* KE = bounded(21, (powerOrd+2)*(4+(skill<<1)), 255) */
    assert(dm1_spell_projectileKineticEnergy(3, 5, 0) == 70);
    assert(dm1_spell_projectileKineticEnergy(1, 0, 0) == 21);
    assert(dm1_spell_projectileKineticEnergy(6, 15, 0) == 255);
    /* Open Door doubles skill: pow=3, skill=5→10: (5)*(4+20)=120 */
    assert(dm1_spell_projectileKineticEnergy(3, 5, DM1_SPELL_TYPE_PROJ_OPEN_DOOR) == 120);

    printf("    PASS\n");
}

/* ── Test 17: Projectile step energy ─────────────────────────────── */
static void test_projectile_step_energy(void) {
    printf("  [19] Projectile step energy...\n");

    /* stepEnergy = 10 - min(8, maxMana >> 3) */
    assert(dm1_spell_projectileStepEnergy(0) == 10);
    assert(dm1_spell_projectileStepEnergy(64) == 2);
    assert(dm1_spell_projectileStepEnergy(100) == 2);
    assert(dm1_spell_projectileStepEnergy(24) == 7);

    printf("    PASS\n");
}

/* ── Test 18: Experience calculation ─────────────────────────────── */
static void test_experience(void) {
    printf("  [20] Experience calculation...\n");

    /* exp = rng8 + (req<<4) + ((powerOrd-1)*baseReq<<3) + req*req
     * pow=3, base=3, rng=5: req=6, exp=5+96+48+36=185 */
    assert(dm1_spell_experience(3, 3, 5) == 185);
    /* pow=1, base=0, rng=0: req=1, exp=0+16+0+1=17 */
    assert(dm1_spell_experience(1, 0, 0) == 17);

    printf("    PASS\n");
}

/* ── Test 19: Spell table integrity ──────────────────────────────── */
static void test_spell_table(void) {
    printf("  [21] Spell table integrity...\n");

    /* Shield: Ya Ir = 0x00666F00 */
    assert(dm1_spells[0].symbols == 0x00666F00);
    assert(dm1_spells[0].baseRequiredSkillLevel == 2);
    assert(dm1_spells[0].skillIndex == DM1_SKILL_DEFEND);
    assert(DM1_SPELL_KIND(&dm1_spells[0]) == DM1_SPELL_KIND_OTHER);

    /* Fireball: Ful Ir = 0x00696F00 */
    assert(dm1_spells[8].symbols == 0x00696F00);
    assert(dm1_spells[8].skillIndex == DM1_SKILL_FIRE);
    assert(DM1_SPELL_KIND(&dm1_spells[8]) == DM1_SPELL_KIND_PROJECTILE);

    /* Zokathra: Zo Kath Ra = 0x006B6E76 */
    assert(dm1_spells[24].symbols == 0x006B6E76);
    assert(dm1_spells[24].baseRequiredSkillLevel == 0);
    assert(dm1_spells[24].skillIndex == DM1_SKILL_WIZARD);
    assert(DM1_SPELL_KIND(&dm1_spells[24]) == DM1_SPELL_KIND_OTHER);
    assert(DM1_SPELL_TYPE(&dm1_spells[24]) == DM1_SPELL_TYPE_OTHER_ZOKATHRA);

    /* Open Door: Zo = 0x006B0000 */
    assert(dm1_spells[14].symbols == 0x006B0000);
    assert(DM1_SPELL_KIND(&dm1_spells[14]) == DM1_SPELL_KIND_PROJECTILE);
    assert(DM1_SPELL_TYPE(&dm1_spells[14]) == DM1_SPELL_TYPE_PROJ_OPEN_DOOR);

    /* All spells have valid kinds */
    for (int i = 0; i < DM1_SPELL_COUNT; i++) {
        int kind __attribute__((unused)) = DM1_SPELL_KIND(&dm1_spells[i]);
        assert(kind >= 1 && kind <= 3);
    }

    printf("    PASS\n");
}

/* ── Test 20: Multiple champions independent ─────────────────────── */
static void test_multiple_champions(void) {
    printf("  [22] Multiple champions independent...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats0 = makeStats(200, 200, 50, 60);
    DM1_ChampionSpellStats stats1 = makeStats(200, 200, 50, 60);

    dm1_spell_addSymbol(&s, 0, &stats0, DM1_POWER_ON);
    dm1_spell_addSymbol(&s, 0, &stats0, DM1_ELEM_FUL);

    dm1_spell_addSymbol(&s, 1, &stats1, DM1_POWER_LO);
    dm1_spell_addSymbol(&s, 1, &stats1, DM1_ELEM_ZO);

    assert(s.input[0].symbolStep == 2);
    assert(s.input[1].symbolStep == 2);
    assert(s.input[0].symbols[0] != s.input[1].symbols[0]);

    /* Champion 1 matches Open Door */
    assert((int)(dm1_spell_lookup(&s, 1) - dm1_spells) == 14);

    printf("    PASS\n");
}

/* ── Test 21: Zokathra full lookup ───────────────────────────────── */
static void test_zokathra(void) {
    printf("  [23] Zokathra full lookup...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(200, 200, 50, 60);

    /* Zo Kath Ra: Zo=step1 idx5, Kath=step2 idx2, Ra=step3 idx4 */
    dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_LO);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_ZO);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_CLASS_KATH);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_ALIGN_RA);

    const DM1_Spell* spell __attribute__((unused)) = dm1_spell_lookup(&s, 0);
    assert(spell != NULL);
    assert((int)(spell - dm1_spells) == 24);
    assert(DM1_SPELL_KIND(spell) == DM1_SPELL_KIND_OTHER);
    assert(DM1_SPELL_TYPE(spell) == DM1_SPELL_TYPE_OTHER_ZOKATHRA);

    printf("    PASS\n");
}

/* ── Test 22: Lightning Bolt lookup ──────────────────────────────── */
static void test_lightning_bolt(void) {
    printf("  [24] Lightning Bolt lookup...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(200, 200, 50, 60);

    /* Oh Kath Ra = 0x00686E76: Oh=step1 idx2, Kath=step2 idx2, Ra=step3 idx4 */
    dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_PAL);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_OH);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_CLASS_KATH);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_ALIGN_RA);

    const DM1_Spell* spell __attribute__((unused)) = dm1_spell_lookup(&s, 0);
    assert(spell != NULL);
    assert((int)(spell - dm1_spells) == 5);
    assert(strcmp(dm1_spell_name(5), "LIGHTNING BOLT") == 0);

    printf("    PASS\n");
}

/* ── Test 23: Only power symbol → no match ───────────────────────── */
static void test_power_only_no_match(void) {
    printf("  [25] Power symbol only → no match...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(200, 200, 50, 40);

    dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_ON);

    /* Single power symbol = no match (F0409: if !symbols[1] → NULL) */
    assert(dm1_spell_lookup(&s, 0) == NULL);

    printf("    PASS\n");
}


/* ── Test 24: Spell cast failure feedback clears needs-practice ─────── */
static void test_spell_cast_needs_practice_feedback(void) {
    printf("  [26] Spell cast failure feedback — needs practice...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(200, 200, 50, 0);
    stats.skillLevels[DM1_SKILL_FIRE] = 0;

    dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_ON);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_FUL);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_CLASS_IR);

    const DM1_Spell* outSpell = NULL;
    int failure = -1;
    int result = dm1_spell_cast(&s, 0, &stats, 0x7FFF, &outSpell, NULL, &failure);
    (void)result;
    assert(result == DM1_SPELL_CAST_FAILURE);
    assert(outSpell != NULL);
    assert(failure == DM1_FAILURE_NEEDS_MORE_PRACTICE);
    assert(s.input[0].symbols[0] == 0);
    assert(s.input[0].symbolStep == 0);

    printf("    PASS\n");
}

static void test_spell_cast_uses_live_f0303_skill_override(void) {
    printf("  [26b] Spell cast uses live F0303 skill override...\n");

    DM1_SpellCastingState s;
    DM1_ChampionSpellStats stats = makeStats(200, 200, 50, 0);
    const DM1_Spell* outSpell = NULL;
    int failure = -1;
    int powerOrdinal = -1;

    dm1_spell_init(&s);
    stats.skillLevels[DM1_SKILL_FIRE] = 0;
    stats.liveSkillLevelOverrideValid = 1;
    stats.liveSkillLevelOverrideIndex = DM1_SKILL_FIRE;
    stats.liveSkillLevelOverrideValue = 6;

    assert(dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_ON) == 1);
    assert(dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_FUL) == 1);
    assert(dm1_spell_addSymbol(&s, 0, &stats, DM1_CLASS_IR) == 1);

    assert(dm1_spell_cast(&s, 0, &stats, 0x7FFF,
                          &outSpell, &powerOrdinal, &failure) ==
           DM1_SPELL_CAST_SUCCESS);
    assert(outSpell == &dm1_spells[8]);
    assert(powerOrdinal == 3);
    assert(failure == -1);
    assert(s.input[0].symbols[0] == 0);
    assert(s.input[0].symbolStep == 0);

    dm1_spell_init(&s);
    memset(&stats, 0, sizeof(stats));
    stats = makeStats(200, 200, 50, 0);
    stats.skillLevels[DM1_SKILL_FIRE] = 0;
    assert(dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_ON) == 1);
    assert(dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_FUL) == 1);
    assert(dm1_spell_addSymbol(&s, 0, &stats, DM1_CLASS_IR) == 1);
    assert(dm1_spell_cast(&s, 0, &stats, 0x7FFF,
                          &outSpell, &powerOrdinal, &failure) ==
           DM1_SPELL_CAST_FAILURE);
    assert(failure == DM1_FAILURE_NEEDS_MORE_PRACTICE);

    printf("    PASS\n");
}

/* ── Test 25: Spell failure feedback metadata ───────────────────────── */
static void test_spell_failure_feedback_metadata(void) {
    printf("  [27] Spell failure feedback metadata...\n");

    const DM1_SpellFailureFeedback* practice = dm1_spell_failureFeedback(DM1_FAILURE_NEEDS_MORE_PRACTICE);
    (void)practice;
    assert(practice != NULL);
    assert(practice->messageColor == 4);
    assert(practice->printsLineFeed == 1);
    assert(practice->printsChampionName == 1);
    assert(practice->appendsBaseSkillName == 1);
    assert(practice->clearsSymbolsOnCastClick == 1);
    assert(strcmp(practice->messageBeforeSkill, " NEEDS MORE PRACTICE WITH THIS ") == 0);
    assert(strcmp(practice->messageAfterSkill, " SPELL.") == 0);

    const DM1_SpellFailureFeedback* meaningless = dm1_spell_failureFeedback(DM1_FAILURE_MEANINGLESS_SPELL);
    (void)meaningless;
    assert(meaningless != NULL);
    assert(meaningless->clearsSymbolsOnCastClick == 1);
    assert(strcmp(meaningless->messageBeforeSkill, " MUMBLES A MEANINGLESS SPELL.") == 0);

    const DM1_SpellFailureFeedback* flask = dm1_spell_failureFeedback(DM1_FAILURE_NEEDS_FLASK_IN_HAND);
    (void)flask;
    assert(flask != NULL);
    assert(flask->clearsSymbolsOnCastClick == 0);
    assert(strcmp(flask->messageBeforeSkill, " NEEDS AN EMPTY FLASK IN HAND FOR POTION.") == 0);

    const DM1_SpellFailureFeedback* magicMap = dm1_spell_failureFeedback(DM1_FAILURE_NEEDS_MAGIC_MAP_IN_HAND);
    (void)magicMap;
    assert(magicMap != NULL);
    assert(magicMap->clearsSymbolsOnCastClick == 0);
    assert(strcmp(magicMap->messageBeforeSkill, " NEEDS A MAGIC MAP IN ACTION HAND FOR THIS SPELL.") == 0);

    assert(dm1_spell_failureFeedback(99) == NULL);

    printf("    PASS\n");
}

/* ── Test 26: F0408 cast-click symbol cleanup predicate ─────────────── */
static void test_spell_cast_click_cleanup_predicate(void) {
    printf("  [28] Spell cast-click cleanup predicate...\n");

    assert(dm1_spell_castClearsSymbolsForResult(DM1_SPELL_CAST_FAILURE) == 1);
    assert(dm1_spell_castClearsSymbolsForResult(DM1_SPELL_CAST_SUCCESS) == 1);
    assert(dm1_spell_castClearsSymbolsForResult(DM1_SPELL_CAST_FAILURE_NEEDS_FLASK) == 0);
    assert(dm1_spell_castClearsSymbolsForResult(DM1_SPELL_CAST_FAILURE_NEEDS_MAGIC_MAP) == 0);

    printf("    PASS\n");
}

/* ═══════════════════════════════════════════════════════════════════ */
int main(void) {
    printf("DM1 V1 Spell Casting — CTest gate\n");
    printf("Source: ReDMCSB WIP20210206\n\n");

    test_symbol_encoding();
    test_symbol_names();
    test_init();
    test_add_symbol();
    test_insufficient_mana();
    test_insufficient_mana_preserves_partial_rune_chain();
    test_delete_symbol();
    test_spell_lookup_fireball();
    test_spell_lookup_open_door();
    test_spell_lookup_meaningless();
    test_mana_cost();
    test_malformed_symbol_step_rejected_without_mutation();
    test_malformed_power_rune_cannot_dispatch_spell();
    test_spell_cast_success();
    test_spell_cast_dead();
    test_spell_cast_meaningless();
    test_spell_cast_potion();
    test_spell_cast_potion_flask_inventory_mutation();
    test_projectile_kinetic_energy();
    test_projectile_step_energy();
    test_experience();
    test_spell_table();
    test_multiple_champions();
    test_zokathra();
    test_lightning_bolt();
    test_power_only_no_match();
    test_spell_cast_needs_practice_feedback();
    test_spell_cast_uses_live_f0303_skill_override();
    test_spell_failure_feedback_metadata();
    test_spell_cast_click_cleanup_predicate();

    printf("\nAll 30 tests PASSED.\n");
    return 0;
}
