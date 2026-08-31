# Firestaff TODO — DM2

Reviewed 2026-08-29. Only open work is listed here.

- Extend real-media gameplay evidence across DOS, Amiga, FM Towns and Mac for
  dialog/input ordering, creature AI/drop routes, audio and save/resume.
- Implement the Macintosh retail `Cmd-O` Open Game owner as a native
  original-save selection dialog. The input resolver already identifies it
  as `DM2_V1_MAC_ACTION_OPEN_GAME`, and the selected-file importer already
  validates a chosen SKSAVE member in memory. Do not map it to Back,
  quickload, or a synthesized default slot: the missing part is only the
  source-faithful selection UI and its handoff of the user's exact original
  member to `DM2_GAME_LOAD`.
- Capture an original PC-DOS `SKSAVE1` WIELD input-to-CD/RAM trace with a
  valid encounter, weapon choice, command arguments and RNG timing, then
  bind the remaining WIELD fallback/luck and creature-drop route to that
  trace. Do not replace it with a generated weapon, creature, save or combat
  result; keep the creature-drop gate closed until the original interaction
  identifies a valid route.
- Bind renderer/HUD V2.2 material, clipping and outdoor routes to original
  GDAT/capture evidence; synthetic V2.2 art is allowed only as a fixture.
