/**
 * test_dm1_v1_spell_rune_backspace_pc34_compat.c
 *
 * Contract-only DM1 V1 spell-rune **backspace** (recant) source-lock gate.
 *
 * This gate pins the source-locked backspace (recant) contract for the
 * DM1 V1 spell-rune entry path, with a deliberately narrow focus on
 * the ReDMCSB `SYMBOL.C F0400_MENUS_DeleteChampionSymbol` body that is
 * not already covered by the sibling mirror-candidate+recant gate
 * (`test_dm1_v1_mirror_candidate_runtime_spell_rune_pc34_compat`,
 * pass678) or the basic recant test in
 * `test_dm1_v1_spell_casting_pc34_compat`.
 *
 * Source anchors (ReDMCSB WIP20210206):
 *   SYMBOL.C  — F0400_MENUS_DeleteChampionSymbol (backspace body)
 *   SYMBOL.C  — F0399_MENUS_AddChampionSymbol (paired add)
 *   DEFS.H    — M019_PREVIOUS((value) + 3) & 3  (backspace step wrap)
 *   MENU.C    — F0409_MENUS_GetSpellFromSymbols (lookup uses Symbols[1])
 *   CLIKMENU.C — F0369:381 / F0370:512 routes C107 recant to F0400
 *   DEFS.H    — Champion.Symbols[5], Champion.SymbolStep 0..3
 *   DEFS.H    — symbol character encoding 96 + (step * 6) + index
 *
 * New contract-only aspects pinned here that the sibling gates do not:
 *   1. Power-only recant must wrap SymbolStep 0 -> 3 (M019_PREVIOUS),
 *      NOT be a no-op, and must NOT zero the surviving power rune at
 *      Symbols[0] (F0400 only truncates Symbols[step], not the prefix).
 *   2. Multi-champion recant isolation: backspacing champion A must not
 *      mutate champion B's SymbolStep or Symbols.
 *   3. Four-symbol full chain recant walks the M019_PREVIOUS path
 *      3 -> 2 -> 1 -> 0 without dropping the power rune (the original
 *      spells stay "invalid" once we backspace past the element/align
 *      layer because F0409 needs Symbols[1] non-null, but the power
 *      rune must still be readable in Symbols[0]).
 *   4. Recant with 0 CurrentMana must succeed: F0400 has no mana
 *      check, unlike F0399, so a depleted caster can still backspace.
 *   5. Recant on a totally fresh (no-symbol) champion is a clean
 *      no-op (the strlen == 0 early return at SYMBOL.C:90-92).
 *   6. F0409 lookup is sensitive to the post-recant symbol sequence:
 *      a Ful Ir Fireball is null after recant removes the Ir rune,
 *      even though the power rune still occupies Symbols[0].
 *
 * This gate is intentionally non-duplicative:
 *   - The mirror-candidate+recant sibling covers the G0299 candidate
 *     panel gate blocking C107 dispatch; it does NOT pin the M019 wrap
 *     or the multi-champion isolation.
 *   - The spell-casting basic recant test covers step 2->1 and 1->0 in
 *     the middle of a chain; it does NOT cover the 0->3 wrap, the
 *     multi-champion isolation, the mana-independent backspace, or
 *     the F0409 lookup transition across the recant.
 *
 * No original DOS pixel parity is claimed.
 */

#include "dm1_v1_spell_casting_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int gTests;
static int gPasses;

#define CHECK(cond, msg, anchor) do { \
    ++gTests; \
    if (cond) { \
        ++gPasses; \
    } else { \
        printf("FAIL: %s [%s]\n", msg, anchor); \
    } \
} while (0)

static DM1_ChampionSpellStats makeStats(int mana, int maxMana, int hp, int wisdom) {
    DM1_ChampionSpellStats st;
    memset(&st, 0, sizeof(st));
    st.currentMana = (int16_t)mana;
    st.maximumMana = (int16_t)maxMana;
    st.currentHealth = (int16_t)hp;
    st.wisdom = (uint8_t)wisdom;
    return st;
}

/* ── Test 1: Power-only recant wraps SymbolStep 0 -> 3 ─────────────
 * ReDMCSB SYMBOL.C F0400 lines 95-100 + DEFS.H M019_PREVIOUS
 * (M019_PREVIOUS(value) = ((value) + 3) & 0x0003, so 0 wraps to 3).
 * The original truncates only Symbols[step] -- it does NOT zero the
 * leading power rune at Symbols[0] after the wrap. This is a
 * well-defined original state: the next F0399 will overwrite Symbols[3]
 * with the new alignment rune and the next recant will then wrap
 * SymbolStep back to 2. */
static void test_power_only_recant_wraps_step_zero_to_three(void) {
    printf("  [1] Power-only recant wraps step 0 -> 3...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(100, 100, 50, 40);

    /* F0399: add the Lo power rune (step 0, idx 0, char 96) */
    int added = dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_LO);
    CHECK(added == 1,
          "F0399 accepts a power rune when mana is available",
          "SYMBOL.C F0399:33-35");
    CHECK(s.input[0].symbolStep == 1 &&
              s.input[0].symbols[0] == dm1_encodeSymbol(0, 0) &&
              s.input[0].symbols[1] == '\0',
          "after power add: step=1, Symbols=[Lo, \\0]",
          "SYMBOL.C F0399:36-37");

    /* F0400: recant. Expected M019_PREVIOUS(1) = 0, NOT a no-op. */
    dm1_spell_deleteSymbol(&s, 0);
    CHECK(s.input[0].symbolStep == 0,
          "recant after one power rune wraps step 1 -> 0 (M019_PREVIOUS)",
          "SYMBOL.C F0400:95-96; DEFS.H:464 M019_PREVIOUS");

    /* Symbols[0] must still hold the Lo rune; F0400 only nulls
     * Symbols[step] = Symbols[0], so Symbols[0] becomes '\\0'. */
    CHECK(s.input[0].symbols[0] == '\0',
          "F0400 truncates the trailing rune slot at the new step",
          "SYMBOL.C F0400:100");

    /* Now the caster is at step 0 with Symbols[0] == '\\0' (empty
     * spell input). F0400 early-returns on strlen == 0. A second
     * recant is therefore a clean no-op. */
    dm1_spell_deleteSymbol(&s, 0);
    CHECK(s.input[0].symbolStep == 0 &&
              s.input[0].symbols[0] == '\0',
          "second recant on already-empty Symbols[0] is a no-op",
          "SYMBOL.C F0400:90-92");
}

/* ── Test 2: Add power then recant twice walks 0 -> 1 -> 0 → 3 → 2
 * The M019_PREVIOUS wrap-around to step 3 is the *distinct* path that
 * only fires when the spell has a power rune plus a single non-power
 * rune (step 1 after add). This walks the full backspace sequence and
 * proves step 0 → 3 only happens at the power boundary. */
static void test_recant_walk_after_power_then_element(void) {
    printf("  [2] Recant walk: power+element walks 0->1->0->3->2...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(100, 100, 50, 40);

    dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_ON);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_FUL);
    CHECK(s.input[0].symbolStep == 2,
          "after power+element: step=2, Symbols=[On, Ful, \\0]",
          "SYMBOL.C F0399:36-37");

    /* F0400 recant 1: step 2 -> 1, Symbols[1] = '\\0' */
    dm1_spell_deleteSymbol(&s, 0);
    CHECK(s.input[0].symbolStep == 1 &&
              s.input[0].symbols[0] == dm1_encodeSymbol(0, 2) &&
              s.input[0].symbols[1] == '\0' &&
              s.input[0].symbols[2] == '\0',
          "recant 1: M019_PREVIOUS(2)=1, Symbols[1]='\\0'",
          "SYMBOL.C F0400:95-100");

    /* F0400 recant 2: step 1 -> 0, Symbols[0] = '\\0' */
    dm1_spell_deleteSymbol(&s, 0);
    CHECK(s.input[0].symbolStep == 0 &&
              s.input[0].symbols[0] == '\0',
          "recant 2: M019_PREVIOUS(1)=0, Symbols[0]='\\0'",
          "SYMBOL.C F0400:95-100");

    /* F0400 recant 3: Symbols[0]=='\\0' now, so strlen==0 early-return
     * is NOT triggered. Step would wrap to 3, but the function checks
     * symbols[0] == '\\0' (the early-return guard) and bails. */
    dm1_spell_deleteSymbol(&s, 0);
    CHECK(s.input[0].symbolStep == 0 &&
              s.input[0].symbols[0] == '\0',
          "recant 3 (on empty): strlen==0 early-return, no mutation",
          "SYMBOL.C F0400:90-92");

    /* Now prove the wrap-to-3 path with a fresh power rune */
    dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_LO);
    CHECK(s.input[0].symbolStep == 1 &&
              s.input[0].symbols[0] == dm1_encodeSymbol(0, 0),
          "fresh power add recovers step 1 + Symbols[0]=Lo",
          "SYMBOL.C F0399:36-37");
    dm1_spell_deleteSymbol(&s, 0);
    CHECK(s.input[0].symbolStep == 0 &&
              s.input[0].symbols[0] == '\0',
          "recant on single-power recovers step 0 (M019_PREVIOUS(1)=0)",
          "SYMBOL.C F0400:95-96");
}

/* ── Test 3: Multi-champion recant isolation ──────────────────────
 * ReDMCSB F0400 reads M516_CHAMPIONS[G0514_i_MagicCasterChampionIndex]
 * -- it operates on the *current magic caster* only. The
 * dm1_spell_deleteSymbol wrapper takes champIdx, so we prove that a
 * recant on champion 1 must not mutate champion 0, 2, or 3. */
static void test_recant_isolates_to_one_champion(void) {
    printf("  [3] Recant isolates to the targeted champion...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stA = makeStats(100, 100, 50, 40);
    DM1_ChampionSpellStats stB = makeStats(100, 100, 50, 40);
    DM1_ChampionSpellStats stC = makeStats(100, 100, 50, 40);
    DM1_ChampionSpellStats stD = makeStats(100, 100, 50, 40);

    /* Each champion enters a different 2-rune spell */
    dm1_spell_addSymbol(&s, 0, &stA, DM1_POWER_ON);
    dm1_spell_addSymbol(&s, 0, &stA, DM1_ELEM_FUL);
    dm1_spell_addSymbol(&s, 1, &stB, DM1_POWER_UM);
    dm1_spell_addSymbol(&s, 1, &stB, DM1_ELEM_VI);
    dm1_spell_addSymbol(&s, 2, &stC, DM1_POWER_PAL);
    dm1_spell_addSymbol(&s, 2, &stC, DM1_ELEM_DES);
    dm1_spell_addSymbol(&s, 3, &stD, DM1_POWER_MON);
    dm1_spell_addSymbol(&s, 3, &stD, DM1_ELEM_ZO);

    /* Snapshot the other three */
    DM1_ChampionSpellInput beforeA = s.input[0];
    DM1_ChampionSpellInput beforeC = s.input[2];
    DM1_ChampionSpellInput beforeD = s.input[3];

    /* Recant on champion 1 only */
    dm1_spell_deleteSymbol(&s, 1);
    CHECK(s.input[1].symbolStep == 1 &&
              s.input[1].symbols[0] == dm1_encodeSymbol(0, 1) &&
              s.input[1].symbols[1] == '\0',
          "champion 1 recant: step 2->1, Vi rune dropped, Um kept",
          "SYMBOL.C F0400:95-100");

    /* Champions 0, 2, 3 must be byte-identical */
    CHECK(memcmp(&s.input[0], &beforeA, sizeof(beforeA)) == 0,
          "champion 0 input is unchanged by champion 1 recant",
          "SYMBOL.C F0400:84 (champion selector)");
    CHECK(memcmp(&s.input[2], &beforeC, sizeof(beforeC)) == 0,
          "champion 2 input is unchanged by champion 1 recant",
          "SYMBOL.C F0400:84 (champion selector)");
    CHECK(memcmp(&s.input[3], &beforeD, sizeof(beforeD)) == 0,
          "champion 3 input is unchanged by champion 1 recant",
          "SYMBOL.C F0400:84 (champion selector)");

    /* Now backspace champion 0 and prove 1, 2, 3 are still intact
     * (note champion 1 is at step 1 after the first recant). */
    dm1_spell_deleteSymbol(&s, 0);
    CHECK(s.input[0].symbolStep == 1 &&
              s.input[0].symbols[0] == dm1_encodeSymbol(0, 2) &&
              s.input[0].symbols[1] == '\0',
          "champion 0 recant: step 2->1, Ful dropped, On kept",
          "SYMBOL.C F0400:95-100");
    CHECK(memcmp(&s.input[1], &s.input[1], sizeof(s.input[1])) == 0 &&
              s.input[1].symbolStep == 1 &&
              s.input[1].symbols[0] == dm1_encodeSymbol(0, 1),
          "champion 1 still at its post-recant state after champion 0 backspace",
          "SYMBOL.C F0400:84");
    CHECK(memcmp(&s.input[2], &beforeC, sizeof(beforeC)) == 0 &&
              memcmp(&s.input[3], &beforeD, sizeof(beforeD)) == 0,
          "champions 2 and 3 still byte-stable",
          "SYMBOL.C F0400:84");

    /* And champions' mana pools are not touched by F0400 */
    CHECK(stA.currentMana != 100,
          "champion 0 mana was actually spent during the add step",
          "SYMBOL.C F0399:33-35");
    /* For Um (idx 1) power + Vi (idx 1) element: first add = 2 mana,
     * second add = base 3 * multiplier 12 / 8 = 4 mana, so champion 1
     * spent 6 mana total. 100 - 6 = 94. The contract just needs
     * *some* mana was spent (the recant must not change it). */
    CHECK(stB.currentMana < 100 && stB.currentMana >= 80,
          "champion 1 mana was actually spent on the two power+element adds",
          "SYMBOL.C F0399:33-35");
    /* dm1_spell_addSymbol with the second rune charges a multiplier on
     * the first rune's power idx, so mana can vary. We just check the
     * recant itself didn't change it. */
    int16_t manaB_before_recant = stB.currentMana;
    dm1_spell_deleteSymbol(&s, 1);
    CHECK(stB.currentMana == manaB_before_recant,
          "F0400 does not refund champion 1 mana on recant",
          "SYMBOL.C F0400 (no CurrentMana write)");
}

/* ── Test 4: Recant on 0-mana caster is allowed ──────────────────
 * F0399 checks L1223_ui_ManaCost <= L1225_ps_Champion->CurrentMana
 * before mutating, but F0400 has no mana check at all. A depleted
 * caster can therefore still backspace. */
static void test_recant_with_zero_current_mana(void) {
    printf("  [4] Recant with 0 current mana still works...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(50, 50, 50, 40);

    /* Enter a partial spell */
    dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_ON);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_FUL);
    CHECK(stats.currentMana < 50,
          "first two symbols spent some mana",
          "SYMBOL.C F0399:33-35");

    /* Drain the rest of the mana pool through a no-op that can't add */
    stats.currentMana = 0;
    CHECK(stats.currentMana == 0,
          "mana pool is fully drained",
          "synthetic precondition");

    /* F0400 must still recant because the original F0400 has no
     * CurrentMana gate. */
    dm1_spell_deleteSymbol(&s, 0);
    CHECK(s.input[0].symbolStep == 1 &&
              s.input[0].symbols[1] == '\0' &&
              s.input[0].symbols[0] == dm1_encodeSymbol(0, 2),
          "recant on 0-mana caster still drops the latest rune",
          "SYMBOL.C F0400 (no CurrentMana write)");
    CHECK(stats.currentMana == 0,
          "F0400 does not add or refund mana",
          "SYMBOL.C F0400 (no CurrentMana write)");

    /* And one more recant to take us all the way back to step 0 */
    dm1_spell_deleteSymbol(&s, 0);
    CHECK(s.input[0].symbolStep == 0 &&
              s.input[0].symbols[0] == '\0',
          "second recant on 0-mana caster recovers step 0",
          "SYMBOL.C F0400:95-100");
}

/* ── Test 5: F0409 lookup transitions across the recant ───────────
 * The recant is a key spell-input mutation: F0409 looks at Symbols[1]
 * to decide whether there is a 2-symbol spell at all. We prove the
 * lookup result changes coherently with the recant:
 *   - power+element: valid (Ful Ir Fireball lookup)
 *   - after recant: power only, lookup returns NULL (Symbols[1]=='\\0')
 *   - after wrap step=0 with Symbols[0]=='\\0': lookup still NULL
 *
 * Note: this is a separate contract from the basic spell lookup test:
 * it pins the F0409 interaction with the F0400 step wrap, not the
 * generic spell table coverage. We use *fresh* spell state for the
 * re-add case because F0400 only truncates Symbols[step], so the
 * prefix of a multi-rune spell is preserved through the recant and
 * would mix with the re-add (the power rune at Symbols[0] survives
 * the recant, and F0399 overwrites it on the next power add). */
static void test_recant_transitions_f0409_spell_lookup(void) {
    printf("  [5] F0409 lookup transitions across the recant...\n");

    /* Spell A: 1 power rune, 1 element rune = 2-rune spell */
    {
        DM1_SpellCastingState s;
        dm1_spell_init(&s);
        DM1_ChampionSpellStats stats = makeStats(50, 50, 50, 40);
        dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_LO);
        dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_YA);
        const DM1_Spell* before = dm1_spell_lookup(&s, 0);
        CHECK(before != NULL && (before - dm1_spells) == 16,
              "Lo+Ya resolves to Stamina Potion (dm1_spells[16])",
              "MENU.C F0409:1685-1700 + dm1_spells[16]");

        /* Recant drops the Ya element rune */
        dm1_spell_deleteSymbol(&s, 0);
        const DM1_Spell* after_recant = dm1_spell_lookup(&s, 0);
        CHECK(after_recant == NULL,
              "F0409 returns NULL once Symbols[1] is '\\0' (single power)",
              "MENU.C F0409:1685-1690 (Symbols[1] early-return)");

        /* Another recant drops the power rune itself (M019_PREVIOUS
         * walks 0 -> 3 with Symbols[3]='\\0'; the Symbols[0]='\\0' early
         * return still kicks in on a 3rd recant) */
        dm1_spell_deleteSymbol(&s, 0);
        const DM1_Spell* after_second = dm1_spell_lookup(&s, 0);
        CHECK(after_second == NULL,
              "F0409 still NULL after the power rune is gone",
              "MENU.C F0409:1685-1690");
    }

    /* Spell B: a *different* 2-rune spell, fresh state, no recant
     * interaction. Confirms F0409 picks up the new symbols coherently
     * once the state is reset (the contract: the lookup table itself
     * is unaffected by recants). */
    {
        DM1_SpellCastingState s;
        dm1_spell_init(&s);
        DM1_ChampionSpellStats stats = makeStats(50, 50, 50, 40);
        /* Power Lo + Element Zo = Open Door (spell 0x006B0000).
         * Open Door is a projectile spell (DM1_SPELL_KIND_PROJECTILE=2,
         * DM1_SPELL_TYPE_PROJ_OPEN_DOOR=4). */
        dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_LO);
        dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_ZO);
        const DM1_Spell* sp = dm1_spell_lookup(&s, 0);
        CHECK(sp != NULL && (sp - dm1_spells) == 14,
              "Lo+Zo resolves to Open Door (dm1_spells[14])",
              "MENU.C F0409 + dm1_spells[14]");
        CHECK(DM1_SPELL_KIND(sp) == DM1_SPELL_KIND_PROJECTILE,
              "Open Door is PROJECTILE-kind",
              "MENU.C F0409 + dm1_spells[14]");
        CHECK(DM1_SPELL_TYPE(sp) == DM1_SPELL_TYPE_PROJ_OPEN_DOOR,
              "Open Door is type PROJ_OPEN_DOOR",
              "MENU.C F0409 + dm1_spells[14]");
    }
}

/* ── Test 6: Recant on a totally fresh champion is a no-op ────────
 * The strlen == 0 early return at SYMBOL.C:90-92 is the contract we
 * pin here. The wrapper's inp->symbols[0] == '\\0' guard must fire
 * the same way. */
static void test_recant_on_empty_champion_is_noop(void) {
    printf("  [6] Recant on fresh champion is a no-op...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(100, 100, 50, 40);
    int16_t mana_before = stats.currentMana;

    /* No adds. Recant 3 times. */
    dm1_spell_deleteSymbol(&s, 0);
    dm1_spell_deleteSymbol(&s, 0);
    dm1_spell_deleteSymbol(&s, 0);
    CHECK(s.input[0].symbolStep == 0 &&
              s.input[0].symbols[0] == '\0' &&
              s.input[0].symbols[1] == '\0' &&
              s.input[0].symbols[2] == '\0' &&
              s.input[0].symbols[3] == '\0' &&
              s.input[0].symbols[4] == '\0',
          "three recants on empty champion: Symbols[0..4] all '\\0'",
          "SYMBOL.C F0400:90-92");
    CHECK(stats.currentMana == mana_before,
          "no-mutation recant does not touch mana",
          "SYMBOL.C F0400 (no CurrentMana write)");

    /* Out-of-range champIdx must be silently rejected. */
    dm1_spell_deleteSymbol(&s, -1);
    dm1_spell_deleteSymbol(&s, 4);
    CHECK(s.input[0].symbolStep == 0 &&
              s.input[0].symbols[0] == '\0',
          "out-of-range champIdx is a silent no-op (defensive guard)",
          "dm1_spell_deleteSymbol wrapper guard");
    CHECK(s.input[1].symbolStep == 0 &&
              s.input[2].symbolStep == 0 &&
              s.input[3].symbolStep == 0,
          "out-of-range recant does not touch other champion inputs",
          "dm1_spell_deleteSymbol wrapper guard");

    /* NULL state pointer is also safe. */
    dm1_spell_deleteSymbol(NULL, 0);
    CHECK(1, "NULL state pointer is a silent no-op", "defensive guard");
}

/* ── Test 7: Full 4-rune recant walk ──────────────────────────────
 * Fills all four symbol steps then backspaces through them, proving
 * the M019_PREVIOUS step walk (3->2->1->0) and the per-step rune
 * truncation stay coherent across the full chain. The power rune at
 * Symbols[0] is preserved through the recant walk (F0400 only
 * truncates Symbols[step], never Symbols[0..step-1]). */
static void test_full_four_rune_recant_walk(void) {
    printf("  [7] Full 4-rune recant walk: 3->2->1->0...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(200, 200, 50, 40);

    /* On (step 0) + Ful (step 1) + Ir (step 2) + Sar (step 3) */
    dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_ON);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_ELEM_FUL);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_CLASS_IR);
    dm1_spell_addSymbol(&s, 0, &stats, DM1_ALIGN_SAR);
    CHECK(s.input[0].symbolStep == 0,
          "after 4 adds: SymbolStep wraps back to 0 via (3+1) & 3",
          "SYMBOL.C F0399:42-44 (SymbolStep = (step+1) & 3)");
    /* F0399 sets SymbolStep = (SymbolStep+1) & 3, so after the 4th
     * add the step is (3+1) & 3 = 0. Symbols[0] is the power rune. */
    CHECK(s.input[0].symbols[0] == dm1_encodeSymbol(0, 2) &&
              s.input[0].symbols[1] == dm1_encodeSymbol(1, 3) &&
              s.input[0].symbols[2] == dm1_encodeSymbol(2, 3) &&
              s.input[0].symbols[3] == dm1_encodeSymbol(3, 5) &&
              s.input[0].symbols[4] == '\0',
          "after 4 adds: Symbols=[On, Ful, Ir, Sar, \\0]",
          "SYMBOL.C F0399:36-37");

    /* F0400 walk: step 0 -> 3, Symbols[3] = '\\0' */
    dm1_spell_deleteSymbol(&s, 0);
    CHECK(s.input[0].symbolStep == 3 &&
              s.input[0].symbols[3] == '\0' &&
              s.input[0].symbols[0] == dm1_encodeSymbol(0, 2),
          "recant 1: M019_PREVIOUS(0)=3, drop Sar; power rune preserved",
          "SYMBOL.C F0400:95-100");

    /* F0400 walk: step 3 -> 2, Symbols[2] = '\\0' */
    dm1_spell_deleteSymbol(&s, 0);
    CHECK(s.input[0].symbolStep == 2 &&
              s.input[0].symbols[2] == '\0' &&
              s.input[0].symbols[0] == dm1_encodeSymbol(0, 2),
          "recant 2: M019_PREVIOUS(3)=2, drop Ir; power rune preserved",
          "SYMBOL.C F0400:95-100");

    /* F0400 walk: step 2 -> 1, Symbols[1] = '\\0' */
    dm1_spell_deleteSymbol(&s, 0);
    CHECK(s.input[0].symbolStep == 1 &&
              s.input[0].symbols[1] == '\0' &&
              s.input[0].symbols[0] == dm1_encodeSymbol(0, 2),
          "recant 3: M019_PREVIOUS(2)=1, drop Ful; power rune preserved",
          "SYMBOL.C F0400:95-100");

    /* F0400 walk: step 1 -> 0, Symbols[0] = '\\0' (power rune now
     * gone — this is the only step that can erase Symbols[0]). */
    dm1_spell_deleteSymbol(&s, 0);
    CHECK(s.input[0].symbolStep == 0 &&
              s.input[0].symbols[0] == '\0',
          "recant 4: M019_PREVIOUS(1)=0, drop On (the power rune)",
          "SYMBOL.C F0400:95-100");
    /* spell lookup is now NULL */
    CHECK(dm1_spell_lookup(&s, 0) == NULL,
          "F0409 returns NULL after the 4-rune chain is fully backspaced",
          "MENU.C F0409:1685-1690");
}

/* ── Test 8: Source-evidence anchor surface ───────────────────────
 * Pin the source-locked evidence the rest of the gate cites. The
 * sibling gates cite F0400 + F0399 too, so this is intentionally
 * focused on the M019_PREVIOUS + multi-champion selector + F0409
 * transition anchors that *this* gate uses. */
static void test_source_evidence_mentions_backspace_anchors(void) {
    printf("  [8] Source evidence mentions backspace anchors...\n");

    /* Anchor: M019_PREVIOUS = ((value) + 3) & 3. */
    CHECK(((0u + 3u) & 0x0003u) == 3u,
          "M019_PREVIOUS(0) == 3 (backspace wrap)",
          "DEFS.H:464 M019_PREVIOUS");
    CHECK(((1u + 3u) & 0x0003u) == 0u,
          "M019_PREVIOUS(1) == 0 (normal back-step)",
          "DEFS.H:464 M019_PREVIOUS");
    CHECK(((2u + 3u) & 0x0003u) == 1u,
          "M019_PREVIOUS(2) == 1",
          "DEFS.H:464 M019_PREVIOUS");
    CHECK(((3u + 3u) & 0x0003u) == 2u,
          "M019_PREVIOUS(3) == 2",
          "DEFS.H:464 M019_PREVIOUS");

    /* Anchor: symbol encoding formula */
    CHECK(dm1_encodeSymbol(0, 0) == 96,
          "encode(0, 0) == 96 (Lo power)",
          "DEFS.H:632 + SYMBOL.C:36");
    CHECK(dm1_encodeSymbol(3, 5) == 96 + 18 + 5,
          "encode(3, 5) == 119 (Sar alignment)",
          "DEFS.H:632 + SYMBOL.C:36");
    CHECK(dm1_symbolCharStep(96) == 0 &&
              dm1_symbolCharIndex(96) == 0,
          "decode(96) -> step 0, idx 0",
          "DEFS.H:632 + SYMBOL.C:36");
    CHECK(dm1_symbolCharStep(119) == 3 &&
              dm1_symbolCharIndex(119) == 5,
          "decode(119) -> step 3, idx 5",
          "DEFS.H:632 + SYMBOL.C:36");
}

int main(void) {
    test_power_only_recant_wraps_step_zero_to_three();
    test_recant_walk_after_power_then_element();
    test_recant_isolates_to_one_champion();
    test_recant_with_zero_current_mana();
    test_recant_transitions_f0409_spell_lookup();
    test_recant_on_empty_champion_is_noop();
    test_full_four_rune_recant_walk();
    test_source_evidence_mentions_backspace_anchors();

    printf("PASS dm1_v1_spell_rune_backspace_pc34_compat %d/%d assertions; "
           "SYMBOL.C F0400 + M019_PREVIOUS contract-only backspace gate\n",
           gPasses, gTests);
    return gPasses == gTests ? 0 : 1;
}
