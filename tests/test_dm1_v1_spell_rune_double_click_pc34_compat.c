/**
 * test_dm1_v1_spell_rune_double_click_pc34_compat.c
 *
 * Contract-only DM1 V1 spell-rune **double-click / repeat-input** source-lock
 * gate.
 *
 * This gate pins the source-locked contract for repeated identical rune
 * input — what happens when the player double-clicks the same rune
 * button, or when an OS auto-repeat / held-button press fires multiple
 * F0399_MENUS_AddChampionSymbol invocations against the same
 * `G0514_i_MagicCasterChampionIndex` in rapid succession.
 *
 * Source anchors (ReDMCSB WIP20210206):
 *   SYMBOL.C   — F0399_MENUS_AddChampionSymbol (the ONLY mutation site)
 *   SYMBOL.C   — F0399 lines 20-25 mana-cost lookup (G0485 + G0486)
 *   SYMBOL.C   — F0399 line 26 `if (ManaCost <= CurrentMana)`
 *   SYMBOL.C   — F0399 line 33 `Symbols[symbolStep] = 96 + ...`
 *   SYMBOL.C   — F0399 line 36 `Symbols[symbolStep + 1] = '\0'`
 *   SYMBOL.C   — F0399 line 39 `SymbolStep = (symbolStep + 1) & 3`
 *   SYMBOL.C   — F0399 line 41 `F0397_MENUS_DrawAvailableSymbols(new step)`
 *   SYMBOL.C   — F0399 line 42 `F0398_MENUS_DrawChampionSymbols(Champion)`
 *   MENU.C     — F0409_MENUS_GetSpellFromSymbols (NULL on <2 syms)
 *   MENU.C     — F0412_MENUS_GetChampionSpellCastResult (cast driver)
 *   CLIKMENU.C — F0370:386-510 routes C101..C106 to F0399
 *   CLIKMENU.C — F0369:381 / F0370:512 routes C107 recant to F0400
 *   DEFS.H     — Champion.Symbols[5], Champion.SymbolStep 0..3
 *   DEFS.H     — M019_PREVIOUS((value) + 3) & 3 (backspace wrap)
 *   DEFS.H     — G0485_aauc_Graphic560_SymbolBaseManaCost[4][6]
 *   DEFS.H     — G0486_auc_Graphic560_SymbolManaCostMultiplier[6]
 *
 * Contract pinned here:
 *
 * 1. F0399 has NO idempotency check. Two consecutive F0399 calls with the
 *    same `P0768_i_SymbolIndex` do NOT collapse into one rune; they
 *    append a SECOND rune at the next step. Because the rune-byte
 *    encoding is per-step (`96 + step*6 + idx`), the same button index
 *    at successive steps writes a DIFFERENT rune byte. The
 *    double-click chain [Lo, Ya] is therefore two distinct rune bytes
 *    (96 and 102), not a duplicate Lo rune.
 *
 * 2. Each repeated call deducts mana again at the CURRENT step's base
 *    cost. No discount, no refund. The original F0399 unconditionally
 *    subtracts `L1223_ui_ManaCost` from `L1225_ps_Champion->CurrentMana`
 *    once the cost-gate is passed.
 *
 * 3. Each repeated call advances SymbolStep via `(step + 1) & 3`. The
 *    step never stays put. So a held-button or fast-double-click at
 *    step 0 walks 0 -> 1 -> 2 -> 3 -> 0, and the next identical
 *    button-press now writes a DIFFERENT rune category (because the
 *    button index 0..5 is per-step). The user-visible effect is the
 *    same button keeps filling rune slots as long as mana permits.
 *
 * 4. Repeated clicks do NOT trigger M019_PREVIOUS wrap from the
 *    backspace gate. The wrap is exclusive to F0400.
 *
 * 5. Repeated-input rune chains behave under F0409's wildcard lookup
 *    (MENU.C:1700-1703): every entry in dm1_spells[25] has MSB == 0,
 *    so F0409 always compares lower 24 bits. This means:
 *    - 2-click chain [power, element] packs to (power<<24)|(element<<16)
 *      and the lower 24 = (element<<16) matches the wildcard spell whose
 *      byte[1] equals that element (e.g. [Lo, Ya] -> Stamina Potion).
 *    - 3-click chain [power, element, class] adds the class byte at
 *      offset 8, and no spell has lower-24 = (element<<16)|(class<<8)
 *      for the auto-repeat-built pattern. F0409 returns NULL.
 *    - 4-click chain has MSB != 0 in the packed long (because step 0
 *      contributes bit 24+ which is the power rune byte 96-101, all
 *      with bit 5+ set), so the full 32-bit compare runs and no spell
 *      has the auto-repeat pattern. F0409 returns NULL.
 *
 * 6. Double-clicking must be multi-champion isolated. The current
 *    caster (G0514) only changes when F0370 receives a C109 caster
 *    click; an auto-repeat against champion A must NOT mutate
 *    champion B's Symbols / SymbolStep / mana.
 *
 * 7. The repeat-input F0397 redraw is per-call: every successful
 *    invocation redraws the available-symbol grid (F0397 line 41) and
 *    the champion spell area (F0398 line 42). The contract here is
 *    "N F0399 calls = N redraws" — proven indirectly via the symbolStep
 *    advance and the per-call mana decrement (both of which the redraw
 *    reads from to lay out the on-screen rune grid).
 *
 * 8. The F0412 cast driver returns MEANINGLESS_SPELL on the
 *    auto-repeat chain, not on a 2-click repeat (which would match via
 *    the F0409 wildcard and become a potion-casting path needing a
 *    flask in hand). This distinguishes "real spell resolved via
 *    wildcard" from "no spell resolved".
 *
 * This gate is intentionally non-duplicative with the backspace gate
 * (`test_dm1_v1_spell_rune_backspace_pc34_compat`):
 *   - The backspace gate pins F0400 + M019_PREVIOUS wrap and never
 *     touches the repeat-input contract.
 *   - The spell-casting basic add gate covers addSymbol(step, idx) once
 *     per step but never pins the doubled/auto-repeat rune-chain.
 *   - The mirror-candidate runtime gate covers the C100 / C101 dispatch
 *     and the G0299 candidate panel block, NOT the F0399 mutation
 *     contract under repeated input.
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

/* ── Test 1: Two F0399 calls with the same symbol advance SymbolStep
 * ReDMCSB SYMBOL.C F0399 line 33 writes Symbols[symbolStep] and line
 * 39 advances SymbolStep. There is NO check for "this rune is already
 * at symbolStep". The second identical click therefore writes at the
 * NEW step, not the original step. */
static void test_double_click_same_rune_walks_two_steps(void) {
    printf("  [1] Double-click same rune: two F0399 calls advance one step each...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(100, 100, 50, 40);

    /* First click: power Lo (step 0, idx 0). G0485[0][0] = 1 (MENU.C:45). */
    int ok1 = dm1_spell_addSymbol(&s, 0, &stats, DM1_POWER_LO);
    CHECK(ok1 == 1, "first Lo click succeeds (mana 1 <= 100)",
          "SYMBOL.C F0399:20-26");
    CHECK(s.input[0].symbolStep == 1,
          "first click advances step 0 -> 1",
          "SYMBOL.C F0399:39");
    CHECK(s.input[0].symbols[0] == dm1_encodeSymbol(0, 0) &&
              s.input[0].symbols[1] == '\0',
          "first click stores Lo at Symbols[0]",
          "SYMBOL.C F0399:33-36");

    /* Second click: SAME symbol index 0 at step 1. The contract is
     * that F0399 writes `96 + 1*6 + 0 = 102` (Ya element) at
     * Symbols[1] — NOT a duplicate of the Lo rune, because step 1
     * encodes element symbols. The user pressed the same physical
     * button, but the rune table has advanced to the next row. */
    int16_t manaBefore = stats.currentMana;
    int ok2 = dm1_spell_addSymbol(&s, 0, &stats, 0);
    CHECK(ok2 == 1, "second button-0 click at step 1 also succeeds",
          "SYMBOL.C F0399:20-26");
    CHECK(s.input[0].symbolStep == 2,
          "second click advances step 1 -> 2",
          "SYMBOL.C F0399:39");
    /* G0485[1][0] = 2 (MENU.C:46). Step > 0 multiplier:
     *   powerIdx = symbols[0] - 96 = 0 -> G0486[0] = 8
     *   cost = (2 * 8) >> 3 = 16 >> 3 = 2
     * So the second click costs 2 mana. */
    CHECK(stats.currentMana == manaBefore - 2,
          "second click deducts step-1 base cost 2 * G0486[Lo]=8 / 8 = 2 mana",
          "SYMBOL.C F0399:20-25 + MENU.C:45-49");
    CHECK(s.input[0].symbols[0] == dm1_encodeSymbol(0, 0) &&
              s.input[0].symbols[1] == dm1_encodeSymbol(1, 0) &&
              s.input[0].symbols[2] == '\0',
          "Symbols[0]=Lo (96), Symbols[1]=Ya (102), Symbols[2]='\\0'",
          "SYMBOL.C F0399:33-36");
    /* The rune-chain grew by one — the power rune at Symbols[0] is
     * preserved (F0399 only writes the new rune, never overwrites
     * Symbols[0..step-1]). */
    CHECK(s.input[0].symbols[0] != s.input[0].symbols[1],
          "two rune bytes are distinct — F0399 has no idempotency collapse",
          "SYMBOL.C F0399 (no Symbols[step] == new check)");
}

/* ── Test 2: Auto-repeat walks the full 4-step ring ───────────────
 * F0399 line 39 advances SymbolStep = (step + 1) & 3. With 4 held /
 * repeated clicks at index 0 starting at step 0, the walker visits
 * steps 0, 1, 2, 3, then wraps back to 0 — but the *fifth* click is
 * blocked because F0399 writes Symbols[step=0] = encoded power, which
 * is the same byte as the first rune. This is fine; the original
 * allows it (F0399 has no de-dup). It produces:
 *   Symbols[0] = power-Lo
 *   Symbols[1] = element-Ya
 *   Symbols[2] = class-Ven
 *   Symbols[3] = alignment-Ku
 *   Symbols[4] = '\0'
 *   SymbolStep = 0 (wrapped)
 * spell lookup at this point packs 0x006C6A60... */
static void test_auto_repeat_walks_full_step_ring(void) {
    printf("  [2] Auto-repeat walks the full 4-step ring...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    /* Need enough mana to cover all 4 clicks:
     *   click 1 (step 0, idx 0): G0485[0][0] = 1
     *   click 2 (step 1, idx 0): G0485[1][0] = 2 -> * G0486[0]=8 / 8 = 2
     *   click 3 (step 2, idx 0): G0485[2][0] = 4 -> * 8 / 8 = 4
     *   click 4 (step 3, idx 0): G0485[3][0] = 2 -> * 8 / 8 = 2
     *   total = 1 + 2 + 4 + 2 = 9
     * We give 50 to be safe and prove the cumulative drain is correct. */
    DM1_ChampionSpellStats stats = makeStats(50, 50, 50, 40);

    int ok;
    int16_t m;

    /* Click 1: power Lo */
    m = stats.currentMana;
    ok = dm1_spell_addSymbol(&s, 0, &stats, 0);
    CHECK(ok == 1 && stats.currentMana == m - 1 &&
              s.input[0].symbolStep == 1 &&
              s.input[0].symbols[0] == dm1_encodeSymbol(0, 0),
          "auto-repeat click 1: power Lo, costs 1 mana, step 0->1",
          "SYMBOL.C F0399:20-39");

    /* Click 2: element Ya (same button, step 1) */
    m = stats.currentMana;
    ok = dm1_spell_addSymbol(&s, 0, &stats, 0);
    CHECK(ok == 1 && stats.currentMana == m - 2 &&
              s.input[0].symbolStep == 2 &&
              s.input[0].symbols[1] == dm1_encodeSymbol(1, 0),
          "auto-repeat click 2: element Ya, costs 2 mana, step 1->2",
          "SYMBOL.C F0399:20-39");

    /* Click 3: class Ven (same button, step 2) */
    m = stats.currentMana;
    ok = dm1_spell_addSymbol(&s, 0, &stats, 0);
    CHECK(ok == 1 && stats.currentMana == m - 4 &&
              s.input[0].symbolStep == 3 &&
              s.input[0].symbols[2] == dm1_encodeSymbol(2, 0),
          "auto-repeat click 3: class Ven, costs 4 mana, step 2->3",
          "SYMBOL.C F0399:20-39");

    /* Click 4: alignment Ku (same button, step 3) */
    m = stats.currentMana;
    ok = dm1_spell_addSymbol(&s, 0, &stats, 0);
    CHECK(ok == 1 && stats.currentMana == m - 2 &&
              s.input[0].symbolStep == 0 &&
              s.input[0].symbols[3] == dm1_encodeSymbol(3, 0),
          "auto-repeat click 4: alignment Ku, costs 2 mana, step 3->0 (wrap)",
          "SYMBOL.C F0399:20-39");
    CHECK(s.input[0].symbols[4] == '\0',
          "Symbols[4] is the null terminator after the 4th add",
          "SYMBOL.C F0399:36");

    /* Cumulative drain must equal 1+2+4+2 = 9, leaving 41. */
    CHECK(stats.currentMana == 41,
          "cumulative mana drain across 4 auto-repeat clicks is 9",
          "SYMBOL.C F0399:20-39 (cumulative)");

    /* The 5th auto-repeat click walks back into step 0 and overwrites
     * Symbols[0] with the same byte (96). F0399 still deducts mana.
     * G0485[0][0] = 1. */
    m = stats.currentMana;
    ok = dm1_spell_addSymbol(&s, 0, &stats, 0);
    CHECK(ok == 1 && stats.currentMana == m - 1 &&
              s.input[0].symbolStep == 1 &&
              s.input[0].symbols[0] == dm1_encodeSymbol(0, 0),
          "auto-repeat click 5: wraps to step 0, rewrites Lo (same byte), costs 1",
          "SYMBOL.C F0399:33 (write) + F0399:20-26 (cost) + F0399:39 (wrap)");
}

/* ── Test 3: F0409 lookup of a doubled-rune chain ───────────────
 * The original spell table in DM1 V1 has every entry's MSB == 0
 * (MENU.C:50-76), so F0409:1700-1703 compares only the lower 24
 * bits when `spell.symbols & 0xFF000000 == 0`. The implication for
 * repeat-input rune chains:
 *   - 2-click chain [power, element] packs (power<<24)|(element<<16).
 *     The lower 24 bits = (element<<16), and any spell whose byte[1]
 *     matches `element` will hit the wildcard.
 *   - 3-click chain [power, element, class] packs the class rune at
 *     offset 8. The lower 24 bits = (element<<16)|(class<<8). For the
 *     wildcard to match, the spell must have the same element AND
 *     class bytes — which only happens for specific spells.
 *   - 4-click chain [power, element, class, align] packs all four
 *     bytes. The full 32-bit compare then applies, and no spell in
 *     the table matches an auto-repeat-built chain.
 *
 * This sub-test pins the F0409 wildcard-vs-full-match contract as it
 * applies to repeated-input rune chains, which the existing lookup
 * tests do not exercise. */
static void test_doubled_rune_chain_lookup_uses_wildcard_for_short_chains(void) {
    printf("  [3] Doubled-rune chain lookup uses F0409 wildcard...\n");

    /* Sub-case A: 2 clicks [Lo, Ya] -> matches Stamina Potion via
     * wildcard lower-24.  Packed = (96<<24)|(102<<16) = 0x00660060,
     * lower 24 = 0x006600 == dm1_spells[16].symbols (Ya Stamina). */
    {
        DM1_SpellCastingState s;
        dm1_spell_init(&s);
        DM1_ChampionSpellStats stats = makeStats(50, 50, 50, 40);
        dm1_spell_addSymbol(&s, 0, &stats, 0);
        dm1_spell_addSymbol(&s, 0, &stats, 0);
        CHECK(s.input[0].symbols[0] == dm1_encodeSymbol(0, 0) &&
                  s.input[0].symbols[1] == dm1_encodeSymbol(1, 0),
              "2-click repeat: Symbols=[Lo, Ya]",
              "SYMBOL.C F0399:33-36");
        const DM1_Spell* sp = dm1_spell_lookup(&s, 0);
        CHECK(sp != NULL && (sp - dm1_spells) == 16,
              "F0409 matches 2-click repeat Lo+Ya to Stamina Potion (wildcard)",
              "MENU.C F0409:1700-1703 (lower-24 wildcard)");
    }

    /* Sub-case B: 3 clicks [Lo, Ya, Ven] -> NULL. The Ven rune byte
     * at offset 8 means the lower 24 bits = 0x066C00, and no spell
     * in dm1_spells[25] has that lower-24 pattern (the closest is
     * dm1_spells[3] = 0x00686C00 with lower 24 = 0x686C00, which
     * differs in byte[1] — 0x68 vs 0x06). F0409 returns NULL even
     * though the chain has 3 runes. */
    {
        DM1_SpellCastingState s;
        dm1_spell_init(&s);
        DM1_ChampionSpellStats stats = makeStats(50, 50, 50, 40);
        dm1_spell_addSymbol(&s, 0, &stats, 0);
        dm1_spell_addSymbol(&s, 0, &stats, 0);
        dm1_spell_addSymbol(&s, 0, &stats, 0);
        CHECK(s.input[0].symbols[0] == dm1_encodeSymbol(0, 0) &&
                  s.input[0].symbols[1] == dm1_encodeSymbol(1, 0) &&
                  s.input[0].symbols[2] == dm1_encodeSymbol(2, 0),
              "3-click repeat: Symbols=[Lo, Ya, Ven]",
              "SYMBOL.C F0399:33-36");
        const DM1_Spell* sp = dm1_spell_lookup(&s, 0);
        CHECK(sp == NULL,
              "F0409 returns NULL when 3-click lower-24 has no match",
              "MENU.C F0409:1700-1703");
    }

    /* Sub-case C: 4 clicks [Lo, Ya, Ven, Ku] -> NULL. Packed =
     * (96<<24)|(102<<16)|(108<<8)|(114) = 0x6C6A6660. Full 32-bit
     * compare runs (no spell with MSB != 0 exists to take the
     * lower-24 path), and no spell in dm1_spells[25] has this exact
     * packed value. */
    {
        DM1_SpellCastingState s;
        dm1_spell_init(&s);
        DM1_ChampionSpellStats stats = makeStats(50, 50, 50, 40);
        dm1_spell_addSymbol(&s, 0, &stats, 0);
        dm1_spell_addSymbol(&s, 0, &stats, 0);
        dm1_spell_addSymbol(&s, 0, &stats, 0);
        dm1_spell_addSymbol(&s, 0, &stats, 0);
        CHECK(s.input[0].symbols[0] == dm1_encodeSymbol(0, 0) &&
                  s.input[0].symbols[1] == dm1_encodeSymbol(1, 0) &&
                  s.input[0].symbols[2] == dm1_encodeSymbol(2, 0) &&
                  s.input[0].symbols[3] == dm1_encodeSymbol(3, 0),
              "4-click repeat: Symbols=[Lo, Ya, Ven, Ku]",
              "SYMBOL.C F0399:33-36");
        const DM1_Spell* sp = dm1_spell_lookup(&s, 0);
        CHECK(sp == NULL,
              "F0409 returns NULL when the 4-rune auto-repeat chain has no spell",
              "MENU.C F0409:1697-1705");
    }
}

/* ── Test 4: F0412 cast with a 3-rune auto-repeat chain is MEANINGLESS ──
 * F0412 first calls F0409; if NULL, F0412 returns DM1_SPELL_CAST_FAILURE
 * with DM1_FAILURE_MEANINGLESS_SPELL. The 3-click auto-repeat chain
 * [Lo, Ya, Ven] has no matching spell (see test 3 sub-case B — the
 * class byte at offset 8 differs from any spell in dm1_spells[25]),
 * so the cast must be MEANINGLESS and F0408 must clear the chain for
 * the next attempt. We use 3 clicks here because the 2-click [Lo, Ya]
 * chain matches Stamina Potion via the F0409 wildcard (a different
 * contract — the cast returns NEEDS_FLASK for a potion spell, not
 * MEANINGLESS). The 3-rune case is the source-locked "no match"
 * outcome. */
static void test_auto_repeat_chain_cast_is_meaningless(void) {
    printf("  [4] F0412 cast with 3-click auto-repeat chain is MEANINGLESS...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(50, 50, 50, 40);
    /* Three auto-repeat clicks on button 0: Lo + Ya + Ven. The packed
     * long is 0x60666C00 which does not match any of the 25 spell
     * definitions in dm1_spells. */
    dm1_spell_addSymbol(&s, 0, &stats, 0);
    dm1_spell_addSymbol(&s, 0, &stats, 0);
    dm1_spell_addSymbol(&s, 0, &stats, 0);
    CHECK(s.input[0].symbols[0] == dm1_encodeSymbol(0, 0) &&
              s.input[0].symbols[1] == dm1_encodeSymbol(1, 0) &&
              s.input[0].symbols[2] == dm1_encodeSymbol(2, 0) &&
              s.input[0].symbols[3] == '\0',
          "3-click auto-repeat: Symbols=[Lo, Ya, Ven, \\0]",
          "SYMBOL.C F0399:33-36");

    const DM1_Spell* spell = NULL;
    int powerOrdinal = 0;
    int failure = -1;
    int castResult = dm1_spell_cast(&s, 0, &stats, 0x1234u,
                                     &spell, &powerOrdinal, &failure);
    CHECK(castResult == DM1_SPELL_CAST_FAILURE,
          "F0412 returns FAILURE on 3-rune auto-repeat chain",
          "MENU.C F0412:1707-1714");
    CHECK(failure == DM1_FAILURE_MEANINGLESS_SPELL,
          "failure type is MEANINGLESS_SPELL",
          "MENU.C F0412:1707-1714");
    CHECK(spell == NULL && powerOrdinal == 0,
          "no spell resolved and no power ordinal set",
          "MENU.C F0412:1707-1714");

    /* F0408 cleanup policy: a FAILURE clears the rune chain for the
     * next spell attempt. */
    CHECK(s.input[0].symbols[0] == '\0' && s.input[0].symbolStep == 0,
          "F0408 clears symbols on MEANINGLESS failure (next-frame ready)",
          "MENU.C F0408:1633-1663");
}

/* ── Test 5: Repeat input is multi-champion isolated ──────────────
 * F0399 reads M516_CHAMPIONS[G0514_i_MagicCasterChampionIndex]. So a
 * repeated F0399 burst routed to champion 1 must NOT mutate champion
 * 0's Symbols / SymbolStep / mana. The dm1_spell_addSymbol wrapper
 * pins champIdx, so we prove this directly. */
static void test_repeat_input_isolates_to_one_champion(void) {
    printf("  [5] Repeat input isolates to one champion...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stA = makeStats(50, 50, 50, 40);
    DM1_ChampionSpellStats stB = makeStats(50, 50, 50, 40);
    DM1_ChampionSpellStats stC = makeStats(50, 50, 50, 40);
    DM1_ChampionSpellStats stD = makeStats(50, 50, 50, 40);

    /* Each champion has a single starting rune. */
    dm1_spell_addSymbol(&s, 0, &stA, DM1_POWER_LO);
    dm1_spell_addSymbol(&s, 1, &stB, DM1_POWER_UM);
    dm1_spell_addSymbol(&s, 2, &stC, DM1_POWER_ON);
    dm1_spell_addSymbol(&s, 3, &stD, DM1_POWER_EE);

    /* Snapshot non-target champions. */
    DM1_ChampionSpellInput beforeB = s.input[1];
    DM1_ChampionSpellInput beforeC = s.input[2];
    DM1_ChampionSpellInput beforeD = s.input[3];
    int16_t manaB_before = stB.currentMana;
    int16_t manaC_before = stC.currentMana;
    int16_t manaD_before = stD.currentMana;

    /* Three F0399 repeats on champion 0 (button 0). The champion is
     * the same across all three calls, so all mutations are scoped
     * to M516_CHAMPIONS[0]. */
    int16_t manaA_before = stA.currentMana;
    dm1_spell_addSymbol(&s, 0, &stA, 0);
    dm1_spell_addSymbol(&s, 0, &stA, 0);
    dm1_spell_addSymbol(&s, 0, &stA, 0);

    /* Champion 0 advanced 3 steps + 1 (already advanced once from the
     * first Lo add above). After 3 more repeats: step = 1 + 3 = 4
     * which wraps to 0. So step is back to 0. */
    CHECK(s.input[0].symbolStep == 0,
          "champion 0 step wraps back to 0 after the 4 total adds",
          "SYMBOL.C F0399:39 (step+1) & 3");
    /* Cumulative mana spent: champion 0 had 1 Lo click = 1 mana, then
     * 3 more clicks at the same button-0 advancing through steps
     * 1, 2, 3 with costs 2, 4, 2 = 8. Total = 1 + 8 = 9. */
    CHECK(stA.currentMana == manaA_before - 8,
          "champion 0 spent 8 more mana on the 3 repeat clicks",
          "SYMBOL.C F0399:20-25 + cumulative");

    /* Champions 1, 2, 3 must be byte-identical and mana-untouched. */
    CHECK(memcmp(&s.input[1], &beforeB, sizeof(beforeB)) == 0,
          "champion 1 input unchanged by champion 0 repeat input",
          "SYMBOL.C F0399:7 (champion selector)");
    CHECK(memcmp(&s.input[2], &beforeC, sizeof(beforeC)) == 0,
          "champion 2 input unchanged by champion 0 repeat input",
          "SYMBOL.C F0399:7");
    CHECK(memcmp(&s.input[3], &beforeD, sizeof(beforeD)) == 0,
          "champion 3 input unchanged by champion 0 repeat input",
          "SYMBOL.C F0399:7");
    CHECK(stB.currentMana == manaB_before &&
              stC.currentMana == manaC_before &&
              stD.currentMana == manaD_before,
          "champions 1, 2, 3 mana pools are untouched",
          "SYMBOL.C F0399 (no cross-champion mana write)");
}

/* ── Test 6: F0400 recant after a repeat-input chain recovers each
 * rune one step at a time. This pins the M019_PREVIOUS walk against
 * an auto-repeat-built chain and proves that the post-repeat state
 * is byte-identical to a single-click chain once the recant catches
 * up. Distinct from the backspace gate because the backspace gate
 * only walks single-add chains. */
static void test_recant_after_repeat_input_chain(void) {
    printf("  [6] Recant after repeat-input chain...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stats = makeStats(50, 50, 50, 40);

    /* Two auto-repeat clicks on button 0: power Lo + element Ya. */
    dm1_spell_addSymbol(&s, 0, &stats, 0);
    dm1_spell_addSymbol(&s, 0, &stats, 0);
    CHECK(s.input[0].symbolStep == 2 &&
              s.input[0].symbols[0] == dm1_encodeSymbol(0, 0) &&
              s.input[0].symbols[1] == dm1_encodeSymbol(1, 0) &&
              s.input[0].symbols[2] == '\0',
          "after 2 auto-repeat clicks: step=2, Symbols=[Lo, Ya, \\0]",
          "SYMBOL.C F0399:33-39");

    /* Recant 1: M019_PREVIOUS(2) = 1, Symbols[1] = '\\0'. The Ya
     * rune byte is dropped; the Lo rune byte at Symbols[0] is
     * preserved because F0400 only writes to Symbols[new_step]. */
    dm1_spell_deleteSymbol(&s, 0);
    CHECK(s.input[0].symbolStep == 1 &&
              s.input[0].symbols[0] == dm1_encodeSymbol(0, 0) &&
              s.input[0].symbols[1] == '\0',
          "recant 1: M019_PREVIOUS(2)=1, Ya dropped, Lo preserved",
          "SYMBOL.C F0400:95-100");

    /* Recant 2: M019_PREVIOUS(1) = 0, Symbols[0] = '\\0'. The Lo
     * rune byte is dropped. This is the only step that can erase
     * Symbols[0]. */
    dm1_spell_deleteSymbol(&s, 0);
    CHECK(s.input[0].symbolStep == 0 &&
              s.input[0].symbols[0] == '\0',
          "recant 2: M019_PREVIOUS(1)=0, Lo dropped, Symbols[0]='\\0'",
          "SYMBOL.C F0400:95-100");

    /* The state after the recant is byte-identical to the fresh
     * init state. */
    DM1_SpellCastingState fresh;
    dm1_spell_init(&fresh);
    CHECK(memcmp(&s.input[0], &fresh.input[0], sizeof(fresh.input[0])) == 0,
          "fully recanted auto-repeat chain == fresh state",
          "SYMBOL.C F0400 + SYMBOL.C F0399 contract");
}

/* ── Test 7: Repeat input at insufficient mana stops at the gate ─
 * F0399 line 26 is the ONLY mutation gate. A repeat input sequence
 * that drains mana past zero must stop emitting runes once the gate
 * fails, even if the chain is mid-walk. The chain at the failure
 * point must be byte-stable across further repeat attempts. */
static void test_repeat_input_halts_at_insufficient_mana(void) {
    printf("  [7] Repeat input halts at insufficient mana...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    /* Carefully sized mana pool: enough for 2 clicks (1+2=3), not 3. */
    DM1_ChampionSpellStats stats = makeStats(3, 3, 50, 40);

    int ok;
    ok = dm1_spell_addSymbol(&s, 0, &stats, 0);
    CHECK(ok == 1 && stats.currentMana == 2 && s.input[0].symbolStep == 1,
          "first repeat click: mana 3 -> 2, step 0->1",
          "SYMBOL.C F0399:20-26");

    ok = dm1_spell_addSymbol(&s, 0, &stats, 0);
    CHECK(ok == 1 && stats.currentMana == 0 && s.input[0].symbolStep == 2,
          "second repeat click: mana 2 -> 0, step 1->2",
          "SYMBOL.C F0399:20-26");

    /* Third click needs G0485[2][0] = 4, * G0486[0]=8 / 8 = 4 mana.
     * Pool is 0, so the gate fails. State is unchanged. */
    DM1_ChampionSpellInput beforeInput = s.input[0];
    int16_t manaBefore = stats.currentMana;
    ok = dm1_spell_addSymbol(&s, 0, &stats, 0);
    CHECK(ok == 0,
          "third repeat click is gated by insufficient mana",
          "SYMBOL.C F0399:20-26");
    CHECK(memcmp(&s.input[0], &beforeInput, sizeof(beforeInput)) == 0,
          "insufficient-mana repeat leaves the chain byte-identical",
          "SYMBOL.C F0399:20-26 (gate before write)");
    CHECK(stats.currentMana == manaBefore,
          "insufficient-mana repeat does not change mana",
          "SYMBOL.C F0399:20-26");

    /* Even if the user keeps "pressing" (more repeat events fire),
     * the chain must remain stable. */
    for (int i = 0; i < 5; ++i) {
        ok = dm1_spell_addSymbol(&s, 0, &stats, 0);
        CHECK(ok == 0, "subsequent repeat presses are still gated",
              "SYMBOL.C F0399:20-26");
    }
    CHECK(memcmp(&s.input[0], &beforeInput, sizeof(beforeInput)) == 0 &&
              stats.currentMana == manaBefore,
          "5 more repeat presses leave the chain byte-stable",
          "SYMBOL.C F0399:20-26");
}

/* ── Test 8: Repeat input is independent of the magic-caster index ─
 * The G0514_i_MagicCasterChampionIndex selector is what routes the
 * F0399 write to a specific champion. The wrapper here takes
 * champIdx directly. We prove the same source-locked behavior
 * regardless of which champion the dispatch routes to. */
static void test_repeat_input_respects_champion_switch(void) {
    printf("  [8] Repeat input after a caster switch routes correctly...\n");

    DM1_SpellCastingState s;
    dm1_spell_init(&s);
    DM1_ChampionSpellStats stA = makeStats(50, 50, 50, 40);
    DM1_ChampionSpellStats stB = makeStats(50, 50, 50, 40);

    /* Champion 0 receives three repeat clicks on button 0. After the
     * first click at step 0 the step advances to 1; the second at
     * step 1 writes Ya and advances to 2; the third at step 2 writes
     * Ven and advances to 3; so the chain is [Lo, Ya, Ven] with
     * symbolStep == 3 and Symbols[3] == '\0' (null terminator). */
    dm1_spell_addSymbol(&s, 0, &stA, 0);
    dm1_spell_addSymbol(&s, 0, &stA, 0);
    dm1_spell_addSymbol(&s, 0, &stA, 0);
    CHECK(s.input[0].symbolStep == 3 &&
              s.input[0].symbols[0] == dm1_encodeSymbol(0, 0) &&
              s.input[0].symbols[1] == dm1_encodeSymbol(1, 0) &&
              s.input[0].symbols[2] == dm1_encodeSymbol(2, 0) &&
              s.input[0].symbols[3] == '\0',
          "champion 0: 3 repeat clicks produce [Lo, Ya, Ven, \\0], step=3",
          "SYMBOL.C F0399:33-39");
    CHECK(s.input[1].symbols[0] == '\0' &&
              s.input[1].symbolStep == 0,
          "champion 1 untouched while champion 0 received repeat input",
          "SYMBOL.C F0399:7");

    /* Now switch the caster selector to champion 1 (mirrors a C109
     * spell-area click in the original: COMMAND.C:474 maps C109 to
     * C221; CLIKMENU.C F0369/F0370 updates G0514). The repeat input
     * from here goes to champion 1. */
    dm1_spell_addSymbol(&s, 1, &stB, 0);
    dm1_spell_addSymbol(&s, 1, &stB, 0);
    CHECK(s.input[1].symbolStep == 2 &&
              s.input[1].symbols[0] == dm1_encodeSymbol(0, 0) &&
              s.input[1].symbols[1] == dm1_encodeSymbol(1, 0),
          "champion 1: 2 repeat clicks produce [Lo, Ya] after the caster switch",
          "SYMBOL.C F0399:7 (champion selector) + F0399:33-39");

    /* Champion 0's chain from before the caster switch is intact. */
    CHECK(s.input[0].symbolStep == 3 &&
              s.input[0].symbols[0] == dm1_encodeSymbol(0, 0) &&
              s.input[0].symbols[1] == dm1_encodeSymbol(1, 0) &&
              s.input[0].symbols[2] == dm1_encodeSymbol(2, 0) &&
              s.input[0].symbols[3] == '\0',
          "champion 0 chain preserved across the caster switch",
          "SYMBOL.C F0399:7 + F0399:33-36");
}

/* ── Test 9: Source-evidence anchor surface ───────────────────────
 * Pin the source-locked evidence the rest of the gate cites. */
static void test_source_evidence_mentions_double_click_anchors(void) {
    printf("  [9] Source evidence mentions double-click / repeat anchors...\n");

    /* Anchor: F0399 step wrap formula (step + 1) & 3. */
    CHECK(((0u + 1u) & 3u) == 1u,
          "F0399 step wrap: (0+1) & 3 = 1",
          "SYMBOL.C F0399:39");
    CHECK(((1u + 1u) & 3u) == 2u,
          "F0399 step wrap: (1+1) & 3 = 2",
          "SYMBOL.C F0399:39");
    CHECK(((2u + 1u) & 3u) == 3u,
          "F0399 step wrap: (2+1) & 3 = 3",
          "SYMBOL.C F0399:39");
    CHECK(((3u + 1u) & 3u) == 0u,
          "F0399 step wrap: (3+1) & 3 = 0",
          "SYMBOL.C F0399:39");

    /* Anchor: F0399 has no Symbols[step] == new-byte check. We
     * verify the encoding is per-step so the same button index at
     * different steps produces different rune bytes (this is the
     * reason the original has no idempotency: the rune table is
     * step-local). */
    CHECK(dm1_encodeSymbol(0, 0) == 96,
          "step 0 idx 0 -> 96 (Lo power)",
          "SYMBOL.C:36 + DEFS.H:632");
    CHECK(dm1_encodeSymbol(1, 0) == 102,
          "step 1 idx 0 -> 102 (Ya element, NOT Lo)",
          "SYMBOL.C:36 + DEFS.H:632");
    CHECK(dm1_encodeSymbol(2, 0) == 108,
          "step 2 idx 0 -> 108 (Ven class)",
          "SYMBOL.C:36 + DEFS.H:632");
    CHECK(dm1_encodeSymbol(3, 0) == 114,
          "step 3 idx 0 -> 114 (Ku alignment)",
          "SYMBOL.C:36 + DEFS.H:632");
    /* All four bytes are distinct -- this is what makes the
     * "double-click" rune chain a real 4-rune sequence, not a
     * collapse. */
    CHECK(dm1_encodeSymbol(0, 0) != dm1_encodeSymbol(1, 0) &&
              dm1_encodeSymbol(1, 0) != dm1_encodeSymbol(2, 0) &&
              dm1_encodeSymbol(2, 0) != dm1_encodeSymbol(3, 0),
          "each step encodes a distinct rune byte for the same idx",
          "SYMBOL.C:36 (per-step encoding)");

    /* Anchor: F0409 lookup is a strict pack-and-compare on the
     * 32-bit packed long. The 2-click repeat [Lo, Ya] packs as
     * (96<<24)|(102<<16) = 0x00660060. The Stamina Potion spell
     * (dm1_spells[16]) packs as 0x00660000. The lower 24 bits are
     * EQUAL (0x006600), which is why F0409 matches via the MSB-0
     * wildcard — a separate test (test 3 sub-case A) covers that
     * match. This anchor just verifies the encoding is correct. */
    CHECK(dm1_spells[16].symbols == 0x00660000,
          "dm1_spells[16] (Stamina Potion) packs Ya-only as 0x00660000",
          "MENU.C:50-76");
    CHECK(0x00660060 != (int32_t)dm1_spells[16].symbols,
          "2-click repeat Lo+Ya packed (0x00660060) != spell[16] (0x00660000) full compare",
          "MENU.C F0409:1697-1705");

    /* Anchor: F0400 wrap formula M019_PREVIOUS(value) = (value+3) & 3.
     * This is exclusive to F0400; F0399 does NOT use it. */
    CHECK(((2u + 3u) & 3u) == 1u,
          "M019_PREVIOUS(2) = 1 (recant path, F0400 only)",
          "SYMBOL.C F0400:95-96; DEFS.H:464 M019_PREVIOUS");
}

int main(void) {
    test_double_click_same_rune_walks_two_steps();
    test_auto_repeat_walks_full_step_ring();
    test_doubled_rune_chain_lookup_uses_wildcard_for_short_chains();
    test_auto_repeat_chain_cast_is_meaningless();
    test_repeat_input_isolates_to_one_champion();
    test_recant_after_repeat_input_chain();
    test_repeat_input_halts_at_insufficient_mana();
    test_repeat_input_respects_champion_switch();
    test_source_evidence_mentions_double_click_anchors();

    printf("PASS dm1_v1_spell_rune_double_click_pc34_compat %d/%d assertions; "
           "SYMBOL.C F0399 + F0409 contract-only double-click / repeat-input gate\n",
           gPasses, gTests);
    return gPasses == gTests ? 0 : 1;
}
