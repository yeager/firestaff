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

## Authentic Saturn samples available; semantics remain unresolved

The original Saturn save header, record size, field mapping, and load
consumer are not identified. Four authentic Mednafen Backup RAM images in the
private real-data corpus contain a `DMNEXUS__01` entry with a 20,480-byte
payload. Firestaff can inspect the Saturn container and preserve/expose the
payload as read-only hex, but the payload must not be decoded into game fields
by analogy with FNXS or another Dungeon Master version. Saturn save import
remains blocked until payload fields and the retail load consumer are
source-bound.
