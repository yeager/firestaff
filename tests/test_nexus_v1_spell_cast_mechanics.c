
#include <stdio.h>
#include <string.h>
#include "nexus_v1_magic.h"
#include "nexus_v1_mechanics.h"
#include "nexus_v1_creatures.h"

int main(void) {
    int fail = 0;

    /* Test 1: spell lookup table — FUL+IR wizard = fireball (0x0005) */
    {
        Nexus_SpellLookup sp = nexus_v1_spell_lookup(
            NEXUS_RUNE_ON, NEXUS_ELEM_FUL, NEXUS_FORM_IR,
            NEXUS_SPELL_CLASS_WIZARD);
        if (!sp.valid || sp.spell_type != NEXUS_SPELL_EFFECT_DARKNESS) {
            fprintf(stderr, "FAIL: FUL+IR wizard lookup: valid=%d type=0x%04X\n",
                    sp.valid, sp.spell_type);
            fail++;
        } else {
            printf("  FUL+IR wizard: spell_type=0x%04X cost=%d OK\n",
                   sp.spell_type, sp.mana_cost);
        }
    }

    /* Test 2: spell lookup — YA+GOR priest = heal (0x0000) */
    {
        Nexus_SpellLookup sp = nexus_v1_spell_lookup(
            NEXUS_RUNE_LO, NEXUS_ELEM_YA, NEXUS_FORM_GOR,
            NEXUS_SPELL_CLASS_PRIEST);
        if (!sp.valid || sp.spell_type != NEXUS_SPELL_EFFECT_HEAL) {
            fprintf(stderr, "FAIL: YA+GOR priest lookup\n");
            fail++;
        } else {
            printf("  YA+GOR priest: spell_type=0x%04X cost=%d OK\n",
                   sp.spell_type, sp.mana_cost);
        }
    }

    /* Test 3: invalid combination — YA+BRO priest = 0xFFFF */
    {
        Nexus_SpellLookup sp = nexus_v1_spell_lookup(
            NEXUS_RUNE_LO, NEXUS_ELEM_YA, NEXUS_FORM_BRO,
            NEXUS_SPELL_CLASS_PRIEST);
        if (sp.valid) {
            fprintf(stderr, "FAIL: YA+BRO priest should be invalid\n");
            fail++;
        } else {
            printf("  YA+BRO priest: correctly invalid OK\n");
        }
    }

    /* Test 4: spell damage table */
    {
        int d0 = nexus_v1_spell_damage(0, NEXUS_SPELL_CLASS_PRIEST);
        int d5 = nexus_v1_spell_damage(5, NEXUS_SPELL_CLASS_WIZARD);
        if (d0 != 6 || d5 != 46) {
            fprintf(stderr, "FAIL: spell damage: d0=%d (exp 6) d5=%d (exp 46)\n", d0, d5);
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
        ch.mana = 100;
        ch.priest_level = 3;
        ch.wizard_level = 5;
        result = nexus_v1_cast_spell(&ch, NEXUS_RUNE_ON, NEXUS_ELEM_FUL,
                                     NEXUS_FORM_IR, 0);
        if (result <= 0) {
            fprintf(stderr, "FAIL: cast spell returned %d (expected >0 damage)\n", result);
            fail++;
        } else if (ch.mana >= 100) {
            fprintf(stderr, "FAIL: mana not deducted (still %d)\n", ch.mana);
            fail++;
        } else {
            printf("  Cast FUL+IR: damage=%d mana=%d/%d OK\n", result, ch.mana, 100);
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

    if (fail) {
        fprintf(stderr, "%d failures\n", fail);
        return 1;
    }
    printf("ok: Nexus spell casting + mechanics integration verified\n");
    return 0;
}
