
#include <stdio.h>
#include <string.h>
#include "nexus_v1_magic.h"
#include "nexus_v1_mechanics.h"
#include "nexus_v1_creatures.h"

int main(void) {
    int fail = 0;

    /* Test 1: spell lookup — FUL+IR wizard = type 2 (DM.BIN 0x038368) */
    {
        Nexus_SpellLookup sp = nexus_v1_spell_lookup(
            NEXUS_RUNE_ON, NEXUS_ELEM_FUL, NEXUS_FORM_IR,
            NEXUS_SPELL_CLASS_WIZARD);
        if (!sp.valid || sp.spell_type != 2) {
            fprintf(stderr, "FAIL: FUL+IR wizard lookup: valid=%d type=0x%04X\n",
                    sp.valid, sp.spell_type);
            fail++;
        } else {
            printf("  FUL+IR wizard: spell_type=0x%04X cost=%d OK\n",
                   sp.spell_type, sp.mana_cost);
        }
    }

    /* Test 2: YA+GOR priest = 0xFFFF (invalid in real DM.BIN table) */
    {
        Nexus_SpellLookup sp = nexus_v1_spell_lookup(
            NEXUS_RUNE_LO, NEXUS_ELEM_YA, NEXUS_FORM_GOR,
            NEXUS_SPELL_CLASS_PRIEST);
        if (sp.valid) {
            fprintf(stderr, "FAIL: YA+GOR priest should be invalid\n");
            fail++;
        } else {
            printf("  YA+GOR priest: correctly invalid OK\n");
        }
    }

    /* Test 3: YA+BRO priest = type 0 (valid in real DM.BIN table) */
    {
        Nexus_SpellLookup sp = nexus_v1_spell_lookup(
            NEXUS_RUNE_LO, NEXUS_ELEM_YA, NEXUS_FORM_BRO,
            NEXUS_SPELL_CLASS_PRIEST);
        if (!sp.valid || sp.spell_type != 0) {
            fprintf(stderr, "FAIL: YA+BRO priest lookup: valid=%d type=0x%04X\n",
                    sp.valid, sp.spell_type);
            fail++;
        } else {
            printf("  YA+BRO priest: spell_type=0x%04X OK\n", sp.spell_type);
        }
    }

    /* Test 4: spell damage — DM.BIN 0x03B5DC magnitude table */
    {
        int d0 = nexus_v1_spell_damage(0, NEXUS_SPELL_CLASS_PRIEST);
        int d5 = nexus_v1_spell_damage(5, NEXUS_SPELL_CLASS_WIZARD);
        if (d0 != 40 || d5 != 240) {
            fprintf(stderr, "FAIL: spell damage: d0=%d (exp 40) d5=%d (exp 240)\n", d0, d5);
            fail++;
        } else {
            printf("  Spell damage: LO priest=%d, MON wizard=%d OK\n", d0, d5);
        }
    }

    /* Test 5: cast spell deducts mana and returns damage */
    {
        Nexus_V1_Champion ch;
        int result;
        memset(&ch, 0, sizeof(ch));
        ch.alive = 1;
        ch.mana = 500;
        ch.priest_level = 3;
        ch.wizard_level = 5;
        result = nexus_v1_cast_spell(&ch, NEXUS_RUNE_ON, NEXUS_ELEM_FUL,
                                     NEXUS_FORM_IR, 0);
        if (result <= 0) {
            fprintf(stderr, "FAIL: cast spell returned %d (expected >0 damage)\n", result);
            fail++;
        } else if (ch.mana >= 500) {
            fprintf(stderr, "FAIL: mana not deducted (still %d)\n", ch.mana);
            fail++;
        } else {
            printf("  Cast FUL+IR: damage=%d mana=%d/%d OK\n", result, ch.mana, 500);
        }
    }

    /* Test 6: mechanics spell rune state */
    {
        Nexus_MechanicsState ms;
        nexus_mechanics_init(&ms, 10, 10, 0);
        if (ms.spell_power != -1 || ms.spell_element != -1) {
            fprintf(stderr, "FAIL: spell state not cleared on init\n");
            fail++;
        }
        nexus_mechanics_set_spell_runes(&ms, 2, 3, 1, 0);
        if (ms.spell_power != 2 || ms.spell_element != 3 ||
            ms.spell_form != 1 || ms.spell_align != 0) {
            fprintf(stderr, "FAIL: spell runes not set\n");
            fail++;
        }
        nexus_mechanics_clear_spell(&ms);
        if (ms.spell_power != -1) {
            fprintf(stderr, "FAIL: spell not cleared\n");
            fail++;
        }
        printf("  Mechanics spell state: init/set/clear OK\n");
    }

    /* Test 7: creature damage_at */
    {
        Nexus_V1_CreatureManager mgr;
        int killed;
        nexus_v1_creatures_init(&mgr);
        nexus_v1_creature_spawn(&mgr, 0, 5, 5, 0);
        mgr.active[0].health = 10;
        mgr.active[0].alive = 1;
        killed = nexus_v1_creature_manager_damage_at(&mgr, 5, 5, 15);
        if (killed != 1 || mgr.active[0].alive != 0) {
            fprintf(stderr, "FAIL: damage_at killed=%d alive=%d\n",
                    killed, mgr.active[0].alive);
            fail++;
        } else {
            printf("  creature_manager_damage_at: killed=%d OK\n", killed);
        }
    }

    /* Test 8: spell category classification */
    {
        if (nexus_v1_spell_category(NEXUS_SPELL_EFFECT_HEAL) != NEXUS_SPELL_CAT_PARTY ||
            nexus_v1_spell_category(NEXUS_SPELL_EFFECT_SHIELD) != NEXUS_SPELL_CAT_PARTY ||
            nexus_v1_spell_category(NEXUS_SPELL_EFFECT_LIGHT) != NEXUS_SPELL_CAT_PARTY ||
            nexus_v1_spell_category(NEXUS_SPELL_EFFECT_STRENGTH) != NEXUS_SPELL_CAT_PARTY) {
            fprintf(stderr, "FAIL: party spell categories\n"); fail++;
        } else if (nexus_v1_spell_category(NEXUS_SPELL_EFFECT_FIREBALL) != NEXUS_SPELL_CAT_ATTACK ||
                   nexus_v1_spell_category(NEXUS_SPELL_EFFECT_LIGHTNING) != NEXUS_SPELL_CAT_ATTACK ||
                   nexus_v1_spell_category(NEXUS_SPELL_EFFECT_POISON) != NEXUS_SPELL_CAT_ATTACK) {
            fprintf(stderr, "FAIL: attack spell categories\n"); fail++;
        } else if (nexus_v1_spell_category(NEXUS_SPELL_EFFECT_DARKNESS) != NEXUS_SPELL_CAT_DEBUFF ||
                   nexus_v1_spell_category(NEXUS_SPELL_EFFECT_CONFUSE) != NEXUS_SPELL_CAT_DEBUFF) {
            fprintf(stderr, "FAIL: debuff spell categories\n"); fail++;
        } else {
            printf("  Spell categories: 4 party, 3 attack, 6 debuff OK\n");
        }
    }

    if (fail) {
        fprintf(stderr, "%d failures\n", fail);
        return 1;
    }
    printf("ok: Nexus spell casting + mechanics integration verified\n");
    return 0;
}
