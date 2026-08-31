# Firestaff TODO — DM2

Reviewed 2026-08-29. Only open work is listed here.

- Replace the active fixed 320x200 M11 dungeon compositor with the original
  224x136 backbuffer and `RECT_7` presentation route. Port the source
  `DM2_DISPLAY_VIEWPORT` pass order from the real PC-DOS route; retain only
  GDAT-owned pixels and verify it against a same-tuple original capture.
- Extend real-media gameplay evidence across DOS, Amiga, FM Towns and Mac for
  dialog/input ordering, creature AI/drop routes, audio and save/resume.
- Capture an original PC-DOS `SKSAVE1` WIELD input-to-CD/RAM trace with a
  valid encounter, weapon choice, command arguments and RNG timing, then
  bind the remaining WIELD fallback/luck and creature-drop route to that
  trace. Do not replace it with a generated weapon, creature, save or combat
  result; keep the creature-drop gate closed until the original interaction
  identifies a valid route.
- Bind renderer/HUD V2.2 material, clipping and outdoor routes to original
  GDAT/capture evidence; synthetic V2.2 art is allowed only as a fixture.
