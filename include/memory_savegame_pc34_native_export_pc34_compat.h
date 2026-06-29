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

/* -------- LSV-02 versioned manifest gate (DM1) --------
 *
 * The PC 3.4 DM_SAVE_HEADER carries an AdditionalData[134] region
 * (meta[41..107] in the obfuscated second half) that the original
 * ReDMCSB engine never reads. We use the first 16 bytes of that
 * region as a Firestaff-only versioned manifest slot so that
 *   - the exported file is still loadable by vanilla DM 3.4 PC
 *     (the engine ignores AdditionalData; only FormatID/Platform/
 *     DungeonID gate loading)
 *   - the Firestaff importer can detect, version-check, and
 *     per-game gate the export at load time, so a wrong-game
 *     save (e.g. a CSB-native file fed into a DM1 runtime)
 *     is rejected before any GLOBAL_DATA is unpacked.
 *
 * The 16-byte manifest layout (LE, embedded in the obfuscated
 * header meta half, so a casual hex-dump of AdditionalData
 * yields the magic exactly as written):
 *
 *   off  size  field
 *    0    8    magic         ASCII "RDMCSB_LSV" (no NUL)
 *    8    2    formatVersion uint16 (currently 1)
 *   10    2    gameCode      uint16 (SAVEGAME_PC34_GAME_CODE_*)
 *   12    2    dungeonCode   uint16 (echoes meta[40] DungeonID)
 *   14    2    flags         uint16 (reserved; must be 0)
 *
 * Source-locked: per ReDMCSB DEFS.H:468-480 the DM_SAVE_HEADER
 * exposes AdditionalData[134] as opaque scratch bytes the
 * runtime never inspects. We treat it as our per-export tag.
 * Vanilla DM 3.4 PC loads the file as before; Firestaff's
 * importer uses it as the per-game compatibility gate.
 */
#define SAVEGAME_PC34_MANIFEST_SIZE            16
#define SAVEGAME_PC34_MANIFEST_OFFSET          41   /* meta[] index 41 == AdditionalData[0] */

/* Per-game codes (LSV-02). The DM1 value mirrors the
 * C10_DUNGEON_DM constant 0x0A split with the I34E
 * L1348_s_GlobalData.GameID namespace. */
#define SAVEGAME_PC34_GAME_CODE_DM1            0x00D1u
#define SAVEGAME_PC34_GAME_CODE_CSB            0x00C5u
#define SAVEGAME_PC34_GAME_CODE_DM2            0x00D2u
#define SAVEGAME_PC34_GAME_CODE_NEXUS          0x00D9u
#define SAVEGAME_PC34_GAME_CODE_THERON         0x00D4u

#define SAVEGAME_PC34_MANIFEST_VERSION         1u

/* Magic as a 64-bit literal so callers can use it in
 * comparisons without including string.h. */
#define SAVEGAME_PC34_MANIFEST_MAGIC_U64 \
    ((uint64_t)0x4C535F4253434D44ull)  /* "RDMCSB_LSV" little-endian */

/* LSV-02 result codes (extend SAVEGAME_PC34_*). Kept distinct
 * from the existing SAVEGAME_PC34_ERROR_* values so a single
 * import call can return a single verdict and a future
 * F0797_SAVEGAME_PC34ErrorToString_Compat lookup can dispatch
 * the manifest codes through the same switch. */
#define SAVEGAME_PC34_MANIFEST_OK                    1
#define SAVEGAME_PC34_MANIFEST_ERR_BAD_MAGIC        -10
#define SAVEGAME_PC34_MANIFEST_ERR_BAD_VERSION      -11
#define SAVEGAME_PC34_MANIFEST_ERR_WRONG_GAME       -12
#define SAVEGAME_PC34_MANIFEST_ERR_BODY_TRUNCATED   -13
#define SAVEGAME_PC34_MANIFEST_ERR_NOT_PRESENT      -14

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

/* ==========================================================
 *  LSV-02 versioned manifest gate (DM1 per-game)
 * ==========================================================
 *
 * F0799_SAVEGAME_PC34PeekManifest_Compat
 *   Quick check whether a buffer starts with the Firestaff
 *   versioned manifest. Returns one of the
 *   SAVEGAME_PC34_MANIFEST_* codes:
 *     MANIFEST_OK              — manifest is present, version
 *                                and gameCode are valid;
 *                                outGameCode/outVersion filled.
 *     MANIFEST_ERR_NOT_PRESENT — vanilla PC 3.4 file (no
 *                                manifest, no Firestaff gating);
 *                                caller may still call
 *                                F0796 (legacy import path).
 *     MANIFEST_ERR_BAD_MAGIC   — first 8 bytes do not match
 *                                "RDMCSB_LSV" — caller should
 *                                reject outright.
 *     MANIFEST_ERR_BAD_VERSION — manifest version above the
 *                                supported envelope; caller
 *                                should reject and refuse to
 *                                interpret the file.
 *   `outBodySize` returns the byte count of the embedded PC 3.4
 *   body that follows the 512-byte header (header + parts,
 *   minus the manifest slot which lives inside the header).
 *   Pass NULL for outGameCode/outVersion/outBodySize if not
 *   needed.
 */
int F0799_SAVEGAME_PC34PeekManifest_Compat(
    const unsigned char* buf,
    int bufSize,
    uint16_t* outVersion,
    uint16_t* outGameCode,
    int* outBodySize);

/* F0800_SAVEGAME_PC34ValidateGameCode_Compat
 *   Pins the importer to a per-game code. Pass the
 *   SAVEGAME_PC34_GAME_CODE_* value matching the runtime
 *   currently trying to load. Returns:
 *     MANIFEST_OK               — manifest, version, and
 *                                 gameCode all match.
 *     MANIFEST_ERR_NOT_PRESENT  — vanilla PC 3.4 file; caller
 *                                 decides whether to allow
 *                                 (legacy ReDMCSB interop)
 *                                 or reject (strict per-game
 *                                 import).
 *     MANIFEST_ERR_WRONG_GAME   — manifest present but
 *                                 gameCode does not match.
 *     MANIFEST_ERR_BAD_VERSION  — manifest present but
 *                                 version above envelope.
 *     MANIFEST_ERR_BODY_TRUNCATED — manifest present but
 *                                 file is shorter than the
 *                                 body size the manifest
 *                                 claims.
 *   The expected body size is taken from the buffer / bufSize
 *   pair (not from the manifest), so a malformed file that
 *   claims a 1 GiB body but ships 1 KiB is caught here.
 */
int F0800_SAVEGAME_PC34ValidateGameCode_Compat(
    const unsigned char* buf,
    int bufSize,
    uint16_t expectedGameCode,
    int requireManifest);

/* F0801_SAVEGAME_PC34GameCodeName_Compat
 *   Human-readable, stable per-game code name. Returns a
 *   static string ("DM1" / "CSB" / "DM2" / "NEXUS" / "THERON"
 *   / "UNKNOWN" / "RESERVED"). */
const char* F0801_SAVEGAME_PC34GameCodeName_Compat(uint16_t gameCode);

#endif /* REDMCSB_MEMORY_SAVEGAME_PC34_NATIVE_EXPORT_PC34_COMPAT_H */
