#include "dm1_v1_combat_pc34_compat.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

void m11_combat_init(M11_CombatState* s) {
    if (!s) return;
    memset(s, 0, sizeof(M11_CombatState));
}

int m11_combat_add_entity(M11_CombatState* s, const M11_Combatant* c) {
    if (!s || !c) return -1;
    if (s->entityCount >= M11_MAX_ENTITIES) return -1;
    
    s->entities[s->entityCount] = *c;
    return s->entityCount++;
}

int m11_combat_remove(M11_CombatState* s, int idx) {
    if (!s) return -1;
    if (idx < 0 || idx >= s->entityCount) return -1;
    
    // Shift entities down
    for (int i = idx; i < s->entityCount - 1; i++) {
        s->entities[i] = s->entities[i + 1];
    }
    s->entityCount--;
    return 0;
}

int m11_combat_distance(const M11_CombatState* s, int a, int b) {
    if (!s) return 0;
    if (a < 0 || a >= s->entityCount) return 0;
    if (b < 0 || b >= s->entityCount) return 0;
    
    const M11_Combatant* entA = &s->entities[a];
    const M11_Combatant* entB = &s->entities[b];
    
    int dx = abs(entA->x - entB->x);
    int dy = abs(entA->y - entB->y);
    int dz = abs(entA->floorIdx - entB->floorIdx);
    
    // Weighted distance: floor difference is significant
    return dx + dy + (dz * 10);
}

int m11_combat_attack(M11_CombatState* s, int attacker, int target) {
    if (!s) return 0;
    if (attacker < 0 || attacker >= s->entityCount) return 0;
    if (target < 0 || target >= s->entityCount) return 0;
    
    M11_Combatant* att = &s->entities[attacker];
    M11_Combatant* tgt = &s->entities[target];
    
    if (!att->alive || !tgt->alive) return 0;
    
    // Check cooldown
    if (att->weapon.attackSpeed > 0) return 0;
    
    // Check range
    int dist = m11_combat_distance(s, attacker, target);
    if (dist > att->weapon.range) return 0;
    
    // Hit check
    if (!m11_combat_hit_check(att)) return 0;
    
    // Calculate damage
    int dmg = m11_combat_damage(&att->weapon);
    
    // Apply damage
    m11_combant_apply_damage(tgt, dmg);
    
    // Reset attack speed (set to max for cooldown)
    att->weapon.attackSpeed = att->weapon.attackSpeed > 0 ? att->weapon.attackSpeed : 1;
    
    return 1;
}

void m11_combat_tick(M11_CombatState* s, int tickMs) {
    if (!s) return;
    
    for (int i = 0; i < s->entityCount; i++) {
        M11_Combatant* ent = &s->entities[i];
        if (ent->alive && ent->weapon.attackSpeed > 0) {
            ent->weapon.attackSpeed--;
        }
    }
}

int m11_combat_find_target(const M11_CombatState* s, int attacker) {
    if (!s) return DM1_TARGET_NONE;
    if (attacker < 0 || attacker >= s->entityCount) return DM1_TARGET_NONE;
    
    const M11_Combatant* att = &s->entities[attacker];
    if (!att->alive) return DM1_TARGET_NONE;
    
    int bestTarget = DM1_TARGET_NONE;
    int bestDist = -1;
    
    for (int i = 0; i < s->entityCount; i++) {
        if (i == attacker) continue;
        
        const M11_Combatant* ent = &s->entities[i];
        if (!ent->alive) continue;
        
        int dist = m11_combat_distance(s, attacker, i);
        
        if (bestTarget == DM1_TARGET_NONE || dist < bestDist) {
            bestTarget = i;
            bestDist = dist;
        }
    }
    
    return bestTarget;
}

int m11_combat_hit_check(M11_Combatant* attacker) {
    if (!attacker) return 0;
    
    int accuracy = attacker->weapon.accuracy;
    if (accuracy <= 0) return 0;
    if (accuracy >= 100) return 1;
    
    return (rand() % 100) < accuracy;
}

int m11_combat_damage(const M11_Weapon* weapon) {
    if (!weapon) return 0;
    
    int minDmg = weapon->damageMin;
    int maxDmg = weapon->damageMax;
    
    if (minDmg == maxDmg) return minDmg;
    if (minDmg > maxDmg) {
        int temp = minDmg;
        minDmg = maxDmg;
        maxDmg = temp;
    }
    
    return minDmg + (rand() % (maxDmg - minDmg + 1));
}

void m11_combant_apply_damage(M11_Combatant* target, int dmg) {
    if (!target) return;
    
    target->health -= dmg;
    if (target->health <= 0) {
        target->health = 0;
        target->alive = 0;
    }
}