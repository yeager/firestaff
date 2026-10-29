# Nexus V1 Saved State

## Status: engine and Firestaff-native save exist

The Firestaff-native FNXS serializer persists the bounded runtime state. It
must not be confused with a decoded Saturn save record.

## State covered by FNXS

- Resume level, party coordinates, facing, game time, and state hash.
- Champion pool, party membership, leader, authenticated PLRD-derived
  champion fields, inventory, and runtime values.
- Serialized world state and the optional `NGLT` light-runtime section.

The native format is little-endian, version 3, CRC-protected, and bounded by
the serializers in `src/nexus/nexus_v1_save_load.c`.

## Authentic Saturn transfer verified; state semantics remain unresolved

Four authentic Mednafen Backup RAM images in the private real-data corpus
contain a `DMNEXUS__01` entry with a 20,480-byte payload. A 2026-10-29 Ymir
cold-boot trace and SH-2 caller analysis source-bind retail lookup, transfer to
WRAM-L at `0x00204000`, and checksum validation. The final captured buffer has
four changed bytes relative to the BKR payload; their writer and timing are
unknown. The payload fields and later consumer that applies loaded state to
the retail engine are not identified. Preserve the payload as opaque
read-only bytes: do not infer champion or world fields by analogy with FNXS or
another Dungeon Master version. Saturn save import remains blocked until those
fields and the post-load state mapping are source-bound.
