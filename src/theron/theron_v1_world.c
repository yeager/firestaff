/*
 * theron_v1_world.c — Theron's Quest V1 Phase 3: Core World Model
 *
 * Implementations for map loading, party placement, map transitions,
 * timers, object database, and deterministic world-state hashing.
 *
 * Source references:
 *   THQUEST.ASM T400  — dungeon bank loading
 *   THQUEST.ASM T520  — party placement / start position
 *   THQUEST.ASM T560  — dungeon loading (header parsing, dungeon_seed)
 *   THQUEST.ASM T600  — map transitions
 *   THQUEST.ASM T700  — timers / world tick
 *   THQUEST.ASM T800  — champion persistence + inventory reset
 *   THQUEST.ASM T900  — object database / thing list
 *   docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md
 */

#include "theron_v1_world.h"
#include "theron_v1_track02_item_properties.h"
#include "theron_v1_track02_item_categories.h"
#include "theron_v1_track02_creature_names.h"
#include "theron_v1_combat.h"
#include "theron_v1_track02.h"
#include "theron_v1_track02_dungeon_map.h"
#include "theron_v1_track02_level_data_blocks.h"
#include "theron_v1_track02_thing_data.h"
#include "theron_v1_track02_actuator.h"
#include "theron_v1_track02_creature_spawn.h"
#include "theron_v1_track02_text_decode.h"
#include <string.h>
#include <limits.h>

#define THERON_LEGACY_GENERATOR_RUNTIME_SLOTS 5u
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* ── Compile-time sanity check ──────────────────────────────────── */
/* 128 bytes per champion block (matches DM1 v1 champion layout) */
_Static_assert(sizeof(Theron_V1_Champion) >= 128,
               "Theron_V1_Champion must be at least 128 bytes");

/* ── FNV-1a 64-bit helpers ─────────────────────────────────────────── */

static uint64_t fnv64_word(uint64_t h, uint64_t val) {
    /* FNV-1a per-octet on a 64-bit word */
    const uint64_t prime = THERON_HASH_FNV_PRIME;
    h ^= val & 0xFFULL;         h *= prime;
    h ^= (val >>  8) & 0xFFULL; h *= prime;
    h ^= (val >> 16) & 0xFFULL; h *= prime;
    h ^= (val >> 24) & 0xFFULL; h *= prime;
    h ^= (val >> 32) & 0xFFULL; h *= prime;
    h ^= (val >> 40) & 0xFFULL; h *= prime;
    h ^= (val >> 48) & 0xFFULL; h *= prime;
    h ^= (val >> 56) & 0xFFULL; h *= prime;
    return h;
}

static uint64_t fnv64_bytes(uint64_t h, const uint8_t *bytes, size_t count) {
    const uint64_t prime = THERON_HASH_FNV_PRIME;
    size_t i;
    if (!bytes) return h;
    for (i = 0; i < count; ++i) {
        h ^= bytes[i];
        h *= prime;
    }
    return h;
}

/* ── Read helpers (big-endian 68000 → host LE) ─────────────────────── */

static uint16_t rb16(const uint8_t *p) {
    return ((uint16_t)p[0] << 8) | (uint16_t)p[1];
}
static uint32_t rb32(const uint8_t *p) {
    return ((uint32_t)rb16(p) << 16) | rb16(p + 2);
}

/* Firestaff world snapshots use explicit little-endian scalar fields.  The
 * original THQUEST data is separately decoded as 68000 big-endian above. */
static uint16_t rw16(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}
static uint32_t rw32(const uint8_t *p) {
    return (uint32_t)p[0] |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}
static uint64_t rw64(const uint8_t *p) {
    uint64_t value = 0;
    for (unsigned i = 0; i < 8; ++i) value |= (uint64_t)p[i] << (i * 8);
    return value;
}
static void ww16(uint8_t *p, uint16_t value) {
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8);
}
static void ww32(uint8_t *p, uint32_t value) {
    for (unsigned i = 0; i < 4; ++i) p[i] = (uint8_t)(value >> (i * 8));
}
static void ww64(uint8_t *p, uint64_t value) {
    for (unsigned i = 0; i < 8; ++i) p[i] = (uint8_t)(value >> (i * 8));
}

#define THERON_INVENTORY_SOURCE_WIRE_BYTES_V6 31u
#define THERON_INVENTORY_SOURCE_WIRE_BYTES_V7 48u
#define THERON_OBJECT_WIRE_BYTES 86u
#define THERON_TIMER_WIRE_BYTES 24u
#define THERON_CREATURE_WIRE_BYTES_V7 87u
#define THERON_CREATURE_WIRE_BYTES_V8 88u
#define THERON_CREATURE_WIRE_BYTES_V9 90u
#define THERON_CREATURE_WIRE_BYTES_V10 107u
#define THERON_GENERATOR_WIRE_BYTES 32u
#define THERON_GENERATOR_WIRE_BYTES_V6 36u
#define THERON_SOURCE_OBJECT_WIRE_BYTES 41u

static size_t theron_generator_wire_size(void) {
    return THERON_GENERATOR_WIRE_BYTES_V6;
}

static size_t theron_creature_wire_size_for_version(uint16_t version) {
    return version >= 10u ? THERON_CREATURE_WIRE_BYTES_V10
           : version >= 9u ? THERON_CREATURE_WIRE_BYTES_V9
                         : version >= 8u ? THERON_CREATURE_WIRE_BYTES_V8
                         : THERON_CREATURE_WIRE_BYTES_V7;
}

static size_t theron_generator_wire_size_for_version(uint16_t version) {
    return version >= 6u ? THERON_GENERATOR_WIRE_BYTES_V6
                         : THERON_GENERATOR_WIRE_BYTES;
}

static uint8_t *theron_source_object_write(
    uint8_t *out, const Theron_V1_SourceObjectRecord *record) {
    ww32(out, (uint32_t)record->dungeon_id); out += 4;
    ww32(out, (uint32_t)record->level); out += 4;
    ww32(out, (uint32_t)record->x); out += 4;
    ww32(out, (uint32_t)record->y); out += 4;
    ww16(out, record->source_ref); out += 2;
    ww16(out, record->next_ref); out += 2;
    ww16(out, record->source_index); out += 2;
    *out++ = record->category;
    *out++ = record->position;
    *out++ = record->raw_size;
    memcpy(out, record->raw, sizeof(record->raw));
    return out + sizeof(record->raw);
}

static const uint8_t *theron_source_object_read(
    const uint8_t *in, Theron_V1_SourceObjectRecord *record) {
    memset(record, 0, sizeof(*record));
    record->dungeon_id = (int32_t)rw32(in); in += 4;
    record->level = (int32_t)rw32(in); in += 4;
    record->x = (int32_t)rw32(in); in += 4;
    record->y = (int32_t)rw32(in); in += 4;
    record->source_ref = rw16(in); in += 2;
    record->next_ref = rw16(in); in += 2;
    record->source_index = rw16(in); in += 2;
    record->category = *in++;
    record->position = *in++;
    record->raw_size = *in++;
    memcpy(record->raw, in, sizeof(record->raw));
    return in + sizeof(record->raw);
}

/* Source generator records are save data, not an inferred executable
 * consumer. Keeping the decoded actuator and runtime counters lets a future
 * source-bound consumer resume without silently resetting the level.
 * THQUEST.ASM T700/T900 remain the authority for when these fields act. */
static uint8_t *theron_generator_write(
    uint8_t *out, const Theron_V1_SourceGeneratorRecord *record) {
    ww32(out, (uint32_t)record->dungeon_id); out += 4;
    ww32(out, (uint32_t)record->level); out += 4;
    ww32(out, (uint32_t)record->x); out += 4;
    ww32(out, (uint32_t)record->y); out += 4;
    ww16(out, record->source_ref); out += 2;
    ww16(out, record->source_index); out += 2;
    *out++ = record->type;
    ww16(out, record->value); out += 2;
    *out++ = record->once;
    *out++ = record->effect;
    *out++ = record->sound;
    *out++ = record->delay;
    *out++ = record->inactive;
    *out++ = record->graphism;
    *out++ = record->target_x;
    *out++ = record->target_y;
    *out++ = record->target_facing;
    *out++ = record->generator_fields_valid;
    *out++ = record->generator_generation;
    *out++ = record->generator_toughness;
    *out++ = record->generator_pause;
    return out;
}

static const uint8_t *theron_generator_read(
    const uint8_t *in, Theron_V1_SourceGeneratorRecord *record,
    uint16_t version) {
    memset(record, 0, sizeof(*record));
    record->dungeon_id = (int32_t)rw32(in); in += 4;
    record->level = (int32_t)rw32(in); in += 4;
    record->x = (int32_t)rw32(in); in += 4;
    record->y = (int32_t)rw32(in); in += 4;
    record->source_ref = rw16(in); in += 2;
    record->source_index = rw16(in); in += 2;
    record->type = *in++;
    record->value = rw16(in); in += 2;
    record->once = *in++;
    record->effect = *in++;
    record->sound = *in++;
    record->delay = *in++;
    record->inactive = *in++;
    record->graphism = *in++;
    record->target_x = *in++;
    record->target_y = *in++;
    record->target_facing = *in++;
    if (version >= 6u) {
        record->generator_fields_valid = *in++;
        record->generator_generation = *in++;
        record->generator_toughness = *in++;
        record->generator_pause = *in++;
    }
    return in;
}

static size_t theron_object_wire_size(void) {
    return THERON_OBJECT_WIRE_BYTES;
}

static uint8_t *theron_object_write(uint8_t *out,
                                    const Theron_V1_Object *object) {
    ww32(out, (uint32_t)object->id); out += 4;
    *out++ = object->type;
    *out++ = object->state;
    ww32(out, (uint32_t)object->x); out += 4;
    ww32(out, (uint32_t)object->y); out += 4;
    ww32(out, (uint32_t)object->level); out += 4;
    ww32(out, (uint32_t)object->dungeon_id); out += 4;
    ww32(out, (uint32_t)object->quantity); out += 4;
    ww32(out, (uint32_t)object->item_index); out += 4;
    ww32(out, (uint32_t)object->linked_id); out += 4;
    ww32(out, object->flags); out += 4;
    ww16(out, object->source_ref); out += 2;
    ww16(out, object->source_next_ref); out += 2;
    ww16(out, object->source_index); out += 2;
    *out++ = object->source_category;
    *out++ = object->source_position;
    *out++ = object->source_raw_size;
    memcpy(out, object->source_raw, sizeof(object->source_raw)); out += 16;
    *out++ = object->source_item_type;
    *out++ = object->source_keep;
    *out++ = object->source_cursed;
    *out++ = object->source_broken;
    *out++ = object->source_poisoned;
    *out++ = object->source_closed;
    *out++ = object->source_dump;
    *out++ = object->source_power;
    *out++ = object->source_capacity;
    ww16(out, object->source_text_ref); out += 2;
    ww16(out, (uint16_t)object->source_chested); out += 2;
    ww16(out, object->source_data1); out += 2;
    *out++ = object->source_item_category;
    *out++ = object->source_property_valid;
    memcpy(out, object->source_property, sizeof(object->source_property));
    return out + sizeof(object->source_property);
}

static const uint8_t *theron_object_read(
    const uint8_t *in, Theron_V1_Object *object) {
    memset(object, 0, sizeof(*object));
    object->id = (int32_t)rw32(in); in += 4;
    object->type = *in++;
    object->state = *in++;
    object->x = (int32_t)rw32(in); in += 4;
    object->y = (int32_t)rw32(in); in += 4;
    object->level = (int32_t)rw32(in); in += 4;
    object->dungeon_id = (int32_t)rw32(in); in += 4;
    object->quantity = (int32_t)rw32(in); in += 4;
    object->item_index = (int32_t)rw32(in); in += 4;
    object->linked_id = (int32_t)rw32(in); in += 4;
    object->flags = rw32(in); in += 4;
    object->source_ref = rw16(in); in += 2;
    object->source_next_ref = rw16(in); in += 2;
    object->source_index = rw16(in); in += 2;
    object->source_category = *in++;
    object->source_position = *in++;
    object->source_raw_size = *in++;
    memcpy(object->source_raw, in, sizeof(object->source_raw)); in += 16;
    object->source_item_type = *in++;
    object->source_keep = *in++;
    object->source_cursed = *in++;
    object->source_broken = *in++;
    object->source_poisoned = *in++;
    object->source_closed = *in++;
    object->source_dump = *in++;
    object->source_power = *in++;
    object->source_capacity = *in++;
    object->source_text_ref = rw16(in); in += 2;
    object->source_chested = (int16_t)rw16(in); in += 2;
    object->source_data1 = rw16(in); in += 2;
    object->source_item_category = *in++;
    object->source_property_valid = *in++;
    memcpy(object->source_property, in, sizeof(object->source_property));
    return in + sizeof(object->source_property);
}

static uint8_t *theron_timer_write(uint8_t *out,
                                   const Theron_V1_Timer *timer) {
    ww32(out, (uint32_t)timer->id); out += 4;
    ww32(out, (uint32_t)timer->kind); out += 4;
    ww32(out, (uint32_t)timer->level); out += 4;
    ww32(out, (uint32_t)timer->remaining_ticks); out += 4;
    ww32(out, (uint32_t)timer->interval_ticks); out += 4;
    ww32(out, timer->flags); out += 4;
    return out;
}

static const uint8_t *theron_timer_read(const uint8_t *in,
                                         Theron_V1_Timer *timer) {
    memset(timer, 0, sizeof(*timer));
    timer->id = (int32_t)rw32(in); in += 4;
    timer->kind = (Theron_TimerKind)rw32(in); in += 4;
    timer->level = (int32_t)rw32(in); in += 4;
    timer->remaining_ticks = (int32_t)rw32(in); in += 4;
    timer->interval_ticks = (int32_t)rw32(in); in += 4;
    timer->flags = rw32(in); in += 4;
    timer->userdata = NULL;
    return in;
}

static uint8_t *theron_creature_write(uint8_t *out,
                                      const Theron_V1_Creature *creature) {
    ww32(out, (uint32_t)creature->id); out += 4;
    *out++ = creature->type;
    *out++ = creature->level;
    ww32(out, (uint32_t)creature->dungeon_id); out += 4;
    ww32(out, (uint32_t)creature->x); out += 4;
    ww32(out, (uint32_t)creature->y); out += 4;
    ww32(out, (uint32_t)creature->hp); out += 4;
    ww32(out, (uint32_t)creature->max_hp); out += 4;
    ww32(out, (uint32_t)creature->attack); out += 4;
    ww32(out, (uint32_t)creature->defense); out += 4;
    ww32(out, (uint32_t)creature->speed); out += 4;
    ww32(out, (uint32_t)creature->next_move_tick); out += 4;
    ww32(out, (uint32_t)creature->ai); out += 4;
    ww32(out, (uint32_t)creature->primary_attack); out += 4;
    ww32(out, (uint32_t)creature->secondary_attack); out += 4;
    ww32(out, creature->flags); out += 4;
    ww32(out, (uint32_t)creature->gold_drop_min); out += 4;
    ww32(out, (uint32_t)creature->gold_drop_max); out += 4;
    memcpy(out, creature->item_drop_table, sizeof(creature->item_drop_table));
    out += sizeof(creature->item_drop_table);
    ww32(out, (uint32_t)creature->link_id); out += 4;
    ww16(out, creature->source_ref); out += 2;
    ww16(out, creature->source_index); out += 2;
    ww16(out, (uint16_t)creature->source_chested); out += 2;
    *out++ = creature->source_position;
    *out++ = creature->source_cell;
    *out++ = creature->source_slot;
    *out++ = creature->source_group_count;
    *out++ = creature->source_direction_flags;
    ww16(out, creature->source_flags_word); out += 2;
    ww16(out, creature->source_unknown_word); out += 2;
    *out++ = creature->source_spawn_category;
    *out++ = creature->source_raw_size;
    memcpy(out, creature->source_raw, sizeof(creature->source_raw));
    out += sizeof(creature->source_raw);
    return out;
}

static const uint8_t *theron_creature_read(
    const uint8_t *in, Theron_V1_Creature *creature, uint16_t version) {
    memset(creature, 0, sizeof(*creature));
    creature->id = (int32_t)rw32(in); in += 4;
    creature->type = *in++;
    creature->level = *in++;
    creature->dungeon_id = (int32_t)rw32(in); in += 4;
    creature->x = (int32_t)rw32(in); in += 4;
    creature->y = (int32_t)rw32(in); in += 4;
    creature->hp = (int32_t)rw32(in); in += 4;
    creature->max_hp = (int32_t)rw32(in); in += 4;
    creature->attack = (int32_t)rw32(in); in += 4;
    creature->defense = (int32_t)rw32(in); in += 4;
    creature->speed = (int32_t)rw32(in); in += 4;
    creature->next_move_tick = (int32_t)rw32(in); in += 4;
    creature->ai = (Theron_AIBehaviour)rw32(in); in += 4;
    creature->primary_attack = (Theron_AttackType)rw32(in); in += 4;
    creature->secondary_attack = (Theron_AttackType)rw32(in); in += 4;
    creature->flags = rw32(in); in += 4;
    creature->gold_drop_min = (int32_t)rw32(in); in += 4;
    creature->gold_drop_max = (int32_t)rw32(in); in += 4;
    memcpy(creature->item_drop_table, in, sizeof(creature->item_drop_table));
    in += sizeof(creature->item_drop_table);
    creature->link_id = (int32_t)rw32(in); in += 4;
    creature->source_ref = rw16(in); in += 2;
    creature->source_index = rw16(in); in += 2;
    creature->source_chested = version >= 9u ? (int16_t)rw16(in) : 0;
    if (version >= 9u) in += 2;
    creature->source_position = *in++;
    creature->source_cell = *in++;
    creature->source_slot = *in++;
    creature->source_group_count = *in++;
    creature->source_direction_flags = *in++;
    creature->source_flags_word = rw16(in); in += 2;
    creature->source_unknown_word = rw16(in); in += 2;
    creature->source_spawn_category = version >= 8u ? *in++ : 0xffu;
    if (version >= 10u) {
        creature->source_raw_size = *in++;
        memcpy(creature->source_raw, in, sizeof(creature->source_raw));
        in += sizeof(creature->source_raw);
    }
    return in;
}

static size_t theron_inventory_source_wire_size_for_version(uint16_t version) {
    size_t bytes = version >= 7u ? THERON_INVENTORY_SOURCE_WIRE_BYTES_V7
                                 : THERON_INVENTORY_SOURCE_WIRE_BYTES_V6;
    return (size_t)THERON_MAX_CHAMPIONS * THERON_INVENTORY_SLOTS * bytes;
}

static size_t theron_inventory_source_wire_size(void) {
    return theron_inventory_source_wire_size_for_version(
        THERON_WORLD_SAVE_VERSION);
}

static int theron_v1_world_source_level_verified(
    const Theron_V1_World *world) {
    if (!world || world->current_dungeon < 1 ||
        world->current_dungeon > THERON_DUNGEON_COUNT ||
        world->current_level < 0 ||
        world->current_level >= THERON_MAX_LEVELS_PER_DUNGEON) {
        return 0;
    }
    return world->level_loaded[world->current_dungeon - 1]
                              [world->current_level] &&
           world->levels[world->current_dungeon - 1]
                        [world->current_level].source_header_verified;
}

static int theron_v1_world_source_item_table_verified(
    const Theron_V1_World *world) {
    if (!theron_v1_world_source_level_verified(world)) return 0;
    return world->levels[world->current_dungeon - 1]
                      [world->current_level].source_item_property_table_verified;
}

static int theron_v1_object_is_carried_item(const Theron_V1_Object *object) {
    if (!object) return 0;
    switch (object->type) {
    case THERON_OBJTYPE_POTION:
    case THERON_OBJTYPE_SCROLL:
    case THERON_OBJTYPE_FOOD:
    case THERON_OBJTYPE_KEY:
    case THERON_OBJTYPE_WEAPON:
    case THERON_OBJTYPE_ARMOR:
    case THERON_OBJTYPE_SOURCE_ITEM:
        return 1;
    default:
        return 0;
    }
}

static int theron_v1_object_is_active_for_lookup(
    const Theron_V1_Object *object) {
    if (!object || (object->flags & THERON_OBJ_F_DESTROYED)) return 0;
    return !(theron_v1_object_is_carried_item(object) &&
             (object->flags & THERON_OBJ_F_PICKED_UP));
}

static uint8_t *theron_inventory_source_write(
    uint8_t *out, const Theron_V1_InventorySourceRecord *record) {
    *out++ = record->valid;
    *out++ = record->category;
    *out++ = record->item_type;
    *out++ = record->keep;
    *out++ = record->cursed;
    *out++ = record->broken;
    *out++ = record->poisoned;
    *out++ = record->closed;
    *out++ = record->dump;
    *out++ = record->power;
    *out++ = record->charges;
    ww16(out, record->source_ref); out += sizeof(uint16_t);
    ww16(out, record->source_next_ref); out += sizeof(uint16_t);
    ww16(out, record->source_index); out += sizeof(uint16_t);
    ww16(out, record->text_ref); out += sizeof(uint16_t);
    ww16(out, (uint16_t)record->chested); out += sizeof(uint16_t);
    ww16(out, record->data1); out += sizeof(uint16_t);
    *out++ = record->item_category;
    *out++ = record->property_valid;
    memcpy(out, record->property, sizeof(record->property));
    out += sizeof(record->property);
    *out++ = record->source_raw_size;
    memcpy(out, record->source_raw, sizeof(record->source_raw));
    return out + sizeof(record->source_raw);
}

static const uint8_t *theron_inventory_source_read(
    const uint8_t *in, Theron_V1_InventorySourceRecord *record,
    uint16_t version) {
    memset(record, 0, sizeof(*record));
    record->valid = *in++;
    record->category = *in++;
    record->item_type = *in++;
    record->keep = *in++;
    record->cursed = *in++;
    record->broken = *in++;
    record->poisoned = *in++;
    record->closed = *in++;
    record->dump = *in++;
    record->power = *in++;
    record->charges = *in++;
    record->source_ref = rw16(in); in += sizeof(uint16_t);
    record->source_next_ref = rw16(in); in += sizeof(uint16_t);
    record->source_index = rw16(in); in += sizeof(uint16_t);
    record->text_ref = rw16(in); in += sizeof(uint16_t);
    record->chested = (int16_t)rw16(in); in += sizeof(uint16_t);
    record->data1 = rw16(in); in += sizeof(uint16_t);
    record->item_category = *in++;
    record->property_valid = *in++;
    memcpy(record->property, in, sizeof(record->property));
    in += sizeof(record->property);
    if (version >= 7u) {
        record->source_raw_size = *in++;
        memcpy(record->source_raw, in, sizeof(record->source_raw));
        in += sizeof(record->source_raw);
    }
    return in;
}

/* ══════════════════════════════════════════════════════════════════════
 * Party management
 * ============================================================================
 *
 * NOTE: These function bodies are defined in theron_v1_champions.c and linked
 * once by the firestaff_theron static library.
 *
 * To keep champions.c as the single canonical definition point, world.c does
 * NOT export duplicate party symbols.  World serialization uses static
 * _tqw_ wrappers for gold-inclusive pack calculations.
 *
 * Source: THQUEST.ASM T800
 */

static size_t _tqw_party_pack_size(void) {
    return (size_t)THERON_MAX_CHAMPIONS * sizeof(Theron_V1_Champion)
           + sizeof(uint32_t);   /* gold field follows champions in save layout */
}

static size_t __attribute__((unused)) _tqw_champion_block_size (void) {
    return sizeof(Theron_V1_Champion);
}

/* All non-static party functions above are defined in theron_v1_champions.c */


/* ── Party pack / unpack ───────────────────────────────────────────── */

size_t _tqw_party_pack(const Theron_V1_Party *p, void *buf, size_t bufsize) {
    if (!p || !buf) return 0;
    size_t need = _tqw_party_pack_size();
    if (bufsize < need) return 0;
    uint8_t *out = (uint8_t *)buf;
    ww32(out, p->gold);
    out += sizeof(uint32_t);
    for (int i = 0; i < THERON_MAX_CHAMPIONS; i++) {
        memcpy(out, &p->champions[i], sizeof(p->champions[i]));
        out += sizeof(p->champions[i]);
    }
    return need;
}

int _tqw_party_unpack(Theron_V1_Party *p, const void *buf, size_t bufsize) {
    if (!p || !buf || bufsize < _tqw_party_pack_size()) return -1;
    const uint8_t *in = (const uint8_t *)buf;
    p->gold = rw32(in);
    in += sizeof(uint32_t);
    for (int i = 0; i < THERON_MAX_CHAMPIONS; i++) {
        memcpy(&p->champions[i], in, sizeof(p->champions[i]));
        in += sizeof(p->champions[i]);
    }
    p->champion_count = THERON_MAX_CHAMPIONS;
    return 0;
}

/* ══════════════════════════════════════════════════════════════════════
 * World initialization & reset
 * ══════════════════════════════════════════════════════════════════════ */

static void theron_v1_world_init_base(Theron_V1_World *world) {
    if (!world) return;
    memset(world, 0, sizeof(*world));
    world->current_dungeon    = THERON_DUNGEON_1_AKUTUBA;
    world->current_level      = 0;
    world->world_tick        = 0;
    world->transition_pending = 0;
    world->state_hash        = THERON_HASH_FNV_OFFSET;
    theron_v1_dungeon_progression_init(&world->progression);
}

void theron_v1_world_init(Theron_V1_World *world) {
    theron_v1_world_init_base(world);
    if (!world) return;
    theron_v1_party_init(&world->party, world->current_dungeon);
}

void theron_v1_world_init_runtime(Theron_V1_World *world) {
    theron_v1_world_init_base(world);
}

void theron_v1_world_reset_for_dungeon(Theron_V1_World *world,
                                       Theron_DungeonID dungeon_id) {
    if (!world) return;
    world->current_dungeon           = dungeon_id;
    world->current_level             = 0;
    world->transition_pending        = 0;
    world->quest_items_in_dungeon    = 0;
    world->dungeon_complete          = 0;
    world->entry_reset_applied       = 0;
    theron_v1_world_runtime_media_invalidate_cache(world);
    world->object_count              = 0;
    world->creature_count            = 0;
    world->source_monster_count      = 0;
    memset(&world->track02_spawn_source, 0,
           sizeof(world->track02_spawn_source));
    world->track02_spawn_source_variant = 0;
    world->source_generator_count    = 0;
    world->source_object_count       = 0;
    world->timer_count               = 0;
    memset(world->objects, 0, sizeof(world->objects));
    memset(world->creatures, 0, sizeof(world->creatures));
    memset(world->source_monsters, 0, sizeof(world->source_monsters));
    memset(world->source_generators, 0, sizeof(world->source_generators));
    memset(world->source_objects, 0, sizeof(world->source_objects));
    memset(world->timers,  0, sizeof(world->timers));
}

/* ══════════════════════════════════════════════════════════════════════
 * Map loading (TQR dungeon format)
 *
 * 12-byte header (big-endian 68000 format):
 *   bytes 0-1:  width  (uint16_t BE = LE in file)
 *   bytes 2-3:  height (uint16_t BE = LE in file)
 *   bytes 4-7:  dungeon_seed (uint32_t LE)
 *   bytes 8-9:  level_index (uint16_t LE)
 *   bytes 10-11: reserved
 * Grid follows: width*height bytes of uint8_t tile values.
 * ══════════════════════════════════════════════════════════════════════ */

Theron_MapLoadResult theron_v1_level_load(Theron_V1_Level *level,
                                           const uint8_t *data,
                                           int data_size,
                                           int dungeon_id,
                                           int sub_level_index) {
    if (!level || !data || data_size < 16) return THERON_MAP_ERR_NULL;
    memset(level, 0, sizeof(*level));
    level->level_index = sub_level_index;

    uint16_t w    = rb16(data + 0);
    uint16_t h    = rb16(data + 2);
    uint32_t seed = rb32(data + 4);
    uint16_t source_header_level_index = rb16(data + 8);

    if (w == 0 || w > THERON_MAX_MAP_SIZE || h == 0 || h > THERON_MAX_MAP_SIZE) {
        return THERON_MAP_ERR_INVALID_GRID;
    }
    level->width  = w;
    level->height = h;
    level->dungeon_seed = seed;
    level->source_header_level_index = source_header_level_index;
    /* Track 02's bounded level envelope does not carry a start direction;
     * the verified runtime receipt admits its documented North pose. */
    level->start_dir = 0;

    /* Guard: minimum size for header + at least one row */
    size_t grid_bytes = (size_t)w * h;
    int header_size = 12;
    if (data_size < header_size + (int)grid_bytes) {
        printf("theron_v1_level_load: truncated — need %d, got %d\n",
               header_size + (int)grid_bytes, data_size);
        return THERON_MAP_ERR_SIZE_TOO_SMALL;
    }

    const uint8_t *grid = data + header_size;
    int has_entrance = 0, has_exit = 0;
    level->start_x = 0;
    level->start_y = 0;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            uint8_t tile = grid[y * w + x] & 0x1F; /* 5-bit tile semantics */
            level->squares[y][x] = tile;
            if (tile == THERON_SQUARE_FLOOR && !has_entrance) {
                level->start_x = (int16_t)x;
                level->start_y = (int16_t)y;
                has_entrance   = 1;
            }
            if (tile == THERON_SQUARE_EXIT       ||
                tile == THERON_SQUARE_STAIRS_DOWN ||
                tile == THERON_SQUARE_STAIRS_UP) {
                has_exit = 1;
            }
        }
    }
    level->thing_count = 0;

    printf("TQR level load: dungeon=%d level=%d size=%dx%d status=%s entrance=(%d,%d)\n",
           dungeon_id, sub_level_index, w, h,
           has_entrance ? "OK" : "NO_ENTRANCE",
           level->start_x, level->start_y);

    if (!has_entrance) return THERON_MAP_ERR_NO_ENTRANCE;
    (void)dungeon_id;
    (void)has_exit;
    return THERON_MAP_OK;
}

/* ── Track 02 quest block → world bridge ───────────────────────── */

static uint8_t track02_tile_to_square(uint8_t tile_byte) {
    Theron_TileType tt = theron_tile_type(tile_byte);
    switch (tt) {
        case THERON_TILE_WALL:       return THERON_SQUARE_WALL;
        case THERON_TILE_OPEN:       return THERON_SQUARE_FLOOR;
        case THERON_TILE_PIT:        return THERON_SQUARE_PIT;
        case THERON_TILE_STAIRS:     return THERON_SQUARE_STAIRS_DOWN;
        case THERON_TILE_DOOR:       return THERON_SQUARE_DOOR;
        case THERON_TILE_TELEPORTER: return THERON_SQUARE_TELEPORTER;
        case THERON_TILE_FAKEWALL:   return THERON_SQUARE_SECRET;
        case THERON_TILE_TYPE7:      return THERON_SQUARE_WALL;
    }
    return THERON_SQUARE_WALL;
}

/* A Track 02 bank is replaceable per dungeon, not a replacement for the
 * complete campaign source ledger.  Keep records from every other dungeon
 * when a bank is reloaded, then append the selected dungeon's fresh records
 * below.  This matters for Continue/late-level routes, where the world can
 * retain multiple authenticated dungeon banks at once. */
static void theron_v1_remove_source_records_for_dungeon(
    Theron_V1_World *world, int dungeon_id) {
    unsigned int write_index;

    if (!world) return;

    write_index = 0;
    for (unsigned int i = 0; i < world->source_monster_count; ++i) {
        if (world->source_monsters[i].dungeon_id == dungeon_id) continue;
        if (write_index != i)
            world->source_monsters[write_index] = world->source_monsters[i];
        ++write_index;
    }
    for (unsigned int i = write_index; i < world->source_monster_count; ++i)
        memset(&world->source_monsters[i], 0,
               sizeof(world->source_monsters[i]));
    world->source_monster_count = write_index;

    write_index = 0;
    for (unsigned int i = 0; i < world->source_generator_count; ++i) {
        if (world->source_generators[i].dungeon_id == dungeon_id) continue;
        if (write_index != i)
            world->source_generators[write_index] = world->source_generators[i];
        ++write_index;
    }
    for (unsigned int i = write_index; i < world->source_generator_count; ++i)
        memset(&world->source_generators[i], 0,
               sizeof(world->source_generators[i]));
    world->source_generator_count = write_index;

    write_index = 0;
    for (unsigned int i = 0; i < world->source_object_count; ++i) {
        if (world->source_objects[i].dungeon_id == dungeon_id) continue;
        if (write_index != i)
            world->source_objects[write_index] = world->source_objects[i];
        ++write_index;
    }
    for (unsigned int i = write_index; i < world->source_object_count; ++i)
        memset(&world->source_objects[i], 0,
               sizeof(world->source_objects[i]));
    world->source_object_count = write_index;
}

static void theron_v1_remove_objects_for_dungeon(
    Theron_V1_World *world, int dungeon_id) {
    int write_index;

    if (!world) return;
    write_index = 0;
    for (int i = 0; i < world->object_count; ++i) {
        if (world->objects[i].dungeon_id == dungeon_id) continue;
        if (write_index != i) world->objects[write_index] = world->objects[i];
        ++write_index;
    }
    for (int i = write_index; i < world->object_count; ++i)
        memset(&world->objects[i], 0, sizeof(world->objects[i]));
    world->object_count = write_index;
}

int theron_v1_world_load_track02_dungeon(
    Theron_V1_World *world,
    int dungeon_id,
    const Theron_DungeonData *dd)
{
    if (!world || !dd) return -1;
    if (dungeon_id < 1 || dungeon_id > THERON_DUNGEON_COUNT) return -1;

    int slot = dungeon_id - 1;
    int loaded = 0;

    /* ReDMCSB: THQUEST.ASM T400/T560 replaces the selected dungeon bank and
     * rebuilds its level directory.  Clear the complete slot first so a
     * reload from a real Track 02 bank with fewer maps cannot expose stale
     * level records from the previous bank. */
    memset(world->levels[slot], 0, sizeof(world->levels[slot]));
    memset(world->level_loaded[slot], 0, sizeof(world->level_loaded[slot]));

    theron_v1_remove_source_records_for_dungeon(world, dungeon_id);
    theron_v1_remove_objects_for_dungeon(world, dungeon_id);

    memcpy(world->source_thing_descriptor_sizes[slot],
           dd->thing_descriptor_sizes,
           sizeof(world->source_thing_descriptor_sizes[slot]));
    world->source_column_thing_count_total[slot] =
        dd->column_thing_count_total;
    world->source_thing_directory_verified[slot] = 1;

    for (unsigned int m = 0; m < dd->map_count && m < THERON_MAX_LEVELS_PER_DUNGEON; m++) {
        const Theron_Map *tm = &dd->maps[m];
        Theron_V1_Level *lv = &world->levels[slot][m];
        memset(lv, 0, sizeof(*lv));

        unsigned int w = (unsigned int)tm->header.x_dim + 1;
        unsigned int h = (unsigned int)tm->header.y_dim + 1;
        lv->level_index = (int)m;
        lv->width  = (int)w;
        lv->height = (int)h;
        lv->dungeon_seed = 0;
        lv->source_header_level_index = tm->header.map_id;
        lv->source_map_x_offset = tm->header.x_offset;
        lv->source_map_y_offset = tm->header.y_offset;
        lv->source_header_unk1 = tm->header.unk1;
        lv->source_header_unk2 = tm->header.unk2;
        lv->source_xp_modifier = tm->header.xp_modifier;
        lv->source_door_type1 = tm->header.door_type1;
        lv->source_door_type2 = tm->header.door_type2;
        lv->source_header_verified = 1;
        lv->source_creature_gfx_bank = dd->creature_gfx_bank[m];
        lv->source_cumulative_column_items = dd->cumulative_column_items[m];
        lv->start_dir = 0;

        int has_entrance = 0;
        for (unsigned int x = 0; x < w && x < THERON_MAX_MAP_SIZE; x++) {
            for (unsigned int y = 0; y < h && y < THERON_MAX_MAP_SIZE; y++) {
                uint8_t sq = track02_tile_to_square(tm->tiles[x][y]);
                lv->squares[y][x] = sq;
                if (sq == THERON_SQUARE_FLOOR && !has_entrance) {
                    lv->start_x = (int16_t)x;
                    lv->start_y = (int16_t)y;
                    has_entrance = 1;
                }
            }
        }

        lv->thing_count = 0;
        lv->creature_budget = (int)tm->header.creature_count;
        world->level_loaded[slot][m] = 1;
        loaded++;
    }
    return loaded;
}

/* ── Square query ─────────────────────────────────────────────────── */

uint8_t theron_v1_world_get_square(const Theron_V1_World *world, int x, int y) {
    if (!world) return THERON_SQUARE_WALL;
    int did = world->current_dungeon;
    int lvl = world->current_level;
    if (did < 1 || did > THERON_DUNGEON_COUNT)     return THERON_SQUARE_WALL;
    if (lvl < 0 || lvl >= THERON_MAX_LEVELS_PER_DUNGEON) return THERON_SQUARE_WALL;
    if (!world->level_loaded[did - 1][lvl])         return THERON_SQUARE_WALL;
    const Theron_V1_Level *lv = &world->levels[did - 1][lvl];
    if (x < 0 || x >= lv->width  || y < 0 || y >= lv->height) {
        return THERON_SQUARE_WALL;
    }
    return lv->squares[y][x];
}

/* ── Party placement ───────────────────────────────────────────────── */
/* THQUEST.ASM T520: party placed at start_x/start_y facing start_dir
 * (default: facing north = 0).  start_x/y are set during level_load. */
void theron_v1_party_place(Theron_V1_World *world, int x, int y, int dir) {
    if (!world) return;
    world->party.leader_x = (int16_t)x;
    world->party.leader_y = (int16_t)y;
    world->party.leader_dir = (int8_t)(dir & 3);
    /* THQUEST.ASM T520 places the party front separately from the map
     * header start marker; runtime movement and viewport sampling must read
     * this mutable party pose after launch. */
}

/* ══════════════════════════════════════════════════════════════════════
 * Object database
 * ══════════════════════════════════════════════════════════════════════ */

int theron_v1_object_place(Theron_V1_World *world, Theron_V1_Object *object) {
    int id = 0;

    if (!world || !object) return -1;
    if (world->object_count >= THERON_MAX_OBJECTS) return -1;
    Theron_V1_Object *o = &world->objects[world->object_count++];
    *o = *object;
    /* Object removal/reload can leave holes in the ID space.  Allocate above
     * the highest retained ID instead of deriving it from object_count,
     * otherwise a dungeon-local reload can alias a preserved object from a
     * different dungeon. */
    for (int i = 0; i < world->object_count - 1; ++i)
        if (world->objects[i].id > id) id = world->objects[i].id;
    if (id == INT_MAX) {
        --world->object_count;
        memset(o, 0, sizeof(*o));
        return -1;
    }
    ++id;
    o->id = id;
    object->id = o->id;
    return 0;
}

int theron_v1_object_remove(Theron_V1_World *world, int id) {
    if (!world || id <= 0) return -1;
    for (int i = 0; i < world->object_count; i++) {
        if (world->objects[i].id == id) {
            world->objects[i] = world->objects[--world->object_count];
            return 0;
        }
    }
    return -1;
}

const Theron_V1_InventorySourceRecord *theron_v1_inventory_source_at(
    const Theron_V1_World *world, int champion_slot, int inventory_slot) {
    if (!world || champion_slot < 0 || champion_slot >= THERON_MAX_CHAMPIONS ||
        inventory_slot < 0 || inventory_slot >= THERON_INVENTORY_SLOTS)
        return NULL;
    return &world->inventory_source[champion_slot][inventory_slot];
}

/* Re-validate the exact source payload before materialising a carried item
 * back into the object table.  Pickup performs the same check in the command
 * router, but save/load and other state transitions can touch the parallel
 * provenance record.  ReDMCSB THQUEST T900 owns this object transition; a
 * source-verified level must never accept a host-mutated compact item ID or
 * raw record as an authentic object.
 *
 * Source layout: DMBUILDER6/src/dms.h:69-176, decoded by
 * theron_v1_track02_item_record_decode(). */
static int theron_v1_inventory_source_record_matches(
    const Theron_V1_InventorySourceRecord *carried) {
    Theron_Track02ItemRecord record;
    const Theron_ItemPropertyRecord *property;

    if (!carried || !carried->valid || carried->source_ref == 0u ||
        carried->source_raw_size == 0u ||
        carried->source_raw_size > sizeof(carried->source_raw) ||
        !theron_v1_track02_item_record_decode(
            carried->category, carried->source_raw,
            carried->source_raw_size, &record) ||
        record.next_ref != carried->source_next_ref ||
        !carried->property_valid ||
        ((carried->category == THERON_CAT_WEAPON &&
          carried->item_category != THERON_ITEM_CAT_WEAPON) ||
         (carried->category == THERON_CAT_CLOTHING &&
          carried->item_category != THERON_ITEM_CAT_ARMOR) ||
         ((carried->category == THERON_CAT_SCROLL ||
           carried->category == THERON_CAT_POTION) &&
          carried->item_category != THERON_ITEM_CAT_CONSUMABLE) ||
         (carried->category == THERON_CAT_MISC &&
          carried->item_category != THERON_ITEM_CAT_COMPASS &&
          carried->item_category != THERON_ITEM_CAT_WEAPON &&
          carried->item_category != THERON_ITEM_CAT_ARMOR &&
          carried->item_category != THERON_ITEM_CAT_CONSUMABLE)) ||
        (carried->category == THERON_CAT_MISC &&
         carried->item_type >= theron_v1_track02_item_category_count()) ||
        carried->item_type >= theron_v1_track02_item_property_count() ||
        !(property = theron_v1_track02_item_property(carried->item_type)) ||
        memcmp(carried->property, property,
               sizeof(carried->property)) != 0) {
        return 0;
    }
    switch (carried->category) {
    case THERON_CAT_WEAPON:
        return record.value.weapon.type == carried->item_type &&
               record.value.weapon.keep == carried->keep &&
               record.value.weapon.cursed == carried->cursed &&
               record.value.weapon.poisoned == carried->poisoned &&
               record.value.weapon.charges == carried->charges &&
               record.value.weapon.broken == carried->broken;
    case THERON_CAT_CLOTHING:
        return record.value.clothing.type == carried->item_type &&
               record.value.clothing.keep == carried->keep &&
               record.value.clothing.cursed == carried->cursed &&
               record.value.clothing.dump == carried->dump &&
               record.value.clothing.broken == carried->broken;
    case THERON_CAT_SCROLL:
        return record.value.scroll.type == carried->item_type &&
               record.value.scroll.closed == carried->closed &&
               record.value.scroll.reftxt == carried->text_ref;
    case THERON_CAT_POTION:
        return record.value.potion.type == carried->item_type &&
               record.value.potion.power == carried->power &&
               record.value.potion.keep == carried->keep;
    case THERON_CAT_MISC:
        return record.value.misc.type == carried->item_type &&
               record.value.misc.keep == carried->keep &&
               theron_v1_track02_item_category(carried->item_type) ==
                   carried->item_category;
    default:
        return 0;
    }
}

int theron_v1_swap_inventory_source_slots(
    Theron_V1_World *world,
    int champion_slot,
    int inventory_slot_a,
    int inventory_slot_b) {
    Theron_V1_Champion *champion;
    Theron_V1_InventorySourceRecord *a;
    Theron_V1_InventorySourceRecord *b;
    uint8_t item_id;

    if (!world || champion_slot < 0 || champion_slot >= THERON_MAX_CHAMPIONS ||
        inventory_slot_a < 0 || inventory_slot_a >= THERON_INVENTORY_SLOTS ||
        inventory_slot_b < 0 || inventory_slot_b >= THERON_INVENTORY_SLOTS ||
        inventory_slot_a == inventory_slot_b)
        return -1;
    champion = &world->party.champions[champion_slot];
    a = &world->inventory_source[champion_slot][inventory_slot_a];
    b = &world->inventory_source[champion_slot][inventory_slot_b];
    if (theron_v1_world_source_level_verified(world)) {
        /* Real levels may only move a complete source-backed occurrence or an
         * empty slot. Never carry a compact ID without its authenticated row. */
        if (!theron_v1_world_source_item_table_verified(world) ||
            (champion->inventory[inventory_slot_a] != THERON_ITEM_NONE &&
             !theron_v1_inventory_source_record_matches(a)) ||
            (champion->inventory[inventory_slot_b] != THERON_ITEM_NONE &&
             !theron_v1_inventory_source_record_matches(b)) ||
            (champion->inventory[inventory_slot_a] == THERON_ITEM_NONE &&
             a->valid) ||
            (champion->inventory[inventory_slot_b] == THERON_ITEM_NONE &&
             b->valid))
            return -1;
    }
    item_id = champion->inventory[inventory_slot_a];
    champion->inventory[inventory_slot_a] =
        champion->inventory[inventory_slot_b];
    champion->inventory[inventory_slot_b] = item_id;
    {
        Theron_V1_InventorySourceRecord source = *a;
        *a = *b;
        *b = source;
    }
    theron_v1_party_recalculate_loads(&world->party);
    return 0;
}

int theron_v1_drop_inventory_source_item(
    Theron_V1_World *world,
    int champion_slot,
    int inventory_slot,
    int x,
    int y) {
    const Theron_V1_InventorySourceRecord *carried;
    Theron_V1_Object object;

    if (!world || champion_slot < 0 || champion_slot >= THERON_MAX_CHAMPIONS ||
        inventory_slot < 0 || inventory_slot >= THERON_INVENTORY_SLOTS ||
        world->party.champions[champion_slot].inventory[inventory_slot] ==
            THERON_ITEM_NONE)
        return -1;
    carried = &world->inventory_source[champion_slot][inventory_slot];
    if (!carried->valid || carried->source_ref == 0u)
        return -1;
    if (theron_v1_world_source_level_verified(world) &&
        (!carried->property_valid ||
         !theron_v1_inventory_source_record_matches(carried) ||
         !theron_v1_world_source_item_table_verified(world) ||
         carried->item_type !=
             world->party.champions[champion_slot].inventory[inventory_slot])) {
        /* Mirror the source pickup gate: a real T900 drop cannot recreate
         * an object whose authenticated property/category/type payload is
         * incomplete. */
        return -1;
    }

    memset(&object, 0, sizeof(object));
    switch (carried->category) {
    case THERON_CAT_WEAPON:   object.type = THERON_OBJTYPE_WEAPON; break;
    case THERON_CAT_CLOTHING: object.type = THERON_OBJTYPE_ARMOR; break;
    case THERON_CAT_SCROLL:   object.type = THERON_OBJTYPE_SCROLL; break;
    case THERON_CAT_POTION:   object.type = THERON_OBJTYPE_POTION; break;
    case THERON_CAT_MISC:     object.type = THERON_OBJTYPE_SOURCE_ITEM; break;
    case THERON_CAT_CHEST:    object.type = THERON_OBJTYPE_CHEST; break;
    default: return -1;
    }
    object.item_index = carried->item_type;
    object.quantity = carried->charges;
    object.level = world->current_level;
    object.dungeon_id = world->current_dungeon;
    object.x = x;
    object.y = y;
    object.source_ref = carried->source_ref;
    object.source_next_ref = carried->source_next_ref;
    object.source_index = carried->source_index;
    object.source_category = carried->category;
    object.source_item_type = carried->item_type;
    object.source_keep = carried->keep;
    object.source_cursed = carried->cursed;
    object.source_broken = carried->broken;
    object.source_poisoned = carried->poisoned;
    object.source_closed = carried->closed;
    object.source_dump = carried->dump;
    object.source_power = carried->power;
    object.source_text_ref = carried->text_ref;
    object.source_chested = carried->chested;
    object.source_data1 = carried->data1;
    object.source_item_category = carried->item_category;
    object.source_property_valid = carried->property_valid;
    memcpy(object.source_property, carried->property,
           sizeof(object.source_property));
    object.source_raw_size = carried->source_raw_size;
    memcpy(object.source_raw, carried->source_raw, sizeof(object.source_raw));
    if (theron_v1_object_place(world, &object) != 0)
        return -1;

    world->party.champions[champion_slot].inventory[inventory_slot] =
        THERON_ITEM_NONE;
    memset(&world->inventory_source[champion_slot][inventory_slot], 0,
           sizeof(world->inventory_source[champion_slot][inventory_slot]));
    theron_v1_party_recalculate_loads(&world->party);
    return object.id;
}

Theron_V1_Object *theron_v1_object_at(Theron_V1_World *world,
                                        int level, int x, int y) {
    if (!world) return NULL;
    for (int i = 0; i < world->object_count; i++) {
        Theron_V1_Object *o = &world->objects[i];
        /* T900 removes a carried occurrence from the active floor-object
         * view.  Keep the record in the table for source/save provenance,
         * but never let later look/use routing rediscover a picked object. */
        if (theron_v1_object_is_active_for_lookup(o) &&
            o->level == level && o->x == x && o->y == y) return o;
    }
    return NULL;
}

Theron_V1_Object *theron_v1_object_at_in_dungeon(
    Theron_V1_World *world, int dungeon_id, int level, int x, int y) {
    if (!world) return NULL;
    for (int i = 0; i < world->object_count; i++) {
        Theron_V1_Object *o = &world->objects[i];
        if (theron_v1_object_is_active_for_lookup(o) &&
            o->dungeon_id == dungeon_id && o->level == level &&
            o->x == x && o->y == y) {
            return o;
        }
    }
    return NULL;
}

Theron_V1_Object *theron_v1_object_by_id(Theron_V1_World *world, int id) {
    if (!world || id <= 0) return NULL;
    for (int i = 0; i < world->object_count; i++) {
        if (world->objects[i].id == id) return &world->objects[i];
    }
    return NULL;
}

int theron_v1_object_set_state(Theron_V1_World *world, int id, uint8_t s) {
    Theron_V1_Object *o = theron_v1_object_by_id(world, id);
    if (!o) return -1;
    o->state = s;
    return 0;
}

int theron_v1_object_set_flag(Theron_V1_World *world, int id, uint32_t f) {
    Theron_V1_Object *o = theron_v1_object_by_id(world, id);
    if (!o) return -1;
    o->flags |= f;
    return 0;
}

int theron_v1_object_clear_flag(Theron_V1_World *world, int id, uint32_t f) {
    Theron_V1_Object *o = theron_v1_object_by_id(world, id);
    if (!o) return -1;
    o->flags &= ~f;
    return 0;
}

int theron_v1_world_apply_track02_object_table(
    Theron_V1_World *world,
    int dungeon_id,
    int level_index,
    const Theron_Track02ObjectTable *table) {

    size_t i;
    Theron_V1_Level *level;

    if (!world || !table) return -1;
    if (dungeon_id < THERON_DUNGEON_1_AKUTUBA ||
        dungeon_id > THERON_DUNGEON_COUNT) return -1;
    if (level_index < 0 || level_index >= THERON_MAX_LEVELS_PER_DUNGEON)
        return -1;
    if (!world->level_loaded[dungeon_id - 1][level_index]) return -1;

    level = &world->levels[dungeon_id - 1][level_index];

    for (i = 0u; i < table->record_count; ++i) {
        const Theron_Track02ObjectTableRecord *rec = &table->records[i];
        Theron_V1_Object object = {0};

        if (rec->level_index != (uint8_t)level_index) continue;
        if (rec->x >= (uint8_t)level->width || rec->y >= (uint8_t)level->height)
            continue;
        if (rec->kind == 0u) continue;

        object.type = rec->kind;
        object.state = rec->flags & 0x03u;
        object.x = rec->x;
        object.y = rec->y;
        object.level = level_index;
        object.dungeon_id = dungeon_id;
        object.quantity = rec->argument ? rec->argument : 1u;
        object.flags = rec->flags;

        /* Door, teleporter, and pit records own the grid tile; trigger and
         * sound records keep their argument as a link/sound id.  Other objects
         * sit on the existing tile (floor, altar object, item, etc.). */
        if (rec->kind == THERON_OBJTYPE_DOOR) {
            level->squares[rec->y][rec->x] = THERON_SQUARE_DOOR;
        } else if (rec->kind == THERON_OBJTYPE_TELEPORTER) {
            level->squares[rec->y][rec->x] = THERON_SQUARE_TELEPORTER;
            object.linked_id = (int)rec->argument;
        } else if (rec->kind == THERON_OBJTYPE_TRIGGER) {
            object.linked_id = (int)rec->argument;
        } else if (rec->kind == THERON_OBJTYPE_PIT) {
            level->squares[rec->y][rec->x] = THERON_SQUARE_PIT;
        } else if (rec->kind == THERON_OBJTYPE_SOUND) {
            /* Sound records do not alter the tile; the argument is the sound
             * id and is preserved in quantity for the movement code. */
            object.quantity = (uint16_t)(rec->argument != 0u
                                             ? rec->argument
                                             : THERON_SOUND_AMBIENT_1);
        }

        if (theron_v1_object_place(world, &object) != 0) return -1;
        level->thing_count++;
    }

    return 0;
}

int theron_v1_world_apply_track02_object_table_for_dungeon(
    Theron_V1_World *world,
    int dungeon_id,
    const Theron_Track02ObjectTable *table) {

    int level_index;
    int result = 0;

    if (!world || !table) return -1;
    if (dungeon_id < THERON_DUNGEON_1_AKUTUBA ||
        dungeon_id > THERON_DUNGEON_COUNT) return -1;

    for (level_index = 0;
         level_index < THERON_MAX_LEVELS_PER_DUNGEON;
         ++level_index) {
        if (!world->level_loaded[dungeon_id - 1][level_index]) continue;
        if (theron_v1_world_apply_track02_object_table(
                world, dungeon_id, level_index, table) != 0) {
            result = -1;
        }
    }

    return result;
}

/* ══════════════════════════════════════════════════════════════════════
 * Timer system
 * ══════════════════════════════════════════════════════════════════════ */

static int g_next_timer_id = 1;

int theron_v1_timer_add(Theron_V1_World *world,
                        Theron_TimerKind kind,
                        int level,
                        int remaining_ticks,
                        int interval_ticks,
                        void *userdata) {
    if (!world || world->timer_count >= THERON_MAX_TIMERS) return -1;
    /* THQUEST.ASM T700's timer table is not the generic host timer API.
     * Until its Track 02 producer/consumer is authenticated, accepting a
     * host timer on a source-bound level would create timing semantics that
     * do not come from the disc. */
    if (theron_v1_world_source_level_verified(world)) return -1;
    Theron_V1_Timer *t = &world->timers[world->timer_count++];
    t->id               = g_next_timer_id++;
    t->kind             = kind;
    t->level            = level;
    t->remaining_ticks  = remaining_ticks;
    t->interval_ticks   = interval_ticks;
    t->flags            = THERON_TIMER_F_ACTIVE;
    t->userdata         = userdata;
    return 0;
}

void theron_v1_timer_remove(Theron_V1_World *world, int id) {
    if (!world || id <= 0) return;
    for (int i = 0; i < world->timer_count; i++) {
        if (world->timers[i].id == id) {
            world->timers[i] = world->timers[--world->timer_count];
            return;
        }
    }
}

void theron_v1_timer_pause(Theron_V1_World *world, int id) {
    if (!world || id <= 0) return;
    for (int i = 0; i < world->timer_count; i++) {
        if (world->timers[i].id == id) {
            world->timers[i].flags |= THERON_TIMER_F_PAUSED;
            return;
        }
    }
}

void theron_v1_timer_resume(Theron_V1_World *world, int id) {
    if (!world || id <= 0) return;
    for (int i = 0; i < world->timer_count; i++) {
        if (world->timers[i].id == id) {
            world->timers[i].flags &= ~THERON_TIMER_F_PAUSED;
            return;
        }
    }
}

void theron_v1_tick_timers(Theron_V1_World *world) {
    if (!world) return;
    /* A save may still contain legacy timer bytes.  Preserve them until the
     * source timer consumer is known; do not let the fixture countdown mutate
     * an authenticated Track 02 level. */
    if (theron_v1_world_source_level_verified(world)) return;
    for (int i = 0; i < world->timer_count; ) {
        Theron_V1_Timer *t = &world->timers[i];
        if ((t->flags & THERON_TIMER_F_ACTIVE) &&
            !(t->flags & THERON_TIMER_F_PAUSED)) {
            if (t->remaining_ticks > 0) {
                t->remaining_ticks--;
            }
            if (t->remaining_ticks == 0) {
                if (t->kind == THERON_TIMER_REPEAT) {
                    t->remaining_ticks = t->interval_ticks;
                    i++; /* stays alive */
                } else {
                    /* oneshot / countdown exhausted — remove */
                    t->flags |= THERON_TIMER_F_DESTROY;
                    world->timers[i] = world->timers[--world->timer_count];
                    continue;
                }
            }
        }
        i++;
    }
}

void theron_v1_timers_clear_level(Theron_V1_World *world, int level) {
    if (!world) return;
    if (level < 0) {
        world->timer_count = 0;
        return;
    }
    for (int i = 0; i < world->timer_count; ) {
        if (world->timers[i].level == level) {
            world->timers[i] = world->timers[--world->timer_count];
        } else {
            i++;
        }
    }
}

/* ══════════════════════════════════════════════════════════════════════
 * Level transitions
 * ══════════════════════════════════════════════════════════════════════ */

/* Queue scratchpad — populated by check_transition (thread-safe access
 * via world->transition_* fields; single-threaded call pattern). */
static __attribute__((unused)) struct {
    int pending;
    Theron_TransitionType type;
    int target_level;
    int spawn_x, spawn_y;
} g_queued = {0};

Theron_TransitionType theron_v1_check_transition(Theron_V1_World *world,
                                                     int x, int y) {
    if (!world) return 0;
    uint8_t tile = theron_v1_world_get_square(world, x, y);
    Theron_TransitionType tt = THERON_SQUARE_TO_TRANSITION_TYPE(tile);
    if (tt == 0) {
        world->transition_pending = 0;
        return 0;
    }

    /* Exit is locked until all quest items in this dungeon are collected */
    if (tt == THERON_TRANSITION_EXIT && !world->dungeon_complete) {
        return 0;
    }

    world->transition_pending = 1;
    world->transition_type    = tt;

    if (tt == THERON_TRANSITION_STAIRS) {
        if (tile == THERON_SQUARE_STAIRS_UP) {
            world->transition_target_level = world->current_level - 1;
        } else {
            world->transition_target_level = world->current_level + 1;
        }
        /* Spawn at same grid coords on new level */
        world->transition_spawn_x = x;
        world->transition_spawn_y = y;
    } else if (tt == THERON_TRANSITION_TELEPORTER) {
        /* Phase 4: resolve teleporter target from linked_id / object DB */
        world->transition_target_level = world->current_level;
        world->transition_spawn_x = x;
        world->transition_spawn_y = y;
    } else if (tt == THERON_TRANSITION_EXIT) {
        /* Quest complete — flag for between-dungeon handler */
        world->transition_target_level = world->current_level;
        world->transition_spawn_x = x;
        world->transition_spawn_y = y;
    }

    return tt;
}

int theron_v1_transition_execute(Theron_V1_World *world) {
    Theron_V1_Level *target_level;
    int dungeon_slot;
    Theron_DungeonID next_dungeon;

    if (!world || !world->transition_pending) return -1;

    dungeon_slot = world->current_dungeon - 1;

    switch (world->transition_type) {
    case THERON_TRANSITION_STAIRS:
        if (world->transition_target_level < 0 ||
            world->transition_target_level >= THERON_MAX_LEVELS_PER_DUNGEON ||
            dungeon_slot < 0 || dungeon_slot >= THERON_DUNGEON_COUNT ||
            !world->level_loaded[dungeon_slot][world->transition_target_level]) {
            world->transition_pending = 0;
            return -1;
        }
        world->current_level = world->transition_target_level;
        target_level = &world->levels[dungeon_slot][world->current_level];
        if (world->transition_spawn_x < 0 ||
            world->transition_spawn_x >= target_level->width ||
            world->transition_spawn_y < 0 ||
            world->transition_spawn_y >= target_level->height) {
            world->party.leader_x = target_level->start_x;
            world->party.leader_y = target_level->start_y;
        } else {
            world->party.leader_x = world->transition_spawn_x;
            world->party.leader_y = world->transition_spawn_y;
        }
        break;

    case THERON_TRANSITION_TELEPORTER:
        if (world->transition_target_level < 0 ||
            world->transition_target_level >= THERON_MAX_LEVELS_PER_DUNGEON ||
            dungeon_slot < 0 || dungeon_slot >= THERON_DUNGEON_COUNT ||
            !world->level_loaded[dungeon_slot][world->transition_target_level]) {
            world->transition_pending = 0;
            return -1;
        }
        world->current_level = world->transition_target_level;
        target_level = &world->levels[dungeon_slot][world->current_level];
        if (world->transition_spawn_x < 0 ||
            world->transition_spawn_x >= target_level->width ||
            world->transition_spawn_y < 0 ||
            world->transition_spawn_y >= target_level->height) {
            world->party.leader_x = target_level->start_x;
            world->party.leader_y = target_level->start_y;
        } else {
            world->party.leader_x = world->transition_spawn_x;
            world->party.leader_y = world->transition_spawn_y;
        }
        break;

    case THERON_TRANSITION_EXIT:
        if (!world->dungeon_complete) {
            world->transition_pending = 0;
            return -1;
        }
        next_dungeon = theron_v1_dungeon_next(world->current_dungeon);
        (void)theron_v1_dungeon_advance(&world->progression);
        if (next_dungeon == THERON_DUNGEON_INVALID) {
            world->progression.quest_complete = 1;
            world->transition_pending = 0;
            theron_v1_world_runtime_media_invalidate_cache(world);
            return 0;
        }
        theron_v1_world_reset_for_dungeon(world, next_dungeon);
        world->current_dungeon = next_dungeon;
        /* The new dungeon's level 0 must already be loaded by the caller. */
        if (world->level_loaded[next_dungeon - 1][0]) {
            world->party.leader_x =
                world->levels[next_dungeon - 1][0].start_x;
            world->party.leader_y =
                world->levels[next_dungeon - 1][0].start_y;
            world->party.leader_dir =
                world->levels[next_dungeon - 1][0].start_dir;
        }
        break;

    default:
        world->transition_pending = 0;
        return -1;
    }

    world->transition_pending = 0;
    theron_v1_world_spawn_level_creatures(world);
    theron_v1_world_init_generators(world);
    theron_v1_world_runtime_media_invalidate_cache(world);
    return 0;
}

/* ══════════════════════════════════════════════════════════════════════
 * Level creature spawning
 *
 * Called after a level transition. Authentic monster records remain in the
 * source ledger until their type/graphics/AI consumer is bound.
 * ══════════════════════════════════════════════════════════════════════ */

/* Materialize one authenticated category-4 member without passing through
 * the generic fixture spawn API.  The source record owns the member HP and
 * packed cell; no host RNG, per-type template, attack, AI, speed, or loot
 * default is allowed to fill fields that the original consumer has not yet
 * been bound to a runtime witness.
 *
 * Source lock: Track 02 DungeonGroup category-4 record and the retained
 * THQUEST.ASM T500/T600 consumer boundary. */
static int theron_v1_world_admit_source_monster_member(
    Theron_V1_World *world,
    const Theron_V1_SourceMonsterRecord *record,
    unsigned int slot,
    unsigned int member_count) {
    Theron_V1_Creature *creature;
    if (!world || !record || slot >= member_count ||
        record->type >= THERON_TRACK02_CREATURE_TYPE_COUNT ||
        record->health[slot] == 0u ||
        world->creature_count >= THERON_MAX_CREATURES_PER_LEVEL)
        return -1;

    creature = &world->creatures[world->creature_count];
    memset(creature, 0, sizeof(*creature));
    /* Keep the live identity tied to the authenticated source group/member,
     * not to the current pool order.  The explicit source-spawn bridge uses
     * the same identity, so remove/reload cannot silently rename a creature
     * or make a later source record collide with an old pool index. */
    creature->id = ((int)record->source_ref << 2) | (int)slot;
    if (creature->id <= 0) creature->id = world->creature_count + 1;
    creature->type = (uint8_t)(THERON_CREATURE_AKUTUBA + record->type);
    creature->level = (uint8_t)record->level;
    creature->dungeon_id = record->dungeon_id;
    creature->x = record->x;
    creature->y = record->y;
    creature->hp = (int)record->health[slot];
    creature->max_hp = (int)record->health[slot];
    /* Category-4 admission proves the source creature and its HP/placement,
     * not its T500/T600 behaviour.  Do not turn missing source semantics
     * into a synthetic PASSIVE creature. */
    creature->ai = THERON_AI_UNAVAILABLE;
    creature->primary_attack = THERON_ATTACK_NONE;
    creature->secondary_attack = THERON_ATTACK_NONE;
    creature->flags = THERON_CF_ACTIVE;
    creature->source_ref = record->source_ref;
    creature->source_index = record->source_index;
    creature->source_chested = record->chested;
    creature->source_position = record->position;
    creature->source_slot = (uint8_t)slot;
    creature->source_cell = (uint8_t)((record->position >> (slot * 2u)) & 0x03u);
    creature->source_group_count = (uint8_t)member_count;
    creature->source_direction_flags = record->direction_flags;
    creature->source_flags_word = record->flags_word;
    creature->source_unknown_word = record->unknown_word;
    creature->source_spawn_category =
        theron_v1_world_track02_spawn_category(
            world, (unsigned int)record->type);
    creature->source_raw_size = record->raw_size;
    memcpy(creature->source_raw, record->raw,
           sizeof(creature->source_raw));
    ++world->creature_count;
    return creature->id;
}

int theron_v1_world_spawn_level_creatures(Theron_V1_World *world) {
    if (!world) return -1;
    int di = world->current_dungeon - 1;
    if (di < 0 || di >= THERON_DUNGEON_COUNT) return -1;
    int lvl = world->current_level;
    if (lvl < 0 || lvl >= THERON_MAX_LEVELS_PER_DUNGEON) return -1;
    if (!world->level_loaded[di][lvl]) return -1;

    /* Track 02 category-4 records are actual in-dungeon monster groups.  A
     * group carries the source creature type, cell byte, count and one HP
     * word per member (the same 16-byte group shape documented by the
     * DungeonGroup layout).  Materialize those bytes directly; do not use
     * the separate random-wave path here.  The latter still requires its
     * original HuC6280 RNG consumer and remains fail-closed in combat.c. */
    if (!world->source_thing_directory_verified[di]) return 0;

    unsigned int required = 0;
    for (unsigned int i = 0; i < world->source_monster_count; ++i) {
        const Theron_V1_SourceMonsterRecord *record =
            &world->source_monsters[i];
        if (record->dungeon_id != world->current_dungeon ||
            record->level != lvl) continue;
        if (record->type >= THERON_TRACK02_CREATURE_TYPE_COUNT) continue;
        /* The on-disk count is the two-bit value; actual members are value+1.
         * This is the source Group count contract, not a gameplay default. */
        unsigned int members = (unsigned int)record->number + 1u;
        if (members > 4u) members = 4u;
        for (unsigned int slot = 0; slot < members; ++slot)
            if (record->health[slot] != 0u) ++required;
    }
    if (required > THERON_MAX_CREATURES_PER_LEVEL) return -1;

    /* Creature pool is explicitly current-level state.  Rebuilding it on a
     * level entry prevents old-level records surviving a transition. */
    world->creature_count = 0;
    memset(world->creatures, 0, sizeof(world->creatures));
    for (unsigned int i = 0; i < world->source_monster_count; ++i) {
        const Theron_V1_SourceMonsterRecord *record =
            &world->source_monsters[i];
        if (record->dungeon_id != world->current_dungeon ||
            record->level != lvl) continue;
        if (record->type >= THERON_TRACK02_CREATURE_TYPE_COUNT) continue;
        unsigned int members = (unsigned int)record->number + 1u;
        if (members > 4u) members = 4u;
        {
            /* The category-4 record encodes the member count in one byte,
             * but the source record owns exactly four health words.  Keep
             * the same bounded Group-count contract in both passes; a
             * malformed or future variant record must not turn admission
             * into an out-of-bounds read. */
            unsigned int slot;
            int has_live_member = 0;
            for (slot = 0; slot < members; ++slot) {
                if (record->health[slot] != 0u) {
                    has_live_member = 1;
                    break;
                }
            }
            if (!has_live_member) continue;
        }
        /* Track 02's category-4 group stores one source HP word per live
         * member.  Admit each non-zero member directly; the regular RNG
         * overlay at $B0E5 remains a separate, still-gated path. */
        {
            unsigned int slot;
            for (slot = 0; slot < members; ++slot) {
                if (record->health[slot] == 0u) continue;
                if (theron_v1_world_admit_source_monster_member(
                        world, record, slot, members) < 0)
                    return -1;
            }
        }
    }
    return 0;
}

int theron_v1_world_bind_track02_spawn_source(
    Theron_V1_World *world,
    const Theron_Track02SpawnSource *source,
    int variant) {
    if (!world) return 0;
    memset(&world->track02_spawn_source, 0,
           sizeof(world->track02_spawn_source));
    if (variant != THERON_V1_TRACK02_VARIANT_JP_BIN &&
        variant != THERON_V1_TRACK02_VARIANT_US_BIN) {
        world->track02_spawn_source_variant = 0;
        return 0;
    }
    world->track02_spawn_source_variant = variant;
    if (variant != THERON_V1_TRACK02_VARIANT_US_BIN || !source ||
        !source->authenticated || source->variant != variant)
        return 0;
    world->track02_spawn_source = *source;
    return 1;
}

uint8_t theron_v1_world_track02_spawn_category(
    const Theron_V1_World *world,
    unsigned int creature_index) {
    const Theron_SpawnZoneDesc *zone;
    if (!world || creature_index >= THERON_TRACK02_CREATURE_TYPE_COUNT)
        return 0xffu;
    /* A regular-spawn category is a runtime-source receipt, not a property
     * of the static creature type.  Do not fall back to the reconstructed
     * descriptor table here: direct level loads and JP data have no
     * authenticated US spawn consumer, so publishing that value would make
     * inferred data look like a captured semantic field. */
    if (!world->track02_spawn_source.authenticated ||
        world->track02_spawn_source.variant !=
            THERON_V1_TRACK02_VARIANT_US_BIN)
        return 0xffu;
    if (creature_index >= THERON_TRACK02_SPAWN_ZONE_COUNT)
        return 0xffu;
    zone = &world->track02_spawn_source.zones[creature_index];
    return zone ? zone->category : 0xffu;
}

int theron_v1_world_bind_track02_monster(
    Theron_V1_World *world,
    int dungeon_id,
    int level_index,
    uint16_t source_ref,
    uint16_t source_index,
    int x,
    int y,
    uint8_t type,
    uint8_t position,
    const uint16_t health[4],
    uint8_t number,
    uint8_t direction_flags,
    uint16_t flags_word,
    uint16_t unknown_word,
    int16_t chested)
{
    /* Keep every real category-4 source record, including reserved or
     * sentinel type bytes. spawn_level_creatures() admits only the
     * authenticated 0..6 roster; no invented live creature is created for
     * an unknown source byte. */
    if (!world || !health || dungeon_id < 1 ||
        dungeon_id > THERON_DUNGEON_COUNT ||
        level_index < 0 || level_index >= THERON_MAX_LEVELS_PER_DUNGEON ||
        x < 0 || x >= THERON_MAX_MAP_SIZE ||
        y < 0 || y >= THERON_MAX_MAP_SIZE ||
        number > 3u ||
        !world->level_loaded[dungeon_id - 1][level_index] ||
        !world->levels[dungeon_id - 1][level_index].source_header_verified ||
        world->source_monster_count >= THERON_MAX_SOURCE_MONSTERS)
        return -1;
    Theron_V1_SourceMonsterRecord *out =
        &world->source_monsters[world->source_monster_count++];
    memset(out, 0, sizeof(*out));
    out->dungeon_id = dungeon_id;
    out->level = level_index;
    out->x = x;
    out->y = y;
    out->source_ref = source_ref;
    out->source_index = source_index;
    out->chested = chested;
    out->type = type;
    out->position = position;
    out->number = number;
    out->direction_flags = direction_flags;
    out->flags_word = flags_word;
    out->unknown_word = unknown_word;
    memcpy(out->health, health, sizeof(out->health));
    return 0;
}

int theron_v1_world_bind_track02_generator(
    Theron_V1_World *world,
    int dungeon_id,
    int level_index,
    uint16_t source_ref,
    uint16_t source_index,
    int x,
    int y,
    uint8_t type,
    uint16_t value,
    uint8_t once,
    uint8_t effect,
    uint8_t sound,
    uint8_t delay,
    uint8_t inactive,
    uint8_t graphism,
    uint8_t target_x,
    uint8_t target_y,
    uint8_t target_facing,
    uint8_t generator_fields_valid,
    uint8_t generator_generation,
    uint8_t generator_toughness,
    uint8_t generator_pause)
{
    if (!world || dungeon_id < 1 ||
        dungeon_id > THERON_DUNGEON_COUNT ||
        level_index < 0 || level_index >= THERON_MAX_LEVELS_PER_DUNGEON ||
        type != TQ_ACT_FLOOR_MONSTER_GEN ||
        x < 0 || x >= THERON_MAX_MAP_SIZE ||
        y < 0 || y >= THERON_MAX_MAP_SIZE ||
        !world->level_loaded[dungeon_id - 1][level_index] ||
        !world->levels[dungeon_id - 1][level_index].source_header_verified ||
        world->source_generator_count >= THERON_MAX_SOURCE_GENERATORS)
        return -1;
    Theron_V1_SourceGeneratorRecord *out =
        &world->source_generators[world->source_generator_count++];
    memset(out, 0, sizeof(*out));
    out->dungeon_id = dungeon_id;
    out->level = level_index;
    out->x = x;
    out->y = y;
    out->source_ref = source_ref;
    out->source_index = source_index;
    out->type = type;
    out->value = value;
    out->once = once;
    out->effect = effect;
    out->sound = sound;
    out->delay = delay;
    out->inactive = inactive;
    out->graphism = graphism;
    out->target_x = target_x;
    out->target_y = target_y;
    out->target_facing = target_facing;
    out->generator_fields_valid = generator_fields_valid;
    out->generator_generation = generator_generation;
    out->generator_toughness = generator_toughness;
    out->generator_pause = generator_pause;
    return 0;
}

int theron_v1_world_bind_track02_source_object(
    Theron_V1_World *world,
    int dungeon_id,
    int level_index,
    uint16_t source_ref,
    uint16_t next_ref,
    uint16_t source_index,
    uint8_t category,
    uint8_t position,
    int x,
    int y,
    const uint8_t *raw,
    uint8_t raw_size)
{
    if (!world || !raw || raw_size == 0u || raw_size > 16u ||
        dungeon_id < 1 || dungeon_id > THERON_DUNGEON_COUNT ||
        level_index < 0 || level_index >= THERON_MAX_LEVELS_PER_DUNGEON ||
        x < 0 || x >= THERON_MAX_MAP_SIZE ||
        y < 0 || y >= THERON_MAX_MAP_SIZE ||
        !world->level_loaded[dungeon_id - 1][level_index] ||
        !world->levels[dungeon_id - 1][level_index].source_header_verified ||
        world->source_object_count >= THERON_MAX_SOURCE_OBJECT_RECORDS)
        return -1;
    Theron_V1_SourceObjectRecord *out =
        &world->source_objects[world->source_object_count++];
    memset(out, 0, sizeof(*out));
    out->dungeon_id = dungeon_id;
    out->level = level_index;
    out->x = x;
    out->y = y;
    out->source_ref = source_ref;
    out->next_ref = next_ref;
    out->source_index = source_index;
    out->category = category;
    out->position = position;
    out->raw_size = raw_size;
    memcpy(out->raw, raw, raw_size);
    return 0;
}

/* ══════════════════════════════════════════════════════════════════════
 * Creature generators — DMWeb ChristopheF maps
 * ══════════════════════════════════════════════════════════════════════ */

void theron_v1_world_init_generators(Theron_V1_World *world) {
    if (!world) return;
    int di = world->current_dungeon - 1;
    if (di < 0 || di >= THERON_DUNGEON_COUNT) return;
    world->generator_active_count = 0;
    for (size_t i = 0; i < sizeof(world->generator_spawn_count) /
                              sizeof(world->generator_spawn_count[0]); i++) {
        world->generator_spawn_count[i] = 0;
        world->generator_next_tick[i] = 0;
    }
    /* The real source-generator records are retained above, but their
     * consumer/timing/re-enable path is not source-bound yet.  Do not expose
     * the old DMWeb-derived table as a production fallback when the Track 02
     * thing directory is absent, and do not turn retained source records into
     * executable generators before that consumer is authenticated. */
    return;
}

void theron_v1_world_tick_generators(Theron_V1_World *world) {
    if (!world || world->generator_active_count == 0) return;
    /* Real actuator records are retained above, but the original generator
     * consumer, timing and re-enable route are not yet source-bound. */
    (void)world;
}

/* ══════════════════════════════════════════════════════════════════════
 * World tick
 * ══════════════════════════════════════════════════════════════════════ */

void theron_v1_world_tick(Theron_V1_World *world) {
    if (!world) return;
    world->world_tick++;
    theron_v1_tick_timers(world);
    theron_v1_creature_ai_tick(world);
    theron_v1_world_tick_generators(world);
}

void theron_v1_world_runtime_media_clear(Theron_V1_World *world) {
    if (!world) return;
    memset(&world->runtime_media, 0, sizeof(world->runtime_media));
}

void theron_v1_world_runtime_media_invalidate_cache(Theron_V1_World *world) {
    if (!world) return;
    ++world->runtime_media.cache_generation;
    memset(&world->runtime_media.level_bank,
           0,
           sizeof(world->runtime_media.level_bank));
}

static int theron_v1_world_runtime_media_has_single_track02_source(
    const Theron_V1_World *world) {
    const Theron_RuntimeMediaSurface *const surfaces[] = {
        &world->runtime_media.title,
        &world->runtime_media.stage,
        &world->runtime_media.soul_room,
        &world->runtime_media.forcefield
    };
    const char *md5;
    Theron_Track02Variant variant;
    size_t i;

    if (!world || !surfaces[0]->ready || !surfaces[0]->raw_source_verified ||
        surfaces[0]->track02_md5[0] == '\0') {
        return 0;
    }
    md5 = surfaces[0]->track02_md5;
    variant = theron_v1_track02_variant_for_md5(md5);
    if (variant == THERON_TRACK02_VARIANT_UNKNOWN) {
        return 0;
    }
    for (i = 1u; i < sizeof(surfaces) / sizeof(surfaces[0]); ++i) {
        if (!surfaces[i]->ready || !surfaces[i]->raw_source_verified ||
            strcmp(surfaces[i]->track02_md5, md5) != 0) {
            return 0;
        }
    }
    return 1;
}

int theron_v1_world_runtime_media_set_surface(
    Theron_V1_World *world,
    Theron_RuntimeMediaSurfaceKind kind,
    const char *track02_md5,
    unsigned int route_bit,
    uint16_t width,
    uint16_t height,
    size_t first_raw_offset,
    size_t last_raw_offset,
    size_t first_user_data_offset,
    size_t tile_count,
    size_t nonzero_pixel_count,
    uint32_t checksum,
    const uint8_t *pixels,
    size_t pixel_count) {

    Theron_RuntimeMediaSurface *surface;

    if (!world || !pixels || !track02_md5 || strlen(track02_md5) != 32u ||
        theron_v1_track02_variant_for_md5(track02_md5) ==
            THERON_TRACK02_VARIANT_UNKNOWN ||
        route_bit == 0u || width == 0u ||
        height == 0u || width > THERON_RUNTIME_MEDIA_MAX_WIDTH ||
        height > THERON_RUNTIME_MEDIA_HEIGHT ||
        first_raw_offset == 0u || first_raw_offset > last_raw_offset ||
        first_user_data_offset == 0u ||
        pixel_count != (size_t)width * (size_t)height ||
        pixel_count > THERON_RUNTIME_MEDIA_PIXELS || tile_count == 0u ||
        nonzero_pixel_count == 0u || checksum == 0u) {
        return 0;
    }
    if (kind == THERON_RUNTIME_MEDIA_SURFACE_TITLE) {
        surface = &world->runtime_media.title;
    } else if (kind == THERON_RUNTIME_MEDIA_SURFACE_STAGE) {
        surface = &world->runtime_media.stage;
    } else if (kind == THERON_RUNTIME_MEDIA_SURFACE_SOUL_ROOM) {
        surface = &world->runtime_media.soul_room;
    } else if (kind == THERON_RUNTIME_MEDIA_SURFACE_FORCEFIELD) {
        surface = &world->runtime_media.forcefield;
    } else {
        return 0;
    }
    {
        const Theron_RuntimeMediaSurface *const surfaces[] = {
            &world->runtime_media.title,
            &world->runtime_media.stage,
            &world->runtime_media.soul_room,
            &world->runtime_media.forcefield
        };
        size_t i;

        for (i = 0u; i < sizeof(surfaces) / sizeof(surfaces[0]); ++i) {
            if (surfaces[i] != surface && surfaces[i]->ready &&
                strcmp(surfaces[i]->track02_md5, track02_md5) != 0) {
                return 0;
            }
        }
    }
    memset(surface, 0, sizeof(*surface));
    surface->ready = 1;
    surface->raw_source_verified = 1;
    snprintf(surface->track02_md5, sizeof(surface->track02_md5), "%s",
             track02_md5);
    surface->route_bit = route_bit;
    surface->width = width;
    surface->height = height;
    surface->first_raw_offset = first_raw_offset;
    surface->last_raw_offset = last_raw_offset;
    surface->first_user_data_offset = first_user_data_offset;
    surface->tile_count = tile_count;
    surface->nonzero_pixel_count = nonzero_pixel_count;
    surface->checksum = checksum;
    memcpy(surface->pixels, pixels, pixel_count);
    world->runtime_media.route_mask |= route_bit;
    world->runtime_media.checksum ^= checksum + (uint32_t)route_bit;
    world->runtime_media.restored =
        world->runtime_media.title.ready &&
        world->runtime_media.stage.ready &&
        world->runtime_media.soul_room.ready &&
        world->runtime_media.forcefield.ready;
    theron_v1_world_runtime_media_invalidate_cache(world);
    return 1;
}

const Theron_RuntimeMediaSurface *theron_v1_world_runtime_media_for_level(
    const Theron_V1_World *world,
    int level_index,
    int forcefield_active) {

    if (!world || !world->runtime_media.restored) return NULL;
    if (forcefield_active && world->runtime_media.forcefield.ready) {
        return &world->runtime_media.forcefield;
    }
    if (level_index == 0 && world->runtime_media.soul_room.ready) {
        return &world->runtime_media.soul_room;
    }
    if (level_index > 0 && world->runtime_media.stage.ready) {
        return &world->runtime_media.stage;
    }
    return NULL;
}

int theron_v1_world_runtime_media_set_identity(
    Theron_V1_World *world,
    const Theron_RuntimeMediaIdentity *identity) {

    if (!world || !identity || !identity->ready ||
        identity->track02_variant == 0 || identity->bank_stride == 0u) {
        return 0;
    }
    world->runtime_media.identity = *identity;
    theron_v1_world_runtime_media_invalidate_cache(world);
    return 1;
}

int theron_v1_world_runtime_media_set_loader_record(
    Theron_V1_World *world,
    const char *track02_md5,
    uint32_t record,
    uint32_t destination,
    size_t raw_user_data_offset,
    size_t payload_bytes,
    uint32_t payload_checksum,
    size_t level_envelope_offset,
    size_t level_envelope_bytes,
    uint32_t level_envelope_checksum,
    size_t post_envelope_offset,
    size_t post_envelope_bytes,
    uint32_t post_envelope_checksum) {

    Theron_RuntimeTrack02LoaderRecord *loader_record;

    if (!world || !track02_md5 || strlen(track02_md5) != 32u ||
        theron_v1_track02_variant_for_md5(track02_md5) ==
            THERON_TRACK02_VARIANT_UNKNOWN ||
        record == 0u || destination == 0u || raw_user_data_offset == 0u ||
        raw_user_data_offset % THERON_TRACK02_RAW_SECTOR_BYTES !=
            THERON_TRACK02_RAW_USER_DATA_OFFSET ||
        payload_bytes == 0u || payload_checksum == 0u ||
        level_envelope_offset >= payload_bytes || level_envelope_bytes == 0u ||
        level_envelope_bytes > payload_bytes - level_envelope_offset ||
        level_envelope_checksum == 0u ||
        post_envelope_offset != level_envelope_offset + level_envelope_bytes ||
        post_envelope_offset >= payload_bytes || post_envelope_bytes == 0u ||
        post_envelope_bytes > payload_bytes - post_envelope_offset ||
        post_envelope_checksum == 0u) {
        return 0;
    }
    loader_record = &world->runtime_media.loader_record;
    memset(loader_record, 0, sizeof(*loader_record));
    loader_record->ready = 1;
    loader_record->raw_source_verified = 1;
    loader_record->no_semantic_promotion = 1;
    snprintf(loader_record->track02_md5,
             sizeof(loader_record->track02_md5), "%s", track02_md5);
    loader_record->record = record;
    loader_record->destination = destination;
    loader_record->raw_user_data_offset = raw_user_data_offset;
    loader_record->payload_bytes = payload_bytes;
    loader_record->payload_checksum = payload_checksum;
    loader_record->level_envelope_bound = 1;
    loader_record->level_envelope_offset = level_envelope_offset;
    loader_record->level_envelope_bytes = level_envelope_bytes;
    loader_record->level_envelope_checksum = level_envelope_checksum;
    loader_record->post_envelope_offset = post_envelope_offset;
    loader_record->post_envelope_bytes = post_envelope_bytes;
    loader_record->post_envelope_checksum = post_envelope_checksum;
    return 1;
}

int theron_v1_world_runtime_media_select_level_bank(
    Theron_V1_World *world,
    Theron_RuntimeLevelBankKind kind,
    Theron_DungeonID dungeon_id,
    int level_index) {

    const Theron_RuntimeMediaSurface *surface = NULL;
    Theron_RuntimeLevelBankSelection selection;

    if (!world || !world->runtime_media.restored ||
        !theron_v1_world_runtime_media_has_single_track02_source(world) ||
        !world->runtime_media.identity.ready ||
        world->runtime_media.identity.track02_variant !=
            (int)theron_v1_track02_variant_for_md5(
                world->runtime_media.title.track02_md5) ||
        dungeon_id < THERON_DUNGEON_1_AKUTUBA ||
        dungeon_id > THERON_DUNGEON_COUNT ||
        level_index < 0 || level_index >= THERON_MAX_LEVELS_PER_DUNGEON) {
        return 0;
    }

    switch (kind) {
    case THERON_RUNTIME_LEVEL_BANK_STARTUP_FORCEFIELD:
        surface = &world->runtime_media.forcefield;
        break;
    case THERON_RUNTIME_LEVEL_BANK_SAVE_RESUME:
        surface = &world->runtime_media.stage;
        break;
    case THERON_RUNTIME_LEVEL_BANK_LATER_LEVEL:
        surface = theron_v1_world_runtime_media_for_level(world,
                                                          level_index,
                                                          0);
        break;
    case THERON_RUNTIME_LEVEL_BANK_NONE:
    default:
        return 0;
    }
    if (!surface || !surface->ready) {
        return 0;
    }

    memset(&selection, 0, sizeof(selection));
    selection.ready = 1;
    selection.real_media_gate = 1;
    selection.kind = kind;
    selection.dungeon_id = dungeon_id;
    selection.level_index = level_index;
    selection.route_bit = surface->route_bit;
    selection.raw_source_verified = surface->raw_source_verified;
    snprintf(selection.track02_md5, sizeof(selection.track02_md5), "%s",
             surface->track02_md5);
    selection.first_raw_offset = surface->first_raw_offset;
    selection.last_raw_offset = surface->last_raw_offset;
    selection.first_user_data_offset = surface->first_user_data_offset;
    selection.width = surface->width;
    selection.height = surface->height;
    selection.tile_count = surface->tile_count;
    selection.nonzero_pixel_count = surface->nonzero_pixel_count;
    selection.surface_checksum = surface->checksum;
    selection.identity_checksum = world->runtime_media.identity.checksum;
    selection.cache_generation = ++world->runtime_media.cache_generation;
    world->runtime_media.level_bank = selection;
    return 1;
}

int theron_v1_world_runtime_media_bind_level_data_block(
    Theron_V1_World *world,
    const uint8_t *user_data,
    size_t user_data_size,
    int track02_variant,
    unsigned int level) {

    Theron_LevelDataBlockReceipt source;
    Theron_RuntimeLevelDataBlockReceipt receipt;
    Theron_Track02Variant variant = (Theron_Track02Variant)track02_variant;

    if (!world || !user_data || user_data_size == 0u ||
        !world->runtime_media.restored ||
        !world->runtime_media.identity.ready ||
        (variant != THERON_TRACK02_VARIANT_US_BIN &&
         variant != THERON_TRACK02_VARIANT_JP_BIN &&
         variant != THERON_TRACK02_VARIANT_US_ISO &&
         variant != THERON_TRACK02_VARIANT_JP_REV1_ISO) ||
        world->runtime_media.identity.track02_variant != track02_variant ||
        !theron_v1_track02_level_data_block_read(
            user_data, user_data_size, variant, level, &source)) {
        return 0;
    }

    memset(&receipt, 0, sizeof(receipt));
    receipt.ready = 1;
    receipt.no_semantic_promotion = 1;
    receipt.track02_variant = track02_variant;
    receipt.level = source.level;
    receipt.block_ud_offset = source.block_ud_offset;
    receipt.compressed_ud_offset = source.compressed_ud_offset;
    receipt.resource_end_ud_offset = source.resource_end_ud_offset;
    receipt.compressed_bytes = source.compressed_bytes;
    receipt.resource_length = source.resource_length;
    receipt.compressed_fnv1a = source.compressed_fnv1a;
    receipt.shared_prologue_fnv1a = source.shared_prologue_fnv1a;
    memcpy(receipt.per_level_meta, source.per_level_meta,
           sizeof(receipt.per_level_meta));
    world->runtime_media.later_level_data = receipt;
    return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Deterministic world-state hashing (FNV-1a 64-bit)
 *
 * Per THQUEST.ASM T700 (world tick) + T900 (object DB state):
 *   - Party position and direction
 *   - All active timer states
 *   - All object states (type, state, flags, position)
 *   - Dungeon progression (quest items, current dungeon)
 *   - World tick counter
 * ══════════════════════════════════════════════════════════════════════ */

uint64_t theron_v1_world_hash(const Theron_V1_World *world) {
    uint64_t h = THERON_HASH_FNV_OFFSET;
    if (!world) return 0;

    /* Seed: party state */
    h = fnv64_word(h, THERON_HASH_SEED_PARTY);
    h = fnv64_word(h, (uint64_t)world->current_dungeon);
    h = fnv64_word(h, (uint64_t)world->current_level);
    h = fnv64_word(h, (uint64_t)world->quest_items_in_dungeon);
    h = fnv64_word(h, (uint64_t)world->dungeon_complete);
    h = fnv64_word(h, world->world_tick);

    /* Seed: object database */
    h = fnv64_word(h, THERON_HASH_SEED_OBJECT);
    h = fnv64_word(h, (uint64_t)world->object_count);
    for (int i = 0; i < world->object_count; i++) {
        const Theron_V1_Object *o = &world->objects[i];
        h = fnv64_word(h, (uint64_t)(o->type  & 0xFF));
        h = fnv64_word(h, (uint64_t)(o->state & 0xFF));
        h = fnv64_word(h, (uint64_t)(o->x) | ((uint64_t)(o->y) << 8));
        h = fnv64_word(h, (uint64_t)o->flags);
    }

    /* Seed: creature roster (source-locked combat state) */
    h = fnv64_word(h, THERON_HASH_SEED_CREATURE);
    h = fnv64_word(h, (uint64_t)world->creature_count);
    for (int i = 0; i < world->creature_count; i++) {
        const Theron_V1_Creature *c = &world->creatures[i];
        h = fnv64_word(h, (uint64_t)(c->type & 0xFF));
        h = fnv64_word(h, (uint64_t)(c->x) | ((uint64_t)(c->y) << 8));
        h = fnv64_word(h, (uint64_t)c->hp);
        h = fnv64_word(h, (uint64_t)c->flags);
    }

    /* Source provenance is part of the save identity.  T900/T700 may not
     * consume these records yet, but dropping them from the hash would allow
     * two different authenticated Track 02 states to compare equal. */
    h = fnv64_word(h, THERON_HASH_SEED_SOURCE);
    h = fnv64_word(h, world->source_monster_count);
    for (unsigned int i = 0; i < world->source_monster_count; ++i) {
        const Theron_V1_SourceMonsterRecord *r = &world->source_monsters[i];
        h = fnv64_word(h, (uint64_t)r->dungeon_id);
        h = fnv64_word(h, (uint64_t)r->level);
        h = fnv64_word(h, (uint64_t)r->x | ((uint64_t)r->y << 16));
        h = fnv64_word(h, (uint64_t)r->source_ref |
                              ((uint64_t)r->source_index << 16));
        h = fnv64_word(h, (uint64_t)r->type | ((uint64_t)r->position << 8) |
                              ((uint64_t)r->number << 16));
        h = fnv64_bytes(h, r->raw, r->raw_size <= sizeof(r->raw) ?
                                      r->raw_size : sizeof(r->raw));
    }
    h = fnv64_word(h, world->source_object_count);
    for (unsigned int i = 0; i < world->source_object_count; ++i) {
        const Theron_V1_SourceObjectRecord *r = &world->source_objects[i];
        h = fnv64_word(h, (uint64_t)r->dungeon_id);
        h = fnv64_word(h, (uint64_t)r->level);
        h = fnv64_word(h, (uint64_t)r->x | ((uint64_t)r->y << 16));
        h = fnv64_word(h, (uint64_t)r->source_ref |
                              ((uint64_t)r->next_ref << 16));
        h = fnv64_word(h, (uint64_t)r->source_index |
                              ((uint64_t)r->category << 16) |
                              ((uint64_t)r->position << 24));
        h = fnv64_bytes(h, r->raw, r->raw_size <= sizeof(r->raw) ?
                                      r->raw_size : sizeof(r->raw));
    }
    h = fnv64_word(h, world->source_generator_count);
    for (unsigned int i = 0; i < world->source_generator_count; ++i) {
        const Theron_V1_SourceGeneratorRecord *r =
            &world->source_generators[i];
        h = fnv64_word(h, (uint64_t)r->dungeon_id);
        h = fnv64_word(h, (uint64_t)r->level);
        h = fnv64_word(h, (uint64_t)r->x | ((uint64_t)r->y << 16));
        h = fnv64_word(h, (uint64_t)r->source_ref |
                              ((uint64_t)r->source_index << 16));
        h = fnv64_word(h, (uint64_t)r->type | ((uint64_t)r->value << 8) |
                              ((uint64_t)r->once << 24) |
                              ((uint64_t)r->effect << 32) |
                              ((uint64_t)r->delay << 40));
    }
    for (int champion = 0; champion < THERON_MAX_CHAMPIONS; ++champion) {
        for (int slot = 0; slot < THERON_INVENTORY_SLOTS; ++slot) {
            const Theron_V1_InventorySourceRecord *r =
                &world->inventory_source[champion][slot];
            h = fnv64_word(h, (uint64_t)r->valid |
                                  ((uint64_t)r->category << 8) |
                                  ((uint64_t)r->item_type << 16) |
                                  ((uint64_t)r->item_category << 24));
            h = fnv64_word(h, (uint64_t)r->source_ref |
                                  ((uint64_t)r->source_next_ref << 16));
            h = fnv64_word(h, (uint64_t)r->source_index |
                                  ((uint64_t)r->text_ref << 16));
            h = fnv64_bytes(h, r->property, sizeof(r->property));
            h = fnv64_bytes(h, r->source_raw,
                            r->source_raw_size <= sizeof(r->source_raw) ?
                                r->source_raw_size : sizeof(r->source_raw));
        }
    }

    /* Seed: timers */
    h = fnv64_word(h, THERON_HASH_SEED_TIMER);
    for (int i = 0; i < world->timer_count; i++) {
        const Theron_V1_Timer *t = &world->timers[i];
        if (!(t->flags & THERON_TIMER_F_ACTIVE)) continue;
        h = fnv64_word(h, (uint64_t)t->id);
        h = fnv64_word(h, (uint64_t)t->kind);
        h = fnv64_word(h, (uint64_t)t->remaining_ticks);
        h = fnv64_word(h, (uint64_t)t->interval_ticks);
        h = fnv64_word(h, (uint64_t)t->flags);
    }

    /* Seed: dungeon state */
    h = fnv64_word(h, THERON_HASH_SEED_DUNG);
    h = fnv64_word(h, (uint64_t)world->progression.quest_items_collected);
    h = fnv64_word(h, (uint64_t)world->progression.current_dungeon);
    h = fnv64_word(h, (uint64_t)world->progression.quest_complete);

    return h;
}

void theron_v1_world_hash_inject(Theron_V1_World *world, uint64_t seed) {
    if (!world) return;
    world->state_hash = seed;
}

/* ══════════════════════════════════════════════════════════════════════
 * Quest item helpers
 * ══════════════════════════════════════════════════════════════════════ */

int theron_v1_check_quest_item(const Theron_V1_World *world) {
    if (!world) return 0;
    /* Returns the per-dungeon quest item bit; 0 if already found */
    uint8_t found = world->progression.quest_items_collected;
    uint8_t dungeon_bit = (uint8_t)(1U << (world->current_dungeon - 1));
    return (found & dungeon_bit) ? 0 : dungeon_bit;
}

uint8_t theron_v1_collect_quest_item(Theron_V1_World *world, uint8_t item_bit_fixed) {
    if (!world) return 0;
    world->progression.quest_items_collected |= item_bit_fixed;
    world->quest_items_in_dungeon++;

    /* Check if dungeon is now complete */
    const Theron_DungeonMeta *meta =
        theron_v1_dungeon_meta((Theron_DungeonID)world->current_dungeon);
    if (meta && world->quest_items_in_dungeon >= meta->quest_item_count) {
        world->dungeon_complete = 1;
    }
    return world->progression.quest_items_collected;
}

/* ================================================================ */
/* Binary serialization                                         */
/* ================================================================ */

static size_t serialize_size(const Theron_V1_World *world) {
    if (!world) return 0;
    if (world->object_count < 0 || world->object_count > THERON_MAX_OBJECTS ||
        world->timer_count < 0 || world->timer_count > THERON_MAX_TIMERS ||
        world->creature_count < 0 ||
        world->creature_count > THERON_MAX_CREATURES_PER_LEVEL ||
        world->source_generator_count > THERON_MAX_SOURCE_GENERATORS ||
        world->source_object_count > THERON_MAX_SOURCE_OBJECT_RECORDS) {
        return 0;
    }
    size_t n = 0;
    n += sizeof(uint32_t); /* magic */
    n += sizeof(uint16_t); /* version */
    n += sizeof(uint16_t); /* pad */
    n += sizeof(uint8_t);  /* current_dungeon */
    n += sizeof(uint8_t);  /* current_level */
    n += sizeof(uint8_t);  /* quest_items_in_dungeon */
    n += sizeof(uint8_t);  /* dungeon_complete */
    n += sizeof(Theron_DungeonProgression);
    n += _tqw_party_pack_size();
    n += sizeof(uint32_t); /* object_count */
    n += (size_t)world->object_count * theron_object_wire_size();
    n += sizeof(uint32_t); /* timer_count */
    n += (size_t)world->timer_count * THERON_TIMER_WIRE_BYTES;
    n += sizeof(uint64_t); /* world_tick */
    n += sizeof(uint64_t); /* state_hash */
    /* T900 inventory provenance: source object/category/property fields have
     * no pointers and can be appended without changing existing offsets. */
    n += theron_inventory_source_wire_size();
    n += sizeof(uint32_t); /* live creature_count */
    n += (size_t)world->creature_count *
         theron_creature_wire_size_for_version(THERON_WORLD_SAVE_VERSION);
    n += sizeof(uint32_t); /* source generator count */
    n += (size_t)world->source_generator_count * theron_generator_wire_size();
    n += sizeof(world->generator_spawn_count);
    n += sizeof(world->generator_next_tick);
    n += sizeof(world->generator_active_count);
    n += sizeof(uint32_t); /* source object count */
    n += (size_t)world->source_object_count *
         THERON_SOURCE_OBJECT_WIRE_BYTES;
    return n;
}

size_t theron_v1_world_serialize_size(const Theron_V1_World *world) {
    return serialize_size(world);
}

static size_t theroned_world_serialize(const Theron_V1_World *world,
                                  void *buf, size_t bufsize) {
    if (!world || !buf) return 0;
    size_t need = serialize_size(world);
    if (bufsize < need) return 0;
    uint8_t *out = (uint8_t *)buf;
    memset(out, 0, need);

    ww32(out, THERON_WORLD_SAVE_MAGIC);
    out += sizeof(uint32_t);
    out[0] = (uint8_t)THERON_WORLD_SAVE_VERSION;
    out[1] = 0;
    out += sizeof(uint16_t) * 2;
    *out++ = (uint8_t)world->current_dungeon;
    *out++ = (uint8_t)world->current_level;
    *out++ = world->quest_items_in_dungeon;
    *out++ = world->dungeon_complete;

    memcpy(out, &world->progression, sizeof(world->progression));
    out += sizeof(world->progression);

    _tqw_party_pack(&world->party, out, bufsize - (out - (uint8_t *)buf));
    out += _tqw_party_pack_size();

    ww32(out, (uint32_t)world->object_count);
    out += sizeof(uint32_t);
    for (int i = 0; i < world->object_count; ++i) {
        out = theron_object_write(out, &world->objects[i]);
    }

    ww32(out, (uint32_t)world->timer_count);
    out += sizeof(uint32_t);
    for (int i = 0; i < world->timer_count; ++i) {
        out = theron_timer_write(out, &world->timers[i]);
    }

    ww64(out, world->world_tick);
    out += sizeof(uint64_t);
    ww64(out, world->state_hash);
    out += sizeof(uint64_t);
    for (int champion = 0; champion < THERON_MAX_CHAMPIONS; ++champion) {
        for (int slot = 0; slot < THERON_INVENTORY_SLOTS; ++slot) {
            out = theron_inventory_source_write(
                out, &world->inventory_source[champion][slot]);
        }
    }
    ww32(out, (uint32_t)world->creature_count);
    out += sizeof(uint32_t);
    for (int i = 0; i < world->creature_count; ++i) {
        out = theron_creature_write(out, &world->creatures[i]);
    }

    ww32(out, world->source_generator_count);
    out += sizeof(uint32_t);
    for (unsigned int i = 0; i < world->source_generator_count; ++i) {
        out = theron_generator_write(out, &world->source_generators[i]);
    }
    for (size_t i = 0; i < sizeof(world->generator_spawn_count) /
                           sizeof(world->generator_spawn_count[0]); ++i) {
        ww32(out, (uint32_t)world->generator_spawn_count[i]);
        out += sizeof(uint32_t);
    }
    for (size_t i = 0; i < sizeof(world->generator_next_tick) /
                           sizeof(world->generator_next_tick[0]); ++i) {
        ww64(out, world->generator_next_tick[i]);
        out += sizeof(uint64_t);
    }
    ww32(out, (uint32_t)world->generator_active_count);
    out += sizeof(uint32_t);
    ww32(out, world->source_object_count);
    out += sizeof(uint32_t);
    for (unsigned int i = 0; i < world->source_object_count; ++i) {
        out = theron_source_object_write(out, &world->source_objects[i]);
    }

    return need;
}

size_t theron_v1_world_serialize(const Theron_V1_World *world,
                                 void *buf, size_t bufsize) {
    return theroned_world_serialize(world, buf, bufsize);
}

int theron_v1_world_deserialize(Theron_V1_World *world,
                                 const void *buf, size_t bufsize) {
    if (!world || !buf) return -1;
    const uint8_t *in = (const uint8_t *)buf;
    const size_t fixed_size = sizeof(uint32_t) + sizeof(uint16_t) * 2 +
        sizeof(uint8_t) * 4 + sizeof(Theron_DungeonProgression) +
        _tqw_party_pack_size() + sizeof(uint32_t) + sizeof(uint32_t) +
        sizeof(uint64_t) * 2;
    if (bufsize < fixed_size) return -1;

    uint32_t magic = rw32(in);
    if (magic != THERON_WORLD_SAVE_MAGIC) return -2;
    in += sizeof(uint32_t);

    uint16_t ver = rw16(in);
    if (ver != 1u && ver != 2u && ver != 3u && ver != 4u && ver != 5u &&
        ver != 6u && ver != 7u && ver != 8u &&
        ver != 9u && ver != 10u &&
        ver != THERON_WORLD_SAVE_VERSION) return -3;
    const int legacy_host_records = (ver == 1u);
    in += sizeof(uint16_t) * 2;

    world->current_dungeon          = *in++;
    world->current_level            = *in++;
    world->quest_items_in_dungeon   = *in++;
    world->dungeon_complete         = *in++;

    memcpy(&world->progression, in, sizeof(world->progression));
    in += sizeof(world->progression);

    if (_tqw_party_unpack(&world->party, in,
                               bufsize - (in - (const uint8_t *)buf)) != 0) {
        return -4;
    }
    in += _tqw_party_pack_size();

    if ((size_t)(in - (const uint8_t *)buf) > bufsize - sizeof(uint32_t)) return -1;
    uint32_t oc = rw32(in);
    in += sizeof(uint32_t);
    if (oc > THERON_MAX_OBJECTS) return -1;
    world->object_count = (int)oc;
    size_t objsz = (size_t)oc * (legacy_host_records ?
        sizeof(Theron_V1_Object) : theron_object_wire_size());
    if (objsz > bufsize - (size_t)(in - (const uint8_t *)buf)) return -1;
    if (legacy_host_records) {
        memcpy(world->objects, in, objsz);
        in += objsz;
    } else {
        for (uint32_t i = 0; i < oc; ++i) {
            in = theron_object_read(in, &world->objects[i]);
        }
    }

    if ((size_t)(in - (const uint8_t *)buf) > bufsize - sizeof(uint32_t)) return -1;
    uint32_t tc = rw32(in);
    in += sizeof(uint32_t);
    if (tc > THERON_MAX_TIMERS) return -1;
    world->timer_count = (int)tc;
    size_t tmsz = (size_t)tc * (legacy_host_records ?
        sizeof(Theron_V1_Timer) : THERON_TIMER_WIRE_BYTES);
    if (tmsz > bufsize - (size_t)(in - (const uint8_t *)buf) ||
        bufsize - (size_t)(in - (const uint8_t *)buf) - tmsz < sizeof(uint64_t) * 2) {
        return -1;
    }
    if (legacy_host_records) {
        memcpy(world->timers, in, tmsz);
        in += tmsz;
    } else {
        for (uint32_t i = 0; i < tc; ++i) {
            in = theron_timer_read(in, &world->timers[i]);
        }
    }

    world->world_tick = rw64(in);
    in += sizeof(uint64_t);
    world->state_hash = rw64(in);
    in += sizeof(uint64_t);

    /* Version 1 snapshots produced before source inventory provenance was
     * appended remain readable. A partial trailing section is malformed and
     * must not silently clear item semantics. Version 3 appends a live
     * creature section; version 4 appends the decoded generator records and
     * their runtime counters after that section. */
    size_t remaining = bufsize - (size_t)(in - (const uint8_t *)buf);
    memset(world->inventory_source, 0, sizeof(world->inventory_source));
    world->creature_count = 0;
    memset(world->creatures, 0, sizeof(world->creatures));
    world->source_generator_count = 0;
    memset(world->source_generators, 0, sizeof(world->source_generators));
    world->source_object_count = 0;
    memset(world->source_objects, 0, sizeof(world->source_objects));
    memset(world->generator_spawn_count, 0,
           sizeof(world->generator_spawn_count));
    memset(world->generator_next_tick, 0,
           sizeof(world->generator_next_tick));
    world->generator_active_count = 0;
    if (ver >= 4u && remaining == 0u) return -1;
    if (remaining != 0u) {
        size_t inventory_wire = theron_inventory_source_wire_size_for_version(ver);
        if (remaining == sizeof(world->inventory_source)) {
            /* Compatibility with the first source-provenance tail format. */
            memcpy(world->inventory_source, in,
                   sizeof(world->inventory_source));
        } else if (remaining == inventory_wire ||
                   (ver >= 3u &&
                    remaining >= inventory_wire + sizeof(uint32_t))) {
            for (int champion = 0; champion < THERON_MAX_CHAMPIONS;
                 ++champion) {
                for (int slot = 0; slot < THERON_INVENTORY_SLOTS; ++slot) {
                    in = theron_inventory_source_read(
                        in, &world->inventory_source[champion][slot], ver);
                }
            }
            remaining -= inventory_wire;
            if (ver >= 3u) {
                if (remaining < sizeof(uint32_t)) return -1;
                uint32_t creature_count = rw32(in);
                in += sizeof(uint32_t);
                remaining -= sizeof(uint32_t);
                const size_t creature_wire =
                    theron_creature_wire_size_for_version(ver);
                if (creature_count > THERON_MAX_CREATURES_PER_LEVEL ||
                    ((size_t)creature_count * creature_wire != remaining &&
                     ver == 3u)) return -1;
                if ((size_t)creature_count * creature_wire >
                    remaining) return -1;
                world->creature_count = (int)creature_count;
                for (uint32_t i = 0; i < creature_count; ++i) {
                    in = theron_creature_read(in, &world->creatures[i], ver);
                }
                remaining -= (size_t)creature_count * creature_wire;
                if (ver >= 4u) {
                    const size_t runtime_slots = (ver == 4u) ?
                        THERON_LEGACY_GENERATOR_RUNTIME_SLOTS :
                        THERON_MAX_SOURCE_GENERATORS;
                    const size_t runtime_tail =
                        runtime_slots * sizeof(uint32_t) +
                        runtime_slots * sizeof(uint64_t) +
                        sizeof(world->generator_active_count) +
                        (ver >= 11u ? sizeof(uint32_t) : 0u);
                    if (remaining < sizeof(uint32_t) + runtime_tail)
                        return -1;
                    uint32_t generator_count = rw32(in);
                    in += sizeof(uint32_t);
                    remaining -= sizeof(uint32_t);
                    const size_t generator_wire =
                        theron_generator_wire_size_for_version(ver);
                    if (generator_count > THERON_MAX_SOURCE_GENERATORS ||
                        (ver < 11u &&
                         (size_t)generator_count * generator_wire +
                             runtime_tail != remaining) ||
                        (ver >= 11u &&
                         (size_t)generator_count * generator_wire +
                             runtime_tail > remaining)) return -1;
                    world->source_generator_count = generator_count;
                    for (uint32_t i = 0; i < generator_count; ++i) {
                        in = theron_generator_read(
                            in, &world->source_generators[i], ver);
                    }
                    for (size_t i = 0; i < runtime_slots; ++i) {
                        world->generator_spawn_count[i] = (int32_t)rw32(in);
                        in += sizeof(uint32_t);
                    }
                    for (size_t i = 0; i < runtime_slots; ++i) {
                        world->generator_next_tick[i] = rw64(in);
                        in += sizeof(uint64_t);
                    }
                    world->generator_active_count = (int32_t)rw32(in);
                    in += sizeof(uint32_t);
                    if (world->generator_active_count < 0 ||
                        world->generator_active_count >
                            (int32_t)THERON_MAX_SOURCE_GENERATORS) return -1;
                    if (ver >= 11u) {
                        const size_t fixed_runtime_tail =
                            runtime_slots * sizeof(uint32_t) +
                            runtime_slots * sizeof(uint64_t) +
                            sizeof(world->generator_active_count);
                        const size_t source_object_tail =
                            remaining - (size_t)generator_count *
                                generator_wire - fixed_runtime_tail;
                        uint32_t source_object_count = rw32(in);
                        in += sizeof(uint32_t);
                        if (source_object_count >
                                THERON_MAX_SOURCE_OBJECT_RECORDS ||
                            source_object_tail != sizeof(uint32_t) +
                                (size_t)source_object_count *
                                    THERON_SOURCE_OBJECT_WIRE_BYTES) {
                            return -1;
                        }
                        world->source_object_count = source_object_count;
                        for (uint32_t i = 0; i < source_object_count; ++i) {
                            in = theron_source_object_read(
                                in, &world->source_objects[i]);
                        }
                    }
                }
            }
        } else {
            return -1;
        }
    }

    return 0;
}

/* ── Source evidence ───────────────────────────────────────────────── */

const char *theron_v1_world_source_evidence(void) {
    return "THQUEST.ASM T400/T520/T560/T600/T700/T800/T900  "
           "+ tqr_v1_phase0_provenance_gate_H2339.md";
}

/* ══════════════════════════════════════════════════════════════════════
 * First-room synthetic fixture + readiness gate
 *
 * Source-lock: THQUEST.ASM T520 (party placement) and
 *   THQUEST.ASM T560 (dungeon loading, 12-byte header).  See
 *   docs/source-lock/tqr_v1_phase1_boot_H2338.md.
 *
 * These helpers exist so the V1 startup probe can exercise the
 * real theron_v1_level_load() path end-to-end without staging the
 * proprietary Track 02 BIN/ISO in CI.  They emit a buffer that
 * matches the documented 12-byte header + grid layout, with a
 * deterministic floor-and-wall pattern that lets the probe assert
 * party placement, wall-block, and forward-move results.
 *
 * The readiness gate lets the same probe take a real-Track-02 path
 * when a user has staged the asset (skip-safe: returns a status
 * instead of pretending the asset exists).  No original game data
 * is ever synthesized by these helpers.
 * ══════════════════════════════════════════════════════════════════════ */

#if defined(THERON_WORLD_FIXTURE_HELPERS)

/* Fixture/CI constructors are not part of the production Theron archive. */
size_t theron_v1_first_room_buffer_size(int width, int height) {
    if (width <= 0 || height <= 0) return 0;
    if (width > THERON_MAX_MAP_SIZE || height > THERON_MAX_MAP_SIZE) return 0;
    return (size_t)THERON_V1_FIRST_ROOM_HEADER_BYTES +
           (size_t)width * (size_t)height;
}

size_t theron_v1_first_room_synthesize(uint8_t *out_buf,
                                        size_t buf_size,
                                        int width,
                                        int height,
                                        int level_index,
                                        uint32_t dungeon_seed,
                                        Theron_V1_Level *out_level) {
    if (!out_buf || !out_level) return 0;
    if (width <= 0 || height <= 0) return 0;
    if (width > THERON_MAX_MAP_SIZE || height > THERON_MAX_MAP_SIZE) return 0;

    size_t needed = (size_t)THERON_V1_FIRST_ROOM_HEADER_BYTES +
                     (size_t)width * (size_t)height;
    if (buf_size < needed) return 0;

    /* Header is big-endian on disk for width/height/level_index (matches
     * theron_v1_level_load() in this file, which uses rb16/rb32 helpers
     * that read big-endian uint16/uint32). */
    out_buf[0] = (uint8_t)((width  >> 8) & 0xFFu);
    out_buf[1] = (uint8_t)( width        & 0xFFu);
    out_buf[2] = (uint8_t)((height >> 8) & 0xFFu);
    out_buf[3] = (uint8_t)( height       & 0xFFu);
    out_buf[4] = (uint8_t)((dungeon_seed >> 24) & 0xFFu);
    out_buf[5] = (uint8_t)((dungeon_seed >> 16) & 0xFFu);
    out_buf[6] = (uint8_t)((dungeon_seed >>  8) & 0xFFu);
    out_buf[7] = (uint8_t)( dungeon_seed        & 0xFFu);
    out_buf[8]  = (uint8_t)((level_index >> 8) & 0xFFu);
    out_buf[9]  = (uint8_t)( level_index       & 0xFFu);
    out_buf[10] = 0u;
    out_buf[11] = 0u;

    uint8_t *grid = out_buf + THERON_V1_FIRST_ROOM_HEADER_BYTES;
    /* Fill with walls, then carve the documented first-room pattern:
     *   - (1,1)            : entrance floor
     *   - (2,1)            : forward-step floor
     *   - (3,1)            : forward-step floor (lets probe do 2 moves)
     *   - (4,1)            : STAIRS_DOWN tile (forward-step special)
     *   - (1,2)            : floor for sideways / backwards movement
     */
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            grid[y * width + x] = THERON_SQUARE_WALL;
        }
    }
    if (width  >= 5) grid[1 * width + 1] = THERON_SQUARE_FLOOR;
    if (width  >= 5) grid[1 * width + 2] = THERON_SQUARE_FLOOR;
    if (width  >= 5) grid[1 * width + 3] = THERON_SQUARE_FLOOR;
    if (width  >= 5) grid[1 * width + 4] = THERON_SQUARE_STAIRS_DOWN;
    if (height >= 3) grid[2 * width + 1] = THERON_SQUARE_FLOOR;

    /* Mirror the layout into the level preview so callers can assert
     * party placement directly without re-running level_load().  This
     * is a probe-time convenience: theron_v1_level_load() will rebuild
     * the same squares on the real Track 02 path. */
    memset(out_level, 0, sizeof(*out_level));
    out_level->level_index = level_index;
    out_level->width       = width;
    out_level->height      = height;
    out_level->dungeon_seed = dungeon_seed;
    out_level->source_header_level_index = (uint16_t)level_index;
    out_level->start_x     = 1;
    out_level->start_y     = 1;
    /* 1 = EAST (matches THERON_DIR_EAST in theron_v1_mechanics.h).
     * Inlined here so this module stays free of the mechanics
     * header dependency. */
    out_level->start_dir   = 1;
    out_level->thing_count = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            out_level->squares[y][x] = grid[y * width + x];
        }
    }

    return needed;
}

size_t theron_v1_startup_fallback_room_synthesize(uint8_t *out_buf,
                                                   size_t buf_size,
                                                   Theron_DungeonID dungeon_id,
                                                   Theron_V1_Level *out_level) {
    /* Synthetic fallback room for no-media testing. The 32x27 dimensions
     * and seed were previously derived from a false reading of the level
     * descriptor table at UD 0x619900. The real dungeon map topology format
     * remains unknown and requires HuC6280 code disassembly to discover. */
    const uint32_t seed = 0u;
    const uint16_t level_index = 0u;
    const int width = 32;
    const int height = 27;
    const int start_x = 2;
    const int start_y = 1;
    const int start_dir = 1; /* EAST, matching THERON_DIR_EAST */
    const int exit_x = 30;
    const int exit_y = 25;
    uint8_t *grid;
    size_t needed;
    int x;
    int y;

    if (!out_buf || !out_level) return 0;
    if (dungeon_id < THERON_DUNGEON_1_AKUTUBA ||
        dungeon_id > THERON_DUNGEON_COUNT) {
        return 0;
    }

    needed = theron_v1_first_room_buffer_size(width, height);
    if (needed == 0u || buf_size < needed) return 0;

    memset(out_buf, 0, needed);
    out_buf[0] = (uint8_t)((width >> 8) & 0xFFu);
    out_buf[1] = (uint8_t)(width & 0xFFu);
    out_buf[2] = (uint8_t)((height >> 8) & 0xFFu);
    out_buf[3] = (uint8_t)(height & 0xFFu);
    out_buf[4] = (uint8_t)((seed >> 24) & 0xFFu);
    out_buf[5] = (uint8_t)((seed >> 16) & 0xFFu);
    out_buf[6] = (uint8_t)((seed >> 8) & 0xFFu);
    out_buf[7] = (uint8_t)(seed & 0xFFu);
    out_buf[8] = (uint8_t)((level_index >> 8) & 0xFFu);
    out_buf[9] = (uint8_t)(level_index & 0xFFu);
    out_buf[10] = 0u;
    out_buf[11] = 0u;

    grid = out_buf + THERON_V1_FIRST_ROOM_HEADER_BYTES;
    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            grid[y * width + x] =
                (x == 0 || y == 0 || x == width - 1 || y == height - 1)
                    ? THERON_SQUARE_WALL
                    : THERON_SQUARE_FLOOR;
        }
    }
    /* Keep the northern edge entrance that theron_v1_level_load() finds
     * first, matching the observed (4,0) entrance in real-data logs. */
    grid[0 * width + 4] = THERON_SQUARE_FLOOR;
    /* A distant exit gives no-media tests a reachable transition target. */
    grid[exit_y * width + exit_x] = THERON_SQUARE_EXIT;

    memset(out_level, 0, sizeof(*out_level));
    out_level->level_index = 0;
    out_level->width = width;
    out_level->height = height;
    out_level->dungeon_seed = 0u;
    out_level->source_header_level_index = 0;
    out_level->start_x = (int16_t)start_x;
    out_level->start_y = (int16_t)start_y;
    out_level->start_dir = (int8_t)start_dir;
    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            out_level->squares[y][x] = grid[y * width + x];
        }
    }

    return needed;
}

#endif /* THERON_WORLD_FIXTURE_HELPERS */

/* ── libcs-only MD5 (RFC 1321) ──────────────────────────────────────
 * We bundle a tiny MD5 implementation here so the readiness gate can
 * hash staged Track 02 files without depending on OpenSSL.  It is
 * local to theron_v1_world.c and not exported.  Source: RFC 1321
 * reference, public-domain reference implementation.
 */
typedef struct {
    uint32_t state[4];
    uint64_t count;
    uint8_t  buffer[64];
} Theron_LocalMd5;

static void md5_init(Theron_LocalMd5 *ctx) {
    ctx->state[0] = 0x67452301u;
    ctx->state[1] = 0xEFCDAB89u;
    ctx->state[2] = 0x98BADCFEu;
    ctx->state[3] = 0x10325476u;
    ctx->count    = 0u;
}

#define F1(x, y, z) ((z) ^ ((x) & ((y) ^ (z))))
#define F2(x, y, z) F1((z), (x), (y))
#define F3(x, y, z) ((x) ^ (y) ^ (z))
#define F4(x, y, z) ((y) ^ ((x) | ~(z)))

#define STEP(f, a, b, c, d, x, t, s) \
    (a) += f((b), (c), (d)) + (x) + (t); \
    (a) = ((a) << (s)) | ((a) >> (32 - (s))); \
    (a) += (b);

static void md5_transform(Theron_LocalMd5 *ctx, const uint8_t block[64]) {
    uint32_t a, b, c, d, x[16];
    for (int i = 0; i < 16; i++) {
        x[i] = ((uint32_t)block[i*4])         |
               ((uint32_t)block[i*4 + 1] << 8) |
               ((uint32_t)block[i*4 + 2] << 16) |
               ((uint32_t)block[i*4 + 3] << 24);
    }
    a = ctx->state[0]; b = ctx->state[1];
    c = ctx->state[2]; d = ctx->state[3];

    STEP(F1, a, b, c, d, x[ 0], 0xD76AA478u,  7)
    STEP(F1, d, a, b, c, x[ 1], 0xE8C7B756u, 12)
    STEP(F1, c, d, a, b, x[ 2], 0x242070DBu, 17)
    STEP(F1, b, c, d, a, x[ 3], 0xC1BDCEEEu, 22)
    STEP(F1, a, b, c, d, x[ 4], 0xF57C0FAFu,  7)
    STEP(F1, d, a, b, c, x[ 5], 0x4787C62Au, 12)
    STEP(F1, c, d, a, b, x[ 6], 0xA8304613u, 17)
    STEP(F1, b, c, d, a, x[ 7], 0xFD469501u, 22)
    STEP(F1, a, b, c, d, x[ 8], 0x698098D8u,  7)
    STEP(F1, d, a, b, c, x[ 9], 0x8B44F7AFu, 12)
    STEP(F1, c, d, a, b, x[10], 0xFFFF5BB1u, 17)
    STEP(F1, b, c, d, a, x[11], 0x895CD7BEu, 22)
    STEP(F1, a, b, c, d, x[12], 0x6B901122u,  7)
    STEP(F1, d, a, b, c, x[13], 0xFD987193u, 12)
    STEP(F1, c, d, a, b, x[14], 0xA679438Eu, 17)
    STEP(F1, b, c, d, a, x[15], 0x49B40821u, 22)

    STEP(F2, a, b, c, d, x[ 1], 0xF61E2562u,  5)
    STEP(F2, d, a, b, c, x[ 6], 0xC040B340u,  9)
    STEP(F2, c, d, a, b, x[11], 0x265E5A51u, 14)
    STEP(F2, b, c, d, a, x[ 0], 0xE9B6C7AAu, 20)
    STEP(F2, a, b, c, d, x[ 5], 0xD62F105Du,  5)
    STEP(F2, d, a, b, c, x[10], 0x02441453u,  9)
    STEP(F2, c, d, a, b, x[15], 0xD8A1E681u, 14)
    STEP(F2, b, c, d, a, x[ 4], 0xE7D3FBC8u, 20)
    STEP(F2, a, b, c, d, x[ 9], 0x21E1CDE6u,  5)
    STEP(F2, d, a, b, c, x[14], 0xC33707D6u,  9)
    STEP(F2, c, d, a, b, x[ 3], 0xF4D50D87u, 14)
    STEP(F2, b, c, d, a, x[ 8], 0x455A14EDu, 20)
    STEP(F2, a, b, c, d, x[13], 0xA9E3E905u,  5)
    STEP(F2, d, a, b, c, x[ 2], 0xFCEFA3F8u,  9)
    STEP(F2, c, d, a, b, x[ 7], 0x676F02D9u, 14)
    STEP(F2, b, c, d, a, x[12], 0x8D2A4C8Au, 20)

    STEP(F3, a, b, c, d, x[ 5], 0xFFFA3942u,  4)
    STEP(F3, d, a, b, c, x[ 8], 0x8771F681u, 11)
    STEP(F3, c, d, a, b, x[11], 0x6D9D6122u, 16)
    STEP(F3, b, c, d, a, x[14], 0xFDE5380Cu, 23)
    STEP(F3, a, b, c, d, x[ 1], 0xA4BEEA44u,  4)
    STEP(F3, d, a, b, c, x[ 4], 0x4BDECFA9u, 11)
    STEP(F3, c, d, a, b, x[ 7], 0xF6BB4B60u, 16)
    STEP(F3, b, c, d, a, x[10], 0xBEBFBC70u, 23)
    STEP(F3, a, b, c, d, x[13], 0x289B7EC6u,  4)
    STEP(F3, d, a, b, c, x[ 0], 0xEAA127FAu, 11)
    STEP(F3, c, d, a, b, x[ 3], 0xD4EF3085u, 16)
    STEP(F3, b, c, d, a, x[ 6], 0x04881D05u, 23)
    STEP(F3, a, b, c, d, x[ 9], 0xD9D4D039u,  4)
    STEP(F3, d, a, b, c, x[12], 0xE6DB99E5u, 11)
    STEP(F3, c, d, a, b, x[15], 0x1FA27CF8u, 16)
    STEP(F3, b, c, d, a, x[ 2], 0xC4AC5665u, 23)

    STEP(F4, a, b, c, d, x[ 0], 0xF4292244u,  6)
    STEP(F4, d, a, b, c, x[ 7], 0x432AFF97u, 10)
    STEP(F4, c, d, a, b, x[14], 0xAB9423A7u, 15)
    STEP(F4, b, c, d, a, x[ 5], 0xFC93A039u, 21)
    STEP(F4, a, b, c, d, x[12], 0x655B59C3u,  6)
    STEP(F4, d, a, b, c, x[ 3], 0x8F0CCC92u, 10)
    STEP(F4, c, d, a, b, x[10], 0xFFEFF47Du, 15)
    STEP(F4, b, c, d, a, x[ 1], 0x85845DD1u, 21)
    STEP(F4, a, b, c, d, x[ 8], 0x6FA87E4Fu,  6)
    STEP(F4, d, a, b, c, x[15], 0xFE2CE6E0u, 10)
    STEP(F4, c, d, a, b, x[ 6], 0xA3014314u, 15)
    STEP(F4, b, c, d, a, x[13], 0x4E0811A1u, 21)
    STEP(F4, a, b, c, d, x[ 4], 0xF7537E82u,  6)
    STEP(F4, d, a, b, c, x[11], 0xBD3AF235u, 10)
    STEP(F4, c, d, a, b, x[ 2], 0x2AD7D2BBu, 15)
    STEP(F4, b, c, d, a, x[ 9], 0xEB86D391u, 21)

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
}

static void md5_update(Theron_LocalMd5 *ctx, const uint8_t *data, size_t len) {
    size_t left = (size_t)(ctx->count & 0x3F);
    size_t fill = 64u - left;
    ctx->count += len;

    if (left && len >= fill) {
        memcpy(ctx->buffer + left, data, fill);
        md5_transform(ctx, ctx->buffer);
        data += fill;
        len  -= fill;
        left  = 0u;
    }
    while (len >= 64u) {
        md5_transform(ctx, data);
        data += 64u;
        len  -= 64u;
    }
    if (len) memcpy(ctx->buffer + left, data, len);
}

static void md5_final(Theron_LocalMd5 *ctx, uint8_t out[16]) {
    static const uint8_t pad[64] = { 0x80u };
    uint8_t bitlen[8];
    uint64_t bits = ctx->count * 8u;
    for (int i = 0; i < 8; i++) {
        bitlen[i] = (uint8_t)(bits >> (8 * i));
    }
    size_t left = (size_t)(ctx->count & 0x3F);
    size_t padlen = (left < 56u) ? (56u - left) : (120u - left);
    md5_update(ctx, pad, padlen);
    md5_update(ctx, bitlen, 8u);

    for (int i = 0; i < 4; i++) {
        out[i*4    ] = (uint8_t)( ctx->state[i]        & 0xFFu);
        out[i*4 + 1] = (uint8_t)((ctx->state[i] >>  8) & 0xFFu);
        out[i*4 + 2] = (uint8_t)((ctx->state[i] >> 16) & 0xFFu);
        out[i*4 + 3] = (uint8_t)((ctx->state[i] >> 24) & 0xFFu);
    }
}

static int md5_file(const char *path, char hex_out[33]) {
    FILE *fp = fopen(path, "rb");
    if (!fp) return -1;
    Theron_LocalMd5 ctx;
    md5_init(&ctx);
    uint8_t buf[4096];
    size_t got;
    while ((got = fread(buf, 1, sizeof(buf), fp)) > 0) {
        md5_update(&ctx, buf, got);
    }
    fclose(fp);
    uint8_t digest[16];
    md5_final(&ctx, digest);
    static const char hex[] = "0123456789abcdef";
    for (int i = 0; i < 16; i++) {
        hex_out[i*2    ] = hex[(digest[i] >> 4) & 0x0Fu];
        hex_out[i*2 + 1] = hex[ digest[i]       & 0x0Fu];
    }
    hex_out[32] = '\0';
    return 0;
}

/* ── Readiness gate ───────────────────────────────────────────────── */

static const char *g_known_track02_md5s[4] = {
    "b7afb338ad31be1025b53f9aff12d73a", /* JP Track 02 BIN */
    "f23601102138f87c33025877767ebf76", /* US Track 02 BIN */
    "397039af02d50d15c70b74088eb8a1cb", /* JP Rev 1 ISO */
    "ceb02343868f80cec899e9b239aff2da"  /* US ISO */
};

static const char *g_track02_candidate_names[4] = {
    "Theron's Quest (Japan) (Track 02).bin",
    "Theron's Quest (US) (Track 02).bin",
    "TQJP02End.iso",
    "TQUS02End.iso"
};

/* Compose "<root>/<file>" into `out` (size-bounded).  Returns 1 on
 * success, 0 on overflow. */
static int join_path(char *out, size_t out_size, const char *root, const char *file) {
    if (!out || out_size == 0 || !root || !file) return 0;
    size_t rl = strlen(root);
    int needs_sep = (rl > 0 && root[rl - 1] != '/' && root[rl - 1] != '\\');
    int n = snprintf(out, out_size, "%s%s%s",
                     root, needs_sep ? "/" : "", file);
    return (n > 0 && (size_t)n < out_size);
}

Theron_RuntimeReadinessStatus theron_v1_runtime_readiness(
    const char *data_root,
    char *scan_out,
    size_t scan_out_size,
    char *md5_out,
    size_t md5_out_size) {
    if (!scan_out || scan_out_size == 0 || !md5_out || md5_out_size < 33) {
        return THERON_RUNTIME_READINESS_BAD_INPUT;
    }
    scan_out[0] = '\0';
    md5_out[0]  = '\0';

    if (!data_root || !data_root[0]) {
        return THERON_RUNTIME_READINESS_NO_DATA_ROOT;
    }

    /* Probe each candidate filename.  First hit wins for the path,
     * then we independently hash and compare against the locked-in
     * Track 02 MD5 catalog. */
    for (int i = 0; i < 4; i++) {
        char candidate[1024];
        if (!join_path(candidate, sizeof(candidate), data_root,
                       g_track02_candidate_names[i])) continue;

        FILE *probe = fopen(candidate, "rb");
        if (!probe) continue;
        fclose(probe);

        char hex[33];
        if (md5_file(candidate, hex) != 0) continue;

        int known = 0;
        for (int j = 0; j < 4; j++) {
            if (strcmp(hex, g_known_track02_md5s[j]) == 0) {
                known = 1;
                break;
            }
        }

        /* Copy the path (size-bounded).  We use snprintf to avoid
         * relying on strncpy semantics. */
        snprintf(scan_out, scan_out_size, "%s", candidate);
        snprintf(md5_out,  md5_out_size,  "%s", hex);

        if (!known) {
            return THERON_RUNTIME_READINESS_NOT_VERIFIED;
        }
        return THERON_RUNTIME_READINESS_OK;
    }

    return THERON_RUNTIME_READINESS_NO_TRACK02;
}

const char *theron_v1_runtime_readiness_status_name(
    Theron_RuntimeReadinessStatus status) {
    switch (status) {
    case THERON_RUNTIME_READINESS_OK:           return "ok";
    case THERON_RUNTIME_READINESS_NO_DATA_ROOT: return "no-data-root";
    case THERON_RUNTIME_READINESS_NO_TRACK02:   return "no-track02";
    case THERON_RUNTIME_READINESS_NOT_VERIFIED: return "not-verified";
    case THERON_RUNTIME_READINESS_BAD_INPUT:    return "bad-input";
    default:                                    return "unknown";
    }
}

int theron_v1_world_load_dungeon_text(Theron_V1_World *world,
                                       const uint16_t *codons,
                                       unsigned int codon_count)
{
    if (!world) return -1;
    world->dungeon_text_count = 0;
    if (!codons || codon_count == 0) return 0;

    Theron_TextBlock tb;
    if (theron_v1_track02_text_decode(codons, codon_count, &tb) != 0)
        return -1;

    /* The authentic Track 02 stream is retained by the decoder for
     * diagnostics, but braces are unresolved source control codes until the
     * original HuC6280 text consumer is identified.  Never expose those
     * candidate strings as gameplay/UI text. */
    if (tb.diagnostic_only || tb.unresolved_control_codes != 0)
        return 0;

    unsigned int max = tb.count < 64 ? tb.count : 64;
    for (unsigned int i = 0; i < max; i++) {
        size_t len = strlen(tb.strings[i]);
        if (len >= 256) len = 255;
        memcpy(world->dungeon_texts[i], tb.strings[i], len);
        world->dungeon_texts[i][len] = '\0';
    }
    world->dungeon_text_count = max;
    return (int)max;
}

const char *theron_v1_world_dungeon_text(const Theron_V1_World *world,
                                          unsigned int text_index)
{
    if (!world || text_index >= world->dungeon_text_count) return NULL;
    return world->dungeon_texts[text_index];
}
