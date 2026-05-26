/* DM2 V1 Save/Load — public API
 *
 * Source lock:
 *   SKULL.ASM: save/load entry points, SUPPRESS codec
 *   docs/dm2_save_format.md — full format specification
 *   docs/dm2_save_slots.md — 10-slot system with 0xBEEF/0xDEAD magic
 *   docs/dm2_party_state.md — champion squad persistence
 */

#ifndef FIRESTAFF_DM2_V1_SAVE_LOAD_H
#define FIRESTAFF_DM2_V1_SAVE_LOAD_H
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ════════════════════════════════════════════════════════════════
 * SUPPRESS codec — bit-level RLE used throughout DM2 save files
 * ReDMCSB: SUPPRESS_WRITER / SUPPRESS_READER
 * ════════════════════════════════════════════════════════════════ */

/* Encode nibble-pairs (data[i], mask[i]) into compact byte stream.
 * mask low nibble: 0=skip field; 1..7 = number of LSB bits to store.
 * Returns output byte count, or -1 on error. */
int dm2_suppress_encode(const uint8_t *data, const uint8_t *mask,
                        size_t count, uint8_t *out, size_t out_capacity);

/* Decode SUPPRESS stream → flat array.
 * fill=0: absent fields stay 0x00; fill=1: absent fields → 0xFF.
 * Returns bytes consumed from input stream, or -1 on underflow. */
int dm2_suppress_decode(const uint8_t *in, size_t in_capacity,
                        const uint8_t *mask, size_t count,
                        uint8_t *out, uint8_t fill);

/* Self-test: encode + decode a known vector; verify round-trip. */
int dm2_suppress_self_verification(void);

/* ════════════════════════════════════════════════════════════════
 * Slot manager — 10-slot system matching SKSave%02u.dat layout
 * Slot is valid when header w38==0xBEEF && w40==0xDEAD
 * Source: docs/dm2_save_slots.md
 * ════════════════════════════════════════════════════════════════ */

#define DM2_SLOT_MAX      10
#define DM2_SLOT_NAME_MAX 33

typedef struct {
    bool     occupied;
    char     name[DM2_SLOT_NAME_MAX + 1];
    uint32_t timestamp;
} DM2_SL_SlotInfo;

typedef struct {
    DM2_SL_SlotInfo slots[DM2_SLOT_MAX];
    uint8_t slot_count;
    char    save_base[256];
    bool    initialized;
} DM2_SL_State;

/* Initialise slot manager with save base directory (NULL = cwd). */
void dm2_sl_init(DM2_SL_State *state, const char *save_base);

/* Scan all 10 slots; populates state->slots[]. */
bool dm2_sl_scan_slots(DM2_SL_State *state);

/* True if slot[N] is occupied (0xBEEF/0xDEAD magic present). */
bool dm2_sl_slot_occupied(const DM2_SL_State *state, uint8_t slot);

/* Slot display name, or NULL if empty. */
const char *dm2_sl_slot_name(const DM2_SL_State *state, uint8_t slot);

/* Save to slot N: renames existing to SKSave.bak, writes new.
 * name can be NULL (anonymous). Returns 0 on success. */
int dm2_sl_save(const char *save_base, uint8_t slot,
                 const char *name,
                 const uint8_t *data, size_t data_size);

/* Load from slot N: tries SKSave%02u.dat first, falls back to SKSave.bak.
 * Returns 0 on success; sets *out_size to bytes read. */
int dm2_sl_load(const char *save_base, uint8_t slot,
                 uint8_t *data, size_t max_size, size_t *out_size);

/* Delete slot N (removes both .dat and .bak). */
int dm2_sl_delete(const char *save_base, uint8_t slot);

/* ════════════════════════════════════════════════════════════════
 * High-level public API
 * ════════════════════════════════════════════════════════════════ */

uint8_t dm2_v1_save_slot_count(void);   /* → 10 */
bool   dm2_v1_save_slot_valid(uint8_t slot);

/* True if SKSave%02u.dat has valid 0xBEEF/0xDEAD slot header. */
bool dm2_v1_save_has_valid_slot(const char *save_base, uint8_t slot);

/* Run dm2_suppress_self_verification; returns true on success. */
bool dm2_v1_save_suppress_self_test(void);

/* ════════════════════════════════════════════════════════════════
 * Cross-version diagnostics
 * ════════════════════════════════════════════════════════════════ */

enum {
    DM2V1_VERSION_UNKNOWN = 0,
    DM2V1_VERSION_DM2      = 2,
    DM2V1_VERSION_DM1      = 1,
};

enum {
    DM2V1_SAVE_DIAG_NULL_FILL     = 1 << 0,
    DM2V1_SAVE_DIAG_SUPPRESS_FILL = 1 << 1,
    DM2V1_SAVE_DIAG_TRUNCATED     = 1 << 2,
};

/* Scan save blob and return diagnostic flags. */
int dm2_v1_save_version_diagnostics(const uint8_t *data, size_t size);

/* Examine a 42-byte slot header; return VERSION_DM2/DM1/UNKNOWN. */
int dm2_v1_save_detect_game_version(const uint8_t *header42);

/* Source evidence string */
const char *dm2_v1_save_source_evidence(void);

/* ════════════════════════════════════════════════════════════════
 * Game state block (56 bytes, SUPPRESS-encoded)
 * Source: docs/dm2_save_format.md § Game state block (skload_table_60)
 * ════════════════════════════════════════════════════════════════ */

#define DM2_GAME_STATE_BLOCK_SIZE 56

typedef struct {
    uint32_t dwGameTick;
    uint32_t dwRandomSeed;
    uint16_t wChampionsCount;
    uint16_t wPlayerPosX;
    uint16_t wPlayerPosY;
    uint16_t wPlayerDir;
    uint16_t wPlayerMap;
    uint16_t wChampionLeader;
    uint16_t wTimersCount;
    uint8_t  rain_state[8];
    uint32_t _dw22;
    uint32_t _dw26;
    uint16_t _w30;
    uint16_t _w34;
} DM2_GameStateBlock;

int dm2_suppress_encode_gamestate(const DM2_GameStateBlock *gs,
                                   uint8_t *out, size_t out_sz);

int dm2_suppress_decode_gamestate(const uint8_t *in, size_t in_sz,
                                   DM2_GameStateBlock *gs, uint8_t fill);

/* ════════════════════════════════════════════════════════════════
 * Global variables (flags/bytes/words) — SUPPRESS encoded
 * Source: docs/dm2_save_format.md § Ingame global flags/bytes/words
 * ════════════════════════════════════════════════════════════════ */

#define DM2_GLOBAL_FLAGS_SIZE  8
#define DM2_GLOBAL_BYTES_SIZE  64
#define DM2_GLOBAL_WORDS_SIZE  64

int dm2_suppress_encode_global_flags(const uint8_t flags[DM2_GLOBAL_FLAGS_SIZE],
                                     uint8_t *out, size_t out_sz);

int dm2_suppress_decode_global_flags(const uint8_t *in, size_t in_sz,
                                     uint8_t flags[DM2_GLOBAL_FLAGS_SIZE],
                                     uint8_t fill);

int dm2_suppress_encode_global_bytes(const uint8_t bytes[DM2_GLOBAL_BYTES_SIZE],
                                     uint8_t *out, size_t out_sz);

int dm2_suppress_decode_global_bytes(const uint8_t *in, size_t in_sz,
                                     uint8_t bytes[DM2_GLOBAL_BYTES_SIZE],
                                     uint8_t fill);

int dm2_suppress_encode_global_words(const uint16_t words[DM2_GLOBAL_WORDS_SIZE],
                                     uint8_t *out, size_t out_sz);

int dm2_suppress_decode_global_words(const uint8_t *in, size_t in_sz,
                                     uint16_t words[DM2_GLOBAL_WORDS_SIZE],
                                     uint8_t fill);

/* ════════════════════════════════════════════════════════════════
 * Global spell effects (6 bytes, SUPPRESS)
 * Source: docs/dm2_party_state.md § Global spell effects
 * ════════════════════════════════════════════════════════════════ */

#define DM2_GLOBAL_SPELL_EFFECTS_SIZE 6

int dm2_suppress_encode_spell_effects(const uint8_t effects[DM2_GLOBAL_SPELL_EFFECTS_SIZE],
                                       uint8_t *out, size_t out_sz);

int dm2_suppress_decode_spell_effects(const uint8_t *in, size_t in_sz,
                                       uint8_t effects[DM2_GLOBAL_SPELL_EFFECTS_SIZE],
                                       uint8_t fill);

/* ════════════════════════════════════════════════════════════════
 * Timers table (10 bytes per timer, SUPPRESS)
 * Source: docs/dm2_save_format.md § Timers table
 * ════════════════════════════════════════════════════════════════ */

#define DM2_TIMER_ENTRY_SIZE  10
#define DM2_MAX_TIMERS        32

typedef struct {
    uint16_t timer_id;
    uint16_t current_tick;
    uint16_t interval_ticks;
    uint16_t flags;
    uint16_t user_data;
} DM2_TimerEntry;

int dm2_suppress_encode_timer(const DM2_TimerEntry *t,
                               uint8_t *out, size_t out_sz);

int dm2_suppress_decode_timer(const uint8_t *in, size_t in_sz,
                               DM2_TimerEntry *t, uint8_t fill);

/* ════════════════════════════════════════════════════════════════
 * Minion association table
 * Source: docs/dm2_party_state.md § Minion Association
 * ════════════════════════════════════════════════════════════════ */

#define DM2_MAX_MINIONS 16

typedef struct {
    uint32_t object_id;
    uint32_t owner_champion;
} DM2_MinionAssoc;

typedef struct {
    DM2_MinionAssoc entries[DM2_MAX_MINIONS];
    uint8_t count;
} DM2_MinionTable;

size_t dm2_minion_table_size(const DM2_MinionTable *t);

int dm2_minion_write(const DM2_MinionTable *t, FILE *f);

int dm2_minion_read(DM2_MinionTable *t, FILE *f);

/* ════════════════════════════════════════════════════════════════
 * Champion inventory serialization via WRITE_RECORD_CHECKCODE
 * Source: docs/dm2_party_state.md § Inventory: The Item Record Chain
 * ════════════════════════════════════════════════════════════════ */

#define DM2_CHAMPION_INVENTORY_SLOTS 30

int dm2_champion_inventory_write(const uint32_t inventory[DM2_CHAMPION_INVENTORY_SLOTS],
                                   FILE *f);

int dm2_champion_inventory_read(uint32_t inventory[DM2_CHAMPION_INVENTORY_SLOTS],
                                 FILE *f);

/* ════════════════════════════════════════════════════════════════
 * Leader hand possession
 * Source: docs/dm2_party_state.md § Leader Hand Possession
 * ════════════════════════════════════════════════════════════════ */

typedef struct {
    uint32_t object; /* ObjectID handle */
} DM2_LeaderPossession;

int dm2_leader_possession_write(const DM2_LeaderPossession *lp, FILE *f);

int dm2_leader_possession_read(DM2_LeaderPossession *lp, FILE *f);

/* ════════════════════════════════════════════════════════════════
 * PC savegame interoperability
 * Source: docs/dm2_save_format.md § DM1 vs DM2 Key Format Differences
 * ════════════════════════════════════════════════════════════════ */

enum {
    DM2_PC_SAVE_DM2     = 0,
    DM2_PC_SAVE_DM1     = 1,
    DM2_PC_SAVE_UNKNOWN = 2,
};

/* Detect the type of PC savegame from raw data */
int dm2_pc_save_detect_type(const uint8_t *data, size_t size);

/* PC savegame interoperability report */
const char *dm2_pc_save_interoperability_report(const uint8_t *data, size_t size);

/* Phase 7 source evidence */
const char *dm2_v1_save_phase7_source_evidence(void);

/* ════════════════════════════════════════════════════════════════
 * Champion persistence — 261 byte SUPPRESS-encoded records
 * Source: docs/dm2_party_state.md
 * ════════════════════════════════════════════════════════════════ */

#define DM2_CHAMPION_NAME_FIRST_LEN   8
#define DM2_CHAMPION_NAME_LAST_LEN   16
#define DM2_CHAMPION_INVENTORY_SLOTS 30

/* Champion record (261 bytes, SUPPRESS-encoded on save).
 * Matches the in-memory glbChampionSquad[4] layout from SKULL.ASM. */
typedef struct {
    char     first_name[DM2_CHAMPION_NAME_FIRST_LEN];
    char     last_name[DM2_CHAMPION_NAME_LAST_LEN];
    uint16_t absolute_direction;   /* 0-3: N/E/S/W */
    uint8_t  squad_position;        /* 0-3: TL/TR/BL/BR */
    uint16_t cur_hp, max_hp;
    uint16_t stamina;
    uint16_t mana;
    uint8_t  poison_value;
    uint8_t  runes_count;
    uint8_t  spelled_runes[4];
    uint16_t attributes[7][2];     /* cur/max pairs */
    int16_t  food;
    int16_t  water;
    uint32_t hand_command[2];
    uint16_t hand_cooldown[2];
    uint8_t  hand_defense_class[2];
    uint8_t  timer_index;
    uint8_t  damage_suffered;
    uint8_t  hero_flag;
    uint8_t  body_flag;
    uint32_t inventory[DM2_CHAMPION_INVENTORY_SLOTS]; /* ObjectID handles */
} DM2_ChampionRecord;

/* Fill mask[261] with SUPPRESS mask for a DM2 champion record.
 * Used with dm2_suppress_encode/decode for champion serialization. */
void dm2_suppress_champion_mask(uint8_t mask[261]);

int dm2_suppress_encode_champion(const DM2_ChampionRecord *c,
                                  const uint8_t *mask,
                                  uint8_t *out, size_t out_sz);

int dm2_suppress_decode_champion(const uint8_t *in, size_t in_sz,
                                  const uint8_t *mask,
                                  DM2_ChampionRecord *c,
                                  uint8_t fill);

/* ════════════════════════════════════════════════════════════════
 * Object/container DB record pools
 * Source: docs/dm2_save_format.md § DB record pools
 * ════════════════════════════════════════════════════════════════ */

#define DM2_DB_POOL_COUNT 16

typedef struct {
    uint8_t *data;
    uint32_t rec_count;
    uint32_t rec_size;
} DM2_DB_Pool;

typedef struct {
    DM2_DB_Pool pools[DM2_DB_POOL_COUNT];
} DM2_DB_State;

/* ObjectID handle: high byte = pool (0-15), low 24 bits = rec index.
 * Returns false if handle is 0 or out of range for the given DB. */
bool dm2_db_resolve(uint32_t object_id,
                     const DM2_DB_State *db,
                     uint8_t *out_pool, uint32_t *out_index);

/* Inverse of dm2_db_resolve: pool + index → ObjectID handle.
 * Returns 0 if pool is out of range. */
uint32_t dm2_db_make_handle(uint8_t pool, uint32_t index);

/* Write one fixed-size DB record for pool[index] to file f. */
bool dm2_db_write_record(uint8_t pool, uint32_t index,
                          FILE *f,
                          const DM2_DB_State *db);

/* Trace an inventory slot's object chain for max_depth hops.
 * Returns the starting handle (caller walks the chain). */
uint32_t dm2_db_trace_inventory_slot(const uint32_t inventory[DM2_CHAMPION_INVENTORY_SLOTS],
                                     uint8_t slot,
                                     uint8_t max_depth,
                                     const DM2_DB_State *db);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_SAVE_LOAD_H */
