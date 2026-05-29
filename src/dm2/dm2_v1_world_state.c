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

/* ── SUPPRESS compression helpers ──────────────────────────────── */
/*
 * SUPPRESS is a bit-plane RLE compression used in DM2 saves.
 * Key idea: per-field masks control which bytes are written.
 * Non-zero nibbles from data+mask are packed LSB-first.
 * Source: docs/dm2_save_format.md — SUPPRESS_WRITER/READER
 */

#define SUPPRESS_MAX_FIELD_MASK  0xFFFF

/*
 * suppress_read_bits — read N bits from a byte stream (LSB first).
 * Source: docs/dm2_save_format.md
 */
static uint32_t suppress_read_bits(const uint8_t **buf, int *bit_offset, int count) {
    uint32_t val = 0;
    for (int i = 0; i < count; i++) {
        int byte_idx = (*bit_offset) >> 3;
        int bit_idx   = (*bit_offset) & 7;
        if (byte_idx >= 4) { /* sentinel check — simplified */
            (*bit_offset)++;
            continue;
        }
        val |= ((*buf)[byte_idx] >> bit_idx) & 1;
        if (i < count - 1) val <<= 1;
        (*bit_offset)++;
    }
    return val;
}

/*
 * suppress_decode_nibble — decode one nibble using a 4-bit mask.
 * Source: docs/dm2_save_format.md
 */
static uint8_t suppress_decode_nibble(uint8_t data, uint8_t mask) {
    if (mask == 0) return 0;
    int nibble_idx = 0;
    for (int b = 0; b < 4; b++) {
        if (mask & (1 << b)) nibble_idx++;
    }
    return data & ((1 << nibble_idx) - 1);
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
    const dm2_dungeon_header_t *hdr;
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

    hdr = (const dm2_dungeon_header_t *)dungeon_data;
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
     * For Phase 2, return a minimal valid buffer with header only.
     * Phase 3+ will implement full SUPPRESS encoding. */
    uint8_t *buf = malloc(64);
    if (!buf) { if (out_size) *out_size = 0; return NULL; }
    memset(buf, 0, 64);
    /* Write save slot magic markers (BEET/DEAD) */
    buf[38] = 0xBE; buf[39] = 0xEF;
    buf[40] = 0xDE; buf[41] = 0xAD;
    if (out_size) *out_size = 64;
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