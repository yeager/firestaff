# Firestaff TODO — Theron's Quest

Reviewed 2026-08-25. Only open work is listed here.

The supplied Japanese Rev 1 Track 02 has been checked directly by
`theron_v1_track02_level_data_blocks`: its raw sectors bind the seven
described level blocks, their shared prologue and per-level metadata, while
mutated bytes are rejected. This is source-data verification only; it does
not promote uncaptured transition, presentation, save or item-action logic.

- Bind the verified Japanese Rev 1 Track 02 source dungeons to captured
  transition, champion, save and item-action consumers; the current all-seven
  dungeon loader remains source-only.
- Capture and decode original bitmap, palette, text and audio ownership for
  production presentation; fallback visuals remain disabled.
- Verify JP and US runtime, save and later-dungeon behavior separately. Do
  not infer JP offsets or gameplay semantics from US media.
