# Nexus V1 Save File Format

## Status: Firestaff-native FNXS implemented; Saturn payload transfer verified, schema unresolved

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
payload as read-only hex. It also exposes the raw language/timestamp header
fields and exact allocated block IDs as read-only container metadata. This
verifies container extraction, not payload semantics. A read-only comparison
view reports contiguous payload-relative byte ranges and bounded raw-byte
previews for two images. Its self-test admits only the four captured files by
exact filename and SHA-256; other or incomplete corpora cannot be reported as
authentic-corpus passes. Neither the view nor the test assigns meaning to
changed bytes, and neither can write or convert a Saturn image.

A separate 2026-10-29 cold-boot Ymir trace now joins the complete authentic
20,480-byte `DMNEXUS__01` payload address sequence to SH-2 PC `0x06003680`.
Ghidra caller analysis binds the read to the named-entry lookup, destination
WRAM-L address `0x00204000`, and a retail checksum check. The captured final
buffer matches all but four payload bytes (`0x58..0x5b`); the write timing and
cause are unknown. This identifies the retail container-to-buffer transfer,
not the payload's game-state schema or its post-load consumer. Firestaff does
not yet import this format or apply its data to the game engine. Details and
hashes are recorded in
[`nexus_sh2_startup_decompilation.md`](nexus_sh2_startup_decompilation.md#continue-payload-transfer-2026-10-29)
and the operator-managed external evidence bundle.

The authentic `DM.BIN` does contain save-related diagnostic strings in the
retail data image: `EV_SAVE`, `EV_SAVELOAD`, `Slot Operation Error`, and the
`iwa\\loader.c` source label. Their file offsets are recorded as `0x36fec`,
`0x36ff4`, `0x36c58`, and `0x36980` respectively (SH-2 address base
`0x06010000`). These are string/provenance observations only. No verified
pointer chain from those strings to a Saturn backup-RAM record, checksum,
load destination, or playable level/position has been established, so they do
not authorize a native Saturn save decoder.

A read-only Ghidra 12.2 follow-up used the same `DM.BIN` hash
(`3bbca125e0bfb486897e4926541e7c31adbff010d01a9b0c736637f432aad124`). The
`iwa\\loader.c` references lead to generic resource-loader functions at
`0x06017e30`, `0x06017e6e`, `0x06018298`, and `0x06018520`: they validate and
relocate resource descriptors and load resource sections. They do not show a
Saturn Backup RAM reader or writer. The save-event and slot-error strings
resolve through data references only in this analysis. This rules out that
loader label as evidence for a Saturn save consumer; it does not rule out a
different, as-yet-unidentified consumer elsewhere in `DM.BIN`. The raw Ghidra
references and decompilation are retained outside the repository at
`/Volumes/Extern-disk/firestaff-evidence/nexus-save-loader-decomp-20261027/`.

## Boundary

FNXS is a Firestaff interchange/resume format. The four hash-pinned real
payloads differ across the two-, three-, and four-champion samples. Bytes
`0x0a..0x0b` read as big-endian `0x0434`, `0x0514`, and `0x05f4` in the two-,
three-, and four-champion samples respectively; these values follow
`0x0274 + champion_count * 0x00e0`. Every byte from the observed value to the
end of the 20,480-byte payload is zero in these four samples. The independent
four-champion sample has the same observed value. This is a count-correlated
payload-boundary observation; it does not identify the boundary field's
consumer or prove a 224-byte champion record. Original Saturn save import
remains gated until the payload mapping and source-owned consumer are
identified and exercised against the retail game.
