#ifndef FIRESTAFF_DM2_V1_WORLD_STATE_H
#define FIRESTAFF_DM2_V1_WORLD_STATE_H
/*
 * dm2_v1_world_state.h — DM2 V1 World-State Ingestion
 *
 * DM2 Phase 2: World-state loading, quest state, NPC positions.
 * DM2 maintains more complex world state than DM1:
 *   - Quest flags (per-savegame, global flags)
 *   - NPC/champion positions and states
 *   - World-map state (outdoor levels with buildings/dungeons)
 *   - Weather state (rain, fog per outdoor level)
 *   - Timers and tick generators
 *
 * Source: docs/dm2_quest.md — quest system
 * Source: docs/dm2_party_state.md — party/champion state
 * Source: docs/dm2_time.md — timer system
 * Source: docs/dm2_save_format.md — SUPPRESS save format
 * Source: SKULL.ASM — world map, quest state
 */

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Quest State ─────────────────────────────────────────────────── */
/*
 * DM2 quest system: global flags, quest log, completion state.
 * Source: docs/dm2_quest.md
 */
#define DM2_QUEST_FLAG_COUNT  256
#define DM2_MAX_QUEST_LOG     32

typedef enum {
    DM2_QUEST_PHASE_PROLOGUE  = 0,
    DM2_QUEST_PHASE_MAIN      = 1,
    DM2_QUEST_PHASE_EPILOGUE  = 2,
} DM2_QuestPhase;

typedef struct {
    int        quest_id;
    DM2_QuestPhase phase;
    int        completion_percent;
    int        active_flag;
} DM2_QuestState;

/* ── Champion/NPC State ──────────────────────────────────────────── */
/*
 * DM2 champion state: 4 champions, each with HP/MP/stats/inventory.
 * Source: docs/dm2_party_state.md, docs/dm2_champ_types.md
 */
#define DM2_MAX_CHAMPIONS  4
#define DM2_CHAMP_MAX_HP  999
#define DM2_CHAMP_MAX_MP  999

typedef enum {
    DM2_CHAMP_CLASS_NOVICE  = 0,
    DM2_CHAMP_CLASS_WARRIOR = 1,
    DM2_CHAMP_CLASS_PALADIN = 2,
    DM2_CHAMP_CLASS_ROBED   = 3,
    DM2_CHAMP_CLASS_MYSTIC  = 4,
    DM2_CHAMP_CLASS_NINJA   = 5,
} DM2_ChampionClass;

typedef enum {
    DM2_CHAMP_CONDITION_HEALTHY   = 0,
    DM2_CHAMP_CONDITION_POISONED  = 1,
    DM2_CHAMP_CONDITION_PLAGUED   = 2,
    DM2_CHAMP_CONDITION_DEAD     = 3,
    DM2_CHAMP_CONDITION_STONED   = 4,
    DM2_CHAMP_CONDITION_ASHES    = 5,
} DM2_ChampionCondition;

/* Champion record (statically sized, matches SKSAVE.DAT layout) */
typedef struct {
    uint8_t  class;
    uint8_t  level;
    int16_t  x, y;
    int16_t  hp, max_hp;
    int16_t  mp, max_mp;
    int16_t  food;
    int16_t  water;
    uint8_t  condition;
    uint8_t  inventory_count;
    uint8_t  strength, agility, stamina, mana;
    uint8_t  gold;
    uint8_t  experience;
    uint8_t  equipped[6];
} DM2_ChampionState;

typedef struct {
    int       champion_count;
    DM2_ChampionState champions[DM2_MAX_CHAMPIONS];
    int       leader_index;
    int       party_gold;
} DM2_PartyState;

/* ── Weather State ───────────────────────────────────────────────── */
/*
 * DM2 outdoor levels have weather: rain, fog.
 * Source: docs/dm2_weather.md, dm2_v1_weather.c
 */
typedef enum {
    DM2_WEATHER_CLEAR   = 0,
    DM2_WEATHER_RAIN    = 1,
    DM2_WEATHER_FOG     = 2,
    DM2_WEATHER_STORM   = 3,
} DM2_WeatherType;

typedef struct {
    int       weather_type;   /* DM2_WEATHER_* */
    int       intensity;      /* 0-100 */
    int       duration;       /* in-game ticks remaining */
} DM2_WeatherState;

/* ── World State Container ───────────────────────────────────────── */

typedef struct {
    /* Dungeon state */
    DM2_PartyState       party;
    DM2_QuestState       quests[DM2_MAX_QUEST_LOG];
    int                  quest_count;
    uint8_t              global_flags[DM2_QUEST_FLAG_COUNT];

    /* Outdoor / world-map state */
    int                  current_level;
    int                  outdoor_level_count;
    DM2_WeatherState     weather_by_level[30];

    /* Timers */
    int                  timer_count;
    int                  game_tick;

    /* Raw save data reference (for SUPPRESS round-trip) */
    uint8_t             *raw_save;
    size_t               raw_save_size;
} DM2_WorldState;

/* ── World-State Loading ─────────────────────────────────────────── */

/* Load world state from DM2 SKSave.dat (SUPPRESS-encoded single-file save).
 * Returns world state on success (caller owns), NULL on failure.
 * Source: docs/dm2_save_format.md, SKULL.ASM SUPPRESS compression */
DM2_WorldState *dm2_v1_world_state_load_from_file(const char *path);

/* Load world state from in-memory SUPPRESS buffer.
 * Source: docs/dm2_save_format.md */
DM2_WorldState *dm2_v1_world_state_load_from_mem(const uint8_t *data, size_t size);

/* Extract party state from DUNGEON.DAT (without savegame).
 * Used for new game initialization.
 * Source: SKULL.ASM T520 party placement */
DM2_WorldState *dm2_v1_world_state_new_from_dungeon(const uint8_t *dungeon_data, size_t size);

/* Save world state to a SUPPRESS-encoded buffer (caller frees).
 * Returns buffer size in *out_size on success.
 * Source: docs/dm2_save_format.md */
uint8_t *dm2_v1_world_state_serialize(const DM2_WorldState *state, size_t *out_size);

/* Free world state and all owned resources. Safe with NULL. */
void dm2_v1_world_state_free(DM2_WorldState *state);

/* Accessors */
int dm2_v1_world_state_get_quest_flag(const DM2_WorldState *state, int flag_index);
void dm2_v1_world_state_set_quest_flag(DM2_WorldState *state, int flag_index, int value);
int dm2_v1_world_state_get_champion_hp(const DM2_WorldState *state, int champ_index);
const char *dm2_v1_world_state_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_WORLD_STATE_H */