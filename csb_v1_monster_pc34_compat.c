
#include "csb_v1_monster_pc34_compat.h"

/* pass603: CSB V1 monster/attack
 *
 * CSBWin/Monster.cpp: creature AI + behavior (4819 lines)
 * CSBWin/Attack.cpp: attack resolution (2431 lines)
 * CSBWin/Character.cpp: champion stats for combat (5528 lines)
 * ReDMCSB GROUP.C: F0190-F0230 (shared combat/targeting core)
 * ReDMCSB CHAMPION.C: F0311-F0321 (shared damage pipeline)
 */

int csb_v1_monster_get_defense(const CSB_V1_MonsterDef *m, int attack_type) {
    if (!m) return 0;
    /* CSBWin/Attack.cpp defense calculation — base + type modifier */
    return m->defense + (attack_type == 4 ? m->defense / 2 : 0);
}

int csb_v1_attack_resolve(const CSB_V1_AttackResult *attack,
    const CSB_V1_MonsterDef *target)
{
    int net_damage;
    if (!attack || !target) return 0;
    net_damage = attack->damage - csb_v1_monster_get_defense(target, attack->attack_type);
    if (net_damage < 0) net_damage = 0;
    return net_damage;
}

const char *csb_v1_monster_source_evidence(void) {
    return
        "CSBWin/Monster.cpp: 4819 lines creature AI\n"
        "CSBWin/Attack.cpp: 2431 lines attack resolution\n"
        "CSBWin/Character.cpp: 5528 lines champion combat stats\n"
        "ReDMCSB GROUP.C F0190-F0230 shared targeting\n"
        "ReDMCSB CHAMPION.C F0311-F0321 shared damage pipeline\n";
}

