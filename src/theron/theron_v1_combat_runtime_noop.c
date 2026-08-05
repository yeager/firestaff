/*
 * theron_v1_combat_runtime_noop.c — production admission boundary
 *
 * The former theron_v1_compat.c contains inferred creature templates and
 * combat constants.  Those values are retained only in fixture targets.
 * Until Track 02 creature records are decoded, the production runtime must
 * expose the API without inventing a creature, attack, drop, or stat result.
 */

#include "theron_v1_combat.h"
#include "theron_v1_world.h"

int theron_v1_creature_spawn(Theron_V1_World *world,
                             Theron_CreatureType type,
                             int dungeon_id, int level, int x, int y) {
    (void)world; (void)type; (void)dungeon_id; (void)level; (void)x; (void)y;
    return -1;
}

int theron_v1_creature_kill(Theron_V1_World *world, int creature_id) {
    (void)world; (void)creature_id; return -1;
}

int theron_v1_creature_remove(Theron_V1_World *world, int creature_id) {
    (void)world; (void)creature_id; return -1;
}

Theron_V1_Creature *theron_v1_creature_at(Theron_V1_World *world,
                                           int level, int x, int y) {
    (void)world; (void)level; (void)x; (void)y; return NULL;
}

Theron_V1_Creature *theron_v1_creature_by_id(Theron_V1_World *world, int id) {
    (void)world; (void)id; return NULL;
}

int theron_v1_creature_count(const Theron_V1_World *world,
                             int dungeon_id, int level) {
    (void)world; (void)dungeon_id; (void)level; return 0;
}

int theron_v1_champion_attack(Theron_V1_World *world,
                              int attacking_slot, int target_creature_id) {
    (void)world; (void)attacking_slot; (void)target_creature_id; return -1;
}

int theron_v1_champion_cast_spell(Theron_V1_World *world,
                                  int casting_slot,
                                  unsigned int spell_index,
                                  int target_creature_id) {
    (void)world;
    (void)casting_slot;
    (void)spell_index;
    (void)target_creature_id;
    return -1;
}

Theron_CombatResult theron_v1_creature_attack_champion(
    Theron_V1_World *world, int creature_id, int champion_slot) {
    (void)world; (void)creature_id; (void)champion_slot;
    return THERON_COMBAT_NONE;
}

void theron_v1_creature_ai_tick(Theron_V1_World *world) { (void)world; }

int theron_v1_calc_attack_damage(int attack_power,
                                 const Theron_V1_Champion *defender,
                                 Theron_AttackType type) {
    (void)attack_power; (void)defender; (void)type; return -1;
}

int theron_v1_calc_defense(const Theron_V1_Champion *defender,
                           Theron_AttackType type) {
    (void)defender; (void)type; return -1;
}

int theron_v1_modify_champion_hp(Theron_V1_Champion *c, int delta) {
    (void)c; (void)delta; return 0;
}

int theron_v1_modify_champion_stamina(Theron_V1_Champion *c, int delta) {
    (void)c; (void)delta; return 0;
}

int theron_v1_modify_champion_mana(Theron_V1_Champion *c, int delta) {
    (void)c; (void)delta; return 0;
}

void theron_v1_champion_die(Theron_V1_World *world, int champ_slot) {
    (void)world; (void)champ_slot;
}

void theron_v1_creature_die(Theron_V1_World *world, int creature_id) {
    (void)world; (void)creature_id;
}

int theron_v1_drop_loot(Theron_V1_World *world, int creature_id, int x, int y) {
    (void)world; (void)creature_id; (void)x; (void)y; return -1;
}

int theron_v1_play_sound(Theron_SoundID id) { (void)id; return 0; }

int theron_v1_sound_is_valid(Theron_SoundID id) {
    (void)id; return 0;
}

const char *theron_v1_combat_source_evidence(void) {
    return "Track 02 creature/combat records not decoded; production route blocked";
}
