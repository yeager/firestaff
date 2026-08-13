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

## Not yet source-locked from Saturn

The original Saturn save header, record size, field mapping, and load
consumer are not identified. The available empty backup-RAM containers do not
provide a played save and cannot be used as substitute data. Saturn save
import therefore remains capture-gated.
