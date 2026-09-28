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
proprietary layout. The exact game-state fields and load consumer are not
source-locked in this codebase. Four authentic 32 KiB Mednafen Backup RAM
images in the private real-data corpus contain a `DMNEXUS__01` entry. The
savegame editor reads their block chains and exposes each exact 20,480-byte
payload as read-only hex. This verifies container extraction, not payload
semantics.

The authentic `DM.BIN` does contain save-related diagnostic strings in the
retail data image: `EV_SAVE`, `EV_SAVELOAD`, `Slot Operation Error`, and the
`iwa\\loader.c` source label. Their file offsets are recorded as `0x36fec`,
`0x36ff4`, `0x36c58`, and `0x36980` respectively (SH-2 address base
`0x06010000`). These are string/provenance observations only. No verified
pointer chain from those strings to a Saturn backup-RAM record, checksum,
load destination, or playable level/position has been established, so they do
not authorize a native Saturn save decoder.

## Boundary

FNXS is a Firestaff interchange/resume format. The four real payloads differ
across the two-, three-, and four-champion samples. Bytes `0x0a..0x0b` read as
big-endian `0x0434`, `0x0514`, and `0x05f4` in the two-, three-, and
four-champion samples respectively; the adjacent differences are `0xe0`
(224). The independent four-champion sample has the same observed value.
This is a count-correlated boundary/extent observation, not a field
assignment or a proven 224-byte champion record. Original Saturn save import
remains gated until the payload mapping and source-owned consumer are
identified and exercised against the retail game.
