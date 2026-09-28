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
payload as read-only hex. Its game-state schema and source-owned load consumer
remain unverified, so original Saturn save import remains capture-gated.
