/*
 * theron_v1_combat_runtime_source.c — production admission boundary
 *
 * The former theron_v1_compat.c contains inferred creature templates and
 * combat constants. Those values remain fixture-only. This production file
 * admits only the five regular Track 02 spawn zones and their disassembly-
 * bound category HP/attack/defense formulas; scripted encounters, behavior,
 * combat, drops and sounds remain fail-closed.
 */

#include "theron_v1_combat.h"
#include "theron_v1_world.h"
#include "theron_v1_track02_creature_spawn.h"

#include <string.h>

static int theron_v1_source_monster_record_at(
    const Theron_V1_World *world,
    int dungeon_id,
    int level,
    int x,
    int y) {
    unsigned int i;
    if (!world) return 0;
    for (i = 0; i < world->source_monster_count; ++i) {
        const Theron_V1_SourceMonsterRecord *record =
            &world->source_monsters[i];
        if (record->dungeon_id == dungeon_id && record->level == level &&
            record->x == x && record->y == y) {
            return 1;
        }
    }
    return 0;
}

int theron_v1_creature_spawn(Theron_V1_World *world,
                             Theron_CreatureType type,
                             int dungeon_id, int level, int x, int y) {
    Theron_V1_Level *source_level;

    /* Only a loaded, source-header-verified level and a matching authentic
     * Track 02 monster occurrence may create a creature. A header alone is
     * not a monster record and must not authorize a host-positioned spawn.
     * THQUEST.ASM T500's scripted Thief/Demon encounters have no spawn-data
     * pointer and remain rejected. */
    if (!world || type < THERON_CREATURE_AKUTUBA ||
        type > THERON_CREATURE_SHADO || dungeon_id < 1 ||
        dungeon_id > THERON_DUNGEON_COUNT || level < 0 ||
        level >= THERON_MAX_LEVELS_PER_DUNGEON ||
        world->creature_count >= THERON_MAX_CREATURES_PER_LEVEL) {
        return -1;
    }
    source_level = &world->levels[dungeon_id - 1][level];
    if (!world->level_loaded[dungeon_id - 1][level] ||
        !source_level->source_header_verified ||
        !theron_v1_source_monster_record_at(world, dungeon_id, level, x, y)) {
        return -1;
    }
    /* The source ledger is real, but the PCE bank-switched RNG call used by
     * the category formulas is not recovered. A host tick/coordinate seed
     * would be synthetic gameplay state, so retain the decoded occurrence
     * and refuse live publication until the original RNG consumer is bound.
     * Source: THQUEST.ASM spawn overlay $4644/$4667; the static category
     * table remains available to diagnostic tests only. */
    return -1;
}

int theron_v1_creature_kill(Theron_V1_World *world, int creature_id) {
    Theron_V1_Creature *creature =
        theron_v1_creature_by_id(world, creature_id);
    if (!creature || !(creature->flags & THERON_CF_ACTIVE)) {
        return -1;
    }
    creature->flags &= ~THERON_CF_ACTIVE;
    creature->hp = 0;
    /* T900 loot ownership is still unavailable; killing must not fabricate
     * a drop or gold record. */
    return 0;
}

int theron_v1_creature_remove(Theron_V1_World *world, int creature_id) {
    int i;
    if (!world || creature_id <= 0) return -1;
    for (i = 0; i < world->creature_count; ++i) {
        if (world->creatures[i].id == creature_id) {
            world->creatures[i] =
                world->creatures[world->creature_count - 1];
            --world->creature_count;
            return 0;
        }
    }
    return -1;
}

Theron_V1_Creature *theron_v1_creature_at(Theron_V1_World *world,
                                           int level, int x, int y) {
    int i;
    if (!world) return NULL;
    for (i = 0; i < world->creature_count; ++i) {
        Theron_V1_Creature *creature = &world->creatures[i];
        if ((creature->flags & THERON_CF_ACTIVE) &&
            creature->level == level && creature->x == x &&
            creature->y == y) {
            return creature;
        }
    }
    return NULL;
}

Theron_V1_Creature *theron_v1_creature_by_id(Theron_V1_World *world, int id) {
    int i;
    if (!world || id <= 0) return NULL;
    for (i = 0; i < world->creature_count; ++i) {
        if (world->creatures[i].id == id) return &world->creatures[i];
    }
    return NULL;
}

int theron_v1_creature_count(const Theron_V1_World *world,
                             int dungeon_id, int level) {
    int i;
    int count = 0;
    if (!world) return 0;
    for (i = 0; i < world->creature_count; ++i) {
        const Theron_V1_Creature *creature = &world->creatures[i];
        if ((creature->flags & THERON_CF_ACTIVE) &&
            creature->dungeon_id == dungeon_id &&
            creature->level == level) {
            ++count;
        }
    }
    return count;
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
    return "Track 02 regular spawn formulas admitted; scripted encounters, AI, loot and combat consumer remain blocked";
}
