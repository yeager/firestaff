# Nexus V1 Save File Format

## Status: Firestaff-native FNXS implemented; Saturn format unresolved

Firestaff has a portable, little-endian native save container named `FNXS`.
It is used for Firestaff resume, save browsing, and save export/import. It is
not the original Dungeon Master Nexus Saturn memory-card format.

The current native format is `NEXUS_SAVE_VERSION = 3` and is implemented by
`src/nexus/nexus_v1_save_load.c`.

## FNXS container

The fixed header is `Nexus_V1_SaveHeader` in `include/nexus_v1_save.h`. It
contains the `FNXS` magic, version, header and section sizes, CRC-32, game
time, resume level/position/direction, state hash, and a bounded description.
The data section contains serialized champion and world sections. The
optional `NGLT` light-runtime blob is appended after those sections and
validated independently.

The loader rejects unknown magic, unsupported version, truncation, invalid
sizes, and CRC mismatch. Writes use a temporary file and atomic rename.

## Original Saturn format context

The original Saturn game used backup-RAM or memory-card storage with a
proprietary layout. The exact save-record header, record size, and fields are
not source-locked in this codebase. The observed empty 512 KiB Mednafen
container is only a container-size observation, not a decoded save format.

## Boundary

FNXS is a Firestaff interchange/resume format. Original Saturn save import
remains capture-gated until a played, hash-bound memory image and its
source-owned consumer are identified.
