/*
 * dm2_v1_new_game.h — DM2 V1 New Game & Session Management API
 *
 * Phase 6: Utility/import flow — DM2-specific load/start flow.
 *
 * Implements:
 *   1. Starter party generation — creates 4 initial champions at game start
 *   2. New game flow — full boot→game→dungeon→party pipeline
 *   3. Session save/load — save/load round-trip with slot manager
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
typedef struct {
    int valid;
    uint8_t map_count;
    uint16_t map_data_byte_count;
    uint16_t column_index_count;
    uint16_t ground_stack_count;
    uint16_t text_word_count;
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

/* Load session from slot N using the slot manager.
 * Combines dm2_sl_load + deserialize.
 * Returns 0 on success.
 * Source: dm2_v1_save_load.h dm2_sl_load() */
int dm2_v1_session_load_slot(const char *save_base, uint8_t slot,
                               DM2_V1_SessionState *session);

/* Load original-style last-session SKSave.dat, falling back to SKSave.bak. */
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
 * New game flow — full boot→game pipeline
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

/* Load a saved game from slot N.
 * Deserializes the session state from the save slot.
 * Caller is responsible for loading dungeon data separately
 * (dungeon is shared across saves and loaded once at boot).
 *
 * Source: dm2_v1_save_load.h dm2_sl_load()
 *         docs/dm2_save_format.md — save slot layout */
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
