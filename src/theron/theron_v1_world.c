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
#include <string.h>
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

/* ── Read helpers (big-endian 68000 → host LE) ─────────────────────── */

static uint16_t rb16(const uint8_t *p) {
    return ((uint16_t)p[0] << 8) | (uint16_t)p[1];
}
static uint32_t rb32(const uint8_t *p) {
    return ((uint32_t)rb16(p) << 16) | rb16(p + 2);
}

/* ── Default companion template ────────────────────────────────────── */
/* Minimal starting stats for companion slots 1-3.
 * Real stats set by player during character creation. */
static const Theron_V1_Champion g_blank_companion = {
    .name           = {0},
    .portrait_index = 0,
    .primary_class  = THERON_CLASS_FIGHTER,
    .alive          = 1,
    .health         = 10,  .max_health  = 10,
    .stamina        = 10,  .max_stamina = 10,
    .mana           =  0,  .max_mana    =  0,
    .strength       = 10,
    .dexterity      = 10,
    .wisdom         = 10,
    .vitality       = 10,
    .anti_magic     =  0,
    .anti_fire      =  0,
    .fighter_level  =  1,
    .ninja_level    =  0,
    .priest_level   =  0,
    .wizard_level   =  0,
    .wounds         =  0,
    .attributes     =  0,
    .inventory      = {0},
    .slots          = {-1,-1,-1,-1,-1,-1,-1,-1,-1},
    .load           =  0,
    .max_load       = 180,   /* (10<<3)+100 */
    .food           = 10,
    .water          = 10,
};

/* ══════════════════════════════════════════════════════════════════════
 * Party management
 * ══════════════════════════════════════════════════════════════════════ */

size_t theron_v1_party_pack_size(void) {
    return (size_t)THERON_MAX_CHAMPIONS * sizeof(Theron_V1_Champion)
           + sizeof(uint32_t);   /* gold */
}

size_t theron_v1_champion_block_size(void) {
    return sizeof(Theron_V1_Champion);
}

void theron_v1_party_init(Theron_V1_Party *party, int dungeon_index) {
    if (!party) return;
    memset(party, 0, sizeof(*party));
    party->champion_count = THERON_MAX_CHAMPIONS;
    party->active_slot    = THERON_CHAMPION_SLOT_THERON;
    party->gold          = 0;

    /* Theron — default starting stats (fighter, 3rd level) */
    Theron_V1_Champion *t = &party->champions[THERON_CHAMPION_SLOT_THERON];
    snprintf(t->name, sizeof(t->name), "Theron");
    t->portrait_index = 0;
    t->primary_class  = THERON_CLASS_FIGHTER;
    t->alive          = 1;
    t->health         = 50;   t->max_health  = 50;
    t->stamina        = 50;   t->max_stamina = 50;
    t->mana           =  0;   t->max_mana    =  0;
    t->strength       = 18;
    t->dexterity      = 12;
    t->wisdom         = 10;
    t->vitality       = 14;
    t->anti_magic     =  8;
    t->anti_fire      =  8;
    t->fighter_level  =  3;
    t->ninja_level    =  0;
    t->priest_level   =  0;
    t->wizard_level   =  0;
    t->wounds         =  0;
    t->attributes     =  0;
    memset(t->inventory, 0, sizeof(t->inventory));
    for (int i = 0; i < THERON_EQUIP_SLOT_COUNT; i++) t->slots[i] = -1;
    t->load           =  0;
    t->max_load       = (18 << 3) + 100;
    t->food           = 20;
    t->water          = 20;

    /* Companions 1-3 — blank slots (stats assigned at party creation) */
    for (int i = 1; i < THERON_MAX_CHAMPIONS; i++) {
        party->champions[i] = g_blank_companion;
        snprintf(party->champions[i].name, sizeof(party->champions[i].name),
                 "Companion%d", i);
    }
    (void)dungeon_index; /* future: companion roster selection per dungeon */
}

void theron_v1_party_dungeon_entry_reset(Theron_V1_Party *party) {
    if (!party) return;
    for (int i = THERON_CHAMPION_SLOT_COMPANION_1; i < THERON_MAX_CHAMPIONS; i++) {
        Theron_V1_Champion *c = &party->champions[i];
        memset(c->inventory, 0, sizeof(c->inventory));
        for (int s = 0; s < THERON_EQUIP_SLOT_COUNT; s++) {
            c->slots[s] = -1;
        }
        int str = c->strength > 0 ? c->strength : 10;
        c->max_load = (str << 3) + 100;
        c->load     = 0;
        /* Stats, skills, wounds, food, water intentionally preserved */
    }
    /* Theron (slot 0): inventory, equipment, stats, skills — all persist */
}

void theron_v1_party_dungeon_exit(Theron_V1_Party *party) {
    (void)party;
    /* No state mutation on exit — saved as-is by between-dungeon save */
}

Theron_V1_Champion *theron_v1_party_getChampion(Theron_V1_Party *p, int slot) {
    if (!p || slot < 0 || slot >= THERON_MAX_CHAMPIONS) return NULL;
    return &p->champions[slot];
}

Theron_V1_Champion *theron_v1_party_leader(Theron_V1_Party *p) {
    if (!p) return NULL;
    return &p->champions[p->active_slot];
}

int theron_v1_party_theron_alive(const Theron_V1_Party *p) {
    if (!p) return 0;
    return p->champions[THERON_CHAMPION_SLOT_THERON].alive;
}

int16_t theron_v1_party_total_health(const Theron_V1_Party *p) {
    if (!p) return 0;
    int16_t total = 0;
    for (int i = 0; i < THERON_MAX_CHAMPIONS; i++) {
        total += p->champions[i].health;
    }
    return total;
}

int theron_v1_champion_is_alive(const Theron_V1_Champion *c) {
    return c && c->alive;
}

int theron_v1_champion_skill_level(const Theron_V1_Champion *c) {
    if (!c) return 0;
    switch (c->primary_class) {
        case THERON_CLASS_FIGHTER: return c->fighter_level;
        case THERON_CLASS_NINJA:   return c->ninja_level;
        case THERON_CLASS_PRIEST:  return c->priest_level;
        case THERON_CLASS_WIZARD:  return c->wizard_level;
        default: return 0;
    }
}

void theron_v1_champion_reset_inventory(Theron_V1_Champion *c) {
    if (!c) return;
    memset(c->inventory, 0, sizeof(c->inventory));
    for (int i = 0; i < THERON_EQUIP_SLOT_COUNT; i++) c->slots[i] = -1;
    c->load = 0;
}

void theron_v1_party_recalculate_loads(Theron_V1_Party *p) {
    if (!p) return;
    for (int i = 0; i < THERON_MAX_CHAMPIONS; i++) {
        Theron_V1_Champion *c = &p->champions[i];
        int base = (c->strength > 0 ? c->strength : 10) << 3;
        c->max_load = base + 100;
        /* Wound penalties */
        if (c->wounds & THERON_WOUND_LEGS)  c->max_load -= 20;
        if (c->wounds & THERON_WOUND_ARMS)  c->max_load -= 10;
        if (c->wounds & THERON_WOUND_HEAD)  c->max_load -=  5;
    }
}

/* ── Party pack / unpack ───────────────────────────────────────────── */

size_t theron_v1_party_pack(const Theron_V1_Party *p, void *buf, size_t bufsize) {
    if (!p || !buf) return 0;
    size_t need = theron_v1_party_pack_size();
    if (bufsize < need) return 0;
    uint8_t *out = (uint8_t *)buf;
    memcpy(out, &p->gold, sizeof(p->gold));
    out += sizeof(p->gold);
    for (int i = 0; i < THERON_MAX_CHAMPIONS; i++) {
        memcpy(out, &p->champions[i], sizeof(p->champions[i]));
        out += sizeof(p->champions[i]);
    }
    return need;
}

int theron_v1_party_unpack(Theron_V1_Party *p, const void *buf, size_t bufsize) {
    if (!p || !buf || bufsize < theron_v1_party_pack_size()) return -1;
    const uint8_t *in = (const uint8_t *)buf;
    memcpy(&p->gold, in, sizeof(p->gold));
    in += sizeof(p->gold);
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

void theron_v1_world_init(Theron_V1_World *world) {
    if (!world) return;
    memset(world, 0, sizeof(*world));
    world->current_dungeon    = THERON_DUNGEON_1_HALL_OF_RECORDS;
    world->current_level      = 0;
    world->world_tick        = 0;
    world->transition_pending = 0;
    world->state_hash        = THERON_HASH_FNV_OFFSET;
    theron_v1_dungeon_progression_init(&world->progression);
    theron_v1_party_init(&world->party, world->current_dungeon);
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
    world->object_count              = 0;
    world->timer_count               = 0;
    memset(world->objects, 0, sizeof(world->objects));
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
    (void)seed; /* Phase 2: wire dungeon_seed into progression table */

    if (w == 0 || w > THERON_MAX_MAP_SIZE || h == 0 || h > THERON_MAX_MAP_SIZE) {
        return THERON_MAP_ERR_INVALID_GRID;
    }
    level->width  = w;
    level->height = h;
    level->start_dir = 0; /* default north; THQUEST.ASM T520 overrides */

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
    (void)world; (void)x; (void)y; (void)dir;
    /* Phase 4: world->levels[did-1][lvl].start_x/y/dir stores party pos */
}

/* ══════════════════════════════════════════════════════════════════════
 * Object database
 * ══════════════════════════════════════════════════════════════════════ */

int theron_v1_object_place(Theron_V1_World *world, Theron_V1_Object *object) {
    if (!world || !object) return -1;
    if (world->object_count >= THERON_MAX_OBJECTS) return -1;
    Theron_V1_Object *o = &world->objects[world->object_count++];
    *o = *object;
    o->id = world->object_count;
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

Theron_V1_Object *theron_v1_object_at(Theron_V1_World *world,
                                        int level, int x, int y) {
    if (!world) return NULL;
    for (int i = 0; i < world->object_count; i++) {
        Theron_V1_Object *o = &world->objects[i];
        if (o->level == level && o->x == x && o->y == y) return o;
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
static struct {
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
    if (!world || !world->transition_pending) return -1;
    world->transition_pending = 0;
    return 0;
}

/* ══════════════════════════════════════════════════════════════════════
 * World tick
 * ══════════════════════════════════════════════════════════════════════ */

void theron_v1_world_tick(Theron_V1_World *world) {
    if (!world) return;
    world->world_tick++;
    /* Tick 1: timers */
    theron_v1_tick_timers(world);
    /* Tick 2: Phase 4 creature AI (placeholder) */
    /* Tick 3: ambient effects placeholder */
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
    size_t n = 0;
    n += sizeof(uint32_t); /* magic */
    n += sizeof(uint16_t); /* version */
    n += sizeof(uint16_t); /* pad */
    n += sizeof(uint8_t);  /* current_dungeon */
    n += sizeof(uint8_t);  /* current_level */
    n += sizeof(uint8_t);  /* quest_items_in_dungeon */
    n += sizeof(uint8_t);  /* dungeon_complete */
    n += sizeof(Theron_DungeonProgression);
    n += theron_v1_party_pack_size();
    n += sizeof(uint32_t); /* object_count */
    n += (size_t)world->object_count * sizeof(Theron_V1_Object);
    n += sizeof(uint32_t); /* timer_count */
    n += (size_t)world->timer_count * sizeof(Theron_V1_Timer);
    n += sizeof(uint64_t); /* world_tick */
    n += sizeof(uint64_t); /* state_hash */
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

    *(uint32_t *)out = THERON_WORLD_SAVE_MAGIC;
    out += sizeof(uint32_t);
    *(uint16_t *)out = THERON_WORLD_SAVE_VERSION;
    out += sizeof(uint16_t) * 2;
    *out++ = (uint8_t)world->current_dungeon;
    *out++ = (uint8_t)world->current_level;
    *out++ = world->quest_items_in_dungeon;
    *out++ = world->dungeon_complete;

    memcpy(out, &world->progression, sizeof(world->progression));
    out += sizeof(world->progression);

    theron_v1_party_pack(&world->party, out, bufsize - (out - (uint8_t *)buf));
    out += theron_v1_party_pack_size();

    *(uint32_t *)out = (uint32_t)world->object_count;
    out += sizeof(uint32_t);
    size_t objsz = (size_t)world->object_count * sizeof(Theron_V1_Object);
    memcpy(out, world->objects, objsz);
    out += objsz;

    *(uint32_t *)out = (uint32_t)world->timer_count;
    out += sizeof(uint32_t);
    size_t tmsz = (size_t)world->timer_count * sizeof(Theron_V1_Timer);
    /* Zero userdata pointers before serializing */
    Theron_V1_Timer tms[THERON_MAX_TIMERS];
    memcpy(tms, world->timers, tmsz);
    for (int i = 0; i < world->timer_count; i++) tms[i].userdata = NULL;
    memcpy(out, tms, tmsz);
    out += tmsz;

    *(uint64_t *)out = world->world_tick;
    out += sizeof(uint64_t);
    *(uint64_t *)out = world->state_hash;
    out += sizeof(uint64_t);

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
    size_t need = serialize_size(world);
    if (bufsize < need) return -1;

    uint32_t magic = *(const uint32_t *)in;
    if (magic != THERON_WORLD_SAVE_MAGIC) return -2;
    in += sizeof(uint32_t);

    uint16_t ver = *(const uint16_t *)in;
    if (ver != THERON_WORLD_SAVE_VERSION) return -3;
    in += sizeof(uint16_t) * 2;

    world->current_dungeon          = *in++;
    world->current_level            = *in++;
    world->quest_items_in_dungeon   = *in++;
    world->dungeon_complete         = *in++;

    memcpy(&world->progression, in, sizeof(world->progression));
    in += sizeof(world->progression);

    if (theron_v1_party_unpack(&world->party, in,
                               bufsize - (in - (const uint8_t *)buf)) != 0) {
        return -4;
    }
    in += theron_v1_party_pack_size();

    uint32_t oc = *(const uint32_t *)in;
    in += sizeof(uint32_t);
    world->object_count = (int)oc;
    size_t objsz = (size_t)oc * sizeof(Theron_V1_Object);
    memcpy(world->objects, in, objsz);
    in += objsz;

    uint32_t tc = *(const uint32_t *)in;
    in += sizeof(uint32_t);
    world->timer_count = (int)tc;
    size_t tmsz = (size_t)tc * sizeof(Theron_V1_Timer);
    memcpy(world->timers, in, tmsz);
    in += tmsz;

    world->world_tick = *(const uint64_t *)in;
    in += sizeof(uint64_t);
    world->state_hash = *(const uint64_t *)in;

    return 0;
}

/* ── Source evidence ───────────────────────────────────────────────── */

const char *theron_v1_world_source_evidence(void) {
    return "THQUEST.ASM T400/T520/T560/T600/T700/T800/T900  "
           "+ tqr_v1_phase0_provenance_gate_H2339.md";
}
