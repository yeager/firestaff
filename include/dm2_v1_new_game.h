/*
 * dm2_v1_new_game.h — DM2 V1 New Game & Session Management API
 *
 * Phase 6: Utility/import flow — DM2-specific load/start flow.
 *
 * Provides the public admission boundary for new-game and save selection.
 * It deliberately does not construct a starter party or deserialize an
 * incomplete save into a playable session: those mutations belong to the
 * source-owned DM2_GAME_LOAD transaction.
 *
 * Source locks (ReDMCSB WIP20210206):
 *   CHAMPION.C F0280 — CHAMPION_AddCandidateChampionToParty: portrait index
 *     to squad position assignment, initial attribute setting.
 *     SKULL.ASM T520 — party placement and start position (Hall of Champions).
 *     SKULL.ASM T560 — dungeon load completion and party state init.
 *     CHAMPRST.C F0278 — CHAMPION_ResetDataToStartGame: clears hand, clears
 *       champion load/HP/name/title masks on new game.
 *   REQDISK.C F0428 — DIALOG_RequireGameDiskInDrive_NoDialogDrawn: floppy
 *     disk check gate (N/A for modern file-based loading).
 *   docs/dm2_party_state.md — champion record (261 bytes), SUPPRESS mask,
 *     portrait→class mapping, HP/stamina/mana initial values.
 *   docs/dm2_save_format.md — slot header layout (42 bytes), SUPPRESS
 *     game-state block (56 bytes), party state encoding.
 *
 * Production new-game state is not constructed from fixed party defaults.
 * It is admitted only through the source-owned GAME_LOAD/LOAD_NEW_DUNGEON
 * boundary; any test session belongs under tests/.
 */

#ifndef FIRESTAFF_DM2_V1_NEW_GAME_H
#define FIRESTAFF_DM2_V1_NEW_GAME_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* dm2_v1_boot.h defines DM2_V1_BootProfile */
#include "dm2_v1_boot.h"

/* dm2_v1_save_load.h defines DM2_ChampionRecord, DM2_GameStateBlock,
 * SUPPRESS codec, and slot manager */
#include "dm2_v1_save_load.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ════════════════════════════════════════════════════════════════
 * Session state — encapsulates all DM2 runtime state for save/load
 * This is what gets serialized to a save slot.
 * Source: docs/dm2_save_format.md — full save file layout
 * ════════════════════════════════════════════════════════════════ */

/* Maximum serialized session size (conservative estimate) */
#define DM2_SESSION_MAX_SIZE  (2 * 1024 * 1024)

/* Session version marker — written in slot header extension */
#define DM2_SESSION_VERSION   1

/* The PC DOS c_hero stream is 0x107 bytes.  It is deliberately kept apart
 * from DM2_ChampionRecord: that older Firestaff convenience model is not a
 * byte-for-byte SKSave representation.  Source: SKWINDOS/src/c_hero.h and
 * dm2data.cpp::table1d6356. */
#define DM2_V1_ORIGINAL_CHAMPION_RECORD_SIZE 263

/* SKProject c_wbbb/savegames1 is the six-byte source state read by
 * DM2_GAME_LOAD immediately before c_tim. It is raw provenance, not a
 * Firestaff session projection: no scalar gold, reputation, or time owner
 * has been proven in this record. */
#define DM2_V1_ORIGINAL_SAVEGAMES1_SIZE 6

typedef struct DM2_V1_SessionState {
    /* Game tick counter — SUPPRESS-encoded 4-byte field */
    uint32_t game_tick;

    /* RNG seed for this session */
    uint32_t rng_seed;

    /* Party state */
    uint8_t  champion_count;
    uint8_t  leader_index;       /* 0-3, index into champions[] */
    uint16_t party_x;
    uint16_t party_y;
    uint8_t  party_dir;          /* 0=N 1=E 2=S 3=W */
    uint8_t  party_level;        /* current dungeon/outdoor level */
    uint8_t  outdoor_mode;       /* 0=dungeon 1=outdoor 2=building */

    /* Time-of-day (0-1439 minutes) */
    uint16_t time_of_day_minutes;

    /* Resources */
    uint32_t gold;             /* up to ~4 billion gold pieces */
    int16_t  reputation;

    /* Weather (outdoor) */
    uint8_t  rain_intensity;     /* 0-100 */
    uint8_t  weather_padding;

    /* Champion records (4 × 261 bytes, SUPPRESS-encoded) */
    uint8_t  champion_data[4][261];

    /* Bounded original SKSave runtime sections. These mirror the documented
     * SUPPRESS blocks after the game-state block/champion squad; full dungeon
     * DB/state import remains separate from this startup session envelope. */
    uint8_t  original_global_flags[DM2_GLOBAL_FLAGS_SIZE];
    uint8_t  original_global_bytes[DM2_GLOBAL_BYTES_SIZE];
    uint16_t original_global_words[DM2_GLOBAL_WORDS_SIZE];
    uint8_t  original_spell_effects[DM2_GLOBAL_SPELL_EFFECTS_SIZE];
    uint8_t  original_timer_count;
    DM2_TimerEntry original_timers[DM2_MAX_TIMERS];
    uint32_t original_leader_hand_object;
    DM2_MinionTable original_minions;

    /* Retained only after a successful original PC-DOS SUPPRESS decode.
     * Consumers that require source byte positions (formation, hero type,
     * and hand command state) must use this record, never the convenience
     * DM2_ChampionRecord overlay above. */
    uint8_t  original_champion_records[4][DM2_V1_ORIGINAL_CHAMPION_RECORD_SIZE];
    uint8_t  original_champion_records_valid;

    /* Dungeon state (variable — level data) */
    /* Note: full dungeon state saved separately via dungeon_serialize() */

    /* Companion state (4 companions × name+stats) */
    /* Note: companion state serialized as fixed-size records */

    /* Minion table */
    /* Bounded startup/import copy of WRITE_MINION_ASSOC records. Full live
     * minion AI state stays with the later dungeon DB/runtime importer. */

    /* Padding to max size */
    uint8_t  reserved[256];
} DM2_V1_SessionState;

typedef enum DM2_V1_SaveCandidateKind {
    DM2_V1_SAVE_CANDIDATE_FIRESTAFF_SESSION = 0,
    DM2_V1_SAVE_CANDIDATE_ORIGINAL_ENVELOPE,
    DM2_V1_SAVE_CANDIDATE_ORIGINAL_RAW
} DM2_V1_SaveCandidateKind;

/* Read-only receipt for the uncompressed dungeon prefix of an original raw
 * SKSave body. skproject c_savegame.cpp::DM2_READ_DUNGEON_STRUCTURE reads
 * these spans before SUPPRESS/GAME_LOAD state. This does not interpret DB
 * records or record links; it preserves only source-owned boundaries and
 * byte identities for an admitted original save. */
#define DM2_RAW_SKSAVE_DB_POOL_COUNT 16
#define DM2_RAW_SKSAVE_MAX_MAPS 64
typedef struct {
    int valid;
    uint8_t map_count;
    uint16_t map_data_byte_count;
    uint16_t column_index_count;
    uint16_t ground_stack_count;
    uint16_t text_word_count;
    /* Source c_map geometry, retained per map instead of being discarded
     * after the prefix hash is calculated. Offsets are relative to the
     * authenticated map-data span. */
    uint8_t map_widths[DM2_RAW_SKSAVE_MAX_MAPS];
    uint8_t map_heights[DM2_RAW_SKSAVE_MAX_MAPS];
    uint16_t map_data_relative_offsets[DM2_RAW_SKSAVE_MAX_MAPS];
    uint32_t map_data_base;
    uint32_t map_data_raw_hashes[DM2_RAW_SKSAVE_MAX_MAPS];
    uint16_t db_record_counts[DM2_RAW_SKSAVE_DB_POOL_COUNT];
    size_t db_pool_offsets[DM2_RAW_SKSAVE_DB_POOL_COUNT];
    size_t map_data_offset;
    uint32_t descriptor_hash;
    uint32_t column_index_hash;
    uint32_t ground_stack_hash;
    uint32_t text_hash;
    uint32_t db_pool_hashes[DM2_RAW_SKSAVE_DB_POOL_COUNT];
    uint32_t map_data_hash;
    uint32_t prefix_hash;
    size_t suppress_state_offset;
} DM2_V1_OriginalRawDungeonReceipt;

typedef struct {
    int valid;
    uint8_t map;
    uint8_t width;
    uint8_t height;
    uint32_t raw_offset;
    uint32_t byte_count;
    uint32_t raw_hash;
} DM2_V1_OriginalRawMapReceipt;

/* Read-only receipt for the source-owned fixed SUPPRESS sections directly
 * after an original raw SKSave dungeon structure. SKProject
 * sksvgame.cpp::DM2_GAME_LOAD reads these fields before
 * DM2_READ_SKSAVE_DUNGEON. This receipt deliberately stops at that exact
 * shared-bitstream boundary: it is evidence for an original body, not a
 * substitute for restoring the later record links, objects, or session. */
typedef struct {
    int valid;
    DM2_V1_OriginalRawDungeonReceipt dungeon;
    uint32_t game_tick;
    uint32_t random_seed;
    uint16_t champion_count;
    uint16_t party_x;
    uint16_t party_y;
    uint16_t party_direction;
    uint16_t party_map;
    uint16_t leader_index;
    uint16_t timer_count;
    uint32_t v1e0104_hash;
    uint32_t globalb_hash;
    uint32_t globalw_hash;
    /* c_savegame.cpp reads each 263-byte c_hero through the same SUPPRESS
     * stream before c_wbbb/timers. Preserve per-hero source identities so a
     * later record-link owner cannot replace an individual hero with a
     * fixture while retaining only a matching aggregate hash. */
    uint32_t hero_hashes[4];
    uint32_t heroes_hash;
    /* Exact 60-byte s_savegamebuffer identity. SKProject's
     * sksvgame.cpp:47/1415 source buffer and DM2_GAME_LOAD own these bytes;
     * this is deliberately not a Firestaff session-field projection. */
    uint32_t savegame_buffer_hash;
    /* Exact c_wbbb/ddat.savegames1 source section; provenance only. */
    uint32_t save_state_hash;
    uint32_t timers_hash;
    uint32_t fixed_sections_hash;
    /* The timer records and the following READ_SKSAVE_DUNGEON links share
     * one MSB-first SUPPRESS reader.  Keep both boundaries so corpus code
     * never reconstructs them with a different (legacy) state layout. */
    size_t timer_bitstream_offset;
    uint8_t timer_bitstream_bits_remaining;
    uint8_t timer_bitstream_current_byte;
    size_t record_link_bitstream_offset;
    uint8_t record_link_bitstream_bits_remaining;
} DM2_V1_OriginalRawSaveStateReceipt;

/* Source-shaped c_tim record receipt.  The v5 save path allocates 12-byte
 * c_tim entries and reads them through the shared SUPPRESS stream; the older
 * 10-byte DM2_TimerEntry convenience type is not an original SKSAVE record. */
#define DM2_V1_ORIGINAL_RAW_TIMER_RECORD_SIZE 12u
typedef struct {
    int valid;
    uint16_t timer_count;
    size_t start_offset;
    size_t end_offset;
    uint8_t start_bits_remaining;
    uint8_t end_bits_remaining;
    uint32_t raw_hash;
} DM2_V1_OriginalRawTimerStreamReceipt;

/* A decoded save candidate. dungeon_bytes aliases the caller-owned input and
 * is populated only for an original raw SKSave body. Its receipt is the
 * source-owned byte-layout proof that runtime must match before publishing
 * the reconstructed dungeon. */
typedef struct DM2_V1_SaveCandidate {
    DM2_V1_SaveCandidateKind kind;
    DM2_V1_SessionState session;
    const uint8_t *dungeon_bytes;
    size_t dungeon_size;
    DM2_V1_OriginalRawDungeonReceipt dungeon_receipt;
} DM2_V1_SaveCandidate;

/* One source-addressed raw c_record from an admitted SKSave DB pool. The
 * receipt intentionally exposes no decoded fields or record links. */
typedef struct {
    int valid;
    uint8_t db_pool;
    uint16_t record_index;
    uint16_t record_size;
    size_t record_offset;
    uint32_t pool_hash;
    uint32_t record_hash;
} DM2_V1_OriginalRawDbRecordReceipt;

typedef struct {
    int valid;
    DM2_V1_OriginalRawDbRecordReceipt record;
    uint16_t attributes;
    uint8_t button;
    uint8_t door_type;
    uint8_t button_state;
    uint8_t opening_dir;
    uint8_t ornate_index;
    uint8_t destroyable_by_fireball;
    uint8_t bashable_by_chopping;
} DM2_V1_OriginalRawDoorReceipt;

/* Source-limited SKWIN/DME.h::Actuator w2/w4/w6 decode from DB3 only. */
typedef struct {
    int valid;
    DM2_V1_OriginalRawDbRecordReceipt record;
    uint8_t actuator_type;
    uint16_t actuator_data;
    uint8_t graphic_number;
    uint8_t disabled;
    uint8_t delay;
    uint8_t sound_effect;
    uint8_t revert_effect;
    uint8_t action_type;
    uint8_t once_only;
    uint8_t active_status;
    uint8_t target_direction;
    uint8_t target_x;
    uint8_t target_y;
} DM2_V1_OriginalRawActuatorReceipt;

typedef struct {
    int valid;
    DM2_V1_OriginalRawDbRecordReceipt record;
    uint8_t creature_type;
    uint16_t hp1;
} DM2_V1_OriginalRawCreatureReceipt;

/* DME.h::Text owns these fields in DB2 w2. GenericRecord::w0 and the text
 * table stay outside the raw-save receipt until their links are proven. */
typedef struct {
    int valid;
    DM2_V1_OriginalRawDbRecordReceipt record;
    uint8_t visible;
    uint8_t mode;
    uint16_t text_index;
} DM2_V1_OriginalRawTextReceipt;

/* DME.h::Teleporter owns the destination/scope fields in DB1 w2/w4. The
 * GenericRecord::w0 next-link remains outside this raw-save receipt. */
typedef struct {
    int valid;
    DM2_V1_OriginalRawDbRecordReceipt record;
    uint8_t destination_x;
    uint8_t destination_y;
    uint8_t destination_map;
    uint8_t scope;
    uint8_t sound;
    uint8_t rotation;
    uint8_t rotation_type;
} DM2_V1_OriginalRawTeleporterReceipt;

/* DME.h::Container owns the open/type bits in DB9 b4. Its GenericRecord::w0
 * and contained-object w2 are deliberately excluded. */
typedef struct {
    int valid;
    DM2_V1_OriginalRawDbRecordReceipt record;
    uint8_t opened;
    uint8_t container_type;
} DM2_V1_OriginalRawContainerReceipt;

typedef struct {
    int valid;
    DM2_V1_OriginalRawDbRecordReceipt record;
    uint8_t item_type;
    uint8_t important;
    uint8_t charges;
} DM2_V1_OriginalRawWeaponReceipt;

/* SKWIN/DME.h gives Weapon, Cloth, Scroll, and Miscellaneous_item the same
 * ItemType() owner: bits 0..6 of their four-byte record's w2.  This keeps
 * the common raw-save fact separate from Weapon-only charges/important bits. */
typedef struct {
    int valid;
    DM2_V1_OriginalRawDbRecordReceipt record;
    uint8_t item_type;
} DM2_V1_OriginalRawItemReceipt;

/* ════════════════════════════════════════════════════════════════
 * New game API
 * ════════════════════════════════════════════════════════════════ */

/* ════════════════════════════════════════════════════════════════
 * Session management API
 * ════════════════════════════════════════════════════════════════ */

/* Serialize session state to a flat byte buffer.
 * Returns bytes written, or -1 on error (buffer too small).
 * Format: see DM2_V1_SessionState layout above.
 * Source: docs/dm2_save_format.md — session serialization */
int dm2_v1_session_serialize(const DM2_V1_SessionState *session,
                               uint8_t *buf, size_t buf_size);

/* Deserialize a byte buffer into a session state.
 * Returns 0 on success, -1 on error (invalid data).
 * Source: docs/dm2_save_format.md — session deserialization */
int dm2_v1_session_deserialize(DM2_V1_SessionState *session,
                                 const uint8_t *buf, size_t buf_size);

/* Import a bounded original/SUPPRESS resume payload into the Firestaff DM2
 * session envelope. This is the startup bridge for SKSave payloads that carry
 * the documented game-state block plus champion SUPPRESS records instead of
 * Firestaff's compact session-version byte at offset 28. */
int dm2_v1_session_import_original_payload(DM2_V1_SessionState *session,
                                           const uint8_t *buf,
                                           size_t buf_size);

/* Validate a raw original-style SKSave body after the 42-byte slot header has
 * been stripped for the resume boundary. It deliberately rejects publication
 * until the complete SKProject DM2_GAME_LOAD order (raw dungeon plus the
 * continuous SUPPRESS and READ_SKSAVE_DUNGEON stream) has a live owner. */
int dm2_v1_session_import_raw_sksave_payload(DM2_V1_SessionState *session,
                                             const uint8_t *buf,
                                             size_t buf_size);

/* Validate the exact raw-SKSave dungeon prefix and expose only its
 * skproject-owned section boundaries/hashes. Unknown or truncated layouts
 * fail closed and leave *out_receipt zeroed. */
int dm2_v1_original_raw_sksave_dungeon_receipt(
    const uint8_t *buf,
    size_t buf_size,
    DM2_V1_OriginalRawDungeonReceipt *out_receipt);

/* Expose one authenticated saved-map tile span. The returned hash covers
 * exactly the source byte-square span; no square type or object link is
 * inferred here. */
int dm2_v1_original_raw_sksave_map_receipt(
    const uint8_t *buf,
    size_t buf_size,
    const DM2_V1_OriginalRawDungeonReceipt *dungeon,
    int map,
    DM2_V1_OriginalRawMapReceipt *out_receipt);

/* Resolve the resident c_map record root for one saved tile without creating
 * a parallel map representation.  SKProject skmap.cpp::DM2_GET_OBJECT_INDEX_FROM_TILE
 * maps each tile with bit 0x10 through v1e03d8 (the column-index span) into
 * dm2_v1e038c (the saved ground-stack link array).  An unmarked tile returns
 * OBJECT_END_MARKER.  This is the owner lookup used before
 * READ_SKSAVE_DUNGEON decides whether a chain is resident or dynamic. */
int dm2_v1_original_raw_sksave_tile_record_link(
    const uint8_t *buf,
    size_t buf_size,
    const DM2_V1_OriginalRawDungeonReceipt *dungeon,
    int map,
    int x,
    int y,
    uint16_t *out_link);

/* Decode exactly s_savegamebuffer, v1e0104, globalb, globalw, c_hero,
 * c_wbbb and c_tim from the one shared SUPPRESS stream. Source: SKProject
 * SKWINSPX/src/v5/sksvgame.cpp::DM2_GAME_LOAD lines 1482-1525. No
 * player-facing importer may treat this as complete until the following
 * DM2_READ_SKSAVE_DUNGEON and possession-index stream has a live owner. */
int dm2_v1_original_raw_sksave_fixed_state_receipt(
    const uint8_t *buf,
    size_t buf_size,
    DM2_V1_OriginalRawSaveStateReceipt *out_receipt);

/* Decode only the authenticated c_tim section from the shared raw stream.
 * This preserves source bytes for a future GAME_LOAD owner and never mutates
 * a runtime session.  `out_records` is a caller-owned array of 12-byte
 * records with `record_capacity` entries. */
int dm2_v1_original_raw_sksave_decode_timer_stream(
    const uint8_t *buf,
    size_t buf_size,
    const DM2_V1_OriginalRawSaveStateReceipt *state_receipt,
    uint8_t *out_records,
    size_t record_capacity,
    DM2_V1_OriginalRawTimerStreamReceipt *out_receipt);
int dm2_v1_original_raw_sksave_db_record_receipt(
    const uint8_t *buf,
    size_t buf_size,
    int db_pool,
    int record_index,
    DM2_V1_OriginalRawDbRecordReceipt *out_receipt);
int dm2_v1_original_raw_sksave_door_receipt(
    const uint8_t *buf,
    size_t buf_size,
    int record_index,
    DM2_V1_OriginalRawDoorReceipt *out_receipt);
int dm2_v1_original_raw_sksave_actuator_receipt(
    const uint8_t *buf,
    size_t buf_size,
    int record_index,
    DM2_V1_OriginalRawActuatorReceipt *out_receipt);
int dm2_v1_original_raw_sksave_creature_receipt(
    const uint8_t *buf,
    size_t buf_size,
    int record_index,
    DM2_V1_OriginalRawCreatureReceipt *out_receipt);
int dm2_v1_original_raw_sksave_text_receipt(
    const uint8_t *buf,
    size_t buf_size,
    int record_index,
    DM2_V1_OriginalRawTextReceipt *out_receipt);
int dm2_v1_original_raw_sksave_teleporter_receipt(
    const uint8_t *buf,
    size_t buf_size,
    int record_index,
    DM2_V1_OriginalRawTeleporterReceipt *out_receipt);
int dm2_v1_original_raw_sksave_container_receipt(
    const uint8_t *buf,
    size_t buf_size,
    int record_index,
    DM2_V1_OriginalRawContainerReceipt *out_receipt);
int dm2_v1_original_raw_sksave_weapon_receipt(
    const uint8_t *buf,
    size_t buf_size,
    int record_index,
    DM2_V1_OriginalRawWeaponReceipt *out_receipt);
/* Read only ItemType() from an original raw DB5 Weapon, DB6 Cloth, DB7
 * Scroll, or DB10 Miscellaneous_item record.  GenericRecord::w0, inventory
 * links, charges, and all other fields deliberately remain unowned here. */
int dm2_v1_original_raw_sksave_item_receipt(
    const uint8_t *buf,
    size_t buf_size,
    int db_pool,
    int record_index,
    DM2_V1_OriginalRawItemReceipt *out_receipt);

/* Parse one payload after the 42-byte SKSave slot header. The function never
 * changes live runtime state; callers must apply the returned candidate only
 * after validating the active dungeon/profile boundary. */
int dm2_v1_session_parse_save_candidate(DM2_V1_SaveCandidate *out_candidate,
                                         const uint8_t *buf,
                                         size_t buf_size);

/* A Firestaff session is not an original DM2 save graph.  Until
 * SKProject's DM2_GAME_SAVE/DM2_SUPPRESS_WRITER path is implemented, these
 * public compatibility entry points must not create SKSave files. */
#define DM2_V1_SESSION_WRITE_ORIGINAL_WRITER_REQUIRED (-5)

/* Refuses with DM2_V1_SESSION_WRITE_ORIGINAL_WRITER_REQUIRED. */
int dm2_v1_session_save_slot(const char *save_base, uint8_t slot,
                               const char *name,
                               const DM2_V1_SessionState *session);

/* Refuses with DM2_V1_SESSION_WRITE_ORIGINAL_WRITER_REQUIRED. */
int dm2_v1_session_save_last_session(const char *save_base,
                                      const char *name,
                                      const DM2_V1_SessionState *session);

/* Player-facing load routes remain fail-closed until the complete original
 * DM2_GAME_LOAD graph is implemented.  They may inspect a hash-validated
 * slot but never publish its partial raw prefix as DM2_V1_SessionState.
 * Source: SKProject sksvgame.cpp::DM2_GAME_LOAD. */
int dm2_v1_session_load_slot(const char *save_base, uint8_t slot,
                               DM2_V1_SessionState *session);

/* Same fail-closed rule for SKSave.dat/SKSave.bak. */
int dm2_v1_session_load_last_session(const char *save_base,
                                      DM2_V1_SessionState *session);

/* Delete a saved session in slot N.
 * Returns 0 on success. */
int dm2_v1_session_delete_slot(const char *save_base, uint8_t slot);

/* Verify session state is internally consistent.
 * Checks: champion_count ≤ 4, leader_index < champion_count,
 * champion records have non-zero names, game_tick ≥ 0.
 * Returns true if valid. */
bool dm2_v1_session_validate(const DM2_V1_SessionState *session);

/* ════════════════════════════════════════════════════════════════
 * New-game and load admission
 * ════════════════════════════════════════════════════════════════ */

/* Result of a new game or load game operation */
typedef enum {
    DM2_FLOW_OK             = 0,
    DM2_FLOW_NO_ASSETS      = -1,
    DM2_FLOW_NO_DUNGEON     = -2,
    DM2_FLOW_BAD_SESSION    = -3,
    DM2_FLOW_SLOT_ERROR     = -4,
    DM2_FLOW_ALLOC_ERROR    = -5,
    DM2_FLOW_GAME_LOAD_REQUIRED = -6,
} DM2_FlowResult;

/* Request a new DM2 game from the boot profile.
 * This stops at the original GAME_LOAD boundary:
 *   1. Scan and verify DM2 assets (uses boot profile's scan)
 *   2. Load dungeon data
 *   3. Require GAME_LOAD/LOAD_NEW_DUNGEON to provide original party records
 *      and initial position; no fixture session is constructed.
 *
 * Source: SKWINSPX SkWinCore.cpp::SHOW_MENU_SCREEN / GAME_LOAD
 */
DM2_FlowResult dm2_v1_new_game_flow(DM2_V1_SessionState *session,
                                      const DM2_V1_BootProfile *boot);

/* Request a saved game from slot N. This remains at the same GAME_LOAD
 * boundary as a new game until one owner has restored the original map,
 * record pools, possessions, heroes and timers. The function never changes
 * session on rejection.
 *
 * Source: SKProject SKULLWIN/c_savegame.cpp::DM2_GAME_LOAD */
DM2_FlowResult dm2_v1_load_game_flow(DM2_V1_SessionState *session,
                                      const DM2_V1_BootProfile *boot,
                                      uint8_t slot);

/* ════════════════════════════════════════════════════════════════
 * Source evidence
 * ════════════════════════════════════════════════════════════════ */

const char *dm2_v1_new_game_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_NEW_GAME_H */
