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
#include "dm2_v1_new_game.h"
#include "dm2_v1_world_model.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int dm2_v1_world_state_has_slot_header(const uint8_t *data, size_t size)
{
    return data && size >= 42u && data[38] == 0xBEu && data[39] == 0xEFu &&
           data[40] == 0xDEu && data[41] == 0xADu;
}

static void dm2_v1_world_state_apply_session(DM2_WorldState *state,
                                             const DM2_V1_SessionState *session)
{
    uint8_t champion_count;

    if (!state || !session) return;
    champion_count = session->champion_count > DM2_MAX_CHAMPIONS
                         ? DM2_MAX_CHAMPIONS : session->champion_count;
    state->game_tick = session->game_tick > (uint32_t)INT_MAX
                           ? INT_MAX : (int)session->game_tick;
    state->timer_count = session->original_timer_count > DM2_MAX_TIMERS
                             ? DM2_MAX_TIMERS : session->original_timer_count;
    state->current_level = session->party_level < DM2_WORLD_STATE_MAX_LEVELS
                               ? session->party_level : 0;
    state->party.champion_count = champion_count;
    state->party.leader_index = session->leader_index < champion_count
                                    ? session->leader_index : 0;
    state->party.party_gold = session->gold > (uint32_t)INT_MAX
                                  ? INT_MAX : (int)session->gold;
    memcpy(state->global_flags, session->original_global_flags,
           DM2_GLOBAL_FLAGS_SIZE);

    if (state->current_level < 30) {
        state->weather_by_level[state->current_level].intensity =
            session->rain_intensity;
        state->weather_by_level[state->current_level].weather_type =
            session->rain_intensity ? DM2_WEATHER_RAIN : DM2_WEATHER_CLEAR;
    }
    for (uint8_t i = 0u; i < champion_count; ++i) {
        const DM2_ChampionRecord *source =
            (const DM2_ChampionRecord *)session->champion_data[i];
        DM2_ChampionState *target = &state->party.champions[i];

        /* These are direct SKSave fields.  Do not infer absent class, level,
         * or maximum-mana fields from a portrait or a current value. */
        target->x = (int16_t)session->party_x;
        target->y = (int16_t)session->party_y;
        target->hp = (int16_t)source->cur_hp;
        target->max_hp = (int16_t)source->max_hp;
        target->mp = (int16_t)source->mana;
        target->food = source->food;
        target->water = source->water;
        target->condition = source->cur_hp == 0u
                                ? DM2_CHAMP_CONDITION_DEAD
                                : DM2_CHAMP_CONDITION_HEALTHY;
    }
}

static DM2_WorldState *dm2_v1_world_state_from_candidate(
    const uint8_t *payload, size_t payload_size)
{
    DM2_V1_SaveCandidate candidate;
    DM2_WorldState *state;

    if (!payload || payload_size == 0u ||
        dm2_v1_session_parse_save_candidate(&candidate, payload,
                                             payload_size) != 0) {
        return NULL;
    }
    state = calloc(1, sizeof(*state));
    if (!state) return NULL;
    state->raw_save = malloc(payload_size);
    if (!state->raw_save) {
        free(state);
        return NULL;
    }
    memcpy(state->raw_save, payload, payload_size);
    state->raw_save_size = payload_size;
    dm2_v1_world_state_apply_session(state, &candidate.session);
    return state;
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
 * SK-projects has a real writer, but it writes the complete source-owned
 * save graph: DM2_GAME_SAVE first records the 0x3c save block, global bytes
 * and words, heroes, timers and then the dungeon through DM2_SUPPRESS_WRITER.
 * This model holds only a bounded load projection, so emitting any buffer
 * would invent missing original state.  Reject until that full writer is
 * ported and bound to verified game data.
 *
 * Source: SKWINSPX/src/v5/sksvgame.cpp::DM2_GAME_SAVE and
 *         ::DM2_SUPPRESS_WRITER (the source-owned save route).
 */
uint8_t *dm2_v1_world_state_serialize(const DM2_WorldState *state, size_t *out_size) {
    (void)state;
    if (out_size) *out_size = 0u;
    return NULL;
}

/*
 * dm2_v1_world_state_load_from_mem — load world state from SUPPRESS buffer.
 * Source: docs/dm2_save_format.md
 */
DM2_WorldState *dm2_v1_world_state_load_from_mem(const uint8_t *data, size_t size) {
    if (!data || size == 0u) return NULL;

    /* SKWIN validates the 42-byte slot envelope before loading its body.
     * Keep that ordering: only strip a demonstrably valid envelope, then let
     * the original-body parser validate all SUPPRESS sections. */
    if (dm2_v1_world_state_has_slot_header(data, size)) {
        if (size <= 42u) return NULL;
        return dm2_v1_world_state_from_candidate(data + 42u, size - 42u);
    }
    return dm2_v1_world_state_from_candidate(data, size);
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
