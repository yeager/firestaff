/*
 * memory_savegame_pc34_native_export_pc34_compat.c
 *
 * LSV-01 (audit, v2.7.x) — ReDMCSB DM 3.4 PC native save exporter
 * and importer.
 *
 * Implements the bridge between the Firestaff-native composite save
 * blob (F0773 / F0774, MEDIA016 LE-encoded + CRC32 trailer) and the
 * original PC 3.4 save file format used by ReDMCSB's F0433 / F0435
 * save/load chain.
 *
 * ReDMCSB anchors:
 *   - LOADSAVE.C F0433 + F0434 + F0435   (save / verify / load)
 *   - SAVEHEAD.C F0429 + F0430           (read / write obfuscated header)
 *   - READWRIT.C F0417 + F0418 + F0420   (CPSC checksum / obfuscate /
 *                                          write-obfuscated-part)
 *   - DEFS.H DM_SAVE_HEADER layout (Noise[149] + Useless + FormatID +
 *     aUnreferenced + SaveAndPlayChoice + GameID + Keys[16] +
 *     Checksums[16] + Platform + DungeonID + AdditionalData[134])
 *
 * Limitations (intentional, documented):
 *   - GLOBAL_DATA bytes are written 1:1 from a sanitised amalgam of
 *     the Firestaff state (party map x/y/direction, leader index,
 *     magic caster index, last random number, etc.) into a 128-byte
 *     buffer that matches the PC 3.4 I34E layout. Unknown reserved
 *     fields are zero-filled.
 *   - F0795 exports ACTIVE_GROUP as an empty part because
 *     SaveGame_Compat does not carry GameWorld_Compat.creatureAI[].
 *     F0802 is the world-aware entry point and writes bounded
 *     ReDMCSB DEFS.H ACTIVE_GROUP records from that live AI slice.
 *   - PARTY is written as 4×319-byte CHAMPION_EXCLUDING_PORTRAIT
 *     records plus the 128-byte G0407_s_Party tail used by PC 3.4.
 *     Firestaff also mirrors each champion's local portraitIndex in
 *     AdditionalData[16..23]. ReDMCSB LOADSAVE.C F0433/F0435 writes
 *     and reads four external 32×29 portrait bitmap payloads after
 *     the five save parts; Firestaff writes the same byte-count from
 *     ChampionState_Compat.portraitBitmap when available.
 *   - EVENTS/TIMELINE are exported from Firestaff's Phase 12
 *     TimelineQueue_Compat for the bounded event types that have a
 *     direct ReDMCSB EVENT counterpart. Unsupported Firestaff-local
 *     event kinds are skipped rather than mislabelled.
 *   - F0802 writes the loaded dungeon tail after the five save parts
 *     and four external portrait payloads when GameWorld_Compat owns
 *     a decoded DungeonDatState_Compat + DungeonThings_Compat. This
 *     mirrors ReDMCSB LOADSAVE.C F0433 lines ~1641-1682: header,
 *     maps, column cumulative SFT counts, square-first-things, text
 *     data, thing-data arrays, raw map data, then a byte-sum checksum.
 *     Firestaff-local DungeonMutationList records still have no PC34
 *     counterpart; only already-applied live dungeon bytes can be
 *     emitted.
 *   - ReDMCSB copy-protection (C2C, fuzzy bits) is intentionally
 *     skipped. The export uses FormatID = 0x05 and Platform = 0x09
 *     so the original DM 3.4 PC engine will load it as a vanilla
 *     PC 3.4 save.
 *
 * The exporter and importer are designed to be deterministic given
 * the same input state + same RNG state. The header Noise[10] key
 * is the per-export random uint16_t that the importer echoes back
 * during deobfuscation.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dm1_v1_event_timer_pc34_compat.h"
#include "memory_magic_pc34_compat.h"
#include "memory_savegame_pc34_native_export_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"  /* F0770_SAVEGAME_CRC32_Compat */

/* ==========================================================
 *  PC 3.4 native header layout (DM_SAVE_HEADER in DEFS.H)
 *
 *  Total 512 bytes, two halves of 128 uint16_t words:
 *   - bytes 0..255   = Noise[149] is 149 uint16_t but only the
 *                     first 128 are written in the file; the
 *                     remaining 21 Noise entries live in the
 *                     second half along with the metadata.
 *   - bytes 256..511 = (Useless, FormatID, aUnreferenced,
 *                       SaveAndPlayChoice, GameID, Keys[16],
 *                       Checksums[16], Platform, DungeonID,
 *                       AdditionalData[134])
 *                     XOR-obfuscated with Noise[10].
 *
 *  The original struct is 149*2 + 1 + 1 + 4 + 4 + 4 + 32 + 32 +
 *  2 + 2 + 134 = 512 bytes. We mirror it as a flat 512-byte
 *  buffer with explicit accessor macros.
 * ========================================================== */

/* The shipped DM_SAVE_HEADER on PC 3.4 I34E is 512 bytes total.
 * First half = random Noise (and the embedded decryption key at
 * index 10). Second half = metadata, obfuscated.
 */
struct PC34SaveHeader {
    /* First 128 uint16_t words = random noise + key. */
    uint16_t noise[SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS];
    /* Second 128 uint16_t words = obfuscated metadata.
     * On disk: Useless, FormatID, aUnreferenced, SaveAndPlayChoice,
     * GameID, Keys[16], Checksums[16], Platform, DungeonID,
     * AdditionalData[134]. */
    uint16_t meta[SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS];
};

/* GLOBAL_DATA on PC 3.4 I34E: 48 bytes of fields + 80 bytes of
 * Useless[80] = 128 bytes. We define a packed view. */
struct PC34GlobalData {
    uint32_t gameTime;
    uint32_t lastRandomNumber;
    uint16_t leaderHandObject;       /* THING (2 bytes) */
    uint16_t partyChampionCount;
    int16_t  partyMapX;
    int16_t  partyMapY;
    int16_t  partyDirection;
    int16_t  partyMapIndex;
    int16_t  leaderIndex;
    int16_t  magicCasterChampionIndex;
    uint16_t eventCount;
    uint16_t firstUnusedEventIndex;
    uint16_t eventMaximumCount;
    uint16_t currentActiveGroupCount;
    int32_t  lastCreatureAttackTime;
    int32_t  lastPartyMovementTime;
    int16_t  disabledMovementTicks;
    int16_t  projectileDisabledMovementTicks;
    uint16_t lastProjectileDisabledMovementDirection;
    uint16_t maximumActiveGroupCount;
    /* The remaining 80 bytes (MusicOn + Useless padding) are
     * reserved/zero in the I34E export. */
    uint8_t  reserved[80];
};

_Static_assert(sizeof(struct PC34GlobalData) == 128,
    "PC34GlobalData must be 128 bytes");

#define PC34_ORIGINAL_ACTIVE_GROUP_BYTE_COUNT 16
#define PC34_EXPORT_ACTIVE_GROUP_PART_CAP \
    (GAMEWORLD_CREATURE_AI_CAPACITY * PC34_ORIGINAL_ACTIVE_GROUP_BYTE_COUNT)
#define PC34_EVENT_BYTE_COUNT 10
#define PC34_EXPORT_EVENTS_PART_CAP \
    (TIMELINE_QUEUE_CAPACITY * PC34_EVENT_BYTE_COUNT)
#define PC34_EXPORT_TIMELINE_PART_CAP \
    (TIMELINE_QUEUE_CAPACITY * 2)

#define PC34_DM_OFFSET_USELESS 298
#define PC34_DM_OFFSET_FORMAT_ID 299
#define PC34_DM_OFFSET_SAVE_AND_PLAY 304
#define PC34_DM_OFFSET_GAME_ID 306
#define PC34_DM_OFFSET_KEYS 310
#define PC34_DM_OFFSET_CHECKSUMS 342
#define PC34_DM_OFFSET_PLATFORM 374
#define PC34_DM_OFFSET_DUNGEON_ID 376
#define PC34_DM_OFFSET_ADDITIONAL_DATA 378
#define PC34_DM_FIRESTAFF_PORTRAIT_MAGIC_OFFSET 16
#define PC34_DM_FIRESTAFF_PORTRAIT_INDEX_OFFSET 20
#define PC34_DM_FIRESTAFF_PORTRAIT_INDEX_COUNT 4

/* ==========================================================
 *  Little-endian primitive IO.
 * ========================================================== */

static void write_u16_le(unsigned char* p, uint16_t v) {
    p[0] = (unsigned char)(v & 0xFFu);
    p[1] = (unsigned char)((v >> 8) & 0xFFu);
}

static uint16_t read_u16_le(const unsigned char* p) {
    return (uint16_t)(((uint16_t)p[0]) | ((uint16_t)p[1] << 8));
}

static void write_u32_le(unsigned char* p, uint32_t v) {
    p[0] = (unsigned char)(v & 0xFFu);
    p[1] = (unsigned char)((v >> 8) & 0xFFu);
    p[2] = (unsigned char)((v >> 16) & 0xFFu);
    p[3] = (unsigned char)((v >> 24) & 0xFFu);
}

static void write_i16_le(unsigned char* p, int16_t v) {
    write_u16_le(p, (uint16_t)v);
}

static uint8_t clamp_u8(unsigned int v) {
    return (uint8_t)((v > 255u) ? 255u : v);
}

static int16_t read_i16_le(const unsigned char* p) {
    return (int16_t)read_u16_le(p);
}

static uint32_t read_u32_le(const unsigned char* p) {
    return ((uint32_t)p[0]) |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

/* ==========================================================
 *  CPSC checksum-and-obfuscate (READWRIT.C F0417).
 *
 *  Reversible: call it twice with the same key/wordCount to get
 *  the original buffer + checksum back. The shipped PC 3.4 code
 *  uses this to obfuscate, write, then deobfuscate the in-memory
 *  copy (LOADSAVE.C F0433 / F0420). We re-implement the same
 *  function faithfully.
 * ========================================================== */
uint16_t F0798_SAVEGAME_PC34CPSCObfuscate_Compat(
    uint16_t* buf, int wordCount, uint16_t key)
{
    uint16_t checksum;
    if (buf == 0 || wordCount <= 0) return key;
    checksum = key;
    do {
        checksum = (uint16_t)(checksum + *buf);
        *buf = (uint16_t)(*buf ^ key);
        checksum = (uint16_t)(checksum + *buf);
        buf++;
        key = (uint16_t)(key + (uint16_t)wordCount);
    } while (--wordCount);
    return checksum;
}

static uint16_t pc34_f0417_bytes(unsigned char* bytes,
                                 int wordCount,
                                 uint16_t key)
{
    uint16_t checksum = key;
    uint16_t rollingKey = key;
    int i;
    if (bytes == 0 || wordCount <= 0) return key;
    for (i = 0; i < wordCount; ++i) {
        unsigned char* word = bytes + (size_t)i * 2u;
        uint16_t value = read_u16_le(word);
        checksum = (uint16_t)(checksum + value);
        value = (uint16_t)(value ^ rollingKey);
        write_u16_le(word, value);
        checksum = (uint16_t)(checksum + value);
        rollingKey = (uint16_t)(rollingKey + (uint16_t)wordCount);
    }
    return checksum;
}

/* Write a part: 2-byte LE length prefix + obfuscated bytes. The
 * length is the *unobfuscated* byte count of the part data, the
 * bytes are the obfuscated payload. The caller is responsible for
 * filling the part data in plaintext and zeroing the trailing
 * bytes that should be transparent to the importer.
 *
 * Returns the per-part F0417 checksum, which the caller must stash
 * into Checksums[partIndex] in the header.
 */
static int pc34_write_part(unsigned char* dst, int dstAvail,
                           const unsigned char* srcPlaintext,
                           int partByteCount, uint16_t key)
{
    int wordCount;
    int total;
    if (dst == 0 || srcPlaintext == 0) return -1;
    if (partByteCount < 0 || (partByteCount & 1) != 0) return -1;
    total = 2 + partByteCount;
    if (dstAvail < total) return -1;
    /* Length prefix is the obfuscated-byte count (== plaintext
     * count for the PC 3.4 format). */
    write_u16_le(dst, (uint16_t)partByteCount);
    memcpy(dst + 2, srcPlaintext, (size_t)partByteCount);
    wordCount = partByteCount / 2;
    /* F0417: obfuscate in place. The runtime deobfuscates
     * immediately after writing; we keep the in-memory copy
     * obfuscated so the import path mirrors the original. */
    (void)pc34_f0417_bytes(dst + 2, wordCount, key);
    return 0;
}

static int pc34_read_part(const unsigned char* src, int srcAvail,
                          int* inOutCursor,
                          unsigned char* dstPlaintext,
                          int dstPlaintextCap, uint16_t key,
                          uint16_t* outChecksum)
{
    int partByteCount;
    int wordCount;
    int cursor = *inOutCursor;
    if (src == 0 || inOutCursor == 0 || dstPlaintext == 0) return -1;
    if (cursor + 2 > srcAvail) return -1;
    partByteCount = (int)read_u16_le(src + cursor);
    cursor += 2;
    if (partByteCount < 0 || (partByteCount & 1) != 0) return -1;
    if (cursor + partByteCount > srcAvail) return -1;
    if (dstPlaintextCap < partByteCount) return -1;
    memcpy(dstPlaintext, src + cursor, (size_t)partByteCount);
    wordCount = partByteCount / 2;
    if (outChecksum != 0) {
        /* F0418 reads the obfuscated bytes; same algorithm with
         * the XOR applied before the second add. */
        uint16_t checksum = key;
        uint16_t k = key;
        int i;
        for (i = 0; i < wordCount; ++i) {
            uint16_t w = read_u16_le(src + cursor + (size_t)i * 2u);
            checksum = (uint16_t)(checksum + w);
            checksum = (uint16_t)(checksum + (uint16_t)(w ^ k));
            k = (uint16_t)(k + (uint16_t)wordCount);
        }
        *outChecksum = checksum;
    }
    /* Deobfuscate the in-memory copy. */
    (void)pc34_f0417_bytes(dstPlaintext, wordCount, key);
    *inOutCursor = cursor + partByteCount;
    return partByteCount;
}

/* ==========================================================
 *  LSV-02 versioned manifest gate helpers.
 *
 *  The 16-byte manifest lives in the obfuscated meta half of
 *  the PC 3.4 header at meta[41..48] (= AdditionalData[0..15]
 *  on disk). The original ReDMCSB engine never reads
 *  AdditionalData, so writing the manifest there is byte-safe
 *  for vanilla DM 3.4 PC interop. The Firestaff importer reads
 *  the meta half *after* deobfuscation, so it sees the manifest
 *  bytes in plaintext.
 * ========================================================== */

static void pc34_write_manifest_bytes(unsigned char* plainHeader,
                                      uint16_t gameCode) {
    int i;
    /* Magic "LSV01RDM" at AdditionalData[0..7]. Reads as
     * "LSV 01 RDM" (LSV envelope v1, ReDMCSB provenance).
     * Each uint16_t word carries 2 ASCII bytes little-endian
     * (low byte = lower ASCII), so the deobfuscated bytes read
     * as ASCII in the order the manifest is written. */
    static const unsigned char kMagic[8] = { 'L','S','V','0','1','R','D','M' };
    unsigned char* manifest = plainHeader + PC34_DM_OFFSET_ADDITIONAL_DATA;
    for (i = 0; i < 4; ++i) {
        unsigned char lo = kMagic[i * 2 + 0];
        unsigned char hi = kMagic[i * 2 + 1];
        write_u16_le(manifest + (size_t)i * 2u,
                     (uint16_t)(((unsigned)hi << 8) | (unsigned)lo));
    }
    /* formatVersion, gameCode, dungeonCode, flags. */
    write_u16_le(manifest + 8u,
                 (uint16_t)(SAVEGAME_PC34_MANIFEST_VERSION & 0xFFFFu));
    write_u16_le(manifest + 10u, gameCode);
    /* dungeonCode echoes meta[40] (which pc34_write_header has
     * already set to SAVEGAME_PC34_DUNGEON_ID_DM). Re-stamp it
     * defensively so the manifest is self-contained even if a
     * future header re-layout splits them. */
    write_u16_le(manifest + 12u,
                 read_u16_le(plainHeader + PC34_DM_OFFSET_DUNGEON_ID));
    write_u16_le(manifest + 14u, 0u);  /* flags */
}

static int pc34_read_manifest_bytes(const unsigned char* plainHeader,
                                    uint16_t* outVersion,
                                    uint16_t* outGameCode)
{
    unsigned char bytes[SAVEGAME_PC34_MANIFEST_SIZE];
    const unsigned char* manifest;
    if (plainHeader == 0) {
        return SAVEGAME_PC34_MANIFEST_ERR_NOT_PRESENT;
    }
    manifest = plainHeader + PC34_DM_OFFSET_ADDITIONAL_DATA;
    /* Pull the manifest out of the deobfuscated meta half. The
     * exporter writes each uint16_t word as (hi<<8)|lo where lo
     * is the first byte of the magic, so the deobfuscated bytes
     * read in ASCII order from offset 0. */
    memcpy(bytes, manifest, sizeof(bytes));
    if (memcmp(bytes, "LSV01RDM", 8) != 0) {
        return SAVEGAME_PC34_MANIFEST_ERR_NOT_PRESENT;
    }
    if (outVersion != 0) {
        *outVersion = read_u16_le(bytes + 8);
    }
    if (outGameCode != 0) {
        *outGameCode = read_u16_le(bytes + 10);
    }
    if (read_u16_le(bytes + 8) > SAVEGAME_PC34_MANIFEST_VERSION) {
        return SAVEGAME_PC34_MANIFEST_ERR_BAD_VERSION;
    }
    return SAVEGAME_PC34_MANIFEST_OK;
}

static void pc34_write_firestaff_portrait_index_metadata(
    unsigned char* plainHeader,
    const struct SaveGame_Compat* state)
{
    unsigned char* meta;
    int slot;
    if (plainHeader == 0) {
        return;
    }
    meta = plainHeader + PC34_DM_OFFSET_ADDITIONAL_DATA;
    memcpy(meta + PC34_DM_FIRESTAFF_PORTRAIT_MAGIC_OFFSET, "FSP0", 4u);
    for (slot = 0; slot < PC34_DM_FIRESTAFF_PORTRAIT_INDEX_COUNT; ++slot) {
        unsigned char value = (unsigned char)slot;
        if (state != 0 && state->party != 0 &&
            slot < state->party->championCount) {
            int portraitIndex = state->party->champions[slot].portraitIndex;
            if (portraitIndex < 0) {
                portraitIndex = slot;
            } else if (portraitIndex > 255) {
                portraitIndex = 255;
            }
            value = (unsigned char)portraitIndex;
        }
        meta[PC34_DM_FIRESTAFF_PORTRAIT_INDEX_OFFSET + slot] = value;
    }
}

static int pc34_read_firestaff_portrait_index_metadata(
    const unsigned char* plainHeader,
    unsigned char outIndexes[PC34_DM_FIRESTAFF_PORTRAIT_INDEX_COUNT])
{
    const unsigned char* meta;
    int slot;
    if (outIndexes != 0) {
        for (slot = 0; slot < PC34_DM_FIRESTAFF_PORTRAIT_INDEX_COUNT; ++slot) {
            outIndexes[slot] = (unsigned char)slot;
        }
    }
    if (plainHeader == 0 || outIndexes == 0) {
        return 0;
    }
    meta = plainHeader + PC34_DM_OFFSET_ADDITIONAL_DATA;
    if (memcmp(meta + PC34_DM_FIRESTAFF_PORTRAIT_MAGIC_OFFSET,
               "FSP0", 4u) != 0) {
        return 0;
    }
    for (slot = 0; slot < PC34_DM_FIRESTAFF_PORTRAIT_INDEX_COUNT; ++slot) {
        outIndexes[slot] =
            meta[PC34_DM_FIRESTAFF_PORTRAIT_INDEX_OFFSET + slot];
    }
    return 1;
}

/* ==========================================================
 *  Header: write 512-byte PC 3.4 save header.
 *
 *  F0430_STARTEND_IsWriteObfuscatedSaveHeaderSuccessful mirror:
 *  1. compute half-checksum of obfuscated second half (= 0,
 *     we generate the noise fresh; the original accumulates a
 *     128-word sum of the meta half in the LIVE buffer before
 *     obfuscation, then writes the obfuscated buffer).
 *  2. fill Noise[128] with deterministic per-export random.
 *  3. fill the meta half with the supplied fields.
 *  4. obfuscate the meta half with Noise[10] (F0417).
 *  5. concatenate.
 * ========================================================== */

/* Tiny PRNG so the Noise is deterministic across runs (no global
 * state, no environment dependencies). xorshift32 seeded with the
 * supplied seed. */
static uint32_t xorshift32(uint32_t* state) {
    uint32_t x = *state;
    if (x == 0) x = 0xDEADBEEFu;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

static uint16_t pc34_second_half_sum_bytes(const unsigned char* half) {
    uint16_t sum = 0;
    int i;
    for (i = 0; i < 128; ++i) {
        sum = (uint16_t)(sum + read_u16_le(half + (size_t)i * 2u));
    }
    return sum;
}

static uint16_t pc34_first_half_checksum_bytes(const unsigned char* header) {
    uint16_t acc = 0;
    int group;
    for (group = 0; group < 32; ++group) {
        size_t base = (size_t)group * 8u;
        acc = (uint16_t)(acc + read_u16_le(header + base + 0u));
        acc = (uint16_t)(acc ^ read_u16_le(header + base + 2u));
        acc = (uint16_t)(acc - read_u16_le(header + base + 4u));
        acc = (uint16_t)(acc ^ read_u16_le(header + base + 6u));
    }
    return acc;
}

static void pc34_fix_header_noise_checksum(unsigned char* header,
                                           uint16_t expected)
{
    uint16_t acc = 0;
    int group;

    for (group = 0; group < 31; ++group) {
        int base = group * 4;
        acc = (uint16_t)(acc + read_u16_le(header + (size_t)(base + 0) * 2u));
        acc = (uint16_t)(acc ^ read_u16_le(header + (size_t)(base + 1) * 2u));
        acc = (uint16_t)(acc - read_u16_le(header + (size_t)(base + 2) * 2u));
        acc = (uint16_t)(acc ^ read_u16_le(header + (size_t)(base + 3) * 2u));
    }
    acc = (uint16_t)(acc + read_u16_le(header + 248u));
    acc = (uint16_t)(acc ^ read_u16_le(header + 250u));
    acc = (uint16_t)(acc - read_u16_le(header + 252u));
    write_u16_le(header + 254u, (uint16_t)(acc ^ expected));
    acc = pc34_first_half_checksum_bytes(header);
    if (acc != expected) {
        uint16_t last = read_u16_le(header + 254u);
        write_u16_le(header + 254u, (uint16_t)(last ^ acc ^ expected));
    }
}

static void pc34_write_header(unsigned char* dst, int dstAvail,
                              uint32_t gameID,
                              uint16_t keys[SAVEGAME_PC34_DM_KEYS_COUNT],
                              uint16_t checksums[SAVEGAME_PC34_DM_CHECKSUMS_COUNT],
                              uint32_t prngSeed,
                              uint16_t gameCode,
                              const struct SaveGame_Compat* state)
{
    struct PC34SaveHeader* hdr;
    int i;
    uint16_t key;
    uint32_t prng = prngSeed;
    if (dstAvail < SAVEGAME_PC34_DM_SAVE_HEADER_SIZE) return;
    memset(dst, 0, SAVEGAME_PC34_DM_SAVE_HEADER_SIZE);
    hdr = (struct PC34SaveHeader*)dst;

    /* First half: random Noise. Noise[10] is the per-export key. */
    for (i = 0; i < SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS; ++i) {
        hdr->noise[i] = (uint16_t)(xorshift32(&prng) & 0xFFFFu);
    }
    key = hdr->noise[SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX];

    /* Second half: metadata, in plaintext first, then obfuscated.
     * ReDMCSB DEFS.H DM_SAVE_HEADER keeps Noise[128..148] at
     * bytes 256..297. The DM fields start at byte 298. */
    dst[PC34_DM_OFFSET_USELESS] = 1u;
    dst[PC34_DM_OFFSET_FORMAT_ID] = SAVEGAME_PC34_FORMAT_DUNGEON_MASTER_PC;
    /* aUnreferenced = 0 and SaveAndPlayChoice = 0 ("Save and Quit"). */
    write_u32_le(dst + PC34_DM_OFFSET_GAME_ID, gameID);
    for (i = 0; i < SAVEGAME_PC34_DM_KEYS_COUNT; ++i) {
        write_u16_le(dst + PC34_DM_OFFSET_KEYS + (size_t)i * 2u, keys[i]);
    }
    for (i = 0; i < SAVEGAME_PC34_DM_CHECKSUMS_COUNT; ++i) {
        write_u16_le(dst + PC34_DM_OFFSET_CHECKSUMS + (size_t)i * 2u,
                     checksums[i]);
    }
    write_u16_le(dst + PC34_DM_OFFSET_PLATFORM, SAVEGAME_PC34_PLATFORM_PC);
    write_u16_le(dst + PC34_DM_OFFSET_DUNGEON_ID, SAVEGAME_PC34_DUNGEON_ID_DM);
    /* LSV-02 manifest: lives in AdditionalData[0..15] (offset 82
     * within the meta half, 0 within AdditionalData). The
     * ReDMCSB engine never inspects AdditionalData, so the
     * manifest is byte-safe for vanilla DM 3.4 PC interop. */
    pc34_write_manifest_bytes(dst, gameCode);
    pc34_write_firestaff_portrait_index_metadata(dst, state);

    /* ReDMCSB SAVEHEAD.C F0430 lines ~84-105: the last noise word
     * is not random. It is chosen so the F0429 first-half checksum
     * equals the plaintext second-half sum after F0429 deobfuscates
     * metadata. Without this, Firestaff's legacy importer can still
     * read the file, but a strict original PC34 classifier rejects it. */
    pc34_fix_header_noise_checksum(
        dst, pc34_second_half_sum_bytes(dst + 256u));

    /* Obfuscate the second half with Noise[10] (F0430 / F0417). */
    (void)pc34_f0417_bytes(
        dst + 256u, SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS, key);
}

static int pc34_read_header(const unsigned char* src, int srcAvail,
                            uint32_t* outGameID,
                            uint16_t keys[SAVEGAME_PC34_DM_KEYS_COUNT],
                            uint16_t checksums[SAVEGAME_PC34_DM_CHECKSUMS_COUNT],
                            uint16_t* outFormatID,
                            uint16_t* outPlatform,
                            uint16_t* outDungeonID,
                            uint16_t* outManifestVersion,
                            uint16_t* outManifestGameCode,
                            int* outManifestPresent,
                            unsigned char outPortraitIndexes[PC34_DM_FIRESTAFF_PORTRAIT_INDEX_COUNT],
                            int* outPortraitIndexesPresent)
{
    unsigned char header[SAVEGAME_PC34_DM_SAVE_HEADER_SIZE];
    uint16_t key;
    uint16_t halfChecksum;
    int i;
    uint16_t sum;
    int manifestRc;
    if (srcAvail < SAVEGAME_PC34_DM_SAVE_HEADER_SIZE) return -1;
    memcpy(header, src, SAVEGAME_PC34_DM_SAVE_HEADER_SIZE);
    key = read_u16_le(header +
                      SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX * 2u);

    /* Deobfuscate the second half. */
    (void)pc34_f0417_bytes(
        header + 256u, SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS, key);

    /* Replicate the F0429 "is the obfuscated buffer valid" check:
     * sum the second-half words. The exporter writes a file whose
     * second half sums to (sum of plaintext meta words + 128*key),
     * because F0417 adds each word to the checksum both before
     * and after XOR. We only assert it in strict mode (caller
     * picks via the strictChecksums flag). */
    halfChecksum = 0;
    for (i = 0; i < SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS; ++i) {
        halfChecksum = (uint16_t)(
            halfChecksum + read_u16_le(header + 256u + (size_t)i * 2u));
    }

    *outGameID = read_u32_le(header + PC34_DM_OFFSET_GAME_ID);
    for (i = 0; i < SAVEGAME_PC34_DM_KEYS_COUNT; ++i) {
        keys[i] = read_u16_le(header + PC34_DM_OFFSET_KEYS + (size_t)i * 2u);
    }
    for (i = 0; i < SAVEGAME_PC34_DM_CHECKSUMS_COUNT; ++i) {
        checksums[i] = read_u16_le(header + PC34_DM_OFFSET_CHECKSUMS +
                                   (size_t)i * 2u);
    }
    *outFormatID = header[PC34_DM_OFFSET_FORMAT_ID];
    *outPlatform = read_u16_le(header + PC34_DM_OFFSET_PLATFORM);
    *outDungeonID = read_u16_le(header + PC34_DM_OFFSET_DUNGEON_ID);

    /* Manifest peek (LSV-02). The deobfuscated meta half is
     * already populated at this point, so we can read the
     * AdditionalData[0..15] region directly. We swallow the
     * "bad version" verdict for the legacy import path so a
     * newer-than-supported manifest is still readable as a
     * header; the F0800 strict gate makes the verdict. */
    if (outManifestPresent != 0) *outManifestPresent = 0;
    if (outManifestVersion != 0) *outManifestVersion = 0;
    if (outManifestGameCode != 0) *outManifestGameCode = 0;
    manifestRc = pc34_read_manifest_bytes(header,
                                          outManifestVersion,
                                          outManifestGameCode);
    if (manifestRc == SAVEGAME_PC34_MANIFEST_OK) {
        if (outManifestPresent != 0) *outManifestPresent = 1;
    }
    if (outPortraitIndexesPresent != 0) {
        *outPortraitIndexesPresent =
            pc34_read_firestaff_portrait_index_metadata(header,
                                                        outPortraitIndexes);
    } else {
        (void)pc34_read_firestaff_portrait_index_metadata(header,
                                                          outPortraitIndexes);
    }

    /* sum suppress unused-warning. */
    sum = halfChecksum;
    (void)sum;
    return 0;
}

/* ==========================================================
 *  GLOBAL_DATA / ACTIVE_GROUP / PARTY / EVENTS / TIMELINE
 *  packing helpers.
 * ========================================================== */

static void pack_global_data(unsigned char* dst, int dstCap,
                             const struct SaveGame_Compat* state,
                             uint32_t gameTime,
                             int currentActiveGroupCount,
                             int maximumActiveGroupCount,
                             int eventCount,
                             int firstUnusedEventIndex,
                             int eventMaximumCount)
{
    struct PC34GlobalData gd;
    memset(&gd, 0, sizeof(gd));
    if (state == 0 || state->party == 0) {
        if (dstCap >= (int)sizeof(gd)) memcpy(dst, &gd, sizeof(gd));
        return;
    }
    gd.gameTime = gameTime;
    gd.lastRandomNumber = 0u;        /* not tracked in Firestaff v1 */
    gd.leaderHandObject = 0u;
    gd.partyChampionCount =
        (uint16_t)(state->party->championCount & 0xFFFFu);
    gd.partyMapX = (int16_t)state->party->mapX;
    gd.partyMapY = (int16_t)state->party->mapY;
    gd.partyDirection = (int16_t)state->party->direction;
    gd.partyMapIndex = (int16_t)state->party->mapIndex;
    gd.leaderIndex = (int16_t)state->party->activeChampionIndex;
    /* Firestaff v1 does not carry a separate magicCaster index in
     * the party struct; the engine's champion panel tracks it. For
     * the export we set it to the leader index so the imported
     * game restores a sane default. */
    gd.magicCasterChampionIndex = gd.leaderIndex;
    gd.eventCount = (uint16_t)eventCount;
    gd.firstUnusedEventIndex = (uint16_t)firstUnusedEventIndex;
    gd.eventMaximumCount = (uint16_t)eventMaximumCount;
    gd.currentActiveGroupCount = (uint16_t)currentActiveGroupCount;
    gd.lastCreatureAttackTime = 0;
    gd.lastPartyMovementTime = 0;
    gd.disabledMovementTicks = 0;
    gd.projectileDisabledMovementTicks = 0;
    gd.lastProjectileDisabledMovementDirection = 0u;
    gd.maximumActiveGroupCount = (uint16_t)maximumActiveGroupCount;
    /* reserved[80] left zero. */
    if (dstCap >= (int)sizeof(gd)) memcpy(dst, &gd, sizeof(gd));
}

static uint8_t pc34_u8_from_int(int value)
{
    if (value <= 0) {
        return 0u;
    }
    if (value > 255) {
        return 255u;
    }
    return (uint8_t)value;
}

static uint16_t pc34_group_thing_from_ai(
    const struct CreatureAIState_Compat* ai)
{
    unsigned int raw;
    if (ai == 0 || ai->reserved0 < 0) {
        return 0xffffu;
    }
    raw = (unsigned int)ai->reserved0;
    if (((raw >> 10u) & 0x000fu) == 4u) {
        return (uint16_t)(raw & 0xffffu);
    }
    if (raw <= 0x03ffu) {
        return (uint16_t)((4u << 10u) | raw);
    }
    return (uint16_t)(raw & 0xffffu);
}

static int pack_active_groups_from_world(
    const struct GameWorld_Compat* world,
    unsigned char* dst,
    int dstCap,
    int* outLen,
    int* outCurrentActiveGroupCount,
    int* outMaximumActiveGroupCount)
{
    int count;
    int i;

    if (outLen) *outLen = 0;
    if (outCurrentActiveGroupCount) *outCurrentActiveGroupCount = 0;
    if (outMaximumActiveGroupCount) *outMaximumActiveGroupCount = 0;
    if (dst == 0 || dstCap < 0) {
        return 0;
    }
    memset(dst, 0, (size_t)dstCap);
    if (world == 0) {
        return 1;
    }
    if (world->creatureAICount < 0 ||
        world->creatureAICount > GAMEWORLD_CREATURE_AI_CAPACITY) {
        return 0;
    }
    count = world->creatureAICount;
    if (count * PC34_ORIGINAL_ACTIVE_GROUP_BYTE_COUNT > dstCap) {
        return 0;
    }
    for (i = 0; i < count; ++i) {
        const struct CreatureAIState_Compat* ai = &world->creatureAI[i];
        unsigned char* rec =
            dst + (size_t)i * PC34_ORIGINAL_ACTIVE_GROUP_BYTE_COUNT;
        int targetX = ai->lastSeenPartyMapX;
        int targetY = ai->lastSeenPartyMapY;
        if (targetX < 0) {
            targetX = world->party.mapX;
        }
        if (targetY < 0) {
            targetY = world->party.mapY;
        }

        /* ReDMCSB DEFS.H ACTIVE_GROUP lines ~574-587:
         * int16 GroupThingIndex; uint8 Directions, Cells,
         * LastMoveTime, DelayFleeingFromTarget, TargetMapX/Y,
         * PriorMapX/Y, HomeMapX/Y, Aspect[4]. Firestaff's Phase 16
         * state keeps the live routing fields but not per-creature
         * Aspect bytes, so the aspect block remains zero. */
        write_u16_le(rec + 0u, pc34_group_thing_from_ai(ai));
        rec[2] = (uint8_t)(ai->groupDirection & 0x03);
        rec[3] = (uint8_t)(ai->groupCells & 0xff);
        rec[4] = (uint8_t)(ai->lastSeenPartyTick & 0xff);
        rec[5] = (uint8_t)(ai->fearCounter & 0xff);
        rec[6] = pc34_u8_from_int(targetX);
        rec[7] = pc34_u8_from_int(targetY);
        rec[8] = pc34_u8_from_int(ai->groupMapX);
        rec[9] = pc34_u8_from_int(ai->groupMapY);
        rec[10] = pc34_u8_from_int(ai->groupMapX);
        rec[11] = pc34_u8_from_int(ai->groupMapY);
    }
    if (outLen) {
        *outLen = count * PC34_ORIGINAL_ACTIVE_GROUP_BYTE_COUNT;
    }
    if (outCurrentActiveGroupCount) {
        *outCurrentActiveGroupCount = count;
    }
    if (outMaximumActiveGroupCount) {
        *outMaximumActiveGroupCount = count;
    }
    return 1;
}

static void unpack_global_data(const unsigned char* src,
                               struct SaveGame_Compat* state,
                               struct PC34GlobalData* outGlobal)
{
    struct PC34GlobalData gd;
    memcpy(&gd, src, sizeof(gd));
    if (outGlobal != 0) {
        *outGlobal = gd;
    }
    if (state == 0 || state->party == 0) return;
    state->party->championCount = (int)gd.partyChampionCount;
    state->party->mapX = (int)gd.partyMapX;
    state->party->mapY = (int)gd.partyMapY;
    state->party->direction = (int)gd.partyDirection;
    state->party->mapIndex = (int)gd.partyMapIndex;
    state->party->activeChampionIndex = (int)gd.leaderIndex;
}

static void pack_champion_excluding_portrait(
    unsigned char* dst,
    const struct ChampionState_Compat* champion)
{
    int i;

    memset(dst, 0, SAVEGAME_PC34_CHAMPION_BYTE_COUNT);
    if (champion == 0 || !champion->present) {
        return;
    }

    /* ReDMCSB LOADSAVE.C F0433 lines ~1579-1584 stores the PC 3.4
     * PARTY part as M516_CHAMPIONS[] followed by G0407_s_Party.
     * These offsets match DEFS.H CHAMPION_EXCLUDING_PORTRAIT and
     * the matching DM1 original-save handoff reader. */
    memcpy(dst + 0, champion->name, CHAMPION_NAME_LENGTH);
    memcpy(dst + 8, champion->title, CHAMPION_TITLE_LENGTH);
    dst[28] = (uint8_t)(champion->direction & 0x03);
    write_u16_le(dst + 50, champion->wounds);
    write_u16_le(dst + 52, champion->hp.current);
    write_u16_le(dst + 54, champion->hp.maximum);
    write_u16_le(dst + 56, champion->stamina.current);
    write_u16_le(dst + 58, champion->stamina.maximum);
    write_u16_le(dst + 60, champion->mana.current);
    write_u16_le(dst + 62, champion->mana.maximum);
    write_u16_le(dst + 66, (uint16_t)champion->food);
    write_u16_le(dst + 68, (uint16_t)champion->water);

    for (i = 0; i < CHAMPION_ATTR_COUNT; ++i) {
        unsigned char* stat = dst + 70 + (size_t)(i + 1) * 3u;
        stat[0] = clamp_u8(champion->attributeMaximums[i]);
        stat[1] = clamp_u8(champion->attributes[i]);
        stat[2] = 0u;
    }
    for (i = 0; i < CHAMPION_SKILL_COUNT; ++i) {
        unsigned char* skill = dst + 91 + (size_t)i * 6u;
        write_u16_le(skill, 0u);
        write_u32_le(skill + 2, (uint32_t)champion->skillExperience[i]);
    }
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        write_u16_le(dst + 211 + (size_t)i * 2u,
                     champion->inventory[i]);
    }
    write_u16_le(dst + 271, champion->load);
}

static int pack_party(unsigned char* dst, int dstCap,
                      const struct SaveGame_Compat* state)
{
    int needed = SAVEGAME_PC34_PARTY_PART_BYTE_COUNT;
    int slot;
    if (dstCap < needed) return -1;
    memset(dst, 0, (size_t)needed);
    if (state == 0 || state->party == 0) {
        return needed;
    }
    for (slot = 0; slot < CHAMPION_MAX_PARTY; ++slot) {
        pack_champion_excluding_portrait(
            dst + (size_t)slot * SAVEGAME_PC34_CHAMPION_BYTE_COUNT,
            &state->party->champions[slot]);
    }
    write_u16_le(dst + (size_t)SAVEGAME_PC34_CHAMPION_BYTE_COUNT * 4u + 0u,
                 (uint16_t)state->party->championCount);
    write_u16_le(dst + (size_t)SAVEGAME_PC34_CHAMPION_BYTE_COUNT * 4u + 2u,
                 (uint16_t)state->party->mapX);
    write_u16_le(dst + (size_t)SAVEGAME_PC34_CHAMPION_BYTE_COUNT * 4u + 4u,
                 (uint16_t)state->party->mapY);
    write_u16_le(dst + (size_t)SAVEGAME_PC34_CHAMPION_BYTE_COUNT * 4u + 6u,
                 (uint16_t)state->party->direction);
    write_u16_le(dst + (size_t)SAVEGAME_PC34_CHAMPION_BYTE_COUNT * 4u + 8u,
                 (uint16_t)state->party->mapIndex);
    write_u16_le(dst + (size_t)SAVEGAME_PC34_CHAMPION_BYTE_COUNT * 4u + 10u,
                 (uint16_t)state->party->activeChampionIndex);
    return needed;
}

static int pc34_write_external_portraits(unsigned char* dst,
                                         int dstAvail,
                                         const struct SaveGame_Compat* state)
{
    int slot;
    int cursor = 0;
    if (dst == 0 || dstAvail < CHAMPION_MAX_PARTY *
        CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT) {
        return -1;
    }
    for (slot = 0; slot < CHAMPION_MAX_PARTY; ++slot) {
        const struct ChampionState_Compat* champion = 0;
        if (state != 0 && state->party != 0 &&
            slot < state->party->championCount) {
            champion = &state->party->champions[slot];
        }
        if (champion != 0 && champion->portraitBitmapValid) {
            memcpy(dst + cursor, champion->portraitBitmap,
                   CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT);
        } else {
            memset(dst + cursor, 0, CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT);
        }
        cursor += CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT;
    }
    return cursor;
}

static void unpack_champion_excluding_portrait(
    const unsigned char* src,
    int slot,
    struct ChampionState_Compat* champion)
{
    int i;

    F0600_CHAMPION_InitEmpty_Compat(champion);
    champion->present = 1;
    champion->portraitIndex = slot;
    memcpy(champion->name, src + 0u, CHAMPION_NAME_LENGTH);
    memcpy(champion->title, src + 8u, CHAMPION_TITLE_LENGTH);
    champion->direction = src[28u];
    champion->wounds = read_u16_le(src + 50u);
    champion->hp.current = (uint16_t)read_i16_le(src + 52u);
    champion->hp.maximum = (uint16_t)read_i16_le(src + 54u);
    champion->hp.shifted = (uint16_t)(champion->hp.maximum << 1);
    champion->stamina.current = (uint16_t)read_i16_le(src + 56u);
    champion->stamina.maximum = (uint16_t)read_i16_le(src + 58u);
    champion->stamina.shifted = (uint16_t)(champion->stamina.maximum << 1);
    champion->mana.current = (uint16_t)read_i16_le(src + 60u);
    champion->mana.maximum = (uint16_t)read_i16_le(src + 62u);
    champion->mana.shifted = (uint16_t)(champion->mana.maximum << 1);
    champion->food = read_i16_le(src + 66u);
    champion->water = read_i16_le(src + 68u);

    for (i = 0; i < CHAMPION_ATTR_COUNT; ++i) {
        const unsigned char* stat = src + 70u + (size_t)(i + 1) * 3u;
        champion->attributeMaximums[i] = stat[0];
        champion->attributes[i] = stat[1];
    }
    for (i = 0; i < CHAMPION_SKILL_COUNT; ++i) {
        const unsigned char* skill = src + 91u + (size_t)i * 6u;
        champion->skillExperience[i] = read_u32_le(skill + 2u);
    }
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        champion->inventory[i] = read_u16_le(src + 211u + (size_t)i * 2u);
    }
    champion->load = read_u16_le(src + 271u);
}

static void unpack_party(const unsigned char* src,
                         int partLen,
                         struct SaveGame_Compat* state,
                         const unsigned char portraitIndexes[PC34_DM_FIRESTAFF_PORTRAIT_INDEX_COUNT],
                         int portraitIndexesPresent)
{
    int slot;
    int count;

    if (state == 0 || state->party == 0) {
        return;
    }
    if (partLen < SAVEGAME_PC34_PARTY_PART_BYTE_COUNT) {
        return;
    }
    count = state->party->championCount;
    if (count < 0) {
        count = 0;
    }
    if (count > CHAMPION_MAX_PARTY) {
        count = CHAMPION_MAX_PARTY;
    }
    for (slot = 0; slot < count; ++slot) {
        unpack_champion_excluding_portrait(
            src + (size_t)slot * SAVEGAME_PC34_CHAMPION_BYTE_COUNT,
            slot,
            &state->party->champions[slot]);
        if (portraitIndexesPresent &&
            slot < PC34_DM_FIRESTAFF_PORTRAIT_INDEX_COUNT) {
            state->party->champions[slot].portraitIndex =
                (int)portraitIndexes[slot];
        }
    }
    for (; slot < CHAMPION_MAX_PARTY; ++slot) {
        F0600_CHAMPION_InitEmpty_Compat(&state->party->champions[slot]);
    }
}

static int pc34_event_type_from_timeline_kind(int kind, int aux0)
{
    switch (kind) {
    case TIMELINE_EVENT_DOOR_ANIMATE:
        return DM1_EVENT_DOOR_ANIMATION;
    case TIMELINE_EVENT_DOOR_DESTRUCTION:
        return DM1_EVENT_DOOR_DESTRUCTION;
    case TIMELINE_EVENT_PROJECTILE_MOVE:
        return DM1_EVENT_MOVE_PROJECTILE;
    case TIMELINE_EVENT_EXPLOSION_ADVANCE:
        return DM1_EVENT_EXPLOSION;
    case TIMELINE_EVENT_MAGIC_LIGHT_DECAY:
        return DM1_EVENT_LIGHT;
    case TIMELINE_EVENT_GROUP_GENERATOR:
        return DM1_EVENT_ENABLE_GROUP_GENERATOR;
    case TIMELINE_EVENT_REMOVE_FLUXCAGE:
        return DM1_EVENT_REMOVE_FLUXCAGE;
    case TIMELINE_EVENT_PLAY_SOUND:
        return DM1_EVENT_PLAY_SOUND;
    case TIMELINE_EVENT_WATCHDOG:
        return DM1_EVENT_WATCHDOG;
    case TIMELINE_EVENT_MOVE_GROUP_SILENT:
        return DM1_EVENT_MOVE_GROUP_SILENT;
    case TIMELINE_EVENT_MOVE_GROUP_AUDIBLE:
        return DM1_EVENT_MOVE_GROUP_AUDIBLE;
    case TIMELINE_EVENT_SQUARE_STATE:
    case TIMELINE_EVENT_SENSOR_DELAYED:
        if (aux0 >= DM1_EVENT_CORRIDOR && aux0 <= DM1_EVENT_DOOR) {
            return aux0;
        }
        return DM1_EVENT_DOOR;
    case TIMELINE_EVENT_STATUS_TIMEOUT:
    case TIMELINE_EVENT_SPELL_TICK:
        /* ReDMCSB TIMELINE.C C71/C73/C74/C77/C78/C79 lines
         * 1953-1999 dispatches these as native EVENT type bytes.
         * Firestaff F0412/F0763 keeps richer spell-family tags in
         * aux0 until runtime expiry, so native PC34 export must fold
         * them back to the source event ids instead of falling through
         * to C72 champion shield. */
        switch (aux0) {
        case TIMELINE_AUX_INVISIBILITY:
            return DM1_EVENT_INVISIBILITY;
        case TIMELINE_AUX_THIEVES_EYE:
            return DM1_EVENT_THIEVES_EYE;
        case TIMELINE_AUX_PARTY_SHIELD:
            return DM1_EVENT_PARTY_SHIELD;
        case TIMELINE_AUX_SPELL_SHIELD:
            return DM1_EVENT_SPELLSHIELD;
        case TIMELINE_AUX_FIRESHIELD:
            return DM1_EVENT_FIRESHIELD;
        case TIMELINE_AUX_FOOTPRINTS:
            return DM1_EVENT_FOOTPRINTS;
        default:
            break;
        }
        if (aux0 >= DM1_EVENT_LIGHT && aux0 <= DM1_EVENT_FOOTPRINTS) {
            return aux0;
        }
        return DM1_EVENT_CHAMPION_SHIELD;
    default:
        return DM1_EVENT_NONE;
    }
}

static int pc34_event_type_is_status_timeout(int type)
{
    return type == DM1_EVENT_INVISIBILITY ||
           type == DM1_EVENT_CHAMPION_SHIELD ||
           type == DM1_EVENT_THIEVES_EYE ||
           type == DM1_EVENT_PARTY_SHIELD ||
           type == DM1_EVENT_POISON_CHAMPION ||
           type == DM1_EVENT_SPELLSHIELD ||
           type == DM1_EVENT_FIRESHIELD ||
           type == DM1_EVENT_FOOTPRINTS;
}

static uint16_t pc34_make_thing_ref(int type, int index)
{
    if (type < 0) type = 0;
    if (index < 0) index = 0;
    return (uint16_t)(((type & 0x0f) << 10) | (index & 0x03ff));
}

static int pc34_thing_ref_type(uint16_t thing)
{
    return (int)((thing >> 10) & 0x0f);
}

static int pc34_thing_ref_index(uint16_t thing)
{
    return (int)(thing & 0x03ff);
}

static int pc34_status_event_defense_from_timeline(
    const struct TimelineEvent_Compat* src,
    int type)
{
    if (!src) {
        return 0;
    }
    switch (type) {
    case DM1_EVENT_PARTY_SHIELD:
        if (src->aux0 == TIMELINE_AUX_PARTY_SHIELD) {
            return src->aux4;
        }
        return src->aux1;
    case DM1_EVENT_SPELLSHIELD:
        if (src->aux0 == TIMELINE_AUX_SPELL_SHIELD) {
            return src->aux2;
        }
        return src->aux1;
    case DM1_EVENT_FIRESHIELD:
        if (src->aux0 == TIMELINE_AUX_FIRESHIELD) {
            return src->aux3;
        }
        return src->aux1;
    default:
        return src->aux1;
    }
}

static int timeline_kind_from_pc34_event_type(int type)
{
    switch (type) {
    case DM1_EVENT_DOOR_ANIMATION:
        return TIMELINE_EVENT_DOOR_ANIMATE;
    case DM1_EVENT_DOOR_DESTRUCTION:
        return TIMELINE_EVENT_DOOR_DESTRUCTION;
    case DM1_EVENT_MOVE_PROJECTILE:
    case DM1_EVENT_MOVE_PROJECTILE_IGNORE_IMPACTS:
        return TIMELINE_EVENT_PROJECTILE_MOVE;
    case DM1_EVENT_EXPLOSION:
        return TIMELINE_EVENT_EXPLOSION_ADVANCE;
    case DM1_EVENT_LIGHT:
        return TIMELINE_EVENT_MAGIC_LIGHT_DECAY;
    case DM1_EVENT_ENABLE_GROUP_GENERATOR:
        return TIMELINE_EVENT_GROUP_GENERATOR;
    case DM1_EVENT_REMOVE_FLUXCAGE:
        return TIMELINE_EVENT_REMOVE_FLUXCAGE;
    case DM1_EVENT_PLAY_SOUND:
        return TIMELINE_EVENT_PLAY_SOUND;
    case DM1_EVENT_WATCHDOG:
        return TIMELINE_EVENT_WATCHDOG;
    case DM1_EVENT_MOVE_GROUP_SILENT:
        return TIMELINE_EVENT_MOVE_GROUP_SILENT;
    case DM1_EVENT_MOVE_GROUP_AUDIBLE:
        return TIMELINE_EVENT_MOVE_GROUP_AUDIBLE;
    case DM1_EVENT_CORRIDOR:
    case DM1_EVENT_WALL:
    case DM1_EVENT_FAKEWALL:
    case DM1_EVENT_TELEPORTER:
    case DM1_EVENT_PIT:
    case DM1_EVENT_DOOR:
        return TIMELINE_EVENT_SQUARE_STATE;
    case DM1_EVENT_INVISIBILITY:
    case DM1_EVENT_CHAMPION_SHIELD:
    case DM1_EVENT_THIEVES_EYE:
    case DM1_EVENT_PARTY_SHIELD:
    case DM1_EVENT_POISON_CHAMPION:
    case DM1_EVENT_SPELLSHIELD:
    case DM1_EVENT_FIRESHIELD:
    case DM1_EVENT_FOOTPRINTS:
        return TIMELINE_EVENT_STATUS_TIMEOUT;
    default:
        return TIMELINE_EVENT_INVALID;
    }
}

static uint16_t pc34_event_type_priority(const unsigned char* eventBytes)
{
    return (uint16_t)(((uint16_t)eventBytes[4] << 8) | eventBytes[5]);
}

static int pc34_event_is_before(const unsigned char* events,
                                int indexA,
                                int indexB)
{
    const unsigned char* a = events + (size_t)indexA * PC34_EVENT_BYTE_COUNT;
    const unsigned char* b = events + (size_t)indexB * PC34_EVENT_BYTE_COUNT;
    uint32_t timeA = read_u32_le(a) & 0x00ffffffu;
    uint32_t timeB = read_u32_le(b) & 0x00ffffffu;
    uint16_t tpA;
    uint16_t tpB;

    if (timeA < timeB) return 1;
    if (timeA > timeB) return 0;
    tpA = pc34_event_type_priority(a);
    tpB = pc34_event_type_priority(b);
    if (tpA > tpB) return 1;
    if (tpA < tpB) return 0;
    return indexA <= indexB;
}

static void pc34_sort_timeline_indices(unsigned char* timeline,
                                       const unsigned char* events,
                                       int count)
{
    int i;
    for (i = 1; i < count; ++i) {
        uint16_t value = read_u16_le(timeline + (size_t)i * 2u);
        int j = i - 1;
        while (j >= 0) {
            uint16_t prior = read_u16_le(timeline + (size_t)j * 2u);
            if (pc34_event_is_before(events, prior, value)) {
                break;
            }
            write_u16_le(timeline + (size_t)(j + 1) * 2u, prior);
            --j;
        }
        write_u16_le(timeline + (size_t)(j + 1) * 2u, value);
    }
}

static int pack_events_and_timeline(const struct SaveGame_Compat* state,
                                    unsigned char* events,
                                    int eventsCap,
                                    unsigned char* timeline,
                                    int timelineCap,
                                    int* outEventsLen,
                                    int* outTimelineLen,
                                    int* outEventCount,
                                    int* outFirstUnusedEventIndex,
                                    int* outEventMaximumCount)
{
    int i;
    int count = 0;

    if (outEventsLen) *outEventsLen = 0;
    if (outTimelineLen) *outTimelineLen = 0;
    if (outEventCount) *outEventCount = 0;
    if (outFirstUnusedEventIndex) *outFirstUnusedEventIndex = 0;
    if (outEventMaximumCount) *outEventMaximumCount = 0;
    if (!events || !timeline || eventsCap < 0 || timelineCap < 0) {
        return 0;
    }
    memset(events, 0, (size_t)eventsCap);
    memset(timeline, 0, (size_t)timelineCap);
    if (!state || !state->timeline) {
        return 1;
    }
    if (state->timeline->count < 0 ||
        state->timeline->count > TIMELINE_QUEUE_CAPACITY) {
        return 0;
    }
    for (i = 0; i < state->timeline->count; ++i) {
        const struct TimelineEvent_Compat* src = &state->timeline->events[i];
        int type = pc34_event_type_from_timeline_kind(src->kind, src->aux0);
        unsigned char* dst;
        if (type == DM1_EVENT_NONE) {
            continue;
        }
        if ((count + 1) * PC34_EVENT_BYTE_COUNT > eventsCap ||
            (count + 1) * 2 > timelineCap) {
            return 0;
        }
        dst = events + (size_t)count * PC34_EVENT_BYTE_COUNT;
        /* ReDMCSB DEFS.H EVENT: Map_Time, Type/Priority, B, C.
         * Firestaff's generic timeline keeps effect-like data in aux1 and
         * an optional source priority in aux4. */
        write_u32_le(dst + 0u,
                     (((uint32_t)src->mapIndex & 0xffu) << 24) |
                     (src->fireAtTick & 0x00ffffffu));
        dst[4] = (uint8_t)type;
        dst[5] = (uint8_t)(src->aux4 & 0xff);
        if (pc34_event_type_is_status_timeout(type)) {
            int defense = pc34_status_event_defense_from_timeline(src, type);
            write_u16_le(dst + 6u, (uint16_t)(defense & 0xffff));
        } else if (type == DM1_EVENT_REMOVE_FLUXCAGE) {
            /* ReDMCSB PROJEXPL.C F0224 lines 989-993 stores the
             * fluxcage explosion THING in EVENT.C.Slot while B.Location
             * remains the target square. Firestaff runtime keeps only the
             * ExplosionList slot index in aux0, so native export rebuilds
             * the source C15 thing reference here. */
            dst[6] = (uint8_t)(src->mapX & 0xff);
            dst[7] = (uint8_t)(src->mapY & 0xff);
            write_u16_le(dst + 8u,
                         pc34_make_thing_ref(THING_TYPE_EXPLOSION, src->aux0));
        } else {
            dst[6] = (uint8_t)(src->mapX & 0xff);
            dst[7] = (uint8_t)(src->mapY & 0xff);
            dst[8] = (uint8_t)(src->cell & 0xff);
            dst[9] = (uint8_t)(src->aux1 & 0xff);
        }
        write_u16_le(timeline + (size_t)count * 2u, (uint16_t)count);
        ++count;
    }
    pc34_sort_timeline_indices(timeline, events, count);
    if (outEventsLen) *outEventsLen = count * PC34_EVENT_BYTE_COUNT;
    if (outTimelineLen) *outTimelineLen = count * 2;
    if (outEventCount) *outEventCount = count;
    if (outFirstUnusedEventIndex) *outFirstUnusedEventIndex = count;
    if (outEventMaximumCount) *outEventMaximumCount = count;
    return 1;
}

static void unpack_events_and_timeline(const struct PC34GlobalData* gd,
                                       const unsigned char* events,
                                       int eventsLen,
                                       const unsigned char* timeline,
                                       int timelineLen,
                                       struct TimelineQueue_Compat* out)
{
    int maxEvents;
    int activeCount;
    int i;

    if (!gd || !events || !timeline || !out) {
        return;
    }
    maxEvents = (int)gd->eventMaximumCount;
    activeCount = (int)gd->eventCount;
    if (maxEvents < 0 || maxEvents > TIMELINE_QUEUE_CAPACITY) {
        return;
    }
    if (activeCount < 0) {
        activeCount = 0;
    }
    if (activeCount > maxEvents) {
        activeCount = maxEvents;
    }
    if (eventsLen < maxEvents * PC34_EVENT_BYTE_COUNT ||
        timelineLen < maxEvents * 2) {
        return;
    }
    memset(out, 0, sizeof(*out));
    out->nowTick = gd->gameTime;
    for (i = 0; i < activeCount; ++i) {
        int sourceIndex = (int)read_u16_le(timeline + (size_t)i * 2u);
        const unsigned char* src;
        struct TimelineEvent_Compat* dst;
        int kind;
        uint32_t mapTime;
        if (sourceIndex < 0 || sourceIndex >= maxEvents) {
            continue;
        }
        src = events + (size_t)sourceIndex * PC34_EVENT_BYTE_COUNT;
        kind = timeline_kind_from_pc34_event_type(src[4]);
        if (kind == TIMELINE_EVENT_INVALID || out->count >= TIMELINE_QUEUE_CAPACITY) {
            continue;
        }
        dst = &out->events[out->count++];
        mapTime = read_u32_le(src + 0u);
        dst->kind = kind;
        dst->fireAtTick = mapTime & 0x00ffffffu;
        dst->mapIndex = (int)((mapTime >> 24) & 0xffu);
        if (pc34_event_type_is_status_timeout(src[4u])) {
            dst->mapX = 0;
            dst->mapY = 0;
            dst->aux1 = (int)read_u16_le(src + 6u);
        } else if (src[4u] == DM1_EVENT_REMOVE_FLUXCAGE) {
            uint16_t thing = read_u16_le(src + 8u);
            dst->mapX = src[6u];
            dst->mapY = src[7u];
            dst->cell = EXPLOSION_CELL_CENTERED;
            dst->aux0 = (pc34_thing_ref_type(thing) == THING_TYPE_EXPLOSION)
                ? pc34_thing_ref_index(thing) : (int)thing;
            dst->aux1 = C050_EXPLOSION_FLUXCAGE;
            dst->aux4 = src[5u];
            continue;
        } else {
            dst->mapX = src[6u];
            dst->mapY = src[7u];
            dst->aux1 = src[9u];
        }
        dst->cell = src[8u];
        dst->aux0 = src[4u];
        dst->aux4 = src[5u];
    }
}

static int pc34_dungeon_column_count(
    const struct DungeonDatState_Compat* dungeon)
{
    int total = 0;
    int mapIndex;
    if (dungeon == 0 || dungeon->maps == 0) {
        return 0;
    }
    for (mapIndex = 0; mapIndex < (int)dungeon->header.mapCount; ++mapIndex) {
        total += (int)dungeon->maps[mapIndex].width;
    }
    return total;
}

static int pc34_dungeon_raw_map_bytes(
    const struct DungeonDatState_Compat* dungeon)
{
    int total = 0;
    int mapIndex;
    if (dungeon == 0 || dungeon->maps == 0) {
        return 0;
    }
    if (dungeon->header.rawMapDataByteCount > 0) {
        return (int)dungeon->header.rawMapDataByteCount;
    }
    for (mapIndex = 0; mapIndex < (int)dungeon->header.mapCount; ++mapIndex) {
        total += (int)dungeon->maps[mapIndex].width *
                 (int)dungeon->maps[mapIndex].height;
        total += (int)dungeon->maps[mapIndex].creatureTypeCount;
    }
    return total;
}

static int pc34_dungeon_tail_size(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things)
{
    int total;
    int type;
    if (dungeon == 0 || things == 0 ||
        !dungeon->loaded || !dungeon->tilesLoaded || !things->loaded ||
        dungeon->maps == 0 || dungeon->tiles == 0 ||
        things->squareFirstThings == 0) {
        return 0;
    }
    total = DUNGEON_HEADER_SIZE;
    total += (int)dungeon->header.mapCount * DUNGEON_MAP_DESC_SIZE;
    total += pc34_dungeon_column_count(dungeon) * 2;
    total += (int)dungeon->header.squareFirstThingCount * 2;
    total += (int)dungeon->header.textDataWordCount * 2;
    for (type = 0; type < DUNGEON_THING_TYPE_COUNT; ++type) {
        total += (int)s_thingDataByteCount[type] *
                 (int)dungeon->header.thingCounts[type];
    }
    total += pc34_dungeon_raw_map_bytes(dungeon);
    total += 2;
    return total;
}

static uint16_t pc34_dungeon_checksum_add(uint16_t checksum,
                                          const unsigned char* bytes,
                                          int byteCount)
{
    int i;
    for (i = 0; i < byteCount; ++i) {
        checksum = (uint16_t)(checksum + bytes[i]);
    }
    return checksum;
}

static int pc34_write_dungeon_chunk(unsigned char* dst,
                                    int dstAvail,
                                    const unsigned char* src,
                                    int byteCount,
                                    uint16_t* inOutChecksum)
{
    if (byteCount < 0 || dst == 0 || inOutChecksum == 0) {
        return -1;
    }
    if (byteCount == 0) {
        return 0;
    }
    if (src == 0 || dstAvail < byteCount) {
        return -1;
    }
    memcpy(dst, src, (size_t)byteCount);
    *inOutChecksum = pc34_dungeon_checksum_add(*inOutChecksum,
                                               dst,
                                               byteCount);
    return byteCount;
}

static void pc34_pack_dungeon_header(unsigned char out[DUNGEON_HEADER_SIZE],
                                     const struct DungeonHeader_Compat* h)
{
    int type;
    memset(out, 0, DUNGEON_HEADER_SIZE);
    write_u16_le(out + 0u, h->ornamentRandomSeed);
    write_u16_le(out + 2u, h->rawMapDataByteCount);
    out[4u] = h->mapCount;
    out[5u] = h->unreferenced;
    write_u16_le(out + 6u, h->textDataWordCount);
    write_u16_le(out + 8u, h->initialPartyLocation);
    write_u16_le(out + 10u, h->squareFirstThingCount);
    for (type = 0; type < DUNGEON_THING_TYPE_COUNT; ++type) {
        write_u16_le(out + 12u + (size_t)type * 2u,
                     h->thingCounts[type]);
    }
}

static uint16_t pc34_pack_map_bitfield_a(
    const struct DungeonMapDesc_Compat* map)
{
    unsigned int width = map->width > 0 ? (unsigned int)map->width - 1u : 0u;
    unsigned int height = map->height > 0 ? (unsigned int)map->height - 1u : 0u;
    return (uint16_t)(((unsigned int)map->level & 0x3fu) |
                      ((width & 0x1fu) << 6) |
                      ((height & 0x1fu) << 11));
}

static void pc34_pack_map_desc(unsigned char out[DUNGEON_MAP_DESC_SIZE],
                               const struct DungeonMapDesc_Compat* map)
{
    memset(out, 0, DUNGEON_MAP_DESC_SIZE);
    write_u16_le(out + 0u, map->rawMapDataByteOffset);
    write_u16_le(out + 2u, map->aUnreferenced);
    write_u16_le(out + 4u, map->bUnreferenced);
    out[6u] = map->offsetMapX;
    out[7u] = map->offsetMapY;
    write_u16_le(out + 8u, pc34_pack_map_bitfield_a(map));
    write_u16_le(out + 10u, map->rawBitfieldB);
    write_u16_le(out + 12u, map->rawBitfieldC);
    write_u16_le(out + 14u, map->rawBitfieldD);
}

static int pc34_thing_decoded_count(const struct DungeonThings_Compat* things,
                                    int type)
{
    switch (type) {
    case THING_TYPE_DOOR:       return things->doorCount;
    case THING_TYPE_TELEPORTER: return things->teleporterCount;
    case THING_TYPE_TEXTSTRING: return things->textStringCount;
    case THING_TYPE_SENSOR:     return things->sensorCount;
    case THING_TYPE_GROUP:      return things->groupCount;
    case THING_TYPE_WEAPON:     return things->weaponCount;
    case THING_TYPE_ARMOUR:     return things->armourCount;
    case THING_TYPE_SCROLL:     return things->scrollCount;
    case THING_TYPE_POTION:     return things->potionCount;
    case THING_TYPE_CONTAINER:  return things->containerCount;
    case THING_TYPE_JUNK:       return things->junkCount;
    case THING_TYPE_PROJECTILE: return things->projectileCount;
    case THING_TYPE_EXPLOSION:  return things->explosionCount;
    default:                    return 0;
    }
}

static int pc34_thing_has_decoded_array(const struct DungeonThings_Compat* things,
                                        int type)
{
    switch (type) {
    case THING_TYPE_DOOR:       return things->doors != 0;
    case THING_TYPE_TELEPORTER: return things->teleporters != 0;
    case THING_TYPE_TEXTSTRING: return things->textStrings != 0;
    case THING_TYPE_SENSOR:     return things->sensors != 0;
    case THING_TYPE_GROUP:      return things->groups != 0;
    case THING_TYPE_WEAPON:     return things->weapons != 0;
    case THING_TYPE_ARMOUR:     return things->armours != 0;
    case THING_TYPE_SCROLL:     return things->scrolls != 0;
    case THING_TYPE_POTION:     return things->potions != 0;
    case THING_TYPE_CONTAINER:  return things->containers != 0;
    case THING_TYPE_JUNK:       return things->junks != 0;
    case THING_TYPE_PROJECTILE: return things->projectiles != 0;
    case THING_TYPE_EXPLOSION:  return things->explosions != 0;
    default:                    return 0;
    }
}

static int pc34_can_pack_decoded_things(const struct DungeonThings_Compat* things,
                                        int type,
                                        int expectedCount)
{
    if (expectedCount <= 0) {
        return 0;
    }
    return pc34_thing_has_decoded_array(things, type) &&
           pc34_thing_decoded_count(things, type) == expectedCount;
}

static void pc34_pack_decoded_thing_slot(unsigned char* out,
                                         const struct DungeonThings_Compat* things,
                                         int type,
                                         int index)
{
    switch (type) {
    case THING_TYPE_DOOR: {
        const struct DungeonDoor_Compat* d = &things->doors[index];
        uint16_t bf = read_u16_le(out + 2u);
        write_u16_le(out + 0u, d->next);
        bf = (uint16_t)((bf & 0xfe00u) |
             ((uint16_t)(d->type & 0x01u)) |
             ((uint16_t)(d->ornamentOrdinal & 0x0fu) << 1) |
             ((uint16_t)(d->vertical & 0x01u) << 5) |
             ((uint16_t)(d->button & 0x01u) << 6) |
             ((uint16_t)(d->magicDestructible & 0x01u) << 7) |
             ((uint16_t)(d->meleeDestructible & 0x01u) << 8));
        write_u16_le(out + 2u, bf);
        break;
    }
    case THING_TYPE_TELEPORTER: {
        const struct DungeonTeleporter_Compat* tp = &things->teleporters[index];
        uint16_t bf2 = read_u16_le(out + 4u);
        write_u16_le(out + 0u, tp->next);
        write_u16_le(out + 2u,
             (uint16_t)(((uint16_t)(tp->targetMapX & 0x1fu)) |
             ((uint16_t)(tp->targetMapY & 0x1fu) << 5) |
             ((uint16_t)(tp->rotation & 0x03u) << 10) |
             ((uint16_t)(tp->absoluteRotation & 0x01u) << 12) |
             ((uint16_t)(tp->scope & 0x03u) << 13) |
             ((uint16_t)(tp->audible & 0x01u) << 15)));
        bf2 = (uint16_t)((bf2 & 0x00ffu) |
                         ((uint16_t)tp->targetMapIndex << 8));
        write_u16_le(out + 4u, bf2);
        break;
    }
    case THING_TYPE_TEXTSTRING: {
        const struct DungeonTextString_Compat* ts = &things->textStrings[index];
        uint16_t bf = read_u16_le(out + 2u);
        write_u16_le(out + 0u, ts->next);
        bf = (uint16_t)((bf & 0x0006u) |
             ((uint16_t)(ts->visible & 0x01u)) |
             ((uint16_t)(ts->textDataWordOffset & 0x1fffu) << 3));
        write_u16_le(out + 2u, bf);
        break;
    }
    case THING_TYPE_SENSOR: {
        const struct DungeonSensor_Compat* s = &things->sensors[index];
        uint16_t bf1 = read_u16_le(out + 4u);
        uint16_t bf2 = read_u16_le(out + 6u);
        write_u16_le(out + 0u, s->next);
        write_u16_le(out + 2u,
             (uint16_t)(((uint16_t)(s->sensorType & 0x7fu)) |
             ((uint16_t)(s->sensorData & 0x01ffu) << 7)));
        bf1 = (uint16_t)((bf1 & 0x0003u) |
             ((uint16_t)(s->onceOnly & 0x01u) << 2) |
             ((uint16_t)(s->effect & 0x03u) << 3) |
             ((uint16_t)(s->revertEffect & 0x01u) << 5) |
             ((uint16_t)(s->audible & 0x01u) << 6) |
             ((uint16_t)(s->value & 0x0fu) << 7) |
             ((uint16_t)(s->localEffect & 0x01u) << 11) |
             ((uint16_t)(s->ornamentOrdinal & 0x0fu) << 12));
        write_u16_le(out + 4u, bf1);
        bf2 = (uint16_t)((bf2 & 0xf000u) |
             ((uint16_t)(s->localMultiple & 0x0fffu)));
        bf2 = (uint16_t)((bf2 & 0x000fu) |
             ((uint16_t)(s->targetCell & 0x03u) << 4) |
             ((uint16_t)(s->targetMapX & 0x1fu) << 6) |
             ((uint16_t)(s->targetMapY & 0x1fu) << 11));
        write_u16_le(out + 6u, bf2);
        break;
    }
    case THING_TYPE_GROUP: {
        const struct DungeonGroup_Compat* g = &things->groups[index];
        uint16_t bf = read_u16_le(out + 14u);
        write_u16_le(out + 0u, g->next);
        write_u16_le(out + 2u, g->slot);
        out[4u] = g->creatureType;
        out[5u] = g->cells;
        write_u16_le(out + 6u, g->health[0]);
        write_u16_le(out + 8u, g->health[1]);
        write_u16_le(out + 10u, g->health[2]);
        write_u16_le(out + 12u, g->health[3]);
        bf = (uint16_t)((bf & 0xf890u) |
             ((uint16_t)(g->behavior & 0x0fu)) |
             ((uint16_t)(g->count & 0x03u) << 5) |
             ((uint16_t)(g->direction & 0x03u) << 8) |
             ((uint16_t)(g->doNotDiscard & 0x01u) << 10));
        write_u16_le(out + 14u, bf);
        break;
    }
    case THING_TYPE_WEAPON: {
        const struct DungeonWeapon_Compat* w = &things->weapons[index];
        write_u16_le(out + 0u, w->next);
        write_u16_le(out + 2u,
             (uint16_t)(((uint16_t)(w->type & 0x7fu)) |
             ((uint16_t)(w->doNotDiscard & 0x01u) << 7) |
             ((uint16_t)(w->cursed & 0x01u) << 8) |
             ((uint16_t)(w->poisoned & 0x01u) << 9) |
             ((uint16_t)(w->chargeCount & 0x0fu) << 10) |
             ((uint16_t)(w->broken & 0x01u) << 14) |
             ((uint16_t)(w->lit & 0x01u) << 15)));
        break;
    }
    case THING_TYPE_ARMOUR: {
        const struct DungeonArmour_Compat* a = &things->armours[index];
        uint16_t bf = read_u16_le(out + 2u);
        write_u16_le(out + 0u, a->next);
        bf = (uint16_t)((bf & 0xc000u) |
             ((uint16_t)(a->type & 0x7fu)) |
             ((uint16_t)(a->doNotDiscard & 0x01u) << 7) |
             ((uint16_t)(a->cursed & 0x01u) << 8) |
             ((uint16_t)(a->chargeCount & 0x0fu) << 9) |
             ((uint16_t)(a->broken & 0x01u) << 13));
        write_u16_le(out + 2u, bf);
        break;
    }
    case THING_TYPE_SCROLL: {
        const struct DungeonScroll_Compat* s = &things->scrolls[index];
        write_u16_le(out + 0u, s->next);
        write_u16_le(out + 2u,
             (uint16_t)(((uint16_t)(s->textStringThingIndex & 0x03ffu)) |
             ((uint16_t)(s->closed & 0x3fu) << 10)));
        break;
    }
    case THING_TYPE_POTION: {
        const struct DungeonPotion_Compat* p = &things->potions[index];
        write_u16_le(out + 0u, p->next);
        write_u16_le(out + 2u,
             (uint16_t)(((uint16_t)p->power) |
             ((uint16_t)(p->type & 0x7fu) << 8) |
             ((uint16_t)(p->doNotDiscard & 0x01u) << 15)));
        break;
    }
    case THING_TYPE_CONTAINER: {
        const struct DungeonContainer_Compat* c = &things->containers[index];
        uint16_t bf = read_u16_le(out + 4u);
        write_u16_le(out + 0u, c->next);
        write_u16_le(out + 2u, c->slot);
        bf = (uint16_t)((bf & 0xfff9u) |
                        ((uint16_t)(c->type & 0x03u) << 1));
        write_u16_le(out + 4u, bf);
        break;
    }
    case THING_TYPE_JUNK: {
        const struct DungeonJunk_Compat* j = &things->junks[index];
        uint16_t bf = read_u16_le(out + 2u);
        write_u16_le(out + 0u, j->next);
        bf = (uint16_t)((bf & 0x3e00u) |
             ((uint16_t)(j->type & 0x7fu)) |
             ((uint16_t)(j->doNotDiscard & 0x01u) << 7) |
             ((uint16_t)(j->cursed & 0x01u) << 8) |
             ((uint16_t)(j->chargeCount & 0x03u) << 14));
        write_u16_le(out + 2u, bf);
        break;
    }
    case THING_TYPE_PROJECTILE: {
        const struct DungeonProjectile_Compat* p = &things->projectiles[index];
        write_u16_le(out + 0u, p->next);
        write_u16_le(out + 2u, p->slot);
        out[4u] = p->kineticEnergy;
        out[5u] = p->attack;
        write_u16_le(out + 6u, p->eventIndex);
        break;
    }
    case THING_TYPE_EXPLOSION: {
        const struct DungeonExplosion_Compat* e = &things->explosions[index];
        write_u16_le(out + 0u, e->next);
        write_u16_le(out + 2u,
             (uint16_t)(((uint16_t)(e->type & 0x7fu)) |
             ((uint16_t)(e->centered & 0x01u) << 7) |
             ((uint16_t)e->attack << 8)));
        break;
    }
    default:
        break;
    }
}

static int pc34_write_dungeon_thing_chunk(
    unsigned char* dst,
    int dstAvail,
    const struct DungeonThings_Compat* things,
    int type,
    int expectedCount,
    uint16_t* inOutChecksum)
{
    int byteCount = (int)s_thingDataByteCount[type] * expectedCount;
    if (byteCount < 0) {
        return -1;
    }
    if (byteCount == 0) {
        return 0;
    }
    if (pc34_can_pack_decoded_things(things, type, expectedCount)) {
        unsigned char* packed = (unsigned char*)calloc((size_t)byteCount, 1u);
        int i;
        int slotBytes = (int)s_thingDataByteCount[type];
        int rc;
        if (!packed) return -1;
        if (things->rawThingData[type]) {
            memcpy(packed, things->rawThingData[type], (size_t)byteCount);
        }
        for (i = 0; i < expectedCount; ++i) {
            pc34_pack_decoded_thing_slot(packed + (size_t)i * slotBytes,
                                         things, type, i);
        }
        rc = pc34_write_dungeon_chunk(dst, dstAvail, packed, byteCount,
                                      inOutChecksum);
        free(packed);
        return rc;
    }
    return pc34_write_dungeon_chunk(dst, dstAvail, things->rawThingData[type],
                                    byteCount, inOutChecksum);
}

static int pc34_write_dungeon_tail(
    unsigned char* dst,
    int dstAvail,
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things)
{
    int cursor = 0;
    int mapIndex;
    int type;
    int cumulative = 0;
    int tailSize;
    uint16_t checksum = 0;
    unsigned char scratch[DUNGEON_HEADER_SIZE];

    if (dst == 0 || dstAvail < 0) {
        return -1;
    }
    tailSize = pc34_dungeon_tail_size(dungeon, things);
    if (tailSize == 0) {
        return 0;
    }
    if (dstAvail < tailSize) {
        return -1;
    }

    /* ReDMCSB LOADSAVE.C F0433 lines ~1661-1682 writes these
     * sections with F0422 byte-sum checksum in this exact order. */
    pc34_pack_dungeon_header(scratch, &dungeon->header);
    if (pc34_write_dungeon_chunk(dst + cursor, dstAvail - cursor,
                                 scratch, DUNGEON_HEADER_SIZE,
                                 &checksum) < 0) return -1;
    cursor += DUNGEON_HEADER_SIZE;

    for (mapIndex = 0; mapIndex < (int)dungeon->header.mapCount; ++mapIndex) {
        pc34_pack_map_desc(scratch, &dungeon->maps[mapIndex]);
        if (pc34_write_dungeon_chunk(dst + cursor, dstAvail - cursor,
                                     scratch, DUNGEON_MAP_DESC_SIZE,
                                     &checksum) < 0) return -1;
        cursor += DUNGEON_MAP_DESC_SIZE;
    }

    for (mapIndex = 0; mapIndex < (int)dungeon->header.mapCount; ++mapIndex) {
        const struct DungeonMapDesc_Compat* map = &dungeon->maps[mapIndex];
        int x;
        if (!dungeon->tiles[mapIndex].squareData) {
            return -1;
        }
        for (x = 0; x < (int)map->width; ++x) {
            int y;
            unsigned char word[2];
            write_i16_le(word, (int16_t)cumulative);
            if (pc34_write_dungeon_chunk(dst + cursor, dstAvail - cursor,
                                         word, 2, &checksum) < 0) return -1;
            cursor += 2;
            for (y = 0; y < (int)map->height; ++y) {
                unsigned char square =
                    dungeon->tiles[mapIndex].squareData[
                        x * (int)map->height + y];
                if ((square & DUNGEON_SQUARE_MASK_THING_LIST) != 0) {
                    ++cumulative;
                }
            }
        }
    }

    if (pc34_write_dungeon_chunk(dst + cursor, dstAvail - cursor,
                                 (const unsigned char*)things->squareFirstThings,
                                 (int)dungeon->header.squareFirstThingCount * 2,
                                 &checksum) < 0) return -1;
    cursor += (int)dungeon->header.squareFirstThingCount * 2;

    if (pc34_write_dungeon_chunk(dst + cursor, dstAvail - cursor,
                                 (const unsigned char*)things->textData,
                                 (int)dungeon->header.textDataWordCount * 2,
                                 &checksum) < 0) return -1;
    cursor += (int)dungeon->header.textDataWordCount * 2;

    for (type = 0; type < DUNGEON_THING_TYPE_COUNT; ++type) {
        int bytes = (int)s_thingDataByteCount[type] *
                    (int)dungeon->header.thingCounts[type];
        if (pc34_write_dungeon_thing_chunk(
                dst + cursor, dstAvail - cursor, things, type,
                (int)dungeon->header.thingCounts[type], &checksum) < 0) {
            return -1;
        }
        cursor += bytes;
    }

    {
        int rawMapBytes = pc34_dungeon_raw_map_bytes(dungeon);
        unsigned char* rawMap = 0;
        if (rawMapBytes < 0) {
            return -1;
        }
        if (rawMapBytes > 0) {
            rawMap = (unsigned char*)calloc((size_t)rawMapBytes, 1u);
            if (!rawMap) return -1;
        }
        for (mapIndex = 0; mapIndex < (int)dungeon->header.mapCount; ++mapIndex) {
            const struct DungeonMapDesc_Compat* map = &dungeon->maps[mapIndex];
            int squares = (int)map->width * (int)map->height;
            int offset = (int)map->rawMapDataByteOffset;
            if (!dungeon->tiles[mapIndex].squareData ||
                squares < 0 ||
                offset < 0 ||
                offset + squares + (int)map->creatureTypeCount > rawMapBytes) {
                free(rawMap);
                return -1;
            }
            if (squares > 0) {
                memcpy(rawMap + offset,
                       dungeon->tiles[mapIndex].squareData,
                       (size_t)squares);
            }
            if (map->creatureTypeCount > 0) {
                memcpy(rawMap + offset + squares,
                       map->allowedCreatureTypes,
                       (size_t)map->creatureTypeCount);
            }
        }
        if (pc34_write_dungeon_chunk(dst + cursor, dstAvail - cursor,
                                     rawMap, rawMapBytes, &checksum) < 0) {
            free(rawMap);
            return -1;
        }
        cursor += rawMapBytes;
        free(rawMap);
    }

    if (cursor + 2 > dstAvail) {
        return -1;
    }
    write_u16_le(dst + cursor, checksum);
    cursor += 2;
    return cursor;
}

static int export_pc34_core(
    const struct SaveGame_Compat* state,
    const unsigned char* activeGroupsPart,
    int activeGroupsLen,
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    uint32_t gameTime,
    int currentActiveGroupCount,
    int maximumActiveGroupCount,
    uint32_t gameID,
    unsigned char* outBuf,
    int outBufSize,
    int* outBytesWritten)
{
    unsigned char* p;
    int cursor = 0;
    uint32_t prngSeed;
    uint16_t keys[SAVEGAME_PC34_DM_KEYS_COUNT];
    uint16_t checksums[SAVEGAME_PC34_DM_CHECKSUMS_COUNT];
    unsigned char partBuf[SAVEGAME_PC34_TIMELINE_BYTE_COUNT];
    static const unsigned char emptyPart[2] = { 0u, 0u };
    unsigned char eventsPart[PC34_EXPORT_EVENTS_PART_CAP];
    unsigned char timelinePart[PC34_EXPORT_TIMELINE_PART_CAP];
    int partLen;
    int eventsLen = 0;
    int timelineLen = 0;
    int eventCount = 0;
    int firstUnusedEventIndex = 0;
    int eventMaximumCount = 0;
    int dungeonTailLen = 0;
    int i;
    int totalNeeded;

    if (outBytesWritten != 0) *outBytesWritten = 0;
    if (state == 0 || outBuf == 0) return SAVEGAME_PC34_ERROR_NULL_ARG;
    if (state->party == 0) return SAVEGAME_PC34_ERROR_NULL_ARG;

    /* Per-export pseudo-random seed: derive from gameID so
     * re-exports of the same game yield identical byte streams
     * (useful for the round-trip test). The original ReDMCSB
     * seeds with the runtime PRNG; we use a fixed mix so the
     * export is stable. */
    prngSeed = gameID ^ 0xC0DECAFEu;
    if (prngSeed == 0) prngSeed = 0xDEADBEEFu;

    if (!pack_events_and_timeline(state,
                                  eventsPart, (int)sizeof(eventsPart),
                                  timelinePart, (int)sizeof(timelinePart),
                                  &eventsLen, &timelineLen,
                                  &eventCount, &firstUnusedEventIndex,
                                  &eventMaximumCount)) {
        return SAVEGAME_PC34_ERROR_INTERNAL;
    }

    /* Pre-compute the exact PC34 payload size to fail fast on small
     * buffers. Header (512) + 5 length prefixes + five save parts. */
    dungeonTailLen = pc34_dungeon_tail_size(dungeon, things);
    totalNeeded = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE
                  + 5 * 2
                  + SAVEGAME_PC34_GLOBAL_DATA_BYTE_COUNT
                  + activeGroupsLen
                  + SAVEGAME_PC34_PARTY_PART_BYTE_COUNT
                  + eventsLen
                  + timelineLen
                  + CHAMPION_MAX_PARTY * CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT
                  + dungeonTailLen;
    if (outBufSize < totalNeeded)
        return SAVEGAME_PC34_ERROR_BUFFER_TOO_SMALL;
    if (outBufSize > (int)SAVEGAME_PC34_MAX_FILE_SIZE)
        return SAVEGAME_PC34_ERROR_BAD_SIZE;

    /* Per-part keys: original PC 3.4 derives these from the live
     * PRNG. We use a deterministic mix so the same export
     * produces the same file. */
    for (i = 0; i < SAVEGAME_PC34_DM_KEYS_COUNT; ++i) {
        keys[i] = 0;
        checksums[i] = 0;
    }
    for (i = 0; i < SAVEGAME_PC34_PART_COUNT; ++i) {
        prngSeed = prngSeed * 1103515245u + 12345u;
        keys[i] = (uint16_t)((prngSeed >> 16) & 0xFFFFu);
    }

    /* Header (write after we know all part checksums). */
    /* Reserve the header bytes; fill at the end. */
    p = outBuf;
    cursor = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE;

    /* Part 0: GLOBAL_DATA. */
    pack_global_data(partBuf, (int)sizeof(partBuf), state,
                     gameTime,
                     currentActiveGroupCount, maximumActiveGroupCount,
                     eventCount, firstUnusedEventIndex, eventMaximumCount);
    if (pc34_write_part(p + cursor, outBufSize - cursor,
                        partBuf,
                        SAVEGAME_PC34_GLOBAL_DATA_BYTE_COUNT,
                        keys[SAVEGAME_PC34_PART_GLOBAL_DATA]) != 0)
        return SAVEGAME_PC34_ERROR_INTERNAL;
    checksums[SAVEGAME_PC34_PART_GLOBAL_DATA] =
        pc34_f0417_bytes(
            partBuf,
            SAVEGAME_PC34_GLOBAL_DATA_BYTE_COUNT / 2,
            keys[SAVEGAME_PC34_PART_GLOBAL_DATA]);
    cursor += 2 + SAVEGAME_PC34_GLOBAL_DATA_BYTE_COUNT;

    /* Part 1: ACTIVE_GROUP. F0795 passes a zero-length part because
     * SaveGame_Compat has no creatureAI slice; F0802 passes ReDMCSB
     * 16-byte ACTIVE_GROUP records packed from GameWorld_Compat. */
    if (pc34_write_part(p + cursor, outBufSize - cursor,
                        activeGroupsPart ? activeGroupsPart : emptyPart,
                        activeGroupsLen,
                        keys[SAVEGAME_PC34_PART_ACTIVE_GROUP]) != 0)
        return SAVEGAME_PC34_ERROR_INTERNAL;
    if (activeGroupsLen > 0) {
        memcpy(partBuf, activeGroupsPart, (size_t)activeGroupsLen);
    } else {
        memset(partBuf, 0, 2u);
    }
    checksums[SAVEGAME_PC34_PART_ACTIVE_GROUP] =
        pc34_f0417_bytes(
            partBuf,
            activeGroupsLen / 2,
            keys[SAVEGAME_PC34_PART_ACTIVE_GROUP]);
    cursor += 2 + activeGroupsLen;

    /* Part 2: PARTY (4×96-byte block). */
    partLen = pack_party(partBuf, (int)sizeof(partBuf), state);
    if (partLen < 0) return SAVEGAME_PC34_ERROR_INTERNAL;
    if (pc34_write_part(p + cursor, outBufSize - cursor,
                        partBuf, partLen,
                        keys[SAVEGAME_PC34_PART_PARTY]) != 0)
        return SAVEGAME_PC34_ERROR_INTERNAL;
    checksums[SAVEGAME_PC34_PART_PARTY] =
        pc34_f0417_bytes(
            partBuf, partLen / 2,
            keys[SAVEGAME_PC34_PART_PARTY]);
    cursor += 2 + partLen;

    /* Part 3: EVENTS. ReDMCSB LOADSAVE.C F0433 writes
     * EventMaximumCount * sizeof(EVENT); PC 3.4 EVENT is 10 bytes. */
    if (pc34_write_part(p + cursor, outBufSize - cursor,
                        eventsPart,
                        eventsLen,
                        keys[SAVEGAME_PC34_PART_EVENTS]) != 0)
        return SAVEGAME_PC34_ERROR_INTERNAL;
    checksums[SAVEGAME_PC34_PART_EVENTS] =
        pc34_f0417_bytes(
            eventsPart,
            eventsLen / 2,
            keys[SAVEGAME_PC34_PART_EVENTS]);
    cursor += 2 + eventsLen;

    /* Part 4: TIMELINE. ReDMCSB LOADSAVE.C F0433 writes
     * EventMaximumCount * sizeof(int16_t) heap indices. */
    if (pc34_write_part(p + cursor, outBufSize - cursor,
                        timelinePart,
                        timelineLen,
                        keys[SAVEGAME_PC34_PART_TIMELINE]) != 0)
        return SAVEGAME_PC34_ERROR_INTERNAL;
    checksums[SAVEGAME_PC34_PART_TIMELINE] =
        pc34_f0417_bytes(
            timelinePart,
            timelineLen / 2,
            keys[SAVEGAME_PC34_PART_TIMELINE]);
    cursor += 2 + timelineLen;

    partLen = pc34_write_external_portraits(p + cursor,
                                            outBufSize - cursor,
                                            state);
    if (partLen < 0) return SAVEGAME_PC34_ERROR_BUFFER_TOO_SMALL;
    cursor += partLen;

    partLen = pc34_write_dungeon_tail(p + cursor,
                                      outBufSize - cursor,
                                      dungeon,
                                      things);
    if (partLen < 0) return SAVEGAME_PC34_ERROR_BUFFER_TOO_SMALL;
    cursor += partLen;

    /* Header. The LSV-02 manifest is stamped into
     * AdditionalData[0..15] by pc34_write_header so the
     * exported file is per-game-tagged as DM1 by default
     * (callers can override gameCode via the new
     * F0795_SAVEGAME_ExportPC34_Compat_For_Game variant;
     * this entry point is the DM1 path). */
    pc34_write_header(p, SAVEGAME_PC34_DM_SAVE_HEADER_SIZE,
                      gameID, keys, checksums, prngSeed,
                      SAVEGAME_PC34_GAME_CODE_DM1, state);

    if (outBytesWritten != 0) *outBytesWritten = cursor;
    return SAVEGAME_PC34_OK;
}

/* ==========================================================
 *  F0795_SAVEGAME_ExportPC34_Compat
 * ========================================================== */

int F0795_SAVEGAME_ExportPC34_Compat(
    const struct SaveGame_Compat* state,
    uint32_t gameID,
    unsigned char* outBuf,
    int outBufSize,
    int* outBytesWritten)
{
    return export_pc34_core(state,
                            0, 0,
                            0, 0,
                            0u,
                            0, 0,
                            gameID,
                            outBuf, outBufSize, outBytesWritten);
}

int F0802_SAVEGAME_ExportPC34FromWorld_Compat(
    const struct GameWorld_Compat* world,
    uint32_t gameID,
    unsigned char* outBuf,
    int outBufSize,
    int* outBytesWritten)
{
    struct SaveGame_Compat state;
    unsigned char activeGroupsPart[PC34_EXPORT_ACTIVE_GROUP_PART_CAP];
    int activeGroupsLen = 0;
    int currentActiveGroupCount = 0;
    int maximumActiveGroupCount = 0;

    if (outBytesWritten != 0) *outBytesWritten = 0;
    if (world == 0 || outBuf == 0) {
        return SAVEGAME_PC34_ERROR_NULL_ARG;
    }
    if (!pack_active_groups_from_world(world,
                                       activeGroupsPart,
                                       (int)sizeof(activeGroupsPart),
                                       &activeGroupsLen,
                                       &currentActiveGroupCount,
                                       &maximumActiveGroupCount)) {
        return SAVEGAME_PC34_ERROR_INTERNAL;
    }

    memset(&state, 0, sizeof(state));
    state.header = world->saveHeader;
    state.party = (struct PartyState_Compat*)&world->party;
    state.pendingSensorEffects =
        (struct SensorEffectList_Compat*)&world->pendingSensorEffects;
    state.timeline = (struct TimelineQueue_Compat*)&world->timeline;
    state.magic = (struct MagicState_Compat*)&world->magic;
    state.mutations =
        (struct DungeonMutationList_Compat*)&world->dungeonMutations;

    return export_pc34_core(&state,
                            activeGroupsPart,
                            activeGroupsLen,
                            world->dungeon,
                            world->things,
                            world->gameTick,
                            currentActiveGroupCount,
                            maximumActiveGroupCount,
                            gameID,
                            outBuf, outBufSize, outBytesWritten);
}

/* ==========================================================
 *  F0796_SAVEGAME_ImportPC34_Compat
 * ========================================================== */

int F0796_SAVEGAME_ImportPC34_Compat(
    const unsigned char* buf,
    int bufSize,
    struct SaveGame_Compat* outState,
    int strictChecksums)
{
    uint16_t keys[SAVEGAME_PC34_DM_KEYS_COUNT];
    uint16_t checksums[SAVEGAME_PC34_DM_CHECKSUMS_COUNT];
    uint16_t formatID = 0;
    uint16_t platform = 0;
    uint16_t dungeonID = 0;
    uint32_t gameID = 0;
    uint16_t manifestVersion = 0;
    uint16_t manifestGameCode = 0;
    int manifestPresent = 0;
    unsigned char portraitIndexes[PC34_DM_FIRESTAFF_PORTRAIT_INDEX_COUNT];
    int portraitIndexesPresent = 0;
    int cursor;
    unsigned char partBuf[SAVEGAME_PC34_TIMELINE_BYTE_COUNT];
    unsigned char eventsPart[PC34_EXPORT_EVENTS_PART_CAP];
    unsigned char timelinePart[PC34_EXPORT_TIMELINE_PART_CAP];
    struct PC34GlobalData globalData;
    int partLen;
    int eventsLen = 0;
    int timelineLen = 0;
    uint16_t actualChecksum = 0;
    if (buf == 0 || outState == 0) return SAVEGAME_PC34_ERROR_NULL_ARG;
    if (bufSize < SAVEGAME_PC34_DM_SAVE_HEADER_SIZE)
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    if (bufSize > (int)SAVEGAME_PC34_MAX_FILE_SIZE)
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    memset(&globalData, 0, sizeof(globalData));

    if (pc34_read_header(buf, bufSize, &gameID, keys, checksums,
                         &formatID, &platform, &dungeonID,
                         &manifestVersion, &manifestGameCode,
                         &manifestPresent,
                         portraitIndexes,
                         &portraitIndexesPresent) != 0) {
        return SAVEGAME_PC34_ERROR_BAD_MAGIC;
    }
    if (formatID != SAVEGAME_PC34_FORMAT_DUNGEON_MASTER_PC &&
        formatID != 0x01 /* tolerate the Atari ST 1.x format too */) {
        return SAVEGAME_PC34_ERROR_UNSUPPORTED;
    }
    /* LSV-02 per-game gate: if a manifest is present and the
     * file claims to be a different game, refuse to import.
     * (Vanilla PC 3.4 files without a manifest are still
     * accepted; F0800 is the strict per-game gate.) */
    if (manifestPresent && manifestGameCode != SAVEGAME_PC34_GAME_CODE_DM1) {
        return SAVEGAME_PC34_ERROR_BAD_MAGIC;
    }

    cursor = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE;

    /* Part 0: GLOBAL_DATA. */
    memset(partBuf, 0, sizeof(partBuf));
    partLen = pc34_read_part(buf, bufSize, &cursor, partBuf,
                             (int)sizeof(partBuf),
                             keys[SAVEGAME_PC34_PART_GLOBAL_DATA],
                             &actualChecksum);
    if (partLen < 0) return SAVEGAME_PC34_ERROR_BAD_SIZE;
    if (strictChecksums &&
        actualChecksum != checksums[SAVEGAME_PC34_PART_GLOBAL_DATA]) {
        return SAVEGAME_PC34_ERROR_BAD_CHECKSUM;
    }
    if (partLen == SAVEGAME_PC34_GLOBAL_DATA_BYTE_COUNT) {
        unpack_global_data(partBuf, outState, &globalData);
    }

    /* Part 1: ACTIVE_GROUP (skip; LSV-01 v1 does not restore). */
    partLen = pc34_read_part(buf, bufSize, &cursor, partBuf,
                             (int)sizeof(partBuf),
                             keys[SAVEGAME_PC34_PART_ACTIVE_GROUP],
                             &actualChecksum);
    if (partLen < 0) return SAVEGAME_PC34_ERROR_BAD_SIZE;
    if (strictChecksums &&
        actualChecksum != checksums[SAVEGAME_PC34_PART_ACTIVE_GROUP]) {
        return SAVEGAME_PC34_ERROR_BAD_CHECKSUM;
    }

    /* Part 2: PARTY. */
    partLen = pc34_read_part(buf, bufSize, &cursor, partBuf,
                             (int)sizeof(partBuf),
                             keys[SAVEGAME_PC34_PART_PARTY],
                             &actualChecksum);
    if (partLen < 0) return SAVEGAME_PC34_ERROR_BAD_SIZE;
    if (strictChecksums &&
        actualChecksum != checksums[SAVEGAME_PC34_PART_PARTY]) {
        return SAVEGAME_PC34_ERROR_BAD_CHECKSUM;
    }
    unpack_party(partBuf, partLen, outState,
                 portraitIndexes, portraitIndexesPresent);

    /* Part 3: EVENTS. */
    partLen = pc34_read_part(buf, bufSize, &cursor, partBuf,
                             (int)sizeof(partBuf),
                             keys[SAVEGAME_PC34_PART_EVENTS],
                             &actualChecksum);
    if (partLen < 0) return SAVEGAME_PC34_ERROR_BAD_SIZE;
    if (strictChecksums &&
        actualChecksum != checksums[SAVEGAME_PC34_PART_EVENTS]) {
        return SAVEGAME_PC34_ERROR_BAD_CHECKSUM;
    }
    if (partLen > (int)sizeof(eventsPart)) {
        return SAVEGAME_PC34_ERROR_BUFFER_TOO_SMALL;
    }
    memset(eventsPart, 0, sizeof(eventsPart));
    memcpy(eventsPart, partBuf, (size_t)partLen);
    eventsLen = partLen;

    /* Part 4: TIMELINE. */
    partLen = pc34_read_part(buf, bufSize, &cursor, partBuf,
                             (int)sizeof(partBuf),
                             keys[SAVEGAME_PC34_PART_TIMELINE],
                             &actualChecksum);
    if (partLen < 0) return SAVEGAME_PC34_ERROR_BAD_SIZE;
    if (strictChecksums &&
        actualChecksum != checksums[SAVEGAME_PC34_PART_TIMELINE]) {
        return SAVEGAME_PC34_ERROR_BAD_CHECKSUM;
    }
    if (partLen > (int)sizeof(timelinePart)) {
        return SAVEGAME_PC34_ERROR_BUFFER_TOO_SMALL;
    }
    memset(timelinePart, 0, sizeof(timelinePart));
    memcpy(timelinePart, partBuf, (size_t)partLen);
    timelineLen = partLen;
    if (outState->timeline != 0) {
        unpack_events_and_timeline(&globalData,
                                   eventsPart, eventsLen,
                                   timelinePart, timelineLen,
                                   outState->timeline);
    }
    if (outState->party != 0) {
        int slot;
        for (slot = 0; slot < outState->party->championCount &&
                       slot < CHAMPION_MAX_PARTY; ++slot) {
            if (cursor + CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT > bufSize) {
                break;
            }
            memcpy(outState->party->champions[slot].portraitBitmap,
                   buf + cursor,
                   CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT);
            outState->party->champions[slot].portraitBitmapValid = 1;
            cursor += CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT;
        }
    }

    /* Stash gameID into the Firestaff header reserved area so the
     * rest of the engine can find it. The reserved[] array is always
     * present in the header struct, so no null check is needed. */
    write_u32_le(outState->header.reserved +
                 SAVEGAME_HEADER_RESERVED_GAME_ID_OFFSET, gameID);
    /* LSV-02: also stamp the manifest-derived gameCode into the
     * reserved area (next to gameID) so a M12 launcher can quote
     * it without re-parsing the PC 3.4 header. We use byte 5 of
     * reserved[36] to avoid clashing with the existing
     * MusicOn slot at byte 4. */
    if (manifestPresent) {
        outState->header.reserved[5] = (unsigned char)(manifestGameCode & 0xFFu);
        outState->header.reserved[6] = (unsigned char)((manifestGameCode >> 8) & 0xFFu);
    }
    /* Suppress unused-warnings for now. */
    (void)platform;
    (void)dungeonID;
    (void)manifestVersion;
    return SAVEGAME_PC34_OK;
}

/* ==========================================================
 *  LSV-02 public API: F0799/F0800/F0801
 * ========================================================== */

int F0799_SAVEGAME_PC34PeekManifest_Compat(
    const unsigned char* buf,
    int bufSize,
    uint16_t* outVersion,
    uint16_t* outGameCode,
    int* outBodySize)
{
    unsigned char header[SAVEGAME_PC34_DM_SAVE_HEADER_SIZE];
    uint16_t key;
    int manifestRc;
    if (buf == 0) return SAVEGAME_PC34_MANIFEST_ERR_BAD_MAGIC;
    if (bufSize < SAVEGAME_PC34_DM_SAVE_HEADER_SIZE) {
        /* Too small to even carry a header; treat as not present
         * so callers can distinguish "not a PC 3.4 file" from
         * "PC 3.4 file with a manifest we don't recognise". */
        if (outVersion != 0) *outVersion = 0;
        if (outGameCode != 0) *outGameCode = 0;
        if (outBodySize != 0) *outBodySize = 0;
        return SAVEGAME_PC34_MANIFEST_ERR_NOT_PRESENT;
    }
    memcpy(header, buf, SAVEGAME_PC34_DM_SAVE_HEADER_SIZE);
    key = read_u16_le(header +
                      SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX * 2u);
    (void)pc34_f0417_bytes(
        header + 256u, SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS, key);
    manifestRc = pc34_read_manifest_bytes(header, outVersion, outGameCode);
    if (outBodySize != 0) {
        *outBodySize = bufSize;
    }
    return manifestRc;
}

int F0800_SAVEGAME_PC34ValidateGameCode_Compat(
    const unsigned char* buf,
    int bufSize,
    uint16_t expectedGameCode,
    int requireManifest)
{
    int bodySize = 0;
    int rc;
    if (buf == 0) return SAVEGAME_PC34_MANIFEST_ERR_BAD_MAGIC;
    if (bufSize < SAVEGAME_PC34_DM_SAVE_HEADER_SIZE) {
        /* Not a header at all. If a manifest is required, the
         * caller gets a clear "not present" verdict; if not,
         * the file is acceptable as a vanilla PC 3.4 import. */
        return requireManifest
                   ? SAVEGAME_PC34_MANIFEST_ERR_NOT_PRESENT
                   : SAVEGAME_PC34_MANIFEST_OK;
    }
    rc = F0799_SAVEGAME_PC34PeekManifest_Compat(
        buf, bufSize, 0, 0, &bodySize);
    if (rc == SAVEGAME_PC34_MANIFEST_ERR_NOT_PRESENT) {
        return requireManifest
                   ? SAVEGAME_PC34_MANIFEST_ERR_NOT_PRESENT
                   : SAVEGAME_PC34_MANIFEST_OK;
    }
    if (rc != SAVEGAME_PC34_MANIFEST_OK) {
        return rc;
    }
    /* Manifest present + valid version. Peek the gameCode and
     * check against the expected value. */
    {
        uint16_t actual = 0;
        (void)F0799_SAVEGAME_PC34PeekManifest_Compat(
            buf, bufSize, 0, &actual, 0);
        if (actual != expectedGameCode) {
            return SAVEGAME_PC34_MANIFEST_ERR_WRONG_GAME;
        }
    }
    if (bodySize < SAVEGAME_PC34_DM_SAVE_HEADER_SIZE) {
        return SAVEGAME_PC34_MANIFEST_ERR_BODY_TRUNCATED;
    }
    return SAVEGAME_PC34_MANIFEST_OK;
}

const char* F0801_SAVEGAME_PC34GameCodeName_Compat(uint16_t gameCode) {
    switch (gameCode) {
    case SAVEGAME_PC34_GAME_CODE_DM1:    return "DM1";
    case SAVEGAME_PC34_GAME_CODE_CSB:    return "CSB";
    case SAVEGAME_PC34_GAME_CODE_DM2:    return "DM2";
    case SAVEGAME_PC34_GAME_CODE_NEXUS:  return "NEXUS";
    case SAVEGAME_PC34_GAME_CODE_THERON: return "THERON";
    default:
        /* Reserved / unknown. We still return a stable string so
         * log lines do not stutter on the unknown code. */
        if (gameCode == 0u) return "UNSET";
        return "UNKNOWN";
    }
}

/* ==========================================================
 *  Error string.
 * ========================================================== */

const char* F0797_SAVEGAME_PC34ErrorToString_Compat(int code) {
    switch (code) {
    case SAVEGAME_PC34_OK:                     return "ok";
    case SAVEGAME_PC34_ERROR_NULL_ARG:         return "null argument";
    case SAVEGAME_PC34_ERROR_BUFFER_TOO_SMALL: return "buffer too small";
    case SAVEGAME_PC34_ERROR_BAD_MAGIC:        return "bad magic / header";
    case SAVEGAME_PC34_ERROR_BAD_VERSION:      return "bad version";
    case SAVEGAME_PC34_ERROR_BAD_SIZE:         return "bad file size";
    case SAVEGAME_PC34_ERROR_BAD_CHECKSUM:     return "bad checksum";
    case SAVEGAME_PC34_ERROR_UNSUPPORTED:      return "unsupported format";
    case SAVEGAME_PC34_ERROR_INTERNAL:         return "internal error";
    case SAVEGAME_PC34_MANIFEST_OK:            return "manifest ok";
    case SAVEGAME_PC34_MANIFEST_ERR_BAD_MAGIC: return "manifest bad magic";
    case SAVEGAME_PC34_MANIFEST_ERR_BAD_VERSION:return "manifest bad version";
    case SAVEGAME_PC34_MANIFEST_ERR_WRONG_GAME: return "manifest wrong game";
    case SAVEGAME_PC34_MANIFEST_ERR_BODY_TRUNCATED: return "manifest body truncated";
    case SAVEGAME_PC34_MANIFEST_ERR_NOT_PRESENT: return "manifest not present";
    default:                                    return "unknown";
    }
}
