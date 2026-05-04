#ifndef FIRESTAFF_DM1_V1_COMBAT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_COMBAT_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_COMBAT_UNARMED = 0,
    DM1_COMBAT_MELEE,
    DM1_COMBAT_RANGED,
    DM1_COMBAT_MAGIC,
    DM1_COMBAT_TYPE_COUNT
};

enum {
    DM1_TARGET_NONE = -1,
    DM1_TARGET_PLAYER = -2,
    DM1_TARGET_CLOSEST = -3,
    DM1_TARGET_FARTHEST = -4
};

typedef struct {
    int type;
    int damageMin;
    int damageMax;
    int attackSpeed;
    int range;
    int accuracy;
} M11_Weapon;

typedef struct {
    M11_Weapon weapon;
    int health;
    int maxHealth;
    int alive;
    int x;
    int y;
    int floorIdx;
    int facingDir;
} M11_Combatant;

#define M11_MAX_ENTITIES 128

typedef struct {
    M11_Combatant entities[M11_MAX_ENTITIES];
    int entityCount;
} M11_CombatState;

void m11_combat_init(M11_CombatState* s);
int m11_combat_add_entity(M11_CombatState* s, const M11_Combatant* c);
int m11_combat_remove(M11_CombatState* s, int idx);
int m11_combat_distance(const M11_CombatState* s, int a, int b);
int m11_combat_attack(M11_CombatState* s, int attacker, int target);
void m11_combat_tick(M11_CombatState* s, int tickMs);
int m11_combat_find_target(const M11_CombatState* s, int attacker);
int m11_combat_hit_check(M11_Combatant* attacker);
int m11_combat_damage(const M11_Weapon* weapon);
void m11_combant_apply_damage(M11_Combatant* target, int dmg);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_COMBAT_PC34_COMPAT_H */