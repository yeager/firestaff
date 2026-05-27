#ifndef NEXUS_V1_SAVE_H
#define NEXUS_V1_SAVE_H

#include <stddef.h>
#include <stdint.h>

/*
 * Nexus V1 Phase 6 — Save/Load and Champion/World Persistence
 * ==========================================================
 *
 * Supports: Firestaff native save format (NEXUS_SAVE_MAGIC = 'FNXS').
 * Format is portable across platforms (little-endian).
 *
 * Unknown/unsupported variants: explicit diagnostics via nexus_v1_save_probe()
 * which returns a reason string for any file that fails magic/version check.
 *
 * Source-lock reference:
 *   ReDMCSB LOADSAVE.C: F0433/F0434 (DM1 save/load structure)
 *   ReDMCSB SAVEHEAD.C: F0429/F0430 (save header obfuscation/checksum)
 *   DM Nexus (Saturn): memory card format (8 KB blocks, proprietary header)
 *
 * The original Saturn save format is not reverse-engineered — this is a
 * Firestaff-native format derived from the state structures documented in
 * docs/nexus_save_state.md and the world model in nexus_v1_world.h.
 * Champion save/load uses the same slot-based design as DM1/CSB.
 */

/* ── Magic and version ────────────────────────────────────────────── */

#define NEXUS_SAVE_MAGIC   0x53584E46U     /* 'FNXS' = 'F' + ('N'<<8) + ('X'<<16) + ('S'<<24) */
#define NEXUS_SAVE_VERSION 2                    /* v2: adds champion_data_size + world_data_size */
#define NEXUS_SAVE_MAX_SLOTS 8

/* ── Save header — stored at the start of every save file ──────────── */

typedef struct {
    uint32_t magic;               /* NEXUS_SAVE_MAGIC ('FNXS') */
    uint16_t version;             /* NEXUS_SAVE_VERSION (2) */
    uint16_t header_size;         /* sizeof(this struct) */
    uint32_t data_size;           /* total bytes of champion_data + world_data sections */
    uint32_t champion_data_size;  /* bytes of champion data section (v2+) */
    uint32_t world_data_size;     /* bytes of world data section (v2+) */
    uint32_t game_time;           /* accumulated tick count */
    uint32_t crc32;               /* CRC-32 of data section (0 if not used) */
    int32_t  current_level;       /* dungeon level 0-15 */
    int32_t  party_x;             /* grid X position */
    int32_t  party_y;             /* grid Y position */
    int32_t  party_dir;           /* 0=N 1=E 2=S 3=W */
    uint32_t state_hash;          /* world-state hash at save time (low 32 bits) */
    char description[32];          /* human-readable description */
} Nexus_V1_SaveHeader;

/* ── Save slot metadata (used for slot browsing) ───────────────────── */

typedef struct {
    int      occupied;            /* true if slot has a save */
    int      slot_index;         /* 0-7 */
    Nexus_V1_SaveHeader header;   /* copy of header for display */
    char     label[64];           /* derived from description + level */
    uint32_t timestamp;          /* file mtime at scan time */
} Nexus_V1_SaveSlot;

/* ── Save/load result codes ────────────────────────────────────────── */

typedef enum {
    NEXUS_SAVE_OK = 0,
    NEXUS_SAVE_ERR_NULL = -1,
    NEXUS_SAVE_ERR_OPEN = -2,
    NEXUS_SAVE_ERR_MAGIC = -3,
    NEXUS_SAVE_ERR_VERSION = -4,
    NEXUS_SAVE_ERR_CRC = -5,
    NEXUS_SAVE_ERR_READ = -6,
    NEXUS_SAVE_ERR_WRITE = -7,
    NEXUS_SAVE_ERR_DATA_DIR = -8,
    NEXUS_SAVE_ERR_SLOT_RANGE = -9,
    NEXUS_SAVE_ERR_UNKNOWN_VARIANT = -10
} Nexus_SaveResult;

/* ── Diagnostics ───────────────────────────────────────────────────── */

/* Return a static human-readable string for any save result code.
 * Never returns NULL — unknown codes map to "UNKNOWN_ERROR". */
const char *nexus_v1_save_strerror(Nexus_SaveResult r);

/* Probe a file path and return a reason string for why it can't be loaded.
 * Returns the reason string (never NULL, never empty) in all cases,
 * including when the file doesn't exist or is not a save file.
 * out_header, if non-NULL, is filled with the parsed header on success.
 *
 * Use this for explicit diagnostics on unknown/unsupported variants:
 *   const char *reason = nexus_v1_save_probe(path, &hdr, &sz);
 *   if (reason[0] != '\0') printf("Cannot load: %s\n", reason);
 */
const char *nexus_v1_save_probe(const char *path,
                                 Nexus_V1_SaveHeader *out_header,
                                 size_t *out_file_size);

/* ── Slot management ────────────────────────────────────────────────── */

typedef struct {
    char save_dir[512];
    Nexus_V1_SaveSlot slots[NEXUS_SAVE_MAX_SLOTS];
    uint8_t slot_count;
    int initialized;
} Nexus_V1_SaveManager;

/* Init save manager with a save directory path.
 * The directory is created if it doesn't exist. */
void nexus_v1_save_init(Nexus_V1_SaveManager *mgr, const char *save_dir);

/* Scan all save slots in the directory.
 * Updates slot[] metadata and slot_count. */
int nexus_v1_save_scan(Nexus_V1_SaveManager *mgr);

/* Get a slot by index (0-7). Returns NULL if out of range or empty. */
const Nexus_V1_SaveSlot *nexus_v1_save_get_slot(const Nexus_V1_SaveManager *mgr,
                                                 uint8_t slot);

/* Get the default save directory path (platform-specific). */
void nexus_v1_save_default_dir(char *buf, size_t bufsz);

/* ── Full save ─────────────────────────────────────────────────────── */

/* Save the entire engine+world+champion state to a slot.
 * The slot file is written atomically (temp file + rename).
 *
 * Components saved:
 *   - Header (magic, version, party position, level, game_time)
 *   - Game state (game_started, foot_step)
 *   - Champion pool (24 champions, party slots, leader)
 *   - World (objects, events, timers, world_tick, state_hash)
 *
 * Returns NEXUS_SAVE_OK on success, else error code.
 */
Nexus_SaveResult nexus_v1_save(Nexus_V1_SaveManager *mgr,
                                uint8_t slot,
                                int32_t current_level,
                                int32_t party_x,
                                int32_t party_y,
                                int32_t party_dir,
                                uint32_t game_time,
                                uint64_t state_hash,
                                const void *champion_data,
                                size_t champion_data_size,
                                const void *world_data,
                                size_t world_data_size);

/* Load state from a slot.
 * On success, fills the provided out buffers with champion_data and world_data.
 * Caller provides the buffers (allocate max sizes upfront).
 *
 * Returns NEXUS_SAVE_OK on success, else error code.
 * On NEXUS_SAVE_ERR_UNKNOWN_VARIANT, the probe string is stored in
 * the optional out_diagnostic buffer (if provided, max 256 bytes).
 */
Nexus_SaveResult nexus_v1_load(Nexus_V1_SaveManager *mgr,
                                uint8_t slot,
                                Nexus_V1_SaveHeader *out_header,
                                void *champion_data, size_t champion_buf_size,
                                size_t *out_champion_data_size,
                                void *world_data, size_t world_buf_size,
                                size_t *out_world_data_size,
                                char *out_diagnostic,
                                size_t diag_size);

/* Delete a save slot. Returns NEXUS_SAVE_OK on success. */
Nexus_SaveResult nexus_v1_save_delete(Nexus_V1_SaveManager *mgr, uint8_t slot);

/* ── Slot-free save/load (single-file convenience) ─────────────────── */

/* Save to a specific file path (no slot management).
 * Convenience wrapper around nexus_v1_save() for saves with a fixed path. */
Nexus_SaveResult nexus_v1_save_to_path(const char *path,
                                        int32_t current_level,
                                        int32_t party_x,
                                        int32_t party_y,
                                        int32_t party_dir,
                                        uint32_t game_time,
                                        uint64_t state_hash,
                                        const void *champion_data,
                                        size_t champion_data_size,
                                        const void *world_data,
                                        size_t world_data_size);

/* Load from a specific file path (no slot management).
 * Returns NEXUS_SAVE_OK on success, or writes diagnostic to out_diagnostic
 * (up to diag_size bytes) for any failure including unknown variant. */
Nexus_SaveResult nexus_v1_load_from_path(const char *path,
                                          Nexus_V1_SaveHeader *out_header,
                                          void *champion_data,
                                          size_t champion_buf_size,
                                          size_t *out_champion_data_size,
                                          void *world_data,
                                          size_t world_buf_size,
                                          size_t *out_world_data_size,
                                          char *out_diagnostic,
                                          size_t diag_size);

/* ── High-level save/load (automatic serialization) ─────────────────── */

/* High-level save: serialize and write to a slot.
 * Automatically handles champion pool and world state serialization.
 * champion_pool and world are serialized using the functions declared in
 * nexus_v1_champions.h and nexus_v1_world.h respectively.
 *
 * Returns NEXUS_SAVE_OK on success, else error code. */
Nexus_SaveResult nexus_v1_save_full(Nexus_V1_SaveManager *mgr, uint8_t slot,
                                     int32_t current_level,
                                     int32_t party_x, int32_t party_y, int32_t party_dir,
                                     uint32_t game_time,
                                     uint64_t state_hash,
                                     const void *champion_pool,
                                     const void *world);

/* High-level load: read and deserialize from a slot.
 * Automatically handles champion pool and world state deserialization.
 * champion_pool and world must be allocated by caller.
 * Returns NEXUS_SAVE_OK on success, else error code.
 * On failure, diagnostic is stored in out_diagnostic (if provided, 256 bytes). */
Nexus_SaveResult nexus_v1_load_full(Nexus_V1_SaveManager *mgr, uint8_t slot,
                                      Nexus_V1_SaveHeader *out_header,
                                      void *champion_pool,
                                      void *world,
                                      char *out_diagnostic, size_t diag_size);

/* High-level save to a specific path (no slot management). */
Nexus_SaveResult nexus_v1_save_full_to_path(const char *path,
                                             int32_t current_level,
                                             int32_t party_x, int32_t party_y, int32_t party_dir,
                                             uint32_t game_time,
                                             uint64_t state_hash,
                                             const void *champion_pool,
                                             const void *world);

/* High-level load from a specific path (no slot management). */
Nexus_SaveResult nexus_v1_load_full_from_path(const char *path,
                                                Nexus_V1_SaveHeader *out_header,
                                                void *champion_pool,
                                                void *world,
                                                char *out_diagnostic, size_t diag_size);

#endif /* NEXUS_V1_SAVE_H */