# Pass447 — DM1 V1 thieves-eye door mask source lock

- ReDMCSB primary source:
  - `STARTUP2.C:695` maps `C15_DOOR_ORNAMENT_DESTROYED_MASK`/`C16_DOOR_ORNAMENT_THIEVES_EYE_MASK` to `M649_GRAPHIC_DOOR_MASK_DESTROYED + {0,1}`.
  - `DUNVIEW.C:4294` draws `C16_DOOR_ORNAMENT_THIEVES_EYE_MASK` only for the D1C door view when Thieves Eye is active.
  - `DEFS.H:2385` has `M649_GRAPHIC_DOOR_MASK_DESTROYED = 439`; `DEFS.H:2466-2467` has `C15=15`, `C16=16`, so C16 resolves to GRAPHICS.DAT entry `440`.
- Hash-locked local sources:
  - DM PC 3.4 English `GRAPHICS.DAT`: SHA256 `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`, registry MD5 `FA6B1AA29E191418713BF2CDA93D962E`.
  - DM PC 3.4 Multilanguage `GRAPHICS.DAT`: SHA256 `291eb38eab683317a2500e13363148425f059a2d35f929257d809174f625a4dc`, registry MD5 `F934D97E43E1BA6E5159839ACBCD0611`.
- Verified atlas entries on both variants:
  - entry `439`: `96x88`, destroyed-door mask.
  - entry `440`: `80x74`, thieves-eye mask.
  - entry `441`: first normal door ornament (`30x19`), proving the C16 mask is not in the normal `M617_GRAPHIC_FIRST_DOOR_ORNAMENT` block.
- Firestaff fix:
  - `m11_game_view.c` adds `M11_GFX_DOOR_MASK_THIEVES_EYE = 440`.
  - `m11_draw_dm1_center_thieves_eye_mask()` now loads entry `440` when `state->world.lifecycle.status.thievesEyeCount > 0` and the D1C cell is a door.

Verifier: `tools/verify_pass447_dm1_v1_thieves_eye_door_mask_source_lock.py`.
