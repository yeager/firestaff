/*
 * theron_v1_compat.c — Theron V1 Phase 5: Combat + Creature Mechanics
 *
 * Source-locked implementation of the Theron's Quest V1 combat and creature
 * API declared in include/theron_v1_combat.h.
 *
 * Source references:
 *   THQUEST.ASM T500  — creature spawning / wave triggers
 *   THQUEST.ASM T600  — party/creature collision + combat resolution
 *   THQUEST.ASM T700  — per-tick HP/Poison processing
 *   THQUEST.ASM T900  — object drops / treasure tables / altar-of-vi
 *
 *   ReDMCSB GROUP.C / COMMAND.C / CLIKMENU.C / GAMELOOP.C analogues
 *   docs/source-lock/tqr_v1_phase2_data_formats_H2339.md
 *
 * Design notes:
 *   - Creatures live in world->creatures[] and are hashed into world state.
 *   - Damage is deterministic; minimum 1 HP on a hit.
 *   - Drops are placed as Theron_V1_Object items on the floor at x,y.
 *   - The sound backend remains a platform-specific stub.
 */

#include "theron_v1_combat.h"
#include "theron_v1_world.h"
#include "theron_v1_track02_creature_spawn.h"
#include "theron_v1_track02_item_categories.h"
#include "theron_v1_track02_item_properties.h"
#include "theron_v1_track02_spell_descriptors.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* ── Internal creature stat table ─────────────────────────────────── */
/* 7 real creature types from Track 02 UD 0x2741EF, one per dungeon.
 * Theron has NO per-type stat table (unlike DM1 G0243). Stats come from
 * category-based formulas at spawn (see theron_v1_track02_creature_spawn.h).
 * Values below are runtime approximations for the combat system. */
typedef struct {
    Theron_CreatureType type;
    int base_hp;
    int base_attack;
    int base_defense;
    int speed;
    Theron_AIBehaviour ai;
    Theron_AttackType primary;
    Theron_AttackType secondary;
    int gold_min;
    int gold_max;
    uint8_t drops[4];
} Theron_CreatureTemplate;

static const Theron_CreatureTemplate g_creature_templates[] = {
    { THERON_CREATURE_AKUTUBA, 12,  4, 1, 3, THERON_AI_GUARD,
      THERON_ATTACK_SLASH, THERON_ATTACK_NONE, 1,  5,  { THERON_ITEM_FOOD, 0, 0, 0 } },
    { THERON_CREATURE_DRATOR,  20,  6, 2, 3, THERON_AI_CHASE,
      THERON_ATTACK_SLASH, THERON_ATTACK_NONE, 2,  8,  { THERON_ITEM_WEAPON, 0, 0, 0 } },
    { THERON_CREATURE_FORMIC,  18,  5, 2, 4, THERON_AI_CHASE,
      THERON_ATTACK_PIERCE, THERON_ATTACK_NONE, 1, 6,  { THERON_ITEM_POTION, 0, 0, 0 } },
    { THERON_CREATURE_SARMON,  24,  5, 1, 2, THERON_AI_CHASE,
      THERON_ATTACK_POISON, THERON_ATTACK_NONE, 1, 4,  { 0, 0, 0, 0 } },
    { THERON_CREATURE_SHADO,   10,  3, 1, 3, THERON_AI_GUARD,
      THERON_ATTACK_SLASH, THERON_ATTACK_NONE, 1, 3,  { 0, 0, 0, 0 } },
    { THERON_CREATURE_THIEF,   35,  8, 3, 2, THERON_AI_CHASE,
      THERON_ATTACK_SLASH, THERON_ATTACK_NONE, 5, 15, { THERON_ITEM_ARMOR, 0, 0, 0 } },
    { THERON_CREATURE_DEMON,   40, 10, 4, 3, THERON_AI_CHASE,
      THERON_ATTACK_MAGIC, THERON_ATTACK_BLAST, 8, 25, { 0, 0, 0, 0 } },
};

static const size_t g_creature_template_count =
    sizeof(g_creature_templates) / sizeof(g_creature_templates[0]);

static const Theron_CreatureTemplate *creature_template_for(Theron_CreatureType type) {
    for (size_t i = 0; i < g_creature_template_count; i++) {
        if (g_creature_templates[i].type == type) return &g_creature_templates[i];
    }
    return NULL;
}

static int creature_id_for_index(int index) {
    return index + 1;
}

/* Look up b4 (damage/defense) from the Track 02 item property table.
 * Weapons (cat 0x80): b4 = attack damage bonus.
 * Armor   (cat 0x81): b4 = defense bonus. */
static int item_property_b4(int item_index) {
    if (item_index < 0) return 0;
    const Theron_ItemPropertyRecord *rec =
        theron_v1_track02_item_property((unsigned int)item_index);
    return rec ? (int)rec->b4 : 0;
}

/* ── Creature lifecycle ───────────────────────────────────────────── */

int theron_v1_creature_spawn(Theron_V1_World *world,
    Theron_CreatureType type, int dungeon_id, int level,
    int x, int y)
{
    if (!world) return -1;
    if (world->creature_count >= THERON_MAX_CREATURES_PER_LEVEL) return -1;
    if (type == THERON_CREATURE_NONE) return -1;

    Theron_CreatureType dungeon_type = (Theron_CreatureType)dungeon_id;
    const Theron_CreatureTemplate *tmpl = creature_template_for(dungeon_type);
    if (!tmpl) return -1;

    Theron_V1_Creature *c = &world->creatures[world->creature_count];
    memset(c, 0, sizeof(*c));
    c->id = creature_id_for_index(world->creature_count);
    c->type = (uint8_t)type;
    c->level = (uint8_t)level;
    c->dungeon_id = dungeon_id;
    c->x = x;
    c->y = y;
    unsigned int creature_idx = (unsigned int)(dungeon_id - 1);
    const Theron_SpawnZoneDesc *zone = theron_v1_track02_spawn_zone(creature_idx);
    if (zone) {
        Theron_SpawnStats stats;
        uint16_t seed = (uint16_t)(world->world_tick ^ (unsigned)(x * 31 + y * 17));
        theron_v1_track02_compute_spawn_stats(
            zone->category, zone->param1, zone->param2, seed, &stats);
        c->hp = stats.hp;
        c->max_hp = stats.hp;
        c->attack = stats.attack;
        c->defense = stats.defense;
    } else {
        c->hp = tmpl->base_hp;
        c->max_hp = tmpl->base_hp;
        c->attack = tmpl->base_attack;
        c->defense = tmpl->base_defense;
    }
    c->speed = tmpl->speed;
    c->ai = tmpl->ai;
    c->primary_attack = tmpl->primary;
    c->secondary_attack = tmpl->secondary;
    c->flags = THERON_CF_ACTIVE;
    c->gold_drop_min = tmpl->gold_min;
    c->gold_drop_max = tmpl->gold_max;
    memcpy(c->item_drop_table, tmpl->drops, sizeof(c->item_drop_table));
    c->next_move_tick = (int)world->world_tick + tmpl->speed;
    world->creature_count++;
    return c->id;
}

int theron_v1_creature_kill(Theron_V1_World *world, int creature_id) {
    if (!world || creature_id <= 0) return -1;
    for (int i = 0; i < world->creature_count; i++) {
        if (world->creatures[i].id == creature_id) {
            world->creatures[i].flags &= ~THERON_CF_ACTIVE;
            theron_v1_drop_loot(world, creature_id,
                                world->creatures[i].x,
                                world->creatures[i].y);
            return 0;
        }
    }
    return -1;
}

int theron_v1_creature_remove(Theron_V1_World *world, int creature_id) {
    if (!world || creature_id <= 0) return -1;
    for (int i = 0; i < world->creature_count; i++) {
        if (world->creatures[i].id == creature_id) {
            world->creatures[i] = world->creatures[--world->creature_count];
            return 0;
        }
    }
    return -1;
}

Theron_V1_Creature *theron_v1_creature_at(Theron_V1_World *world,
                                           int level, int x, int y)
{
    if (!world) return NULL;
    for (int i = 0; i < world->creature_count; i++) {
        Theron_V1_Creature *c = &world->creatures[i];
        if ((c->flags & THERON_CF_ACTIVE) && c->level == level &&
            c->x == x && c->y == y) {
            return c;
        }
    }
    return NULL;
}

Theron_V1_Creature *theron_v1_creature_by_id(Theron_V1_World *world, int id) {
    if (!world || id <= 0) return NULL;
    for (int i = 0; i < world->creature_count; i++) {
        if (world->creatures[i].id == id) return &world->creatures[i];
    }
    return NULL;
}

int theron_v1_creature_count(const Theron_V1_World *world,
                             int dungeon_id, int level)
{
    if (!world) return 0;
    int count = 0;
    for (int i = 0; i < world->creature_count; i++) {
        const Theron_V1_Creature *c = &world->creatures[i];
        if ((c->flags & THERON_CF_ACTIVE) &&
            c->dungeon_id == dungeon_id && c->level == level) {
            count++;
        }
    }
    return count;
}

/* ── Damage calculation ───────────────────────────────────────────── */

int theron_v1_calc_defense(const Theron_V1_Champion *defender,
                            Theron_AttackType type)
{
    if (!defender) return 0;
    int defense = defender->vitality / 4;
    if (type == THERON_ATTACK_MAGIC || type == THERON_ATTACK_BLAST) {
        defense += defender->anti_magic / 4;
    } else {
        defense += defender->dexterity / 6;
    }
    /* Equipment defense from Track 02 item property table (b4 field). */
    defense += item_property_b4(defender->slots[THERON_ESLOT_ARMOR]);
    defense += item_property_b4(defender->slots[THERON_ESLOT_SHIELD]);
    defense += item_property_b4(defender->slots[THERON_ESLOT_HELM]);
    return defense;
}

int theron_v1_calc_attack_damage(int attack_power,
                                   const Theron_V1_Champion *defender,
                                   Theron_AttackType type)
{
    if (!defender) return 0;
    int defense = theron_v1_calc_defense(defender, type);
    int damage = attack_power - defense / 2;
    if (damage < 1) damage = 1;
    if (type == THERON_ATTACK_BLAST) damage += 2;
    if (type == THERON_ATTACK_POISON) damage += 1;
    return damage;
}

/* ── Champion attack ──────────────────────────────────────────────── */

int theron_v1_champion_attack(Theron_V1_World *world,
    int attacking_slot, int target_creature_id)
{
    if (!world || attacking_slot < 0 ||
        attacking_slot >= THERON_MAX_CHAMPIONS) return -1;

    Theron_V1_Creature *c = theron_v1_creature_by_id(world, target_creature_id);
    if (!c || !(c->flags & THERON_CF_ACTIVE)) return -1;

    Theron_V1_Champion *attacker =
        theron_v1_party_getChampion(&world->party, attacking_slot);
    if (!attacker || !attacker->alive) return -1;

    /* Action index determines stamina cost from Track 02 spell descriptor
     * table (UD 0x09EFAF).  Unarmed = PUNCH (6, cost 1), armed = CHOP (2,
     * cost 8).  Source: THQUEST.ASM T600 action dispatch. */
    unsigned int action_index = (attacker->slots[THERON_ESLOT_WEAPON] >= 0)
                                    ? 2u   /* CHOP */
                                    : 6u;  /* PUNCH */
    int stamina_cost = (int)theron_v1_track02_action_spell_cost(action_index);
    if (attacker->stamina < stamina_cost) return -1;
    theron_v1_modify_champion_stamina(attacker, -stamina_cost);

    int weapon_bonus = item_property_b4(attacker->slots[THERON_ESLOT_WEAPON]);
    int attack_power = attacker->strength / 4 + weapon_bonus;
    if (attacker->primary_class == THERON_CLASS_NINJA) {
        attack_power += attacker->dexterity / 6;
    }

    int damage = attack_power - c->defense / 2;
    if (damage < 1) damage = 1;

    c->hp -= damage;
    theron_v1_play_sound(THERON_SOUND_SWORD_SWING);

    if (c->hp <= 0) {
        c->hp = 0;
        c->flags &= ~THERON_CF_ACTIVE;
        theron_v1_play_sound(THERON_SOUND_CREATURE_DIE);
        theron_v1_drop_loot(world, c->id, c->x, c->y);
        return 1; /* killed */
    }
    theron_v1_play_sound(THERON_SOUND_CREATURE_HIT);
    return 0; /* hit but alive */
}

/* ── Spell casting ───────────────────────────────────────────────── */

int theron_v1_champion_cast_spell(Theron_V1_World *world,
    int casting_slot, unsigned int spell_index,
    int target_creature_id)
{
    if (!world || casting_slot < 0 ||
        casting_slot >= THERON_MAX_CHAMPIONS) return -1;
    if (spell_index < 20 || spell_index >= 41) return -1;

    Theron_V1_Champion *caster =
        theron_v1_party_getChampion(&world->party, casting_slot);
    if (!caster || !caster->alive) return -1;

    int mana_cost = (int)theron_v1_track02_action_spell_cost(spell_index);
    if (caster->mana < mana_cost) return -1;
    theron_v1_modify_champion_mana(caster, -mana_cost);

    switch (spell_index) {
    case 35: /* HEAL */
        theron_v1_modify_champion_hp(caster, 15 + caster->anti_magic / 2);
        return 0;
    case 37: /* LIGHT — environmental, no combat effect */
        return 0;
    case 36: /* CALM — environmental, no combat effect */
        return 0;
    case 33: /* SPELLSHIELD — buff, no target */
        caster->anti_magic += 10;
        return 0;
    case 34: /* FIRESHIELD — buff, no target */
        caster->anti_magic += 15;
        return 0;
    default:
        break;
    }

    /* Offensive spells target a creature */
    Theron_V1_Creature *c = theron_v1_creature_by_id(world, target_creature_id);
    if (!c || !(c->flags & THERON_CF_ACTIVE)) return -1;

    int spell_power = caster->anti_magic / 2 + mana_cost / 3;

    switch (spell_index) {
    case 20: /* FIREBALL — blast damage */
        theron_v1_play_sound(THERON_SOUND_FIREBALL);
        spell_power += 5;
        break;
    case 23: /* LIGHTNING — highest damage spell */
        theron_v1_play_sound(THERON_SOUND_EXPLOSION);
        spell_power += 8;
        break;
    case 21: /* DISPELL — effective vs magic creatures */
        if (c->primary_attack == THERON_ATTACK_MAGIC)
            spell_power += spell_power;
        break;
    case 22: /* CONFUSE — reduces creature speed */
        c->speed += 2;
        break;
    case 24: /* DISRUPT — low cost, low damage */
        break;
    case 31: /* STUN — paralyze creature for several ticks */
        c->flags |= THERON_CF_PARALYZED;
        c->next_move_tick += 8;
        break;
    default:
        break;
    }

    int damage = spell_power - c->defense / 3;
    if (damage < 1) damage = 1;
    c->hp -= damage;

    if (c->hp <= 0) {
        c->hp = 0;
        c->flags &= ~THERON_CF_ACTIVE;
        theron_v1_play_sound(THERON_SOUND_CREATURE_DIE);
        theron_v1_drop_loot(world, c->id, c->x, c->y);
        return 1;
    }
    return 0;
}

/* ── Creature attack champion ─────────────────────────────────────── */

Theron_CombatResult theron_v1_creature_attack_champion(
    Theron_V1_World *world,
    int creature_id,
    int champion_slot)
{
    if (!world || champion_slot < 0 ||
        champion_slot >= THERON_MAX_CHAMPIONS) return THERON_COMBAT_MISS;

    Theron_V1_Creature *c = theron_v1_creature_by_id(world, creature_id);
    if (!c || !(c->flags & THERON_CF_ACTIVE)) return THERON_COMBAT_MISS;

    Theron_V1_Champion *defender =
        theron_v1_party_getChampion(&world->party, champion_slot);
    if (!defender || !defender->alive) return THERON_COMBAT_MISS;

    Theron_AttackType type = c->primary_attack;
    int damage = theron_v1_calc_attack_damage(c->attack, defender, type);
    if (type == THERON_ATTACK_POISON) {
        defender->attributes |= THERON_ATTR_POISONED;
    }

    theron_v1_modify_champion_hp(defender, -damage);
    theron_v1_play_sound(THERON_SOUND_PARTY_HIT);

    if (defender->health <= 0) {
        theron_v1_champion_die(world, champion_slot);
        theron_v1_play_sound(THERON_SOUND_PARTY_DIE);
        return THERON_COMBAT_KILL;
    }
    return THERON_COMBAT_HIT;
}

/* ── Death processing ─────────────────────────────────────────────── */

void theron_v1_champion_die(Theron_V1_World *world, int champ_slot) {
    if (!world) return;
    if (champ_slot < 0 || champ_slot >= THERON_MAX_CHAMPIONS) return;
    Theron_V1_Champion *c = theron_v1_party_getChampion(&world->party, champ_slot);
    if (!c) return;
    c->alive = 0;
    c->health = 0;
}

void theron_v1_creature_die(Theron_V1_World *world, int creature_id) {
    if (!world) return;
    theron_v1_creature_kill(world, creature_id);
}

/* ── HP / stamina / mana modification (clamped to valid ranges) ────── */

int theron_v1_modify_champion_hp(Theron_V1_Champion *c, int delta) {
    if (!c) return 0;
    int new_hp = c->health + delta;
    if (new_hp < 0) new_hp = 0;
    if (new_hp > c->max_health) new_hp = c->max_health;
    c->health = (int16_t)new_hp;
    return c->health;
}

int theron_v1_modify_champion_stamina(Theron_V1_Champion *c, int delta) {
    if (!c) return 0;
    int new_stamina = c->stamina + delta;
    if (new_stamina < 0) new_stamina = 0;
    if (new_stamina > c->max_stamina) new_stamina = c->max_stamina;
    c->stamina = (int16_t)new_stamina;
    return c->stamina;
}

int theron_v1_modify_champion_mana(Theron_V1_Champion *c, int delta) {
    if (!c) return 0;
    int new_mana = c->mana + delta;
    if (new_mana < 0) new_mana = 0;
    if (new_mana > c->max_mana) new_mana = c->max_mana;
    c->mana = (int16_t)new_mana;
    return c->mana;
}

/* ── Drops ────────────────────────────────────────────────────────── */

int theron_v1_drop_loot(Theron_V1_World *world,
    int creature_id, int x, int y)
{
    if (!world) return -1;
    const Theron_V1_Creature *c = theron_v1_creature_by_id(world, creature_id);
    if (!c) return -1;

    /* Gold drop */
    if (c->gold_drop_max > 0 && world->object_count < THERON_MAX_OBJECTS) {
        int gold = c->gold_drop_min;
        if (c->gold_drop_max > c->gold_drop_min) {
            gold += rand() % (c->gold_drop_max - c->gold_drop_min + 1);
        }
        if (gold > 0) {
            Theron_V1_Object o;
            memset(&o, 0, sizeof(o));
            o.id = world->object_count + 1;
            o.type = THERON_OBJTYPE_CHEST;
            o.x = x;
            o.y = y;
            o.level = world->current_level;
            o.dungeon_id = world->current_dungeon;
            o.quantity = gold;
            o.item_index = -1;
            theron_v1_object_place(world, &o);
        }
    }

    /* Item drops — resolve synthetic category to real Track 02 item index */
    for (int i = 0; i < 4 && world->object_count < THERON_MAX_OBJECTS; i++) {
        uint8_t item = c->item_drop_table[i];
        if (item == 0) continue;
        int real_idx = theron_v1_track02_resolve_drop_item(
            item, (unsigned int)rand());
        if (real_idx < 0) continue;
        uint8_t cat = theron_v1_track02_item_category((unsigned int)real_idx);
        uint8_t otype = THERON_OBJTYPE_CHEST;
        if (cat == THERON_ITEM_CAT_WEAPON) otype = THERON_OBJTYPE_WEAPON;
        else if (cat == THERON_ITEM_CAT_ARMOR) otype = THERON_OBJTYPE_ARMOR;
        else if (cat == THERON_ITEM_CAT_CONSUMABLE) otype = THERON_OBJTYPE_POTION;
        Theron_V1_Object o;
        memset(&o, 0, sizeof(o));
        o.id = world->object_count + 1;
        o.type = otype;
        o.x = x;
        o.y = y;
        o.level = world->current_level;
        o.dungeon_id = world->current_dungeon;
        o.quantity = 1;
        o.item_index = real_idx;
        theron_v1_object_place(world, &o);
    }
    return 0;
}

/* ── Creature AI tick ─────────────────────────────────────────────── */

void theron_v1_creature_ai_tick(Theron_V1_World *world) {
    if (!world) return;

    for (int i = 0; i < world->creature_count; i++) {
        Theron_V1_Creature *c = &world->creatures[i];
        if (!(c->flags & THERON_CF_ACTIVE)) continue;
        if (c->level != world->current_level) continue;

        if (world->world_tick < (uint64_t)c->next_move_tick) continue;
        c->next_move_tick = (int)world->world_tick + c->speed;

        if (c->ai == THERON_AI_PASSIVE) continue;

        int dx = world->party.leader_x - c->x;
        int dy = world->party.leader_y - c->y;
        int dist = (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);

        /* Suicide / fireball: explode when adjacent. */
        if (c->ai == THERON_AI_SUICIDE) {
            if (dist <= 1) {
                /* Blast hits the active champion. */
                theron_v1_creature_attack_champion(world, c->id,
                                                   world->party.active_slot);
                c->flags &= ~THERON_CF_ACTIVE;
                c->hp = 0;
                theron_v1_play_sound(THERON_SOUND_EXPLOSION);
            } else if (dist <= 5) {
                /* Move one step toward the party. */
                int mx = (dx > 0) - (dx < 0);
                int my = (dy > 0) - (dy < 0);
                uint8_t t = theron_v1_world_get_square(world, c->x + mx, c->y + my);
                if (THERON_SQUARE_IS_PASSABLE(t) && t != THERON_SQUARE_DOOR) {
                    c->x += mx;
                    c->y += my;
                }
            }
            continue;
        }

        /* GUARD: only act when party is adjacent. */
        if (c->ai == THERON_AI_GUARD) {
            if (dist == 1) {
                theron_v1_creature_attack_champion(world, c->id,
                                                   world->party.active_slot);
            }
            continue;
        }

        /* CHASE: attack if adjacent, otherwise move toward party. */
        if (c->ai == THERON_AI_CHASE) {
            if (dist == 1) {
                theron_v1_creature_attack_champion(world, c->id,
                                                   world->party.active_slot);
            } else if (dist <= 8) {
                int mx = (dx > 0) - (dx < 0);
                int my = (dy > 0) - (dy < 0);
                uint8_t t = theron_v1_world_get_square(world, c->x + mx, c->y + my);
                if (THERON_SQUARE_IS_PASSABLE(t) && t != THERON_SQUARE_DOOR) {
                    c->x += mx;
                    c->y += my;
                }
            }
        }
    }
}

/* ── Sound / audio trigger interface ──────────────────────────────── */

int theron_v1_play_sound(Theron_SoundID id) {
    (void)id;
    /* Platform-specific backend stub: no audio output in headless builds. */
    return 0;
}

int theron_v1_sound_is_valid(Theron_SoundID id) {
    return id >= 0 && id < THERON_SOUND_COUNT;
}

const char *theron_v1_combat_source_evidence(void) {
    return
        "Theron V1 Combat/Creature System — source-locked implementation\n"
        "Source: THQUEST.ASM T500/T600/T700/T900 (creature spawn, combat, drops)\n"
        "Source: ReDMCSB GROUP.C / COMMAND.C / CLIKMENU.C / GAMELOOP.C analogues\n"
        "Source: docs/source-lock/tqr_v1_phase2_data_formats_H2339.md\n";
}
