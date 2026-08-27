# Nexus `0DMSTRT.BIN` boot-stub evidence

Status: source/disassembly evidence only. This document does not make the
boot image executable in Firestaff and does not provide a gameplay start pose.

## Authenticated input

The original observation used a standalone, unmodified retail asset.  The
same bytes were rechecked on 2026-08-27 through Firestaff's in-memory
ISO9660 reader from the Japanese retail CUE in the configured Nexus data
directory (`~/.firestaff/data/nexus/Dungeon Master Nexus (Japan).cue`).  The
reader selected the CUE-declared data track and read `0DMSTRT.BIN` directly
from the ISO member; it did not extract, rewrite, or retain a game-data copy.

| Property | Value |
|---|---|
| Size | 39,516 bytes |
| SHA-256 | `8a026f155af27cfd43a33b29f7da5b75ee7b09b2c4f016fc3be1ebb4787d20b6` |
| SH-2 byte order | Big-endian 16-bit instruction words |
| Image base used for this static listing | `0x06010000` |

The image is admitted by `nexus_v1_0dmstrt_structure_admission`. That receipt
binds the retail bytes and their partition tables; it intentionally does not
claim code, relocation, load, execution, or presentation semantics.

The Japanese CUE observation establishes byte identity only.  It does not
make this boot image a new-game state record and does not bind a level,
coordinate, or facing value.

## Bounded entry evidence

The first words form this source-backed chain:

| File offset | Runtime address | Observation |
|---:|---:|---|
| `0x0000` | `0x06010000` | `MOV.L` loads the initial stack value `0x060FFFFC` |
| `0x0002` | `0x06010002` | moves that value into `R15` |
| `0x0004` | `0x06010004` | `MOV.L` loads the entry pointer `0x06010014` |
| `0x0006` | `0x06010006` | indirect jump through that pointer |
| `0x0014` | `0x06010014` | `BSR` to `0x06010026` |
| `0x0018` | `0x06010018` | loads `0x0601363C` from the literal pool |
| `0x001C` | `0x0601001C` | loads `0x06010074` and calls it indirectly |
| `0x0074` | `0x06010074` | prologue followed by calls to `0x06010278` and `0x060101F6` |

The same early block loads additional pointers to `0x06010888`,
`0x0601088C`, `0x06010890`, `0x06010894`, `0x06010898`, and `0x0601089C`.
Those are recorded as literal/pointer observations only. Their ownership and
the meaning of the pointed-to state require an original runtime trace or a
stronger disassembly proof.

## What this proves, and what it does not

This proves that the authentic `0DMSTRT.BIN` begins with an SH-2 boot-library
entry stub and that its literal pool contains internal image pointers. It does
not prove that the file is the consumer of `LEV01.DGN`, nor does it identify a
source-owned level, coordinate, or facing field. No pose is derived from the
first bytes, the relocation table, a first walkable cell, or a synthetic test
image.

Consequently the production Nexus launcher remains fail-closed until an
authenticated original execution trace or save consumer binds the startup
pose to `LEV01.DGN`.
