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

/* Source-faithful SUPPRESS stream state.
 * SKProject's DM2_SUPPRESS_WRITER/READER scan each mask byte from bit 7
 * through bit 0. A set mask bit selects the corresponding source-data bit.
 * Bits are emitted MSB-first and adjacent save sections share this state
 * until the writer is flushed. */
typedef struct {
    uint8_t pending_byte;
    uint8_t pending_bits;
} DM2_SuppressWriter;

typedef struct {
    const uint8_t *data;
    size_t size;
    size_t position;
    uint8_t current_byte;
    uint8_t bits_remaining;
} DM2_SuppressReader;

void dm2_suppress_writer_init(DM2_SuppressWriter *writer);
int dm2_suppress_writer_write(DM2_SuppressWriter *writer,
                              const uint8_t *data, const uint8_t *mask,
                              size_t count, uint8_t *out,
                              size_t out_capacity, size_t *out_size);
int dm2_suppress_writer_flush(DM2_SuppressWriter *writer,
                              uint8_t *out, size_t out_capacity,
                              size_t *out_size);

/* Write a single bit (0 or 1) into the SUPPRESS stream.
 * Source: sksvgame.cpp DM2_WRITE_1BIT — calls SUPPRESS_WRITER with mask=1. */
int dm2_suppress_writer_write_bit(DM2_SuppressWriter *writer,
                                  int bit_value,
                                  uint8_t *out, size_t out_capacity,
                                  size_t *out_size);

void dm2_suppress_reader_init(DM2_SuppressReader *reader,
                              const uint8_t *data, size_t size);

/* Read a single bit from the SUPPRESS stream.
 * Source: sksvgame.cpp DM2_READ_1BIT — calls SUPPRESS_READER with mask=1. */
int dm2_suppress_reader_read_bit(DM2_SuppressReader *reader, int *out_bit);
int dm2_suppress_reader_read(DM2_SuppressReader *reader,
                             const uint8_t *mask, size_t count,
                             uint8_t *out, uint8_t fill);

/* Encode data+mask pairs into one self-contained compact byte stream.
 * Each set bit in mask[i] stores the matching data[i] bit. Returns output
 * byte count, or -1 on error. */
int dm2_suppress_encode(const uint8_t *data, const uint8_t *mask,
                        size_t count, uint8_t *out, size_t out_capacity);

/* Decode SUPPRESS stream → flat array.
 * fill=0: absent/masked-off bits stay 0x00; fill=1: they stay 0xFF.
 * Returns bytes consumed from input stream, or -1 on underflow. */
int dm2_suppress_decode(const uint8_t *in, size_t in_capacity,
                        const uint8_t *mask, size_t count,
                        uint8_t *out, uint8_t fill);

enum {
    DM2_V1_SAVE_SUPPRESS_SYMBOL_INIT = 1u << 0,
    DM2_V1_SAVE_SUPPRESS_SYMBOL_WRITER = 1u << 1,
    DM2_V1_SAVE_SUPPRESS_SYMBOL_FLUSH = 1u << 2,
    DM2_V1_SAVE_SUPPRESS_SYMBOL_READER = 1u << 3,
    DM2_V1_SAVE_SUPPRESS_SYMBOL_WRITE_1BIT = 1u << 4,
    DM2_V1_SAVE_SUPPRESS_SYMBOL_READ_1BIT = 1u << 5
};

typedef struct {
    int valid;
    uint32_t covered_symbol_mask;
    int init_ready;
    int writer_ready;
    int flush_ready;
    int reader_ready;
    int write_1bit_ready;
    int read_1bit_ready;
    int section_carry_ready;
    int fill_zero_ready;
    int fill_one_ready;
    int underflow_rejected;
    uint8_t source_vector_hash;
    uint8_t mask_vector_hash;
    uint8_t encoded_vector_hash;
    uint8_t decoded_vector_hash;
    uint8_t carry_encoded_byte;
    uint8_t first_section_decoded;
    uint8_t second_section_decoded;
    size_t encoded_size;
    size_t reader_position_after_decode;
    uint32_t receipt_hash;
} DM2_V1_SaveSuppressSymbolReceipt;

/* Source-named receipt for SKProject c_savegame.cpp's SUPPRESS_INIT,
 * SUPPRESS_WRITER, SUPPRESS_FLUSH, SUPPRESS_READER, WRITE_1BIT, and READ_1BIT
 * bitstream contract. It proves MSB-first source-bit selection, cross-section
 * carry, fill policy, and underflow rejection without inventing save payloads. */
int dm2_v1_save_suppress_symbol_receipt(
    DM2_V1_SaveSuppressSymbolReceipt *out_receipt);

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

#define DM2_SK_CORPUS_RECEIPT_MAX 16u
typedef struct {
    int kind;
    int import_rejected;
    size_t payload_size;
    uint32_t payload_hash;
    /* FNV-1a receipt over the complete original file, including the 42-byte
     * SKSave header. It detects a changed corpus artifact before import. */
    uint32_t source_file_hash;
    char path[256];
} DM2_SKSaveCandidateReceipt;

typedef enum {
    DM2_SK_SAVE_KIND_NONE = 0,
    DM2_SK_SAVE_KIND_FIRESTAFF_SESSION = 1,
    DM2_SK_SAVE_KIND_ORIGINAL_ENVELOPE = 2,
    DM2_SK_SAVE_KIND_ORIGINAL_RAW = 3
} DM2_SKSaveKind;

typedef struct {
    uint8_t  valid_slot_count;
    uint16_t valid_slot_mask;
    bool     has_last_session;
    bool     has_last_session_backup;
    bool     last_session_uses_backup;
    uint8_t  invalid_candidate_count;
    uint8_t  importable_candidate_count;
    uint8_t  import_rejected_candidate_count;
    uint8_t  firestaff_session_candidate_count;
    uint8_t  original_envelope_candidate_count;
    uint8_t  original_raw_candidate_count;
    uint16_t recursive_candidate_count;
    uint16_t recursive_importable_candidate_count;
    uint16_t alternate_name_candidate_count;
    /* A corpus artifact may have been renamed outside SKProject's direct
     * resume path.  Retain only files independently admitted by the exact
     * 42-byte SKSave header and payload parser, never by extension alone. */
    uint16_t header_discovered_candidate_count;
    uint16_t extra_valid_candidate_count;
    uint16_t recursive_scan_depth_limit;
    uint16_t recursive_scan_candidate_cap;
    uint8_t  recursive_scan_truncated;
    size_t   largest_payload_size;
    size_t   total_payload_size;
    size_t   largest_importable_payload_size;
    size_t   total_importable_payload_size;
    size_t   first_importable_payload_size;
    DM2_SKSaveKind first_importable_kind;
    uint32_t importable_kind_mask;
    uint32_t importable_payload_hash;
    uint8_t candidate_receipt_count;
    DM2_SKSaveCandidateReceipt candidate_receipts[DM2_SK_CORPUS_RECEIPT_MAX];
    char     first_valid_path[256];
    char     first_importable_path[256];
} DM2_SKSaveCorpusReceipt;

/* Skip-safe provenance gate for the still-unmapped live weather timer area.
 * A valid SKSave header proves only a save candidate, never a timer layout. */
typedef struct {
    int scan_complete;
    int has_header_verified_candidate;
    int live_distant_environment_timer_present;
    int skipped_missing_live_timer;
    uint32_t verified_payload_bytes;
    uint32_t matching_timer_record_count;
    uint32_t corpus_hash;
} DM2_DistantEnvironmentTimerCorpusReceipt;

/* Raw-only inventory of original save candidates considered by a timer-format
 * probe. A slot header and a parsed envelope/raw candidate do not identify a
 * timer owner or wire layout, so every retained row remains rejected. */
typedef struct {
    int scan_complete;
    int has_header_verified_candidate;
    int timer_layout_owner_proven;
    int matching_timer_record_count;
    int original_candidate_list_complete;
    uint16_t original_candidate_count;
    uint16_t rejected_unowned_candidate_count;
    uint32_t retained_original_payload_bytes;
    uint32_t corpus_hash;
    uint8_t candidate_receipt_count;
    DM2_SKSaveCandidateReceipt
        candidate_receipts[DM2_SK_CORPUS_RECEIPT_MAX];
} DM2_OriginalTimerFormatCorpusReceipt;

enum { DM2_ORIGINAL_SAVE_RAW_DB_POOL_COUNT = 16 };

/* Read-only census of the original save fields already decoded by the
 * source-locked SKSave importer.  This is not a restore input: dungeon DB
 * records and timer payload ownership remain outside this receipt until their
 * original byte-level contracts are proven. */
typedef struct {
    DM2_SKSaveCandidateReceipt candidate;
    uint32_t game_tick;
    uint32_t rng_seed;
    uint16_t party_x;
    uint16_t party_y;
    uint8_t party_dir;
    uint8_t party_map;
    uint8_t champion_count;
    uint8_t timer_count;
    uint8_t rain_intensity;
    /* GAME_LOAD restores these SUPPRESS sections before it sorts timers and
     * rebuilds the saved dungeon. Hashes preserve their original byte
     * identities without assigning unproven timer or DB semantics. */
    uint32_t global_flags_hash;
    uint32_t global_bytes_hash;
    uint32_t global_words_hash;
    uint32_t spell_effects_hash;
    uint32_t raw_timer_stream_offset;
    uint32_t raw_timer_stream_byte_count;
    uint32_t raw_timer_stream_hash;
    /* Raw SKSave candidates additionally retain only the complete parsed
     * dungeon-prefix identity. These are pool/span facts, not DB semantics
     * or permission to follow GenericRecord links. */
    int raw_dungeon_layout_valid;
    uint8_t raw_dungeon_map_count;
    uint16_t raw_db_record_counts[DM2_ORIGINAL_SAVE_RAW_DB_POOL_COUNT];
    uint32_t raw_dungeon_prefix_hash;
    uint32_t raw_map_data_hash;
    uint32_t state_hash;
} DM2_OriginalSaveStateCorpusEntry;

typedef struct {
    int scan_complete;
    int original_candidate_list_complete;
    uint16_t original_candidate_count;
    uint16_t parsed_candidate_count;
    uint16_t rejected_candidate_count;
    uint32_t corpus_hash;
    uint8_t entry_count;
    DM2_OriginalSaveStateCorpusEntry
        entries[DM2_SK_CORPUS_RECEIPT_MAX];
} DM2_OriginalSaveStateCorpusReceipt;

bool dm2_v1_original_save_state_corpus_probe(
    const char *save_base,
    DM2_OriginalSaveStateCorpusReceipt *out_receipt);

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

/* Save original-style last-session file SKSave.dat and rotate the old
 * primary to SKSave.bak. */
int dm2_sl_save_last_session(const char *save_base,
                             const char *name,
                             const uint8_t *data,
                             size_t data_size);

/* Load from slot N: tries SKSave%02u.dat first, falls back to SKSave.bak
 * when the primary file is missing, truncated, or fails the DM2 slot-header
 * magic check. Returns 0 on success; sets *out_size to bytes read. */
int dm2_sl_load(const char *save_base, uint8_t slot,
                 uint8_t *data, size_t max_size, size_t *out_size);

/* Load the original-style resume file SKSave.dat, falling back to
 * SKSave.bak when the primary is missing, truncated, or has a bad header. */
int dm2_sl_load_last_session(const char *save_base,
                             uint8_t *data,
                             size_t max_size,
                             size_t *out_size);

/* Delete slot N (removes both .dat and .bak). */
int dm2_sl_delete(const char *save_base, uint8_t slot);

/* ════════════════════════════════════════════════════════════════
 * High-level public API
 * ════════════════════════════════════════════════════════════════ */

uint8_t dm2_v1_save_slot_count(void);   /* → 10 */
bool   dm2_v1_save_slot_valid(uint8_t slot);

/* True if SKSave%02u.dat has valid 0xBEEF/0xDEAD slot header. */
bool dm2_v1_save_has_valid_slot(const char *save_base, uint8_t slot);

/* True if SKSave.dat or SKSave.bak has a valid 0xBEEF/0xDEAD header. */
bool dm2_v1_save_has_valid_last_session(const char *save_base);

/* Scan a directory containing original-style SKSave.dat/SKSave.bak and
 * SKSave%02u.dat files. This is a lightweight real-save corpus hook: it
 * validates the DM2 42-byte slot header, classifies payloads through the
 * original-format importer used by runtime resume, and reports byte totals
 * without mutating live runtime state. Firestaff-private D2RS blobs remain
 * diagnostic rejections and are never loadable corpus entries. */
bool dm2_v1_sksave_corpus_scan(const char *save_base,
                               DM2_SKSaveCorpusReceipt *out_receipt);

/* Diagnostic-only wrapper for the isolated save-load test.  A live DM2
 * session must validate an actual save candidate; it must not expose a
 * synthetic self-verification entry point. */
#ifdef FIRESTAFF_DM2_SAVE_LOAD_TESTING
bool dm2_v1_save_suppress_self_test(void);
#endif

bool dm2_v1_sksave_corpus_load_receipted_candidate(
    const DM2_SKSaveCandidateReceipt *candidate_receipt,
    uint8_t *out_payload,
    size_t out_capacity,
    size_t *out_payload_size);

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

/* Packed wire-layout view of SKProject's skload_table_60. The original DM2
 * save block is byte-addressed; do not let host alignment move dw22 from byte
 * 22. Source: SKWIN/DME.h:956-990 and SKULLWIN/c_savegame.cpp:1483-1517. */
#if defined(_MSC_VER)
#pragma pack(push, 1)
#endif
typedef struct
#if defined(__GNUC__) || defined(__clang__)
__attribute__((packed))
#endif
{
    uint32_t dwGameTick;
    uint32_t dwRandomSeed;
    uint16_t wChampionsCount;
    uint16_t wPlayerPosX;
    uint16_t wPlayerPosY;
    uint16_t wPlayerDir;
    uint16_t wPlayerMap;
    uint16_t wChampionLeader;
    uint16_t wTimersCount;
    uint32_t dw22;
    uint32_t dw26;
    uint16_t w30;
    uint16_t wPlayerThrowCounter;
    uint16_t w34;
    uint8_t  b36;
    uint8_t  b37;
    uint8_t  b38;
    uint8_t  b39;
    uint16_t wRainFlagSomething;
    uint8_t  bRainAmbientLightModifier;
    uint8_t  bRainDirection;
    uint8_t  bRainStrength;
    uint8_t  bRainLevelForSky;
    uint8_t  bRainLevelForGround;
    uint8_t  bRainMultiplicator;
    uint16_t wRainStormController;
    uint8_t  bRainRelated3;
    uint8_t  bRainRelated2;
    uint32_t dwRainSpecialNextTick;
} DM2_GameStateBlock;
#if defined(_MSC_VER)
#pragma pack(pop)
#endif

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
    /* Firestaff session-tail metadata: stored in the unused 261-byte record
     * tail for bounded startup/rendering. Original SUPPRESS masks leave this
     * byte untouched, so source save import/export fields are not displaced. */
    uint8_t  portrait_index;
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

/* Decode/format a DM2 ObjectID without requiring a loaded DB pool. Startup
 * and UI handoff paths use this to preserve DM2 handle identity instead of
 * truncating it to a DM1/CSB THING. */
bool dm2_db_decode_handle(uint32_t object_id,
                          uint8_t *out_pool,
                          uint32_t *out_index);
const char *dm2_db_pool_label(uint8_t pool);
bool dm2_db_format_handle_name(uint32_t object_id,
                               char *out,
                               size_t out_size);

/* Write one fixed-size DB record for pool[index] to file f. */
bool dm2_db_write_record(uint8_t pool, uint32_t index,
                          FILE *f,
                          const DM2_DB_State *db);

/* ════════════════════════════════════════════════════════════════
 * Save game writing — DM2_GAME_SAVE_MENU flow
 * Source: skproject/SKULLWIN/c_savegame.cpp:2087
 * ════════════════════════════════════════════════════════════════ */

typedef struct {
    int valid;
    int header_written;
    int sgwords_written;
    int gamestate_written;
    int global_flags_written;
    int global_bytes_written;
    int global_words_written;
    int champions_written;
    int timers_written;
    int inventories_written;
    int fail_closed;
    size_t total_bytes_written;
    uint32_t receipt_hash;
} DM2_V1_SaveWriteReceipt;

int dm2_v1_save_game_write(const char *path,
                           const DM2_GameStateBlock *gamestate,
                           const uint8_t global_flags[DM2_GLOBAL_FLAGS_SIZE],
                           const uint8_t global_bytes[DM2_GLOBAL_BYTES_SIZE],
                           const uint16_t global_words[DM2_GLOBAL_WORDS_SIZE],
                           const DM2_ChampionRecord *champions,
                           uint8_t champion_count,
                           const DM2_TimerEntry *timers,
                           uint8_t timer_count,
                           DM2_V1_SaveWriteReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

bool dm2_v1_distant_environment_timer_corpus_probe( const char *save_base, DM2_DistantEnvironmentTimerCorpusReceipt *out_receipt);

bool dm2_v1_original_timer_format_corpus_probe( const char *save_base, DM2_OriginalTimerFormatCorpusReceipt *out_receipt);

bool dm2_v1_sksave_corpus_load_first_importable( const char *save_base, uint8_t *out_payload, size_t out_capacity, size_t *out_payload_size, DM2_SKSaveCorpusReceipt *out_receipt);

#endif /* FIRESTAFF_DM2_V1_SAVE_LOAD_H */
