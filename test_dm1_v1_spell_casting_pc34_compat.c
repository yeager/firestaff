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
    int ok = dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_ON);
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

/* ── Test 5: Insufficient mana blocks symbol ─────────────────────── */
static void test_insufficient_mana(void) {
    printf("  [5] Insufficient mana blocks symbol...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(0, 100, 50, 40);

    int ok = dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_LO);
    assert(ok == 0);
    assert(s.input[0].symbols[0] == '\0');
    assert(stats.currentMana == 0);

    printf("    PASS\n");
}

/* ── Test 6: Delete symbol (recant) ──────────────────────────────── */
static void test_delete_symbol(void) {
    printf("  [6] Delete symbol (recant)...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(100, 100, 50, 40);

    dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_ON);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_FUL);
    int manaBefore = stats.currentMana;

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
    printf("  [7] Spell lookup — Fireball...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(200, 200, 50, 40);

    /* Fireball = Ful Ir = 0x00696F00
     * Power "On" (step 0 idx 2) + Ful (step 1 idx 3) + Ir (step 2 idx 3) */
    dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_ON);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_FUL);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_CLASS_IR);

    const DM1_Spell* spell = dm1_spell_lookup(&s, 0);
    assert(spell != NULL);
    assert(DM1_SPELL_KIND(spell) == DM1_SPELL_KIND_PROJECTILE);
    assert(spell->skillIndex == DM1_SKILL_FIRE);
    assert((int)(spell - dm1_spells) == 8);
    assert(strcmp(dm1_spell_name(8), "FIREBALL") == 0);

    printf("    PASS\n");
}

/* ── Test 8: Spell lookup — Open Door ────────────────────────────── */
static void test_spell_lookup_open_door(void) {
    printf("  [8] Spell lookup — Open Door...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(200, 200, 50, 40);

    /* Open Door = Zo = 0x006B0000
     * Lo (power) + Zo (step 1 idx 5) */
    dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_LO);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_ZO);

    const DM1_Spell* spell = dm1_spell_lookup(&s, 0);
    assert(spell != NULL);
    assert((int)(spell - dm1_spells) == 14);
    assert(DM1_SPELL_KIND(spell) == DM1_SPELL_KIND_PROJECTILE);
    assert(DM1_SPELL_TYPE(spell) == DM1_SPELL_TYPE_PROJ_OPEN_DOOR);

    printf("    PASS\n");
}

/* ── Test 9: Spell lookup — meaningless sequence ─────────────────── */
static void test_spell_lookup_meaningless(void) {
    printf("  [9] Spell lookup — meaningless...\n");

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
    printf("  [10] Mana cost calculation...\n");

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

/* ── Test 11: Spell cast — success ───────────────────────────────── */
static void test_spell_cast_success(void) {
    printf("  [11] Spell cast — success...\n");

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

    int result = dm1_spell_cast(&s, 0, &stats, 0x1234, &outSpell, &powerOrd, &failure);
    assert(result == DM1_SPELL_CAST_SUCCESS);
    assert(outSpell != NULL);
    assert(powerOrd == 3);  /* On = char 98, ordinal = 98 - 95 = 3 */
    assert(DM1_SPELL_KIND(outSpell) == DM1_SPELL_KIND_PROJECTILE);

    /* Symbols cleared after success */
    assert(s.input[0].symbols[0] == '\0');
    assert(s.input[0].symbolStep == 0);

    printf("    PASS\n");
}

/* ── Test 12: Spell cast — dead champion ─────────────────────────── */
static void test_spell_cast_dead(void) {
    printf("  [12] Spell cast — dead champion...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(200, 200, 0, 60);
    stats.skillLevels[DM1_SKILL_FIRE] = 10;

    dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_ON);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_FUL);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_CLASS_IR);

    assert(dm1_spell_cast(&s, 0, &stats, 0, NULL, NULL, NULL) == DM1_SPELL_CAST_FAILURE);

    printf("    PASS\n");
}

/* ── Test 13: Spell cast — meaningless ───────────────────────────── */
static void test_spell_cast_meaningless(void) {
    printf("  [13] Spell cast — meaningless...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(200, 200, 50, 60);

    dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_MON);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_YA);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_CLASS_VEN);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_ALIGN_KU);

    int failure = -1;
    assert(dm1_spell_cast(&s, 0, &stats, 0, NULL, NULL, &failure) == DM1_SPELL_CAST_FAILURE);
    assert(failure == DM1_FAILURE_MEANINGLESS_SPELL);

    printf("    PASS\n");
}

/* ── Test 14: Spell cast — potion needs flask ────────────────────── */
static void test_spell_cast_potion(void) {
    printf("  [14] Spell cast — potion needs flask...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(200, 200, 50, 60);
    stats.skillLevels[DM1_SKILL_HEAL] = 10;

    /* Health Potion = Vi (step1 idx1 = char 103 = 0x67)
     * Spell: { 0x00670000, 1, HEAL, 0x80E1 } kind=1=potion */
    dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_LO);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_VI);

    const DM1_Spell* outSpell = NULL;
    int result = dm1_spell_cast(&s, 0, &stats, 0, &outSpell, NULL, NULL);
    assert(result == DM1_SPELL_CAST_FAILURE_NEEDS_FLASK);
    assert(outSpell != NULL);
    assert(DM1_SPELL_KIND(outSpell) == DM1_SPELL_KIND_POTION);

    printf("    PASS\n");
}

/* ── Test 15: Projectile kinetic energy ──────────────────────────── */
static void test_projectile_kinetic_energy(void) {
    printf("  [15] Projectile kinetic energy...\n");

    /* KE = bounded(21, (powerOrd+2)*(4+(skill<<1)), 255) */
    assert(dm1_spell_projectileKineticEnergy(3, 5, 0) == 70);
    assert(dm1_spell_projectileKineticEnergy(1, 0, 0) == 21);
    assert(dm1_spell_projectileKineticEnergy(6, 15, 0) == 255);
    /* Open Door doubles skill: pow=3, skill=5→10: (5)*(4+20)=120 */
    assert(dm1_spell_projectileKineticEnergy(3, 5, DM1_SPELL_TYPE_PROJ_OPEN_DOOR) == 120);

    printf("    PASS\n");
}

/* ── Test 16: Projectile step energy ─────────────────────────────── */
static void test_projectile_step_energy(void) {
    printf("  [16] Projectile step energy...\n");

    /* stepEnergy = 10 - min(8, maxMana >> 3) */
    assert(dm1_spell_projectileStepEnergy(0) == 10);
    assert(dm1_spell_projectileStepEnergy(64) == 2);
    assert(dm1_spell_projectileStepEnergy(100) == 2);
    assert(dm1_spell_projectileStepEnergy(24) == 7);

    printf("    PASS\n");
}

/* ── Test 17: Experience calculation ─────────────────────────────── */
static void test_experience(void) {
    printf("  [17] Experience calculation...\n");

    /* exp = rng8 + (req<<4) + ((powerOrd-1)*baseReq<<3) + req*req
     * pow=3, base=3, rng=5: req=6, exp=5+96+48+36=185 */
    assert(dm1_spell_experience(3, 3, 5) == 185);
    /* pow=1, base=0, rng=0: req=1, exp=0+16+0+1=17 */
    assert(dm1_spell_experience(1, 0, 0) == 17);

    printf("    PASS\n");
}

/* ── Test 18: Spell table integrity ──────────────────────────────── */
static void test_spell_table(void) {
    printf("  [18] Spell table integrity...\n");

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
        int kind = DM1_SPELL_KIND(&dm1_spells[i]);
        assert(kind >= 1 && kind <= 3);
    }

    printf("    PASS\n");
}

/* ── Test 19: Multiple champions independent ─────────────────────── */
static void test_multiple_champions(void) {
    printf("  [19] Multiple champions independent...\n");

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

/* ── Test 20: Zokathra full lookup ───────────────────────────────── */
static void test_zokathra(void) {
    printf("  [20] Zokathra full lookup...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(200, 200, 50, 60);

    /* Zo Kath Ra: Zo=step1 idx5, Kath=step2 idx2, Ra=step3 idx4 */
    dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_LO);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_ZO);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_CLASS_KATH);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_ALIGN_RA);

    const DM1_Spell* spell = dm1_spell_lookup(&s, 0);
    assert(spell != NULL);
    assert((int)(spell - dm1_spells) == 24);
    assert(DM1_SPELL_KIND(spell) == DM1_SPELL_KIND_OTHER);
    assert(DM1_SPELL_TYPE(spell) == DM1_SPELL_TYPE_OTHER_ZOKATHRA);

    printf("    PASS\n");
}

/* ── Test 21: Lightning Bolt lookup ──────────────────────────────── */
static void test_lightning_bolt(void) {
    printf("  [21] Lightning Bolt lookup...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(200, 200, 50, 60);

    /* Oh Kath Ra = 0x00686E76: Oh=step1 idx2, Kath=step2 idx2, Ra=step3 idx4 */
    dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_PAL);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_OH);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_CLASS_KATH);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_ALIGN_RA);

    const DM1_Spell* spell = dm1_spell_lookup(&s, 0);
    assert(spell != NULL);
    assert((int)(spell - dm1_spells) == 5);
    assert(strcmp(dm1_spell_name(5), "LIGHTNING BOLT") == 0);

    printf("    PASS\n");
}

/* ── Test 22: Only power symbol → no match ───────────────────────── */
static void test_power_only_no_match(void) {
    printf("  [22] Power symbol only → no match...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(200, 200, 50, 40);

    dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_ON);

    /* Single power symbol = no match (F0409: if !symbols[1] → NULL) */
    assert(dm1_spell_lookup(&s, 0) == NULL);

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
    test_delete_symbol();
    test_spell_lookup_fireball();
    test_spell_lookup_open_door();
    test_spell_lookup_meaningless();
    test_mana_cost();
    test_spell_cast_success();
    test_spell_cast_dead();
    test_spell_cast_meaningless();
    test_spell_cast_potion();
    test_projectile_kinetic_energy();
    test_projectile_step_energy();
    test_experience();
    test_spell_table();
    test_multiple_champions();
    test_zokathra();
    test_lightning_bolt();
    test_power_only_no_match();

    printf("\nAll 22 tests PASSED.\n");
    return 0;
}
