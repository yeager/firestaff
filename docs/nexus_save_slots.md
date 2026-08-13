# Nexus V1 Save Slot System

## Status: Firestaff-native slots implemented

Firestaff-native FNXS persistence exposes `NEXUS_SAVE_MAX_SLOTS = 8` through
`Nexus_V1_SaveManager`. It scans, validates, lists, writes, loads, and deletes
FNXS slot files. The slot browser is a Firestaff UI feature and must not be
confused with the original Saturn memory-card record layout.

## API boundary

- `nexus_v1_save_scan()` enumerates and validates slots.
- `nexus_v1_save()` and `nexus_v1_load()` write and read validated FNXS data.
- `nexus_v1_save_delete()` removes a Firestaff-native slot.
- The original Saturn save route remains blocked until a played,
  source-bound memory-card consumer is captured.
