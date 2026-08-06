
#include "nexus_v1_magic.h"
#include "nexus_v1_combat.h"
#include <stdio.h>
#include <string.h>

static int g_failures;

static void expect(int cond, const char *msg) {
    if (!cond) { fprintf(stderr, "FAIL: %s\n", msg); ++g_failures; }
}

int main(void) {
    Nexus_SpellLookup sp;
    Nexus_V1_Champion ch;
    int cost;

    nexus_v1_combat_seed(42);

    /* Spell table lookups — real DM.BIN 0x038368.
     * Index: (element * 4 + form) * 2 + class.
     * YA=0 VI=1 OH=2 FUL=3, GOR=0 KATH=1 IR=2 BRO=3, PRIEST=0 WIZARD=1. */

    sp = nexus_v1_spell_lookup(0, NEXUS_ELEM_YA, NEXUS_FORM_GOR, NEXUS_SPELL_CLASS_PRIEST);
    expect(!sp.valid, "YA+GOR priest is invalid (0xFFFF)");

    sp = nexus_v1_spell_lookup(0, NEXUS_ELEM_YA, NEXUS_FORM_BRO, NEXUS_SPELL_CLASS_PRIEST);
    expect(sp.valid, "YA+BRO priest is valid");
    expect(sp.spell_type == 0, "YA+BRO priest = type 0");

    sp = nexus_v1_spell_lookup(0, NEXUS_ELEM_FUL, NEXUS_FORM_GOR, NEXUS_SPELL_CLASS_PRIEST);
    expect(sp.valid, "FUL+GOR priest is valid");
    expect(sp.spell_type == 10, "FUL+GOR priest = type 10");

    sp = nexus_v1_spell_lookup(0, NEXUS_ELEM_FUL, NEXUS_FORM_IR, NEXUS_SPELL_CLASS_WIZARD);
    expect(sp.valid, "FUL+IR wizard is valid");
    expect(sp.spell_type == 2, "FUL+IR wizard = type 2");

    /* VI element lookups */
    sp = nexus_v1_spell_lookup(0, NEXUS_ELEM_VI, NEXUS_FORM_GOR, NEXUS_SPELL_CLASS_PRIEST);
    expect(sp.valid, "VI+GOR priest is valid");
    expect(sp.spell_type == 0, "VI+GOR priest = type 0");

    sp = nexus_v1_spell_lookup(0, NEXUS_ELEM_VI, NEXUS_FORM_GOR, NEXUS_SPELL_CLASS_WIZARD);
    expect(sp.valid, "VI+GOR wizard is valid");
    expect(sp.spell_type == 1, "VI+GOR wizard = type 1");

    /* Out of bounds */
    sp = nexus_v1_spell_lookup(0, -1, 0, NEXUS_SPELL_CLASS_PRIEST);
    expect(!sp.valid, "negative element invalid");
    sp = nexus_v1_spell_lookup(0, 0, 4, NEXUS_SPELL_CLASS_PRIEST);
    expect(!sp.valid, "form >= 4 invalid");

    /* Mana cost — DM.BIN 0x0601ABC0 formula: 50*power + 25 */
    expect(nexus_v1_spell_mana_cost(0, 0) == 25, "power 0 cost = 25");
    expect(nexus_v1_spell_mana_cost(5, 0) == 275, "power 5 cost = 275");
    expect(nexus_v1_spell_mana_cost(6, 0) == 999, "power 6 invalid");

    /* Cast is intentionally blocked: the DM.BIN spell table is real, but no
     * Saturn dispatcher receipt proves the host-side mana/effect mutation. */
    memset(&ch, 0, sizeof(ch));
    snprintf(ch.name_ascii, sizeof(ch.name_ascii), "Mage");
    ch.alive = 1;
    ch.mana = 500;
    ch.wizard_level = 5;
    ch.priest_level = 3;

    cost = nexus_v1_cast_spell(&ch, 0, NEXUS_ELEM_YA, NEXUS_FORM_BRO, 0);
    expect(cost == -1, "cast remains blocked without Saturn dispatcher receipt");
    expect(ch.mana == 500, "blocked cast does not consume mana");

    /* Cast with insufficient mana */
    ch.mana = 1;
    cost = nexus_v1_cast_spell(&ch, 5, NEXUS_ELEM_FUL, NEXUS_FORM_IR, 0);
    expect(cost == -1, "cast fails with insufficient mana");

    /* Null safety */
    cost = nexus_v1_cast_spell(NULL, 0, 0, 0, 0);
    expect(cost == -1, "null caster returns -1");

    if (g_failures) {
        fprintf(stderr, "test_nexus_v1_magic: %d failure(s)\n", g_failures);
        return 1;
    }
    puts("ok: Nexus magic system with real DM.BIN spell table verified");
    return 0;
}
