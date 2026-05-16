
#ifndef FIRESTAFF_CSB_V1_MONSTER_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_MONSTER_PC34_COMPAT_H

#include <stdint.h>

/* CSB V1 Monster System
 *
 * CSB shares DM1's creature system but adds:
 * - More creature types
 * - DSA-scripted creature behaviors
 * - Different attack patterns
 * - Chaos-specific creature spawning
 *
 * Source: CSBWin/Monster.cpp (4819 lines)
 * Source: CSBWin/Attack.cpp (2431 lines)
 * Base: ReDMCSB GROUP.C (shared creature core)
 */

typedef struct {
    int type;
    int health;
    int attack;
    int defense;
    int speed;
    int poison_attack;
    int see_invisible;
    int dsa_script_id;  /* CSB-specific: linked DSA script */
} CSB_V1_MonsterDef;

typedef struct {
    int attacker_champion;
    int target_creature;
    int damage;
    int attack_type;
    int weapon_type;
    int skill_index;
} CSB_V1_AttackResult;

int csb_v1_monster_get_defense(const CSB_V1_MonsterDef *m, int attack_type);
int csb_v1_attack_resolve(const CSB_V1_AttackResult *attack, const CSB_V1_MonsterDef *target);
const char *csb_v1_monster_source_evidence(void);

#endif

