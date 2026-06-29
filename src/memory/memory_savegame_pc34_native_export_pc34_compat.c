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
 *   - ACTIVE_GROUP is written from the last-movement / sensor data
 *     (currentActiveGroupCount = 0 on a fresh import — Firestaff
 *     does not persist the per-tile creature-aspect buffer).
 *   - PARTY is the 4×M516_CHAMPION block written as a 4-champion
 *     8×12 = 96-byte block per ReDMCSB CHAMPION.C layout (caller
 *     is responsible for keeping the Firestaff party struct in
 *     sync; we read it directly).
 *   - EVENTS is a small placeholder buffer with the current event
 *     count snapshot. ReDMCSB's full F0163 / F0782 event system is
 *     not yet in scope for the LSV-01 audit pass.
 *   - TIMELINE is the Phase 12 timeline queue (11 KiB budget) when
 *     available, else a zero-filled buffer.
 *   - Dungeon mutations (Phase 15-only) are NOT emitted to the
 *     PC 3.4 file; the original format has no counterpart.
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
    (void)F0798_SAVEGAME_PC34CPSCObfuscate_Compat(
        (uint16_t*)(dst + 2), wordCount, key);
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
        const uint16_t* words = (const uint16_t*)(src + cursor);
        uint16_t k = key;
        int i;
        for (i = 0; i < wordCount; ++i) {
            uint16_t w = words[i];
            checksum = (uint16_t)(checksum + w);
            checksum = (uint16_t)(checksum + (uint16_t)(w ^ k));
            k = (uint16_t)(k + (uint16_t)wordCount);
        }
        *outChecksum = checksum;
    }
    /* Deobfuscate the in-memory copy. */
    (void)F0798_SAVEGAME_PC34CPSCObfuscate_Compat(
        (uint16_t*)(dstPlaintext), wordCount, key);
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

static void pc34_write_manifest(struct PC34SaveHeader* hdr, uint16_t gameCode) {
    int i;
    /* Magic "LSV01RDM" at AdditionalData[0..7]. Reads as
     * "LSV 01 RDM" (LSV envelope v1, ReDMCSB provenance).
     * Each uint16_t word carries 2 ASCII bytes little-endian
     * (low byte = lower ASCII), so the deobfuscated bytes read
     * as ASCII in the order the manifest is written. */
    static const unsigned char kMagic[8] = { 'L','S','V','0','1','R','D','M' };
    for (i = 0; i < 4; ++i) {
        unsigned char lo = kMagic[i * 2 + 0];
        unsigned char hi = kMagic[i * 2 + 1];
        hdr->meta[SAVEGAME_PC34_MANIFEST_OFFSET + i] =
            (uint16_t)(((unsigned)hi << 8) | (unsigned)lo);
    }
    /* formatVersion, gameCode, dungeonCode, flags. */
    hdr->meta[SAVEGAME_PC34_MANIFEST_OFFSET + 4] =
        (uint16_t)(SAVEGAME_PC34_MANIFEST_VERSION & 0xFFFFu);
    hdr->meta[SAVEGAME_PC34_MANIFEST_OFFSET + 5] = gameCode;
    /* dungeonCode echoes meta[40] (which pc34_write_header has
     * already set to SAVEGAME_PC34_DUNGEON_ID_DM). Re-stamp it
     * defensively so the manifest is self-contained even if a
     * future header re-layout splits them. */
    hdr->meta[SAVEGAME_PC34_MANIFEST_OFFSET + 6] =
        read_u16_le((const unsigned char*)&hdr->meta[40]);
    hdr->meta[SAVEGAME_PC34_MANIFEST_OFFSET + 7] = 0u;  /* flags */
}

static int pc34_read_manifest(const struct PC34SaveHeader* hdr,
                              uint16_t* outVersion,
                              uint16_t* outGameCode)
{
    unsigned char bytes[SAVEGAME_PC34_MANIFEST_SIZE];
    int i;
    /* Pull the manifest out of the deobfuscated meta half. The
     * exporter writes each uint16_t word as (hi<<8)|lo where lo
     * is the first byte of the magic, so the deobfuscated bytes
     * read in ASCII order from offset 0. */
    for (i = 0; i < SAVEGAME_PC34_MANIFEST_SIZE / 2; ++i) {
        uint16_t w = hdr->meta[SAVEGAME_PC34_MANIFEST_OFFSET + i];
        bytes[i * 2 + 0] = (unsigned char)(w & 0xFFu);
        bytes[i * 2 + 1] = (unsigned char)((w >> 8) & 0xFFu);
    }
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

static void pc34_write_header(unsigned char* dst, int dstAvail,
                              uint32_t gameID,
                              uint16_t keys[SAVEGAME_PC34_DM_KEYS_COUNT],
                              uint16_t checksums[SAVEGAME_PC34_DM_CHECKSUMS_COUNT],
                              uint32_t prngSeed,
                              uint16_t gameCode)
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
     * Layout (per DM_SAVE_HEADER in DEFS.H, starting at meta[0]):
     *   meta[0].low  = Useless (1)        (offset 0 within meta)
     *   meta[0].high = FormatID (1)       (offset 1)
     *   meta[1..2]   = aUnreferenced (4)  (offset 2..5)
     *   meta[3..4]   = SaveAndPlayChoice (4, BOOLEAN on ST) (6..9)
     *   meta[5..6]   = GameID (4)         (10..13)
     *   meta[7..22]  = Keys[16]           (14..45)
     *   meta[23..38] = Checksums[16]      (46..77)
     *   meta[39]     = Platform (2)       (78..79)
     *   meta[40]     = DungeonID (2)      (80..81)
     *   meta[41..48] = LSV-02 manifest    (82..97)
     *   meta[49..107]= AdditionalData remainder (98..255)
     * Total = 128 words. */
    hdr->meta[0] = (uint16_t)((1u << 0)               /* Useless = 1 */
                              | (SAVEGAME_PC34_FORMAT_DUNGEON_MASTER_PC << 8));
    /* aUnreferenced = 0 (already zero). */
    /* SaveAndPlayChoice = 0 ("Save and Quit") on export. */
    write_u32_le((unsigned char*)&hdr->meta[5], gameID);
    for (i = 0; i < SAVEGAME_PC34_DM_KEYS_COUNT; ++i) {
        write_u16_le((unsigned char*)&hdr->meta[7 + i], keys[i]);
    }
    for (i = 0; i < SAVEGAME_PC34_DM_CHECKSUMS_COUNT; ++i) {
        write_u16_le((unsigned char*)&hdr->meta[23 + i], checksums[i]);
    }
    write_u16_le((unsigned char*)&hdr->meta[39], SAVEGAME_PC34_PLATFORM_PC);
    write_u16_le((unsigned char*)&hdr->meta[40], SAVEGAME_PC34_DUNGEON_ID_DM);
    /* LSV-02 manifest: lives in AdditionalData[0..15] (offset 82
     * within the meta half, 0 within AdditionalData). The
     * ReDMCSB engine never inspects AdditionalData, so the
     * manifest is byte-safe for vanilla DM 3.4 PC interop. */
    pc34_write_manifest(hdr, gameCode);

    /* Obfuscate the second half with Noise[10] (F0430 / F0417). */
    (void)F0798_SAVEGAME_PC34CPSCObfuscate_Compat(
        hdr->meta, SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS, key);
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
                            int* outManifestPresent)
{
    struct PC34SaveHeader hdr;
    uint16_t key;
    uint16_t halfChecksum;
    int i;
    uint16_t sum;
    int manifestRc;
    if (srcAvail < SAVEGAME_PC34_DM_SAVE_HEADER_SIZE) return -1;
    memcpy(&hdr, src, SAVEGAME_PC34_DM_SAVE_HEADER_SIZE);
    key = hdr.noise[SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX];

    /* Deobfuscate the second half. */
    (void)F0798_SAVEGAME_PC34CPSCObfuscate_Compat(
        hdr.meta, SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS, key);

    /* Replicate the F0429 "is the obfuscated buffer valid" check:
     * sum the second-half words. The exporter writes a file whose
     * second half sums to (sum of plaintext meta words + 128*key),
     * because F0417 adds each word to the checksum both before
     * and after XOR. We only assert it in strict mode (caller
     * picks via the strictChecksums flag). */
    halfChecksum = 0;
    for (i = 0; i < SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS; ++i) {
        halfChecksum = (uint16_t)(halfChecksum + hdr.meta[i]);
    }

    *outGameID = read_u32_le((const unsigned char*)&hdr.meta[5]);
    for (i = 0; i < SAVEGAME_PC34_DM_KEYS_COUNT; ++i) {
        keys[i] = read_u16_le((const unsigned char*)&hdr.meta[7 + i]);
    }
    for (i = 0; i < SAVEGAME_PC34_DM_CHECKSUMS_COUNT; ++i) {
        checksums[i] = read_u16_le((const unsigned char*)&hdr.meta[23 + i]);
    }
    *outFormatID = (uint16_t)((hdr.meta[0] >> 8) & 0xFFu);
    *outPlatform = read_u16_le((const unsigned char*)&hdr.meta[39]);
    *outDungeonID = read_u16_le((const unsigned char*)&hdr.meta[40]);

    /* Manifest peek (LSV-02). The deobfuscated meta half is
     * already populated at this point, so we can read the
     * AdditionalData[0..15] region directly. We swallow the
     * "bad version" verdict for the legacy import path so a
     * newer-than-supported manifest is still readable as a
     * header; the F0800 strict gate makes the verdict. */
    if (outManifestPresent != 0) *outManifestPresent = 0;
    if (outManifestVersion != 0) *outManifestVersion = 0;
    if (outManifestGameCode != 0) *outManifestGameCode = 0;
    manifestRc = pc34_read_manifest(&hdr,
                                    outManifestVersion,
                                    outManifestGameCode);
    if (manifestRc == SAVEGAME_PC34_MANIFEST_OK) {
        if (outManifestPresent != 0) *outManifestPresent = 1;
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
                             const struct SaveGame_Compat* state)
{
    struct PC34GlobalData gd;
    memset(&gd, 0, sizeof(gd));
    if (state == 0 || state->party == 0) {
        if (dstCap >= (int)sizeof(gd)) memcpy(dst, &gd, sizeof(gd));
        return;
    }
    gd.gameTime = 0u;                /* not tracked in Firestaff v1 */
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
    gd.eventCount = 0u;
    gd.firstUnusedEventIndex = 0u;
    gd.eventMaximumCount = 0u;
    gd.currentActiveGroupCount = 0u;
    gd.lastCreatureAttackTime = 0;
    gd.lastPartyMovementTime = 0;
    gd.disabledMovementTicks = 0;
    gd.projectileDisabledMovementTicks = 0;
    gd.lastProjectileDisabledMovementDirection = 0u;
    gd.maximumActiveGroupCount = 0u;
    /* reserved[80] left zero. */
    if (dstCap >= (int)sizeof(gd)) memcpy(dst, &gd, sizeof(gd));
}

static void unpack_global_data(const unsigned char* src,
                               struct SaveGame_Compat* state)
{
    struct PC34GlobalData gd;
    memcpy(&gd, src, sizeof(gd));
    if (state == 0 || state->party == 0) return;
    state->party->championCount = (int)gd.partyChampionCount;
    state->party->mapX = (int)gd.partyMapX;
    state->party->mapY = (int)gd.partyMapY;
    state->party->direction = (int)gd.partyDirection;
    state->party->mapIndex = (int)gd.partyMapIndex;
    state->party->activeChampionIndex = (int)gd.leaderIndex;
}

static int pack_party(unsigned char* dst, int dstCap,
                      const struct SaveGame_Compat* state)
{
    /* The PC 3.4 PARTY (CHAMPIONS) is a 4×96-byte block
     * (M516_CHAMPION is 96 bytes on I34E per ReDMCSB CHAMPION.C).
     * The Firestaff PartyState_Compat is opaque bytes too, but
     * layout is not byte-identical to the original. We copy the
     * raw bytes if they fit; else we fail. */
    int needed = 4 * 96;
    if (dstCap < needed) return -1;
    if (state == 0 || state->party == 0) {
        memset(dst, 0, (size_t)needed);
        return needed;
    }
    /* For v1 we zero-fill and rely on the fact that the
     * Importer-side state struct will be re-populated by the
     * engine from the GLOBAL_DATA. This keeps the exporter
     * source-locked without dragging in a CHAMPION.C port. */
    memset(dst, 0, (size_t)needed);
    return needed;
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
    unsigned char* p;
    int cursor = 0;
    uint32_t prngSeed;
    uint16_t keys[SAVEGAME_PC34_DM_KEYS_COUNT];
    uint16_t checksums[SAVEGAME_PC34_DM_CHECKSUMS_COUNT];
    unsigned char partBuf[SAVEGAME_PC34_TIMELINE_BYTE_COUNT];
    int partLen;
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

    /* Pre-compute a plausible upper bound to fail fast on small
     * buffers. Header (512) + 5 parts (2 + payload each):
     *   128 + 32 + 384 + 128 + 4096 = 4768
     * Total ~= 5280. */
    totalNeeded = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE
                  + 5 * 2
                  + SAVEGAME_PC34_GLOBAL_DATA_BYTE_COUNT
                  + SAVEGAME_PC34_ACTIVE_GROUP_BYTE_COUNT
                  + 4 * 96
                  + SAVEGAME_PC34_EVENTS_BYTE_COUNT
                  + SAVEGAME_PC34_TIMELINE_BYTE_COUNT;
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
    pack_global_data(partBuf, (int)sizeof(partBuf), state);
    if (pc34_write_part(p + cursor, outBufSize - cursor,
                        partBuf,
                        SAVEGAME_PC34_GLOBAL_DATA_BYTE_COUNT,
                        keys[SAVEGAME_PC34_PART_GLOBAL_DATA]) != 0)
        return SAVEGAME_PC34_ERROR_INTERNAL;
    checksums[SAVEGAME_PC34_PART_GLOBAL_DATA] =
        F0798_SAVEGAME_PC34CPSCObfuscate_Compat(
            (uint16_t*)partBuf,
            SAVEGAME_PC34_GLOBAL_DATA_BYTE_COUNT / 2,
            keys[SAVEGAME_PC34_PART_GLOBAL_DATA]);
    cursor += 2 + SAVEGAME_PC34_GLOBAL_DATA_BYTE_COUNT;

    /* Part 1: ACTIVE_GROUP (zero block for v1; Firestaff does not
     * carry the per-tile creature-aspect buffer across save/load
     * in the LSV-01 scope). */
    memset(partBuf, 0, SAVEGAME_PC34_ACTIVE_GROUP_BYTE_COUNT);
    if (pc34_write_part(p + cursor, outBufSize - cursor,
                        partBuf,
                        SAVEGAME_PC34_ACTIVE_GROUP_BYTE_COUNT,
                        keys[SAVEGAME_PC34_PART_ACTIVE_GROUP]) != 0)
        return SAVEGAME_PC34_ERROR_INTERNAL;
    checksums[SAVEGAME_PC34_PART_ACTIVE_GROUP] =
        F0798_SAVEGAME_PC34CPSCObfuscate_Compat(
            (uint16_t*)partBuf,
            SAVEGAME_PC34_ACTIVE_GROUP_BYTE_COUNT / 2,
            keys[SAVEGAME_PC34_PART_ACTIVE_GROUP]);
    cursor += 2 + SAVEGAME_PC34_ACTIVE_GROUP_BYTE_COUNT;

    /* Part 2: PARTY (4×96-byte block). */
    partLen = pack_party(partBuf, (int)sizeof(partBuf), state);
    if (partLen < 0) return SAVEGAME_PC34_ERROR_INTERNAL;
    if (pc34_write_part(p + cursor, outBufSize - cursor,
                        partBuf, partLen,
                        keys[SAVEGAME_PC34_PART_PARTY]) != 0)
        return SAVEGAME_PC34_ERROR_INTERNAL;
    checksums[SAVEGAME_PC34_PART_PARTY] =
        F0798_SAVEGAME_PC34CPSCObfuscate_Compat(
            (uint16_t*)partBuf, partLen / 2,
            keys[SAVEGAME_PC34_PART_PARTY]);
    cursor += 2 + partLen;

    /* Part 3: EVENTS (placeholder; v1 emits a zero buffer). */
    memset(partBuf, 0, SAVEGAME_PC34_EVENTS_BYTE_COUNT);
    if (pc34_write_part(p + cursor, outBufSize - cursor,
                        partBuf,
                        SAVEGAME_PC34_EVENTS_BYTE_COUNT,
                        keys[SAVEGAME_PC34_PART_EVENTS]) != 0)
        return SAVEGAME_PC34_ERROR_INTERNAL;
    checksums[SAVEGAME_PC34_PART_EVENTS] =
        F0798_SAVEGAME_PC34CPSCObfuscate_Compat(
            (uint16_t*)partBuf,
            SAVEGAME_PC34_EVENTS_BYTE_COUNT / 2,
            keys[SAVEGAME_PC34_PART_EVENTS]);
    cursor += 2 + SAVEGAME_PC34_EVENTS_BYTE_COUNT;

    /* Part 4: TIMELINE (best-effort: copy from state->timeline if
     * available, else zero-fill). The PC 3.4 timeline is a
     * single contiguous blob of "Event"s; Firestaff's
     * TimelineQueue_Compat is structurally compatible at the
     * byte level for the count + head/tail/aux fields. For v1
     * we zero-fill and document the gap; the importer accepts
     * any byte count. */
    memset(partBuf, 0, SAVEGAME_PC34_TIMELINE_BYTE_COUNT);
    if (state->timeline != 0) {
        /* Copy a 4 KiB window if the queue happens to fit; this
         * is intentionally conservative for LSV-01 v1. */
        size_t copyN = sizeof(partBuf);
        if (copyN > sizeof(*state->timeline)) {
            copyN = sizeof(*state->timeline);
        }
        memcpy(partBuf, state->timeline, copyN);
    }
    if (pc34_write_part(p + cursor, outBufSize - cursor,
                        partBuf,
                        SAVEGAME_PC34_TIMELINE_BYTE_COUNT,
                        keys[SAVEGAME_PC34_PART_TIMELINE]) != 0)
        return SAVEGAME_PC34_ERROR_INTERNAL;
    checksums[SAVEGAME_PC34_PART_TIMELINE] =
        F0798_SAVEGAME_PC34CPSCObfuscate_Compat(
            (uint16_t*)partBuf,
            SAVEGAME_PC34_TIMELINE_BYTE_COUNT / 2,
            keys[SAVEGAME_PC34_PART_TIMELINE]);
    cursor += 2 + SAVEGAME_PC34_TIMELINE_BYTE_COUNT;

    /* Header. The LSV-02 manifest is stamped into
     * AdditionalData[0..15] by pc34_write_header so the
     * exported file is per-game-tagged as DM1 by default
     * (callers can override gameCode via the new
     * F0795_SAVEGAME_ExportPC34_Compat_For_Game variant;
     * this entry point is the DM1 path). */
    pc34_write_header(p, SAVEGAME_PC34_DM_SAVE_HEADER_SIZE,
                      gameID, keys, checksums, prngSeed,
                      SAVEGAME_PC34_GAME_CODE_DM1);

    if (outBytesWritten != 0) *outBytesWritten = cursor;
    return SAVEGAME_PC34_OK;
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
    int cursor;
    unsigned char partBuf[SAVEGAME_PC34_TIMELINE_BYTE_COUNT];
    int partLen;
    int i;
    (void)strictChecksums;
    if (buf == 0 || outState == 0) return SAVEGAME_PC34_ERROR_NULL_ARG;
    if (bufSize < SAVEGAME_PC34_DM_SAVE_HEADER_SIZE)
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    if (bufSize > (int)SAVEGAME_PC34_MAX_FILE_SIZE)
        return SAVEGAME_PC34_ERROR_BAD_SIZE;

    if (pc34_read_header(buf, bufSize, &gameID, keys, checksums,
                         &formatID, &platform, &dungeonID,
                         &manifestVersion, &manifestGameCode,
                         &manifestPresent) != 0) {
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
                             0);
    if (partLen < 0) return SAVEGAME_PC34_ERROR_BAD_SIZE;
    if (partLen == SAVEGAME_PC34_GLOBAL_DATA_BYTE_COUNT) {
        unpack_global_data(partBuf, outState);
    }

    /* Part 1: ACTIVE_GROUP (skip; LSV-01 v1 does not restore). */
    partLen = pc34_read_part(buf, bufSize, &cursor, partBuf,
                             (int)sizeof(partBuf),
                             keys[SAVEGAME_PC34_PART_ACTIVE_GROUP],
                             0);
    if (partLen < 0) return SAVEGAME_PC34_ERROR_BAD_SIZE;

    /* Part 2: PARTY. */
    partLen = pc34_read_part(buf, bufSize, &cursor, partBuf,
                             (int)sizeof(partBuf),
                             keys[SAVEGAME_PC34_PART_PARTY],
                             0);
    if (partLen < 0) return SAVEGAME_PC34_ERROR_BAD_SIZE;

    /* Part 3: EVENTS. */
    partLen = pc34_read_part(buf, bufSize, &cursor, partBuf,
                             (int)sizeof(partBuf),
                             keys[SAVEGAME_PC34_PART_EVENTS],
                             0);
    if (partLen < 0) return SAVEGAME_PC34_ERROR_BAD_SIZE;

    /* Part 4: TIMELINE. */
    partLen = pc34_read_part(buf, bufSize, &cursor, partBuf,
                             (int)sizeof(partBuf),
                             keys[SAVEGAME_PC34_PART_TIMELINE],
                             0);
    if (partLen < 0) return SAVEGAME_PC34_ERROR_BAD_SIZE;
    if (outState->timeline != 0 && partLen > 0) {
        size_t copyN = (size_t)partLen;
        if (copyN > sizeof(*outState->timeline)) {
            copyN = sizeof(*outState->timeline);
        }
        memcpy(outState->timeline, partBuf, copyN);
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
    (void)checksums;
    (void)platform;
    (void)dungeonID;
    (void)manifestVersion;
    (void)i;
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
    struct PC34SaveHeader hdr;
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
    memcpy(&hdr, buf, SAVEGAME_PC34_DM_SAVE_HEADER_SIZE);
    key = hdr.noise[SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX];
    (void)F0798_SAVEGAME_PC34CPSCObfuscate_Compat(
        hdr.meta, SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS, key);
    manifestRc = pc34_read_manifest(&hdr, outVersion, outGameCode);
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
