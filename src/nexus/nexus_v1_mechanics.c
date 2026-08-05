#include "nexus_v1_mechanics.h"
#include "nexus_v1_engine.h"
#include "nexus_v1_movement.h"
#include "nexus_v1_squares.h"
#include "nexus_v1_creatures.h"
#include "nexus_v1_combat.h"
#include "nexus_v1_inventory.h"
#include "nexus_v1_drops.h"
#include "nexus_v1_script_vm.h"
#include "nexus_v1_sound.h"
#include "nexus_v1_rasterizer.h"
#include "nexus_v1_dungeon.h"
#include "nexus_v1_magic.h"
#include "nexus_v1_automap.h"
#include "nexus_v1_light.h"
#include "nexus_v1_status.h"
#include "nexus_v1_rest.h"
#include "nexus_v1_encumbrance.h"
#include "nexus_v1_throw.h"
#include "nexus_v1_save.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* DM.BIN/ITEM.IBS presence alone does not authenticate the Saturn action
 * dispatcher. Keep the live combat/spell bridge closed until an original
 * runtime trace binds command routing, stat reads, RNG, and side effects. */
int nexus_v1_action_semantics_proven(void)
{
    return 0;
}

/* Nexus V1 mechanics — assembled game loop.
 * Combines: movement, square events, creature AI, combat, resource drain,
 * script triggers, sound triggers.
 * Source: DM1 game loop (CLIKMENU.C F0366 + FIRESTAFF game loop),
 * ReDMCSB COMMAND.C, MOVESENS.C, CHAMPION.C, CREATURE.C.
 *
 * Tick rate: 55ms (18.2 Hz) — same as DM1.
 *
 * Order per tick:
 *   1. Input queue → command dispatch
 *   2. Movement (if command == step/turn)
 *   3. Square event (stairs/teleport/pit/alarm/exit)
 *   4. Creature AI tick
 *   5. Creature attack resolution (adjacent creatures attack party)
 *   6. Resource drain (food/water every N ticks)
 *   7. Script VM events (party move, level load, etc.)
 *   8. Sound triggers
 *
 * DM1 source references:
 *   CLIKMENU.C:167-169 (stairs turn), 176-179 (movement sensor order)
 *   CLIKMENU.C:237-255 (stamina), 264-276 (stairs step), 269-323 (step result)
 *   CLIKMENU.C:325-346 (cooldown assignment)
 *   MOVESENS.C:316-328 (move result), 752-783 (walk-on/off sensors)
 *   CHAMPION.C:2025-2048 (stamina decrement), 117-130 (turn)
 *   F0209_GROUP_ProcessEvents29to41 (creature AI timeline) */

#define FOOD_DRAIN_TICKS   60
#define WATER_DRAIN_TICKS  60
#define NEXUS_MAX_LEVEL    15

void nexus_mechanics_init(Nexus_MechanicsState *st,
                            int start_x, int start_y, int start_dir) {
    if (!st) return;
    memset(st, 0, sizeof(*st));
    st->party_x = start_x;
    st->party_y = start_y;
    st->party_dir = start_dir;
    st->map_index = 0;
    st->party_alive = 1;
    st->gold_pieces = 0;
    st->game_over = 0;
    st->game_over_reason = 0;
    st->pending_level_change = -1;
    st->pending_teleport = 0;
    st->move_cooldown_ticks = 0;
    st->total_ticks = 0;
    st->food_drain_timer = FOOD_DRAIN_TICKS;
    st->water_drain_timer = WATER_DRAIN_TICKS;
    st->input_head = 0;
    st->input_tail = 0;
    st->input_count = 0;
    st->use_item_slot = -1;
    st->spell_power = -1;
    st->spell_element = -1;
    st->spell_form = -1;
    st->spell_align = -1;
}

int nexus_mechanics_push_command(Nexus_MechanicsState *st, int command) {
    if (!st || st->input_count >= 8) return -1;
    st->input_queue[st->input_tail] = command;
    st->input_tail = (st->input_tail + 1) % 8;
    st->input_count++;
    return 0;
}

int nexus_mechanics_pop_command(Nexus_MechanicsState *st, int *out_cmd) {
    if (!st || st->input_count <= 0) return 0;
    if (out_cmd) *out_cmd = st->input_queue[st->input_head];
    st->input_head = (st->input_head + 1) % 8;
    st->input_count--;
    return 1;
}

int nexus_mechanics_party_alive(const Nexus_MechanicsState *st, Nexus_V1_Engine *engine) {
    /* Check if at least one living party champion is alive.
     * engine pointer required — st does not carry engine state.
     * Source: DM1 CLIKMENU.C F0366 — party dead when all 4 champions dead.
     * A party with zero members cannot be alive. */
    int i;
    (void)st;  /* engine carries the live champion state */
    if (!engine) return 0;
    if (engine->champions.party_count == 0) return 0; /* no party = dead */
    for (i = 0; i < engine->champions.party_count; i++) {
        int ci = engine->champions.party[i];
        if (ci < 0 || ci >= engine->champions.champion_count) continue;
        if (engine->champions.champions[ci].alive) return 1;
    }
    /* All dead */
    return 0;
}

void nexus_mechanics_get_party_pos(const Nexus_MechanicsState *st,
                                     int *out_x, int *out_y, int *out_dir) {
    if (out_x) *out_x = st->party_x;
    if (out_y) *out_y = st->party_y;
    if (out_dir) *out_dir = st->party_dir;
}

void nexus_mechanics_teleport(Nexus_MechanicsState *st,
                                int target_x, int target_y, int target_level) {
    if (!st) return;
    st->pending_teleport = 1;
    st->teleport_target_x = target_x;
    st->teleport_target_y = target_y;
    st->teleport_target_level = target_level;
}

void nexus_mechanics_change_level(Nexus_MechanicsState *st, int target_level,
                                    int target_x, int target_y) {
    if (!st) return;
    st->pending_level_change = target_level;
    st->party_x = target_x;
    st->party_y = target_y;
    (void)target_y;
}

void nexus_mechanics_set_use_item_slot(Nexus_MechanicsState *st, int slot) {
    if (!st) return;
    st->use_item_slot = slot;
}

void nexus_mechanics_set_spell_runes(Nexus_MechanicsState *st,
                                     int power, int element, int form, int align) {
    if (!st) return;
    st->spell_power = power;
    st->spell_element = element;
    st->spell_form = form;
    st->spell_align = align;
}

void nexus_mechanics_clear_spell(Nexus_MechanicsState *st) {
    if (!st) return;
    st->spell_power = -1;
    st->spell_element = -1;
    st->spell_form = -1;
    st->spell_align = -1;
}

void nexus_mechanics_set_leader_slot(Nexus_MechanicsState *st, int slot) {
    if (!st) return;
    st->set_leader_slot = slot;
}

static uint64_t mechanics_fnv1a64_bytes(const uint8_t *data, size_t size) {
    uint64_t h = 1469598103934665603ULL;
    size_t i;
    if (!data) return 0;
    for (i = 0; i < size; i++) {
        h ^= data[i];
        h *= 1099511628211ULL;
    }
    return h;
}

/* Compute the actor-model signature for a Structure3 model index: FNV-1a64
 * over the extracted typed mesh rows (vertices + faces) of the exact
 * authenticated DGN buffer that loaded the level.  Returns 0 when the
 * retained buffer is unavailable or the entry cannot be extracted, which
 * degrades actor binding to the Structure3 index alone (documented in
 * nexus_v1_creature_actor_type_for).
 * Source: DMWeb DGN Structure3 model format,
 *         nexus_v1_dungeon.c nexus_v1_level_extract_structure3_mesh_entry. */
#define MECHANICS_ACTOR_MESH_MAX_VERTICES 512
#define MECHANICS_ACTOR_MESH_MAX_FACES 512
static uint64_t mechanics_structure3_model_signature(Nexus_V1_Engine *engine,
                                                     int model_index) {
    Nexus_V1_DgnStructure3Vector vertices[MECHANICS_ACTOR_MESH_MAX_VERTICES];
    Nexus_V1_DgnStructure3Face faces[MECHANICS_ACTOR_MESH_MAX_FACES];
    Nexus_V1_DgnStructure3MeshEntryReceipt receipt;
    uint64_t h = 1469598103934665603ULL;
    int i;

    if (!engine || !engine->current_level_dgn_data ||
            engine->current_level_dgn_size <= 0 || model_index < 0)
        return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!nexus_v1_level_extract_structure3_mesh_entry(
            &engine->current_level,
            engine->current_level_dgn_data,
            engine->current_level_dgn_size,
            model_index,
            vertices, MECHANICS_ACTOR_MESH_MAX_VERTICES,
            faces, MECHANICS_ACTOR_MESH_MAX_FACES,
            NULL, 0, &receipt) ||
            !receipt.valid ||
            receipt.vertex_count <= 0 ||
            receipt.vertex_count > MECHANICS_ACTOR_MESH_MAX_VERTICES ||
            receipt.face_count < 0 ||
            receipt.face_count > MECHANICS_ACTOR_MESH_MAX_FACES)
        return 0;
    for (i = 0; i < receipt.vertex_count; i++) {
        h ^= mechanics_fnv1a64_bytes((const uint8_t *)&vertices[i],
                                     sizeof(vertices[i]));
        h *= 1099511628211ULL;
    }
    for (i = 0; i < receipt.face_count; i++) {
        h ^= mechanics_fnv1a64_bytes((const uint8_t *)&faces[i],
                                     sizeof(faces[i]));
        h *= 1099511628211ULL;
    }
    return h;
}

/* Spawn creatures from authenticated DGN Structure1A actor records.
 * DMWeb documents Structure1A byte 0 as 00h/01h/02h (in LEV1.DGN only the
 * talking-head apparition model carries 01h) and Structure1B cell byte 4
 * bit 4 as "3D model is invisible by default (set only for Grey Lord in
 * LEV1.DGN)".  Across the retail LEV01-LEV15 corpus every Structure1A
 * record with kind 01h/02h is a creature actor placement: kind 01h
 * corresponds to the invisible-by-default cell bit (hidden actor), kind
 * 02h to a visible actor, and each record has exactly one Structure1B
 * owner cell.  Records with a missing or ambiguous owner stay fail-closed
 * (not spawned).  Returns the number of spawned actors.
 * Source: DMWeb DGN Structure1A/Structure1B, ReDMCSB DUNGEON.C F0151,
 *         GROUP.C F0183, CREATURE.C. */
static int mechanics_spawn_creatures_from_dgn(Nexus_V1_Engine *engine,
                                              int level_index) {
    const Nexus_V1_Level *level;
    int spawned = 0;
    int m, x, y;

    if (!engine) return 0;
    level = &engine->current_level;
    if (!level->structure1a_table_valid ||
            level->structure1a_model_count <= 0 ||
            level->structure1a_model_count > NEXUS_DGN_MAX_STRUCTURE1A_ENTRIES)
        return 0;

    for (m = 0; m < level->structure1a_model_count; m++) {
        const Nexus_V1_DgnStructure1AModel *model =
            &level->structure1a_models[m];
        int owner_x = -1, owner_y = -1, owners = 0;
        int dir, hidden;
        uint64_t signature;

        if (model->kind != 1U && model->kind != 2U) continue;

        /* Resolve the unique Structure1B owner cell for this model. */
        for (y = 0; y < NEXUS_MAX_MAP_SIZE; y++) {
            for (x = 0; x < NEXUS_MAX_MAP_SIZE; x++) {
                if (level->structure1a_owner_ref_valid[y][x] &&
                        level->structure1a_owner_refs[y][x] == (uint16_t)m) {
                    owner_x = x;
                    owner_y = y;
                    owners++;
                }
            }
        }
        if (owners != 1) continue; /* fail-closed: no unique spawn cell */

        /* Structure1A byte 2: Z rotation, 64 per quarter turn
         * (DMWeb).  Map onto NEXUS_DIR_NORTH/EAST/SOUTH/WEST. */
        dir = ((int)model->z_rotation >> 6) & 3;
        /* kind 01h == Structure1B invisible-by-default actor
         * (DMWeb: the Grey Lord on LEV1.DGN). */
        hidden = (model->kind == 1U) ? 1 : 0;
        signature = mechanics_structure3_model_signature(
            engine, (int)model->structure3_model_index);
        if (nexus_v1_creature_spawn_actor(&engine->creatures,
                                          owner_x, owner_y, dir, level_index,
                                          hidden,
                                          model->structure3_model_index,
                                          signature) >= 0) {
            spawned++;
        }
    }
    return spawned;
}

/* Load real Track 1 mechanics data for the current engine level.
 * Resets and repopulates door/teleporter/stair/pit/altar/floor-item registries
 * from authenticated DGN Structure1F records.  Synthetic fallbacks are blocked
 * when real records are present.
 * Source: DMWeb DGN Structure1F layout, DM1 MOVESENS.C/CHAMPION.C item use,
 *         ReDMCSB DUNGEON.C / COMMAND.C.
 * Returns 0 on success, -1 on bad arguments. */
int nexus_v1_mechanics_load_level(Nexus_V1_Engine *engine, int level_index) {
    const Nexus_V1_Level *level;
    int x, y, i;
    int has_real_data;

    if (!engine || level_index < 0 || level_index > 15) return -1;
    level = &engine->current_level;
    if (!engine->level_loaded || level->width != NEXUS_MAX_MAP_SIZE ||
        level->height != NEXUS_MAX_MAP_SIZE)
        return -1;

    has_real_data = level->geometry_info.structure1f_valid &&
                    level->structure1f_entry_count >= 0;

    nexus_doors_init();
    nexus_teleporters_init();
    nexus_stairs_init();
    nexus_pits_init();
    nexus_altars_init();
    nexus_floor_init();
    nexus_gold_init();
    /* Reset the active creature pool for the new level (roster types and
     * evidence-gated actor bindings survive).
     * Source: ReDMCSB GROUP.C F0183 per-map active-group pool. */
    nexus_v1_creatures_reset_active(&engine->creatures);

    /* Spawn creatures from authenticated DGN Structure1A actor records
     * before any synthetic path may run: when real actor records exist,
     * real_actor_spawn_count > 0 blocks synthetic spawn fixtures. */
    (void)mechanics_spawn_creatures_from_dgn(engine, level_index);

    if (has_real_data) {
        /* Drop real items on the floor only when ITEM.IBS has been
         * authenticated and parsed.  The DGN item_id is an IBS declaration
         * index; inventory_association gives the DM1-style inventory id used
         * by the mechanics item catalog. */
        if (engine->item_ibs_runtime_source.source_bound &&
            engine->item_ibs_bank.valid) {
            for (i = 0; i < level->structure1f_entry_count; ++i) {
                const Nexus_V1_DgnStructure1FEntry *e =
                    &level->structure1f_entries[i];
                int inv_id;
                if (e->family != NEXUS_V1_DGN_STRUCTURE1F_ITEMS) continue;
                if (e->x >= (uint8_t)level->width || e->y >= (uint8_t)level->height)
                    continue;
                if (e->item_id >= NEXUS_V1_ITEM_IBS_DECLARATION_COUNT) continue;
                inv_id = (int)engine->item_ibs_bank.inventory_association[e->item_id];
                if (inv_id < 0 || inv_id >= nexus_itemdef_count()) continue;
                if (!nexus_itemdef_get(inv_id)) continue;
                nexus_floor_drop(e->x, e->y, inv_id, 1);
            }
        }

        /* Register teleport/pit/door/stair targets from real floor and wall
         * sensors.  Wall sensors resolve to their Structure1A owner cell.
         * Where the target semantics are not source-locked, the path stays
         * fail-closed (no synthetic destination). */
        for (i = 0; i < level->structure1f_entry_count; ++i) {
            const Nexus_V1_DgnStructure1FEntry *e =
                &level->structure1f_entries[i];
            int sx, sy, sq, dx, dy, dl;
            if (e->family == NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS) {
                sx = e->x;
                sy = e->y;
            } else if (e->family == NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS &&
                       e->structure1a_relation_valid) {
                sx = e->structure1a_owner_x;
                sy = e->structure1a_owner_y;
            } else {
                continue;
            }
            if (sx < 0 || sx >= level->width || sy < 0 || sy >= level->height)
                continue;
            sq = level->squares[sy][sx];
            dx = e->destination_x;
            dy = e->destination_y;
            dl = (e->type_or_control <= 15) ? (int)e->type_or_control : -1;

            switch (sq) {
            case NEXUS_SQUARE_DOOR:
                nexus_doors_register(sx, sy);
                if (e->item_id != 0 &&
                    e->item_id < nexus_itemdef_count() &&
                    nexus_itemdef_get(e->item_id) &&
                    (nexus_itemdef_get(e->item_id)->flags & NEXUS_ITEMF_KEY)) {
                    nexus_doors_lock(sx, sy, e->item_id);
                }
                break;
            case NEXUS_SQUARE_TELEPORT:
                nexus_teleporters_register(sx, sy, dx, dy, level_index);
                break;
            case NEXUS_SQUARE_TELEPORT2:
                nexus_teleporters_register(sx, sy, dx, dy,
                    (dl >= 0) ? dl : level_index);
                break;
            case NEXUS_SQUARE_TELEPORT3:
                nexus_teleporters_register(sx, sy, dx, dy, level_index);
                break;
            case NEXUS_SQUARE_CHUTE:
                if (dl >= 0) {
                    nexus_pits_register(sx, sy, dx, dy, dl);
                }
                break;
            case NEXUS_SQUARE_STAIRS_DN:
                nexus_stairs_register(sx, sy,
                    (dl >= 0) ? dl : level_index + 1, dx, dy,
                    (int)e->destination_orientation);
                break;
            case NEXUS_SQUARE_STAIRS_UP:
                nexus_stairs_register(sx, sy,
                    (dl >= 0) ? dl : level_index - 1, dx, dy,
                    (int)e->destination_orientation);
                break;
            default:
                break;
            }
        }

        /* Floor decorations are recorded as candidate altars but remain
         * blocked: the exact altar tag/aspect is not source-locked.
         * Source: DM1 COMMAND.C altar use dispatch. */
        for (i = 0; i < level->structure1f_entry_count; ++i) {
            const Nexus_V1_DgnStructure1FEntry *e =
                &level->structure1f_entries[i];
            uint8_t tag;
            if (e->family != NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS) continue;
            if (e->x >= (uint8_t)level->width || e->y >= (uint8_t)level->height)
                continue;
            /* Register the candidate altar with its raw model/aspect tag.
             * The ritual itself stays blocked until COMMAND.C semantics are
             * source-locked. */
            tag = (uint8_t)(e->model_or_aspect & 0xFFU);
            nexus_altars_register_tagged(e->x, e->y, tag);
        }
    }

    /* Register doors from the authenticated Structure1B grid. Stairs are
     * intentionally absent here: their destination belongs to a real
     * Structure1F sensor/script record, and a same-coordinate adjacent-level
     * link would be synthetic. */
    for (y = 0; y < level->height; ++y) {
        for (x = 0; x < level->width; ++x) {
            int sq = level->squares[y][x];
            if (sq == NEXUS_SQUARE_DOOR) {
                nexus_doors_register(x, y);
            }
        }
    }

    if (engine->mechanics) {
        engine->mechanics->map_index = level_index;
    }
    return 0;
}

static int get_champion_defense(Nexus_V1_ChampionPool *pool) {
    /* Sum defense from all equipped armor on the party leader.
     * Defense comes from: head, torso, legs, feet, hands, shield, amulet.
     * Weapons contribute attack, not defense.
     * Source: DM1 CHAMPION.C combat defense calculation,
     * CHAMPION.C F0309 equipment slot layout (C05_SLOT_WEAPON..C13_SLOT_AMULET). */
    int total_def = 0;
    int leader_idx, leader_slot, item_id, i;
    Nexus_V1_Champion *leader;
    const Nexus_ItemDef *def;

    if (!pool || pool->party_count == 0) return 0;
    if (pool->leader_index < 0 || pool->leader_index >= pool->party_count) return 0;

    leader_idx = pool->party[pool->leader_index];
    if (leader_idx < 0 || leader_idx >= pool->champion_count) return 0;
    leader = &pool->champions[leader_idx];
    if (!leader->alive) return 0;

    /* Iterate all equipment slots (NEXUS_SLOT_HEAD..NEXUS_SLOT_AMULET).
     * The slot indices in the champion slots[] array match NEXUS_SLOT_* values.
     * Source: CHAMPION.C F0309 slot layout, nexus_v1_champions.h NEXUS_SLOT_COUNT.
     * Slot mapping: HEAD=1, TORSO=2, LEGS=3, FEET=4, HANDS=5,
     *                WEAPON=6, SHIELD=7, RING1=8, RING2=9, AMULET=10. */
    for (leader_slot = 1; leader_slot <= 10; leader_slot++) {
        if (leader_slot == 6 || leader_slot == 8 || leader_slot == 9) {
            /* WEAPON, RING1, RING2 — these contribute attack not defense.
             * Source: DM1 combat — defense only from armor slots. */
            continue;
        }
        item_id = leader->slots[leader_slot - 1];
        if (item_id < 0) continue;
        def = nexus_itemdef_get(item_id);
        if (def && def->defense > 0) {
            total_def += def->defense;
        }
    }

    /* Ring slots can also contribute defense in some DM1 configurations.
     * Rings (RING1=8, RING2=9) sometimes give defense if equipped.
     * Source: DM1 ring slot defense. */
    for (i = 8; i <= 9; i++) {
        item_id = leader->slots[i];
        if (item_id < 0) continue;
        def = nexus_itemdef_get(item_id);
        if (def && def->defense > 0) {
            total_def += def->defense;
        }
    }

    /* Minimum defense of 1 so attacks always deal at least 1 damage.
     * Source: DM1 CREATURE.C minimum damage rule. */
    return total_def > 0 ? total_def : 1;
}

/* Return the champion pool index of the current party leader, or -1. */
static int mechanics_party_leader_index(const Nexus_V1_Engine *engine) {
    int idx;
    if (!engine || engine->champions.party_count <= 0) return -1;
    if (engine->champions.leader_index < 0 ||
        engine->champions.leader_index >= engine->champions.party_count) {
        return -1;
    }
    idx = engine->champions.party[engine->champions.leader_index];
    if (idx < 0 || idx >= engine->champions.champion_count) return -1;
    return idx;
}

/* Attack an adjacent creature from the party leader's facing/front square.
 * Implements DM1 melee attack resolution (weapon power + champion attack roll
 * vs creature defense).  On kill, award fighter XP and roll creature drops.
 * Returns 1 if a creature was attacked (hit or miss), 0 if no adjacent target.
 * Source: DM1 CREATURE.C / CHAMPION.C melee attack wiring,
 *         KILLMON.C drop roll, GOLDDROP.C gold generation. */
static int mechanics_attack_adjacent_creature(Nexus_V1_Engine *engine,
                                              Nexus_MechanicsState *st) {
    Nexus_V1_CreatureManager *mgr;
    Nexus_V1_Champion *leader;
    int leader_idx, target_idx = -1;
    int fx, fy, i;
    int weapon_power = 2; /* unarmed base */
    int weapon_item_id = -1;
    Nexus_CombatResult result;

    if (!engine || !st) return 0;
    /* ITEM.IBS declaration bytes do not prove the Saturn combat-action ABI.
     * In particular, the old unarmed power=2 path was inherited from DM1.
     * Keep live Nexus combat fail-closed until a Saturn attack dispatcher
     * capture/disassembly binds the player attack value and target routing. */
    if (!nexus_v1_action_semantics_proven()) return 0;
    mgr = &engine->creatures;
    if (mgr->active_count <= 0) return 0;

    leader_idx = mechanics_party_leader_index(engine);
    if (leader_idx < 0 || leader_idx >= engine->champions.champion_count)
        return 0;
    leader = &engine->champions.champions[leader_idx];
    if (!leader->alive) return 0;

    /* Front square from current facing */
    nexus_dir_deltas(st->party_dir, &fx, &fy);
    fx += st->party_x;
    fy += st->party_y;

    /* Prefer a creature directly in front; fallback to any adjacent square.
     * Fail-closed: hidden actors (Structure1B invisible-by-default bit) and
     * type-unbound actors (no proven stats) cannot be targeted. */
    for (i = 0; i < mgr->active_count; i++) {
        Nexus_Creature *c = &mgr->active[i];
        if (!c->alive || c->level != st->map_index) continue;
        if (c->type_index < 0 || c->hidden) continue;
        if (c->x == fx && c->y == fy) {
            target_idx = i;
            break;
        }
    }
    if (target_idx < 0) {
        static const int dx[4] = {0, 1, 0, -1};
        static const int dy[4] = {-1, 0, 1, 0};
        for (i = 0; i < mgr->active_count && target_idx < 0; i++) {
            Nexus_Creature *c = &mgr->active[i];
            int d;
            if (!c->alive || c->level != st->map_index) continue;
            if (c->type_index < 0 || c->hidden) continue;
            for (d = 0; d < 4; d++) {
                if (c->x == st->party_x + dx[d] && c->y == st->party_y + dy[d]) {
                    target_idx = i;
                    break;
                }
            }
        }
    }
    if (target_idx < 0) return 0;

    /* Equipped weapon contributes its attack value. */
    weapon_item_id = leader->slots[NEXUS_SLOT_WEAPON - 1];
    if (weapon_item_id >= 0) {
        const Nexus_ItemDef *wdef = nexus_itemdef_get(weapon_item_id);
        if (wdef && wdef->attack > 0) weapon_power = wdef->attack;
    }

    result = nexus_v1_attack(leader, weapon_power,
                             mgr->types[mgr->active[target_idx].type_index].defense);
    if (result.hit) {
        Nexus_Creature *c = &mgr->active[target_idx];
        c->health -= result.damage;
        nexus_sound_play(&engine->audio, NEXUS_SFX_ATTACK_HIT);
        if (c->health <= 0) {
            c->alive = 0;
            nexus_v1_gain_experience(leader, NEXUS_CLASS_FIGHTER,
                                     mgr->types[c->type_index].experience_value);
            nexus_sound_play(&engine->audio, NEXUS_SFX_CREATURE_DEATH);
            /* Nexus Saturn creature-drop ownership is still unproven.
             * Do not call the retired DM1-compatible drop table here. */
        }
    } else {
        nexus_sound_play(&engine->audio, NEXUS_SFX_ATTACK_MISS);
    }
    return 1;
}

int nexus_mechanics_tick(Nexus_MechanicsState *st, Nexus_V1_Engine *engine) {
    int needs_redraw = 0;
    int cmd = 0;
    int tx, ty, tl, td;
    int i;

    if (!st || !engine) return 0;
    st->total_ticks++;

    /* Advance door animation each tick, independent of input cooldown.
     * Source: DM1 viewport door open/close stepping (SDDRVS.TSK / renderer). */
    if (nexus_doors_tick_animation()) {
        needs_redraw = 1;
    }

    /* Process pending teleport before the movement cooldown gate.
     * Teleporter warps are immediate level/position transitions; they must
     * not be blocked by the normal step cooldown that was set when the
     * party walked onto the teleporter square.
     * Source: DM1 MOVESENS.C F0267/F0268 teleporter sensor (immediate). */
    if (st->pending_teleport) {
        st->party_x = st->teleport_target_x;
        st->party_y = st->teleport_target_y;
        if (st->teleport_target_level >= 0 &&
            st->teleport_target_level != st->map_index) {
            st->pending_level_change = st->teleport_target_level;
        }
        st->pending_teleport = 0;
        needs_redraw = 1;
        if (engine->audio_enabled)
            nexus_sound_play(&engine->audio, NEXUS_SFX_TELEPORT);
        if (st->move_cooldown_ticks > 0) {
            st->move_cooldown_ticks--;
            return needs_redraw;
        }
    }

    if (st->move_cooldown_ticks > 0) {
        st->move_cooldown_ticks--;
        return 0;
    }

    /* Dequeue command */
    if (!nexus_mechanics_pop_command(st, &cmd)) {
        cmd = 0;
    }

    /* Dispatch command */
    if (cmd != 0) {
        if (cmd == NEXUS_CMD_TURN_LEFT || cmd == NEXUS_CMD_TURN_RIGHT) {
            int turn_right = (cmd == NEXUS_CMD_TURN_RIGHT) ? 1 : 0;
            nexus_turn(&st->party_dir, turn_right);
            needs_redraw = 1;
        } else if (cmd == NEXUS_CMD_USE_ITEM) {
            /* Use the selected inventory item on the party leader.
             * Source: DM1 COMMAND.C item-use dispatch; CLIKMENU.C action menu. */
            int leader_idx = -1;
            if (engine->champions.party_count > 0 &&
                engine->champions.leader_index >= 0 &&
                engine->champions.leader_index < engine->champions.party_count) {
                leader_idx = engine->champions.party[engine->champions.leader_index];
            }
            if (leader_idx >= 0 && leader_idx < engine->champions.champion_count &&
                st->use_item_slot >= 0 && st->use_item_slot < 30) {
                Nexus_V1_Champion *leader = &engine->champions.champions[leader_idx];
                int item_id = leader->inventory[st->use_item_slot];
                /* ITEM.IBS proves declaration/icon bytes and the DGN item
                 * reference, not the Saturn action/use ABI.  The previous
                 * condition was inverted and ran the DM1 compatibility
                 * catalog exactly when the real IBS source was absent,
                 * manufacturing consumable/equipment effects in Nexus.
                 * Keep this runtime route no-op until a Saturn action
                 * dispatcher and item-semantics receipt are bound. */
                (void)leader;
                (void)item_id;
            }
        } else if (cmd == NEXUS_CMD_INTERACT) {
            /* Click-route interaction: pick up a floor item at the party's
             * current square.  If there is no floor item, attack an adjacent
             * creature (melee combat).
             * Source: DM1 COMMAND.C object/floor-item click dispatch,
             *         CREATURE.C / CHAMPION.C melee attack wiring. */
            int leader_idx = -1;
            int attacked = 0;
            if (engine->champions.party_count > 0 &&
                engine->champions.leader_index >= 0 &&
                engine->champions.leader_index < engine->champions.party_count) {
                leader_idx = engine->champions.party[engine->champions.leader_index];
            }
            if (leader_idx >= 0 && leader_idx < engine->champions.champion_count) {
                Nexus_V1_Champion *leader = &engine->champions.champions[leader_idx];
                int item_id = -1, qty = 0;
                int idx = nexus_floor_get_at(st->party_x, st->party_y, 0,
                                              &item_id, &qty);
                int fnt_idx = nexus_v1_fountain_find_at(&engine->fountains,
                                                        st->party_x, st->party_y);
                int altar_state = nexus_altar_at(st->party_x, st->party_y);
                int sw_idx = nexus_v1_switch_find_at(&engine->switches,
                                                     st->party_x, st->party_y);
                int ct_idx = nexus_v1_container_find_at(&engine->containers,
                                                        st->party_x, st->party_y);
                if (sw_idx >= 0) {
                    Nexus_SwitchResult sr = nexus_v1_switch_activate(
                        &engine->switches, sw_idx);
                    if (sr.activated && sr.target_type == NEXUS_SWITCH_TARGET_DOOR)
                    {
                        int door_is_open;
                        nexus_v1_door_toggle(&engine->doors, sr.target_id);
                        door_is_open = nexus_v1_door_is_passable(&engine->doors, sr.target_id);
                        nexus_script_on_door_change(&engine->script_vm,
                            st->party_x, st->party_y, door_is_open);
                    }
                    needs_redraw = 1;
                } else if (ct_idx >= 0) {
                    nexus_v1_container_open(&engine->containers, ct_idx, -1);
                    needs_redraw = 1;
                } else if (fnt_idx >= 0) {
                    nexus_v1_fountain_drink(&engine->fountains, fnt_idx, leader);
                    /* Nexus HUD text ownership is not established by the
                     * retail fountain record. Do not substitute the DM1
                     * English action label in the production HUD. */
                    needs_redraw = 1;
                } else if (altar_state != 0) {
                    /* Altar square: attempt ritual.  Real effect is blocked
                     * until COMMAND.C altar semantics are source-locked.
                     * Source: DM1 COMMAND.C altar use dispatch. */
                    nexus_altar_perform_ritual(st->party_x, st->party_y, leader);
                    needs_redraw = 1;
                } else if (idx >= 0 && item_id >= 0) {
                    int slot;
                    /* Find first empty slot in the flat uint8_t inventory. */
                    for (slot = 0; slot < NEXUS_INVENTORY_SLOTS; slot++) {
                        if (leader->inventory[slot] == 0xFFU) {
                            leader->inventory[slot] = (uint8_t)item_id;
                            nexus_floor_pickup(idx, &item_id, &qty);
                            nexus_champion_recalc_load(leader);
                            nexus_sound_play(&engine->audio, NEXUS_SFX_PICKUP_ITEM);
                            needs_redraw = 1;
                            break;
                        }
                    }
                } else {
                    attacked = mechanics_attack_adjacent_creature(engine, st);
                    if (attacked) needs_redraw = 1;
                }
            }
        } else if (cmd == NEXUS_CMD_CAST_SPELL) {
            /* Cast a spell using the buffered rune selection.
             * Resolves rune combination, deducts mana, and dispatches
             * by spell category: party buffs heal/shield the caster,
             * attack spells damage adjacent creatures.
             * Source: DM1 COMMAND.C F0412 spell cast dispatch;
             *         DM.BIN 0x0383AC spell handler vtable. */
            int leader_idx = mechanics_party_leader_index(engine);
            if (nexus_v1_action_semantics_proven() &&
                leader_idx >= 0 && st->spell_power >= 0 && st->spell_element >= 0 &&
                st->spell_form >= 0) {
                Nexus_V1_Champion *leader = &engine->champions.champions[leader_idx];
                Nexus_SpellClass cls = (leader->wizard_level >= leader->priest_level)
                    ? NEXUS_SPELL_CLASS_WIZARD : NEXUS_SPELL_CLASS_PRIEST;
                Nexus_SpellLookup sp = nexus_v1_spell_lookup(st->spell_power,
                    st->spell_element, st->spell_form, cls);
                if (!sp.valid) {
                    cls = (cls == NEXUS_SPELL_CLASS_PRIEST)
                        ? NEXUS_SPELL_CLASS_WIZARD : NEXUS_SPELL_CLASS_PRIEST;
                    sp = nexus_v1_spell_lookup(st->spell_power,
                        st->spell_element, st->spell_form, cls);
                }
                if (sp.valid) {
                    int effect = nexus_v1_cast_spell(leader, st->spell_power,
                                                     st->spell_element, st->spell_form,
                                                     st->spell_align);
                    if (effect > 0) {
                        Nexus_SpellCategory cat = nexus_v1_spell_category(sp.spell_type);
                        if (cat == NEXUS_SPELL_CAT_PARTY) {
                            if (sp.spell_type == NEXUS_SPELL_EFFECT_HEAL) {
                                leader->health += effect;
                                if (leader->health > leader->max_health)
                                    leader->health = leader->max_health;
                            }
                        } else {
                            enum Nexus_ProjectileType pt = NEXUS_PROJ_FIREBALL;
                            if (sp.spell_type == NEXUS_SPELL_EFFECT_LIGHTNING)
                                pt = NEXUS_PROJ_LIGHTNING;
                            else if (sp.spell_type == NEXUS_SPELL_EFFECT_POISON)
                                pt = NEXUS_PROJ_POISON_CLOUD;
                            nexus_v1_projectile_spawn(&engine->projectiles, pt,
                                st->party_x, st->party_y, st->party_dir,
                                effect, 2, leader_idx);
                        }
                        nexus_sound_play(&engine->audio, NEXUS_SFX_SPELL_CAST);
                        nexus_v1_experience_award_spell(&engine->experience,
                                                        leader, leader_idx,
                                                        st->spell_power);
                        needs_redraw = 1;
                    }
                }
                nexus_mechanics_clear_spell(st);
            }
        } else if (cmd == NEXUS_CMD_REST) {
            if (nexus_v1_rest_is_resting(&engine->rest))
                nexus_v1_rest_stop(&engine->rest);
            else
                nexus_v1_rest_start(&engine->rest);
        } else if (cmd == NEXUS_CMD_SET_LEADER) {
            int slot = st->set_leader_slot;
            if (slot >= 0 && slot < engine->champions.party_count) {
                int idx = engine->champions.party[slot];
                if (idx >= 0 && idx < 4 &&
                    engine->champions.champions[idx].alive) {
                    engine->champions.leader_index = slot;
                    needs_redraw = 1;
                }
            }
        } else if (cmd == NEXUS_CMD_DROP_ITEM) {
            int li4 = mechanics_party_leader_index(engine);
            if (li4 >= 0 && st->drop_slot >= 0 && st->drop_slot < NEXUS_INVENTORY_SLOTS) {
                Nexus_V1_Champion *ldr = &engine->champions.champions[li4];
                uint8_t item_id = ldr->inventory[st->drop_slot];
                if (item_id != 0xFFU) {
                    ldr->inventory[st->drop_slot] = 0xFFU;
                    nexus_floor_drop(st->party_x, st->party_y, item_id, 1);
                    nexus_champion_recalc_load(ldr);
                    needs_redraw = 1;
                }
            }
        } else if (cmd == NEXUS_CMD_THROW) {
            int li3 = mechanics_party_leader_index(engine);
            if (li3 >= 0) {
                Nexus_V1_Champion *ldr = &engine->champions.champions[li3];
                nexus_v1_throw_item(&engine->thrown, &engine->projectiles,
                                    ldr, li3, st->throw_slot,
                                    NULL, st->party_x, st->party_y, st->party_dir);
                needs_redraw = 1;
            }
        } else {
            /* Step movement */
            int forward = (cmd == NEXUS_CMD_FORWARD) ? 1 : 0;
            int strafe = (cmd == NEXUS_CMD_STRAFE_LEFT || cmd == NEXUS_CMD_STRAFE_RIGHT) ? 1 : 0;
            int strafe_left = (cmd == NEXUS_CMD_STRAFE_LEFT) ? 1 : 0;
            int t_x, t_y;

            /* BACKWARD: pass forward=0 so target_square moves in opposite dir.
             * Source: nexus_v1_movement.c nexus_target_square(), CLIKMENU.C:224. */
            if (cmd == NEXUS_CMD_BACKWARD) forward = 0;

            nexus_target_square(st->party_x, st->party_y, st->party_dir,
                                 forward, strafe, strafe_left,
                                 &t_x, &t_y);

            int sq = nexus_v1_level_get_square(&engine->current_level, t_x, t_y);

            /* Door blocking: check if door is passable (open or animating
             * past the threshold).  Locked doors require a key; the first
             * successful key check starts an opening animation.
             * Source: DM1 door processing — locked doors block without key;
             *         viewport door animation steps open/closed. */
            if (sq == NEXUS_SQUARE_DOOR && !nexus_doors_is_passable(t_x, t_y)) {
                uint8_t leader_inv[30] = {[0 ... 29] = (uint8_t)-1};
                int leader_idx = mechanics_party_leader_index(engine);
                if (leader_idx >= 0) {
                    memcpy(leader_inv, engine->champions.champions[leader_idx].inventory, 30);
                }
                if (!nexus_doors_can_open(t_x, t_y, leader_inv)) {
                    /* Locked and no key — blocked */
                    sq = 0; /* treat as wall */
                } else {
                    nexus_doors_open(t_x, t_y);
                    nexus_sound_play(&engine->audio, NEXUS_SFX_DOOR_OPEN);
                    needs_redraw = 1;
                }
            }

            /* Water/fire square traversal — DM.BIN disassembly evidence:
             *
             * Water (type 21): no crossing gate found in movement path.
             * All four CMP/EQ #21,R0 sites (0x01CE40, 0x020E3A, 0x02CBAA,
             * 0x02CD52) are rendering/dispatch, not passability checks.
             * Water squares are always passable; damage is applied on entry.
             *
             * Fire (type 22): DM.BIN 0x0603C386 gates on bit 0 of a 16-bit
             * attribute word at offset 310 from state pointer. If fire
             * protection is active (fire_shield_ticks > 0), the party can
             * cross. Otherwise, the square blocks movement. */
            if (sq == NEXUS_SQUARE_FIRE && st->fire_shield_ticks <= 0) {
                sq = 0; /* block: no fire protection */
            }

            /* A teleporter without a decoded source link is not an ordinary
             * floor square. Reject it before mutating party position; the
             * square-event dispatcher may consume only registered links. */
            if ((sq == NEXUS_SQUARE_TELEPORT ||
                 sq == NEXUS_SQUARE_TELEPORT2 ||
                 sq == NEXUS_SQUARE_TELEPORT3) &&
                nexus_teleporters_resolve(t_x, t_y, NULL, NULL, NULL) != 0) {
                sq = 0;
            }

            /* A staircase without an authenticated Structure1F destination
             * is not a license to invent an adjacent-level same-coordinate
             * link. Keep the party on the source square until the real
             * sensor/script owner is decoded. */
            if ((sq == NEXUS_SQUARE_STAIRS_DN ||
                 sq == NEXUS_SQUARE_STAIRS_UP) &&
                nexus_stairs_resolve(t_x, t_y, NULL, NULL, NULL, NULL) != 0) {
                sq = 0;
            }

            if (sq != 0 && nexus_v1_level_move_allowed(&engine->current_level,
                                                       st->party_x, st->party_y,
                                                       t_x, t_y)) {
                st->party_x = t_x;
                st->party_y = t_y;
                {
                    int li2 = mechanics_party_leader_index(engine);
                    if (li2 >= 0) {
                        Nexus_V1_Champion *ldr = &engine->champions.champions[li2];
                        st->move_cooldown_ticks = nexus_v1_encumbrance_move_ticks(ldr);
                        int stam_cost = nexus_v1_encumbrance_stamina_cost(ldr);
                        if (ldr->stamina > stam_cost)
                            ldr->stamina -= stam_cost;
                        else
                            ldr->stamina = 0;
                    } else {
                        st->move_cooldown_ticks = 8;
                    }
                }
                needs_redraw = 1;

                /* Check traps at new position */
                {
                    int li = mechanics_party_leader_index(engine);
                    if (li >= 0) {
                        Nexus_V1_TrapResult tr = nexus_v1_trap_trigger(
                            &engine->traps,
                            &engine->champions.champions[li],
                            st->map_index, t_x, t_y);
                        if (tr.triggered && tr.damage_dealt > 0) {
                            nexus_v1_damage_display_add(&engine->damage_display,
                                li, tr.damage_dealt, NEXUS_DMG_TAKEN);
                        }
                    }
                }

                /* Check transition table for level changes */
                {
                    const Nexus_V1_Transition *trans =
                        nexus_v1_transition_find(&engine->transitions,
                                                  st->map_index, t_x, t_y);
                    if (trans) {
                        int dl, dx, dy, dd;
                        if (nexus_v1_transition_apply(trans, &dl, &dx, &dy, &dd)) {
                            st->pending_level_change = dl;
                            st->party_x = dx;
                            st->party_y = dy;
                            if (dd >= 0) st->party_dir = dd;
                        }
                    }
                }

                /* Process square event */
                Nexus_SquareEvent sq_event = nexus_process_square_event(sq, t_x, t_y, &tx, &ty, &tl, &td);
                switch (sq_event) {
                case NEXUS_EVENT_STAIRS_DOWN:
                case NEXUS_EVENT_STAIRS_UP:
                    /* Stairs: only registered links supply a target. The
                     * movement gate rejects unregistered stairs, and the
                     * square-event API also returns BLOCKED for them; neither
                     * route may invent an adjacent-level transition.
                     * Source: DM1 CLIKMENU.C F0364_COMMAND_TakeStairs,
                     *         MOVESENS.C F0267/F0268 stairs sensor. */
                    if (tl >= 0) {
                        st->pending_level_change = tl;
                        st->party_x = tx;
                        st->party_y = ty;
                        if (td >= 0) st->party_dir = td;
                    }
                    nexus_sound_play(&engine->audio, NEXUS_SFX_STAIRS);
                    break;
                case NEXUS_EVENT_TELEPORT:
                    nexus_mechanics_teleport(st, tx, ty, tl);
                    break;
                case NEXUS_EVENT_PIT_FALL:
                case NEXUS_EVENT_CHUTE_FALL:
                    /* Pit/chute forces the party to the authenticated target.
                     * An absent target is rejected by the square-event route;
                     * no adjacent-level fallback is permitted here.
                     * Source: DM1 MOVESENS.C F0267/F0268 chute/pit sensor;
                     *         CLIKMENU.C:264-276 stairs special cases. */
                    if (tl < 0) break;
                    st->pending_level_change = tl;
                    if (st->pending_level_change > NEXUS_MAX_LEVEL)
                        st->pending_level_change = NEXUS_MAX_LEVEL;
                    st->party_x = tx;
                    st->party_y = ty;
                    nexus_sound_play(&engine->audio, NEXUS_SFX_PIT_FALL);
                    break;
                case NEXUS_EVENT_ALARM_TRIGGER:
                    /* Alarm: alert all creatures on the current level and start
                     * a bounded alarm timer that keeps them chasing.
                     * Source: DM1 MOVESENS.C F0277 — ALARM sets creature alert=255. */
                    nexus_v1_creatures_alert_all(&engine->creatures, st->map_index);
                    nexus_sound_play(&engine->audio, NEXUS_SFX_ALARM);
                    break;
                case NEXUS_EVENT_DOOR_OPEN:
                    nexus_sound_play(&engine->audio, NEXUS_SFX_DOOR_OPEN);
                    break;
                case NEXUS_EVENT_EXIT_REACHED:
                    /* Exit: only the final level exit completes the game.
                     * Earlier exits are treated as ordinary floor squares;
                     * the original may use them as decorative markers or
                     * return-to-previous-level portals, but that behavior is
                     * not source-locked here.
                     * Source: DM1 MOVESENS.C exit square sensor. */
                    if (st->map_index >= NEXUS_MAX_LEVEL) {
                        st->game_over = 1;
                        st->game_over_reason = 1;
                        nexus_sound_play(&engine->audio, NEXUS_SFX_EXIT_REACHED);
                    }
                    break;
                case NEXUS_EVENT_CROSS_WATER:
                    /* Water crossed with rope. No additional effect in V1.
                     * Source: DM1 MOVESENS.C water square sensor. */
                    nexus_sound_play(&engine->audio, NEXUS_SFX_FOOTSTEP);
                    break;
                case NEXUS_EVENT_CROSS_FIRE:
                    /* Fire crossed with fire-rune protection. No additional
                     * effect in V1.
                     * Source: DM1 MOVESENS.C fire square sensor. */
                    nexus_sound_play(&engine->audio, NEXUS_SFX_FOOTSTEP);
                    break;
                default:
                    break;
                }
            }
        }
    }

    /* The host automap storage is retained for isolated state tests, but the
     * retail Saturn explored-state write path is not yet authenticated. Do
     * not turn a host radius reveal into a claimed Nexus HUD behavior. */
    if (engine->smap_runtime_receipt.explored_state_write_proven) {
        nexus_v1_automap_reveal_radius(&engine->automap, st->map_index,
                                        st->party_x, st->party_y, 1);
        nexus_v1_automap_set_level(&engine->automap, st->map_index);
    }

    /* Creature AI tick */
    if (engine->creatures.type_count > 0 && engine->level_loaded) {
        nexus_v1_creatures_tick(&engine->creatures,
                                 st->party_x, st->party_y,
                                 engine->current_level.squares,
                                 st->map_index);

        /* Creature attack against adjacent party.
         * Source: DM1 CREATURE.C F0209 creature attack vs champion;
         *         CHAMPION.C F0309 defense/absorb wiring.
         * Damage is applied to the party leader when possible; if the leader
         * is dead, the first living party member absorbs the blow. */
        for (i = 0; i < engine->creatures.active_count; i++) {
            Nexus_Creature *c = &engine->creatures.active[i];
            if (!c->alive) continue;
            /* Only creatures on the current level can attack the party. */
            if (c->level != st->map_index) continue;
            if (c->state == 3) { /* attack range */
                int champ_def = get_champion_defense(&engine->champions);
                int dmg = 0;
                if (nexus_v1_action_semantics_proven() &&
                    nexus_v1_creature_attack(&engine->creatures, i, champ_def, &dmg)) {
                    if (dmg > 0) {
                        int target_idx = -1;
                        int pi;
                        /* Prefer party leader. */
                        if (engine->champions.leader_index >= 0 &&
                            engine->champions.leader_index < engine->champions.party_count) {
                            int li = engine->champions.party[engine->champions.leader_index];
                            if (li >= 0 && li < engine->champions.champion_count &&
                                engine->champions.champions[li].alive) {
                                target_idx = li;
                            }
                        }
                        /* Fallback to first living party member. */
                        for (pi = 0; target_idx < 0 && pi < engine->champions.party_count; pi++) {
                            int ci = engine->champions.party[pi];
                            if (ci >= 0 && ci < engine->champions.champion_count &&
                                engine->champions.champions[ci].alive) {
                                target_idx = ci;
                            }
                        }
                        if (target_idx >= 0) {
                            int was_alive = engine->champions.champions[target_idx].alive;
                            int died = nexus_v1_take_damage(&engine->champions.champions[target_idx], dmg);
                            if (died && was_alive) {
                                nexus_v1_champion_on_death_update_leader(
                                    &engine->champions, target_idx);
                            }
                            nexus_sound_play(&engine->audio, NEXUS_SFX_CHAMPION_HURT);
                            nexus_v1_rest_interrupt(&engine->rest);
                            {
                                int psn = engine->creatures.types[c->type_index].poison;
                                if (psn > 0) {
                                    int pi;
                                    for (pi = 0; pi < engine->champions.party_count && pi < 4; pi++) {
                                        if (engine->champions.party[pi] == target_idx) {
                                            nexus_v1_status_apply(&engine->champion_status[pi],
                                                NEXUS_STATUS_POISON, psn * 10, psn);
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                        nexus_sound_play(&engine->audio, NEXUS_SFX_CREATURE_ATTACK);
                    }
                }
            }
        }

        /* Creature ranged attacks — creatures with ranged_type != 0 fire
         * projectiles when chasing (state==2) and within detection range.
         * CRET byte 4 ranged_type: 4=fireball, 6=poison, 10=lightning.
         * Source: DM1 CREATURE.C ranged creature attacks; CRET stat table. */
        for (i = 0; i < engine->creatures.active_count; i++) {
            Nexus_Creature *c = &engine->creatures.active[i];
            int rtype, dir_to_party;
            enum Nexus_ProjectileType pt;
            if (!c->alive || c->level != st->map_index) continue;
            if (c->type_index < 0 || c->hidden) continue;
            rtype = engine->creatures.types[c->type_index].ranged_type;
            if (rtype == 0 || !nexus_v1_action_semantics_proven()) continue;
            if (c->state != 2) continue;
            {
                int dist = nexus_v1_creature_distance(c->x, c->y,
                    st->party_x, st->party_y);
                if (dist <= 1 || dist > 6) continue;
            }
            if (c->ai_timer % 8 != 0) continue;
            if (abs(st->party_x - c->x) > abs(st->party_y - c->y))
                dir_to_party = (st->party_x > c->x) ? 1 : 3;
            else
                dir_to_party = (st->party_y > c->y) ? 2 : 0;
            switch (rtype) {
                case 6:  pt = NEXUS_PROJ_POISON; break;
                case 10: pt = NEXUS_PROJ_LIGHTNING; break;
                default: pt = NEXUS_PROJ_FIREBALL; break;
            }
            nexus_v1_projectile_spawn(&engine->projectiles, pt,
                c->x, c->y, dir_to_party,
                engine->creatures.types[c->type_index].attack / 2,
                2, -1);
            nexus_sound_play(&engine->audio, NEXUS_SFX_CREATURE_ATTACK);
        }

        /* Projectile tick — move active projectiles and apply damage.
         * Source: DM1 OBJECTMAN.C F0258 projectile movement,
         *         PROJECTIL.C F0218 collision resolution. */
        {
            Nexus_ProjectileHit proj_hits[NEXUS_MAX_PROJECTILES];
            int n_hits = nexus_v1_projectiles_tick(&engine->projectiles,
                engine->current_level.squares, proj_hits, NEXUS_MAX_PROJECTILES);
            for (i = 0; i < n_hits; i++) {
                if (!nexus_v1_action_semantics_proven()) continue;
                if (proj_hits[i].hit_wall) continue;
                if (proj_hits[i].source_champion >= 0) {
                    int killed = nexus_v1_creature_manager_damage_at(
                        &engine->creatures,
                        proj_hits[i].hit_x, proj_hits[i].hit_y,
                        proj_hits[i].damage);
                    if (killed > 0) {
                        needs_redraw = 1;
                        if (proj_hits[i].source_champion >= 0 &&
                            proj_hits[i].source_champion < 4) {
                            nexus_v1_xp_add(
                                &engine->champion_xp[proj_hits[i].source_champion],
                                NEXUS_XP_CLASS_WIZARD, killed * 50);
                            nexus_v1_xp_check_levelup(
                                &engine->champion_xp[proj_hits[i].source_champion],
                                NEXUS_XP_CLASS_WIZARD);
                        }
                    }
                } else if (proj_hits[i].hit_x == st->party_x &&
                           proj_hits[i].hit_y == st->party_y) {
                    int target_idx = -1, pi;
                    if (engine->champions.leader_index >= 0 &&
                        engine->champions.leader_index < engine->champions.party_count) {
                        int li = engine->champions.party[engine->champions.leader_index];
                        if (li >= 0 && li < engine->champions.champion_count &&
                            engine->champions.champions[li].alive)
                            target_idx = li;
                    }
                    for (pi = 0; target_idx < 0 && pi < engine->champions.party_count; pi++) {
                        int ci = engine->champions.party[pi];
                        if (ci >= 0 && ci < engine->champions.champion_count &&
                            engine->champions.champions[ci].alive)
                            target_idx = ci;
                    }
                    if (target_idx >= 0) {
                        int was_alive = engine->champions.champions[target_idx].alive;
                        int died = nexus_v1_take_damage(
                            &engine->champions.champions[target_idx],
                            proj_hits[i].damage);
                        if (died && was_alive)
                            nexus_v1_champion_on_death_update_leader(
                                &engine->champions, target_idx);
                        nexus_sound_play(&engine->audio, NEXUS_SFX_CHAMPION_HURT);
                    }
                }
            }
        }

        /* Check for total party death.
         * Source: DM1 CLIKMENU.C F0366 — game ends when all champions die. */
        if (!nexus_mechanics_party_alive(st, engine)) {
            st->game_over = 1;
            st->game_over_reason = 2; /* all_dead */
        }
    }

    /* Light tick — decay ambient light, burn torches/FUL spells */
    nexus_v1_light_tick(&engine->light);

    /* Status effect tick — poison damage, buff expiry */
    for (i = 0; i < engine->champions.party_count && i < 4; i++) {
        int ci = engine->champions.party[i];
        if (ci < 0 || ci >= engine->champions.champion_count) continue;
        if (!engine->champions.champions[ci].alive) continue;
        nexus_v1_status_tick(&engine->champion_status[i]);
        {
            int pdmg = nexus_v1_status_poison_damage(&engine->champion_status[i]);
            if (pdmg > 0) {
                int was_alive = engine->champions.champions[ci].alive;
                int died = nexus_v1_take_damage(&engine->champions.champions[ci], pdmg);
                if (died && was_alive)
                    nexus_v1_champion_on_death_update_leader(&engine->champions, ci);
            }
        }
    }

    /* Rest tick — regenerate stamina/mana/health while resting */
    nexus_v1_rest_tick(&engine->rest, &engine->champions);

    /* Resource drain — every FOOD_DRAIN_TICKS ticks, drain 1 food from
     * each living party champion. When food reaches 0, stamina drains
     * at double rate. When food AND water both reach 0, champions take
     * wound damage per tick.
     * Source: DM1 CHAMPION.C food/water drain (F0325 stamina decrement). */
    if (st->food_drain_timer > 0) st->food_drain_timer--;
    if (st->water_drain_timer > 0) st->water_drain_timer--;
    if (st->fire_shield_ticks > 0) st->fire_shield_ticks--;
    if (st->food_drain_timer <= 0) {
        st->food_drain_timer = FOOD_DRAIN_TICKS;
        /* Drain 1 food from each living party champion.
         * Source: DM1 food drain on party movement. */
        for (i = 0; i < engine->champions.party_count; i++) {
            int ci = engine->champions.party[i];
            if (ci < 0 || ci >= engine->champions.champion_count) continue;
            Nexus_V1_Champion *c = &engine->champions.champions[ci];
            if (!c->alive) continue;
            if (c->food > 0) c->food--;
            /* When food=0, stamina penalty: -2 extra stamina per step.
             * Source: DM1 CHAMPION.C — starving reduces stamina regen. */
            if (c->food == 0) c->stamina -= 2;
        }
    }
    if (st->water_drain_timer <= 0) {
        st->water_drain_timer = WATER_DRAIN_TICKS;
        for (i = 0; i < engine->champions.party_count; i++) {
            int ci = engine->champions.party[i];
            if (ci < 0 || ci >= engine->champions.champion_count) continue;
            Nexus_V1_Champion *c = &engine->champions.champions[ci];
            if (!c->alive) continue;
            if (c->water > 0) c->water--;
            if (c->water == 0) c->stamina -= 2;
        }
    }

    /* Stamina cost per step — party leader spends 3 stamina per step.
     * Source: DM1 CLIKMENU.C:237-255 (living champion stamina decrement),
     * CHAMPION.C F0325_DECREMENTSTAMINA lines 2025-2048. */
    if (needs_redraw && cmd != NEXUS_CMD_TURN_LEFT && cmd != NEXUS_CMD_TURN_RIGHT) {
        int leader_idx;
        if (engine->champions.party_count > 0 &&
            engine->champions.leader_index >= 0 &&
            engine->champions.leader_index < engine->champions.party_count) {
            leader_idx = engine->champions.party[engine->champions.leader_index];
            if (leader_idx >= 0 && leader_idx < engine->champions.champion_count) {
                int was_alive = engine->champions.champions[leader_idx].alive;
                nexus_champion_decrement_stamina(
                    &engine->champions.champions[leader_idx], 3);
                if (was_alive && !engine->champions.champions[leader_idx].alive) {
                    nexus_v1_champion_on_death_update_leader(
                        &engine->champions, leader_idx);
                }
            }
        }
    }

    /* Script VM: party move event.
     * Fires ON_XY rules when party steps on a scripted square.
     * Source: nexus_v1_script_vm.c nexus_script_on_party_move(). */
    if (needs_redraw) {
        nexus_script_on_party_move(&engine->script_vm,
                                   st->party_x,
                                   st->party_y,
                                   st->map_index);
    }

    /* Gold pickup — only if party is alive.
     * Source: DM1 GOLDDROP.C gold pile pickup on move result. */
    if (engine->champions.party_count > 0) {
        int gold = nexus_gold_at(st->party_x, st->party_y);
        if (gold > 0) {
            st->gold_pieces += gold;
            nexus_gold_remove(st->party_x, st->party_y);
            nexus_sound_play(&engine->audio, NEXUS_SFX_GOLD_PICKUP);
        }
    }

    return needs_redraw || (st->pending_level_change >= 0);
}

int nexus_mechanics_dispatch_event(Nexus_MechanicsState *st,
                                   Nexus_V1_Engine *engine,
                                   int event_type, int param)
{
    if (!st || !engine) return -1;

    switch (event_type) {
    case NEXUS_UI_EVENT_MAP:
        nexus_v1_automap_toggle(&engine->automap);
        return 0;
    case NEXUS_UI_EVENT_SPELL:
        nexus_mechanics_push_command(st, NEXUS_CMD_CAST_SPELL);
        return 0;
    case NEXUS_UI_EVENT_INVENTORY:
        nexus_mechanics_set_use_item_slot(st, param);
        nexus_mechanics_push_command(st, NEXUS_CMD_USE_ITEM);
        return 0;
    case NEXUS_UI_EVENT_REST:
        nexus_mechanics_push_command(st, NEXUS_CMD_REST);
        return 0;
    case NEXUS_UI_EVENT_SAVE:
        nexus_v1_save_full(&engine->save_manager, (uint8_t)param,
                           engine->game.current_level,
                           st->party_x, st->party_y, st->party_dir,
                           (uint32_t)engine->game.tick_count,
                           0, &engine->champions, NULL);
        /* Save success has no authenticated Nexus text owner yet. Avoid
         * presenting a fabricated English HUD string. */
        return 0;
    case NEXUS_UI_EVENT_SET_LEADER:
        nexus_mechanics_set_leader_slot(st, param);
        nexus_mechanics_push_command(st, NEXUS_CMD_SET_LEADER);
        return 0;
    case NEXUS_UI_EVENT_THROW:
        st->throw_slot = param;
        nexus_mechanics_push_command(st, NEXUS_CMD_THROW);
        return 0;
    case NEXUS_UI_EVENT_DROP:
        st->drop_slot = param;
        nexus_mechanics_push_command(st, NEXUS_CMD_DROP_ITEM);
        return 0;
    default:
        return -1;
    }
}
