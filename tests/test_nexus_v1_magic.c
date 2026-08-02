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

    /* Verify spell table lookups match DM.BIN 0x038368 */
    sp = nexus_v1_spell_lookup(0, NEXUS_ELEM_YA, NEXUS_FORM_GOR, NEXUS_SPELL_CLASS_PRIEST);
    expect(sp.valid, "YA+GOR priest is valid");
    expect(sp.spell_type == 0, "YA+GOR priest = type 0");

    sp = nexus_v1_spell_lookup(0, NEXUS_ELEM_YA, NEXUS_FORM_GOR, NEXUS_SPELL_CLASS_WIZARD);
    expect(sp.valid, "YA+GOR wizard is valid");
    expect(sp.spell_type == 1, "YA+GOR wizard = type 1");

    sp = nexus_v1_spell_lookup(0, NEXUS_ELEM_YA, NEXUS_FORM_BRO, NEXUS_SPELL_CLASS_PRIEST);
    expect(!sp.valid, "YA+BRO priest is invalid (0xFFFF)");

    sp = nexus_v1_spell_lookup(0, NEXUS_ELEM_YA, NEXUS_FORM_BRO, NEXUS_SPELL_CLASS_WIZARD);
    expect(sp.valid, "YA+BRO wizard is valid");
    expect(sp.spell_type == 7, "YA+BRO wizard = type 7");

    sp = nexus_v1_spell_lookup(0, NEXUS_ELEM_FUL, NEXUS_FORM_GOR, NEXUS_SPELL_CLASS_PRIEST);
    expect(sp.valid && sp.spell_type == 10, "FUL+GOR priest = type 10");

    sp = nexus_v1_spell_lookup(0, NEXUS_ELEM_FUL, NEXUS_FORM_GOR, NEXUS_SPELL_CLASS_WIZARD);
    expect(sp.valid && sp.spell_type == 12, "FUL+GOR wizard = type 12");

    /* Out of bounds */
    sp = nexus_v1_spell_lookup(0, -1, 0, NEXUS_SPELL_CLASS_PRIEST);
    expect(!sp.valid, "negative element invalid");
    sp = nexus_v1_spell_lookup(0, 0, 4, NEXUS_SPELL_CLASS_PRIEST);
    expect(!sp.valid, "form >= 4 invalid");

    /* Mana cost from parameter tables */
    expect(nexus_v1_spell_mana_cost(0, 0) == 4, "power 0 cost = 4");
    expect(nexus_v1_spell_mana_cost(5, 0) == 24, "power 5 cost = 24");
    expect(nexus_v1_spell_mana_cost(6, 0) == 999, "power 6 invalid");

    /* Cast spell */
    memset(&ch, 0, sizeof(ch));
    snprintf(ch.name_ascii, sizeof(ch.name_ascii), "Mage");
    ch.alive = 1;
    ch.mana = 100;
    ch.wizard_level = 5;
    ch.priest_level = 3;

    cost = nexus_v1_cast_spell(&ch, 0, NEXUS_ELEM_YA, NEXUS_FORM_GOR, 0);
    expect(cost > 0, "cast succeeds with enough mana/skill");
    expect(ch.mana < 100, "mana consumed");

    /* Cast with insufficient mana */
    ch.mana = 1;
    cost = nexus_v1_cast_spell(&ch, 5, NEXUS_ELEM_FUL, NEXUS_FORM_IR, 0);
    expect(cost == -1, "cast fails with insufficient mana");

    /* Cast invalid combo */
    ch.mana = 100;
    cost = nexus_v1_cast_spell(&ch, 0, NEXUS_ELEM_YA, NEXUS_FORM_BRO, 0);
    expect(cost > 0, "YA+BRO succeeds via wizard fallback");

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
