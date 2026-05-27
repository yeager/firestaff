#ifndef THERON_V1_SAVE_LOAD_H
#define THERON_V1_SAVE_LOAD_H

#include <stdint.h>
#include <stddef.h>

/* ══════════════════════════════════════════════════════════════════════
 * Theron V1 Phase 6 — Between-Dungeon Save/Load
 *
 * Theron's Quest has a strict save restriction: no in-dungeon saves.
 * Saves are only permitted at dungeon entrances (between-dungeon).
 * This is a design constraint from the original PC Engine game.
 *
 * Save format (saves/theron/slotN.tqsv):
 *   Header: 64 bytes
 *   Champion state: variable (same as DM1 champion block)
 *   Dungeon progression: ~32 bytes
 *   Footer: 4 bytes checksum
 *
 * Source: THQUEST.ASM T080 — between-dungeon save/load
 *         THQUEST.ASM T800 — champion persistence
 * ══════════════════════════════════════════════════════════════════════ */

/* ── Save slot constants ─────────────────────────────────────────── */

#define THERON_SAVE_SLOT_COUNT   8   /* 8 between-dungeon save slots */
#define THERON_SAVE_MAGIC        0x54515220U  /* 'TQR ' in ASCII */
#define THERON_SAVE_VERSION      1   /* Format version */
#define THERON_SAVE_HEADER_SIZE  64
#define THERON_SAVE_FOOTER_SIZE   4

/* Obfuscation seed for between-dungeon saves.
 * TQR uses a simple XOR obfuscation — different from CSB's CRC approach.
 * Source: THQUEST.ASM T080 save routine. */
#define THERON_SAVE_OBFUSCATE_SEED  0x5A

/* ── Save slot descriptor (in-memory) ─────────────────────────── */

typedef struct {
    int         valid;        /* 1 = slot has a save, 0 = empty */
    int         slot_index;   /* 0..THERON_SAVE_SLOT_COUNT-1 */
    char        label[32];    /* User label, e.g. "After Dungeon 2" */
    uint32_t    timestamp;    /* Unix timestamp of save */
    uint8_t     quest_items;  /* Quest items collected (7-bit bitmap) */
    uint8_t     current_dungeon;
    uint8_t     dungeon_state; /* Current dungeon state */
    uint32_t    playtime_secs; /* Total playtime in seconds */
    size_t      size_bytes;   /* Total save file size */
} Theron_SaveSlot;

/* ── Between-dungeon save header ────────────────────────────────── */

/* Layout of the 64-byte header (THQUEST.ASM T080):
 *   Offset  Size  Field
 *   0x00    4     magic  = 0x54515220 ('TQR ')
 *   0x04    2     version = 1
 *   0x06    2     checksum (16-bit sum of all data words)
 *   0x08    1     quest_items_collected (7-bit bitmap)
 *   0x09    1     current_dungeon_id
 *   0x0A    1     current_dungeon_state
 *   0x0B    1     current_level (1..3)
 *   0x0C    4     dungeon_seeds[7] (packed: 4 bits per seed * 7 = 28 bits)
 *   0x10    4     dungeon_states[7] (packed: 2 bits per state * 7 = 14 bits)
 *   0x14    4     champion_gold (32-bit gold total for party)
 *   0x18    4     playtime_seconds
 *   0x1C    4     timestamp (Unix epoch)
 *   0x20    32    label (null-terminated, max 31 chars)
 *   0x40    36    reserved for future use
 *   Total: 64 bytes
 *
 * Champion state (variable after header):
 *   4 champion slots × champion_block_size (same layout as DM1 v1).
 *   No in-dungeon save = no creature/object state to serialize.
 *
 * Footer: 4-byte checksum of entire file (same algorithm as header checksum).
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

/* Save current game state to slot index (0..7).
 * Returns 0 on success, -1 on error.
 * Will overwrite existing save in that slot. */
int theron_v1_save_to_slot(const char *save_root,
                           int slot_index,
                           const void *champion_data,   /* 4 × champion blocks */
                           size_t champion_data_size,
                           const void *dungeon_progression, /* Theron_DungeonProgression */
                           const char *label);

/* Load game state from slot index (0..7).
 * Populates champion_data and dungeon_progression from the save.
 * Returns 0 on success, -1 if slot empty/corrupt, -2 if slot invalid. */
int theron_v1_save_load_from_slot(const char *save_root,
                                   int slot_index,
                                   void *champion_data,
                                   size_t champion_data_size,
                                   void *dungeon_progression,
                                   size_t dungeon_progression_size,
                                   Theron_SaveSlot *out_slot_info);

/* Delete a save slot. Returns 0 on success, -1 on error. */
int theron_v1_save_delete_slot(const char *save_root, int slot_index);

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