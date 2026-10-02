#ifndef THERON_V1_SAVE_LOAD_H
#define THERON_V1_SAVE_LOAD_H

#include <stdint.h>
#include <stddef.h>

/* ══════════════════════════════════════════════════════════════════════
 * Theron V1 Phase 6 — Firestaff Host Save/Load
 *
 * The original PC Engine game saves between dungeons and has no in-dungeon
 * save transaction. This module's eight-slot TQSV format is Firestaff host
 * state, not an implementation of the retail Backup RAM or community SRM.
 *
 * Firestaff host/interchange format (not a retail save):
 *   Header: 64 bytes
 *   Champion state: variable (same as DM1 champion block)
 *   Dungeon progression: ~32 bytes
 *   Footer: 4 bytes checksum
 *
 * Retail evidence: docs/source-lock/theron-original-backup-ram-body-layout-2026-09-23.md
 * ══════════════════════════════════════════════════════════════════════ */

/* ── Save slot constants ─────────────────────────────────────────── */

#define THERON_SAVE_SLOT_COUNT   8   /* Firestaff host slots */
#define THERON_SAVE_MAGIC        0x54515220U  /* 'TQR ' in ASCII */
#define THERON_SAVE_VERSION      1   /* Format version */
#define THERON_SAVE_HEADER_SIZE  64
#define THERON_SAVE_FOOTER_SIZE   4

/* Firestaff host-format obfuscation seed. This is not an authenticated
 * retail PC Engine Backup RAM or community SRM encoding. */
#define THERON_SAVE_OBFUSCATE_SEED  0x5A

/* ── Save slot descriptor (in-memory) ─────────────────────────── */

typedef struct {
    int         valid;        /* 1 = slot has a save, 0 = empty */
    int         slot_index;   /* 0..THERON_SAVE_SLOT_COUNT-1 */
    char        label[32];    /* User label, e.g. "After Dungeon 2" */
    uint32_t    timestamp;    /* Unix timestamp of save */
    uint8_t     quest_items;  /* Host-format field; retail persistence is partial */
    uint8_t     current_dungeon;
    uint8_t     dungeon_state; /* Current dungeon state */
    uint32_t    party_gold;     /* Shared party gold from save header */
    uint32_t    playtime_secs; /* Total playtime in seconds */
    size_t      size_bytes;   /* Total save file size */
} Theron_SaveSlot;

/* ── Firestaff host-format header ───────────────────────────────── */

/* Layout of the Firestaff TQSV 64-byte host header (not retail):
 *   Offset  Size  Field
 *   0x00    4     magic  = 0x54515220 ('TQR ')
 *   0x04    2     version = 1
 *   0x06    2     checksum (16-bit sum of all data words)
 *   0x08    1     quest_items_collected (7-bit bitmap)
 *   0x09    1     current_dungeon_id
 *   0x0A    1     current_dungeon_state
 *   0x0B    1     current_level (1..3)
 *   0x0C    4     low-nibble dungeon seed summaries (7 × 4 bits)
 *   0x10    4     dungeon states (7 × 2 bits)
 *   0x14    4     champion_gold (32-bit gold total for party)
 *   0x18    4     playtime_seconds
 *   0x1C    4     timestamp (Unix epoch)
 *   0x20    32    label (null-terminated, max 31 chars)
 *   0x40    36    reserved for future use
 *   Total: 64 bytes
 *
 * Champion state (variable after header):
 *   4 champion slots × champion_block_size (same layout as DM1 v1).
 *   No in-dungeon Firestaff host save = no creature/object state in this
 *   container; the original game's separately authenticated save boundary
 *   does not make this TQSV format a retail format.
 *
 * Footer: 4 bytes: little-endian checksum plus 0x5A, 0xA5 marker.
 *
 * The compact header summaries are Firestaff metadata.  The full
 * Theron_DungeonProgression snapshot follows the champion stream. The
 * DMS-SG.001 System Card source lock documents the original save boundary and
 * bounded restore fields; TQSV remains a Firestaff host/interchange format.
 */

#define THERON_SAVE_CHAMPION_BLOCK_SIZE  128  /* per champion slot */
#define THERON_SAVE_CHAMPION_COUNT        4   /* Theron + 3 champions */

/* Offsets within the save header */
#define THERON_SAVE_OFF_MAGIC             0
#define THERON_SAVE_OFF_VERSION           4
#define THERON_SAVE_OFF_CHECKSUM           6
#define THERON_SAVE_OFF_QUEST_ITEMS        8
#define THERON_SAVE_OFF_CURRENT_DUNGEON    9
#define THERON_SAVE_OFF_DUNGEON_STATE     10
#define THERON_SAVE_OFF_CURRENT_LEVEL     11
#define THERON_SAVE_OFF_SEEDS             12
#define THERON_SAVE_OFF_DUNGEON_STATES    16
#define THERON_SAVE_OFF_CHAMPION_GOLD     20
#define THERON_SAVE_OFF_PLAYTIME          24
#define THERON_SAVE_OFF_TIMESTAMP         28
#define THERON_SAVE_OFF_LABEL             32

/* ── Save API ────────────────────────────────────────────────────── */

/* Enumerate available save slots in saves/theron/.
 * Populates slots[] with metadata for up to max_slots entries.
 * Returns number of valid slots found (0..THERON_SAVE_SLOT_COUNT). */
int theron_v1_save_enum_slots(const char *save_root,
                               Theron_SaveSlot *slots,
                               int max_slots);

/* Save current Firestaff host state to slot index (0..7).
 * Returns 0 on success, -1 on error.
 * Will overwrite existing host-format save in that slot. This TQR/.tqsv
 * container is not the original PC Engine Backup RAM or community SRM
 * format; original-format writes use theron_v1_pce_bram_encode_original_*
 * only with authenticated receipts. */
int theron_v1_save_to_slot(const char *save_root,
                           int slot_index,
                           const void *champion_data,   /* 4 × champion blocks */
                           size_t champion_data_size,
                           const void *dungeon_progression, /* Theron_DungeonProgression */
                           const char *label);

/* Save variant with the live party gold value supplied explicitly.  The
 * legacy API above remains available for callers that have no party context
 * and writes an honest zero/no-gold value. */
int theron_v1_save_to_slot_with_gold(const char *save_root,
                                     int slot_index,
                                     const void *champion_data,
                                     size_t champion_data_size,
                                     const void *dungeon_progression,
                                     uint32_t party_gold,
                                     const char *label);

/* Load Firestaff host state from slot index (0..7).
 * Populates champion_data and dungeon_progression from the save.
 * Returns 0 on success, -1 if slot empty/corrupt, -2 if slot invalid. */
int theron_v1_save_load_from_slot(const char *save_root,
                                   int slot_index,
                                   void *champion_data,
                                   size_t champion_data_size,
                                   void *dungeon_progression,
                                   size_t dungeon_progression_size,
                                   Theron_SaveSlot *out_slot_info);
int theron_v1_save_load_from_path(const char *save_path,
                                  void *champion_data,
                                  size_t champion_data_size,
                                  void *dungeon_progression,
                                  size_t dungeon_progression_size,
                                  Theron_SaveSlot *out_slot_info);

/* Delete a save slot. Returns 0 on success, -1 on error. */
int theron_v1_save_delete_slot(const char *save_root, int slot_index);

/* Export an existing valid save slot to an external file.
 * Returns 0 on success, -1 if the slot/path is invalid or corrupt. */
int theron_v1_save_export_slot(const char *save_root,
                               int slot_index,
                               const char *export_path);

/* Import a valid external save image into a slot.
 * Returns 0 on success, -1 if the input/path is invalid or corrupt.
 * out_slot_info may be NULL. */
int theron_v1_save_import_slot(const char *save_root,
                               int slot_index,
                               const char *import_path,
                               Theron_SaveSlot *out_slot_info);

/* Get the default saves root path for Theron. */
void theron_v1_save_default_root(char *buf, size_t buf_size);

/* Verify save slot integrity (magic + checksum). Returns 1 if valid, 0 if corrupt. */
int theron_v1_save_verify_slot(const char *save_root, int slot_index);

/* Build save file path for a given slot. */
void theron_v1_save_slot_path(const char *save_root,
                               int slot_index,
                               char *out_path,
                               size_t out_path_size);

/* Source evidence citation. */
const char *theron_v1_save_source_evidence(void);

#endif /* THERON_V1_SAVE_LOAD_H */
