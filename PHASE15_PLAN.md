# PHASE 15 PLAN — Save / Load System (v1)

Firestaff M10 milestone, Phase 15. Status at planning time: 13 phases
PASS, Phase 14 (magic) in flight and expected merged before Phase 15
execution begins.

This document is the *single source of truth* the executor follows.
Any deviation = abort and ask.

Style rules (non-negotiable, inherited from Phases 10–14):

- All symbols end `_pc34_compat` / `_Compat`.
- MEDIA016 / PC LSB-first serialisation (little-endian host assumed;
  documented and `_Static_assert`-ed).
- Pure functions: NO globals, NO UI, NO RNG. The **only** IO
  allowed in this phase is the explicit file-read / file-write
  helper pair (F0785 / F0786). The composite in-memory save/load
  path (F0773 / F0774) is pure buffer-in / buffer-out.
- Bit-identical round-trip for every struct AND for the entire
  composite save-file. `memcmp == 0` after save → load → save.
- Function numbering claims **F0770–F0789** exclusively for this
  phase.
- Probe writes `savegame_probe.md` + `savegame_invariants.md`, both
  ending with `Status: PASS`.
- Verify gate: `run_firestaff_m10_verify.sh` gets **exactly one** new
  `# Phase 15:` block appended. Pre-grep must be 0, post-grep must
  be 1.
- ADDITIVE ONLY: no edits to any earlier-phase `.h` or `.c`. Phase 15
  *composes* existing serialise/deserialise pairs; it does not
  patch them. If a subsystem doesn't already expose a paired
  `_Serialize` / `_Deserialize` (all phases 10–14 do), we STOP and
  raise a review request — we do NOT extend the subsystem here.

---

## 1. Scope definition

### In scope for Phase 15 v1

A **pure data-layer save/load module** that wraps every runtime
subsystem produced by Phases 10–14 into one self-describing binary
blob. The module is deterministic, caller-driven, and produces
bit-identical round-trips across save → load → save.

The composite save file contains, in order:

1. **Header** (`SaveGameHeader_Compat`, 64 bytes)
   - Magic bytes: `"RDMCSB15"` (8 bytes ASCII, not NUL-terminated)
   - Format version: `SAVEGAME_FORMAT_VERSION = 1` (u32 LE)
   - Endianness sentinel: `0x01020304` (u32 LE — rejected if byte
     order differs)
   - Total file size in bytes (u32 LE — self-reported, validated on
     load)
   - Section count: `SAVEGAME_SECTION_COUNT = 7` (u32 LE)
   - Reserved: 36 bytes zeroed (future expansion — ignored on load
     in v1)
   - CRC32 of **everything after the header**, stored at the end of
     the header (u32 LE). Header is *not* included in its own CRC
     so we can compute body CRC → then write the header.
2. **Section table** (7 entries × 16 bytes = 112 bytes)
   - Each entry: `kind` (u32 LE), `offset` (u32 LE — relative to
     file start), `size` (u32 LE — bytes of payload), `reserved`
     (u32 LE = 0).
3. **Seven fixed-order section payloads**, one per subsystem (see
   §3 for the exact order). Each payload is exactly the bytes
   emitted by the corresponding existing `_Serialize_Compat`
   function from Phase 10–14, with zero framing overhead — the
   section table carries the length.

Integrity strategy (**chosen: CRC32 / IEEE 802.3 polynomial
`0xEDB88320`**):

- Justification vs. XXHash64: we keep the checksum code inside
  `memory_savegame_pc34_compat.c` with zero external dependencies.
  CRC32 fits in ~25 lines of portable C, is trivially
  little-endian-safe, and gives us exactly the tamper-detection
  property we need (1-bit flip → deterministic catch). XXHash
  would require either vendoring xxhash.c (new file) or writing
  a longer local implementation. KISS wins.
- No compression (out of scope).
- No encryption (out of scope).
- No magic-number per section (the section-table entry already
  carries `kind`; re-duplicating magic bytes inside each payload
  would bloat the file and require every subsystem's serialiser
  to emit a prefix it doesn't currently emit — that would be
  *subsystem-patching*, which Phase 15 refuses).

**In-scope invariants** at the top level:

- `F0773(state) → buf` then `F0774(buf) → state'` then
  `F0773(state') → buf'` ⇒ `memcmp(buf, buf') == 0`.
- Any 1-bit mutation of `buf` (outside the stored CRC field)
  causes `F0774(mutated)` to return `SAVEGAME_ERROR_BAD_CRC`.
- File-IO pair: `F0785(path, state)` then `F0786(path, &state')`
  then `F0773(state')` produces bytes identical to the first
  `F0773(state)`.

### Out of scope for Phase 15

Deferred to later phases / milestones:

- UI save-slot menu.
- Incremental save (diff since last save).
- Async save / background flush.
- Cloud save / platform storage abstraction.
- Savegame migration between format versions (we *reject* any
  version ≠ `SAVEGAME_FORMAT_VERSION`; a future upgrade phase
  will add a migration table).
- Compression (LZSS / zlib / Borland PAK etc.).
- Encryption or per-section "keys" (the obfuscation `Noise[]`
  array in Fontanel's `DM_SAVE_HEADER` is intentionally *not*
  reproduced — we're not shipping an obfuscation façade).
- Recovery from a corrupted-but-partially-readable save (we
  fail-fast on any integrity error; no "best effort" mode).
- A separate "backup save" slot (Fontanel's `L1364_B_LoadBackupGame`
  path) — v1 is single-file.
- Interop with the original DM/CSB `DMSAVE1.DAT` format. This is a
  **new** format for ReDMCSB; we share structure *inspiration* with
  the Fontanel layout (5 SAVE_PART sections → our 7 sections) but
  not bytes.

### Decision on dungeon state capture

Three candidates were considered:

| Option | Pros | Cons |
|--------|------|------|
| A. Full-dungeon-dump (copy entire `DungeonDatState_Compat` + `DungeonThings_Compat`) | Trivially correct; no mutation tracking | 200–400 KB per save; turns a 25 KB save into 400 KB; 99%+ of bytes never change |
| B. Delta-from-DUNGEON.DAT (Phase 15 recomputes the diff at save time) | Compact | Phase 15 would need to read DUNGEON.DAT at save time (violates "one IO helper" rule); also requires a deterministic diff function with stable ordering |
| C. Caller-supplied mutation list (caller records every mutation as it happens; Phase 15 just serialises that list verbatim) | Pure; small (<1 KB typical); no extra IO; matches how every other Phase-15 subsystem behaves | Requires callers in later phases to actually call the mutation recorder — but that's an *additive* contract, not a Phase 15 obligation |

**Decision: Option C.** Justification:

- Matches the existing Phase 10–14 pattern ("caller produces the
  blob, we just serialise it").
- Keeps Phase 15 free of any DUNGEON.DAT dependency (the probe
  can still load DUNGEON.DAT for smoke invariants, but the
  save/load core doesn't).
- Bit-identical round-trip is the test — it doesn't matter whether
  the list is "real" mutations or a synthetic fixture.
- Phase 15 defines the `DungeonMutation_Compat` struct + the
  serialiser; subsequent phases (door destruction, sensor
  toggle, group kill) wire the recorder in.

The v1 mutation list is bounded at **1024 entries** (see §2). In
practice DM1 produces ~50–200 mutations per dungeon floor across a
full playthrough — 1024 is ~5× the expected worst case and costs
~16 KB in the save when full.

---

## 2. Data structures

All sizes rounded to multiples of 4. Every `int` = 32-bit
(platform-checked via `_Static_assert(sizeof(int) == 4)`). Every
`uint32_t` is 4 bytes LE.

### `SaveGameSectionKind_Compat`

Fixed enum of section kinds. Hard-coded count = 7. These are the
`kind` values stored in each section-table entry; they also fix
**write order** (the table entries appear in this order in every
save file, and every load must see them in this order).

```
enum {
    SAVEGAME_SECTION_PARTY           = 0x00010001,
    SAVEGAME_SECTION_CHAMPIONS       = 0x00010002,  /* 4 × champion snapshot */
    SAVEGAME_SECTION_MOVEMENT        = 0x00010003,  /* party movement state */
    SAVEGAME_SECTION_SENSOR          = 0x00010004,  /* pending sensor effects */
    SAVEGAME_SECTION_TIMELINE        = 0x00010005,
    SAVEGAME_SECTION_COMBAT          = 0x00010006,  /* scratch: last CombatResult + RNG */
    SAVEGAME_SECTION_MAGIC           = 0x00010007,
    SAVEGAME_SECTION_DUNGEON_DELTA   = 0x00010008,
};

#define SAVEGAME_SECTION_COUNT   7   /* NB: DUNGEON_DELTA folded into COMBAT row via a separate entry — see §3 for the exact 7-entry mapping */
```

**7-section write order (locked at plan time — no renaming):**

| Slot | kind                              | Producer / consumer |
|------|-----------------------------------|---------------------|
| 0    | `SAVEGAME_SECTION_PARTY`          | `F0604_PARTY_Serialize_Compat` (Phase 10) |
| 1    | `SAVEGAME_SECTION_MOVEMENT`       | reserved for F0702 `MovementResult_Compat` (Phase 10) |
| 2    | `SAVEGAME_SECTION_SENSOR`         | `F0713_SENSOR_ListSerialize_Compat` (Phase 11) |
| 3    | `SAVEGAME_SECTION_TIMELINE`       | `F0727_TIMELINE_QueueSerialize_Compat` (Phase 12) |
| 4    | `SAVEGAME_SECTION_COMBAT`         | `F0742 + F0747a` combined (Phase 13 scratch) |
| 5    | `SAVEGAME_SECTION_MAGIC`          | `F0768a_MAGIC_MagicStateSerialize_Compat` (Phase 14) |
| 6    | `SAVEGAME_SECTION_DUNGEON_DELTA`  | Phase 15's own `F0782` |

Note: the `SAVEGAME_SECTION_CHAMPIONS` entry in the enum above is
**aliased** — `F0604` already writes all four champions as part of
the party blob, so Phase 15 does not emit a separate `CHAMPIONS`
section. The enum value is reserved for a future split.

### `SaveGameHeader_Compat` (64 bytes on disk)

```
struct SaveGameHeader_Compat {
    unsigned char magic[8];          /* "RDMCSB15", no NUL */
    uint32_t      formatVersion;     /* == SAVEGAME_FORMAT_VERSION (1) */
    uint32_t      endianSentinel;    /* == 0x01020304 */
    uint32_t      totalFileSize;     /* self-reported, validates on load */
    uint32_t      sectionCount;      /* == SAVEGAME_SECTION_COUNT (7) */
    uint32_t      bodyCRC32;         /* CRC32 of bytes [64 .. totalFileSize) */
    unsigned char reserved[36];      /* zeroed; ignored on load in v1 */
};
```

- Offsets (all LE):

  ```
  +0x00  magic[8]
  +0x08  formatVersion   (u32)
  +0x0C  endianSentinel  (u32)
  +0x10  totalFileSize   (u32)
  +0x14  sectionCount    (u32)
  +0x18  bodyCRC32       (u32)
  +0x1C  reserved[36]
  +0x40  = end of header (section table starts here)
  ```
- `SAVEGAME_HEADER_SERIALIZED_SIZE = 64`.

### `SaveGameSectionEntry_Compat` (16 bytes on disk)

```
struct SaveGameSectionEntry_Compat {
    uint32_t kind;          /* SAVEGAME_SECTION_* */
    uint32_t offset;        /* absolute offset from file start */
    uint32_t size;          /* payload length in bytes */
    uint32_t reserved;      /* == 0 in v1 */
};
```

- `SAVEGAME_SECTION_ENTRY_SERIALIZED_SIZE = 16`.
- Table total: `16 * 7 = 112` bytes, laid out at file offset `0x40`
  immediately after the header.

### `DungeonMutation_Compat` (32 bytes on disk)

Caller-produced log entry. Each entry records one atomic state
change in the dungeon. The save does NOT interpret the semantics
(that's the replay engine's job in a future phase); Phase 15 only
serialises the list.

```
#define DUNGEON_MUTATION_KIND_INVALID     0
#define DUNGEON_MUTATION_KIND_SQUARE_BYTE 1   /* raw map byte flipped */
#define DUNGEON_MUTATION_KIND_DOOR_STATE  2   /* door field changed */
#define DUNGEON_MUTATION_KIND_SENSOR_TOG  3   /* sensor toggle bit */
#define DUNGEON_MUTATION_KIND_GROUP_HP    4   /* DungeonGroup_Compat.health[k] */
#define DUNGEON_MUTATION_KIND_THING_LINK  5   /* squareFirstThings link changed */
#define DUNGEON_MUTATION_KIND_FIELD_GENERIC 6 /* catch-all: (type, index, fieldOff, newValue) */

struct DungeonMutation_Compat {
    int      kind;          /* DUNGEON_MUTATION_KIND_* */
    int      mapIndex;      /* 0..mapCount-1, or -1 if not cell-scoped */
    int      x;             /* 0..width-1, or -1 */
    int      y;             /* 0..height-1, or -1 */
    int      cell;          /* 0..3, or -1 */
    int      thingType;     /* 0..15, or -1 */
    int      thingIndex;    /* 0..count-1, or -1 */
    uint32_t beforeValue;   /* pre-mutation word (LE) */
    uint32_t afterValue;    /* post-mutation word (LE) */
    int      fieldMask;     /* bitmask identifying which field changed */
    int      reserved;
};
```

- 8 × int32 + 2 × uint32 + 1 × int32 = 11 fields × 4 = 44 bytes.
  Rounded to 11 × 4 = 44. But per-struct cache alignment convention
  we bump to **48** by padding `reserved1` at the end so offsets
  stay clean.

  **Locked layout (48 bytes, 12 × int32 LE):**

  ```
  +00 kind
  +04 mapIndex
  +08 x
  +12 y
  +16 cell
  +20 thingType
  +24 thingIndex
  +28 beforeValue
  +32 afterValue
  +36 fieldMask
  +40 reserved
  +44 reserved1
  ```

- `DUNGEON_MUTATION_SERIALIZED_SIZE = 48`.

### `DungeonMutationList_Compat`

```
#define DUNGEON_MUTATION_LIST_MAX_COUNT 1024

struct DungeonMutationList_Compat {
    int count;                                  /* 0..1024 */
    struct DungeonMutation_Compat entries[DUNGEON_MUTATION_LIST_MAX_COUNT];
};

/* Fixed-size layout: 4-byte count (LE) + 1024 × 48 bytes = 49156. */
#define DUNGEON_MUTATION_LIST_SERIALIZED_SIZE \
    (4 + DUNGEON_MUTATION_LIST_MAX_COUNT * DUNGEON_MUTATION_SERIALIZED_SIZE)
```

Unused entries are zero-filled on serialise. This fixed-size scheme
matches the `SensorEffectList_Compat` pattern from Phase 11 — same
pattern, same justification (simpler bounds checks on load).

### `CombatScratch_Compat`

Phase 13 exposes `CombatResult_Compat` + `RngState_Compat` + a
one-shot `WeaponProfile_Compat` that the combat resolver last used.
None of these are "combat *state*" in the persistent sense; they're
the scratch block that the caller needs after a reload to pick up
exactly where combat paused.

```
struct CombatScratch_Compat {
    struct CombatResult_Compat   lastResult;       /* Phase 13 F0742 / F0743 */
    struct WeaponProfile_Compat  lastWeapon;       /* Phase 13 F0747a / F0747b */
    struct RngState_Compat       rng;              /* Phase 13 F0730 etc. */
    int                           combatActive;     /* 1 if combat paused mid-turn */
    int                           reserved;
};

/* Layout: 56 (result) + 32 (weapon) + 4 (rng) + 4 + 4 = 100 bytes. */
#define COMBAT_SCRATCH_SERIALIZED_SIZE 100
```

### `SaveGame_Compat`

Composite in-memory struct. Points at every subsystem's current
runtime state. NO ownership — Phase 15 never allocates the
subsystem structs; it reads them (during save) or writes into
caller-provided instances (during load).

```
struct SaveGame_Compat {
    struct SaveGameHeader_Compat header;           /* written by Phase 15 */

    /* Section-0 PARTY (contains champions[4] + position + shields) */
    struct PartyState_Compat*           party;

    /* Section-1 MOVEMENT */
    struct MovementResult_Compat*       lastMovement;

    /* Section-2 SENSOR */
    struct SensorEffectList_Compat*     pendingSensorEffects;

    /* Section-3 TIMELINE */
    struct TimelineQueue_Compat*        timeline;

    /* Section-4 COMBAT (scratch) */
    struct CombatScratch_Compat*        combatScratch;

    /* Section-5 MAGIC */
    struct MagicState_Compat*           magic;

    /* Section-6 DUNGEON_DELTA */
    struct DungeonMutationList_Compat*  mutations;
};
```

No serialised-size macro for `SaveGame_Compat` itself — it's a
container of pointers, not a persistable struct. The *file* size
is computed during save via §4.1.

### MovementResult footprint

`MovementResult_Compat` (Phase 10) is 5 × int32 = 20 bytes. No
serialiser currently exists; this is the ONE case where Phase 15
needs a new pair. **ADDITIVE ONLY rule clarification:** we add the
pair as a file-local helper inside `memory_savegame_pc34_compat.c`,
named `F0776a_SAVEGAME_MovementResultSerialize_Compat` /
`F0776b_…Deserialize_Compat`. We do *not* touch
`memory_movement_pc34_compat.{h,c}`.

If during implementation the executor discovers that a
`MovementResult_Compat` serialiser already exists upstream (because
Phase 10 was extended between planning and execution), it **uses
the upstream pair and deletes the local helper**. This is the only
interface-drift allowance in this plan.

### Total file-size budget

Fixed sections:

| Section | Size (bytes) |
|---------|-------------:|
| Header  | 64 |
| Section table (7 × 16) | 112 |
| Party (PARTY_SERIALIZED_SIZE = 32 + 4×256) | 1056 |
| Movement (MOVEMENT_RESULT_SERIALIZED_SIZE) | 20 |
| Sensor list | 228 |
| Timeline queue | 11 272 |
| Combat scratch | 100 |
| Magic state | 72 |
| Dungeon-mutation list | 49 156 |
| **Total** | **62 080** |

≈ 61 KB. That's above the 24 KB soft target in the brief but
inside the 100 KB "usable on any disk" ceiling. The dominant
cost is the fixed-size mutation list; if this becomes a pain
point later, we swap in a variable-length encoding — but not in
v1 (KISS, avoid two independent framing schemes).

---

## 3. Function API (F0770 – F0789)

All functions live in `memory_savegame_pc34_compat.{h,c}`. All are
pure **except** F0785 / F0786 which wrap `fopen`/`fread`/`fwrite`.
Conventions carried forward:

- Input pointers: `const` always.
- Output pointers: non-`const`, caller-owned (including the byte
  buffer).
- Every validator returns a member of
  `enum SaveGameError_Compat` (see §3.0). `0` means
  `SAVEGAME_OK`. Non-zero means failure and the output is
  zero-initialised by the callee.

### 3.0 Error codes

```
enum SaveGameError_Compat {
    SAVEGAME_OK                      = 0,
    SAVEGAME_ERROR_NULL_ARG          = 1,
    SAVEGAME_ERROR_BUFFER_TOO_SMALL  = 2,
    SAVEGAME_ERROR_BAD_MAGIC         = 3,
    SAVEGAME_ERROR_BAD_VERSION       = 4,
    SAVEGAME_ERROR_BAD_ENDIAN        = 5,
    SAVEGAME_ERROR_BAD_SIZE          = 6,  /* totalFileSize mismatch */
    SAVEGAME_ERROR_BAD_CRC           = 7,
    SAVEGAME_ERROR_BAD_SECTION_COUNT = 8,
    SAVEGAME_ERROR_BAD_SECTION_KIND  = 9,
    SAVEGAME_ERROR_BAD_SECTION_ORDER = 10,
    SAVEGAME_ERROR_SECTION_OVERFLOW  = 11, /* section straddles EOF */
    SAVEGAME_ERROR_SECTION_OVERLAP   = 12,
    SAVEGAME_ERROR_SUBSYS_DESERIALIZE = 13, /* Phase 10-14 helper rejected the payload */
    SAVEGAME_ERROR_FILE_OPEN         = 14,
    SAVEGAME_ERROR_FILE_READ         = 15,
    SAVEGAME_ERROR_FILE_WRITE        = 16,
    SAVEGAME_ERROR_FILE_SIZE         = 17,
    SAVEGAME_ERROR_INTERNAL          = 99
};
```

`F0789_SAVEGAME_ErrorToString_Compat` returns a static string for
each.

### Group A — Checksum / header validation (F0770–F0772)

| # | Signature | Purity |
|---|-----------|--------|
| F0770 | `uint32_t F0770_SAVEGAME_CRC32_Compat(const unsigned char* buf, size_t len);` | Pure. IEEE 802.3 reflected polynomial `0xEDB88320`, init `0xFFFFFFFF`, final `XOR 0xFFFFFFFF`. Computes in one shot (no streaming state in v1 — we always have the whole body in memory). |
| F0771 | `int F0771_SAVEGAME_WriteHeader_Compat(struct SaveGameHeader_Compat* hdr, uint32_t totalSize, uint32_t bodyCRC);` | Pure. Fills magic/version/endianSentinel/totalFileSize/sectionCount=7/bodyCRC/reserved=0. Returns `SAVEGAME_OK` or `SAVEGAME_ERROR_NULL_ARG`. |
| F0772 | `int F0772_SAVEGAME_ValidateHeader_Compat(const struct SaveGameHeader_Compat* hdr, uint32_t actualBufferSize);` | Pure. Verifies magic, version, endian sentinel, sectionCount, totalFileSize == actualBufferSize. Does NOT verify CRC (that's done against body bytes inside F0774). Returns specific `SAVEGAME_ERROR_BAD_*` codes. |

### Group B — Top-level save/load (F0773–F0775)

| # | Signature | Purity |
|---|-----------|--------|
| F0773 | `int F0773_SAVEGAME_SaveToBuffer_Compat(const struct SaveGame_Compat* state, unsigned char* outBuf, int outBufSize, int* outBytesWritten);` | Pure. Walks the composite in-memory state, computes section offsets, invokes each subsystem serialiser into the buffer at the correct offset, writes the section table, then computes body CRC and writes the header. Returns `SAVEGAME_OK` on success. |
| F0774 | `int F0774_SAVEGAME_LoadFromBuffer_Compat(const unsigned char* buf, int bufSize, struct SaveGame_Compat* outState);` | Pure. Validates header (F0772), validates CRC, validates section table (F0784), then for each of the 7 sections invokes the matching subsystem deserialiser into `outState->*`. All `outState->*` pointers MUST be non-NULL and point to caller-owned destination structs (Phase 15 doesn't allocate). |
| F0775 | `int F0775_SAVEGAME_InitEmpty_Compat(struct SaveGame_Compat* state);` | Pure. Zeros `state->header`, leaves every subsystem pointer unchanged (caller-owned). Used by the probe to build a canonical zero-state fixture. |

### Group C — Per-section serialisers (F0776–F0782)

Each of these is a *thin wrapper* around an existing Phase 10–14
helper plus the exact-size check Phase 15 needs. NO field
manipulation beyond delegating to the existing helper. Input:
subsystem pointer; output: byte offset + size written.

| # | Section | Signature |
|---|---------|-----------|
| F0776 | PARTY     | `int F0776_SAVEGAME_SerializeParty_Compat(const struct PartyState_Compat* party, unsigned char* buf, int bufSize, int* outBytes);` — wraps `F0604_PARTY_Serialize_Compat`. Expected size: `PARTY_SERIALIZED_SIZE` = 1056. |
| F0777 | MOVEMENT  | `int F0777_SAVEGAME_SerializeMovement_Compat(const struct MovementResult_Compat* mv, unsigned char* buf, int bufSize, int* outBytes);` — uses the file-local helper `F0776a_…MovementResultSerialize_Compat`. Expected size: `MOVEMENT_RESULT_SERIALIZED_SIZE` = 20. |
| F0778 | SENSOR    | `int F0778_SAVEGAME_SerializeSensor_Compat(const struct SensorEffectList_Compat* s, unsigned char* buf, int bufSize, int* outBytes);` — wraps `F0713_SENSOR_ListSerialize_Compat`. Expected size: `SENSOR_EFFECT_LIST_SERIALIZED_SIZE` = 228. |
| F0779 | TIMELINE  | `int F0779_SAVEGAME_SerializeTimeline_Compat(const struct TimelineQueue_Compat* q, unsigned char* buf, int bufSize, int* outBytes);` — wraps `F0727_TIMELINE_QueueSerialize_Compat`. Expected size: `TIMELINE_QUEUE_SERIALIZED_SIZE` = 11272. |
| F0780 | COMBAT    | `int F0780_SAVEGAME_SerializeCombat_Compat(const struct CombatScratch_Compat* c, unsigned char* buf, int bufSize, int* outBytes);` — composes `F0742` + `F0747a` + a 4-byte write of `rng.seed` + two int32 writes. Expected size: 100. |
| F0781 | MAGIC     | `int F0781_SAVEGAME_SerializeMagic_Compat(const struct MagicState_Compat* m, unsigned char* buf, int bufSize, int* outBytes);` — wraps `F0768a_MAGIC_MagicStateSerialize_Compat`. Expected size: `MAGIC_STATE_SERIALIZED_SIZE` = 72. |
| F0782 | DUNGEON_DELTA | `int F0782_SAVEGAME_SerializeDungeonDelta_Compat(const struct DungeonMutationList_Compat* list, unsigned char* buf, int bufSize, int* outBytes);` — pure: writes `count` (LE u32) then `DUNGEON_MUTATION_LIST_MAX_COUNT` fixed-size entries (12 × int32 LE each), zeroing unused slots. Expected size: 49156. |

A mirrored `..._Deserialize_Compat` exists for each (same numbers
with suffix `b` for F0776/F0777/F0780/F0782 — the others delegate
to existing Phase 10–14 `_Deserialize` calls, no new numbering
needed).

### Group D — Composition + section-table helpers (F0783–F0784)

| # | Signature |
|---|-----------|
| F0783 | `int F0783_SAVEGAME_ComputeSectionTable_Compat(const struct SaveGame_Compat* state, struct SaveGameSectionEntry_Compat outTable[SAVEGAME_SECTION_COUNT], uint32_t* outTotalSize);` — Pure. Fills the 7-entry table, monotonically increasing offsets, each offset = sum of previous sections' sizes + header + table. Writes `*outTotalSize = header + table + sum_of_sections`. Determines each section's size by calling the subsystem-specific `_Serialize_Compat` into a local stack buffer and taking the byte count — avoids double-allocation by reusing the `*_SERIALIZED_SIZE` macro when it's a compile-time constant (all seven are). |
| F0784 | `int F0784_SAVEGAME_ValidateSectionTable_Compat(const struct SaveGameSectionEntry_Compat table[SAVEGAME_SECTION_COUNT], uint32_t totalFileSize);` — Pure. Checks: (a) each `kind` matches the expected-order sequence (§2); (b) offset_i + size_i <= offset_{i+1} (no overlap); (c) offset_0 == 64 + 112 = 176; (d) offset_6 + size_6 == totalFileSize; (e) `reserved == 0`. Returns `SAVEGAME_OK` or `SAVEGAME_ERROR_*`. |

### Group E — Filesystem wrapper (F0785–F0786)

**The only functions in Phase 15 that perform IO.** Both open via
`fopen`, fully read or fully write, then close. No partial writes,
no mmap.

| # | Signature |
|---|-----------|
| F0785 | `int F0785_SAVEGAME_SaveToFile_Compat(const char* path, const struct SaveGame_Compat* state);` — Allocates a single buffer of the pre-computed total size (via `F0783`), calls `F0773`, writes whole buffer to `path` with `fopen(path, "wb")`, closes, frees. Returns `SAVEGAME_OK` or `SAVEGAME_ERROR_FILE_*`. |
| F0786 | `int F0786_SAVEGAME_LoadFromFile_Compat(const char* path, struct SaveGame_Compat* outState);` — `fopen(path,"rb")`, `fseek`+`ftell` to get size, allocate, `fread`, close, call `F0774`, free. Returns `SAVEGAME_OK` on success. |

### Group F — Diagnostics (F0787–F0789)

| # | Signature | Purity |
|---|-----------|--------|
| F0787 | `int F0787_SAVEGAME_Compare_Compat(const unsigned char* bufA, int sizeA, const unsigned char* bufB, int sizeB, int* outFirstDiffOffset);` | Pure. `memcmp`-equivalent with `*outFirstDiffOffset` pointing at the first mismatched byte (or -1 if equal). Returns 1 iff bit-identical. |
| F0788 | `int F0788_SAVEGAME_InspectHeader_Compat(const unsigned char* buf, int bufSize, struct SaveGameHeader_Compat* outHdr);` | Pure. Tolerant "read header for display" — does NOT validate, returns whatever bytes are at offset 0..63 decoded. Probe uses it to print `magic_probe.md` data; load path still goes through F0772. |
| F0789 | `const char* F0789_SAVEGAME_ErrorToString_Compat(int code);` | Pure. Static string for each `SAVEGAME_ERROR_*`. |

### Pure / IO matrix

- **Pure** (no IO, no globals, no RNG): F0770, F0771, F0772, F0773,
  F0774, F0775, F0776–F0782, F0776a/b, F0783, F0784, F0787, F0788,
  F0789.
- **IO allowed** (single `fopen`/`fread`/`fwrite`/`fclose` each):
  F0785, F0786.
- **No RNG anywhere** in Phase 15 (save/load is deterministic
  serialisation; RNG comes through only as state bytes inside the
  COMBAT section).

---

## 4. Algorithm specifications

### 4.1 Save orchestration (`F0773_SAVEGAME_SaveToBuffer_Compat`)

Reference: Fontanel `F0433_STARTEND_ProcessCommand140_SaveGame_CPSCDF`
(LOADSAVE.C:550), which walks 5 SAVE_PART entries, runs XOR-key
obfuscation over each part, writes a header, computes per-part
checksums. Our v1 keeps the *shape* (table-driven walk) and drops
the obfuscation + per-part checksum (we use one whole-body CRC
instead).

```
pseudocode F0773(state, outBuf, outBufSize, outBytesWritten):
    if state==NULL || outBuf==NULL || outBytesWritten==NULL:
        return SAVEGAME_ERROR_NULL_ARG
    /* Pass 1: compute section table + total size */
    table[7] = {0}
    totalSize = 0
    err = F0783(state, table, &totalSize)
    if err != SAVEGAME_OK: return err
    if (int)totalSize > outBufSize:
        return SAVEGAME_ERROR_BUFFER_TOO_SMALL

    /* Pass 2: write body (sections in fixed order) */
    memset(outBuf, 0, totalSize)          /* determinism in reserved areas */
    for slot in 0..6:
        sizeWritten = 0
        err = serialise_slot(slot, state, outBuf + table[slot].offset,
                             (int)table[slot].size, &sizeWritten)
        if err != SAVEGAME_OK: return err
        if (uint32_t)sizeWritten != table[slot].size:
            return SAVEGAME_ERROR_INTERNAL
    /* Section table */
    for slot in 0..6:
        write_u32_le(outBuf + 64 + slot*16 + 0,  table[slot].kind)
        write_u32_le(outBuf + 64 + slot*16 + 4,  table[slot].offset)
        write_u32_le(outBuf + 64 + slot*16 + 8,  table[slot].size)
        write_u32_le(outBuf + 64 + slot*16 + 12, 0)

    /* Pass 3: body CRC + header */
    bodyCRC = F0770(outBuf + 64, totalSize - 64)
    struct SaveGameHeader_Compat hdr
    F0771(&hdr, totalSize, bodyCRC)
    write_header_to_buf(outBuf, &hdr)     /* inline writer, mirrors §2 offsets */

    *outBytesWritten = (int)totalSize
    return SAVEGAME_OK
```

`serialise_slot()` is a small dispatch that invokes
`F0776..F0782` based on the slot index.

### 4.2 Load orchestration (`F0774_SAVEGAME_LoadFromBuffer_Compat`)

Reference: Fontanel `F0435_STARTEND_LoadGame` (LOADSAVE.C:2192) —
5 SAVE_PART reads, per-part key check, per-part checksum validate,
finally consume GLOBAL_DATA into globals. We inline the same
structure but centralise validation.

```
pseudocode F0774(buf, bufSize, outState):
    if buf==NULL || outState==NULL: return NULL_ARG
    if bufSize < 64: return BUFFER_TOO_SMALL

    /* Phase 1: header */
    SaveGameHeader_Compat hdr
    read_header_from_buf(buf, &hdr)
    err = F0772(&hdr, (uint32_t)bufSize)
    if err != OK: return err

    /* Phase 2: CRC */
    uint32_t crc = F0770(buf + 64, hdr.totalFileSize - 64)
    if crc != hdr.bodyCRC: return BAD_CRC

    /* Phase 3: section table */
    SaveGameSectionEntry_Compat table[7]
    for slot in 0..6:
        table[slot].kind     = read_u32_le(buf + 64 + slot*16)
        table[slot].offset   = read_u32_le(buf + 64 + slot*16 + 4)
        table[slot].size     = read_u32_le(buf + 64 + slot*16 + 8)
        table[slot].reserved = read_u32_le(buf + 64 + slot*16 + 12)
    err = F0784(table, hdr.totalFileSize)
    if err != OK: return err

    /* Phase 4: for each section, delegate */
    for slot in 0..6:
        const unsigned char* p = buf + table[slot].offset
        int n = (int)table[slot].size
        if (uint32_t)(p - buf) + (uint32_t)n > hdr.totalFileSize:
            return SECTION_OVERFLOW
        err = deserialise_slot(slot, p, n, outState)
        if err != OK: return SUBSYS_DESERIALIZE     /* NB: preserve specific code via out-param in v2 */

    /* Phase 5: stamp header into outState for round-trip parity */
    outState->header = hdr
    return OK
```

### 4.3 CRC32 (`F0770`)

IEEE 802.3 reflected, no lookup table in v1 (bit-by-bit for
portability; we can promote to a 256-entry table later if perf
matters — unlikely for 62 KB saves). Reference — standard
implementation:

```
pseudocode F0770(buf, len):
    c = 0xFFFFFFFF
    for i in 0..len-1:
        c = c ^ buf[i]
        for k in 0..7:
            c = (c >> 1) ^ (0xEDB88320 & -(c & 1))
    return c ^ 0xFFFFFFFF
```

Validated against two known vectors in §5 (invariants #1–#2).

### 4.4 `F0783` section table computation

```
pseudocode F0783(state, outTable, outTotalSize):
    /* Sizes are compile-time constants except DUNGEON_DELTA + COMBAT
       + MOVEMENT + SENSOR which are all ALSO compile-time constants
       (fixed-length lists). So no serialise-into-scratch pass needed. */
    const uint32_t sizes[7] = {
        PARTY_SERIALIZED_SIZE,
        MOVEMENT_RESULT_SERIALIZED_SIZE,
        SENSOR_EFFECT_LIST_SERIALIZED_SIZE,
        TIMELINE_QUEUE_SERIALIZED_SIZE,
        COMBAT_SCRATCH_SERIALIZED_SIZE,
        MAGIC_STATE_SERIALIZED_SIZE,
        DUNGEON_MUTATION_LIST_SERIALIZED_SIZE,
    };
    const uint32_t kinds[7] = { SAVEGAME_SECTION_PARTY, ..., SAVEGAME_SECTION_DUNGEON_DELTA };
    uint32_t off = 64 + 7*16;      /* 176 */
    for slot in 0..6:
        outTable[slot].kind = kinds[slot]
        outTable[slot].offset = off
        outTable[slot].size = sizes[slot]
        outTable[slot].reserved = 0
        off += sizes[slot]
    *outTotalSize = off
    return OK
```

### 4.5 File-IO (`F0785` / `F0786`)

```
pseudocode F0785(path, state):
    uint32_t total = 64 + 7*16 + SUM(sizes[])
    unsigned char* buf = malloc(total)
    if !buf: return INTERNAL
    int written
    err = F0773(state, buf, (int)total, &written)
    if err != OK: free(buf); return err
    FILE* f = fopen(path, "wb")
    if !f: free(buf); return FILE_OPEN
    size_t w = fwrite(buf, 1, total, f)
    fclose(f)
    free(buf)
    return (w == total) ? OK : FILE_WRITE

pseudocode F0786(path, outState):
    FILE* f = fopen(path, "rb"); if !f: return FILE_OPEN
    fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET)
    if sz <= 0 || sz > SAVEGAME_MAX_FILE_SIZE: fclose(f); return FILE_SIZE
    unsigned char* buf = malloc((size_t)sz)
    size_t r = fread(buf, 1, (size_t)sz, f); fclose(f)
    if r != (size_t)sz: free(buf); return FILE_READ
    err = F0774(buf, (int)sz, outState)
    free(buf)
    return err
```

`SAVEGAME_MAX_FILE_SIZE` is `1 << 20` (1 MiB) — plenty of headroom
vs. the 62 KB we produce.

### 4.6 Error-handling contract

Every entry point returns a `SaveGameError_Compat` code — **no
`abort()`, no `assert()` on untrusted input, no UB on malformed
data**. This is Phase 15's single most important guarantee: an
arbitrary attacker-supplied buffer must not crash the loader.

Specifically:

- Length-prefixed fields are validated against remaining buffer
  before read.
- Section offset+size arithmetic is done in `uint64_t` to avoid
  wraparound on 32-bit `uint32_t` math.
- Every subsystem deserialiser (Phase 10–14) already returns
  success/failure; we propagate failure as
  `SAVEGAME_ERROR_SUBSYS_DESERIALIZE` and include the failing
  slot index in a `lastFailingSection` sidecar field on
  `SaveGame_Compat` for diagnostics (not serialised).

### 4.7 Fontanel mapping

| Fontanel                                       | Phase 15            |
|------------------------------------------------|---------------------|
| `F0433_STARTEND_ProcessCommand140_SaveGame`    | F0773 / F0785       |
| `F0435_STARTEND_LoadGame`                      | F0774 / F0786       |
| `SAVE_PART` table                              | §2 `sections[7]`    |
| `DM_SAVE_HEADER.Noise[]` obfuscation           | **DROPPED** (v1)    |
| `DM_SAVE_HEADER.Checksums[16]`                 | Replaced by single `bodyCRC32` in our header |
| `DM_SAVE_HEADER.GameID`                        | **DEFERRED** — placeholder sits inside `reserved[36]` for v2 |
| `GLOBAL_DATA`                                  | spread across our PARTY + TIMELINE + COMBAT sections |
| `ACTIVE_GROUP`                                 | covered by our DUNGEON_DELTA section (group HP mutations) |
| `C0_SAVE_PART_GLOBAL_DATA` … `C4_SAVE_PART_TIMELINE` | our 7 sections (expanded to separate magic + sensor + movement) |

### 4.8 Items NEEDING DISASSEMBLY REVIEW (documented, not fabricated)

1. `DM_SAVE_HEADER.Keys[16]` / `Checksums[16]` are per-part
   obfuscation masks. The exact XOR-key derivation from
   `Noise[10]` is in LOADSAVE.C:550+ but we intentionally do not
   re-implement it (we're not trying to produce format-compatible
   saves). Tagged inline `/* NEEDS DISASSEMBLY REVIEW if we ever
   want DMSAVE1.DAT interop. v1: new format, no Fontanel parity. */`.
2. Fontanel's `aUnreferenced` / `Useless` padding fields (BUG0_00
   in DEFS.H:471, 486, 561–571) are intentionally *not* mirrored
   — our header is clean.
3. `MusicOn` in `GLOBAL_DATA` (MEDIA505 conditional) — not a
   runtime state field in ReDMCSB (no audio yet). Tagged
   `NEEDS DISASSEMBLY REVIEW if audio phase adds a music-state
   field later.`

---

## 5. Invariant list (target 40)

Total planned: **40 invariants**. The verify gate requires ≥ 30 —
we ship with a 10-margin. Probe prints
`Invariant count: 40` + `Status: PASS`.

| # | Category | Invariant |
|---|----------|-----------|
| 1  | crc | `F0770("", 0) == 0x00000000` (empty buffer → 0xFFFFFFFF ^ 0xFFFFFFFF = 0) |
| 2  | crc | `F0770("123456789", 9) == 0xCBF43926` (standard CRC32 test vector) |
| 3  | size | `SAVEGAME_HEADER_SERIALIZED_SIZE == 64` |
| 4  | size | `SAVEGAME_SECTION_ENTRY_SERIALIZED_SIZE == 16` |
| 5  | size | `SAVEGAME_SECTION_COUNT == 7` |
| 6  | size | `DUNGEON_MUTATION_SERIALIZED_SIZE == 48` |
| 7  | size | `DUNGEON_MUTATION_LIST_SERIALIZED_SIZE == 4 + 1024*48 == 49156` |
| 8  | size | `COMBAT_SCRATCH_SERIALIZED_SIZE == 100` |
| 9  | size | `MOVEMENT_RESULT_SERIALIZED_SIZE == 20` |
| 10 | size | `sizeof(int) == 4 && sizeof(uint32_t) == 4` |
| 11 | header | `F0771` writes `magic == "RDMCSB15"` at offset 0..7 |
| 12 | header | `F0771(&hdr, 176, 0xDEADBEEF)` followed by a buffer walk: `hdr.formatVersion == 1`, `hdr.endianSentinel == 0x01020304`, `hdr.totalFileSize == 176`, `hdr.sectionCount == 7`, `hdr.bodyCRC32 == 0xDEADBEEF`, `hdr.reserved[] all zero` |
| 13 | header | `F0772(valid)` returns `SAVEGAME_OK` |
| 14 | header | `F0772(hdr with magic='X' at byte 0)` returns `SAVEGAME_ERROR_BAD_MAGIC` |
| 15 | header | `F0772(hdr with formatVersion=2)` returns `SAVEGAME_ERROR_BAD_VERSION` |
| 16 | header | `F0772(hdr with endianSentinel=0x04030201)` returns `SAVEGAME_ERROR_BAD_ENDIAN` |
| 17 | header | `F0772(hdr with totalFileSize=99)` + `actualBufferSize=62080` returns `SAVEGAME_ERROR_BAD_SIZE` |
| 18 | header | `F0772(hdr with sectionCount=6)` returns `SAVEGAME_ERROR_BAD_SECTION_COUNT` |
| 19 | section-table | `F0783` on a zero-state SaveGame produces offsets `176, 1232, 1252, 1480, 12752, 12852, 12924` in slots 0..6 and `*outTotalSize == 62080` |
| 20 | section-table | `F0784` on the F0783-produced table returns `SAVEGAME_OK` |
| 21 | section-table | `F0784` with table[2].offset rewritten to overlap table[1] returns `SAVEGAME_ERROR_SECTION_OVERLAP` |
| 22 | section-table | `F0784` with table[0].kind rewritten to `0xCAFE` returns `SAVEGAME_ERROR_BAD_SECTION_KIND` |
| 23 | composite-rt | Build a **zero-state** `SaveGame_Compat` (fresh init of every subsystem), save to buf A, load into fresh `SaveGame2`, save to buf B: `F0787(A, B) == 1` and first-diff-offset == -1. |
| 24 | composite-rt | Build a **populated** `SaveGame_Compat`: 2 champions in party, 37 timeline events, 5 sensor effects, 8 dungeon mutations, combat scratch with known RNG seed, magic state with non-zero shields — round-trip: `F0787(A, B) == 1`. |
| 25 | per-subsys-rt | After load, `party` field-by-field equals original (`memcmp` over `PartyState_Compat`, 4 champions included). |
| 26 | per-subsys-rt | After load, `timeline` field-by-field equals original (count + every event). |
| 27 | per-subsys-rt | After load, `sensorEffects` field-by-field equals original. |
| 28 | per-subsys-rt | After load, `combatScratch` field-by-field equals original (result + weapon + rng seed). |
| 29 | per-subsys-rt | After load, `magic` field-by-field equals original (every shield + every event counter). |
| 30 | per-subsys-rt | After load, `mutations` field-by-field equals original (count + entries[0..count-1]; entries[count..1023] all zero). |
| 31 | tamper | Save populated state to buf. Flip bit 0 of byte `64 + 7*16 + 100` (inside the PARTY section). Call `F0774(mutated)`. Expect `SAVEGAME_ERROR_BAD_CRC`. |
| 32 | tamper | Save populated state. Flip bit in the CRC field itself (byte `0x18`). Call `F0774`. Expect `SAVEGAME_ERROR_BAD_CRC`. |
| 33 | truncation | Save to buf, call `F0774(buf, 32)` (truncated inside header). Expect `SAVEGAME_ERROR_BUFFER_TOO_SMALL`. |
| 34 | truncation | Save to buf, truncate to 100 bytes (inside section table). Call `F0774`. Expect `SAVEGAME_ERROR_BAD_SIZE`. |
| 35 | truncation | Save to buf, truncate to `64 + 112 + 500` (mid-PARTY section). Call `F0774`. Expect `SAVEGAME_ERROR_BAD_SIZE`. |
| 36 | version-skew | Construct a buf with `formatVersion == 0xFFFF0001` (future). `F0774` → `SAVEGAME_ERROR_BAD_VERSION`. |
| 37 | null-safety | `F0773(NULL, buf, size, &n)` → `SAVEGAME_ERROR_NULL_ARG`. |
| 38 | null-safety | `F0774(NULL, size, state)` → `SAVEGAME_ERROR_NULL_ARG`. |
| 39 | null-safety | `F0770(NULL, 0) == 0` (defined as empty-buffer CRC, NO deref). |
| 40 | null-safety | `F0789(-1)` returns a non-NULL diagnostic string (e.g. `"unknown"`). |
| 41 | file-io | `F0785("/tmp/phase15-test.dat", populated)` returns `OK`; `F0786(same path, &loaded)` returns `OK`; save `loaded` to buf B; `memcmp(bufA, bufB) == 0`. Remove `/tmp/phase15-test.dat` at end. |
| 42 | edge-zero | Round-trip a `SaveGame_Compat` where every subsystem is init-only (empty queues, 0 mutations, RNG seed = 0). Result `memcmp == 0`. |
| 43 | edge-max | Round-trip a `SaveGame_Compat` where the timeline queue holds exactly 256 events (full). Every event has a unique `fireAtTick`. Verify after load: `timeline.count == 256` and events[255].fireAtTick matches. |
| 44 | edge-max | Round-trip a `SaveGame_Compat` where `mutations.count == 1024` (list full). Verify after load that entries[0] and entries[1023] match original. |
| 45 | integration | After F0774 restores a populated save into `loaded`, invoke Phase 13 `F0736_COMBAT_ResolveCreatureMelee_Compat` with a canned creature attacker against `loaded.party.champions[0]` (via a built `CombatantChampionSnapshot_Compat`), using `loaded.combatScratch.rng`. Compare result with the same call done against the PRE-save state. Outcome (hit / miss / damage / rng-seed-after) must be **bit-identical**. This is the only "run game code post-load" invariant and proves the save is semantically live, not just byte-matching. |

*Probe renumbers to a contiguous 1..40 (invariants 41–45 above
collapse the `edge-max` / `file-io` / `integration` pairs into a
single line each where possible).* The file-io, edge-zero,
edge-max-timeline, edge-max-mutations, and integration entries are
the five mandatory "shaping" invariants from the brief — **all
five ship**. No stub or "FIXME later" — if any of the five can't
pass on its own, the phase is not done.

Probe output footer:

```
Invariant count: 40
Status: PASS
```

---

## 6. Implementation order for the Codex executor

Strict linear sequence. Compile after every step. No renames
mid-task. If step N fails, fix before step N+1.

### Step 1 — Write `memory_savegame_pc34_compat.h`

- All `#define` constants from §2 + §3.0 (enum).
- `SaveGameHeader_Compat`, `SaveGameSectionEntry_Compat`,
  `DungeonMutation_Compat`, `DungeonMutationList_Compat`,
  `CombatScratch_Compat`, `SaveGame_Compat`.
- Function prototypes F0770–F0789 (plus the four `_b` /
  `_Deserialize` mirrors).
- Includes: `<stddef.h>`, `<stdint.h>`, `<stdio.h>` (for `FILE*`
  in F0785/F0786 — NOT used in any pure function),
  `"memory_champion_state_pc34_compat.h"`,
  `"memory_movement_pc34_compat.h"`,
  `"memory_sensor_execution_pc34_compat.h"`,
  `"memory_timeline_pc34_compat.h"`,
  `"memory_combat_pc34_compat.h"`,
  `"memory_magic_pc34_compat.h"`.

Syntax smoke:
```
cc -Wall -Wextra -c -o /tmp/savegame_h_check.o -x c \
    <(printf '#include "memory_savegame_pc34_compat.h"\nint main(void){return 0;}') \
    -I/Users/bosse/.openclaw/workspace-main/tmp/firestaff
```
Must compile clean (zero warnings).

### Step 2 — Write `.c` stub

- Includes + every function returning
  `SAVEGAME_ERROR_INTERNAL` / zero.
- `_Static_assert(sizeof(int) == 4, "int32 assumption");`
  `_Static_assert(sizeof(uint32_t) == 4, "uint32 assumption");`
  at top-of-file.
- Static CRC-initial constant.
- Build with `-Wall -Wextra -O1`. Zero warnings — note that
  `-Wunused-parameter` may fire on stubs; suppress with
  `(void)param;` rather than `__attribute__((unused))` for C99
  portability.

### Step 3 — Implement Group A (F0770–F0772)

- F0770: bit-by-bit CRC32. Verify against invariants #1–#2
  *before* wiring into save orchestration.
- F0771: simple writer.
- F0772: field-by-field validator with specific error codes.

Compile & smoke — write a 20-line scratch main that runs CRC on
"123456789" and prints hex.

### Step 4 — Implement Group C (F0776–F0782) as thin wrappers

- For each section, a 10-line wrapper:
  ```
  int F0776_...(const PartyState_Compat* p, unsigned char* buf, int size, int* out) {
      if (!p||!buf||!out) return SAVEGAME_ERROR_NULL_ARG;
      if (size < PARTY_SERIALIZED_SIZE) return SAVEGAME_ERROR_BUFFER_TOO_SMALL;
      int n = F0604_PARTY_Serialize_Compat(p, buf, size);
      if (n < 0 || n != PARTY_SERIALIZED_SIZE) return SAVEGAME_ERROR_SUBSYS_DESERIALIZE;
      *out = n;
      return SAVEGAME_OK;
  }
  ```
- For F0777 (MOVEMENT) and F0780 (COMBAT) and F0782
  (DUNGEON_DELTA), the delegation is a new pair — implement
  them here:
  - F0776a/b for MovementResult: 5 × int32 LE.
  - F0780 / F0780b for CombatScratch: delegates to F0742 (56B)
    + F0747a (32B) + 1 × u32 (rng seed, 4B) + 2 × int32 (combat
    active + reserved, 8B) = 100 B total.
  - F0782 / F0782b for DungeonMutationList: writes
    `count` LE u32 + 1024 × (12 × int32 LE). Unused slots zeroed.

Compile & smoke.

### Step 5 — Implement Group D (F0783 / F0784)

- F0783: table-of-constants walker (§4.4).
- F0784: expected-kinds array, overlap check, total-size check.

Compile & smoke.

### Step 6 — Implement Group B (F0773 / F0774 / F0775)

- F0773 per §4.1.
- F0774 per §4.2.
- F0775: 3-line zeroiser of `state->header`.

Compile & smoke — a scratch main that:
1. builds a tiny state (zero-init every subsystem),
2. calls F0773 into a 64K buffer,
3. calls F0774 from that buffer into a second state,
4. calls F0773 on state #2 into a second buffer,
5. `memcmp`s the two buffers → must be 0.

### Step 7 — Implement Group E + F (F0785–F0789)

- F0785 / F0786: per §4.5.
- F0787: `memcmp`-wrapper with first-diff reporting.
- F0788: tolerant header-read (no validation).
- F0789: `switch` returning a const string per code.

### Step 8 — Write `firestaff_m10_savegame_probe.c`

- Scaffold from `firestaff_m10_combat_probe.c`.
- Opens `savegame_probe.md` + `savegame_invariants.md`.
- `CHECK(cond, name)` macro.
- Argv: `$1 = DUNGEON.DAT` (unused by save/load core, but probe
  uses it to build a canonical party/things fixture for
  invariant #45); `$2 = output dir`.

### Step 9 — Add invariants incrementally

- **Block A (sizes + CRC, 1–10):** compile-time + CRC vectors.
- **Block B (header, 11–18):** F0771/F0772 exercise.
- **Block C (section-table, 19–22):** F0783/F0784.
- **Block D (composite round-trip, 23–24):** zero + populated.
- **Block E (per-subsys round-trip, 25–30):** individual equality.
- **Block F (tamper + truncation + version, 31–36):**
  deliberately-corrupted buffers.
- **Block G (null safety, 37–40):** NULL inputs.
- **Block H (file-io, 41):** `/tmp/phase15-test.dat`. Delete
  post-check.
- **Block I (edge-zero + edge-max, 42–44):** full timeline + full
  mutation list.
- **Block J (integration, 45):** call Phase 13 `F0736` against
  loaded snapshot. **This is the cross-phase canary** — run the
  full verify script after this block, not just the Phase 15
  probe, before moving on.

### Step 10 — Driver script `run_firestaff_m10_savegame_probe.sh`

- Mirror `run_firestaff_m10_combat_probe.sh`. Two argv:
  `$1 = DUNGEON.DAT`, `$2 = output dir`.
- Compile command:
  ```
  cc -Wall -Wextra -O1 -I"$ROOT" \
      -o "$PROBE_BIN" \
      "$ROOT/firestaff_m10_savegame_probe.c" \
      "$ROOT/memory_savegame_pc34_compat.c" \
      "$ROOT/memory_magic_pc34_compat.c" \
      "$ROOT/memory_combat_pc34_compat.c" \
      "$ROOT/memory_timeline_pc34_compat.c" \
      "$ROOT/memory_sensor_execution_pc34_compat.c" \
      "$ROOT/memory_movement_pc34_compat.c" \
      "$ROOT/memory_champion_state_pc34_compat.c" \
      "$ROOT/memory_dungeon_dat_pc34_compat.c"
  ```
- Invoke `"$PROBE_BIN" "$1" "$2"`.
- `chmod +x`.

### Step 11 — Append to `run_firestaff_m10_verify.sh`

**Pre-check:**
```
grep -c '^# Phase 15:' run_firestaff_m10_verify.sh    # must be 0
```
If 1+: `git checkout tmp/firestaff/run_firestaff_m10_verify.sh`
before editing.

Insert the new block IMMEDIATELY BEFORE the line
`echo "=== M10 verification complete ==="`:

```
# Phase 15: Save / Load system probe
echo "=== Phase 15: M10 savegame probe ==="
SAVEGAME_DIR="$OUT_DIR/savegame"
"$ROOT/tmp/firestaff/run_firestaff_m10_savegame_probe.sh" "$DUNGEON_DAT" "$SAVEGAME_DIR" || {
    echo "FAIL: M10 savegame probe did not pass"
    exit 1
}
echo "M10 savegame probe: PASS"

python3 - <<'PY' "$SAVEGAME_DIR/savegame_invariants.md" "$SUMMARY_MD" "$SAVEGAME_DIR"
from pathlib import Path
import sys
inv_path = Path(sys.argv[1])
out = Path(sys.argv[2])
subdir = Path(sys.argv[3])
inv = inv_path.read_text(encoding='utf-8')
text = out.read_text(encoding='utf-8')
failures = []
for required in ['savegame_probe.md', 'savegame_invariants.md']:
    if not (subdir / required).exists():
        failures.append(f'savegame: missing artifact {required}')
if 'Status: PASS' not in inv:
    failures.append('savegame: invariant status is not PASS')
if failures:
    text += '\n## M10 savegame check: FAIL\n\n'
    for item in failures:
        text += f'- {item}\n'
    text += '\n'
    text += inv
    out.write_text(text, encoding='utf-8')
    raise SystemExit(1)
text += '\n## M10 savegame check: PASS\n\n'
text += '- artifact present: savegame_probe.md\n'
text += '- artifact present: savegame_invariants.md\n'
for line in inv.splitlines():
    if line.startswith('- '):
        text += line + '\n'
out.write_text(text, encoding='utf-8')
PY
```

**Post-check:**
```
grep -c '^# Phase 15:' run_firestaff_m10_verify.sh   # must be 1
grep -c '^# Phase 14:' run_firestaff_m10_verify.sh   # must still be 1
grep -c '^# Phase 13:' run_firestaff_m10_verify.sh   # must still be 1
```
If any of these fails: `git checkout` the script and retry from
the pre-check.

### Step 12 — Full verify

```
bash run_firestaff_m10_verify.sh \
    /Users/bosse/.openclaw/data/redmcsb-original/DungeonMasterPC34/DATA/DUNGEON.DAT \
    /tmp/m10-verify-out
```
Exit must be 0. All 15 phases (1..14 unchanged + new 15) PASS.

If any earlier phase regresses: `git diff` every file not in §7
and revert.

---

## 7. Files to create + modify

| Path | Action | Estimated size |
|------|--------|----------------|
| `tmp/firestaff/memory_savegame_pc34_compat.h` | CREATE | ~7 KB (~260 lines) |
| `tmp/firestaff/memory_savegame_pc34_compat.c` | CREATE | ~18 KB (~650 lines; CRC + 20 functions + the `DungeonMutation` writer is the longest routine at ~60 lines) |
| `tmp/firestaff/firestaff_m10_savegame_probe.c` | CREATE | ~26 KB (~820 lines; 40 invariants × ~18 lines + fixture builders + the Phase-13 integration harness in invariant 45) |
| `tmp/firestaff/run_firestaff_m10_savegame_probe.sh` | CREATE | ~1.0 KB (~32 lines; one more source file in the compile list than earlier phases) |
| `tmp/firestaff/run_firestaff_m10_verify.sh` | MODIFY | +34 lines appended exactly once |

No other files touched. No `.phase*-attempt-*` backups created.
No edits to any Phase 10–14 source.

---

## 8. Risk register

| # | Risk | Likelihood | Impact | Plan B |
|---|------|------------|--------|--------|
| R1 | **Section-offset math errors.** Offset sums computed separately in F0783 (save) and F0784 (load) — if the expected-size constants drift, offsets silently diverge and load picks up the wrong section. | Medium (this is the whole-of-Phase-15 arithmetic foundation) | Very high — corrupts the whole save | Invariant #19 nails all 7 offsets by exact value. Every size constant is `_Static_assert`-ed against its subsystem's own macro in the `.c` file's top. F0784 explicitly re-computes offsets and compares against the on-disk table (not just checks monotonicity). |
| R2 | **Verify-script duplicate append.** Burnt multiple cycles in Phases 12, 13, 14. Still the top repeat risk. | Medium | High (test gate silently passes on duplicate) | Step 11 mandates pre-check (`grep -c == 0`) AND post-check (`grep -c == 1` for 13, 14, 15 each). On any failure: `git checkout tmp/firestaff/run_firestaff_m10_verify.sh` then retry. No exceptions. |
| R3 | **Phase 14 interface drift.** Phase 14 is in flight as this plan is written. If the implementer renames `MAGIC_STATE_SERIALIZED_SIZE` or adds a new field to `MagicState_Compat`, our Phase 15 COMBAT section offsets shift. | Medium | High | **Implementer rule (non-negotiable):** the executor reads `memory_magic_pc34_compat.h` AS-MERGED, NOT this plan, for every size constant. All size usages in Phase 15 go via `MAGIC_STATE_SERIALIZED_SIZE` (the macro), never via a hand-typed `72`. If the macro doesn't exist with that name → STOP and raise a review request. Same rule for every Phase 10–14 size macro. |
| R4 | **Endianness assumption.** We claim LE-everywhere; host is ARM64 macOS (LE); target disk is LE. A future build on big-endian would silently produce garbage. | Very low (no BE target planned) | High on a hypothetical BE host | Explicit `endianSentinel = 0x01020304` field in header. `F0772` rejects any mismatch. `_Static_assert((union{uint32_t u; unsigned char c[4];}){.u=1}.c[0] == 1, "LE required")` at top of `.c` (via a tiny runtime check if static version is rejected by strict C99). |
| R5 | **/tmp permissions on target host.** The file-IO invariant writes `/tmp/phase15-test.dat`. If `/tmp` is read-only in the sandbox, F0785 fails. | Low (macOS + Linux + CI all allow /tmp writes) | Medium (only invariant #41 would fail) | Probe computes its temp path as `$2/phase15-test.dat` where `$2` is the output dir — same directory as `savegame_probe.md`. The driver script already passes a fresh dir there. Invariant #41 uses that path, not a hard-coded `/tmp`. |
| R6 | **Buffer-overflow on hostile input.** Malformed section table with wrapped-around offsets. | Medium (classic format-parser bug) | Critical (arbitrary read) | §4.2 mandates `uint64_t` arithmetic for `offset + size` before comparing against `totalFileSize`. Dedicated invariants: #33/#34/#35 (truncation) + #21 (overlap) + one fuzz-style case that sets `offset = 0xFFFFFFF0, size = 0x100` — expected `SECTION_OVERFLOW`. |
| R7 | **CombatScratch 100-byte layout wrong.** Custom composition of 56 + 32 + 4 + 8 that isn't on any existing serialiser path. | Medium | Medium (invariant 28 fails) | Invariant 28 tests `memcmp` of the *entire* `CombatScratch_Compat` after round-trip, which is a strict equality check. The layout is documented in §2; if the equality fails, the executor prints both structs byte-by-byte into `savegame_probe.md` for diff. |
| R8 | **Fixed-size 49156-byte dungeon-mutation list vastly inflates the save.** User-visible save size = ~61 KB even for a fresh game with 0 mutations. | Certain | Low (design choice, not a bug) | Documented in §2 budget table. Swap to variable-length framing in v2 if we ever care about disk footprint — v1 prefers simplicity and fixed offsets. |
| R9 | **CRC32 collision on tamper.** 1-in-4-billion chance a random bit flip produces a valid CRC. | Negligible (2⁻³²) | Test flake risk | Invariants 31/32 flip *deterministic* bits (byte 0 bit 0 and the CRC-field bit 0); neither is a random collision candidate. |
| R10 | **90-min executor budget overrun.** Phase 15 has 40 invariants + a 100-byte custom-struct serialiser + a file-IO path. Heavier than Phase 14. | Medium | Schedule only | Incremental blocks A…J in Step 9. If time tight, blocks H (file-io), I (edge-max), J (integration) are the tail — blocks A–G alone ship 36 invariants (≥ 30 gate). But the brief explicitly mandates file-io + edge-max + integration — so we budget for all of them. **Realistic target: ~16 min** (more code than Phase 13 at 12 min, less math than Phase 14 at 18 min). |

**Top two risks by expected cost:**
**R2 (verify-script duplicate)** — because it has literally bitten
every prior phase's first attempt; and **R3 (Phase 14 interface
drift)** — because Phase 15 is the first phase with hard build-time
dependencies on every single prior phase's size constants.

---

## 9. Acceptance criteria

Phase 15 is complete and ready for merge when ALL of the following
hold:

- [ ] `bash run_firestaff_m10_verify.sh <dungeon.dat> <out>` exits 0.
- [ ] `grep -c '^# Phase 15:' run_firestaff_m10_verify.sh` equals
      exactly **1**.
- [ ] `grep -c '^# Phase 14:' run_firestaff_m10_verify.sh` still
      equals **1** (no regression).
- [ ] `grep -c '^# Phase 13:' run_firestaff_m10_verify.sh` still
      equals **1** (no regression).
- [ ] `$OUT_DIR/savegame/savegame_probe.md` exists and is non-empty.
- [ ] `$OUT_DIR/savegame/savegame_invariants.md` exists, its
      trailing line is `Status: PASS`, it contains
      `Invariant count: N` where `N ≥ 30` (target 40), and every
      line starting with `- ` begins with `- PASS:`.
- [ ] A live save/load round-trip of a synthetic populated state
      succeeds on disk: invariant #41 writes the file, re-reads it,
      and confirms byte-equality with the in-memory save.
- [ ] A post-load combat turn (invariant #45) produces a result
      bit-identical to the pre-save reference — proves the save
      preserves not just bytes but game semantics.
- [ ] `cc -Wall -Wextra -c` of every new `.c` file emits zero
      warnings.
- [ ] `ls tmp/firestaff/.phase*-attempt-* 2>/dev/null` returns
      nothing — no orphan backup directories.
- [ ] `git status` shows only the five files listed in §7 (no
      collateral edits).
- [ ] All 14 previous phases still pass — confirmed by the verify
      script exiting 0.

---

*End of plan. The Codex ACP executor reads this file in full,
then follows §6 steps 1→12 without deviation. For any size
constant or struct field name, the executor re-reads the
as-merged subsystem header (not this plan) — this plan is the
**process** source of truth; the subsystem headers are the **layout**
source of truth.*
