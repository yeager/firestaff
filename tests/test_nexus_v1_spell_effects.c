
#include <stdio.h>
#include <string.h>
#include "nexus_v1_spell_effects.h"

int main(void) {
    int fail = 0;

    /* Test 1: heal effect */
    {
        Nexus_V1_Champion ch;
        Nexus_SpellEffectResult r;
        memset(&ch, 0, sizeof(ch));
        ch.alive = 1;
        ch.health = 50;
        ch.max_health = 100;
        r = nexus_v1_spell_effect_party(NEXUS_SPELL_EFFECT_HEAL, 2,
                                         &ch, NULL, NULL);
        if (!r.applied || ch.health != 80) {
            fprintf(stderr, "FAIL: heal hp=%d\n", ch.health); fail++;
        } else {
            printf("  Heal: 50->%d OK\n", ch.health);
        }
    }

    /* Test 2: heal clamps to max */
    {
        Nexus_V1_Champion ch;
        memset(&ch, 0, sizeof(ch));
        ch.alive = 1;
        ch.health = 95;
        ch.max_health = 100;
        nexus_v1_spell_effect_party(NEXUS_SPELL_EFFECT_HEAL, 5, &ch, NULL, NULL);
        if (ch.health != 100) {
            fprintf(stderr, "FAIL: heal clamp hp=%d\n", ch.health); fail++;
        } else {
            printf("  Heal clamp: 95->100 OK\n");
        }
    }

    /* Test 3: shield applies status */
    {
        Nexus_V1_Champion ch;
        Nexus_StatusEffects se;
        Nexus_SpellEffectResult r;
        memset(&ch, 0, sizeof(ch));
        ch.alive = 1;
        nexus_v1_status_init(&se);
        r = nexus_v1_spell_effect_party(NEXUS_SPELL_EFFECT_SHIELD, 1,
                                         &ch, &se, NULL);
        if (!r.applied || !r.status_applied ||
            !nexus_v1_status_is_active(&se, NEXUS_STATUS_SHIELD)) {
            fprintf(stderr, "FAIL: shield\n"); fail++;
        } else {
            printf("  Shield status applied OK\n");
        }
    }

    /* Test 4: light spell increases light */
    {
        Nexus_V1_Champion ch;
        Nexus_LightState ls;
        Nexus_SpellEffectResult r;
        memset(&ch, 0, sizeof(ch));
        ch.alive = 1;
        nexus_v1_light_init(&ls);
        r = nexus_v1_spell_effect_party(NEXUS_SPELL_EFFECT_LIGHT, 2,
                                         &ch, NULL, &ls);
        if (!r.applied || r.light_added != 9) {
            fprintf(stderr, "FAIL: light added=%d\n", r.light_added); fail++;
        } else {
            printf("  Light spell: +%d OK\n", r.light_added);
        }
    }

    /* Test 5: fireball projectile */
    {
        enum Nexus_ProjectileType pt;
        int dmg;
        int ok = nexus_v1_spell_effect_attack_projectile(
            NEXUS_SPELL_EFFECT_FIREBALL, 3, &pt, &dmg);
        if (!ok || pt != NEXUS_PROJ_FIREBALL || dmg != 60) {
            fprintf(stderr, "FAIL: fireball type=%d dmg=%d\n", pt, dmg); fail++;
        } else {
            printf("  Fireball projectile: type=%d dmg=%d OK\n", pt, dmg);
        }
    }

    /* Test 6: lightning projectile higher damage */
    {
        enum Nexus_ProjectileType pt;
        int dmg;
        nexus_v1_spell_effect_attack_projectile(
            NEXUS_SPELL_EFFECT_LIGHTNING, 3, &pt, &dmg);
        if (pt != NEXUS_PROJ_LIGHTNING || dmg != 80) {
            fprintf(stderr, "FAIL: lightning type=%d dmg=%d\n", pt, dmg); fail++;
        } else {
            printf("  Lightning projectile: type=%d dmg=%d OK\n", pt, dmg);
        }
    }

    /* Test 7: confuse debuff */
    {
        Nexus_StatusEffects se;
        Nexus_SpellEffectResult r;
        nexus_v1_status_init(&se);
        r = nexus_v1_spell_effect_debuff(NEXUS_SPELL_EFFECT_CONFUSE, 2, &se);
        if (!r.applied || !nexus_v1_status_is_active(&se, NEXUS_STATUS_CONFUSION)) {
            fprintf(stderr, "FAIL: confuse debuff\n"); fail++;
        } else {
            printf("  Confuse debuff OK\n");
        }
    }

    /* Test 8: dispel clears poison */
    {
        Nexus_V1_Champion ch;
        Nexus_StatusEffects se;
        memset(&ch, 0, sizeof(ch));
        ch.alive = 1;
        nexus_v1_status_init(&se);
        nexus_v1_status_apply(&se, NEXUS_STATUS_POISON, 100, 10);
        nexus_v1_spell_effect_party(NEXUS_SPELL_EFFECT_DISPEL, 0, &ch, &se, NULL);
        if (nexus_v1_status_is_active(&se, NEXUS_STATUS_POISON)) {
            fprintf(stderr, "FAIL: dispel poison\n"); fail++;
        } else {
            printf("  Dispel clears poison OK\n");
        }
    }

    /* Test 9: NULL safety */
    {
        Nexus_SpellEffectResult r;
        r = nexus_v1_spell_effect_party(NEXUS_SPELL_EFFECT_HEAL, 0, NULL, NULL, NULL);
        if (r.applied) {
            fprintf(stderr, "FAIL: NULL caster\n"); fail++;
        }
        r = nexus_v1_spell_effect_debuff(NEXUS_SPELL_EFFECT_CONFUSE, 0, NULL);
        if (r.applied) {
            fprintf(stderr, "FAIL: NULL target\n"); fail++;
        }
        if (nexus_v1_spell_effect_attack_projectile(NEXUS_SPELL_EFFECT_FIREBALL, 0, NULL, NULL)) {
            fprintf(stderr, "FAIL: NULL out params\n"); fail++;
        }
        printf("  NULL safety OK\n");
    }

    if (fail) {
        fprintf(stderr, "%d failures\n", fail);
        return 1;
    }
    printf("ok: Nexus spell effects verified\n");
    return 0;
}
