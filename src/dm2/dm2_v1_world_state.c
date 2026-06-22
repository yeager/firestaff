/*
 * dm2_v1_world_state.c — DM2 V1 World-State Ingestion Implementation
 *
 * DM2 Phase 2: World-state loading, quest state, NPC positions.
 *
 * World-state in DM2 is encoded in SKSave.dat (SUPPRESS-compressed).
 * The SUPPRESS format is a bit-plane RLE compression used in DM2 saves.
 * For a new game, the initial world state comes from DUNGEON.DAT.
 *
 * Source: docs/dm2_save_format.md — SUPPRESS compression, save sections
 * Source: docs/dm2_quest.md — quest flags and phases
 * Source: docs/dm2_party_state.md — party/champion state
 * Source: docs/dm2_time.md — timer system, game tick
 * Source: SKULL.ASM T520 — party placement from dungeon
 */

#include "dm2_v1_world_state.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_world_model.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define DM2_WORLD_STATE_STUB_SIZE 64u
#define DM2_WORLD_STATE_EXPLORE_MAGIC "FS2E"
#define DM2_WORLD_STATE_EXPLORE_HEADER_SIZE 12u
#define DM2_WORLD_STATE_EXPLORE_PAYLOAD_SIZE \
    (DM2_WORLD_STATE_MAX_LEVELS * DM2_WORLD_STATE_EXPLORED_BYTES)

static void write_u32_le(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v & 0xFFu);
    p[1] = (uint8_t)((v >> 8) & 0xFFu);
    p[2] = (uint8_t)((v >> 16) & 0xFFu);
    p[3] = (uint8_t)((v >> 24) & 0xFFu);
}

static uint32_t read_u32_le(const uint8_t *p)
{
    return (uint32_t)p[0]
         | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}

/* ── World-state construction from dungeon ─────────────────────── */

/*
 * dm2_v1_world_state_new_from_dungeon — create initial world state
 * from DUNGEON.DAT initial party location.
 *
 * DM2 initial party location is encoded in DUNGEON_HEADER.initial_party_location:
 *   bits[5:0]   = map X
 *   bits[11:6]  = map Y
 *   bits[15:12] = direction (0=N,1=E,2=S,3=W)
 *
 * Source: ReDMCSB DEFS.H:985-998, SKULL.ASM T520
 */
DM2_WorldState *dm2_v1_world_state_new_from_dungeon(const uint8_t *dungeon_data,
                                                     size_t size) {
    DM2_WorldState *state;
    dm2_dungeon_world_t *world;
    uint16_t party_loc;
    int px, py, pdir;

    if (!dungeon_data || size < 44) return NULL;

    state = calloc(1, sizeof(DM2_WorldState));
    if (!state) return NULL;

    world = dm2_world_from_mem(dungeon_data, size);
    if (!world) {
        free(state);
        return NULL;
    }

    party_loc = (uint16_t)dungeon_data[16] | ((uint16_t)dungeon_data[17] << 8);

    dm2_unpack_party_start(party_loc, &px, &py, &pdir);

    state->current_level = 0;
    state->party.leader_index = 0;
    state->party.champion_count = 0;
    state->party.party_gold = 0;
    state->quest_count = 0;
    state->game_tick = 0;
    state->timer_count = 0;

    /* Initialize all champions to empty */
    for (int i = 0; i < DM2_MAX_CHAMPIONS; i++) {
        state->party.champions[i].hp = 0;
        state->party.champions[i].max_hp = 0;
        state->party.champions[i].mp = 0;
        state->party.champions[i].max_mp = 0;
        state->party.champions[i].x = px;
        state->party.champions[i].y = py;
        state->party.champions[i].condition = DM2_CHAMP_CONDITION_HEALTHY;
    }

    /* Initialize weather for each level */
    for (int i = 0; i < 30; i++) {
        state->weather_by_level[i].weather_type = DM2_WEATHER_CLEAR;
        state->weather_by_level[i].intensity = 0;
        state->weather_by_level[i].duration = 0;
    }

    /* Count outdoor levels */
    int outdoor_count = 0;
    for (int i = 0; i < world->map_count; i++) {
        if (dm2_world_is_outdoor(world, i)) outdoor_count++;
    }
    state->outdoor_level_count = outdoor_count;

    dm2_world_free(world);
    return state;
}

/* ── Save game serialization ─────────────────────────────────────── */

/*
 * dm2_v1_world_state_serialize — serialize world state to SUPPRESS buffer.
 *
 * The SUPPRESS format for DM2 saves:
 *   [header 42 bytes] + [dungeon header 44] + [map headers] +
 *   [tile→object index] + [ground stacks] + [text data] +
 *   [16 DB record pools] + [map data] + [game state block] +
 *   [global flags] + [global bytes] + [global words] +
 *   [champion squad] + [spell effects] + [timers] +
 *   [champion inventories] + [leader hand] + [extra dungeon] +
 *   [minion association]
 *
 * Source: docs/dm2_save_format.md
 */
uint8_t *dm2_v1_world_state_serialize(const DM2_WorldState *state, size_t *out_size) {
    /* Placeholder: full SUPPRESS encoding is a separate implementation.
     * For Phase 2, return a minimal valid buffer with a Firestaff-private
     * explored-map extension. The extension stays outside the original DM2
     * SUPPRESS claim until the full save writer lands. */
    const size_t total_size = DM2_WORLD_STATE_STUB_SIZE
                            + DM2_WORLD_STATE_EXPLORE_HEADER_SIZE
                            + DM2_WORLD_STATE_EXPLORE_PAYLOAD_SIZE;
    uint8_t *buf;

    if (!state) { if (out_size) *out_size = 0; return NULL; }

    buf = malloc(total_size);
    if (!buf) { if (out_size) *out_size = 0; return NULL; }
    memset(buf, 0, total_size);
    /* Write save slot magic markers (BEET/DEAD) */
    buf[38] = 0xBE; buf[39] = 0xEF;
    buf[40] = 0xDE; buf[41] = 0xAD;
    memcpy(buf + DM2_WORLD_STATE_STUB_SIZE, DM2_WORLD_STATE_EXPLORE_MAGIC, 4);
    buf[DM2_WORLD_STATE_STUB_SIZE + 4] = 1; /* extension version */
    buf[DM2_WORLD_STATE_STUB_SIZE + 5] = DM2_WORLD_STATE_MAX_LEVELS;
    buf[DM2_WORLD_STATE_STUB_SIZE + 6] = DM2_WORLD_STATE_MAP_EDGE;
    buf[DM2_WORLD_STATE_STUB_SIZE + 7] = DM2_WORLD_STATE_MAP_EDGE;
    write_u32_le(buf + DM2_WORLD_STATE_STUB_SIZE + 8,
                 (uint32_t)DM2_WORLD_STATE_EXPLORE_PAYLOAD_SIZE);
    memcpy(buf + DM2_WORLD_STATE_STUB_SIZE + DM2_WORLD_STATE_EXPLORE_HEADER_SIZE,
           state->explored_by_level,
           DM2_WORLD_STATE_EXPLORE_PAYLOAD_SIZE);
    if (out_size) *out_size = total_size;
    return buf;
}

/*
 * dm2_v1_world_state_load_from_mem — load world state from SUPPRESS buffer.
 * Source: docs/dm2_save_format.md
 */
DM2_WorldState *dm2_v1_world_state_load_from_mem(const uint8_t *data, size_t size) {
    DM2_WorldState *state;
    uint8_t slot_magic[4];

    if (!data || size < 42) return NULL;

    /* Check slot validity markers */
    slot_magic[0] = data[38]; slot_magic[1] = data[39];
    slot_magic[2] = data[40]; slot_magic[3] = data[41];
    if (!(slot_magic[0] == 0xBE && slot_magic[1] == 0xEF &&
          slot_magic[2] == 0xDE && slot_magic[3] == 0xAD)) {
        return NULL; /* Invalid save slot */
    }

    state = calloc(1, sizeof(DM2_WorldState));
    if (!state) return NULL;

    state->raw_save = malloc(size);
    if (!state->raw_save) { free(state); return NULL; }
    memcpy(state->raw_save, data, size);
    state->raw_save_size = size;

    /* Parse game state block (simplified — full parse deferred to Phase 3) */
    state->game_tick = 0;
    state->timer_count = 0;
    state->quest_count = 0;

    if (size >= DM2_WORLD_STATE_STUB_SIZE + DM2_WORLD_STATE_EXPLORE_HEADER_SIZE &&
        memcmp(data + DM2_WORLD_STATE_STUB_SIZE, DM2_WORLD_STATE_EXPLORE_MAGIC, 4) == 0) {
        const uint8_t *ext = data + DM2_WORLD_STATE_STUB_SIZE;
        uint32_t payload_size = read_u32_le(ext + 8);
        size_t payload_offset = DM2_WORLD_STATE_STUB_SIZE + DM2_WORLD_STATE_EXPLORE_HEADER_SIZE;
        if (ext[4] == 1 &&
            ext[5] == DM2_WORLD_STATE_MAX_LEVELS &&
            ext[6] == DM2_WORLD_STATE_MAP_EDGE &&
            ext[7] == DM2_WORLD_STATE_MAP_EDGE &&
            payload_size == DM2_WORLD_STATE_EXPLORE_PAYLOAD_SIZE &&
            size >= payload_offset + DM2_WORLD_STATE_EXPLORE_PAYLOAD_SIZE) {
            memcpy(state->explored_by_level,
                   data + payload_offset,
                   DM2_WORLD_STATE_EXPLORE_PAYLOAD_SIZE);
        }
    }

    return state;
}

DM2_WorldState *dm2_v1_world_state_load_from_file(const char *path) {
    FILE *f;
    uint8_t *buf;
    long fsize;
    DM2_WorldState *state;

    if (!path) return NULL;
    f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (fsize <= 0 || fsize > 2*1024*1024) { fclose(f); return NULL; }
    buf = malloc((size_t)fsize);
    if (!buf) { fclose(f); return NULL; }
    if (fread(buf, 1, (size_t)fsize, f) != (size_t)fsize) {
        free(buf); fclose(f); return NULL;
    }
    fclose(f);

    state = dm2_v1_world_state_load_from_mem(buf, (size_t)fsize);
    free(buf);
    return state;
}

/* ── Accessors ──────────────────────────────────────────────────── */

int dm2_v1_world_state_get_quest_flag(const DM2_WorldState *state, int flag_index) {
    if (!state || flag_index < 0 || flag_index >= DM2_QUEST_FLAG_COUNT) return 0;
    return (int)state->global_flags[flag_index];
}

void dm2_v1_world_state_set_quest_flag(DM2_WorldState *state,
                                        int flag_index, int value) {
    if (!state || flag_index < 0 || flag_index >= DM2_QUEST_FLAG_COUNT) return;
    state->global_flags[flag_index] = (uint8_t)(value & 0xFF);
}

int dm2_v1_world_state_get_champion_hp(const DM2_WorldState *state, int champ_index) {
    if (!state || champ_index < 0 || champ_index >= state->party.champion_count) return -1;
    return (int)state->party.champions[champ_index].hp;
}

int dm2_v1_world_state_get_explored(const DM2_WorldState *state, int level, int x, int y) {
    int bit_index;
    if (!state ||
        level < 0 || level >= DM2_WORLD_STATE_MAX_LEVELS ||
        x < 0 || x >= DM2_WORLD_STATE_MAP_EDGE ||
        y < 0 || y >= DM2_WORLD_STATE_MAP_EDGE) {
        return 0;
    }
    bit_index = y * DM2_WORLD_STATE_MAP_EDGE + x;
    return (state->explored_by_level[level][bit_index >> 3] & (uint8_t)(1u << (bit_index & 7))) != 0;
}

void dm2_v1_world_state_set_explored(DM2_WorldState *state,
                                      int level, int x, int y, int value) {
    int bit_index;
    uint8_t mask;
    if (!state ||
        level < 0 || level >= DM2_WORLD_STATE_MAX_LEVELS ||
        x < 0 || x >= DM2_WORLD_STATE_MAP_EDGE ||
        y < 0 || y >= DM2_WORLD_STATE_MAP_EDGE) {
        return;
    }
    bit_index = y * DM2_WORLD_STATE_MAP_EDGE + x;
    mask = (uint8_t)(1u << (bit_index & 7));
    if (value) {
        state->explored_by_level[level][bit_index >> 3] |= mask;
    } else {
        state->explored_by_level[level][bit_index >> 3] &= (uint8_t)~mask;
    }
}

/*
 * dm2_v1_world_state_set_current_level — transition the world-state
 * current_level pointer to a new map index.
 *
 * The DM2 per-level explored bitmap is keyed by level index, not by
 * current_level, so changing the pointer MUST NOT wipe previously
 * revealed cells on other levels. This helper enforces the bounds
 * check the field itself cannot provide, and keeps callers from
 * having to mutate the struct field directly. Stair/transition
 * callers (SKULL.ASM T520 stairs handler, ReDMCSB CLIKMENU.C:177-179)
 * and save/load round-trip code (ReDMCSB LOADSAVE.C:1523 PartyMapIndex)
 * use the same pointer-only semantics.
 *
 * Source: ReDMCSB DEFS.H:560 GLOBAL_DATA.PartyMapIndex
 *         ReDMCSB LOADSAVE.C:1515-1524 GLOBAL_DATA round-trip
 *         ReDMCSB CLIKMENU.C:177-179,265 stairs / map transition
 *         SKULL.ASM T520 party placement tick
 */
void dm2_v1_world_state_set_current_level(DM2_WorldState *state, int target_level) {
    if (!state) return;
    if (target_level < 0 || target_level >= DM2_WORLD_STATE_MAX_LEVELS) return;
    state->current_level = target_level;
}

void dm2_v1_world_state_free(DM2_WorldState *state) {
    if (!state) return;
    if (state->raw_save) { free(state->raw_save); state->raw_save = NULL; }
    free(state);
}

const char *dm2_v1_world_state_source_evidence(void) {
    return
        "DM2 V1 World State — Phase 2 World/Data Ingestion\n"
        "Source: docs/dm2_save_format.md — SUPPRESS compression, save sections\n"
        "Source: docs/dm2_quest.md — quest flags and phases\n"
        "Source: docs/dm2_party_state.md — party/champion state\n"
        "Source: docs/dm2_time.md — timer system, game tick\n"
        "Source: SKULL.ASM T520 — party placement from dungeon\n"
        "Source: SKULL.ASM T000 — DM2 startup, save I/O\n"
        "Source: SKULL.ASM SUPPRESS_WRITER/READER — save compression\n"
        "Asset: DM2 PC English SKSave.dat (SUPPRESS format)\n";
}
