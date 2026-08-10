/*
 * theron_v1_combat_runtime_source.c — production admission boundary
 *
 * The former theron_v1_compat.c contains inferred creature templates and
 * combat constants. Those values remain fixture-only. This production file
 * admits only source-identified static Track 02 monster occurrences; the
 * disassembly-bound category formulas are diagnostic-only until the original
 * RNG return contract is captured. Scripted encounters, behavior, combat,
 * drops and sounds remain fail-closed.
 */

#include "theron_v1_combat.h"
#include "theron_v1_world.h"
#include "theron_v1_track02_creature_spawn.h"

#include <string.h>

static const Theron_V1_SourceMonsterRecord *
theron_v1_source_monster_record_at(
    const Theron_V1_World *world,
    Theron_CreatureType type,
    int dungeon_id,
    int level,
    int x,
    int y) {
    unsigned int i;
    uint8_t raw_type;
    if (!world || type < THERON_CREATURE_AKUTUBA ||
        type > THERON_CREATURE_DEMON) return NULL;
    /* Track 02 stores creature names/types zero-based (AKUTUBA=0), while
     * the live API reserves zero for NONE.  This is the same explicit
     * source-record -> live-creature boundary used by the static loader. */
    raw_type = (uint8_t)(type - THERON_CREATURE_AKUTUBA);
    for (i = 0; i < world->source_monster_count; ++i) {
        const Theron_V1_SourceMonsterRecord *record =
            &world->source_monsters[i];
        if (record->dungeon_id == dungeon_id && record->level == level &&
            record->x == x && record->y == y &&
            record->type == raw_type) {
            return record;
        }
    }
    return NULL;
}

static int theron_v1_source_group_already_admitted(
    const Theron_V1_World *world,
    int dungeon_id,
    int level,
    const Theron_V1_SourceMonsterRecord *record) {
    int i;

    if (!world || !record) return 0;
    for (i = 0; i < world->creature_count; ++i) {
        const Theron_V1_Creature *creature = &world->creatures[i];
        if (creature->dungeon_id == dungeon_id &&
            creature->level == level &&
            creature->source_ref == record->source_ref &&
            creature->source_index == record->source_index) {
            return 1;
        }
    }
    return 0;
}

static int theron_v1_publish_source_group(
    Theron_V1_World *world,
    const Theron_V1_SourceMonsterRecord *record,
    int dungeon_id,
    int level) {
    unsigned int members;
    unsigned int live_members = 0;
    unsigned int slot;

    if (!world || !record || record->number > 3u) return -1;
    members = (unsigned int)record->number + 1u;
    for (slot = 0; slot < members; ++slot) {
        if (record->health[slot] != 0u) ++live_members;
    }
    /* Admission is transactional.  A source group is one authenticated
     * record, so never expose a partially materialized group when the live
     * pool cannot hold all of its non-zero HP members. */
    if (live_members == 0u ||
        world->creature_count < 0 ||
        world->creature_count > THERON_MAX_CREATURES_PER_LEVEL ||
        live_members > (unsigned int)THERON_MAX_CREATURES_PER_LEVEL ||
        live_members > (unsigned int)THERON_MAX_CREATURES_PER_LEVEL -
                          (unsigned int)world->creature_count) {
        return -1;
    }
    for (slot = 0; slot < members; ++slot) {
        Theron_V1_Creature *creature;
        if (record->health[slot] == 0u) continue;
        creature = &world->creatures[world->creature_count++];
        memset(creature, 0, sizeof(*creature));
        creature->id = ((int)record->source_ref << 2) | (int)slot;
        if (creature->id <= 0) creature->id = world->creature_count;
        creature->type = (uint8_t)(record->type + 1u);
        creature->level = (uint8_t)level;
        creature->dungeon_id = dungeon_id;
        creature->x = record->x;
        creature->y = record->y;
        creature->hp = (int)record->health[slot];
        creature->max_hp = creature->hp;
        creature->primary_attack = THERON_ATTACK_NONE;
        creature->secondary_attack = THERON_ATTACK_NONE;
        creature->flags = THERON_CF_ACTIVE;
        creature->source_ref = record->source_ref;
        creature->source_index = record->source_index;
        creature->source_chested = record->chested;
        creature->source_position = record->position;
        creature->source_cell = (uint8_t)((record->position >> (slot * 2u)) & 0x03u);
        creature->source_slot = (uint8_t)slot;
        creature->source_group_count = (uint8_t)members;
        creature->source_direction_flags = record->direction_flags;
        creature->source_flags_word = record->flags_word;
        creature->source_unknown_word = record->unknown_word;
        creature->source_spawn_category =
            theron_v1_world_track02_spawn_category(world, record->type);
    }
    return world->creature_count > 0 ?
        world->creatures[world->creature_count - 1].id : -1;
}

int theron_v1_creature_spawn(Theron_V1_World *world,
                             Theron_CreatureType type,
                             int dungeon_id, int level, int x, int y) {
    Theron_V1_Level *source_level;
    const Theron_V1_SourceMonsterRecord *source_record;

    /* Only a loaded, source-header-verified level and a matching authentic
     * Track 02 monster occurrence may create a creature. A header alone is
     * not a monster record and must not authorize a host-positioned spawn.
     * THQUEST.ASM T500's scripted Thief/Demon encounters have no spawn-data
     * pointer and remain rejected. */
    if (!world || type < THERON_CREATURE_AKUTUBA ||
        type > THERON_CREATURE_DEMON || dungeon_id < 1 ||
        dungeon_id > THERON_DUNGEON_COUNT || level < 0 ||
        level >= THERON_MAX_LEVELS_PER_DUNGEON ||
        world->creature_count >= THERON_MAX_CREATURES_PER_LEVEL) {
        return -1;
    }
    source_level = &world->levels[dungeon_id - 1][level];
    if (!world->level_loaded[dungeon_id - 1][level] ||
        !source_level->source_header_verified) {
        return -1;
    }
    source_record = theron_v1_source_monster_record_at(
        world, type, dungeon_id, level, x, y);
    if (!source_record || source_record->number > 3u ||
        source_record->health[0] == 0u) {
        return -1;
    }
    /* A static source occurrence has a stable source identity.  Until the
     * original respawn consumer is captured, re-admitting an inactive copy
     * would invent a respawn event and could duplicate the same Track 02
     * group after a kill. */
    if (theron_v1_source_group_already_admitted(
            world, dungeon_id, level, source_record)) {
        return -1;
    }
    if (theron_v1_creature_at_in_dungeon(world, dungeon_id, level, x, y))
        return -1;
    /* This API is also used for an explicit source occurrence.  Publish the
     * authenticated group bytes directly; this is not the random generator
     * path.  The latter still requires the original HuC6280 RNG return
     * contract and remains fail-closed in theron_v1_world_tick_generators().
     * Source: THQUEST.ASM static category-4 group records, with the regular
     * spawn overlay at $4644/$4667 kept separate. */
    return theron_v1_publish_source_group(world, source_record,
                                          dungeon_id, level);
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

Theron_V1_Creature *theron_v1_creature_at_in_dungeon(
    Theron_V1_World *world, int dungeon_id, int level, int x, int y) {
    int i;
    if (!world) return NULL;
    for (i = 0; i < world->creature_count; ++i) {
        Theron_V1_Creature *creature = &world->creatures[i];
        if ((creature->flags & THERON_CF_ACTIVE) &&
            creature->dungeon_id == dungeon_id &&
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

int theron_v1_play_sound(Theron_SoundID id) {
    /*
     * The authenticated full capture proves ADPCM FIFO -> RAM transport,
     * but no CPU/event read owns a gameplay sound yet.  Do not report a
     * successful play until a source-bound event consumer is identified.
     */
    (void)id;
    return -1;
}

int theron_v1_sound_is_valid(Theron_SoundID id) {
    (void)id; return 0;
}

const char *theron_v1_combat_source_evidence(void) {
    return "Track 02 regular static monster records admitted; RNG spawn formulas, scripted encounters, AI, loot and combat consumer remain blocked";
}
