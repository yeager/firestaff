#ifndef REDMCSB_MEMORY_SAVEGAME_PC34_COMPAT_H
#define REDMCSB_MEMORY_SAVEGAME_PC34_COMPAT_H

/*
 * Save-game composite data layer for ReDMCSB PC 3.4 — Phase 15 of M10.
 *
 * Pure, caller-driven save/load wrapper that composes every Phase 10–14
 * serialiser into a single self-describing binary blob. Authoritative
 * plan: PHASE15_PLAN.md (M10 milestone).
 *
 * Conventions inherited from Phases 10–14:
 *   - All symbols suffixed _pc34_compat / _Compat.
 *   - MEDIA016 / PC LSB-first serialisation; little-endian host assumed.
 *   - NO globals, NO UI, NO RNG. Randomness flows through CombatScratch
 *     state bytes only. The ONLY IO-allowed pair is F0785 / F0786;
 *     every other function is pure (buffer-in / buffer-out).
 *   - Bit-identical round-trip for every struct AND the composite blob.
 *   - Function numbering claims F0770–F0789 exclusively.
 *   - ADDITIVE ONLY: composes existing serialisers; never edits them.
 *     The only NEW pair is MovementResult_Compat (Phase 10 never
 *     shipped one) — added file-local as F0776a / F0776b (§2).
 *
 * Integrity: CRC32 / IEEE 802.3 reflected polynomial 0xEDB88320,
 * init 0xFFFFFFFF, final XOR 0xFFFFFFFF. Covers bytes [64 .. EOF).
 *
 * See PHASE15_PLAN.md §1/§2/§3 for full rationale.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "memory_champion_state_pc34_compat.h"
#include "memory_movement_pc34_compat.h"
#include "memory_sensor_execution_pc34_compat.h"
#include "memory_timeline_pc34_compat.h"
#include "memory_combat_pc34_compat.h"
#include "memory_magic_pc34_compat.h"

/* -------- Header / table constants (plan §2) -------- */

#define SAVEGAME_FORMAT_VERSION                 1u
#define SAVEGAME_ENDIAN_SENTINEL                0x01020304u
#define SAVEGAME_HEADER_SERIALIZED_SIZE         64
#define SAVEGAME_SECTION_ENTRY_SERIALIZED_SIZE  16
#define SAVEGAME_SECTION_COUNT                  7
#define SAVEGAME_MAX_FILE_SIZE                  (1 << 20)  /* 1 MiB hard cap */

/* Section kinds (plan §2). Values are 0x00010001 + slot; the slot
 * number is *not* exported separately (kept implicit in the table
 * walk). */
#define SAVEGAME_SECTION_PARTY          0x00010001u
#define SAVEGAME_SECTION_CHAMPIONS      0x00010002u  /* reserved; not emitted in v1 */
#define SAVEGAME_SECTION_MOVEMENT       0x00010003u
#define SAVEGAME_SECTION_SENSOR         0x00010004u
#define SAVEGAME_SECTION_TIMELINE       0x00010005u
#define SAVEGAME_SECTION_COMBAT         0x00010006u
#define SAVEGAME_SECTION_MAGIC          0x00010007u
#define SAVEGAME_SECTION_DUNGEON_DELTA  0x00010008u

/* -------- Per-section payload sizes (plan §2 budget table) -------- */

#define MOVEMENT_RESULT_SERIALIZED_SIZE  20   /* 5 int32 LE */

#define DUNGEON_MUTATION_SERIALIZED_SIZE 48   /* 12 int32 LE */
#define DUNGEON_MUTATION_LIST_MAX_COUNT  1024
#define DUNGEON_MUTATION_LIST_SERIALIZED_SIZE \
    (4 + (DUNGEON_MUTATION_LIST_MAX_COUNT * DUNGEON_MUTATION_SERIALIZED_SIZE))

#define COMBAT_SCRATCH_SERIALIZED_SIZE   100  /* 56 + 32 + 4 + 8 */

/* -------- Dungeon-mutation kinds (plan §2) -------- */

#define DUNGEON_MUTATION_KIND_INVALID      0
#define DUNGEON_MUTATION_KIND_SQUARE_BYTE  1
#define DUNGEON_MUTATION_KIND_DOOR_STATE   2
#define DUNGEON_MUTATION_KIND_SENSOR_TOG   3
#define DUNGEON_MUTATION_KIND_GROUP_HP     4
#define DUNGEON_MUTATION_KIND_THING_LINK   5
#define DUNGEON_MUTATION_KIND_FIELD_GENERIC 6

/* -------- Error codes (plan §3.0) -------- */

enum SaveGameError_Compat {
    SAVEGAME_OK                       = 0,
    SAVEGAME_ERROR_NULL_ARG           = 1,
    SAVEGAME_ERROR_BUFFER_TOO_SMALL   = 2,
    SAVEGAME_ERROR_BAD_MAGIC          = 3,
    SAVEGAME_ERROR_BAD_VERSION        = 4,
    SAVEGAME_ERROR_BAD_ENDIAN         = 5,
    SAVEGAME_ERROR_BAD_SIZE           = 6,
    SAVEGAME_ERROR_BAD_CRC            = 7,
    SAVEGAME_ERROR_BAD_SECTION_COUNT  = 8,
    SAVEGAME_ERROR_BAD_SECTION_KIND   = 9,
    SAVEGAME_ERROR_BAD_SECTION_ORDER  = 10,
    SAVEGAME_ERROR_SECTION_OVERFLOW   = 11,
    SAVEGAME_ERROR_SECTION_OVERLAP    = 12,
    SAVEGAME_ERROR_SUBSYS_DESERIALIZE = 13,
    SAVEGAME_ERROR_FILE_OPEN          = 14,
    SAVEGAME_ERROR_FILE_READ          = 15,
    SAVEGAME_ERROR_FILE_WRITE         = 16,
    SAVEGAME_ERROR_FILE_SIZE          = 17,
    SAVEGAME_ERROR_INTERNAL           = 99
};

/* -------- Serialisable framing structs -------- */

/*
 * NEEDS DISASSEMBLY REVIEW: Fontanel's DM_SAVE_HEADER carries
 * Noise[10] obfuscation input + Keys[16] + Checksums[16]; we
 * intentionally DO NOT reproduce it (see plan §1 and §4.8 item 1).
 * v1 ships a clean single-CRC32 header. The XOR-key derivation from
 * Noise[] would need to be re-implemented only if DMSAVE1.DAT
 * byte-level interop is ever required.
 *
 * NEEDS DISASSEMBLY REVIEW: GameID / MusicOn fields from Fontanel's
 * GLOBAL_DATA block live in reserved[36] conceptually — deferred to
 * the audio / migration phase (plan §4.7, §4.8 item 3).
 */
struct SaveGameHeader_Compat {
    unsigned char magic[8];      /* "RDMCSB15" literal, no NUL */
    uint32_t      formatVersion;
    uint32_t      endianSentinel;
    uint32_t      totalFileSize;
    uint32_t      sectionCount;
    uint32_t      bodyCRC32;
    unsigned char reserved[36];
};

struct SaveGameSectionEntry_Compat {
    uint32_t kind;
    uint32_t offset;
    uint32_t size;
    uint32_t reserved;
};

/* One atomic dungeon mutation — caller-produced, Phase 15 only
 * serialises it. Fixed 48-byte LE layout (12 × int32). */
struct DungeonMutation_Compat {
    int      kind;         /* DUNGEON_MUTATION_KIND_* */
    int      mapIndex;     /* 0..mapCount-1 or -1 */
    int      x;            /* 0..width-1 or -1 */
    int      y;            /* 0..height-1 or -1 */
    int      cell;         /* 0..3 or -1 */
    int      thingType;    /* 0..15 or -1 */
    int      thingIndex;   /* 0..count-1 or -1 */
    uint32_t beforeValue;
    uint32_t afterValue;
    int      fieldMask;    /* bitmask identifying which field changed */
    int      reserved;
    int      reserved1;
};

struct DungeonMutationList_Compat {
    int count;
    struct DungeonMutation_Compat entries[DUNGEON_MUTATION_LIST_MAX_COUNT];
};

/* Scratch block that captures the combat-resolver state between
 * turns (plan §2). Layout: 56 (result) + 32 (weapon) + 4 (rng) +
 * 4 (combatActive) + 4 (reserved) = 100 bytes. */
struct CombatScratch_Compat {
    struct CombatResult_Compat   lastResult;
    struct WeaponProfile_Compat  lastWeapon;
    struct RngState_Compat       rng;
    int                          combatActive;
    int                          reserved;
};

/*
 * Composite save state. Holds non-owning pointers into caller-owned
 * subsystem structs; Phase 15 never allocates these.
 */
struct SaveGame_Compat {
    struct SaveGameHeader_Compat header;

    struct PartyState_Compat*           party;            /* slot 0 */
    struct MovementResult_Compat*       lastMovement;    /* slot 1 */
    struct SensorEffectList_Compat*     pendingSensorEffects; /* slot 2 */
    struct TimelineQueue_Compat*        timeline;        /* slot 3 */
    struct CombatScratch_Compat*        combatScratch;   /* slot 4 */
    struct MagicState_Compat*           magic;           /* slot 5 */
    struct DungeonMutationList_Compat*  mutations;       /* slot 6 */

    /* Diagnostic: last section index that failed a subsystem
     * deserialise (-1 if none). Never serialised. */
    int lastFailingSection;
};

/* ==========================================================
 *  Group A — Checksum / header helpers (F0770–F0772)
 * ========================================================== */

uint32_t F0770_SAVEGAME_CRC32_Compat(
    const unsigned char* buf, size_t len);

int F0771_SAVEGAME_WriteHeader_Compat(
    struct SaveGameHeader_Compat* hdr,
    uint32_t totalSize, uint32_t bodyCRC);

int F0772_SAVEGAME_ValidateHeader_Compat(
    const struct SaveGameHeader_Compat* hdr,
    uint32_t actualBufferSize);

/* ==========================================================
 *  Group B — Top-level save/load (F0773–F0775)
 * ========================================================== */

int F0773_SAVEGAME_SaveToBuffer_Compat(
    const struct SaveGame_Compat* state,
    unsigned char* outBuf, int outBufSize,
    int* outBytesWritten);

int F0774_SAVEGAME_LoadFromBuffer_Compat(
    const unsigned char* buf, int bufSize,
    struct SaveGame_Compat* outState);

int F0775_SAVEGAME_InitEmpty_Compat(
    struct SaveGame_Compat* state);

/* ==========================================================
 *  Group C — Per-section (de)serialisers (F0776–F0782)
 *  Each (aside from F0776a/b, F0780b, F0782b) delegates to an
 *  existing Phase 10–14 helper with size checks.
 * ========================================================== */

int F0776_SAVEGAME_SerializeParty_Compat(
    const struct PartyState_Compat* party,
    unsigned char* buf, int bufSize, int* outBytes);

int F0776_SAVEGAME_DeserializeParty_Compat(
    struct PartyState_Compat* party,
    const unsigned char* buf, int bufSize);

/* MovementResult Phase-15-local pair (§2 — no upstream serialiser). */
int F0776a_SAVEGAME_MovementResultSerialize_Compat(
    const struct MovementResult_Compat* mv,
    unsigned char* buf, int bufSize);

int F0776b_SAVEGAME_MovementResultDeserialize_Compat(
    struct MovementResult_Compat* mv,
    const unsigned char* buf, int bufSize);

int F0777_SAVEGAME_SerializeMovement_Compat(
    const struct MovementResult_Compat* mv,
    unsigned char* buf, int bufSize, int* outBytes);

int F0777_SAVEGAME_DeserializeMovement_Compat(
    struct MovementResult_Compat* mv,
    const unsigned char* buf, int bufSize);

int F0778_SAVEGAME_SerializeSensor_Compat(
    const struct SensorEffectList_Compat* s,
    unsigned char* buf, int bufSize, int* outBytes);

int F0778_SAVEGAME_DeserializeSensor_Compat(
    struct SensorEffectList_Compat* s,
    const unsigned char* buf, int bufSize);

int F0779_SAVEGAME_SerializeTimeline_Compat(
    const struct TimelineQueue_Compat* q,
    unsigned char* buf, int bufSize, int* outBytes);

int F0779_SAVEGAME_DeserializeTimeline_Compat(
    struct TimelineQueue_Compat* q,
    const unsigned char* buf, int bufSize);

int F0780_SAVEGAME_SerializeCombat_Compat(
    const struct CombatScratch_Compat* c,
    unsigned char* buf, int bufSize, int* outBytes);

int F0780b_SAVEGAME_DeserializeCombat_Compat(
    struct CombatScratch_Compat* c,
    const unsigned char* buf, int bufSize);

int F0781_SAVEGAME_SerializeMagic_Compat(
    const struct MagicState_Compat* m,
    unsigned char* buf, int bufSize, int* outBytes);

int F0781_SAVEGAME_DeserializeMagic_Compat(
    struct MagicState_Compat* m,
    const unsigned char* buf, int bufSize);

int F0782_SAVEGAME_SerializeDungeonDelta_Compat(
    const struct DungeonMutationList_Compat* list,
    unsigned char* buf, int bufSize, int* outBytes);

int F0782b_SAVEGAME_DeserializeDungeonDelta_Compat(
    struct DungeonMutationList_Compat* list,
    const unsigned char* buf, int bufSize);

/* ==========================================================
 *  Group D — Section table helpers (F0783–F0784)
 * ========================================================== */

int F0783_SAVEGAME_ComputeSectionTable_Compat(
    const struct SaveGame_Compat* state,
    struct SaveGameSectionEntry_Compat outTable[SAVEGAME_SECTION_COUNT],
    uint32_t* outTotalSize);

int F0784_SAVEGAME_ValidateSectionTable_Compat(
    const struct SaveGameSectionEntry_Compat table[SAVEGAME_SECTION_COUNT],
    uint32_t totalFileSize);

/* ==========================================================
 *  Group E — Filesystem wrapper (F0785–F0786). Only IO in phase.
 * ========================================================== */

int F0785_SAVEGAME_SaveToFile_Compat(
    const char* path,
    const struct SaveGame_Compat* state);

int F0786_SAVEGAME_LoadFromFile_Compat(
    const char* path,
    struct SaveGame_Compat* outState);

/* ==========================================================
 *  Group F — Diagnostics (F0787–F0789)
 * ========================================================== */

int F0787_SAVEGAME_Compare_Compat(
    const unsigned char* bufA, int sizeA,
    const unsigned char* bufB, int sizeB,
    int* outFirstDiffOffset);

int F0788_SAVEGAME_InspectHeader_Compat(
    const unsigned char* buf, int bufSize,
    struct SaveGameHeader_Compat* outHdr);

const char* F0789_SAVEGAME_ErrorToString_Compat(int code);

#endif /* REDMCSB_MEMORY_SAVEGAME_PC34_COMPAT_H */
