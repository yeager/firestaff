# Nexus V1 Save/Load System

## Status: Firestaff-native save/load implemented

Nexus V1 supports the portable little-endian `FNXS` format, version 3,
through `src/nexus/nexus_v1_save_load.c` and
`include/nexus_v1_save.h`. It supports slot scanning, atomic writes, CRC
validation, bounded full-state serialization, and path-based load/save
helpers. The save browser and quick-resume route classify FNXS files as Nexus
saves.

This is Firestaff-native persistence. It is not an implementation of the
original Saturn backup-RAM format.

## Persisted state

The native serializer covers the Firestaff runtime state: resume level and
party position/direction, game time and state hash, champion-pool data,
inventory and world sections. The optional `NGLT` light-runtime blob is
appended and validated independently of the FNXS header.

The loader rejects bad magic, unsupported version, truncation, invalid sizes,
and CRC mismatches. Writes use a temporary file and atomic rename.

## Original Saturn boundary

The original Dungeon Master Nexus stored saves in Saturn backup RAM or
memory-card storage. Four authentic Mednafen BKR images with the
`DMNEXUS__01` save entry are available in the private real-data corpus. The
savegame editor reads their block chains and displays the exact 20,480-byte
payload as read-only hex. Read-only container inspection also shows the raw
language and timestamp fields and exact allocated block IDs; it assigns no
meaning to those fields.

A 2026-10-29 Ymir cold-boot trace with an authentic two-champion BKR now
source-binds the retail Continue payload transfer: the retail game reads the
complete `DMNEXUS__01` entry and copies it to WRAM-L at `0x00204000`, then
checks its checksum. The final memory snapshot differs from the BKR at four
payload bytes. A targeted write trace identifies a stack-derived 32-bit value
written by PC `0x0602ec94` over payload offset `0x58` nine frames after the
copy; the field meaning and whether this is an intentional runtime relocation
remain unknown. This establishes container-to-runtime transfer, but not
payload fields, the later consumer that applies the loaded state, or a
Firestaff engine-state mapping. Firestaff's Saturn import and Continue
handoff therefore remain unimplemented. See
[`nexus_sh2_startup_decompilation.md`](nexus_sh2_startup_decompilation.md#continue-payload-transfer-2026-10-29).
