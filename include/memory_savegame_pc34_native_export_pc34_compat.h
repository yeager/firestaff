#ifndef REDMCSB_MEMORY_SAVEGAME_PC34_NATIVE_EXPORT_PC34_COMPAT_H
#define REDMCSB_MEMORY_SAVEGAME_PC34_NATIVE_EXPORT_PC34_COMPAT_H

/*
 * memory_savegame_pc34_native_export_pc34_compat.h
 *
 * LSV-01 (audit, v2.7.x) — ReDMCSB DM 3.4 PC native save exporter
 * and importer.
 *
 * Bridges the Firestaff-native composite save blob (F0773/F0774,
 * MEDIA016 LE-encoded with a CRC32 trailer) to the original PC 3.4
 * save file format used by ReDMCSB's F0433 / F0435 save/load chain.
 *
 * The original PC 3.4 file layout (LOADSAVE.C F0433 + SAVEHEAD.C
 * F0429 / F0430 + READWRIT.C F0417 / F0418 / F0420):
 *
 *   File = SAVE_HEADER (512 bytes) + N PARTS
 *   SAVE_HEADER is two halves of 128 uint16_t words:
 *     - first  128 words: random "Noise" array, Noise[10] is the
 *       key (C10_DM_SAVE_HEADER_DECRYPTION_KEY_INDEX) used to
 *       obfuscate the second half
 *     - second 128 words: Useless/FormatID/aUnreferenced/...
 *       SaveAndPlayChoice/GameID/Keys[16]/Checksums[16]/Platform/
 *       DungeonID/AdditionalData[134], XOR-obfuscated with the key
 *     - the half-checksum is the simple sum of the obfuscated
 *       second-half words; the full header is the same 512 bytes
 *   Each PART is a length-prefixed (uint16_t, 0 = missing) CPSC-
 *   obfuscated byte block: writes F0420 which calls F0417 once to
 *   XOR each word with a per-part key and accumulate the
 *   word-by-word checksum, writes the bytes, then calls F0417
 *   again to deobfuscate the in-memory copy.
 *   Parts emitted in this order: 0 = GLOBAL_DATA, 1 = ACTIVE_GROUP,
 *   2 = PARTY (Champions), 3 = EVENTS, 4 = TIMELINE. The original
 *   allows up to 16 parts but only uses 5 in shipped DM1 PC 3.4.
 *
 * This module is INTENTIONALLY a one-way bridge with documented
 * gaps. It does NOT preserve:
 *   - ReDMCSB copy-protection (C2C, fuzzy bits) — irrelevant on
 *     modern hardware.
 *   - ReDMCSB-specific MAP / SQUARE-level mutations (Firestaff
 *     carries those in the DungeonMutationList section, which has
 *     no PC 3.4 counterpart).
 *   - Firestaff-specific V2 graphics / filter state.
 *
 * The two halves of the LSV-01 fix are:
 *   - F0795_SAVEGAME_ExportPC34_Compat  (exporter)
 *   - F0796_SAVEGAME_ImportPC34_Compat  (importer, best-effort)
 *
 * ReDMCSB anchors:
 *   - LOADSAVE.C F0433 + F0434 + F0435   (save / verify / load)
 *   - SAVEHEAD.C F0429 + F0430           (read / write obfuscated header)
 *   - READWRIT.C F0417 + F0418 + F0420   (CPSC checksum / obfuscate /
 *                                          write-obfuscated-part)
 *   - DEFS.H DM_SAVE_HEADER layout + C10_DM_SAVE_HEADER_DECRYPTION_KEY_INDEX
 */

#include <stddef.h>
#include <stdint.h>

#include "memory_savegame_pc34_compat.h"

/* -------- PC 3.4 native constants (DEFS.H mirror) -------- */

#define SAVEGAME_PC34_DM_SAVE_HEADER_SIZE       512
#define SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS 128
#define SAVEGAME_PC34_DM_NOISE_WORDS            149
#define SAVEGAME_PC34_DM_KEYS_COUNT             16
#define SAVEGAME_PC34_DM_CHECKSUMS_COUNT        16
#define SAVEGAME_PC34_DM_ADDITIONAL_DATA_SIZE   134
#define SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX 10

#define SAVEGAME_PC34_FORMAT_DUNGEON_MASTER_PC  0x05  /* C5 on PC 3.4 */
#define SAVEGAME_PC34_PLATFORM_PC               0x09
#define SAVEGAME_PC34_DUNGEON_ID_DM             0x0A  /* C10_DUNGEON_DM */

#define SAVEGAME_PC34_PART_GLOBAL_DATA          0
#define SAVEGAME_PC34_PART_ACTIVE_GROUP         1
#define SAVEGAME_PC34_PART_PARTY                2
#define SAVEGAME_PC34_PART_EVENTS               3
#define SAVEGAME_PC34_PART_TIMELINE             4
#define SAVEGAME_PC34_PART_COUNT                5

/* GLOBAL_DATA on PC 3.4 I34E is 128 bytes per ReDMCSB GLOBAL_DATA
 * struct (5 parts: GameTime, LastRandomNumber, LeaderHandObject
 * THING, etc.; the Useless[80] BUG0_00 padding pushes the field
 * set into 128 bytes for shipped platforms). We keep the byte
 * count fixed for stable round-trip; the importer tolerates other
 * sizes via the LENGTH prefix. */
#define SAVEGAME_PC34_GLOBAL_DATA_BYTE_COUNT    128
#define SAVEGAME_PC34_ACTIVE_GROUP_BYTE_COUNT   32    /* 4-byte header + aspect block */
#define SAVEGAME_PC34_EVENTS_BYTE_COUNT         128   /* small per-Part buffer */
#define SAVEGAME_PC34_TIMELINE_BYTE_COUNT       4096  /* generous cut for v1 */

/* Result codes for the exporter / importer. Match the F078x
 * error string table style; added values for native IO. */
#define SAVEGAME_PC34_OK                       0
#define SAVEGAME_PC34_ERROR_NULL_ARG          -1
#define SAVEGAME_PC34_ERROR_BUFFER_TOO_SMALL  -2
#define SAVEGAME_PC34_ERROR_BAD_MAGIC         -3
#define SAVEGAME_PC34_ERROR_BAD_VERSION       -4
#define SAVEGAME_PC34_ERROR_BAD_SIZE          -5
#define SAVEGAME_PC34_ERROR_BAD_CHECKSUM      -6
#define SAVEGAME_PC34_ERROR_UNSUPPORTED       -7
#define SAVEGAME_PC34_ERROR_INTERNAL          -8

/* Maximum native save file size (512 header + 5 parts + 4
 * LENGTH prefixes + slack). The original PC 3.4 file is well
 * under 32 KiB in shipped dungeons; 1 MiB matches
 * SAVEGAME_MAX_FILE_SIZE in the Firestaff-native path. */
#define SAVEGAME_PC34_MAX_FILE_SIZE  (1u << 20)

/* LSV-01 export / import API.
 *
 * Export:
 *   - `state` is the Firestaff composite save state. Required:
 *     state->header, state->party, state->timeline, state->magic.
 *     Optional: state->lastMovement, state->pendingSensorEffects,
 *     state->combatScratch, state->mutations.
 *   - `gameID` is the CPSC GameID; pass 0 to auto-pick from
 *     state->header.reserved.
 *   - `outBuf` is the destination buffer; `outBufSize` is the
 *     caller-allocated size. `outBytesWritten` returns the
 *     actual file size on success.
 *
 * Import:
 *   - `buf` / `bufSize` is a PC 3.4 native save file.
 *   - `outState` is the destination Firestaff composite. The
 *     caller is responsible for owning the subsystem structs
 *     (party / timeline / magic). The importer copies bytes
 *     in-place into the supplied buffers; if the destination
 *     struct is smaller than the PC 3.4 blob, the call fails
 *     with SAVEGAME_PC34_ERROR_BUFFER_TOO_SMALL.
 */
int F0795_SAVEGAME_ExportPC34_Compat(
    const struct SaveGame_Compat* state,
    uint32_t gameID,
    unsigned char* outBuf,
    int outBufSize,
    int* outBytesWritten);

int F0796_SAVEGAME_ImportPC34_Compat(
    const unsigned char* buf,
    int bufSize,
    struct SaveGame_Compat* outState,
    int strictChecksums);

/* Diagnostics. Returns the static English error string. */
const char* F0797_SAVEGAME_PC34ErrorToString_Compat(int code);

/* CPSC obfuscation primitive, exposed for tests + cross-checks.
 * `buf` is treated as `wordCount` little-endian uint16_t values.
 * The operation is its own inverse (F0417 reversible) and updates
 * the running checksum the same way F0417 does.
 *
 * Returns the 16-bit F0417 checksum after the obfuscation pass. */
uint16_t F0798_SAVEGAME_PC34CPSCObfuscate_Compat(
    uint16_t* buf,
    int wordCount,
    uint16_t key);

#endif /* REDMCSB_MEMORY_SAVEGAME_PC34_NATIVE_EXPORT_PC34_COMPAT_H */
